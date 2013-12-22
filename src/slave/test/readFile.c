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


int read_lines(const char * fname, int (*call_back)(const char*, const char*))
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

   /* fs.st_size could have been 0 actually */
   buf = mmap(0, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (buf == (void*) UNDEFINED) {
      err(1, "mmap: %s", fname);
      close(fd);
      return 0;
   }

   buf_end = buf + fs.st_size;

   begin = end = buf;
   while (1) {
      if (! (*end == '\r' || *end == '\n')) {
         if (++end < buf_end) continue;
      } else if (1 + end < buf_end) {
         /* see if we got "\r\n" or "\n\r" here */
         c = *(1 + end);
         if ( (c == '\r' || c == '\n') && c != *end)
            ++end;
      }

      /* call the call back and check error indication. Announce
       *                    error here, because we didn't tell call_back the file name */
      if (! call_back(begin, end)) {
         err(1, "[callback] %s", fname);
         break;
      }

      if ((begin = ++end) >= buf_end)
         break;
   }

   //free the memory
   //munmap(buf, fs.st_size);
   close(fd);
   return 1;
}

int print_line(const char* begin, const char* end)
{
   //original
   //if (write(fileno(stdout), begin, end - begin + 1) == UNDEFINED) {
   listOfWords[globalCounter]->begin=begin;
   listOfWords[globalCounter]->end=end;
   globalCounter++;
   if(begin>end)
      print_error("memory address wrong");
   //if (write(fileno(stdout), begin, end - begin ) == UNDEFINED) {
     // return 0;
  // }
   //printf("WORD \t %s\n",word);
   return 1;
}
int getSpace()
{
   int i=0;
   for(i;i<NUM_WORDS;i++)
   {
      listOfWords[i]=(pwd*) malloc(sizeof(pwd));
      if (listOfWords[i] == NULL) {
         printf("Out of memory\n");
         exit(-1);
      }
   }
}
int printWords()
{
   int i=0;
   int len;
   for(i;i<NUM_WORDS;i++)
   {
      char str[25];
      len = snprintf(str, (listOfWords[i]->end - listOfWords[i]->begin), "%s",listOfWords[i]->begin);
      printf("%s",str);
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
   if(errno == EDESTADDRREQ)
      perror("file descriptor refers to a datagram socket");
   else if(errno == EAGAIN)
      perror("file descriptor refers to a socket, which is nonblocking");
   else if (errno == EFBIG)
      perror("Tried to write to a file that exceeds max file size");
   else if(errno == EINTR)
      perror("The call was interrupted before any data was written");
   else if(errno ==  EINVAL)
      perror("file descriptor specifies an object unsuitable for writing");
   else if(errno==ENAMETOOLONG)
      perror("File name too long.");
   else if(errno==ENOENT)
      perror("part or all of the path does not exist.");
   else if (errno == ENOMEM)
      perror("Out of memory (i.e., kernel memory).");
   else if (errno ==ENOTDIR)
      perror("A component of the path prefix of path is not a directory.");
   else if(errno==EFAULT)
      perror("Referenced invalid memory");
   else if(errno==EPERM)
      perror("Unpriviledge access attempt");
   else if( errno == ENAMETOOLONG)
      perror("path name too long");
   else if(errno == EACCES)
      perror("Search permission is denied for part of the path prefix.");
   else if(errno == ELOOP)
      perror("Too many symbolic links were found in the pathname.");
   else if(errno == EROFS)
      perror("The named file resides on a read-only file system.");
   else if(errno == EIO)
      perror("An I/O error occurred while reading or writing.");
   else if(errno == EMFILE)
      perror("Too many opens on this file");
   else if (errno == ENFILE)
      perror("Too many opens in this system");
   else if(errno ==EBADF)
      perror("The file descriptor is not valid.");
   else if(errno == EINVAL)
      perror("The fd argument refers to a socket, not to a file.");
   else if(errno == EISDIR)
      perror("file descriptor refers to a directory.");
   /*error was either not listed or not found, 
    * so we print a message for the specific command*/
   else
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
   //only 1000 words in the list
   listOfWords = (pwd**) malloc (sizeof(pwd*)*NUM_WORDS);
   getSpace();
   int returnValue = read_lines(argv[1], print_line);
   if(returnValue==1)
   {
     // printWords();
   }
   else
   {
      printf("ERRRROR!\n");
      return 1;
   }
   return 0;
}
