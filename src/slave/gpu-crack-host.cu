
// Endianess, init values, speed report, finish report, spin lock on GPUs????, error checking
// Optimization - Verifying MIC some parts are not required or can be done beforehand
// TODOs
// Check speed by not reporting finish, turn off cpus and see if gpu working, etc.
// Set the THREADS_PER_BLOCK and BATCH_SIZE accordingly for different GPUs
// Output total time to crack
// GPU asynch calls what to do??

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <sys/time.h>

#include <openssl/hmac.h>
#include <openssl/sha.h>

// Helper functions common to CUDA SDK samples
#include "sdkHelper.h"
#include "shrQATest.h"
#include "shrUtils.h"

#include "cpu-crack.h"
#include "gpu-crack.h"

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

// These are CUDA Helper functions

// This will output the proper CUDA error strings in the event that a CUDA host call returns an error
#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

inline void __checkCudaErrors(cudaError err, const char *file, const int line )
{
    if(cudaSuccess != err)
    {
        fprintf(stderr, "%s(%i) : CUDA Runtime API error %d: %s.\n",file, line, (int)err, cudaGetErrorString( err ) );
        exit(-1);        
    }
}

// This will output the proper error string when calling cudaGetLastError
#define getLastCudaError(msg)      __getLastCudaError (msg, __FILE__, __LINE__)

inline void __getLastCudaError(const char *errorMessage, const char *file, const int line )
{
    cudaError_t err = cudaGetLastError();
    if (cudaSuccess != err)
    {
        fprintf(stderr, "%s(%i) : getLastCudaError() CUDA error : %s : (%d) %s.\n",
        file, line, errorMessage, (int)err, cudaGetErrorString( err ) );
        exit(-1);
    }
}

// End of CUDA Helper Functions

// Returns the number of GPUs (possibly 0)
int 
num_of_gpus() {
    int deviceCount;
    checkCudaErrors ( cudaGetDeviceCount ( &deviceCount ) );
    return deviceCount;
}

// Precompute the IKeypad, OKeypad and 1st Round hashes
inline 
void 
precompute ( const char *key , const char *essid_pre , kernel_input_buffer *gpu_input ) {

    // Key length
    int keyLength = strlen ( key );
    // ESSID length
    int essidLength = strlen ( essid_pre );
    
    // ESSID (extra 4 bytes for the '\1' and '\2' during calculation)
    uchar essid[32 + 4];
    memset ( essid , 0 , sizeof ( essid ) );
    memcpy ( essid , essid_pre , essidLength );
    
    // 64 byte buffer used in SHA-1
    uchar buffer[64];
    
    // Loop variables
    int i;
    
    // Current hash state
    SHA_CTX ctx_pad;
    
    // Calculate the Keypad initial hashes
    // IKeypad
    memcpy ( buffer , key , keyLength );
    memset ( buffer + keyLength , 0 , sizeof ( buffer ) - keyLength );
    for ( i = 0 ; i < sizeof ( buffer ) / sizeof ( uint32_t ) ; ++i )
        ( (uint32_t*) buffer )[i] ^= 0x36363636;
    SHA1_Init ( &ctx_pad );
    SHA1_Update ( &ctx_pad , buffer , sizeof ( buffer ) );
    COPY_DEVCTX( gpu_input -> ctx_ipad , ctx_pad );
    
    // OKeypad
    for ( i = 0 ; i < sizeof ( buffer ) / sizeof ( uint32_t ) ; ++i )
        ( (uint32_t*) buffer )[i] ^= 0x6A6A6A6A;
    SHA1_Init ( &ctx_pad );
    SHA1_Update ( &ctx_pad , buffer , sizeof ( buffer ) );
    COPY_DEVCTX( gpu_input -> ctx_opad , ctx_pad );
    
    // 1st Round hashes
    uchar temp[20];
    // ESSID '\1'
    essid [ essidLength + 4 - 1 ] = '\1';
    HMAC ( EVP_sha1() , key , keyLength , essid , essidLength + 4 , temp , NULL );
    GET_BE( gpu_input -> e1.h0 , temp , 0 );
    GET_BE( gpu_input -> e1.h1 , temp , 4 );
    GET_BE( gpu_input -> e1.h2 , temp , 8 );
    GET_BE( gpu_input -> e1.h3 , temp , 12 );
    GET_BE( gpu_input -> e1.h4 , temp , 16 );
    
    // ESSID '\2'
    essid [ essidLength + 4 - 1 ] = '\2';
    HMAC ( EVP_sha1() , key , keyLength , essid , essidLength + 4 , temp , NULL );
    GET_BE( gpu_input -> e2.h0 , temp , 0 );
    GET_BE( gpu_input -> e2.h1 , temp , 4 );
    GET_BE( gpu_input -> e2.h2 , temp , 8 );
    GET_BE( gpu_input -> e2.h3 , temp , 12 );
    GET_BE( gpu_input -> e2.h4 , temp , 16 );
}

