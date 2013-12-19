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
ifeq ($(TEST),arch)
   include SDK/arch_common.mk  
   #THIS WORKS ONLY FOR ARCH
   LIB += -lcrypto -lpthread -L SDK/I_AM_LIB -L /opt/cuda/lib64
else
   include SDK/common.mk  
   LIB += -lcrypto -lpthread -L SDK/I_AM_LIB -L /usr/local/cuda/lib64
endif
# Extra libs (Should be specified AFTER common.mk)
#THIS OWRKS FOR OTHER LINUS OS's


#include /home/calpoly/DWPACLEC2/src/slave/findcudalib.mk
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

.PHONY: all clean user_clean print_vars

print_vars:
	echo $(TEST)
