#include <sys/stat.h>
#include <sys/mman.h> 
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct _matrix_t{
   int rows;
   int cols;
#ifdef DOUBLE
   double *array;
#else
   float *array;
#endif
} matrix_t;

#define TILE_WIDTH 32

matrix_t matrix_read(char *);   
int check_dimensions(matrix_t matrix1,matrix_t matrix2);
void print_matrix(matrix_t matrix);
matrix_t multiply_matrices(matrix_t matrix1,matrix_t matrix2);


