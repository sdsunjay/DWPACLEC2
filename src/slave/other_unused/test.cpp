#include <fstream>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include<string.h>
#include <iostream>
#define NUM_WORDS 1000000
using namespace std;
char** listOfWords;
int main(int argc, char* argv[])
{
   if(argc==1)
   {
      printf("usage: ./a.out <name of file>\n");
      return 0;
   }
   std::ifstream infile(argv[1]);
   //std::unordered_map<std::string, unsigned int> bandwidth;
   std::string line;//, ip1, ip2;
   //size_t pos1, pos2;
   //unsigned int bytes;

   int i=0;
   listOfWords = (char**) malloc (sizeof(char*)*NUM_WORDS);
   while (std::getline(infile, line))
   {
      listOfWords[i]=(char*)malloc(sizeof(char)*strlen(line));
      if(listOfWords[i]!=NULL)
      {
         strcpy(listOfWords[i],line);
      }
      else
      {
         printf("problem\n");
         break;
      }
      i++;
         //cout<<line<<endl;
      //printf("%s",line);
      //pos1 = line.find('\t');
      //pos2 = line.find('\t', pos1 + 1);
      //ip1 = line.substr(pos1 + 1, pos2 - pos1 - 1);
      //pos1 = line.find('\t', pos2 + 1);
     // pos2 = line.find('\t', pos1 + 1);
      //ip2 = line.substr(pos1 + 1, pos2 - pos1 - 1);
      //pos1 = line.find('\t', pos1 + 1);
      //pos1 = line.find('\t', pos1 + 1);
      //pos1 = line.find('\t', pos1 + 1);
      //pos2 = line.find('\t', pos1 + 1);
      //bytes = atoi(line.substr(pos1 + 1, pos2 - pos1 - 1).c_str());

      //bandwidth[ip1] += bytes;
      //bandwidth[ip2] += bytes;
   }

   return 0;
}
