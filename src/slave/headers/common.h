
#ifndef _COMMON_H_
#define _COMMON_H_

#include <pthread.h>
#include <mysql/mysql.h>

// how many passwords to feed to a CPU/GPU crack thread every time
#define PWD_BATCH_SIZE_CPU 100000
#define PWD_BATCH_SIZE_GPU 2000000
#define QUERY_BUFFER 100


//SUNJAY
//FOR DATABASE
//Now sent by master
//#define hostName "54.193.149.75"
//#define hostName "localhost"
//define userId "root"
//define password "1fYegM6Dw157"

#define userId "calpoly"
#define password "7751DW6MegYf1"
//#define password "6f141H64TyPi"
#define DB_NAME "DWPA"
#define PORT_NUMBER 3306
#define TABLE_NAME "DICT1"
#define COLUMN_NAME "WORD"
#define LONGEST_PASSWORD 128
//for SUNJAY test machine we have 
//8 cpu threads and 1 gpu thread for a total of 9 connections 
//for AWS GPU CG1 we have 16 CPU threads and 2 gpu threads
 #define NUM_DB_CONNECTIONS 20 
// WPA 4-way handshake structure
typedef struct _wpa_hdsk
{
  unsigned char smac[6];      // supplicant/station MAC
  unsigned char snonce[32];   // supplicant/station nonce
  unsigned char amac[6];      // authenticator/AP MAC
  unsigned char anonce[32];   // authenticator/AP nonce
  unsigned char keyver;       // key version
  unsigned char keymic[16];   // EAPOL frame MIC (2nd key frame)
  unsigned char eapol_size;   // EAPOL frame size (2nd key frame)
  unsigned char eapol[192];   // EAPOL frame contents (2nd key frame)
} wpa_hdsk;

// structure to pass to a CPU/GPU crack thread
typedef struct _ck_td_struct
{
  int cpu_core_id;
  int gpu_core_id;
  char set_affinity;
  char* essid;
  wpa_hdsk* phdsk;
  float* calc_speed;
  char* final_key;
  char* final_key_flag;
} ck_td_struct;

typedef struct _pwd_range
{
  unsigned long start;
  unsigned long end;
} pwd_range;
#endif

