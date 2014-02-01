
#include "headers/gpu-crack.h"

/* 
    Optimized SHA1 function written by author of Pyrit:
    (I added some comments to his code to explain the optimizations he did)
    
    Quote from his comments:
    "This is a 'special-version' of the SHA1 round function. *ctx is the current state,
     that gets updated by *data. Notice the lack of endianess-changes here.
     This SHA1-implementation follows the more-instructions-less-space paradigm, since registers
     and (fast) memory on the device are precious, threads are not. Only the starting values
     of W[0] to W[4] are defined by parameters. We fix the rest to invariant values and leave
     the possible register allocation optimization to the compiler."
*/
__device__
void sha1_process ( const SHA_DEV_CTX *ctx , SHA_DEV_CTX *data ) {

    uint32_t temp, W[16], A, B, C, D, E;

    // W[5] contains the appended '1' bit
    // W[15] is the total length of buffer being hashed: 64 byte keypad + 20 byte prior hash
    W[ 0] = data->h0; W[ 1] = data->h1;
    W[ 2] = data->h2; W[ 3] = data->h3;
    W[ 4] = data->h4; W[ 5] = 0x80000000;
    W[ 6] = 0; W[ 7] = 0;
    W[ 8] = 0; W[ 9] = 0;
    W[10] = 0; W[11] = 0;
    W[12] = 0; W[13] = 0;
    W[14] = 0; W[15] = (64+20)*8;

    // Values from the previous round
    A = ctx->h0;
    B = ctx->h1;
    C = ctx->h2;
    D = ctx->h3;
    E = ctx->h4;

// This is a 'rotateleft by n bits' macro
#undef S
#define S(x,n) ((x << n) | (x >> (32 - n)))

// In this special SHA1, we do not need to actually extend to 80 words, since
// each word ends up being constructed from the initial 16 words.
// (Try it out on paper and see yourself)
// The 0x0F translates the index into the first 16 words.
#undef R
#define R(t)                                            \
(                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = S(temp,1) )                         \
)

// The calculation of 'temp' in wiki pseudocode
#undef P
#define P(a,b,c,d,e,x)                                  \
{                                                       \
    e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);        \
}

// Note that the ABCDE rotation is done manually here (80 rotations total)
// This is much more efficient than using a for loop and doing rotations
// using assignments in each iteration
#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

    P( A, B, C, D, E, W[0]  );
    P( E, A, B, C, D, W[1]  );
    P( D, E, A, B, C, W[2]  );
    P( C, D, E, A, B, W[3]  );
    P( B, C, D, E, A, W[4]  );
    P( A, B, C, D, E, W[5]  );
    P( E, A, B, C, D, W[6]  );
    P( D, E, A, B, C, W[7]  );
    P( C, D, E, A, B, W[8]  );
    P( B, C, D, E, A, W[9]  );
    P( A, B, C, D, E, W[10] );
    P( E, A, B, C, D, W[11] );
    P( D, E, A, B, C, W[12] );
    P( C, D, E, A, B, W[13] );
    P( B, C, D, E, A, W[14] );
    P( A, B, C, D, E, W[15] );
    P( E, A, B, C, D, R(16) );
    P( D, E, A, B, C, R(17) );
    P( C, D, E, A, B, R(18) );
    P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    P( A, B, C, D, E, R(20) );
    P( E, A, B, C, D, R(21) );
    P( D, E, A, B, C, R(22) );
    P( C, D, E, A, B, R(23) );
    P( B, C, D, E, A, R(24) );
    P( A, B, C, D, E, R(25) );
    P( E, A, B, C, D, R(26) );
    P( D, E, A, B, C, R(27) );
    P( C, D, E, A, B, R(28) );
    P( B, C, D, E, A, R(29) );
    P( A, B, C, D, E, R(30) );
    P( E, A, B, C, D, R(31) );
    P( D, E, A, B, C, R(32) );
    P( C, D, E, A, B, R(33) );
    P( B, C, D, E, A, R(34) );
    P( A, B, C, D, E, R(35) );
    P( E, A, B, C, D, R(36) );
    P( D, E, A, B, C, R(37) );
    P( C, D, E, A, B, R(38) );
    P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    P( A, B, C, D, E, R(40) );
    P( E, A, B, C, D, R(41) );
    P( D, E, A, B, C, R(42) );
    P( C, D, E, A, B, R(43) );
    P( B, C, D, E, A, R(44) );
    P( A, B, C, D, E, R(45) );
    P( E, A, B, C, D, R(46) );
    P( D, E, A, B, C, R(47) );
    P( C, D, E, A, B, R(48) );
    P( B, C, D, E, A, R(49) );
    P( A, B, C, D, E, R(50) );
    P( E, A, B, C, D, R(51) );
    P( D, E, A, B, C, R(52) );
    P( C, D, E, A, B, R(53) );
    P( B, C, D, E, A, R(54) );
    P( A, B, C, D, E, R(55) );
    P( E, A, B, C, D, R(56) );
    P( D, E, A, B, C, R(57) );
    P( C, D, E, A, B, R(58) );
    P( B, C, D, E, A, R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

    P( A, B, C, D, E, R(60) );
    P( E, A, B, C, D, R(61) );
    P( D, E, A, B, C, R(62) );
    P( C, D, E, A, B, R(63) );
    P( B, C, D, E, A, R(64) );
    P( A, B, C, D, E, R(65) );
    P( E, A, B, C, D, R(66) );
    P( D, E, A, B, C, R(67) );
    P( C, D, E, A, B, R(68) );
    P( B, C, D, E, A, R(69) );
    P( A, B, C, D, E, R(70) );
    P( E, A, B, C, D, R(71) );
    P( D, E, A, B, C, R(72) );
    P( C, D, E, A, B, R(73) );
    P( B, C, D, E, A, R(74) );
    P( A, B, C, D, E, R(75) );
    P( E, A, B, C, D, R(76) );
    P( D, E, A, B, C, R(77) );
    P( C, D, E, A, B, R(78) );
    P( B, C, D, E, A, R(79) );

#undef K
#undef F

    data->h0 = ctx->h0 + A;
    data->h1 = ctx->h1 + B;
    data->h2 = ctx->h2 + C;
    data->h3 = ctx->h3 + D;
    data->h4 = ctx->h4 + E;
}

