#include <stdio.h>
#include <stdlib.h>     /* calloc, exit, free */
#include <mysql.h>
#include <iostream>
#include <sys/stat.h>
//#include <boost/asio.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
#define userId "root"
#define hostname "localhost"
#define password "1fYegM6Dw157"
//#define password "6f141H64TyPi"

//ec2
//#define userId "sunjay"
//#define password "751DW6MegYf1"
int checkSize(char* name)
{

   struct stat st;
   unsigned long size;
   stat(name, &st);
   size = st.st_size;
   if(size > 1073741822 )
   {
      fprintf(stderr,"Size of %s is greater than 1GB\n");
      fprintf(stderr,"%lu MB not supported\n",(size/(1024*1024*1024)));
      return 0;
   } 

   return 1;
}

int main(int argc, char* argv[] )
{

   char* query;
   char c;
   char* path;
   char u;
   char* realPath;
   char* realPath1;
   if(geteuid()!=0)
   {
      fprintf(stderr,"Must run as root or segfault WILL occur!\n");
      return 1;

   }
   if(argc==1)
   {
      fprintf(stderr,"You only entered %s\n",argv[0]);
      fprintf(stderr,"Usage: genDatabase <name_of_file> [<name_of_file>]\n");
      printf("Proceed anyway? (y | n)");
      c = getchar();
      getchar();
      if(c=='n')
	 return 1;
   }
   else if(argc==2)
   {
      //makeSpace(path,200);
      fprintf(stderr,"You entered %s %s\n",argv[0],argv[1]);
      realPath=argv[1];
      //realPath=realpath(argv[1],path);	
      printf("Absolute path %s\n",realPath);
      //free(path);
   }
   else if(argc==3)
   {
      //makeSpace(path,200);
      fprintf(stderr,"You entered %s %s %s\n",argv[0],argv[1],argv[2]);
      realPath1=argv[2];
      //realPath1=realpath(argv[2],path);	
      printf("Absolute path %s\n",realPath1);
      //free(path);
   }
   else
   {
      fprintf(stderr,"unsupported number of args\n");
      return 1;
   }

   // --------------------------------------------------------------------
   // Connect to the database


   MYSQL      *MySQLConRet;
   MYSQL      *MySQLConnection = NULL;

   MySQLConnection = mysql_init( NULL );
   printf("Compile: g++ -o genDatabase genDatabase.cpp  `mysql_config --cflags --libs`\n");
   try
   {
      MySQLConRet=mysql_real_connect(MySQLConnection,hostname,userId,password,NULL,0,NULL,0);
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
      printf("Unable to connect to DB. Quitting.\n");
      return 1;
   }


   // --------------------------------------------------------------------
   //  Create database
   printf("CREATE DATABASE IF NOT EXISTS DWPA\n");
   if (mysql_query(MySQLConnection, "CREATE DATABASE IF NOT EXISTS DWPA")) 
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      printf("Proceed anyway? (y | n)");
      c = getchar();
      getchar();
      if(c=='n')
	 return(1);
   }

   //## FLUSH Priviledges
   printf("FLUSH PRIVILEGES\n");	
   if(mysql_query(MySQLConnection, "FLUSH PRIVILEGES") )
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      printf("Proceed anyway? (y | n)");
      c = getchar();
      getchar();
      if(c=='n')
	 return(1);
   }

   //we need to creat cal poly at local host and calpoly for remote connections
   //see below for more explanation.
   //http://stackoverflow.com/questions/10236000/allow-all-remote-connections-mysql
   //CREATE USER ##
   printf("Create users? (y | n)\n");

   u = getchar();
   getchar();
   if(u=='y')
   {
      printf("CREATE USER 'calpoly'@'localhost' IDENTIFIED BY '7751DW6MegYf1'\n");
      if(mysql_query(MySQLConnection, "CREATE USER 'calpoly'@'localhost' IDENTIFIED BY '7751DW6MegYf1'")) 
      {
	 printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	 printf("Proceed anyway? (y | n)");
	 c = getchar();
	 getchar();
	 if(c=='n')
	    return(1);
      }
      printf("GRANT ALL PRIVILEGES ON *.* TO 'calpoly'@'localhost'\n");

      //## GRANT PERMISSIONS ##
      if(mysql_query(MySQLConnection, "GRANT ALL PRIVILEGES ON *.* TO 'calpoly'@'localhost'"))	
      {
	 printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	 printf("Proceed anyway? (y | n)");
	 c = getchar();
	 getchar();
	 if(c=='n')
	    return(1);
      }
      //## FLUSH Priviledges
      printf("FLUSH PRIVILEGES\n");	
      if(mysql_query(MySQLConnection, "FLUSH PRIVILEGES") )
      {
	 printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	 return(1);
      }
      printf("CREATE USER 'calpoly'@'%%' IDENTIFIED BY '751DW6MegYf1'\n");
      if(mysql_query(MySQLConnection, "CREATE USER 'calpoly'@'%' IDENTIFIED BY '7751DW6MegYf1'")) 
      {
	 printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	 printf("Proceed anyway? (y | n)");
	 c = getchar();
	 getchar();
	 if(c=='n')
	    return(1);
      }
      printf("GRANT ALL PRIVILEGES ON *.* TO 'calpoly'@'%'\n");
      //## GRANT PERMISSIONS ##
      if(mysql_query(MySQLConnection, "GRANT ALL PRIVILEGES ON *.* TO 'calpoly'@'%'"))	
      {
	 printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	 printf("Proceed anyway? (y | n)");
	 c = getchar();
	 getchar();
	 if(c=='n')
	    return(1);
      }
      //## FLUSH Priviledges
      printf("FLUSH PRIVILEGES\n");	
      if(mysql_query(MySQLConnection, "FLUSH PRIVILEGES") )
      {
	 printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	 printf("Proceed anyway? (y | n)");
	 c = getchar();
	 getchar();
	 if(c=='n')
	    return(1);
      }
   }
   // --------------------------------------------------------------------
   //  Now that database has been created set default database
   printf("USE DWPA\n");
   if (mysql_query(MySQLConnection, "USE DWPA") )
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      printf("Proceed anyway? (y | n)");
      c = getchar();
      getchar();
      if(c=='n')
	 return(1);
   }

   // --------------------------------------------------------------------
   //  Create tables 
   printf("CREATE TABLE IF NOT EXISTS DICT1 (WORD CHAR(20) NOT NULL UNIQUE,PRIMARY KEY (WORD)) CHARACTER SET ascii\n");
   if (mysql_query(MySQLConnection, "CREATE TABLE IF NOT EXISTS DICT1 (WORD CHAR(20) NOT NULL UNIQUE, PRIMARY KEY (WORD)) CHARACTER SET ascii") )
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      printf("Proceed anyway? (y | n)");
      c = getchar();
      getchar();
      if(c=='n')
	 return(1);
   }
   printf("CREATE TABLE IF NOT EXISTS DICT (WORD CHAR(20) NOT NULL UNIQUE,PRIMARY KEY (WORD)) CHARACTER SET ascii\n");
   if (mysql_query(MySQLConnection, "CREATE TABLE IF NOT EXISTS DICT (WORD CHAR(20) NOT NULL UNIQUE,PRIMARY KEY (WORD)) CHARACTER SET ascii") )
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      printf("Proceed anyway? (y | n)");
      c = getchar();
      getchar();
      if(c=='n')
	 return(1);
   }
   printf("SET sql_mode = 'STRICT_TRANS_TABLES'");
   if (mysql_query(MySQLConnection, "SET sql_mode = 'STRICT_TRANS_TABLES'") )
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      return(1);
   }
   printf("SET sql_mode = 'STRICT_ALL_TABLES'\n");
   if (mysql_query(MySQLConnection, "SET sql_mode = 'STRICT_ALL_TABLES'") )
   {
      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
      return(1);
   }

   //insert
   //  #original for 8055.dic

   if(argv[1])
   {
      printf("SET GLOBAL max_allowed_packet = 1024 * 1024 * 1024\n");
      if (mysql_query(MySQLConnection,"SET GLOBAL max_allowed_packet = 1024 * 1024 * 1024") )
      {
	 printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	 return(1);
      }


      query = (char*) calloc (500,sizeof(char));
      if (query==NULL)
      {	
	      fprintf(stderr,"Error with calloc\n");
	      exit (1);
      } 
      if (checkSize(realPath))
      {
	      printf("Inserting from %s\n",realPath);
	      sprintf (query, "LOAD DATA LOCAL INFILE '%s' IGNORE INTO TABLE DICT CHARACTER SET ascii LINES TERMINATED BY '\n' (@word) SET WORD=@word", realPath);
	      //sprintf (query, "LOAD DATA LOCAL INFILE '/home/ec2-user/DWPACLEC2/src/slave/test/small' IGNORE INTO TABLE DICT1 CHARACTER SET ascii LINES TERMINATED BY '\n' (@word) SET WORD=@word,LENGTH = CHAR_LENGTH(word)");
	      printf("%s\n",query);
	      if (mysql_query(MySQLConnection, query) )
	      {
		      printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
		      return(1);
	      }
      }
      else
      {
	      printf("Did not insert\n");
      }
      free(query);
   }

   if(argv[2])
   {
   query = (char*) calloc (500,sizeof(char));
   if (query==NULL)
   {	
      fprintf(stderr,"Error with calloc\n");
      exit (1);
   } 

	   if(checkSize(realPath1))
	   {
		   printf("Inserting from %s\n",realPath1);
		   sprintf(query,"LOAD DATA LOCAL INFILE '%s' IGNORE INTO TABLE DICT1 CHARACTER SET ascii LINES TERMINATED BY '\n' (@word) SET WORD=@word",realPath1);
		   printf("%s\n",query);	
		   if (mysql_query(MySQLConnection,query))
		   {
			   printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
			   return(1);
		   }
	   }
	   else
	   {
		   printf("Did not insert\n");
	   }
	   free(query);
   }
   //readFile(argv[2]);
   //call function to do inserting
   //insertIntoDB(MySQLConnection,word);

   // --------------------------------------------------------------------
   // Close datbase connection
   printf("Closing Database connections\n");
   mysql_close(MySQLConnection);

   return 0;
}
int insertIntoDB(MYSQL *MySQLConnection,char* word)
{

	char q[1024];
	sprintf(q,"INSERT INTO dict(WORD) VALUES ('%s')",word);
	if (mysql_query(MySQLConnection,q) )
	{
		printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
		return(1);
	}
}
