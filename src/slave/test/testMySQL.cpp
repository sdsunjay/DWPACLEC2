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

#define hostName "localhost"
#define userId "root"
#define password "6f141H64TyPi"
#define DB_NAME "DWPA"

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

//mysql db connection
int mysqlSelection(MYSQL* MySQLConnection, int limit, int offset)
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
      char query[100];
      //sprintf(query,"SELECT WORD FROM DICT LIMIT 100;");
      sprintf(query, "SELECT DICT.WORD FROM DICT LIMIT %d OFFSET %d;",limit,offset);
      printf("Query is: %s\n",query);
      // string sqlSelStatement = "SELECT WORD FROM DICT LIMIT 1000000000";
      mysqlStatus = mysql_query(MySQLConnection,query);

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



      // Print column headers

      mysqlFields = mysql_fetch_fields(mysqlResult);

      for(int jj=0; jj < numFields; jj++)
      {
      printf("%s\t",mysqlFields[jj].name);
      }
      printf("\n");

      // print query results
      int ii;
      ii=0;
      while(mysqlRow = mysql_fetch_row(mysqlResult)) // row pointer in the result set
      {
         for(ii=0; ii < numFields; ii++)
         {
            printf("%s\t", mysqlRow[ii] ? mysqlRow[ii] : "NULL");  // Not NULL then print
            //needs to be fixed
            //printf("%d\t", mysqlRow[1] ? mysqlRow[1] : "NULL");  // Not NULL then print
         }
         printf("\n");
      }

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

//not local connection to db
//http://stackoverflow.com/questions/16424828/how-to-connect-mysql-database-using-c


//local connection
//MySQLConnection is now a global, so no params passed
int mysqlConnect(MYSQL *MySQLConnection)
{

   // --------------------------------------------------------------------
   //     // Connect to the database

   MySQLConnection = mysql_init( NULL );

   try
   {
      if(!mysql_real_connect(MySQLConnection, // MySQL obeject
               hostName, // Server Host
               userId,// User name of user
               password,// Password of the database
               DB_NAME,// Database name
               0,// port number
               NULL,// Unix socket  ( for us it is Null )
               0))
      {
         throw FFError( (char*) mysql_error(MySQLConnection) );
      }
      printf("MySQL Connection Info: %s \n", mysql_get_host_info(MySQLConnection));
      printf("MySQL Client Info: %s \n", mysql_get_client_info());
      printf("MySQL Server Info: %s \n", mysql_get_server_info(MySQLConnection));

   }
   catch (FFError e )
   {
      printf("%s\n",e.Label.c_str());
      return 1;
   }
   return 0;
}

int main()
{

   MYSQL* MySQLConnection = NULL;
   //connect
   int flag;
   flag=mysqlConnect(MySQLConnection);


   if(MySQLConnection!=NULL)
   {
      //query
      int limit=100;
      int offset=0;
      mysqlSelection(MySQLConnection,limit,offset);
      // --------------------------------------------------------------------
      // Close datbase connection
      mysql_close(MySQLConnection);
   }
   else
   {
      fprintf(stderr,"MySQLConnection is null\n");
      return 1;
   }
   //mysql_close(MySQLConnection);
   return 0;
}
