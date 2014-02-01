void iodb_getNumAddresses(int id, int *numAddresses){

   MYSQL *conn;
   MYSQL_STMT *stmt;
   char *sql;

   // Bind variables
   MYSQL_BIND param[1], result[1];

   int myId, myNumAddresses;
   my_bool is_null[1];

   sql = "select LENGTH from DICT LIMIT ?";

   // Open Database
   openDB(&conn);

   // Allocate statement handler
   stmt = mysql_stmt_init(conn);

   if (stmt == NULL) {
      print_error(conn, "Could not initialize statement handler");
      return;
   }

   // Prepare the statement
   if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0) {
      print_stmt_error(stmt, "Could not prepare statement");
      return;
   }

   // Initialize the result column structures
   memset (param, 0, sizeof (param)); /* zero the structures */
   memset (result, 0, sizeof (result)); /* zero the structures */

   // Init param structure
   // Select
   param[0].buffer_type     = MYSQL_TYPE_LONG;
   param[0].buffer         = (void *) &myId;
   param[0].is_unsigned    = 0;
   param[0].is_null         = 0;
   param[0].length         = 0;

   // Result
   result[0].buffer_type     = MYSQL_TYPE_LONG;
   result[0].buffer         = (void *) &myNumAddresses;
   result[0].is_unsigned    = 0;
   result[0].is_null         = &is_null[0];
   result[0].length         = 0;

   // Bind param structure to statement
   if (mysql_stmt_bind_param(stmt, param) != 0) {
      print_stmt_error(stmt, "Could not bind parameters");
      return;
   }

   // Bind result
   if (mysql_stmt_bind_result(stmt, result) != 0) {
      print_stmt_error(stmt, "Could not bind results");
      return;
   }

   // Set bind parameters
   myId            = id;

   // Execute!!
   if (mysql_stmt_execute(stmt) != 0) {
      print_stmt_error(stmt, "Could not execute statement");
      return;
   }

   if (mysql_stmt_store_result(stmt) != 0) {
      print_stmt_error(stmt, "Could not buffer result set");
      return;
   }

   // Init data
   (*numAddresses) = 0;

   // Fetch
   if(mysql_stmt_fetch (stmt) == 0){
      (*numAddresses) = myNumAddresses;
   }

   // Deallocate result set
   mysql_stmt_free_result(stmt); /* deallocate result set */

   // Close the statement
   mysql_stmt_close(stmt);

   // Close Database
   closeDB(conn);

}
