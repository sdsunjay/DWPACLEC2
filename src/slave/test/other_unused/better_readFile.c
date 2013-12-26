
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include<string.h>
#define UNDEFINED (-1)
//100,000,000 - 100 million words
#define NUM_WORDS 100000000

int main(int argc, char* argv[])
{
   if(argc==1)
   {
      printf("usage: ./a.out <name of file>\n");
      return 1;
   }
   FILE *f= fopen(argv[1],"r");

   // find the file size
   fseek(f, 0, SEEK_END);
   int size = ftell(f);
   rewind(f);
   printf("Size is: %d\n",size);
   printf("buffer is: %d\n",size/500);
   // Allocate buffer and read the entire file in a single read.
   char buff[size/500];
   int real_size=size/500;
   if (f) {
      int lastlen=0;
      int len;
      while((len = fread(buff+lastlen, 1, size-lastlen-1, f)) > 0) {
         lastlen+=len;
         buff[len]='\0';
         char *token  = strtok(buff, "\n");
         for (; token; token = strtok(NULL, "\n"))
            printf ("%s\n", token);
      }
      //int len = fread(buff, 1, size/100, f);
      fclose(f);
   }

   // Process the file (assuming entries are separated by newlines)
   return 0;
}
