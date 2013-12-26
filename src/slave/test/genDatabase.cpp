#include <stdio.h>
#include <mysql/mysql.h>

using namespace std;
#define userId "root"
#define hostname "localhost"
#define password "6f141H64TyPi"

int main()
{
	// --------------------------------------------------------------------
	// Connect to the database

	MYSQL      *MySQLConRet;
	MYSQL      *MySQLConnection = NULL;

        //char* hostName= "localhost";

        //ec2
        //string userId   = "sunjay";
        //string password = "751DW6MegYf1";

	MySQLConnection = mysql_init( NULL );
	printf("Compile: g++ -o genDatabase genDatabase.cpp 'mysql_config --cflags' 'mysql_config --libs'\n");
	try
	{
		MySQLConRet = mysql_real_connect( MySQLConnection,
				hostname,userId,password, 
				NULL,  // No database specified
				0, 
				NULL,
				0 );

		if ( MySQLConRet == NULL )
                {
                   printf("An error occurred\n");
		      return 1;
                }
                      //throw  FFError( (char*) mysql_error(MySQLConnection) );

		printf("MySQL Connection Info: %s \n", mysql_get_host_info(MySQLConnection));
		printf("MySQL Client Info: %s \n", mysql_get_client_info());
		printf("MySQL Server Info: %s \n", mysql_get_server_info(MySQLConnection));

	}
	catch (int e )
        {
           printf("An error occurred\n");
           return 1;
        }


        // --------------------------------------------------------------------
        //  Create database

        if (mysql_query(MySQLConnection, "CREATE DATABASE DWPA")) 
        {
           printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
           return(1);
        }

        // --------------------------------------------------------------------
        //  Now that database has been created set default database

        if (mysql_query(MySQLConnection, "USE DWPA") )
        {
           printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
           return(1);
        }

        // --------------------------------------------------------------------
        //  Create table and records
        if (mysql_query(MySQLConnection, "CREATE TABLE DICT (ID INT UNSIGNED NOT NULL AUTO_INCREMENT,WORD VARCHAR(25) NOT NULL UNIQUE,LENGTH TINYINT UNSIGNED NOT NULL,FILENAME VARCHAR(20),CHECK (LENGTH>7), PRIMARY KEY (ID))") )
        {
           printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
           return(1);
        }
        //readFile(argv[2]);
        //call function to do inserting
        //insertIntoDB(MySQLConnection,word,strlen(word),filename);

        // --------------------------------------------------------------------
        // Close datbase connection

        mysql_close(MySQLConnection);

        return 0;
}
int insertIntoDB(MYSQL *MySQLConnection,char* word,int length,char* filename)
{

   char q[1024];
   sprintf(q,"INSERT INTO dict(WORD,LENGTH,FILENAME) VALUES ('%s',%d,'%s')",word,length,filename);
   if (mysql_query(MySQLConnection,q) )
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      return(1);
   }
}
