
#ifndef _COMMON_H_
#define _COMMON_H_

#include <pthread.h>
#include <mysql/mysql.h>

// how many passwords to feed to a CPU/GPU crack thread every time
#define PWD_BATCH_SIZE_CPU 10000
#define PWD_BATCH_SIZE_GPU 50000
//#define PWD_BATCH_SIZE_GPU 100000

//SUNJAY
//FOR DATABASE
#define hostName "localhost"
#define userId "root"
#define password "6f141H64TyPi"
#define DB_NAME "DWPA"


//FOR DATABASE
/*
class FFError
{
   public:
      std::string    Label;

      FFError( ) { Label = (char *)"Generic Error"; }
      FFError( char *message ) { Label = message; }
      ~FFError() { }
      inline const char*   GetMessage  ( void )   { return Label.c_str(); }
};
*/

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
//global db connector
//extern MYSQL* MySQLConnection = NULL;
#endif