int areWeDone()
{

}
// This is where the information from the .cap file is used
// Check if the key was found, by verifying the MIC
inline
int 
is_key_found ( const kernel_output_buffer *gpu_output , const wpa_hdsk *phdsk ) {
    
    // PKE and PTK used in MIC calculation
    uchar pke[100];
    uchar ptk[80];
    uchar mic[20];
    
    // Extract the 32 byte PMK from the gpu output buffer
    uchar pmk[32];
    PUT_BE( gpu_output -> pmk1.h0 , pmk , 0 );
    PUT_BE( gpu_output -> pmk1.h1 , pmk , 4 );
    PUT_BE( gpu_output -> pmk1.h2 , pmk , 8 );
    PUT_BE( gpu_output -> pmk1.h3 , pmk , 12 );
    PUT_BE( gpu_output -> pmk1.h4 , pmk , 16 );
    PUT_BE( gpu_output -> pmk2.h0 , pmk , 20 );
    PUT_BE( gpu_output -> pmk2.h1 , pmk , 24 );
    PUT_BE( gpu_output -> pmk2.h2 , pmk , 28 );

    // Loop Variable
    int i = 0;

    // Construct the key expansion buffer
    memcpy ( pke , "Pairwise key expansion" , 23 );
    // Add the MACs
    if ( memcmp ( phdsk -> smac , phdsk -> amac , 6 ) < 0 ) {
        memcpy ( pke + 23 , phdsk -> smac , 6 );
        memcpy ( pke + 29 , phdsk -> amac , 6 );
    }
    else {
        memcpy ( pke + 23 , phdsk -> amac , 6 );
        memcpy ( pke + 29 , phdsk -> smac , 6 );
    }
    // Add the Nonces
    if ( memcmp ( phdsk -> snonce , phdsk -> anonce , 32 ) < 0 ) {
        memcpy ( pke + 35 , phdsk -> snonce , 32 );
        memcpy ( pke + 67 , phdsk -> anonce , 32 );
    }
    else {
        memcpy ( pke + 35 , phdsk -> anonce , 32 );
        memcpy ( pke + 67 , phdsk -> snonce , 32 );
    }

    // Calculate the PTK
    for ( i = 0 ; i < 4 ; i++ ) {
        pke[99] = i;
        HMAC ( EVP_sha1() , pmk , 32 , pke , 100 , ptk + i * 20 , NULL );
    }

    // Calculate the MIC
    if ( phdsk -> keyver == 1 )
        HMAC ( EVP_md5() , ptk , 16 , phdsk -> eapol , phdsk -> eapol_size , mic , NULL );
    else
        HMAC ( EVP_sha1() , ptk , 16 , phdsk -> eapol , phdsk -> eapol_size , mic , NULL );

    // Check if MIC agrees
    if ( memcmp ( mic , phdsk -> keymic , 16 ) == 0 )
        return 1;

    // Return 0 if not found, 1 if found
    return 0;
}

