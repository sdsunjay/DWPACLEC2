#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "headers/common.h"
#include "headers/cpu-crack.h"
#include "headers/gpu-crack.h"
using namespace std;
int wait_connect(int* psd, char* port)
{
  int sd = -1, rc = -1, on = 1;
  struct sockaddr_in serveraddr, their_addr;
  
  // get a socket descriptor
  if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return -1;
    
  // allow socket descriptor to be reusable
  if((rc = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) < 0)
  {
    close(sd);
    return -1;
  }
  
  // bind to an address
  memset(&serveraddr, 0, sizeof(struct sockaddr_in));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(atoi(port));
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if((rc = bind(sd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))) < 0)
  {
    close(sd);
    return -1;
  }
    
  // up to 1 client can be queued
  if((rc = listen(sd, 1)) < 0)
  {
    close(sd);
    return -1;
  }

  // accept the incoming connection request
  socklen_t sin_size = sizeof(struct sockaddr_in);
  if((*psd = accept(sd, (struct sockaddr*)&their_addr, &sin_size)) < 0)
  {
    close(sd);
    return -1;
  }
  
  return 0;
}

int master_request(int sd, unsigned long* pfpwd, unsigned long* plpwd, wpa_hdsk* phdsk, char* essid)
{
  char buf[512];
  char buf2[32];
  char essid_len = 0;
  int count = 0, rc = 0;
  int tcnt = 17+sizeof(wpa_hdsk);
  
  count = write(sd, &("r"), 1);
  if(count != 1)
    return -1;

  count = 0;
  memset(buf, 0, 512);
  while (count < tcnt)
  {
    rc = read(sd, &buf[count], 512-count);
    
    if (rc <= 0)
      return -1;
    else
      count += rc;
  }
  memset(buf2, 0, 32);
  memcpy(buf2, buf, 8);
  *pfpwd = atol(buf2);
  memset(buf2, 0, 32);
  memcpy(buf2, &buf[8], 8);
  *plpwd = atol(buf2);
  memcpy(phdsk, &buf[16], sizeof(wpa_hdsk));
  essid_len = buf[16+sizeof(wpa_hdsk)];
  while (count < tcnt+essid_len)
  {
    rc = read(sd, &buf[count], 512-count);
    
    if (count <= 0)
      return -1;
    else
      count += rc;
  }
  memset(essid, 0, 32);
  memcpy(essid, &buf[17+sizeof(wpa_hdsk)], essid_len);
  
  // reset the key mic field in eapol frame to zero for later calculation
  memset(&(phdsk->eapol[81]), 0, 16);

  return 0;
}

int master_answer(int sd, char* key)
{
  unsigned char len = 0;
  int count = 0;
  
  len = strlen(key);
  
  count = write(sd, &("a"), 1);
  if(count != 1)
    return -1;
  count = write(sd, &len, 1);
  if(count != 1)
    return -1;
  count = write(sd, key, len);
  if(count != len)
    return -1;
  
  return 0;
}

void print_work(unsigned long* pfpwd, unsigned long* plpwd, wpa_hdsk* phdsk, char* essid)
{
  int i = 0;
  
  printf("----------------------------------------\n");
  printf("password range: %08lu to %08lu\n", *pfpwd, *plpwd);
  printf("essid: %s\n", essid);
  printf("s-mac: %02x", phdsk->smac[0]);
  for (i=1; i<6; ++i)
    printf(":%02x", phdsk->smac[i]);
  putchar('\n');
  printf("a-mac: %02x", phdsk->amac[0]);
  for (i=1; i<6; ++i)
    printf(":%02x", phdsk->amac[i]);
  putchar('\n');
  printf("s-nonce: ");
  for (i=0; i<32; ++i)
    printf("%02x", phdsk->snonce[i]);
  putchar('\n');
  printf("a-nonce: ");
  for (i=0; i<32; ++i)
    printf("%02x", phdsk->anonce[i]);
  putchar('\n');
  printf("key version: %u (%s)\n", phdsk->keyver, phdsk->keyver==1?"HMAC-MD5":"HMAC-SHA1-128");
  printf("key mic: ");
  for (i=0; i<16; ++i)
    printf("%02x", phdsk->keymic[i]);
  putchar('\n');
  printf("eapol frame content size: %u bytes\n", phdsk->eapol_size);
  printf("eapol frame content (with mic reset): \n");
  for (i=1; i<=phdsk->eapol_size; ++i)
    printf("%02x%c", phdsk->eapol[i-1], i%16==0?'\n':' ');
  putchar('\n');
  printf("----------------------------------------\n");
}

