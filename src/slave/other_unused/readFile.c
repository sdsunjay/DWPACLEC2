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
//#define NUM_WORDS 100000000
//global bad...dgaf
void print_error(char* command);
int safe_output(char* msg);

typedef struct _pwd_range
{
  const char* begin;
  const char* end;
} pwd;
int globalCounter=0;
pwd** listOfWords;

/*safely fill output string, string, with @param msg*/
/*return number of characters 'printed'*/
int safe_output(char* msg)
{
   if(UNDEFINED==write(STDERR_FILENO,msg,strlen(msg)))
   {
      print_error("write");
      exit(EXIT_FAILURE);
   }
   return 1;
}

int read_lines(const char * fname)
{
   int fd = open(fname, O_RDONLY);
   struct stat fs;
   char *buf, *buf_end;
   char *begin, *end, c;

   if (fd == UNDEFINED) {
      err(1, "open: %s", fname);
      return 0;
   }

   if (fstat(fd, &fs) == UNDEFINED) {
      err(1, "stat: %s", fname);
      return 0;
   }

   int counter=0;
   int i=0;
   char* s_begin;
   /* fs.st_size could have been 0 actually */
   //buf = mmap(0, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
   buf= (char*) mmap(0, fs.st_size+1, PROT_READ, MAP_PRIVATE, fd, 0);
   if (buf == (void*) UNDEFINED) {
      err(1, "mmap: %s", fname);
      close(fd);
      return 0;
   }
   //size of file
   //printf("%x\n",fs.st_size);
   buf_end = buf + fs.st_size;

   begin = end = s_begin = buf;
   while(buf[i]!='\0')
   {
      //printf("%d\n",i);
      //count the number of rows and columns in the matrix
      if(buf[i]=='\n')
      {
         int len;
         char str[25];
         len = snprintf(str,(counter+1)*sizeof(char), "%s",s_begin);
         i++;
         s_begin=&buf[i];
         counter=1;
         if((i+4)<fs.st_size)
         {
            i+=4;
            counter+=4;
         }
         //no password shorter than 2 characters
        // i++;
        // i++;
        // counter++;
         //counter++;
        // if(i%2==0)
        // {
          //  printf("%d",i);
         printf("%d) %s\n",i,str);
         //}
      }
      else
      {
         counter++;
      }
      i++;
      /*  if (! (*end == '\r' || *end == '\n')) {
          if (++end < buf_end) continue;
          } else if (1 + end < buf_end) {
      // see if we got "\r\n" or "\n\r" here 
      c = *(1 + end);
      if ( (c == '\r' || c == '\n') && c != *end)
      ++end;
      }*/

      /* call the call back and check error indication. Announce
       *                    error here, because we didn't tell call_back the file name */
      /* if (! call_back(begin, end)) {
         err(1, "[callback] %s", fname);
         break;
         }*/

   }

   //free the memory
   munmap(buf, fs.st_size);
   close(fd);
   return 1;
}

int print_line(const char* begin, const char* end)
{
   //original
   //if (write(fileno(stdout), begin, end - begin + 1) == UNDEFINED) {
   //listOfWords[globalCounter]->begin=begin;
   //listOfWords[globalCounter]->end=end;
   //printf("%d\n",globalCounter);
   if(begin>end)
   {
      print_error("memory address wrong");
      return 1;
   }
   int len;
   char str[25];
   //len = snprintf(str, (end - (begin-1)), "%s",begin);
  // globalCounter++;
   //printf("%s \n",str);
   //if (write(fileno(stdout), begin, end - begin ) == UNDEFINED) {
   // return 0;
   // }
   //printf("WORD \t %s\n",word);
   return 1;
}
int printWords()
{
   int i=0;
   int len;
   for(i;i<globalCounter;i++)
   {
      char str[25];
      len = snprintf(str, (listOfWords[i]->end - (listOfWords[i]->begin-1)), "%s",listOfWords[i]->begin);
      printf("%s \n",str);
      // snprintf(char * restrict str, size_t size, const char * restrict format,
      //               ...);
      //if (write(fileno(stdout), listOfWords[i]->begin, listOfWords[i]->end - listOfWords[i]->begin ) == UNDEFINED) {
      //  print_error("write");
      //}
   }
}
/*print out the specific error message
 * as determined by errno
 * unless it cannot be found, 
 * then print error message for specific @param command*/
void print_error(char* command)
{
   if(command)
   {
      safe_output("Unknown ");
      safe_output(command);
      safe_output(" error. Quitting.\n");
   }
   else
   {
      safe_output("Unknown error. Quitting.\n");
   }
   /*the program has seg faulted, quit*/
   exit(EXIT_FAILURE);

}
int main(int argc, char* argv[])
{
   if(argc==1)
   {
      printf("usage: ./a.out <name of file>\n");
      return 1;
   }
   int returnValue = read_lines(argv[1]);
   return 0;
}
