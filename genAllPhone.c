#include<stdio.h>
#include<stdlib.h>
int main(int argc, char* argv[])
{

   char ans;
   int num[10];
   int a,b,c,d,e,i,j,k,l,q,s;
   num[0]=8;
   num[1]=0;
   num[2]=5;


   FILE *fp=NULL;
   ans='0';
   if(argc==1)
   {
      printf("output to stdout? (y | n)\n");
   }
   else if(argc==2)
   {
      printf("output to %s? (y | n) \n ",argv[1]);
      fp = fopen(argv[1], "w");
   }
   else if(argc>2)
   {
      printf("output undefined. exitting\n");
      return 0;
   }

   ans=getchar();
   getchar();
   printf("You said: %c\n\n",ans);
   printf("only printing phone numbers with area code (%d%d%d).\n",num[0],num[1],num[2]);
   printf("Is that okay? (y | n)\n");
   if(ans=='y' || ans=='Y') 
   {

      printf("only printing phone numbers with area code (%d%d%d).\n",num[0],num[1],num[2]);
      printf("Is that okay? (y | n)\n");
   ans=getchar();
   getchar();
      printf("You said: %c\n\n",ans);

      if(ans=='y' || ans=='Y') 
      {
         for(i=0;i<10;i++)
         {
            num[3]=i;
            for(j=0;j<10;j++)
            { 
               num[4]=j;
               for(k=0;k<10;k++)
               {
                  num[5]=k;
                  for(l=0;l<10;l++)
                  {
                     num[6]=l;
                     for(d=0;d<10;d++)
                     {
                        num[7]=d;
                        for(q=0;q<10;q++)
                        {
                           num[8]=q;
                           for(a=0;a<10;a++)
                           {
                              num[9]=a;
                              if(fp)
                              {
                                 fprintf(fp,"%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
                              }
                              else
                              {
                                 printf("%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
                              }
                           }	 
                        }
                     }
                  }
               }
            }
         }
      }
      else
      {

         printf("Printing ALL phone numbers\n"); 
         printf("Is that okay? (y | n)\n");
         ans=getchar();
         getchar();
         printf("You said: %c\n\n",ans);
         if(ans=='y' || ans=='Y') 
         {

            for(b=0;b<10;b++)
            {
               num[0]=b;
               for(c=0;c<10;c++)
               { 
                  num[1]=c;
                  for(e=0;e<10;k++)
                  {
                     num[2]=e;
                     for(i=0;i<10;i++)
                     {
                        num[3]=i;
                        for(j=0;j<10;j++)
                        { 
                           num[4]=j;
                           for(k=0;k<10;k++)
                           {
                              num[5]=k;
                              for(l=0;l<10;l++)
                              {
                                 num[6]=l;
                                 for(d=0;d<10;d++)
                                 {
                                    num[7]=d;
                                    for(q=0;q<10;q++)
                                    {
                                       num[8]=q;
                                       for(a=0;a<10;a++)
                                       {
                                          num[9]=a;
                                          if(fp)
                                          {
                                             fprintf(fp,"%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
                                          }
                                          else
                                          {
                                             printf("%d%d%d%d%d%d%d%d%d%d\n",num[0],num[1],num[2],num[3],num[4],num[5],num[6],num[7],num[8],num[9]); 
                                          }
                                       }	 
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
}
