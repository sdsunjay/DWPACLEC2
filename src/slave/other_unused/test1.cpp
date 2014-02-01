#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;

#include <cstring>

const int MAX_CHARS_PER_LINE = 25;
const int MIN_CHARS_PER_LINE = 8;

int main(int argc, char* argv[])
{
   //the number of words we have skipped due to length.
   int skipped = 0;
   //the number of words greater than 7 and less than 63
   int wordsAdded = 0;
   if(argc==1)
   {
      printf("usage: ./a.out <name of file>\n");
      return 1;
   }
   printf("File must have 1 password per line\n");
   // create a file-reading object
   ifstream fin;
   fin.open(argv[1]); // open a file
   if (!fin.good())
      return 1; // exit if file not found

   // read each line of the file
   while (!fin.eof())
   {
      // read an entire line into memory
      char buf[MAX_CHARS_PER_LINE];
      fin.getline(buf, MAX_CHARS_PER_LINE);
      int length = strnlen(buf,MAX_CHARS_PER_LINE);

      /* Test length of word.  IEEE 802.11i indicates the passphrase must be
       * at least 8 characters in length, and no more than 63 characters in
       * length.
       */
      if (length < MIN_CHARS_PER_LINE || length > MAX_CHARS_PER_LINE) {
         /*if (verbose) {
           printf("Invalid passphrase length: %s (%d).\n",
           passphrase, (int)strlen(passphrase));
           } */
         /*
          *                           * Output message to user*/
        // fprintf(stderr, "Skipped a word\n");
         skipped++;
         //continue;
      } else {
         /* This word is good, increment the words tested counter */
         wordsAdded++;
        // printf("Added a word\n");
         //printf("%d\t%s\n",length,buf);
      }
   }
   printf("Skipped %d words due to length\n",skipped);
   printf("Added %d words to the database\n",wordsAdded);
}
