/* pcap2hcap_and_beyond.c
 *
 * hashkill - a hash cracking tool
 * Copyright (C) 2010 Milen Rangelov <gat3way@gat3way.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 *
 * Shamelessly stolen from:
 * https://github.com/gat3way/hashkill/blob/master/src/tools/pcap2hcap.c
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>


#define AP_MAX 1024*1024
#define HSHAKE_MAX 1024*1024
//change to 1 to output hccap to file
//#define OUTPUT_HCCAP 0
//change to 1 to output verbose info
//#define PRINT_TO_SCREEN 1

//my special file
//#define NAME_OF_OUTPUT_FILE "cap.txt"
typedef struct pcap_hdr_s
{
    unsigned int magic_number;   /* magic number */
    unsigned short version_major;/* major version number */
    unsigned short version_minor;/* minor version number */
    int  thiszone;               /* GMT to local correction */
    unsigned int sigfigs;        /* accuracy of timestamps */
    unsigned int snaplen;        /* max length of captured packets, in octets */
    unsigned int network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s
{
    unsigned int ts_sec;         /* timestamp seconds */
    unsigned int ts_usec;        /* timestamp microseconds */
    unsigned int incl_len;       /* number of octets of packet saved in file */
    unsigned int orig_len;       /* actual length of packet */
} pcaprec_hdr_t;


/* This is the hashcat format header, taken from hashcat wiki */
typedef struct
{
    char          essid[36];
    unsigned char mac1[6];
    unsigned char mac2[6];
    unsigned char nonce1[32];
    unsigned char nonce2[32];
    unsigned char eapol[256];
    int           eapol_size;
    int           keyver;
    unsigned char keymic[16];
} hccap_t;


/* Found APs. We currently detect them from Beacon frames. TBD: detection from association/probe frames */
typedef struct aps_s
{
    unsigned char essid[36];	/* ESSID */
    unsigned char mac[6];	/* MAC */
} aps_t;


typedef struct handshakes_s
{
    unsigned char amac[6];		/* AP MAC */
    unsigned char smac[6];		/* Supplicant MAC */
    unsigned char anonce[32];		/* AP Nonce */
    unsigned char snonce[32];		/* Supplicant nonce */
    unsigned char finalkey[16];		/* Final MIC key */
    unsigned char eapol[256];		/* EAPOL packet data */
    int eapol_size;			/* EAPOL data size */
    int keyver;				/* 1 for WPA, 2 for WPA2 */
    char steps[4];			/* handshake state */
    unsigned char pmkid[16];		/* PMKID, not used?!? */
    unsigned char essid[36];		/* ESSID of the AP */
    int replaycount;			/* replay counter */
    int grade;				/* Not used ?!? */
    int good;				/* handshake quality */
} handshake_t;

aps_t 		aps[AP_MAX];
handshake_t 	hshake[HSHAKE_MAX];
handshake_t 	hshake2[HSHAKE_MAX];
hccap_t		hccap;
int 		aps_c=0;
int 		hshake_c=0;
int 		hshake2_c=0;
int		incorrect=0;	/* incorrect mode (-b) */
int		getbest=0;	/* getbest mode (-g) */
int		csv=0;		/* csv mode (-g) */


/* usage routine */
void usage(char *self)
{
    printf("Usage: %s <pcapfile> <output_file_name> <print_to_screen_flag (0|1) > [<hcapfile>] [-b] [-g]\nWhen -b provided, checks are less strict meaning that bad handshakes will be displayed\nWhen -g provided, pcap2hcap will automatically choose the best handshake\n\n",self);
}


