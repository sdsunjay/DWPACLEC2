#include <stdio.h>
#include <stdlib.h>

void function(int* test)
{
   *test=10;
}
int main(void)
{
   int* test;
   *test=1;
   printf("TEST: %d\n\n",*test);
   //function(test);
   printf("TEST: %d\n\n",*test);
   return 0;
}
