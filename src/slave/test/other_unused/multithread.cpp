extern "C"
{
#include <pthread.h>
#include <unistd.h>
}
#include <iostream>


using namespace std ;

void  * function1(void * argument);
void  * function2(void * argument);

int main( void )
{
   pthread_t t1, t2 ; // declare 2 threads.
   pthread_create( &t1, NULL, function1,NULL); // create a thread running function1
   pthread_create( &t2, NULL, function2,NULL); // create a thread running function2

   // Because all created threads are terminated when main() finishes, we have
   // to give the threads some time to finish. Unfortunately for function1, main()
   // will give only 1 second, but function1 needs at least 2 seconds. So function1 will
   // probably be terminated before it can finish. This is a BAD way to manage threads.
   sleep(1);

   return 0;
}

void * function1(void * argument)
{
   cout << " hello " << endl ;
   sleep(2); // fall alseep here for 2 seconds...
   return 0;
}

void * function2(void * argument)
{
   cout << " world "  << endl ;
   return 0;
}
//local connection
//MySQLConnection is now a global, so no params passed
int mysqlConnect()
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


   //connect
   int flag;
   flag=mysqlConnect();
   mysql_close(MySQLConnection);
   return 0;
}
