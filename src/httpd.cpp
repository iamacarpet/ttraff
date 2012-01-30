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
// MicroHTTPd Stuff...
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
// Our headers
#include "functions.h"
#include "main.h"
#include "ttgraph.h"

#define PORT 8888

using namespace std;

int answer_to_connection (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls){

	//cout << "Processing response..." << endl;
	
	string page;
	
	struct MHD_Response *response;
	int ret;
	
	if (strcmp(url, "/ttgraph.cgi") == 0){
		int month;
		int year;
		string iface;
		if (MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "iface")){
			iface = MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "iface");
			if (atoi(MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "month"))){
				month = atoi(MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "month"));
				if (atoi(MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "year"))){
					year = atoi(MHD_lookup_connection_value (connection, MHD_GET_ARGUMENT_KIND, "year"));
					page = do_ttgraph(month, year, iface);
				} else {
					page = "<html><body>&nbsp;</body></html>";
				}
			} else {
				page = "<html><body>&nbsp;</body></html>";
			}
		} else {
			page = "<html><body>&nbsp;</body></html>";
		}
	} else if (strcmp(url, "/") == 0){
		page = do_graphPage();
	} else if (strcmp(url, "/style/1.css") == 0){
		page = do_stylesheet();
	} else if (strcmp(url, "/tgraph.cgi") == 0){
		page = do_graphPage();
	} else {
		page = "<html><body>&nbsp;</body></html>";
	}

	//cout << "Sending response..." << endl;
	response = MHD_create_response_from_data (page.size(), (void*)page.c_str(), MHD_NO, MHD_YES);
	MHD_add_response_header (response, "Content-Type", "text/html");
	//cout << "........" << endl;
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);

	return ret;

}

void *httpd_main(void *arg){
	struct MHD_Daemon *daemon;
	
	logWrite("Starting httpd daemon...");
	
	daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &answer_to_connection, NULL, MHD_OPTION_END);
	
	if (NULL == daemon){
		logWrite("httpd failure");
		return (void*)1;
	}
	
	logWrite("Get connections...");
	
	getchar ();

	logWrite("Stop daemon...");
	
	MHD_stop_daemon (daemon);
	
	return (void*)0;
}