/* Real work */
int parse_pcap(char *filename)
{
    int erev=0;
    int fd,err,count=0,a,b,c;
    pcap_hdr_t mainhead;
    pcaprec_hdr_t rhead;
    unsigned char buf[65535];
    unsigned short framecontrol;
    unsigned int u321;
    unsigned short u161;
    unsigned char u81,u82;
    int keyver;
    unsigned char src[6];
    unsigned char dst[6];
    unsigned char rxst[6];
    unsigned char txst[6];
    unsigned char essid[36];
    int headerseek;
    int haveqos;

    /* Initialization */
    for (a=0;a<HSHAKE_MAX;a++) 
    {
	hshake[a].steps[0]=0;
	hshake[a].steps[1]=0;
	hshake[a].steps[2]=0;
	hshake[a].steps[3]=0;
	bzero(hshake[a].essid,36);
    }

    fd = open(filename,O_RDONLY);
    if (fd<0)
    {
        printf("Cannot open pcap file: %s\n",filename);
        return 0;
    }


    /* Read in main pcap header. TBD: endianness fix (see erev variable)!!! */
    read(fd,&mainhead,sizeof(pcap_hdr_t));
    if (mainhead.magic_number==0xa1b2c3d4) erev=0;
    else if (mainhead.magic_number==0xd4c3b2a1) erev=1;
    else 
    {
        printf("Not a pcap dump file!%s\n",filename);
        return 0;
    }


    /* Is that a 802.11 dump? */
    if ((mainhead.network!=105)&&(mainhead.network!=127)&&(mainhead.network!=163)&&(mainhead.network!=192))
    {
	printf("pcap dump not from 802.11 network! (type=%d)\n",mainhead.network);
    	return 0;
    }

    /* Radiotap header is always 24 bytes */
    if (mainhead.network==105) headerseek=0;
    if (mainhead.network==127) headerseek=24;
    if (mainhead.network==192) headerseek=8;


    err=1;
    while (err>0)
    {
        /* Read the pcap packet header */
        count++;
        read(fd,&rhead,sizeof(pcaprec_hdr_t));

        /* Skip the radiotap header if exists */
        lseek(fd,headerseek,SEEK_CUR);

        /* Read in the whole packet */
        err=read(fd,buf,rhead.incl_len-headerseek);
        memcpy(src,&buf[10],6);
        memcpy(dst,&buf[4],6);

        /* Do we have a beacon frame? */
        memcpy(&framecontrol,&buf[0],2);
        if (framecontrol==0x80) 
        {
            memcpy(&u81,&buf[37],1);
    	    memcpy(essid,&buf[38],u81);
    	    essid[(int)u81]=0;
    	    /* Try to add it if that's new */
    	    if (aps_c<AP_MAX)
    	    {
    		b=-1;
    		for (a=0;a<aps_c;a++) if (memcmp(aps[a].mac,src,6)==0) b=a;
    		if (b<0)
    		{
    		    memcpy(aps[aps_c].mac,src,6);
    		    strcpy(aps[aps_c].essid,essid);
    		    aps_c++;
    		}
    	    }
    	}

        /* Do we have a probe response frame? */
        memcpy(&framecontrol,&buf[0],2);
        if (framecontrol==0x50) 
        {
            memcpy(&u81,&buf[37],1);
    	    memcpy(essid,&buf[38],u81);
    	    essid[(int)u81]=0;
    	    /* Try to add it if that's new */
    	    if (aps_c<AP_MAX)
    	    {
    		b=-1;
    		for (a=0;a<aps_c;a++) if (memcmp(aps[a].mac,src,6)==0) b=a;
    		if (b<0)
    		{
    		    memcpy(aps[aps_c].mac,src,6);
    		    strcpy(aps[aps_c].essid,essid);
    		    aps_c++;
    		}
    	    }
    	}

	
	if (incorrect==0)
	{
	    /* Do we have an AP challenge? */
	    if ((framecontrol==0x0208)||(framecontrol==0x0288)||(framecontrol==0x0A88)||(framecontrol==0x0A08))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x288)||(framecontrol==0xA88)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);

		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a zero key info byte 1? It must be an AP challenge*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if (((u81==2)||(u81==254))&&((u161&255)==0))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			if (hshake_c<HSHAKE_MAX)
			{
			    memcpy(hshake[hshake_c].amac,src,6);
			    memcpy(hshake[hshake_c].smac,dst,6);
			    hshake[hshake_c].steps[0]=1;
			    memcpy(hshake[hshake_c].anonce,&buf[49+haveqos],32);
			    memcpy(&u81,&buf[48+haveqos],1);
			    hshake[hshake_c].replaycount=u81;
			    hshake[hshake_c].keyver=keyver;
			    hshake_c++;
			}
		    }
		}
	    }

	    /* Do we have an SP response? */
	    if ((framecontrol==0x0108)||(framecontrol==0x0188)||(framecontrol==0x0988)||(framecontrol==0x0908))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x188)||(framecontrol==0x988)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);
		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a 1 key info byte 1? It must be an SP response*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if (((u81==2)||(u81==254))&&((u161&255)==1))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			memcpy(&u81,&buf[48+haveqos],1);
			b=-1;
			for (a=0;a<hshake_c;a++) if ( (memcmp(hshake[a].smac,src,6)==0)&&(memcmp(hshake[a].amac,dst,6)==0)&&(hshake[a].steps[2]==0)&&(hshake[a].steps[3]==0)&&(hshake[a].steps[3]==0)&&(hshake[a].steps[1]==0)&&(hshake[a].replaycount==u81) &&(hshake[a].keyver==keyver)) b=a;
			/* New entry? */
			if (b<0)
			{
			    if (hshake_c<HSHAKE_MAX)
			    {
				memcpy(hshake[hshake_c].smac,src,6);
				memcpy(hshake[hshake_c].amac,dst,6);
				hshake[hshake_c].steps[1]=1;
				hshake[hshake_c].replaycount=u81;
				memcpy(hshake[hshake_c].snonce,&buf[49+haveqos],32);
				hshake[hshake_c].keyver=keyver;
				memcpy(hshake[hshake_c].finalkey,&buf[113+haveqos],16);
				bzero(&buf[0x71+haveqos],16);
				memcpy(&u81,&buf[35+haveqos],1);
				bzero(&buf[0x71+haveqos],16);
				hshake[hshake_c].eapol_size=u81+4;
				memcpy(hshake[hshake_c].eapol,&buf[32+haveqos],u81+4);
				hshake_c++;
			    }
			}
			else
			{
			    memcpy(hshake[b].smac,src,6);
			    memcpy(hshake[b].amac,dst,6);
			    hshake[b].steps[1]=1;
			    hshake[b].keyver=keyver;
			    memcpy(hshake[b].snonce,&buf[49+haveqos],32);
			    memcpy(hshake[b].finalkey,&buf[113+haveqos],16);
			    memcpy(&u81,&buf[35+haveqos],1);
			    bzero(&buf[0x71+haveqos],16);
			    hshake[b].eapol_size=u81+4;
			    memcpy(hshake[b].eapol,&buf[32+haveqos],u81+4);
			}
		    }
		}
	    }
	    /* Do we have an AP confirmation packet? */
	    if ((framecontrol==0x0208)||(framecontrol==0x0288)||(framecontrol==0x0A88)||(framecontrol==0x0A08))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x288)||(framecontrol==0xA88)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);
		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a 19 or 1 key info byte 1? It must be an SP response*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if (((u81==2)||(u81==254))&&(((u161&255)==19)||((u161&255)==1)))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			memcpy(&u81,&buf[48+haveqos],1);
			b=-1;
			for (a=0;a<hshake_c;a++) if ( (memcmp(hshake[a].smac,dst,6)==0)&&(memcmp(hshake[a].amac,src,6)==0)&&(hshake[a].steps[2]==0)&&(hshake[a].steps[3]==0)&&(hshake[a].replaycount==u81-1) &&(hshake[a].keyver==keyver)) b=a;
			/* New entry? - why do we insert it anyway? It's bad data */
			if (b<0)
			{
			    if (hshake_c<HSHAKE_MAX)
			    {
				memcpy(hshake[hshake_c].smac,dst,6);
				memcpy(hshake[hshake_c].amac,src,6);
				hshake[hshake_c].steps[2]=1;
				hshake[hshake_c].replaycount=u81;
				memcpy(hshake[hshake_c].anonce,&buf[49+haveqos],32);
				hshake[hshake_c].keyver=keyver;
				hshake_c++;
			    }
			}
			else
			{
			    memcpy(hshake[b].smac,dst,6);
			    memcpy(hshake[b].amac,src,6);
			    hshake[b].steps[2]=1;
			    hshake[b].keyver=keyver;
			    hshake[b].replaycount=u81;
			    memcpy(hshake[b].anonce,&buf[49+haveqos],32);
			}
		    }
		}
	    }

	    /* Best handshakes should contain the final packet though it's not needed actually*/
	    if ((framecontrol==0x0108)||(framecontrol==0x0188)||(framecontrol==0x0988)||(framecontrol==0x0908))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x188)||(framecontrol==0x988)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);
		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a 3 key info byte 1? It must be a SP final confirmation*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if ( (((u81==254)&&((u161&255)==1))||((u81==2)&&((u161&255)==3)))&&(buf[49+haveqos]==buf[50+haveqos]==buf[51+haveqos]==buf[52+haveqos]==buf[53+haveqos]==buf[54+haveqos]==buf[55+haveqos]==0))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			memcpy(&u81,&buf[48+haveqos],1);
			b=-1;
			for (a=0;a<hshake_c;a++) 
			if ( (memcmp(hshake[a].smac,src,6)==0)&&(memcmp(hshake[a].amac,dst,6)==0)&&(hshake[a].steps[3]==0)&&(hshake[a].steps[2]==1)&&(hshake[a].steps[1]==1) &&(hshake[a].keyver==keyver)) b=a;
			/* New entry? - why do we insert it anyway? It's bad data */
			if (b<0)
			{
			    if (hshake_c<HSHAKE_MAX)
			    {
				memcpy(hshake[hshake_c].smac,src,6);
				memcpy(hshake[hshake_c].amac,dst,6);
				hshake[hshake_c].steps[3]=1;
				hshake[hshake_c].keyver=keyver;
				hshake_c++;
			    }
			}
			else
			{
			    memcpy(hshake[b].smac,src,6);
			    memcpy(hshake[b].amac,dst,6);
			    hshake[b].steps[3]=1;
			    hshake[b].keyver=keyver;
			}
		    }
		}
	    }
	}
	
	/* Try to get bad ones if -b provided */
	else
	{
	    /* Do we have an AP challenge? */
	    if ((framecontrol==0x0208)||(framecontrol==0x0288)||(framecontrol==0x0A88)||(framecontrol==0x0A08))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x288)||(framecontrol==0xA88)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);

		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a zero key info byte 1? It must be an AP challenge*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if (((u81==2)||(u81==254))&&((u161&255)==0))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			if (hshake_c<HSHAKE_MAX)
			{
			    memcpy(hshake[hshake_c].amac,src,6);
			    memcpy(hshake[hshake_c].smac,dst,6);
			    hshake[hshake_c].steps[0]=1;
			    hshake[hshake_c].steps[2]=1;
			    memcpy(hshake[hshake_c].anonce,&buf[49+haveqos],32);
			    memcpy(&u81,&buf[48+haveqos],1);
			    hshake[hshake_c].replaycount=u81;
			    hshake[hshake_c].keyver=keyver;
			    hshake_c++;
			}
		    }
		}
	    }

	    /* Do we have an SP response? */
	    if ((framecontrol==0x0108)||(framecontrol==0x0188)||(framecontrol==0x0988)||(framecontrol==0x0908))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x188)||(framecontrol==0x988)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);
		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a 1 key info byte 1? It must be an SP response*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if (((u81==2)||(u81==254))&&((u161&255)==1))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			memcpy(&u81,&buf[48+haveqos],1);
			b=-1;
			for (a=0;a<hshake_c;a++) if ( (memcmp(hshake[a].smac,src,6)==0)&&(memcmp(hshake[a].amac,dst,6)==0)/*&&(hshake[a].steps[3]==0)&&(hshake[a].steps[3]==0)&&(hshake[a].steps[1]==0)&&(hshake[a].replaycount==u81)*/ &&(hshake[a].keyver==keyver)) b=a;
			/* New entry? */
			if (b<0)
			{
			    if (hshake_c<HSHAKE_MAX)
			    {
				memcpy(hshake[hshake_c].smac,src,6);
				memcpy(hshake[hshake_c].amac,dst,6);
				hshake[hshake_c].steps[1]=1;
				hshake[hshake_c].replaycount=u81;
				memcpy(hshake[hshake_c].snonce,&buf[49+haveqos],32);
				hshake[hshake_c].keyver=keyver;
				memcpy(hshake[hshake_c].finalkey,&buf[113+haveqos],16);
				bzero(&buf[0x71+haveqos],16);
				memcpy(&u81,&buf[35+haveqos],1);
				bzero(&buf[0x71+haveqos],16);
				hshake[hshake_c].eapol_size=u81+4;
				memcpy(hshake[hshake_c].eapol,&buf[32+haveqos],u81+4);
				hshake_c++;
			    }
			}
			else
			{
			    memcpy(hshake[b].smac,src,6);
			    memcpy(hshake[b].amac,dst,6);
			    hshake[b].steps[1]=1;
			    hshake[b].keyver=keyver;
			    memcpy(hshake[b].snonce,&buf[49+haveqos],32);
			    memcpy(hshake[b].finalkey,&buf[113+haveqos],16);
			    memcpy(&u81,&buf[35+haveqos],1);
			    bzero(&buf[0x71+haveqos],16);
			    hshake[b].eapol_size=u81+4;
			    memcpy(hshake[b].eapol,&buf[32+haveqos],u81+4);
			}
		    }
		}
	    }
	    /* Do we have an AP confirmation packet? */
	    if ((framecontrol==0x0208)||(framecontrol==0x0288)||(framecontrol==0x0A88)||(framecontrol==0x0A08))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x288)||(framecontrol==0xA88)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);
		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a 19 or 1 key info byte 1? It must be an SP response*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if (((u81==2)||(u81==254))&&(((u161&255)==19)||((u161&255)==1)))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			memcpy(&u81,&buf[48+haveqos],1);
			b=-1;
			for (a=0;a<hshake_c;a++) if ( (memcmp(hshake[a].smac,dst,6)==0)&&(memcmp(hshake[a].amac,src,6)==0)&&(hshake[a].steps[3]==0)/*&&(hshake[a].replaycount==u81-1)*/ &&(hshake[a].keyver==keyver)) b=a;
			/* New entry? - why do we insert it anyway? It's bad data */
			if (b<0)
			{
			    if (hshake_c<HSHAKE_MAX)
			    {
				memcpy(hshake[hshake_c].smac,dst,6);
				memcpy(hshake[hshake_c].amac,src,6);
				hshake[hshake_c].steps[2]=1;
				hshake[hshake_c].replaycount=u81;
				memcpy(hshake[hshake_c].anonce,&buf[49+haveqos],32);
				hshake[hshake_c].keyver=keyver;
				hshake_c++;
			    }
			}
			else
			{
			    memcpy(hshake[b].smac,dst,6);
			    memcpy(hshake[b].amac,src,6);
			    hshake[b].steps[2]=1;
			    hshake[b].keyver=keyver;
			    hshake[b].replaycount=u81;
			    memcpy(hshake[b].anonce,&buf[49+haveqos],32);
			}
		    }
		}
	    }

	    /* Best handshakes should contain the final packet though it's not needed actually*/
	    if ((framecontrol==0x0108)||(framecontrol==0x0188)||(framecontrol==0x0988)||(framecontrol==0x0908))
	    {
		/* Peek at the SNAP header, is frame type 802.1X authentication? */
		if ((framecontrol==0x188)||(framecontrol==0x988)) haveqos=2;
		else haveqos=0;
		memcpy(&u161,&buf[30+haveqos],2);
		if (u161==0x8e88)
		{
		    /* EAPOL RSN/WPA key? Then we have a 3 key info byte 1? It must be a SP final confirmation*/
		    memcpy(&u81,&buf[36+haveqos],1);
		    memcpy(&u82,&buf[38+haveqos],1);
		    memcpy(&u161,&buf[37+haveqos],2);
		    if ( (((u81==254)&&((u161&255)==1))||((u81==2)&&((u161&255)==3)))&&(buf[49+haveqos]==buf[50+haveqos]==buf[51+haveqos]==buf[52+haveqos]==buf[53+haveqos]==buf[54+haveqos]==buf[55+haveqos]==0))
		    {
			keyver = ((u82&3)==2) ? 2 : 1;
			memcpy(&u81,&buf[48+haveqos],1);
			b=-1;
			for (a=0;a<hshake_c;a++) 
			if ( (memcmp(hshake[a].smac,src,6)==0)&&(memcmp(hshake[a].amac,dst,6)==0)&&(hshake[a].steps[3]==0)&&(hshake[a].steps[2]==1)&&(hshake[a].steps[1]==1) &&(hshake[a].keyver==keyver)) b=a;
			/* New entry? - why do we insert it anyway? It's bad data */
			if (b<0)
			{
			    if (hshake_c<HSHAKE_MAX)
			    {
				memcpy(hshake[hshake_c].smac,src,6);
				memcpy(hshake[hshake_c].amac,dst,6);
				hshake[hshake_c].steps[3]=1;
				hshake[hshake_c].keyver=keyver;
				hshake_c++;
			    }
			}
			else
			{
			    memcpy(hshake[b].smac,src,6);
			    memcpy(hshake[b].amac,dst,6);
			    hshake[b].steps[3]=1;
			    hshake[b].keyver=keyver;
			}
		    }
		}
	    }
	}
    }

