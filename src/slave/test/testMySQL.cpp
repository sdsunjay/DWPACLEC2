#include <stdio.h>
#include <ctime>
#include <mysql.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pcap.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

class FFError
{
   public:
      std::string    Label;

      FFError( ) { Label = (char *)"Generic Error"; }
      FFError( char *message ) { Label = message; }
      ~FFError() { }
      inline const char*   GetMessage  ( void )   { return Label.c_str(); }
};
using namespace std;

//time stuff
//globals
struct timeval start, end;
float elapsed = 0;



void do_time_stuff(int flag)
{
   if(flag==1)
   {
      gettimeofday(&start, 0);
   }
   else
   {
      gettimeofday(&end, 0);
      /* print time elapsed */
      if (end.tv_usec < start.tv_usec) {
         end.tv_sec -= 1;
         end.tv_usec += 1000000;
      }
      end.tv_sec -= start.tv_sec;
      end.tv_usec -= start.tv_usec;
      elapsed = end.tv_sec + end.tv_usec / 1000000.00;
      printf("%.3f seconds elapsed for query\n",elapsed);
   }
   // printf("\n%d passphrases tested in %.2f seconds:  %.2f passphrases/second\n", i, elapsed, i / elapsed);
}

int mysqlSelection(int limit, int offset,MYSQL *MySQLConnection)
{


   // --------------------------------------------------------------------
   // Perform a SQL SELECT and retrieve data

   int             mysqlStatus = 0;
   MYSQL_RES      *mysqlResult = NULL;
   MYSQL_ROW       mysqlRow;
   MYSQL_FIELD    *mysqlFields;
   my_ulonglong    numRows;
   unsigned int    numFields;


   do_time_stuff(1);

   try
   {
      
      //sprintf(sqlSelStatement, "%s",listOfWords[i]->begin);
      //string sqlSelStatement = "SELECT WORD,LENGTH FROM DICT LIMIT 1000000000";
      string sqlSelStatement = "SELECT WORD FROM DICT LIMIT 1000000000";
      mysqlStatus = mysql_query( MySQLConnection, sqlSelStatement.c_str() );

      if (mysqlStatus)
         throw FFError( (char*)mysql_error(MySQLConnection) );
      else
         mysqlResult = mysql_store_result(MySQLConnection); // Get the Result Set

      if (mysqlResult)  // there are rows
      {
         // # of rows in the result set
         numRows = mysql_num_rows(mysqlResult);

         // # of Columns (mFields) in the latest results set
         numFields = mysql_field_count(MySQLConnection);

         // Returns the number of columns in a result set specified
         numFields = mysql_num_fields(mysqlResult);

         printf("Number of rows=%u  Number of fields=%u \n",numRows,numFields);
      }
      else
      {
         printf("Result set is empty");
      }


      /*
      // Print column headers

      mysqlFields = mysql_fetch_fields(mysqlResult);

      for(int jj=0; jj < numFields; jj++)
      {
      printf("%s\t",mysqlFields[jj].name);
      }
      printf("\n");*/
/*
      // print query results
      while(mysqlRow = mysql_fetch_row(mysqlResult)) // row pointer in the result set
      {
         //for(int ii=0; ii < numFields; ii++)
         //{
         printf("%s\t", mysqlRow[0] ? mysqlRow[0] : "NULL");  // Not NULL then print
         //needs to be fixed
         //printf("%d\t", mysqlRow[1] ? mysqlRow[1] : "NULL");  // Not NULL then print
         //}
         printf("\n");
      }*/

      if(mysqlResult)
      {
         mysql_free_result(mysqlResult);
         mysqlResult = NULL;
      }
   }
   catch ( FFError e )
   {
      printf("%s\n",e.Label.c_str());
      mysql_close(MySQLConnection);
      return 1;
   }

   do_time_stuff(2);
}
int main()
{
   // --------------------------------------------------------------------
   //     // Connect to the database

   MYSQL      *MySQLConRet;
   MYSQL      *MySQLConnection = NULL;

   string hostName = "localhost";
   string userId   = "root";
   string password = "6f141H64TyPi"; /* set me first */
   string DB = "DWPA";

   MySQLConnection = mysql_init( NULL );

   try
   {
      MySQLConRet = mysql_real_connect( MySQLConnection,
            hostName.c_str(), 
            userId.c_str(), 
            password.c_str(), 
            DB.c_str(), 
            0, 
            NULL,
            0 );

      if ( MySQLConRet == NULL )
         throw FFError( (char*) mysql_error(MySQLConnection) );

      printf("MySQL Connection Info: %s \n", mysql_get_host_info(MySQLConnection));
      printf("MySQL Client Info: %s \n", mysql_get_client_info());
      printf("MySQL Server Info: %s \n", mysql_get_server_info(MySQLConnection));

   }
   catch ( FFError e )
   {
      printf("%s\n",e.Label.c_str());
      return 1;
   }


   int limit=100;
   int offset=0;
   mysqlSelection(limit,offset,MySQLConnection);

   // --------------------------------------------------------------------
   // Close datbase connection
   mysql_close(MySQLConnection);

   return 0;
}
