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
   matrix3 = MMonDevice(matrix1, matrix2);
   print_matrix(matrix3);

   free(matrix1.array);
   free(matrix2.array);
   free(matrix3.array);
   return 0;
}