/*
    printf("Access points:\n===================\n\n");
    for (a=0;a<aps_c;a++) printf("essid: %s  mac: %02x:%02x:%02x:%02x:%02x:%02x\n",aps[a].essid,aps[a].mac[0]&255,aps[a].mac[1]&255,aps[a].mac[2]&255,aps[a].mac[3]&255,aps[a].mac[4]&255,aps[a].mac[5]&255);
    printf("\n");

    printf("Handshakes:\n===================\n\n");
    for (a=0;a<hshake_c;a++)
    {
       printf(" SP: %02x%02x%02x%02x%02x%02x \n AP: %02x%02x%02x%02x%02x%02x \n keyver (WPA or WPA2) : %d\n",hshake[a].smac[0]&255,hshake[a].smac[1]&255,hshake[a].smac[2]&255,hshake[a].smac[3]&255,hshake[a].smac[4]&255,hshake[a].smac[5]&255,hshake[a].amac[0]&255,hshake[a].amac[1]&255,hshake[a].amac[2]&255,hshake[a].amac[3]&255,hshake[a].amac[4]&255,hshake[a].amac[5]&255,hshake[a].keyver);

       int i;
       printf(" ANONCE: \n");
       for(i=0;i<32;i++)
       {
          printf("%02x",hshake[a].anonce[i]&255);
       }

       printf("\n");

       printf(" SNONCE: \n");
       for(i=0;i<32;i++)
       {
          printf("%02x",hshake[a].snonce[i]&255);
       }

       printf("\n");
       printf(" EAPOL: \n");
       for(i;i<256;i++)
       {
          printf("%02x",hshake[a].eapol[i]&255);
       }
    }
    printf("\n");*/
    close(fd);
    return 1;
}

