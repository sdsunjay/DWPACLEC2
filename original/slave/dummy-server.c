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
#define SLAVEPORT 3030
#define SLAVEADDR "0.0.0.0"

int create_hdsk(wpa_hdsk* phdsk)
{
  static char loaded = 0;
  FILE* fp = NULL;

  if (loaded)
    return 0;

  fp = fopen("../test/cap-02-anonce","rb");
  if (fread(phdsk->anonce, 32, 1, fp) != 1)
    return -1;
  else
    fclose(fp);
  fp = fopen("../test/cap-02-snonce","rb");
  if (fread(phdsk->snonce, 32, 1, fp) != 1)
    return -1;
  else
    fclose(fp);

  memcpy(phdsk->smac, "\xd8\x5d\x4c\x8c\xb2\x41", 6);
  memcpy(phdsk->amac, "\x98\x0c\x82\x45\xe0\xc5", 6);
  phdsk->keyver = 2;
  phdsk->eapol_size = 121;
  fp = fopen("../test/cap-02-eapol","rb");
  if (fread(phdsk->eapol, 121, 1, fp) != 1)
    return -1;
  else
    fclose(fp);
  memcpy(phdsk->keymic, &(phdsk->eapol[81]), 16);

  loaded = 1;
  return 0;
}

int main(void)
{
  int sd, rc;
  //int totalcnt = 0;
  char buffer[BUFSIZE];
  struct sockaddr_in slave_addr;
  wpa_hdsk hdsk;

  /* connect to the slave  */
  memset(&slave_addr, 0, sizeof(struct sockaddr_in));
  slave_addr.sin_family = AF_INET;
  inet_pton(AF_INET, SLAVEADDR, &(slave_addr.sin_addr.s_addr));
  slave_addr.sin_port = htons(SLAVEPORT);
  if((sd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("create socket failed\n");
    exit(-1);
  }
  if(connect(sd, (struct sockaddr*)&slave_addr, sizeof(slave_addr)) < 0)
  {
    printf("connect to slave failed");
    close(sd);
    exit(-1);
  }

  /* communicate the client */
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
      char essid[] = ESSID;
      if ((rc = create_hdsk(&hdsk)) != 0)
      {
        perror("create_hdsk() error");
        close(sd);
        exit(-1);
      }
      memcpy(buffer, "00070000", 8);
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
        printf("successfully sends data to client\n");
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
      buffer[(int)ch] = 0;
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

