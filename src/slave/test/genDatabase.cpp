#include <stdio.h>
#include <mysql/mysql.h>

using namespace std;
#define userId "root"
#define hostname "localhost"
#define password "1fYegM6Dw157"
//#define password "6f141H64TyPi"

//ec2
//#define userId "sunjay"
//#define password "751DW6MegYf1"
int main()
{
	// --------------------------------------------------------------------
	// Connect to the database

	MYSQL      *MySQLConRet;
	MYSQL      *MySQLConnection = NULL;

		char c;
	MySQLConnection = mysql_init( NULL );
	printf("Compile: g++ -o genDatabase genDatabase.cpp 'mysql_config --cflags' 'mysql_config --libs'\n");
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
		printf("An error occurred\n");
		return 1;
	}


	// --------------------------------------------------------------------
	//  Create database
	printf("CREATE DATABASE DWPA\n");
	if (mysql_query(MySQLConnection, "CREATE DATABASE DWPA")) 
	{
		printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
		printf("Proceed anyway? (y | n)");
		c = getchar();
		getchar();
		if(c=='n')
			return(1);
	}
	//CREATE USER ##
	printf("CREATE USER 'calpoly'@'66.214.64.87' IDENTIFIED BY '751DW6MegYf1'\n");
	if(mysql_query(MySQLConnection, "CREATE USER 'calpoly'@'66.214.64.87' IDENTIFIED BY '7751DW6MegYf1'")) 
	{
		printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
		printf("Proceed anyway? (y | n)");
		c = getchar();
		getchar();
		if(c=='n')
			return(1);
	}
	printf("GRANT SELECT ON *.* TO 'calpoly'@'66.214.64.87'\n");
	//## GRANT PERMISSIONS ##
	if(mysql_query(MySQLConnection, "GRANT SELECT ON *.* TO 'calpoly'@'66.214.64.87'"))	
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
	// --------------------------------------------------------------------
	//  Now that database has been created set default database
	printf("USE DWPA\n");
	if (mysql_query(MySQLConnection, "USE DWPA") )
	{
		printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
		return(1);
	}

	// --------------------------------------------------------------------
	//  Create tables 
	printf("CREATE TABLE DICT1 (WORD CHAR(20) NOT NULL UNIQUE,LENGTH TINYINT UNSIGNED NOT NULL,CHECK (LENGTH>7), PRIMARY KEY (WORD)) CHARACTER SET ascii\n");
	if (mysql_query(MySQLConnection, "CREATE TABLE DICT1 (WORD CHAR(20) NOT NULL UNIQUE,LENGTH TINYINT UNSIGNED NOT NULL,CHECK (LENGTH>7), PRIMARY KEY (WORD)) CHARACTER SET ascii") )
	{
		printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
		return(1);
	}
	printf("CREATE TABLE DICT (WORD CHAR(20) NOT NULL UNIQUE,LENGTH TINYINT UNSIGNED NOT NULL,CHECK (LENGTH>7), PRIMARY KEY (WORD)) CHARACTER SET ascii\n");
	if (mysql_query(MySQLConnection, "CREATE TABLE DICT (WORD CHAR(20) NOT NULL UNIQUE,LENGTH TINYINT UNSIGNED NOT NULL,CHECK (LENGTH>7), PRIMARY KEY (WORD)) CHARACTER SET ascii") )
	{
		printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
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
	printf("Inserting 805.dic\n");
	if (mysql_query(MySQLConnection, "LOAD DATA LOCAL INFILE '/home/ec2-user/DWPACLEC2/805.dic' IGNORE INTO TABLE DICT1 CHARACTER SET ascii LINES TERMINATED BY '\n' (@word) SET WORD=@word,LENGTH = CHAR_LENGTH(word)") )
	{
		printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
		return(1);
	}
	/*
	   printf("Inserting word.dic\n");
	   if (mysql_query(MySQLConnection, "LOAD DATA LOCAL INFILE '/home/ec2-user/DWPACLEC2/src/slave/test/word.dic' IGNORE INTO TABLE DICT CHARACTER SET ascii LINES TERMINATED BY '\n' (@word) SET WORD=@word,LENGTH = CHAR_LENGTH(wor    d)") )
	   {
	   printf("Error %u: %s\n", mysql_errno(MySQLConnection), mysql_error(MySQLConnection));
	   return(1);
	   }
	   */
	//readFile(argv[2]);
	//call function to do inserting
	//insertIntoDB(MySQLConnection,word,strlen(word),filename);

	// --------------------------------------------------------------------
	// Close datbase connection
	printf("Closing Database connections\n");
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
