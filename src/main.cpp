// Include the Linux Thread Library.
#include <pthread.h>
// Include the Linux Input/Output Library.
#include <iostream>
#include <fstream>
// Include Strings Library.
#include <string>
#include <time.h>
#include <map>
// Some crap...
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sstream>
#include <sqlite3.h>
// Microhttpd Stuff
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
// Our headers
#include "functions.h"
#include "ttraff.h"
#include "httpd.h"

#define PORT 8888

// Define where the database will be stored.
#define DATABASE_FILE "/var/ttraff/db/ttraff.infodb"

// Define the initial SQL query to run, to make sure the database is ok.
#define INITIAL_SQL "CREATE TABLE IF NOT EXISTS ttraff (day int NOT NULL, month int NOT NULL, year int NOT NULL, iface varchar(20) NOT NULL, in_dev BIGINT NOT NULL, out_dev BIGINT NOT NULL, PRIMARY KEY (day,month,year,iface));"

using namespace std;

sqlite3 *db_conn;
pthread_mutex_t db_lock = PTHREAD_MUTEX_INITIALIZER;

int main(){
	/* Our process ID and Session ID */
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		return 0;
	}
	/* If we got a good PID, then we can exit the parent process. */
	if (pid > 0) {
		return 1;
	}

	/* Change the file mode mask */
	umask(0);

	/* Open any logs here */
	//cout << "Opening the log..." << endl;
	openLog();
	//cout << "Done opening the log..." << endl;
	//logWrite("Starting daemon...");

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log the failure */
		return 0;
	}

	/* Change the current working directory */
	if ((chdir("/var/ttraff/")) < 0) {
		return 0;
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// Define error holding variable...
	char *zErrMsg = 0;
	// Define holding variable...
	int rc;

	// Open SQLite connection...
	//pthread_mutex_lock( &db_lock );
	rc = sqlite3_open(DATABASE_FILE, &db_conn);
	if(rc){
		logWrite("Can't open database: ");
		logWrite(sqlite3_errmsg(db_conn));
		sqlite3_close(db_conn);
		exit(1);
	}
	//pthread_mutex_unlock( &db_lock );

	// Run the initial SQL query...
	//pthread_mutex_lock( &db_lock );
	rc = sqlite3_exec(db_conn, INITIAL_SQL, NULL, 0, &zErrMsg);
	if(rc != SQLITE_OK){
		logWrite(zErrMsg);
		sqlite3_free(zErrMsg);
	}
	//pthread_mutex_unlock( &db_lock );
		
	int status;
	int varTruth = 1;
		
	// Define threads...
	pthread_t TTRAFF_SRV;
	
	// Create each thread...
	//logWrite("Creating httpd thread...");
	logWrite("Creating ttraff thread...");
	pthread_create(&TTRAFF_SRV, NULL, ttraff_main, &varTruth);
	
	struct MHD_Daemon *daemon;
	logWrite("Starting daemon...");
	daemon = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END);
	
	if (NULL == daemon){
		logWrite("httpd failure");
	}
	
	logWrite("Getting stuff...");
	
	getchar ();
	
	pthread_join(TTRAFF_SRV, (void **) &status);
	if (status != 1){
		//logWrite("Web UI Server Thread Error");
		exit(1);
	}
	
	MHD_stop_daemon (daemon);
	
	closeLog();
	return 0;
}