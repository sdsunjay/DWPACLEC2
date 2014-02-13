
#ifndef _CPU_CRACK_H_
#define _CPU_CRACK_H_

#include "common.h"

//extern "C"
pwd_range fetch_pwd(char type, const unsigned long* first, const unsigned long* last,int num);

//extern "C"
void* crack_cpu_thread(void *arg);

#endif

