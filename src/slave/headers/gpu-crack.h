#ifndef _GPU_CRACK_H_
#define _GPU_CRACK_H_

#include <stdint.h>

// Number of threads per GPU core
#define THREADS_PER_BLOCK 64

// SHA_CTX struct for the device
typedef struct {
    uint32_t h0 , h1 , h2 , h3 , h4;
} SHA_DEV_CTX;

// Input to the GPU kernel
typedef struct {
    SHA_DEV_CTX ctx_ipad;    // Precalculated IKeyPad hash
    SHA_DEV_CTX ctx_opad;    // Precalculated OKeyPad hash
    SHA_DEV_CTX e1;          // First round Hash for PMK1
    SHA_DEV_CTX e2;          // First round Hash for PMK2 
} kernel_input_buffer;

// Output of the GPU kernel
typedef struct {
    SHA_DEV_CTX pmk1;        // PMK first 20 bytes
    SHA_DEV_CTX pmk2;        // PMK last 12 bytes
} kernel_output_buffer;

// To copy between device and host CTXs
#define COPY_DEVCTX( dst , src )      \
{                                     \
    dst.h0 = src.h0; dst.h1 = src.h1; \
    dst.h2 = src.h2; dst.h3 = src.h3; \
    dst.h4 = src.h4;                  \
}

// To copy hash bytes in big endian format (same macro used in Pyrit)
#define GET_BE( n , b , i )                      \
{                                                \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )      \
        | ( (uint32_t) (b)[(i) + 1] << 16 )      \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )      \
        | ( (uint32_t) (b)[(i) + 3]       );     \
}

#define PUT_BE( n , b , i )                       \
{                                                 \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 ); \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 ); \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 ); \
    (b)[(i) + 3] = (unsigned char) ( (n)       ); \
}

// Returns the number of GPUs (possibly 0)
//extern "C"
int num_of_gpus();

// GPU Crack Host thread
extern "C"
void* crack_gpu_thread ( void *arg );

// Function that triggers the GPU Kernel (Device Thread)
extern "C"
void invoke_gpu_kernel ( int blocksPerGrid , int threadsPerBlock ,kernel_input_buffer *inbuffer , kernel_output_buffer *outbuffer , int max_num );

#endif  // _GPU_CRACK_H_
