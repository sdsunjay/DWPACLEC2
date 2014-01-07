#include <stdio.h>
#include <sqlite3.h>
#include <iostream>
using namespace std;
#define BUFFER_SIZE 256

#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;

#include <cstring>

const int MAX_CHARS_PER_LINE = 25;
const int MIN_CHARS_PER_LINE = 8;
using namespace std;

// read an entire line into memory
char buf[MAX_CHARS_PER_LINE];
int verbose=1;
int read_from_file(char* name_of_file,sqlite3_stmt* stmt)
{
   //the number of words we have skipped due to length.
   int skipped = 0;
   //the number of words greater than 7 and less than 63
   int wordsAdded = 0;
   // create a file-reading object
   ifstream fin;
   fin.open(name_of_file); // open a file
   if (!fin.good())
   {
      fprintf(stderr,"File not found\n");
      return wordsAdded;
   }
   // read each line of the file
   while (!fin.eof())
   {
      fin.getline(buf,12);
      int length = strnlen(buf,12);

      /* Test length of word.  IEEE 802.11i indicates the passphrase must be
       * at least 8 characters in length, and no more than 63 characters in
       * length.
       */
      if (length < MIN_CHARS_PER_LINE || length > MAX_CHARS_PER_LINE) {
	 if (verbose) {
	    fprintf(stderr, "Invalid passphrase length: %s (%d).\n",buf, length);
	    fprintf(stderr, "Skipped a word\n");
	 } 
	 /*
	  *                           * Output message to user*/
	 skipped++;
	 //continue;
      } else {
	 /* This word is good, increment the words tested counter */
	 wordsAdded++;

	 sqlite3_bind_text(stmt, 1, buf, -1, SQLITE_TRANSIENT);
	 sqlite3_bind_int(stmt, 2, length);
	 sqlite3_step(stmt);     /* Execute the SQL Statement */
	 sqlite3_clear_bindings(stmt);   /* Clear bindings */
	 sqlite3_reset(stmt);        /* Reset VDBE */
	 if(verbose)
	 {
	    printf("Length: %d\tWord: %s\n",length,buf);
	 }
      }
   }
   printf("Skipped %d words due to length\n",skipped);
   printf("Added %d words to the database\n",wordsAdded);
   return wordsAdded;
}
int select_from_db(sqlite3 * db)
{    
   char * selectQuery;
   sqlite3_stmt * stmt;
   int i;
   int row = 0;
   int s = 0;
   const unsigned char * text;
   //LIMIT 10,5
   //equivalent to LIMIT <count> OFFSET <skip>
   selectQuery = "select WORD from DICT1 LIMIT 10,5";
   if(verbose)
   {
      printf("query: ");
      printf("%s",selectQuery);
      printf("\n");
   }
   sqlite3_stmt *selectStmt;
   sqlite3_prepare(db, selectQuery, strlen(selectQuery)+1, &selectStmt, NULL);
   while (1) {
      s = sqlite3_step (selectStmt);
      if (s == SQLITE_ROW) {
	 text  = sqlite3_column_text (selectStmt, 0);
	 printf ("%s\n",text);
      }
      else if (s == SQLITE_DONE) {
	 break;
      }
      else if(s == SQLITE_BUSY ) {
	 fprintf (stderr, "Busy.\n");
      }
      else if(s == SQLITE_MISUSE )
      {
	 fprintf(stderr,"Misue\n");
	 return 0;
      }
      else {
	 fprintf (stderr, "Failed.\n");
	 return 0;
      }
   }
   return 0;
}
int main(int argc, char **argv){
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;

   sqlite3_stmt *createStmt;
   sqlite3_stmt* stmt;
   if( argc!=3 ){
      fprintf(stderr, "Usage: %s DATABASE <INPUT_FILE>\n", argv[0]);
      return(1);
   }
   rc = sqlite3_open(argv[1], &db);
   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
   }
   string createQuery = "CREATE TABLE IF NOT EXISTS DICT1 (WORD CHAR(20) NOT NULL UNIQUE PRIMARY KEY,LENGTH TINYINT UNSIGNED NOT NULL,CHECK (LENGTH>7));";
   cout << "Creating Table Statement" << endl;
   sqlite3_prepare(db, createQuery.c_str(), createQuery.size(), &createStmt, NULL);
   cout << "Stepping Table Statement" << endl;
   if (sqlite3_step(createStmt) != SQLITE_DONE) cout << "Didn't Create Table!" << endl;

   sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &zErrMsg);
   sqlite3_exec(db, "PRAGMA journal_mode = MEMORY", NULL, NULL, &zErrMsg);
   char sSQL [BUFFER_SIZE] = "\0";
   char* tail = 0;
   sprintf(sSQL, "INSERT INTO DICT1(WORD,LENGTH) VALUES (@buf, @length)");
   sqlite3_prepare(db,  sSQL, BUFFER_SIZE, &stmt, NULL);
   sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
   read_from_file(argv[2],stmt);
   select_from_db(db);
   /* 
      string insertQuery = "INSERT INTO DICT1 (WORD,LENGTH) VALUES('8050000000',10);";
      sqlite3_stmt *insertStmt;
      cout << "Creating Insert Statement" << endl;
      sqlite3_prepare(db, insertQuery.c_str(), insertQuery.size(), &insertStmt, NULL);
      cout << "Stepping Insert Statement" << endl;
      if (sqlite3_step(insertStmt) != SQLITE_DONE) cout << "Didn't Insert Item!" << endl;  
      */

   sqlite3_close(db);
   return 0;
}