void print_eapol(char* eapol)
{
}
void rate_hshakes()
{
   int a,b;

   for (a=0;a<hshake_c;a++)
   {
      if (((hshake[a].steps[0]==1)||(hshake[a].steps[2]==1))&&(hshake[a].steps[1]==1))
      {
         hshake[a].good=1;
         if (hshake[a].steps[3]==1) hshake[a].good=2;
         if (hshake[a].steps[0]==1) hshake[a].good=3;
      }
      else hshake[a].good=0;
      //printf("id %d: %d %d %d %d\n",a,hshake[a].steps[0],hshake[a].steps[1],hshake[a].steps[2],hshake[a].steps[3]);
      /* Add essid */
      for (b=0;b<aps_c;b++) if (memcmp(aps[b].mac,hshake[a].amac,6)==0) strcpy(hshake[a].essid,aps[b].essid);
      /* If we have no ESSID, mark it as bad! */
      if (strlen(hshake[a].essid)==0) hshake[a].good=0;
   }
   /* Drop bad handshakes */
   for (a=0;a<hshake_c;a++)
   {
      if (hshake[a].good>0) 
      {
         memcpy(&hshake2[hshake2_c],&hshake[a],sizeof(handshake_t));
         hshake2_c++;
      }
   }
}


void print_hshakes()
{
   int a,b,cnt=0;
   char essid_l[36];

   printf("\n");
   printf("ID \tESSID            AP_MAC \t\tSP_MAC \t\t\tProt \tQuality\n");
   printf("==========================================================================================\n");
   for (a=0;a<hshake2_c;a++) if (hshake2[a].good>0)
   {
      for (b=0;b<16;b++) essid_l[b]=' ';
      essid_l[b]=0;
      memcpy(essid_l,hshake2[a].essid,strlen(hshake2[a].essid));
      essid_l[16]=0;

      if (hshake2[a].keyver==2)
      {
         printf("%d \t%s %02x:%02x:%02x:%02x:%02x:%02x \t%02x:%02x:%02x:%02x:%02x:%02x \tWPA%d \t",
               a, essid_l,hshake2[a].amac[0]&255,hshake2[a].amac[1]&255,hshake2[a].amac[2]&255,hshake2[a].amac[3]&255,hshake2[a].amac[4]&255,hshake2[a].amac[5]&255,
               hshake2[a].smac[0]&255,hshake2[a].smac[1]&255,hshake2[a].smac[2]&255,hshake2[a].smac[3]&255,hshake2[a].smac[4]&255,hshake2[a].smac[5]&255,
               hshake2[a].keyver
               );
         cnt++;
      }
      else 
      {
         printf("%d \t%s %02x:%02x:%02x:%02x:%02x:%02x \t%02x:%02x:%02x:%02x:%02x:%02x \tWPA \t",
               a, essid_l,hshake2[a].amac[0]&255,hshake2[a].amac[1]&255,hshake2[a].amac[2]&255,hshake2[a].amac[3]&255,hshake2[a].amac[4]&255,hshake2[a].amac[5]&255,
               hshake2[a].smac[0]&255,hshake2[a].smac[1]&255,hshake2[a].smac[2]&255,hshake2[a].smac[3]&255,hshake2[a].smac[4]&255,hshake2[a].smac[5]&255
               );
         cnt++;
      }
      if (hshake2[a].good==0) printf("Bad\n");
      if (hshake2[a].good==1) printf("Workable\n");
      if (hshake2[a].good==2) printf("Good\n");
      if (hshake2[a].good==3) printf("Excellent\n");
   }
   printf("\n");
   if (cnt==0) 
   {
      printf("No crackable handshakes found!\n");
      exit(1);
   }
   else
   {
      printf("%d crackable handshakes found!\n\n",cnt);
   }


   //for (a=0;a<aps_c;a++) printf("%d: %s\n",a,aps[a].essid);
}





