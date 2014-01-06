################################################################################
#
# Build script for WPA Crack Slave and Dummy Master Client
#
################################################################################

# WPA Crack executable (slave)
EXECUTABLE	:= wpa-crack-s
# Cuda source files (compiled with cudacc)
CUFILES		:= gpu-crack-host.cu gpu-crack-device.cu
# C/C++ source files (compiled with gcc / c++)
CCFILES		:= wpa-crack-s.cpp cpu-crack.cpp 
# CUDA file Header Dependencies
CU_DEPS		:=  \
	headers/common.h    \
	headers/cpu-crack.h \
	headers/gpu-crack.h \
# C file Header Dependencies
C_DEPS		:=  \
	headers/common.h    \
	headers/cpu-crack.h \
	headers/gpu-crack.h \

# C Compiler Flags
WPA_DEBUG := no
ifeq (yes, ${WPA_DEBUG})
  USER_CFLAGS += -g
else
  USER_CFLAGS += -O3 
endif

# Do not link with CUTIL
OMIT_CUTIL_LIB := 1

# Verbose 
verbose := 1

################################################################################
# Rules and targets
#this needs to be editted
#include /root/cudarun/NVIDIA_GPU_Computing_SDK/C/common/common.mk
#INCLUDES  := -I/opt/cuda/samples/common/inc
#include /opt/cuda/samples/common/inc
#HAD TO EDIT COMMON.mk to explicitly set nvcc location
#BELOW WORKS DO NOT MODIFY
MYOS := $(shell uname --all)
TEST := $(findstring arch,$(MYOS))
#GUESS := $(shell uname --all\ | awk -v N=2 \'{echo $$N}\')
#ifeq ($(TEST),arch)
   #THIS WORKS ONLY FOR ARCH
   #include SDK/arch_common.mk  
   #LIB += -lcrypto -lpthread -L SDK/I_AM_LIB -L /usr/local/cuda/lib64 `mysql_config --cflags` `mysql_config --libs`
#else
   #THIS WORKS FOR OTHER LINUS OS's
   #include SDK/common.mk  
   #LIB += -lcrypto -lpthread -L SDK/I_AM_LIB -L /usr/local/cuda/lib64 `mysql_config --cflags` `mysql_config --libs`
#endif
ifeq ($(TEST),arch)
   include SDK/arch_common.mk  
   #THIS WORKS ONLY FOR ARCH
   LIB += -lcrypto -lpthread -L SDK/I_AM_LIB -L /opt/cuda/lib64 `mysql_config --cflags` `mysql_config --libs`
else
   include SDK/common.mk  
   #THIS WORKS FOR OTHER OS's.
   LIB += -lcrypto -lpthread -L SDK/I_AM_LIB -L /usr/local/cuda/lib64 `mysql_config --cflags` `mysql_config --libs`
endif
# Extra libs (Should be specified AFTER common.mk)

#include /home/calpoly/DWPACLEC2/src/slave/findcudalib.mk
#LIB += -lcrypto -lpthread -L /opt/cuda/lib
#LIB += -lcrypto -lpthread -L /media/disk/cudarun/NVIDIA_GPU_Computing_SDK/shared/lib
#LIB += -lcrypto -lpthread  -L /opt/cuda
#/opt/cuda/lib /opt/cuda/lib64 /media/disk/cudarun/NVIDIA_GPU_Computing_SDK/shared/lib
# Rules for WPA Crack
all: 
	$(TARGET) 
run:
	sudo ../../bin/linux/release/wpa-crack-s 7373
cuda-memcheck:
	sudo cuda-memcheck --leak-check full ../../bin/linux/release/wpa-crack-s 7373
nvprof:
	sudo nvprof -o nvprof_output --print-gpu-trace --print-api-trace  ../../bin/linux/release/wpa-crack-s 7373	
#dummy-server dummy-master

#dummy-server: test.c
#	gcc ${USER_CFLAGS} $^ -o $@

#dummy-master: test1.c
#	gcc ${USER_CFLAGS} $^ -o $@
love: 
	@echo "Not now, I'm too tired love."	
user_clean: clean
	rm -rf *~ wpa-crack-s ../../bin

.PHONY: all clean user_clean print_vars

print_vars:
	echo $(TEST)