// GPU Crack Host thread
void* 
crack_gpu_thread ( void *arg ) {

    // ESSID
    char essid[32];
    
    // Password (key) in string format
    char key[128];
    memset ( key , 0 , sizeof ( key ) );

    // Params passed in arguments
    ck_td_struct* ck_td_arg = (ck_td_struct*)arg;
    wpa_hdsk* phdsk = ck_td_arg->phdsk;
    int cpu_num = ck_td_arg->cpu_core_id;
    int gpu_num = ck_td_arg->gpu_core_id;
    if ( gpu_num == 0 ) // GPU not used
        return NULL;
    float* calc_speed = ck_td_arg->calc_speed;
    char* final_key = ck_td_arg->final_key;
    char* final_key_flag = ck_td_arg->final_key_flag;
    memset(essid, 0, sizeof(essid));
    memcpy(essid, ck_td_arg->essid, 32);
  
    // Normal Loop Variable
    int i = 0;
    
    // Loop variable for the password range
    unsigned long cur_key_digit = 0;

    // For calculating the PMK/sec speed
    struct timeval tprev;
    struct timeval tnow;

    // Password range for each GPU
    pwd_range *range;
    range = (pwd_range*) malloc ( sizeof ( pwd_range ) * gpu_num );
    
    // Number of working GPUs
    int gpu_working = 0;
    // Number of PMKs being computed in one dispatch
    int num_keys = 0;
    
    // Input Buffer of the GPUs
    kernel_input_buffer *gpu_input = (kernel_input_buffer*) malloc ( sizeof ( kernel_input_buffer ) * PWD_BATCH_SIZE_GPU * gpu_num );
    // Output Buffer of the GPUs
    kernel_output_buffer *gpu_output = (kernel_output_buffer*) malloc ( sizeof ( kernel_output_buffer ) * PWD_BATCH_SIZE_GPU * gpu_num );
    
    // Buffers in the GPU Memory space
    kernel_input_buffer **device_input;
    kernel_output_buffer **device_output;
    device_input = (kernel_input_buffer**) malloc ( sizeof ( kernel_input_buffer* ) * gpu_num );
    device_output = (kernel_output_buffer**) malloc ( sizeof ( kernel_output_buffer* ) * gpu_num );
    
    // Allocate device memory beforehand itself, we can reuse it again and again
    int devMemSize;
    for ( i = 0 ; i < gpu_num ; ++i ) {
        checkCudaErrors ( cudaSetDevice ( i ) );
        devMemSize = sizeof ( kernel_input_buffer ) * PWD_BATCH_SIZE_GPU;
        checkCudaErrors ( cudaMalloc ( (void**) &device_input[i] , devMemSize ) );
        devMemSize = sizeof ( kernel_output_buffer ) * PWD_BATCH_SIZE_GPU;
        checkCudaErrors ( cudaMalloc ( (void**) &device_output[i] , devMemSize ) );
    }
    
    // Repeatedly get password ranges to dispatch to the GPUs
    while ( 1 ) {

        // Get the password range for each gpu
        gpu_working = 0;
        for ( i = 0 ; i < gpu_num ; ++i ) {
            range[i] = fetch_pwd ( 'g' , NULL , NULL );
            if ( range[i].start == -1 )
                break;
            ++gpu_working;
        }
        
        // Check if password range is over
        if ( gpu_working <= 0 ) {

            // Tell main thread we are terminating
            calc_speed[ cpu_num ] = -1;
            
            // Free resources
            free ( range );
            free ( gpu_input );
            free ( gpu_output );
            for ( i = 0 ; i < gpu_num ; ++i ) {
                checkCudaErrors ( cudaSetDevice ( i ) );
                checkCudaErrors ( cudaFree ( (void*) device_input[i] ) );
                checkCudaErrors ( cudaFree ( (void*) device_output[i] ) );
            }
            free ( device_input );
            free ( device_output );
            
            return NULL;
        }
        
        // Start time of the computation (including memory transfers Host mem <==> Device mem)
        gettimeofday ( &tprev , NULL );
        
        // Precompute the iKeypads, oKeypads and 1st Round Hashes
        num_keys = 0;
        for ( i = 0 ; i < gpu_working ; ++i ) {
            for ( cur_key_digit = range[i].start ; cur_key_digit <= range[i].end ; ++cur_key_digit ) {
                
                // Convert the key from digit to string
                sprintf ( key , "%08lu" , cur_key_digit );
               
	       //printf("GPU Password: %s\n",key);	
                // Calculate the Kernel input buffer values for this key
                precompute ( key , essid , & gpu_input[ ( i * PWD_BATCH_SIZE_GPU ) + ( cur_key_digit - range[i].start ) ] );
                
                // Count the total number of keys
                ++num_keys;
            }
        }
        
        // Now let the GPUs do the work
        /*
         * We need to be careful here. The calls to CUDA runtime API are asynchronous.
         * The CUDA manual doesn't really explain it properly :P
         * Here's the deal: (http://forums.nvidia.com/index.php?showtopic=175073)
         * 1. Copies from Host to Device are asynchchronous if data size <= 64 kb, otherwise they are synchronous
         * 2. Kernel calls are ALWAYS asynchronous
         * 3. Copies from Device to Host are ALWAYS synchronous (otherwise we cannot use the output buffer after the call!!)
         * 
         * Here the data size is 10000 * sizeof ( input buffer ) == 10000 * 80 = 800000 bytes = approx 800 kb
         * So, the call to copy from HtoD is Synch. Can't do anything about that :P
         * 
         * But, the copying of data from Device to Host (Synch) should be in a SEPARATE FOR LOOP. Otherwise, we cannot
         * dispatch work to the second GPU until the first GPU finishes computation :P
         */
        for ( i = 0 ; i < gpu_working ; ++i ) 
	{    
            // Set the GPU Device we are currently dispatching work to crack
            checkCudaErrors ( cudaSetDevice ( i ) );

            // Copy the Input buffers from the Host to Device (GPU) Memory
            devMemSize = sizeof ( kernel_input_buffer ) * PWD_BATCH_SIZE_GPU;
            checkCudaErrors ( cudaMemcpy ( device_input[i] , gpu_input + (i * PWD_BATCH_SIZE_GPU) , devMemSize , cudaMemcpyHostToDevice ) );
            
            // Calculate the PMKs using GPU
            int max_num = range[i].end - range[i].start + 1;
            int blocksPerGrid = ( max_num + THREADS_PER_BLOCK - 1 ) / THREADS_PER_BLOCK;
            invoke_gpu_kernel ( blocksPerGrid , THREADS_PER_BLOCK , device_input[i] , device_output[i] , max_num );
            getLastCudaError ( "Kernel launch failure!!" );
        }

        // Copy the Output buffers to the Host from Device (GPU) Memory in SEPARATE FOR LOOP
        for ( i = 0 ; i < gpu_working ; ++i ) {
            
            // Set the GPU Device we are currently dispatching work to crack
            checkCudaErrors ( cudaSetDevice ( i ) );
            
            // Copy the Output buffers to the Host from Device (GPU) Memory
            devMemSize = sizeof ( kernel_output_buffer ) * PWD_BATCH_SIZE_GPU;
            checkCudaErrors ( cudaMemcpy ( gpu_output + (i * PWD_BATCH_SIZE_GPU) , device_output[i] , devMemSize , cudaMemcpyDeviceToHost ) );
        }
        
        // Check if the key (password) was found
        for ( i = 0 ; i < gpu_working ; ++i ) {
            for ( cur_key_digit = range[i].start ; cur_key_digit <= range[i].end ; ++cur_key_digit ) {
                
                // Verify the MIC
                if ( is_key_found ( & gpu_output[ ( i * PWD_BATCH_SIZE_GPU ) + ( cur_key_digit - range[i].start ) ] , phdsk ) ) {

                    // !!!!! We found the key !!!!!
                    
                    // End time of computation (including memory transfers Host mem <==> Device mem)
                    gettimeofday ( &tnow , NULL );
        
                    // Report speed to main thread
                    calc_speed[ cpu_num ] = (float) num_keys / ( tnow.tv_sec - tprev.tv_sec + ( tnow.tv_usec - tprev.tv_usec ) * 0.000001F );
                    
                    // Sleep a little so that the main thread will read the speed
                    sleep ( 1 );
                    
                    // Convert the key from digit to string
                    sprintf ( key , "%08lu" , cur_key_digit );
                    
                    // Report the key to the main thread
                    memcpy ( final_key , key , strlen ( key ) );
                    *final_key_flag = 1;

                    // Tell main thread we are terminating
                    calc_speed[ cpu_num ] = -1;
                    
                    // Free resources
                    free ( range );
                    free ( gpu_input );
                    free ( gpu_output );
                    for ( i = 0 ; i < gpu_num ; ++i ) {
                        checkCudaErrors ( cudaSetDevice ( i ) );
                        checkCudaErrors ( cudaFree ( (void*) device_input[i] ) );
                        checkCudaErrors ( cudaFree ( (void*) device_output[i] ) );
                    }
                    free ( device_input );
                    free ( device_output );
                    
                    return NULL;
                }
            }
        }
        
        // End time of computation (including memory transfers Host mem <==> Device mem)
        gettimeofday ( &tnow , NULL );
        
        // Report speed to main thread
        calc_speed[ cpu_num ] = (float) num_keys / ( tnow.tv_sec - tprev.tv_sec + ( tnow.tv_usec - tprev.tv_usec ) * 0.000001F );
    }

    return NULL;
}
