
// Endianess, init values, speed report, finish report, spin lock on GPUs????, error checking
// Optimization - Verifying MIC some parts are not required or can be done beforehand
// TODOs
// Check speed by not reporting finish, 
// turn off cpus and see if gpu working, etc.
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
#include "headers/sdkHelper.h"
#include "headers/shrQATest.h"
#include "headers/shrUtils.h"

#include "headers/cpu-crack.h"
#include "headers/gpu-crack.h"

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

//GLOBALS
//Connection to MySQL DB
extern MYSQL* MySQLConnection[NUM_DB_CONNECTIONS];
//Stores the list of passwords we queried for
//char **passwordList;
extern unsigned long keys;
char* final_key; 
//verbosity flag
extern int vflag;
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
precompute ( const char *key, const char *essid_pre , kernel_input_buffer *gpu_input ) {

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

	//Unspecific Loop variables
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
	{
		printf("FOUND!\n");
		return 1;
	}
	// Return 0 if not found, 1 if found
	return 0;
}
/**
 * Used to get the password given a particular offset
 * Used to get the correct password immediately before exiting
 */
int get_password(int db_connector_index, int gpu_num,unsigned long offset)
{

	//connection to DB
	MYSQL_RES      *mysqlResult = NULL;
	MYSQL_ROW       mysqlRow;
	//unsigned int numRows;
	//unsigned int numFields;
	int mysqlStatus = 0;
	//for the query
	char query[QUERY_BUFFER];

	// --------------------------------------------------------------------
	// Perform a SQL SELECT and retrieve data
	// There should not be a terminating ';'
	sprintf(query, "SELECT %s FROM %s LIMIT 1 OFFSET %lu",COLUMN_NAME,TABLE_NAME,offset);
	// printf("Query is: %s\n",query);
	//printf("Range start is : %lu\n",range.start);

	mysqlStatus = mysql_query(MySQLConnection[db_connector_index],query);
	if (mysqlStatus)
	{
		fprintf(stderr,"Unable to connect to get the password at %lu, quitting\n",offset);
		return 0; 
	}
	else
	{
		mysqlResult = mysql_store_result(MySQLConnection[db_connector_index]); // Get the Result Set
	}
	if(!mysqlResult)
	{
		fprintf(stderr,"Result set is empty");
		mysql_close(MySQLConnection[db_connector_index]);
		return 0;
	}
	if(mysqlRow = mysql_fetch_row(mysqlResult)) // row pointer in the result set
	{
		printf("%s\n",mysqlRow[0]);
		memcpy ( final_key , mysqlRow[0] , strlen ( mysqlRow[0] ) ); 
		return 1;
	}
	return 0;
}
//make query and place potential passwords into the already allocated space
unsigned int query_and_fill(int db_connector_index, int gpu_id,char* essid,unsigned long start_index,kernel_input_buffer *gpu_input)
{

	//connection to DB
	MYSQL_RES      *mysqlResult = NULL;
	MYSQL_ROW       mysqlRow;
	//unsigned int numRows;
	//unsigned int numFields;
	int mysqlStatus = 0;
	//for the query
	char query[QUERY_BUFFER];


	unsigned int password_index;
	//how many passwords we saved
	//set back to 0	
	password_index = 0;
	// --------------------------------------------------------------------
	// Perform a SQL SELECT and retrieve data
	// There should not be a terminating ';'
	sprintf(query, "SELECT %s FROM %s LIMIT %d OFFSET %lu",COLUMN_NAME,TABLE_NAME,PWD_BATCH_SIZE_GPU,start_index);
	if(vflag)
	{
		printf("GPU Query is %s\n",query);
	}
	//printf("Range start is : %lu\n",range.start);

	mysqlStatus = mysql_query(MySQLConnection[db_connector_index],query);
	if (mysqlStatus)
	{
		fprintf(stderr,"GPU Thread: MySQL Error:\nQuitting");
		return 0;
	}
	else
	{
		mysqlResult = mysql_store_result(MySQLConnection[db_connector_index]); // Get the Result Set
	}
	/* if (mysqlResult)  // there are rows
	   {
	// # of rows in the result set
	numRows = mysql_num_rows(mysqlResult);

	// Returns the number of columns in a result set specified
	numFields = mysql_num_fields(mysqlResult);

	//printf("Number of rows=%u  Number of fields=%u \n",numRows,numFields);
	}*/
	if(!mysqlResult)
	{
		fprintf(stderr,"Result set is empty");
		return 0;
	}
	while(mysqlRow = mysql_fetch_row(mysqlResult)) // row pointer in the result set
	{
		//IF we just kept track of the index of where the password was found,
		//we THINK we could replace the two lines below with the following:
		//no longer necessary to keep track of password, we just track index
		// Precompute the iKeypads, oKeypads and 1st Round Hashes
		//precompute(mysqlRow[0], essid, & gpu_input[password_index]);
		precompute(mysqlRow[0], essid, & gpu_input[ (gpu_id*PWD_BATCH_SIZE_GPU)+ password_index]);


		// Count the total number of keys
		password_index++;
	}
	if(vflag)
	{
		printf("GPU got %u number of rows\n",password_index);
	}
	return password_index;
}
void cleanUp(int cpu_num,int gpu_num)
{

	printf("GPU thread: closing DB connection\n");
	mysql_close(MySQLConnection[cpu_num]);
	printf("Tested ");
	printf("%lu",keys);
	printf(" keys total.\n");
	printf("GPU thread exiting\n");

}
// GPU Crack Host thread
void* 
crack_gpu_thread ( void *arg ) {

	// ESSID
	char essid[32];

	//ORIGINAL
	// Password (key) in string format
	//char key[128];
	//memset ( key , 0 , sizeof ( key ) );

	// Params passed in arguments
	ck_td_struct* ck_td_arg = (ck_td_struct*)arg;
	wpa_hdsk* phdsk = ck_td_arg->phdsk;
	int cpu_num = ck_td_arg->cpu_core_id;
	int gpu_num = ck_td_arg->gpu_core_id;
	if ( gpu_num == 0 ) // GPU not used
	{
		printf("No GPUs found\n");
		return NULL;
	}   
	float* calc_speed = ck_td_arg->calc_speed;
	final_key = ck_td_arg->final_key;
	char* final_key_flag = ck_td_arg->final_key_flag;
	memset(essid, 0, sizeof(essid));
	memcpy(essid, ck_td_arg->essid, 32);

	// Normal (unspecific) Loop Variable
	int i = 0;

	//iterates through the gpu_num in loops
	int gpu_iter;

	//index variable for the password range
	unsigned int password_total;

	// For calculating the PMK/sec speed
	struct timeval tprev;
	struct timeval tnow;

	gettimeofday (&tprev, NULL );
	// Password range for each GPU
	pwd_range *range;
	range = (pwd_range*) malloc ( sizeof ( pwd_range )*gpu_num);

	//time that has passed
	float total_time =0.00;
	/*
	//allocate password space

	printf("Allocating space for %d passwords of length %d\n",(gpu_num * PWD_BATCH_SIZE_GPU),LONGEST_PASSWORD);
	passwordList = (char**) calloc( (gpu_num * PWD_BATCH_SIZE_GPU),  sizeof(char*));

	for(i = 0; i <gpu_num*PWD_BATCH_SIZE_GPU ; i++) {
	passwordList[i] = (char*) calloc(LONGEST_PASSWORD, sizeof(char));

	}

	 */
	// Number of working GPUs
	int gpu_working = 0;

	printf("%d threads per block\n",THREADS_PER_BLOCK);
	printf("%d blocks per grid\n",( PWD_BATCH_SIZE_GPU + THREADS_PER_BLOCK - 1 ) / THREADS_PER_BLOCK);
	// Input Buffer of the GPUs
	kernel_input_buffer *gpu_input = (kernel_input_buffer*) malloc ( sizeof ( kernel_input_buffer ) * PWD_BATCH_SIZE_GPU * gpu_num );
	// Output Buffer of the GPUs
	kernel_output_buffer *gpu_output = (kernel_output_buffer*) malloc ( sizeof ( kernel_output_buffer ) * PWD_BATCH_SIZE_GPU * gpu_num );

	// Buffers in the GPU Memory space
	kernel_input_buffer **device_input;
	kernel_output_buffer **device_output;

	if(vflag==1)
	{
		printf("malloc called for input and output buffers\n");
	}
	device_input = (kernel_input_buffer**) malloc ( sizeof ( kernel_input_buffer* ) * gpu_num );
	device_output = (kernel_output_buffer**) malloc ( sizeof ( kernel_output_buffer* ) * gpu_num );

	// Allocate device memory beforehand itself, we can reuse it again and again
	int devMemSize;
	for ( gpu_iter = 0 ; gpu_iter < gpu_num ; ++gpu_iter ) {
		if(vflag==1)
		{
			printf("cuda malloc called for gpu %d of %d\n",gpu_iter,gpu_num);
		}
		checkCudaErrors ( cudaSetDevice(gpu_iter) );
		devMemSize = sizeof ( kernel_input_buffer ) * PWD_BATCH_SIZE_GPU;
		checkCudaErrors ( cudaMalloc ( (void**) &device_input[gpu_iter] , devMemSize ) );
		devMemSize = sizeof ( kernel_output_buffer ) * PWD_BATCH_SIZE_GPU;
		checkCudaErrors ( cudaMalloc ( (void**) &device_output[gpu_iter] , devMemSize ) );
	}

	// Repeatedly get password ranges to dispatch to the GPU(s)
	while ( 1 ) {

		// Get the password range for each gpu
		gpu_working = 0;
		//original
		for ( gpu_iter = 0 ; gpu_iter < gpu_num ; ++gpu_iter ) {

			range[gpu_iter] = fetch_pwd ( 'g' , NULL,NULL);
			++gpu_working;
			if ( range[gpu_iter].start == 0.5 )
			{
				fprintf(stderr,"starting range is invalid for GPU, quitting\n");
				// Tell main thread we are terminating
				gpu_working=0;
				goto stop;
			}

			//for the number of GPUs working
			password_total=query_and_fill(cpu_num,gpu_iter,essid,range[gpu_iter].start,gpu_input);
			//this will happen when query does not return anymore rows
			if(password_total == 0)
			{
				printf("read 0 passwords from %s in %s\nQuitting!\n",TABLE_NAME,DB_NAME);
				gpu_working=0;
				goto stop;
			}
			//password_total should be the number of passwords in the range, meaning we looped through all of them and precomputed them
			keys+=password_total;
		}	
		/*
		   if(vflag==1)
		   {
		   printf("GPU id %d offset is: %lu\n",gpu_iter,range[0].start);
		   }*/

		//	}
		//printf("%d\n",gpu_working);
		// Check if password range is over
		if ( gpu_working <= 0 ) {

			// Tell main thread we are terminating
			calc_speed[ cpu_num ] = -1;

			// Free resources
			free ( range );
			free ( gpu_input );
			free ( gpu_output );
			for ( gpu_iter = 0 ; gpu_iter < gpu_num ; ++gpu_iter ) {
				checkCudaErrors ( cudaSetDevice (gpu_iter) );
				checkCudaErrors ( cudaFree ( (void*) device_input[gpu_iter] ) );
				checkCudaErrors ( cudaFree ( (void*) device_output[gpu_iter] ) );
			}
			free ( device_input );
			free ( device_output );
			printf("\nNo GPUs working, quitting\n");
			goto stop;
		}



		// Start time of the computation (including memory transfers Host mem <==> Device mem)
		//	gettimeofday ( &tprev , NULL );

		//printf("Total number of passwords checked: %lu in \n",keys);
		//for each unique password in our range
		//  for (i = range[gpu_iter].start; i < range[gpu_iter].end ; i++ ) 
		//this seems right,
		//FIX THE REST...later

		/*
		   THIS IS NOW BEING DONE IN QUERY_AND_FILL

		   for(i=0;i<total_num_passwords;i++)
		   {
		//ORIGINAL
		// Convert the key from digit to string
		//             sprintf ( key , "%08lu" , cur_key_digit );

		//strcpy(key,passwordList[i]);
		//SUNJAY
		//printf("%d) GPU Password: %s\n",i,key);	
		// Calculate the Kernel input buffer values for this key
		//precompute (passwordList[i], essid , & gpu_input[ ( gpu_iter * PWD_BATCH_SIZE_GPU ) + (i - range[gpu_iter].start ) ] );
		 ******
		//ORIGINAL 
		// Calculate the Kernel input buffer values for this key
		precompute ( key , essid , & gpu_input[ ( i * PWD_BATCH_SIZE_GPU ) + ( cur_key_digit - range[i].start ) ] );
		 *****
		//precompute (passwordList[i], essid , & gpu_input[i]);


		// Count the total number of keys
		++num_keys;
		// if(num_keys%100==0)
		//  printf("Total number of passwords we precomputed: %d\n",num_keys);
		}*/
		// }
		//printf("Total number of passwords read is %d\n",total_num_passwords);

		// if(num_keys%PWD_BATCH_SIZE_GPU==0)
		// {
		//  printf("Total number of passwords we precomputed: %d\n",num_keys);
		// }
/*if(num_keys!=total_num_passwords)
  {
  printf("We did NOT precompute all the passwords\n");
  printf("Total number of passwords we got from the DB: %d\n",total_num_passwords);
  printf("Total number of passwords we precomputed: %d\n",num_keys);
  }*/
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

if(vflag)
{
	printf("There are %d GPUs working\n",gpu_working);
}
for ( gpu_iter = 0 ; gpu_iter < gpu_working ; ++gpu_iter ) 
{
	// Set the GPU Device we are currently dispatching work to crack
	checkCudaErrors ( cudaSetDevice ( gpu_iter ) );

	// Copy the Input buffers from the Host to Device (GPU) Memory
	devMemSize = sizeof ( kernel_input_buffer ) * PWD_BATCH_SIZE_GPU;
	checkCudaErrors ( cudaMemcpy ( device_input[gpu_iter] , gpu_input + (gpu_iter * PWD_BATCH_SIZE_GPU) , devMemSize , cudaMemcpyHostToDevice ) );

	// Calculate the PMKs using GPU
	//int max_num = range[gpu_iter].end - range[gpu_iter].start + 1;
	//GPU ALWAYS HAS MAX NUMBER OF THREADS, regardless of number of passwords
	int blocksPerGrid = (PWD_BATCH_SIZE_GPU + THREADS_PER_BLOCK - 1 ) / THREADS_PER_BLOCK;

	invoke_gpu_kernel ( blocksPerGrid , THREADS_PER_BLOCK , device_input[gpu_iter] , device_output[gpu_iter] , PWD_BATCH_SIZE_GPU);
	getLastCudaError ( "Kernel launch failure!!" );
}

// Copy the Output buffers to the Host from Device (GPU) Memory in SEPARATE FOR LOOP
for ( gpu_iter = 0 ; gpu_iter < gpu_working ; ++gpu_iter ) {

	// Set the GPU Device we are currently dispatching work to crack
	checkCudaErrors ( cudaSetDevice ( gpu_iter ) );

	// Copy the Output buffers to the Host from Device (GPU) Memory
	devMemSize = sizeof ( kernel_output_buffer ) * PWD_BATCH_SIZE_GPU;
	checkCudaErrors ( cudaMemcpy ( gpu_output + (gpu_iter * PWD_BATCH_SIZE_GPU) , device_output[gpu_iter] , devMemSize , cudaMemcpyDeviceToHost ) );
}

// Check if the key (password) was found
for ( gpu_iter = 0 ; gpu_iter < gpu_working ; ++gpu_iter ) {
	//ORIGINAL
	//loop through all passwords for a particular gpu
	//for (i = range[gpu_iter].start; i <= range[gpu_iter].end; ++i ) {
	//printf("Checking if passwords from %d to %d are the password\n",(gpu_iter*PWD_BATCH_SIZE_GPU),(PWD_BATCH_SIZE_GPU*(gpu_iter+1)-1));
	for(i=gpu_iter*PWD_BATCH_SIZE_GPU;i<PWD_BATCH_SIZE_GPU*(gpu_iter+1);i++){	
		//for(i=gpu_iter*PWD_BATCH_SIZE_GPU;i<PWD_BATCH_SIZE_GPU+(gpu_iter*PWD_BATCH_SIZE_GPU);i++){
		//printf("checking %d for key\n",i);
		// Verify the MIC
		//ORIGINAL
		//if ( is_key_found ( & gpu_output[ ( gpu_iter * PWD_BATCH_SIZE_GPU ) + ( i - range[gpu_iter].start ) ] , phdsk ) ) {
		if ( is_key_found (&gpu_output[i] , phdsk) ) {
			*final_key_flag = 1;

			//printf("GPU found the key\nPassword: %s\n",passwordList[i]);
			unsigned int password_found_index=range[gpu_iter].start+(i-(gpu_iter*PWD_BATCH_SIZE_GPU));
			printf("GPU found the key\nPassword is at index %u\n",password_found_index);
			if(get_password(cpu_num,gpu_num,password_found_index))
			{
				printf("Successfully looked up password.\n");
			}
			else
			{
				printf("Error in get_password() function\n");
			}

			// !!!!! We found the key !!!!!

			// End time of computation (including memory transfers Host mem <==> Device mem)

			// Report speed to main thread
			//calc_speed[ cpu_num ] = (float) num_keys / ( tnow.tv_sec - tprev.tv_sec + ( tnow.tv_usec - tprev.tv_usec ) * 0.000001F );

			// Sleep a little so that the main thread will read the speed
			// sleep ( 1 );

			// Convert the key from digit to string
			//ORIGINAL
			//sprintf ( key , "%08lu" , cur_key_digit );
			// Report the key to the main thread
			//ORIGINALLY WAS NOT COMMENTED OUT

			//memcpy ( final_key , passwordList[i] , strlen (passwordList[i] ) );

			// Tell main thread we are terminating
			// calc_speed[ cpu_num ] = -1;

			// Free resources
			free ( range );
			free ( gpu_input );
			free ( gpu_output );

			int ii;
			for ( ii = 0 ; ii < gpu_num ; ++ii ) {
				checkCudaErrors ( cudaSetDevice ( ii ) );
				checkCudaErrors ( cudaFree ( (void*) device_input[ii] ) );
				checkCudaErrors ( cudaFree ( (void*) device_output[ii] ) );
			}
			free ( device_input );
			free ( device_output );
			goto stop;
			//closes if key is found
		}
		//closes for loop for iterating through generated pmks
	}
	//closes for loop for passwords generated by a single gpu, incrementing by GPU_PWD_BATCH_SIZE
	}

	gettimeofday ( &tnow , NULL );
	total_time = tnow.tv_sec - tprev.tv_sec + ( tnow.tv_usec - tprev.tv_usec ) * 0.000001F;
	printf ( "Tested %lu passwords in %.2f seconds.\n" ,keys,total_time);
	// End time of computation (including memory transfers Host mem <==> Device mem)
	//gettimeofday ( &tnow , NULL );

	// Report speed to main thread
	// calc_speed[ cpu_num ] = (float) num_keys / ( tnow.tv_sec - tprev.tv_sec + ( tnow.tv_usec - tprev.tv_usec ) * 0.000001F );
	}
stop: 
	// Tell main thread we are terminating
	calc_speed[ cpu_num ] = -1;
	gettimeofday ( &tnow , NULL );
	cleanUp(cpu_num,gpu_num);
	//cudaDeviceReset();
	return NULL;
}
