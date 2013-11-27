#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"
#define ESSID "AndroidAP"
#define BUFSIZE 384
#define SERVPORT 3030
#define SERVADDR "0.0.0.0"
//#define SERVADDR "54.241.241.203"

int create_hdsk(wpa_hdsk* phdsk)
{
  static char loaded = 0;
  FILE* fp = NULL;
  
  if (loaded)
    return 0;
    
  fp = fopen("../test/cap-02-anonce","rb");
  if ( fp < 0 )
    return -1;
  if (fread(phdsk->anonce, 32, 1, fp) != 1)
    return -1;
  else
    fclose(fp);
  fp = fopen("../test/cap-02-snonce","rb");
  if ( fp < 0 )
    return -1;
  if (fread(phdsk->snonce, 32, 1, fp) != 1)
    return -1;
  else
    fclose(fp);
  
  memcpy(phdsk->smac, "\xd8\x5d\x4c\x8c\xb2\x41", 6);
  memcpy(phdsk->amac, "\x98\x0c\x82\x45\xe0\xc5", 6);
  phdsk->keyver = 2;
  phdsk->eapol_size = 121;
  fp = fopen("../test/cap-02-eapol","rb");
  if ( fp < 0 )
    return -1;
  if (fread(phdsk->eapol, 121, 1, fp) != 1)
    return -1;
  else
    fclose(fp);
  memcpy(phdsk->keymic, &(phdsk->eapol[81]), 16);
  
  loaded = 1;
  return 0;
}
 
int main()
{
  int sd;
  int rc;
  wpa_hdsk hdsk;
  char essid[100];
  /*int totalcnt = 0;
   on = 1;*/
  char buffer[BUFSIZE];
  struct sockaddr_in serveraddr;
  /*
   * struct sockaddr_in their_addr;
   *
   * */
 
  /* get a socket descriptor */
  printf("I AM ACTUALLY WRITING CODE\n\n");
  if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket() error");
    exit (-1);
  }

  /* connect to the slave server */
  memset(&serveraddr, 0, sizeof(struct sockaddr_in));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(SERVPORT);
  inet_pton(AF_INET, SERVADDR, &serveraddr.sin_addr);
  if(connect(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
  {
    perror("connect() error");
    close(sd);
    exit(-1);
  }
    
  /* communicate the slave */
  while (1)
  {
    char ch = 0;
    
    rc = read(sd, &ch, 1);
    if(rc != 1)
    {
      perror("read() client command error");
      close(sd);
      exit (-1);
    }
    
    if (ch == 'r')
    {
      printf("client program has issued a request\n");
      /*The network name is defined above*/
      strcpy(essid,ESSID);
      if ((rc = create_hdsk(&hdsk)) != 0)
      {
        perror("create_hdsk() error");
        close(sd);
        exit(-1);
      }
      memcpy(buffer, "00000000", 8);
      memcpy(&(buffer[8]), "99999999", 8);
      memcpy(&(buffer[16]), &hdsk, sizeof(wpa_hdsk));
      buffer[16+sizeof(wpa_hdsk)]=(unsigned char)strlen(essid);
      memcpy(&(buffer[17+sizeof(wpa_hdsk)]), essid, strlen(essid));
      rc = write(sd, buffer, 17+sizeof(wpa_hdsk)+strlen(essid));
      if(rc != 17+sizeof(wpa_hdsk)+strlen(essid))
      {
        perror("write() error");
        close(sd);
        exit(-1);
      }
      else
        printf("successfully sent data to client\n");
    }
    else if (ch == 'a')
    {
      printf("!!! client program has found the key !!!\n");
      rc = read(sd, &ch, 1);
      if(rc != 1)
      {
        perror("read() client answer key size error");
        close(sd);
        exit (-1);
      }
      memset(buffer, 0, BUFSIZE);
      rc = read(sd, buffer, ch);
      if(rc != ch)
      {
        perror("read() client answer key content error");
        close(sd);
        exit (-1);
      }
      buffer[(int) ch] = 0;
      printf("the key is: %s\n", buffer);
      break;
    }
    else
    {
      perror("client sends unknown data");
      close(sd);
      exit(-1);
    }
  }

  /* close the connection to the slave */
  close(sd);
  return 0;
}
