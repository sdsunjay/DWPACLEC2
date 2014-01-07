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

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}


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
      // read an entire line into memory
      char buf[MAX_CHARS_PER_LINE];
      fin.getline(buf, MAX_CHARS_PER_LINE);
      int length = strnlen(buf,MAX_CHARS_PER_LINE);

      /* Test length of word.  IEEE 802.11i indicates the passphrase must be
       * at least 8 characters in length, and no more than 63 characters in
       * length.
       */
      if (length < MIN_CHARS_PER_LINE || length > MAX_CHARS_PER_LINE) {
	 /*if (verbose) {
	   printf("Invalid passphrase length: %s (%d).\n",
	   passphrase, (int)strlen(passphrase));
	   } */
	 /*
	  *                           * Output message to user*/
	 // fprintf(stderr, "Skipped a word\n");
	 skipped++;
	 //continue;
      } else {
	 /* This word is good, increment the words tested counter */
	 wordsAdded++;

	 sqlite3_bind_text(stmt, 1, buf, -1, SQLITE_TRANSIENT);
	// sqlite3_bind_int(stmt, 2, 10, -1, SQLITE_TRANSIENT);
	 sqlite3_step(stmt);     /* Execute the SQL Statement */
	 sqlite3_clear_bindings(stmt);   /* Clear bindings */
	 sqlite3_reset(stmt);        /* Reset VDBE */
	 // printf("Added a word\n");
	 printf("Length: %d\tWord: %s\n",length,buf);
      }
   }
   printf("Skipped %d words due to length\n",skipped);
   printf("Added %d words to the database\n",wordsAdded);
   return wordsAdded;
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
   sprintf(sSQL, "INSERT INTO DICT1(WORD,LENGTH) VALUES (@RT, @BR)");
   sqlite3_prepare(db,  sSQL, BUFFER_SIZE, &stmt, NULL);
   sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
   read_from_file(argv[2],stmt);
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