int main(int argc, char** argv)
{
  int cpu_num = 0;
  int cpu_working = 0;
  int gpu_num = 0;
  int gpu_working = 0;
  int sd = -1;
  int flag = -1;
  int i = 0;
  
  wpa_hdsk hdsk;
  char essid[32];
  pwd_range range;
  unsigned long first_pwd = 0;
  unsigned long last_pwd = 0;
  
  // check and parse arguments
  if (argc != 2)
  {
    printf("usage: %s <port>\n", argv[0]);
    printf("sunjay has opened port# 7373 for this\n");
    exit(0);
  }
  
  // get the number of CPU processors
  cpu_num = sysconf(_SC_NPROCESSORS_ONLN );
  printf("number of CPU processors: %d\n", cpu_num);
  
  gpu_num = num_of_gpus();
  printf("number of GPU devices: %d\n", gpu_num);
  
  // connect to the master
  printf("wait for connection from master on port %s ...\n", argv[1]);
  flag = wait_connect(&sd, argv[1]);
  printf("%s\n", flag==0?"connection established":"failed");
  if (flag)
    exit(0);
    
  // request for work from the master
  printf("requesting for work from the master ... ");
  flag = master_request(sd, &first_pwd, &last_pwd, &hdsk, essid);
  printf("%s\n", flag==0?"done":"failed");
  if (flag)
  {
    close(sd);
    exit(0);
  }
  
  // print out the received information
  print_work(&first_pwd, &last_pwd, &hdsk, essid);
  
  // prepare for CPU and GPU thread creation
  range = fetch_pwd('\0', &first_pwd, &last_pwd);
  if ( range.start != 0)
  {
    printf("error while preparing cpu/gpu thread creation\n");
    printf("Range does not start at 0\n");
    exit(0);
  }
  float* calc_speed = (float*)malloc((cpu_num+1)*sizeof(float));
  memset(calc_speed, 0, (cpu_num+1)*sizeof(float));
  char final_key[128];
  memset(final_key, 0, sizeof(final_key));
  char final_key_flag = 0;
  // Number of Host threads = Number of CPU Crack Threads + 1 GPU Managing Thread for all GPUs
  // For GPU, it is wasteful to have a separate host thread for each GPU, since the host thread *could* 
  // spin wait for the GPU to finish, wasting precious CPU cycles
  pthread_t* tid_vector = (pthread_t*)malloc((cpu_num+1)*sizeof(pthread_t));
  memset(tid_vector, 0, (cpu_num+1)*sizeof(pthread_t));
  
  // For calculating the total time taken
  struct timeval tprev;
  struct timeval tnow;

  // Start time of the computation
  gettimeofday ( &tprev , NULL );

  // create the cracking threads for CPU
  for (i=0; i<cpu_num; ++i)
  {
    ck_td_struct* arg = (ck_td_struct*)malloc(sizeof(ck_td_struct));
    arg->cpu_core_id = i;
    arg->gpu_core_id = -1;
    arg->set_affinity = 1;
    arg->essid = essid;
    arg->phdsk = &hdsk;
    arg->calc_speed = calc_speed;
    arg->final_key = final_key;
    arg->final_key_flag = &final_key_flag;

    flag = pthread_create(&tid_vector[i], NULL, crack_cpu_thread, arg);
    if (flag)
      printf("can't create thread on cpu core %d: %s\n", i, strerror(flag));
    else
      cpu_working++;
  }

  // create the cracking host thread for GPU
  // the host thread will dispatch work to all available GPU devices
  ck_td_struct* arg = (ck_td_struct*)malloc(sizeof(ck_td_struct));
  arg->cpu_core_id = cpu_num;
  arg->gpu_core_id = gpu_num; // Num of GPUs available
  arg->set_affinity = 1;
  arg->essid = essid;
  arg->phdsk = &hdsk;
  arg->calc_speed = calc_speed;
  arg->final_key = final_key;
  arg->final_key_flag = &final_key_flag;
    
  flag = pthread_create(&tid_vector[cpu_num], NULL, crack_gpu_thread, arg);
  if (flag)
  {
    printf("can't create thread for gpu: %s\n", strerror(flag));
  }
  else
  {
    printf("created thread for gpu\n");
    gpu_working++;
  }
  // monitor the runtime status
  float cpu_speed_all = 0;
  float gpu_speed_all = 0;
  printf ( "...\n" );
  while (1)
  {
    // print the calculation speed
    cpu_speed_all = 0;
    gpu_speed_all = 0;
    for (i=0; i<cpu_num; ++i)
    {
      if (calc_speed[i] == -1)
      {
        calc_speed[i] = -2;
        cpu_working--;
      }
      else if (calc_speed[i] > 0)
        cpu_speed_all += calc_speed[i];
    }

    // Speed of all GPUs
    if (calc_speed[cpu_num] == -1)
    {
      calc_speed[cpu_num] = -2;
      gpu_working--;
    }
    else if (calc_speed[cpu_num] > 0)
        gpu_speed_all += calc_speed[cpu_num];

    // delete the last output line (http://stackoverflow.com/questions/1348563)
    fputs("\033[A\033[2K",stdout);
    rewind(stdout);
    flag = ftruncate(1, 0);

    range = fetch_pwd('\0', &first_pwd, NULL);
    printf("SPD: %08.1fPMK/S [CPU(%02d):%08.1f|GPU(%02d):%08.1f|G/C:%06.1f] CUR: %08lu\n", 
      cpu_speed_all+gpu_speed_all, cpu_working, cpu_speed_all, gpu_working, gpu_speed_all, 
      gpu_speed_all/cpu_speed_all, range.start);
    
    // rest for some time (this only blocks the current thread)
    sleep(1);
    
    // check if the correct key is found
    if (final_key_flag)
    {
      printf("!!! key found !!! [%s]\n", final_key);
      
      // End time of computation
      gettimeofday ( &tnow , NULL );
      // Report total time
      float total_time = tnow.tv_sec - tprev.tv_sec + ( tnow.tv_usec - tprev.tv_usec ) * 0.000001F;
      printf ( "Time Taken: %.2f seconds, i.e. %.2f hours\n" , total_time , total_time / 3600 );
      
      printf("send back the key to the master ... ");
      flag = master_answer(sd, final_key);
      printf("%s\n", flag==0?"done":"failed");
      if (flag)
      {
          close(sd);
          free(calc_speed);
          exit(0);
      }
      else
        break;
    }
    
    // if there are no remaining cpu or gpu thread, then exit
    if (cpu_working<=0 && gpu_working<=0)
    {
      printf("no thread calculating, exit\n");
      close(sd);
      free(calc_speed);
      exit(0);
    }
  }
  
  // release resources
  close(sd);
  free(calc_speed);
  
  return 0;
}

