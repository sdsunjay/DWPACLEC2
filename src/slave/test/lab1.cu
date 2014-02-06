#include "lab1.h"

#ifdef DOUBLE
__global__ void mmKernel(double *Md, double *Nd, double *Pd, int width)
#else
__global__ void mmKernel(float *Md, float *Nd, float *Pd, int width)
#endif
{
   //Calculate the row index of the Pd element and M
   int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
   // Calculate the column idenx of Pd and N
   int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

   if (Row * width + Col > width * width)
      return;
   
   #ifdef DOUBLE
   double Pvalue = 0.0;
   #else
   float Pvalue = 0.0;
   #endif

   for (int k = 0; k < width; k++) {
      #ifdef DOUBLE
      double Melem = Md[Row * width + k];
      double Nelem = Nd[k * width + Col];
      #else
      float Melem = Md[Row * width + k];
      float Nelem = Nd[k * width + Col];
      #endif

      Pvalue += Melem * Nelem;
   }
   Pd[Row* width + Col] = Pvalue;
}


matrix_t MMonDevice(matrix_t matrix1, matrix_t matrix2) {
   int blocks;
   blocks=1;
   int width = matrix1.rows;
   int size = width * width;
   matrix_t retMatrix;
   #ifdef DOUBLE
   double *Md, *Nd, *Pd, *P;
   size = size * sizeof(double);
   P = (double *) malloc(size);
   #else
   float *Md, *Nd, *Pd, *P;
   size = size * sizeof(float);
   P = (float *) malloc(size);
   #endif
   dim3 dimBlock;

   cudaMalloc(&Md, size);
   cudaMemcpy(Md, matrix1.array, size, cudaMemcpyHostToDevice);

   cudaMalloc(&Nd, size);
   cudaMemcpy(Nd, matrix2.array, size, cudaMemcpyHostToDevice);

   cudaMalloc(&Pd, size);

   blocks = width / TILE_WIDTH;
   //if (width % TILE_WIDTH > 0)
     // blocks++;
   printf("Number of blocks is: %d\n",blocks); 
   //invoke kernel
   // we probably need more than 1 block
   dim3 dimGrid(blocks, blocks);
   
   //width*width must be less than 1024
   if (width < TILE_WIDTH) {
      dimBlock.x = width;
      dimBlock.y = width;
   }
   else {
      dimBlock.x = TILE_WIDTH;
      dimBlock.y = TILE_WIDTH;
   }

   printf("Before mmKernel call\n");
   mmKernel<<<dimGrid, dimBlock>>>(Md, Nd, Pd, width);

   printf("array mmKernel call\n");
   //copy back
   cudaMemcpy(P, Pd, size, cudaMemcpyDeviceToHost);
   
   cudaFree(Md);
   cudaFree(Nd);
   cudaFree(Pd);

   retMatrix.array = P;
   retMatrix.rows = width;
   retMatrix.cols = width;

   return retMatrix;
}

int main(int argc, char* argv[])
{
   matrix_t matrix1, matrix2, matrix3;

   matrix1 = matrix_read(argv[1]);   
   matrix2 = matrix_read(argv[2]);   

   if(!check_dimensions(matrix1, matrix2)) {
      printf("Matrix dimensions don't match: m1 %d %d m2 %d %d\n", 
            matrix1.rows, matrix1.cols, matrix2.rows, matrix2.cols);
      exit(1);
   }

   matrix3 = MMonDevice(matrix1, matrix2);
   print_matrix(matrix3);

   free(matrix1.array);
   free(matrix2.array);
   free(matrix3.array);
   return 0;
}

