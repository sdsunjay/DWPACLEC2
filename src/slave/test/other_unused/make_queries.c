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
#include <mysql/mysql.h>
#include <mysql/my_sys.h>
#include <mysql/my_global.h>
char *words;
/*USED to connect to the database*/
 MYSQL* conn;
/* Prototypes */
//void usage(char *message);
int nextword();
void connection();

int main(void)
{
   //time stuff
   struct timeval start, end;
   float elapsed = 0;

   gettimeofday(&start, 0);
   int i;
   i=0;

   connection();
   nextword();
   gettimeofday(&end, 0);

   /* print time elapsed */
   if (end.tv_usec < start.tv_usec) {
      end.tv_sec -= 1;
      end.tv_usec += 1000000;
   }
   end.tv_sec -= start.tv_sec;
   end.tv_usec -= start.tv_usec;
   elapsed = end.tv_sec + end.tv_usec / 1000000.0;

   printf("\n%d passphrases tested in %.2f seconds:  %.2f passphrases/second\n", i, elapsed, i / elapsed);

   mysql_close(conn);
   return 0;
}

int nextword()
{

   char word[128];
   MYSQL_RES *result;
   MYSQL_ROW row;
   MYSQL_FIELD *mysqlFields;
   int num_fields,num_rows;
   int i;
   int mysqlStatus;

  // mysql_query(conn, "SELECT * FROM DICT LIMIT 1024");
   
   mysqlStatus=mysql_query(conn, "SELECT * FROM DICT LIMIT 1024");
   if(mysqlStatus)
   {
      fprintf(stderr,"error\n");
      return 1;
    }
   result = mysql_store_result(conn);

   mysqlFields = mysql_fetch_fields(result);
   int jj;
   for(jj=0; jj < num_fields; jj++)
   {
      printf("%s\t",mysqlFields[jj].name);
   }
   printf("\n");

   if (result)  // there are rows
   {
      // # of rows in the result set
      num_rows = mysql_num_rows(result);
      num_fields = mysql_num_fields(result);
      while ((row = mysql_fetch_row(result)))
         // if((row = mysql_fetch_row(result)))
      {
         for(i = 0; i < num_fields; i++)
         {
            printf("%s ", row[i] ? row[i] : "NULL");
         }
         printf("\n");

         //word=row[0];
         // printf("Word: %s  (NO NEWLINE IN PRINTF)",word);
      }
   }
   //mysql_free_result(result);
   //mysql_close(conn);
   /* Remove newline */
   //word[strlen(word) - 1] = '\0';


   return (strlen(word));

}

void connection()
{

   //MYSQL *conn;
   //MYSQL_RES *res;
   //MYSQL_ROW row;

   char *server = "localhost";
   char *user = "root";
   //set the password for mysql server here
   char *password = "6f141H64TyPi"; /* set me first */
   char *database = "DWPA";

   conn = mysql_init(NULL);

   /* Connect to database */
   if (!mysql_real_connect(conn, server,
            user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "%s\n", mysql_error(conn));
      exit(1);
      //return 0;
   }

   /* send SQL query */
   if (mysql_query(conn, "show tables")) {
      fprintf(stderr, "ERROR: %s\n", mysql_error(conn));
      //return 0;
      exit(1);
   } 

   /*create table*/
   // mysql_query(conn, "CREATE TABLE writers(name VARCHAR(25))");

   // mysql_query(conn, "INSERT INTO writers VALUES('Leo Tolstoy')");
   //mysql_query(conn, "INSERT INTO writers VALUES('Jack London')");
   //mysql_query(conn, "INSERT INTO writers VALUES('Honore de Balzac')");
   //mysql_query(conn, "INSERT INTO writers VALUES('Lion Feuchtwanger')");
   //mysql_query(conn, "INSERT INTO writers VALUES('Emile Zola')");
   //return conn;


   //res = mysql_use_result(conn);

   /* output table name */
   //printf("MySQL Tables in mysql database:\n");
   //while ((row = mysql_fetch_row(res)) != NULL)
   // printf("%s \n", row[0]);

   /* close connection */
   //mysql_free_result(res);
   //mysql_close(conn);
}