void print_hshakes_csv()
{
   int a,b,cnt=0;
   char essid_l[36];

   for (a=0;a<hshake2_c;a++) if (hshake2[a].good>0)
   {
      for (b=0;b<16;b++) essid_l[b]=' ';
      essid_l[b]=0;
      memcpy(essid_l,hshake2[a].essid,strlen(hshake2[a].essid));
      essid_l[16]=0;

      if (hshake2[a].keyver==2)
      {
         printf("%d,%s,%02x:%02x:%02x:%02x:%02x:%02x,%02x:%02x:%02x:%02x:%02x:%02x,WPA%d,",
               a, essid_l,hshake2[a].amac[0]&255,hshake2[a].amac[1]&255,hshake2[a].amac[2]&255,hshake2[a].amac[3]&255,hshake2[a].amac[4]&255,hshake2[a].amac[5]&255,
               hshake2[a].smac[0]&255,hshake2[a].smac[1]&255,hshake2[a].smac[2]&255,hshake2[a].smac[3]&255,hshake2[a].smac[4]&255,hshake2[a].smac[5]&255,
               hshake2[a].keyver
               );
         cnt++;
      }
      else 
      {
         printf("%d,%s,%02x:%02x:%02x:%02x:%02x:%02x,%02x:%02x:%02x:%02x:%02x:%02x,WPA,",
               a, essid_l,hshake2[a].amac[0]&255,hshake2[a].amac[1]&255,hshake2[a].amac[2]&255,hshake2[a].amac[3]&255,hshake2[a].amac[4]&255,hshake2[a].amac[5]&255,
               hshake2[a].smac[0]&255,hshake2[a].smac[1]&255,hshake2[a].smac[2]&255,hshake2[a].smac[3]&255,hshake2[a].smac[4]&255,hshake2[a].smac[5]&255
               );
         cnt++;
      }
      if (hshake2[a].good==0) printf("Bad,");
      if (hshake2[a].good==1) printf("Workable,");
      if (hshake2[a].good==2) printf("Good,");
      if (hshake2[a].good==3) printf("Excellent,");
   }
   printf("\n");
   if (cnt==0) 
   {
      exit(1);
   }
}