matrix_t matrix_read(char* filename)
{
   //pointer to a new line
   matrix_t matrix;
   int columns = 0;
   int rows = 0;

   /* The file descriptor. */
   int fd;
   /* Information about the file. */
   struct stat s;
   int status;
   size_t size;
   /* The memory-mapped thing itself. */
   void* mapped;
   int i;
   int counter;
   counter=0;

   /* Open the file for reading. */
   fd = open (filename, O_RDONLY);

   /* Get the size of the file. */
   fstat (fd, &s);

   /* Memory-map the file. */
   mapped = mmap (0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
   check (mapped == MAP_FAILED, "mmap %s failed: %s",
	 file_name, strerror (errno));

   /* Now do something with the information. */
   for (i = 0; i < size; i++) {


      if((char*) mapped[i]=='\n')
      {
	 printf("number of numbers in a row is %d\n",counter);
	 break;
      }
      else if((char*) mapped[i]==32)
      {
	 counter++;
      }

   }

   return 0;





/*completely working code
 * *****************************************************************/
   struct stat sb;
   //mmap pointer
   void* file_memory;
   //back up of mmap pointer
   void* backup_file_memory;
   //temp str used for str tok
   char* temp_str;


   /*character used to parse first line*/
   char c;
   char* ptr;
   //file descriptor
   int fd;
   //loop counters
   int j;
   int i;

   fd = open (filename, O_RDONLY);
   // figure out the size
   fstat(fd, &sb);

   file_memory = mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
   if(file_memory==MAP_FAILED)
   {

      close (fd);
      fprintf(stderr,"mmap failed. Exitting\n");
      exit(-1);
   }
   close (fd);

   //save original location 
   backup_file_memory=file_memory;

   i=0; 
/*find out how many numbers there are before the newline */ 
   do{
      sscanf ((char*) file_memory, "%c", &c);
      file_memory+=sizeof(char);

      if(c==' ')
      {
	 i++;
      }
      else if(c=='.')
      {
	 //if we are to assume always 2 digits after decimal
	 file_memory=(2*sizeof(char))+file_memory;
      }
      else if(c=='-')
      {
	 //if we assume guaranteed decimal with 2 digits after it
	 file_memory=(4*sizeof(char))+file_memory;
      }

   }while(c!='\n');
   
   
  /*we have to malloc to insert a null character*/
  /*without null we don't know where to stop reading floats or doubles*/ 
  ptr = (char *)malloc( sb.st_size + 1);
   /*copy the memory*/
  memcpy(ptr, backup_file_memory, sb.st_size);
  /*add the null character*/
  ptr[sb.st_size] = '\0';

  /*we no longer need mmapped memory*/
  munmap(file_memory, sb.st_size);
   //munmap(file_memory, sb.st_size);
   
  /*malloc for matrix*/ 
  float* array = (float *)malloc(sizeof(float) * (i*i));
   if (array == NULL) {
      perror("No space to allocate matrix.");
      exit(1);
   }
   /*parse string using tokens*/
   temp_str=strtok(ptr," \n");
   for(i=0;i<fd;i++)
   {
      for(j=0;j<fd;j++)
      {
	 //copy string token into matrix
	 sscanf(temp_str,"%f",&array[i*fd+j]);
	 temp_str=strtok(NULL," \n");
      }
   }
   /*
   for(i=0;i<fd;i++)
   {
      for(j=0;j<fd;j++)
      {
	 printf("%f\n",array[i*fd+j]);
      }
   }
*/

     return 0;
}

/*end completely working code
 * ****************************************************************/





#ifdef DOUBLE
   void *array;
#else
   float *array;
#endif 

   fp = fopen(filename, "r");
   if(fp == NULL)
   {
      fprintf(stderr,"Error opening file\n");
      exit(-1);
   }

   //square matrix
   rows=columns;
   /*
#ifdef DOUBLE
array = (double *) calloc(rows * columns,sizeof(double));
#else
array = (float *) calloc(rows * columns,sizeof(float));
#endif

   //store numbers from matrix
#ifdef DOUBLE
while(fscanf(fp, "%lf", &array[counter])==1) 
#else
while(fscanf(fp, "%f", &array[counter])==1)
#endif
{
counter++;
}

fclose(fp);
    */
   matrix.rows = rows;
   matrix.cols = columns;
   matrix.array = array;

   return matrix;
   }
//return if the two matrices have valid dimensions
int check_dimensions(matrix_t matrix1,matrix_t matrix2)
{

   return matrix1.cols==matrix2.rows;
}
//multiply the matrices together
matrix_t multiply_matrices(matrix_t matrix1, matrix_t matrix2)
{
   //loop counters
   int i;
   int inner;
   int row, col;
   //output matrix
   matrix_t matrix;
   //dot product sum
#ifdef DOUBLE
   double sum;
   matrix.array = (double *) calloc(matrix1.rows * matrix2.cols, sizeof(double));
#else
   float sum;
   matrix.array = (float *) calloc(matrix1.rows * matrix2.cols, sizeof(float));
#endif

   matrix.rows = matrix1.rows;
   matrix.cols = matrix2.cols;
   for(i = 0; i < matrix1.rows * matrix2.cols; i++)
   {
      sum = 0.0;

      //find position in resulting matrix
      col = i % matrix2.cols;
      row = i / matrix2.cols;
      for(inner = 0; inner < matrix1.cols; inner++)
      {
	 sum += matrix1.array[row * matrix1.cols + inner]
	    * matrix2.array[inner * matrix2.cols + col];
      }

      matrix.array[row * matrix.cols + col] = sum;
   }

   return matrix;
}
//print result matrix to 'result.out'
void print_matrix(matrix_t matrix) {
   int i,j;
   FILE *fp;

   fp = fopen("result.out", "w");

   for (i = 0; i < matrix.rows; i++) {
      for (j = 0; j < matrix.cols; j++) {
#ifdef DOUBLE
	 fprintf(fp, "%.2f ", matrix.array[i * matrix.cols + j]);
#else
	 fprintf(fp, "%.2f ", matrix.array[i * matrix.cols + j]);
#endif
      }
      fprintf(fp, "\n");
   }
   fclose(fp);
}
