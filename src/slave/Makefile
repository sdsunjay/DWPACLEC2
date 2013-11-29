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
include /media/disk/cudarun/NVIDIA_GPU_Computing_SDK/C/common/common.mk  
#include /opt/cuda

#../../common/common.mk

# Extra libs (Should be specified AFTER common.mk)
LIB += -lcrypto -lpthread -L /media/disk/cudarun/NVIDIA_GPU_Computing_SDK/shared/lib -L /media/disk/cudarun/NVIDIA_GPU_Computing_SDK/shared/lib64 -L /opt/cuda/lib64

#LIB += -lcrypto -lpthread -L /opt/cuda/lib
#LIB += -lcrypto -lpthread -L /media/disk/cudarun/NVIDIA_GPU_Computing_SDK/shared/lib
#LIB += -lcrypto -lpthread  -L /opt/cuda
#/opt/cuda/lib /opt/cuda/lib64 /media/disk/cudarun/NVIDIA_GPU_Computing_SDK/shared/lib
# Rules for WPA Crack
all: $(TARGET) 
#dummy-server dummy-master

#dummy-server: test.c
#	gcc ${USER_CFLAGS} $^ -o $@

#dummy-master: test1.c
#	gcc ${USER_CFLAGS} $^ -o $@

user_clean: clean
	rm -rf *~ wpa-crack-s ../../bin

.PHONY: all clean user_clean
