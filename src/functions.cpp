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
// Our database stuff...
#include <sqlite3.h>
#include <sstream>

using namespace std;

FILE *logFile;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

void openLog(){
	//pthread_mutex_lock( &log_lock );
	if ((logFile = fopen("/var/log/ttraff", "w")) != NULL) {
		return;
	} else {
		exit(0);
	}
	//pthread_mutex_unlock( &log_lock );
}

void closeLog(){
	fclose(logFile);
}

void logWrite(std::string logData){
	long tloc;
	time(&tloc);
	char t[(CHAR_BIT * sizeof tloc + 2) / 3 + 1];
	sprintf(t, "%lu", tloc);
	string logString = t;
	logString += " ttraff: ";
	logString += logData;
	logString += "\n";
	cout << logString << endl;
	//pthread_mutex_lock( &log_lock );
	fputs(logString.c_str(), logFile);
	fflush(logFile);
	//pthread_mutex_unlock( &log_lock );
}

std::string trim (std::string str){
	string temp;
	for (unsigned int i = 0; i < str.length(); i++)
	if (str[i] != ' ') temp += str[i];
	return temp;
}

unsigned int daysformonth(unsigned int month, unsigned int year){
	return (30 + (((month & 9) == 8) || ((month & 9) == 1)) - (month == 2) - (!(((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0))) && (month == 2)));
}

int weekday(int month, int day, int year){
	int ix, tx, vx;

	switch (month) {
		case 2 :
		case 6 :
			vx = 0;
			break;
		case 8 :
			vx = 4;
			break;
		case 10 :
			vx = 8;
			break;
		case 9 :
		case 12 :
			vx = 12;
			break;
		case 3 :
		case 11 :
			vx = 16;
			break;
		case 1 :
		case 5 :
			vx = 20;
			break;
		case 4 :
		case 7 :
			vx = 24;
			break;
	}
	if (year > 1900) // 1900 was not a leap year
		year -= 1900;
	ix = ((year - 21) % 28) + vx + (month > 2); // take care of February
	tx = (ix + (ix / 4)) % 7 + day; // take care of leap year
	return (tx % 7);
}