/**
 * Prints the important cap file info to screen and/or file.
 * @param hs - the important handshake info.
 * @param name - the name of the file we will write to.
 * @param screen - 1 we print to screen, 0 we do not.
 */
void print_info(handshake_t hs,char* filename,int screen)
{
/*
 *
 *
    unsigned char amac[6];		 AP MAC
    unsigned char smac[6];	         Supplicant MAC
    unsigned char anonce[32];		 AP Nonce
    unsigned char snonce[32];		 Supplicant nonce
    unsigned char finalkey[16];		 Final MIC key
    unsigned char eapol[256];		 EAPOL packet data
    int eapol_size;			 EAPOL data size
    int keyver;				 1 for WPA, 2 for WPA2
    char steps[4];			 handshake state
    unsigned char pmkid[16];		 PMKID, not used?!?
    unsigned char essid[36];		 ESSID of the AP
    int replaycount;			 replay counter
    int grade;				 Not used ?!?
    int good;				 handshake quality
 *
 */

   //loop counter
   int i;
   if(screen)
   {
      printf("smac:\t\t");
      for(i=0;i<6;i++)
      {
         printf("%02x",hs.smac[i]&255);
      }
      printf("\n");
      printf("amac:\t\t");
      for(i=0;i<6;i++)
      {
         printf("%02x",hs.amac[i]&255);
      }
      printf("\n");
      printf("keyver (WPA or WPA2):\tWPA%d\n",hs.keyver);
      printf("anonce:\t\t");
      for(i=0;i<32;i++)
      {
         printf("%02x",hs.anonce[i]&255);
      }

      printf("\n");

      printf("snonce:\t\t");
      for(i=0;i<32;i++)
      {
         printf("%02x",hs.snonce[i]&255);
      }

      printf("\n");
      printf("eapol_size:\t%d",hs.eapol_size);
      printf("\n");
      printf("eapol:\t\t");
      //there is no reason to print all 256
      for(i=0;i<hs.eapol_size;i++)
      {
         printf("%02x",hs.eapol[i]&255);
      }
      printf("\nmic:\t\t");
      for(i=0;i<16;i++)
      {
         printf("%02x",hs.finalkey[i]&255);
      }
      printf("\n");
      printf("ssid:\t\t");
      printf("%s\n",hs.essid);
      printf("ssidLength:\t%d\n",strnlen(hs.essid,32));
   }
   if(filename)
   {
      /**
       * file pointer for writing a new cap.txt file
       */
      FILE *fp;
      if (fp = fopen(filename, "w"))
      {
         fprintf(fp,"smac:");

         for(i=0;i<6;i++)
         {
            fprintf(fp,"%02x",hs.smac[i]&255);
         }
         fprintf(fp,"\n");
         fprintf(fp,"amac:");
         for(i=0;i<6;i++)
         {
            fprintf(fp,"%02x",hs.amac[i]&255);
         }
         fprintf(fp,"\n");
         fprintf(fp,"snounce:");
         for(i=0;i<32;i++)
         {
            fprintf(fp,"%02x",hs.snonce[i]&255);
         }
         fprintf(fp,"\n");
         fprintf(fp,"anonuce:");
         for(i=0;i<32;i++)
         {
            fprintf(fp,"%02x",hs.anonce[i]&255);
         }

         fprintf(fp,"\n");

         fprintf(fp,"keyver:%d",hs.keyver);
         fprintf(fp,"\n");
         fprintf(fp,"keymic:");
         for(i=0;i<16;i++)
         {
            fprintf(fp,"%02x",hs.finalkey[i]&255);
         }
         fprintf(fp,"\n");
         fprintf(fp,"eapol_size:%d",hs.eapol_size);
         fprintf(fp,"\n");
         fprintf(fp,"eapol:");
         for(i=0;i<hs.eapol_size;i++)
         {
            fprintf(fp,"%02x",hs.eapol[i]&255);
         }
         fprintf(fp,"\n");
         fprintf(fp,"ssidLength:%d",strnlen(hs.essid,32));
         fprintf(fp,"\n");
         fprintf(fp,"ssid:");
         fprintf(fp,"%s",hs.essid);
         fprintf(fp,"\n");
         fclose(fp);

      }
      else
      {
         printf("Error opening %s\n",filename);
      }
   }


}