// GPU Kernel called by the Host Cracking Thread
// This is similar to pyrit's code
__global__
void crack_gpu_kernel ( kernel_input_buffer *inbuffer , kernel_output_buffer *outbuffer , int max_num ) {

    // Loop variable
    int i;

    // Hash value of previous round
    SHA_DEV_CTX prev_ctx;
    // Part of the PMK (20 bytes first part, 12 bytes second part)
    SHA_DEV_CTX pmk_ctx;

    // Thread ID
    const int id = blockIdx.x * blockDim.x + threadIdx.x;
    
    //ORIGINAL
    // Is ID out of range?
    //if ( id >= max_num )
      //  return;

    // First round's hash (for PMK1) is stored in e1
    COPY_DEVCTX( prev_ctx , inbuffer[id].e1 );
    COPY_DEVCTX( pmk_ctx , prev_ctx );

    // PMK Part 1 (20 bytes): Finish the remaining 4095 rounds
    for ( i = 1 ; i <= 4095 ; i++ ) {

        sha1_process ( &inbuffer[id].ctx_ipad , &prev_ctx );
        sha1_process ( &inbuffer[id].ctx_opad , &prev_ctx );

        // Keep XORing all the rounds
        pmk_ctx.h0 ^= prev_ctx.h0;
        pmk_ctx.h1 ^= prev_ctx.h1;
        pmk_ctx.h2 ^= prev_ctx.h2;
        pmk_ctx.h3 ^= prev_ctx.h3;
        pmk_ctx.h4 ^= prev_ctx.h4;
    }

    // Store PMK Part 1 in the output buffer
    COPY_DEVCTX( outbuffer[id].pmk1 , pmk_ctx );

    // First round's hash (for PMK2) is stored in e2
    COPY_DEVCTX( prev_ctx , inbuffer[id].e2 );
    COPY_DEVCTX( pmk_ctx , prev_ctx );
    
    // PMK Part 2 (12 bytes): Finish the remaining 4095 rounds
    for ( i = 1 ; i <= 4095 ; i++ ) {
    
        sha1_process ( &inbuffer[id].ctx_ipad , &prev_ctx );
        sha1_process ( &inbuffer[id].ctx_opad , &prev_ctx );
        
        // Keep XORing all the rounds
        pmk_ctx.h0 ^= prev_ctx.h0;
        pmk_ctx.h1 ^= prev_ctx.h1;
        pmk_ctx.h2 ^= prev_ctx.h2; 
        pmk_ctx.h3 ^= prev_ctx.h3;
        pmk_ctx.h4 ^= prev_ctx.h4;
    }
    
    // Store PMK Part 2 in the output buffer
    COPY_DEVCTX( outbuffer[id].pmk2 , pmk_ctx );
}

// Function that triggers the GPU Kernel (Device Thread)
void invoke_gpu_kernel ( int blocksPerGrid , int threadsPerBlock , 
                         kernel_input_buffer *inbuffer , kernel_output_buffer *outbuffer , int max_num ) {
    crack_gpu_kernel<<<blocksPerGrid, threadsPerBlock>>>( inbuffer , outbuffer , max_num );
}
