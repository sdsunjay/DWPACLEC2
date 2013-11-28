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
/*
 * my website ip address
 * hosted on ChicagoVPS
 */
//#define SERVADDR "172.245.22.244"

#define SERVADDR "127.0.0.1"

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
  printf("I AM SUCCESSFUL\n\n");
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
}