void write_out_hshakes(int id,char* filename1,int printH, int printScreen)
{
   int fd,a,m1;

   bzero(hccap.essid,36);
   strcpy(hccap.essid,hshake2[id].essid);
   m1=0;

   if (memcmp(hshake2[id].amac,hshake2[id].smac,6)>0) {m1=1;}
   if (m1==1)
   {
      memcpy(hccap.mac1,hshake2[id].smac,6);
      memcpy(hccap.mac2,hshake2[id].amac,6);
   }
   else
   {
      memcpy(hccap.mac1,hshake2[id].amac,6);
      memcpy(hccap.mac2,hshake2[id].smac,6);
   }

   m1=0;
   if (memcmp(hshake2[id].anonce,hshake2[id].snonce,32)>0) {m1=1;}
   if (m1==1)
   {
      memcpy(hccap.nonce1,hshake2[id].snonce,32);
      memcpy(hccap.nonce2,hshake2[id].anonce,32);
   }
   else
   {
      memcpy(hccap.nonce1,hshake2[id].anonce,32);
      memcpy(hccap.nonce2,hshake2[id].snonce,32);
   }
   memcpy(hccap.eapol,hshake2[id].eapol,256);
   hccap.eapol_size = hshake2[id].eapol_size;
   hccap.keyver = hshake2[id].keyver;
   memcpy(hccap.keymic,hshake2[id].finalkey,16);

   if(printH)
   {
      int c;
      printf("\n");
      //fflush(stdout);
      
     while ((c = getchar()) != '\n' && c != EOF);
     printf("Enter filename (must be less than 20 chars) for HCCAP\n");
      char filename[21];
      fgets(filename,20, stdin);
      filename[strnlen(filename,20)-1] = '\0';
      fd=open(filename,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
      if (fd<1)
      {
         printf("Cannot open hccap file for writing: %s\n",filename);
         //exit(1);
      }
      write(fd,&hccap,sizeof(hccap));
      close(fd);
      printf("Output hccap written to: %s\n\n",filename);
   }
   else
   {
      printf("hccap NOT written.\n");
   }
   print_info(hshake2[id],filename1,printScreen);
}


int get_best_hshake()
{
   int a;

   for (a=0;a<hshake2_c;a++) if (hshake2[a].good==3) return a;
   for (a=0;a<hshake2_c;a++) if (hshake2[a].good==2) return a;
   for (a=0;a<hshake2_c;a++) if (hshake2[a].good==1) return a;
   printf("Unable to find good handshake.\n");
   return -1;
}


/*this runs stuff */
int main(int argc,char **argv)
{

   int selected;
   //whether or not to print info to screen, 1 - yes, 0 - no.
   int PRINT_TO_SCREEN;  
   if (argc<4)
   {
      usage(argv[0]);
      exit(0);
   }
   PRINT_TO_SCREEN=atoi(argv[3]);
   printf("Reading %s\nWritting to %s\n",argv[1],argv[2]);
   printf("Output HCAP file? 1- yes, 0 - no\n");
   int OUTPUT_HCCAP_FLAG;
   OUTPUT_HCCAP_FLAG=getchar();

   //scanf("%d ",&OUTPUT_HCCAP_FLAG);

   if (argv[4])
   {
      if (strcmp(argv[4],"-b")==0) incorrect=1;
      if (strcmp(argv[4],"-g")==0) getbest=1;
      if (strcmp(argv[4],"-c")==0) csv=1;
   }
   if (argv[5])
   {
      if (strcmp(argv[5],"-b")==0) incorrect=1;
      if (strcmp(argv[5],"-g")==0) getbest=1;
      if (strcmp(argv[5],"-c")==0) csv=1;
   }
   if (argv[6])
   {
      if (strcmp(argv[6],"-b")==0) incorrect=1;
      if (strcmp(argv[6],"-g")==0) getbest=1;
      if (strcmp(argv[6],"-c")==0) csv=1;
   }

   parse_pcap(argv[1]);
   rate_hshakes();

   if (csv==1)
   {
      print_hshakes_csv();
      exit(1);
   }
   else if (getbest==1)
   {
      print_hshakes();
      selected=get_best_hshake();
      printf("Getting best handshake\n");
      if (selected==-1) 
      {
         printf("Quitting!\n");
         exit(1);
      }
      //printf("Chosen handshake #%d\n",selected);
   }
   else
   {
      while (selected<0)
      {
         print_hshakes();
         printf("Getting best handshake\n");
         //printf("Enter an ID to include into the output hcap file: ");
         //fgets(buf,255,stdin);
         selected=get_best_hshake();
         //selected=atoi(buf);
         if (hshake2[selected].good<1) 
         {
            printf("Bad ID, try again!\n");
            selected=-1;
         }
      }
   }

   if (hshake2[selected].good>0)
   {
      //write_out_hshakes(int id,char* filename1, int printH, int printScreen)
      write_out_hshakes(selected,argv[2],OUTPUT_HCCAP_FLAG,PRINT_TO_SCREEN);
   }
   else
   {
      printf("No good .cap file found\n");
   }
   return 0;
}
