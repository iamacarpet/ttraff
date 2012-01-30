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
#include <string.h>
// Our database stuff...
#include <sqlite3.h>
#include <sstream>
// Our includes...
#include "functions.h"
#include "main.h"
#include "ttraff.h"

#define COL_WIDTH 16            /* single column width */

using namespace std;

template <class T> string to_string (const T& t){
	stringstream ss;
	ss << t;
	return ss.str();
}

string do_ttgraph(int month, int year, string iface){

	char *next;
	char var[80];
	
	struct tm *currtime;
	long tloc;
	
	time(&tloc);            // get time in seconds since epoch
	currtime = localtime(&tloc);    // convert seconds to date structure

	string outputStream;

	unsigned int days;
	int wd;
	int i = 0;
	string months[12] = { "January", "Februrary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	unsigned long rcvd[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned long sent[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned long max = 5, smax = 5, f = 1;
	unsigned long totin = 0;
	unsigned long totout = 0;

	if (month < 1 || month > 12)
		return outputStream;

	days = daysformonth(month, year);
	wd = weekday(month, 1, year);   // first day in month (mon=0, tue=1, ..., sun=6)

	for (int tk = 0; tk < 31; tk++){
		string sqlDay = "SELECT in_dev, out_dev FROM ttraff WHERE day = ";
		sqlDay += to_string(tk+1);
		sqlDay += " AND month = ";
		sqlDay += to_string(month);
		sqlDay += " AND year = ";
		sqlDay += to_string(year);
		sqlDay += " AND iface = '";
		sqlDay += iface;
		sqlDay += "';";

		char *zErrMsg;
		char **result;
		int rc;
		int nrow,ncol;

		pthread_mutex_lock( &db_lock );
		rc = sqlite3_get_table(db_conn, sqlDay.c_str(), &result, &nrow, &ncol, &zErrMsg);
		pthread_mutex_unlock( &db_lock );

		if (rc == SQLITE_OK){
			//logWrite("Got day OK");
			//logWrite(to_string(nrow));
			if (nrow == 1){
				//logWrite("We have one day row, processing...");

				rcvd[tk] = atoi(result[2]);
				sent[tk] = atoi(result[3]);

				totin = totin + atoi(result[2]);
				totout = totout + atoi(result[3]);

				if (rcvd[tk] > max)
					max = rcvd[tk];
				if (sent[tk] > max)
					max = sent[tk];

			} else {
				//logWrite("We don't have a one day row...");
			}
			if ( year == (currtime->tm_year + 1900) ){
				//logWrite("Got the right year...");
				if ( month == (currtime->tm_mon + 1) ){
					//logWrite("Got the right month...");
					//logWrite(to_string(tk2));
					//logWrite(to_string(currtime->tm_mday));
					if ( (tk+1) == currtime->tm_mday ){
						//logWrite("Got the right day...");
						rcvd[tk] = rcvd[tk] + get_indiff(iface);
						totin = totin + get_indiff(iface);
						sent[tk] = sent[tk] + get_outdiff(iface);
						totout = totout + get_outdiff(iface);
					}
				}
			}
		} else {
			logWrite("Query error - Check exists...");
			logWrite(zErrMsg);
		}

		sqlite3_free_table(result);
		sqlite3_free(zErrMsg);
	}

	while (max > smax) {
		if (max > (f * 5))
			smax = f * 10;
		if (max > (f * 10))
			smax = f * 25;
		if (max > (f * 25))
			smax = f * 50;
		f = f * 10;
	}

	string incom = "Incoming";
	string outcom = "Outgoing";

	string monthname = months[month-1];

	outputStream = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
	outputStream += "<html>\n";
	outputStream += "<head>\n";
	outputStream += "<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml\" />\n";
	outputStream += "<title>Infitialis Traffic Graph</title>\n";

	outputStream += "<script type=\"text/javascript\">\n";
	outputStream += "//<![CDATA[\n";
	outputStream += "function Show(label) {\n";
	outputStream += "document.getElementById(\"label\").innerHTML = label;\n";
	outputStream += "}\n";
	outputStream += "//]]>\n";
	outputStream += "</script>\n";

	outputStream += "<style type=\"text/css\">\n\n";
	outputStream += "#t-graph {position: relative; width: ";
	outputStream += to_string(days * COL_WIDTH);
	outputStream += "px; height: 300px;\n";
	outputStream += "  margin: 1.1em 0 3.5em; padding: 0;\n";
	outputStream += "  border: 1px solid gray; list-style: none;\n";
	outputStream += "  font: 9px Tahoma, Arial, sans-serif;}\n";
	outputStream += "#t-graph ul {margin: 0; padding: 0; list-style: none;}\n";
	outputStream += "#t-graph li {position: absolute; bottom: 0; width: ";
	outputStream += to_string(COL_WIDTH);
	outputStream += "px; z-index: 2;\n";
	outputStream += "  margin: 0; padding: 0;\n";
	outputStream += "  text-align: center; list-style: none;}\n";
	outputStream += "#t-graph li.day {height: 298px; padding-top: 2px; border-right: 1px dotted #C4C4C4; color: #AAA;}\n";
	outputStream += "#t-graph li.day_sun {height: 298px; padding-top: 2px; border-right: 1px dotted #C4C4C4; color: #E00;}\n";
	outputStream += "#t-graph li.bar {width: 4px; border: 1px solid; border-bottom: none; color: #000;}\n";
	outputStream += "#t-graph li.bar p {margin: 5px 0 0; padding: 0;}\n";
	outputStream += "#t-graph li.rcvd {left: 3px; background: #228B22;}\n"; // set rcvd bar colour here (green)
	outputStream += "#t-graph li.sent {left: 8px; background: #CD0000;}\n"; // set sent bar colour here (red)

	for (i = 0; i < days - 1; i++) {
		outputStream += "#t-graph #d";
		outputStream += to_string(i+1);
		outputStream += "{left: ";
		outputStream += to_string(i * COL_WIDTH);
		outputStream += "px;}\n";
	}
	
	outputStream += "#t-graph #d";
	outputStream += to_string(days);
	outputStream += "{left: ";
	outputStream += to_string((days - 1) * COL_WIDTH);
	outputStream += "px; border-right: none;}\n";

	outputStream += "#t-graph #ticks {width: ";
	outputStream += to_string(days * COL_WIDTH);
	outputStream += "px; height: 300px; z-index: 1;}\n";

	outputStream += "#t-graph #ticks .tick {position: relative; border-bottom: 1px solid #BBB; width: ";
	outputStream += to_string(days * COL_WIDTH);
	outputStream += "px;}\n";

	outputStream += "#t-graph #ticks .tick p {position: absolute; left: 100%%; top: -0.67em; margin: 0 0 0 0.5em;}\n";
	outputStream += "#t-graph #label {width: 500px; bottom: -20px;  z-index: 1; font: 12px Tahoma, Arial, sans-serif; font-weight: bold;}\n";
	outputStream += "</style>\n";
	outputStream += "</head>\n\n";
	outputStream += "<body>\n";
	outputStream += "<ul id=\"t-graph\">\n";

	for (i = 0; i < days; i++) {
		outputStream += "<li class=\"day";
		outputStream += (wd % 7) == 6 ? "_sun" : "";
		outputStream += "\" id=\"d";
		outputStream += to_string(i + 1);
		outputStream += "\" ";
		wd++;

		outputStream += "onmouseover=\"Show(\'";
		outputStream += monthname;
		outputStream += " ";
		outputStream += to_string(i+1);
		outputStream += ", ";
		outputStream += to_string(year);
		outputStream += " (";
		outputStream += incom;
		outputStream += ": ";
		outputStream += to_string(rcvd[i]);
		outputStream += " MB / ";
		outputStream += outcom;
		outputStream += ": ";
		outputStream += to_string(sent[i]);
		outputStream += "MB)\')\" ";

		outputStream += "onmouseout=\"Show(\'";
		outputStream += monthname;
		outputStream += ", ";
		outputStream += to_string(year);
		outputStream += " (";
		outputStream += incom;
		outputStream += ": ";
		outputStream += to_string(totin);
		outputStream += " MB / ";
		outputStream += outcom;
		outputStream += ": ";
		outputStream += to_string(totout);
		outputStream += "MB)\')\" ";

		outputStream += ">";
		outputStream += to_string(i+1);
		outputStream += "\n";

		outputStream += "<ul>\n";
		outputStream += "<li class=\"rcvd bar\" style=\"height: ";
		outputStream += to_string(rcvd[i] * 300 / smax);
		outputStream += "px;\"><p></p></li>\n";

		outputStream += "<li class=\"sent bar\" style=\"height: ";
		outputStream += to_string(sent[i] * 300 / smax);
		outputStream += "px;\"><p></p></li>\n";

		outputStream += "</ul>\n";
		outputStream += "</li>\n";
	}

	outputStream += "<li id=\"ticks\">\n";
	for (i = 5; i; i--){    // scale
		outputStream += "<div class=\"tick\" style=\"height: 59px;\"><p>";
		outputStream += to_string(smax * i / 5);
		outputStream += (smax > 10000) ? " " : "&nbsp;";
		outputStream += "</p></div>\n";
	}

	outputStream += "</li>\n\n";

	outputStream += "<li id=\"label\">\n";
	outputStream += monthname;
	outputStream += " ";
	outputStream += to_string(year);
	outputStream += " (";
	outputStream += incom;
	outputStream += ": ";
	outputStream += to_string(totin);
	outputStream += " MB / ";
	outputStream += outcom;
	outputStream += ": ";
	outputStream += to_string(totout);
	outputStream += "MB)\n";

	outputStream += "</li>\n";

	outputStream += "</ul>\n\n";
	outputStream += "</body>\n";
	outputStream += "</html>\n";

	return outputStream;
}

string do_stylesheet(){
	string stylesheet = ".normaltext {  color: #515151;  font-size: 12px;  font-family: verdana, arial; }  .smalltext {  color: #515151;  font-size: 10px;  font-family: verdana, arial; }  .headblue {  font-weight: bold;  color: #457baf;  font-size: 14px;  font-family: helvetica, arial, verdana; }  .headblue2 {  font-weight: bold;  color: #457baf;  font-size: 18px;  font-family: helvetica, arial, verdana; }  .headblue3 {  font-weight: bold;  color: #457baf;  font-size: 25px;  font-family: helvetica, arial, verdana; }  .headblue a {  font-weight: bold;  color: #457baf;  font-size: 14px;  font-family: helvetica, arial, verdana;  text-decoration: none; }  .headblack1 {  font-weight: normal;  color: #000000;  font-size: 12px;  font-family: helvetica, arial, verdana; }  .headblue1 {  font-weight: normal;  color: #457baf;  font-size: 12px;  font-family: helvetica, arial, verdana; }  .nopermission {  font-weight: bold;  color: red;  font-size: 12px;  font-family: Helvetica, arial, verdana;  text-align: center; }  .headerhr {  color: #bdbdbd; }  .normalblue {  font-weight: normal;  color: #457baf;  font-size: 12px;  font-family: helvetica, arial, verdana;   line-height: 20px; }  .boldblue12 {  font-weight: bold;  color: #457baf;  font-size: 12px;  font-family: helvetica, arial, verdana; }  .boldblack12 {  font-weight: bold;  color: #000000;  font-size: 12px;  font-family: helvetica, arial, verdana; }  .normalblack {  font-weight: normal;  color: #000000;  font-size: 12px;  font-family: helvetica, arial, verdana;   line-height: 20px; }  .normalblackitalic {  font-weight: normal;  color: #000000;  font-size: 12px;  font-family: helvetica, arial, verdana;   line-height: 20px;   font-style: italic; }  .normalblack a {  font-weight: normal;  color: #000000;  font-size: 12px;  font-family: helvetica, arial, verdana;   line-height: 20px; }  .warning12 {  font-weight: normal;  color: red;  font-size: 12px;  font-family: helvetica, arial, verdana; }  .ok12 {  font-weight: normal;  color: #20840c;  font-size: 12px;  font-family: helvetica, arial, verdana; }  .info12 {  font-weight: normal;  color: #ff6500;  font-size: 12px;  font-family: helvetica, arial, verdana; }  .normalblack10 {  font-weight: normal;  color: #000000;  font-size: 10px;  font-family: helvetica, arial, verdana; }  tr.boxesodd {  background-color: #FFFFFF;  font-weight: normal;  color: #000000;  font-size: 12px;  font-family: helvetica, arial, verdana; }  tr.boxeseven {  background-color: #EDEDED;  font-weight: normal;  color: #000000;  font-size: 12px;  font-family: helvetica, arial, verdana; }  div#rmadiv_allocate {  display:none; }  .button {  border-color: #bdbdbd;  border-style: solid;  font-family:    verdana;  font-size:    11px;  color:      #457baf;  background-color: #ffffff;  border-width:   thin; }  table#box_warning{  border-width: 1px;  padding: 5px;  border-color: #c3c3c3;  border-style: dashed;  background-color: #fffcdb; }  td#box_info{  background-color: #EDEDED;  border-color: #868e8c;  border-style: solid;  border-width: thin;  padding:   5px; }  td#box_info2{  background-color: #bde5ed;  border-color: #868e8c;  border-style: solid;  border-width: thin;  padding:   5px; }  .box_warning{  border-width: 1px;  padding: 5px;  border-color: #c3c3c3;  border-style: dashed;  background-color: #fffcdb;   color: red;   font-family: verdana, arial;   font-size: 10px; }  .box_ok{  border-width: 1px;  padding: 5px;  border-color: #c3c3c3;  border-style: dashed;  background-color: #fffcdb;   color: green;   font-family: verdana, arial;   font-size: 10px; }  table#box_blue{  border-width: 1px;  padding: 5px;  border-color: #cccccc;  border-style: dashed;  background-color: #e0ffe0; }  table#box_red{  border-width: 1px;  padding: 5px;  border-color: #cccccc;  border-style: dashed;  background-color: #ffdede; }  .normalformbox {  font-weight: normal;  color: #000000;  font-size: 10px;  font-family: helvetica, arial, verdana;   text-align: center; }  .normalformboxleft {  font-weight: normal;  color: #000000;  font-size: 10px;  font-family: helvetica, arial, verdana;   text-align: left; }  .normalformselect {  font-weight: normal;  color: #000000;  font-size: 10px;  font-family: helvetica, arial, verdana;   text-align: left; }  .normalformtextarea {  font-weight: normal;  color: #000000;  font-size: 10px;  font-family: helvetica, arial, verdana; }";
	return stylesheet;
}

string do_mainPage(){
	string mainPage = "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\"><TITLE>301 Moved</TITLE></HEAD><BODY><H1>301 Moved</H1>The document has moved <A HREF=\"/tgraph.cgi\">here</A>.</BODY></HTML>";
	return mainPage;
}

string do_graphPage(){
	string graphPage;
	struct tm *currtime;
	long tloc;
	int day;
	int month;
	int year;
	
	time(&tloc);
	currtime = localtime(&tloc);

	day = currtime->tm_mday;
	month = currtime->tm_mon + 1;   // 1 - 12
	year = currtime->tm_year + 1900;
	
	graphPage = "<html>\n";
		graphPage += "<head>\n";
			graphPage += "<title>Infitialis Traffic Grapher</title>\n";
			graphPage += "<link rel=\"stylesheet\" href=\"/style/1.css\" type=\"text/css\">\n";
			graphPage += "<script type=\"text/javascript\">\n";
				graphPage += "//<![CDATA[\n";
				graphPage += "function changeGraph(){\n";
					graphPage += "var month = document.getElementById('month');\n";
					graphPage += "var year = document.getElementById('year');\n";
					graphPage += "var iface = document.getElementById('iface');\n";
				
					graphPage += "var f = document.getElementById('graph');\n";
				
					graphPage += "var monthInt = month.options[month.selectedIndex].value;\n";
					graphPage += "var yearInt = year.options[year.selectedIndex].value;\n";
				
					graphPage += "var daysInMonth = (30 + (((monthInt & 9) == 8) || ((monthInt & 9) == 1)) - (monthInt == 2) - (!(((yearInt % 4) == 0) && (((yearInt % 100) != 0) || ((yearInt % 400) == 0))) && (monthInt == 2)));\n";
				
					graphPage += "var width = (daysInMonth * ";
					graphPage += to_string(COL_WIDTH);
					graphPage += ") + 30;\n";
				
					graphPage += "f.width = width;\n";
					graphPage += "f.src = \"/ttgraph.cgi?month=\" + month.options[month.selectedIndex].value + \"&year=\" + year.options[year.selectedIndex].value + \"&iface=\" + iface.options[iface.selectedIndex].value;\n";
				graphPage += "}\n";
				graphPage += "//]]>\n";
			graphPage += "</script>\n";
		graphPage += "</head>\n";
		graphPage += "<body>\n";
			graphPage += "<table width=\"100%\" cellpadding=\"2\" border=\"0\">\n";
				graphPage += "<tr>\n";
					graphPage += "<td width=\"150\" class=\"noprint\">&nbsp;</td>\n";
					graphPage += "<td bgcolor=\"#bdbdbd\">\n";
						graphPage += "<table bgcolor=\"#FFFFFF\" width=\"100%\" height=\"100%\">\n";
							graphPage += "<tr>\n";
								graphPage += "<td valign=\"top\">\n";
									graphPage += "<br />\n";

									graphPage += "<table width=\"800\" cellspacing=\"1\" cellpadding=\"1\" border=\"0\" align=\"center\">\n";
										graphPage += "<tr>\n";
											graphPage += "<td class=\"headblue\" align=\"center\">\n";
												graphPage += "<iframe id=\"graph\" scrolling=\"no\" src=\"/ttgraph.cgi?month=";
												graphPage += to_string(month);
												graphPage += "&year=";
												graphPage += to_string(year);
												graphPage += "&iface=eth0\" width=\"";
												graphPage += to_string( (daysformonth(month, year) * COL_WIDTH) + 30 );
												graphPage += "\" height=\"350\" frameborder=\"0\" type=\"text/html\"></iframe>\n";
											graphPage += "</td>\n";
										graphPage += "</tr>\n";

										graphPage += "<tr>\n";
											graphPage += "<td>\n";
												graphPage += "<br />\n";
											graphPage += "</td>\n";
										graphPage += "</tr>\n";
									
										graphPage += "<tr>\n";
											graphPage += "<td valign=\"top\">\n";
												graphPage += "<table width=\"600\" cellpadding=\"1\" cellspacing=\"1\" id=\"box_blue\" align=\"center\">\n";
													graphPage += "<tr>\n";
														graphPage += "<td class=\"normaltext\">\n";
															graphPage += "<br />\n";
															graphPage += "<div align=\"center\">\n";
																graphPage += "Month : <select id=\"month\">\n";
																	graphPage += "<option value=\"1\"";
																	if (month == 1){
																		graphPage += " selected";
																	}
																	graphPage += ">January</option>\n";
																	graphPage += "<option value=\"2\"";
																	if (month == 2){
																		graphPage += " selected";
																	}
																	graphPage += ">February</option>";
																	graphPage += "<option value=\"3\"";
																	if (month == 3){
																		graphPage += " selected";
																	}
																	graphPage += ">March</option>\n";
																	graphPage += "<option value=\"4\"";
																	if (month == 4){
																		graphPage += " selected";
																	}
																	graphPage += ">April</option>\n";
																	graphPage += "<option value=\"5\"";
																	if (month == 5){
																		graphPage += " selected";
																	}
																	graphPage += ">May</option>\n";
																	graphPage += "<option value=\"6\"";
																	if (month == 6){
																		graphPage += " selected";
																	}
																	graphPage += ">June</option>\n";
																	graphPage += "<option value=\"7\"";
																	if (month == 7){
																		graphPage += " selected";
																	}
																	graphPage += ">July</option>\n";
																	graphPage += "<option value=\"8\"";
																	if (month == 8){
																		graphPage += " selected";
																	}
																	graphPage += ">August</option>\n";
																	graphPage += "<option value=\"9\"";
																	if (month == 9){
																		graphPage += " selected";
																	}
																	graphPage += ">September</option>\n";
																	graphPage += "<option value=\"10\"";
																	if (month == 10){
																		graphPage += " selected";
																	}
																	graphPage += ">October</option>\n";
																	graphPage += "<option value=\"11\"";
																	if (month == 11){
																		graphPage += " selected";
																	}
																	graphPage += ">November</option>\n";
																	graphPage += "<option value=\"12\"";
																	if (month == 12){
																		graphPage += " selected";
																	}
																	graphPage += ">December</option>\n";
																graphPage += "</select>\n";
																graphPage += "&nbsp;\n";
																graphPage += "Year : <select id=\"year\">\n";
																	graphPage += "<option value=\"2010\">2010</option>\n";
																	for (int gh = (year - 2010); gh > 0; gh--){
																		graphPage += "<option value=\"";
																		graphPage += to_string(gh+2010);
																		graphPage += "\"";
																		if ( (gh+2010) == year ){
																			graphPage += " selected";
																		}
																		graphPage += ">";
																		graphPage += to_string(gh+2010);
																		graphPage += "</option>";
																	}
																graphPage += "</select>\n";
																graphPage += "&nbsp;\n";
																graphPage += "Interface : <select id=\"iface\">\n";
																
																char *zErrMsg;
																char **result;
																int rc;
																int nrow,ncol;
																
																string sqliface = "SELECT iface FROM ttraff GROUP BY iface ORDER BY iface ASC";

																pthread_mutex_lock( &db_lock );
																rc = sqlite3_get_table(db_conn, sqliface.c_str(), &result, &nrow, &ncol, &zErrMsg);
																pthread_mutex_unlock( &db_lock );
																
																if (rc == SQLITE_OK){
																	for (int ty = 0; ty < nrow; ty++){
																		graphPage += "<option value=\"";
																		graphPage += result[ty+1];
																		graphPage += "\"";
																		if (strcmp(result[ty+1], "eth0") == 0){
																			graphPage += " selected";
																		}
																		graphPage += ">";
																		graphPage += result[ty+1];
																		graphPage += "</option>\n";
																	}
																} else {
																	logWrite("Query error - List Interfaces...");
																	logWrite(zErrMsg);
																}
																
																graphPage += "</select>\n";
																graphPage += "&nbsp;\n";
																graphPage += "<input type=\"submit\" onClick=\"changeGraph();\" class=\"button\" value=\"View\" />\n";
															graphPage += "</div>\n";
															graphPage += "<br />\n";
														graphPage += "</td>\n";
													graphPage += "</tr>\n";
												graphPage += "</table>\n";

											graphPage += "</td>\n";
										graphPage += "</tr>\n";
									graphPage += "</table>\n";
								graphPage += "</td>\n";
							graphPage += "</tr>\n";
							graphPage += "<tr>\n";

								graphPage += "<td>\n";
									graphPage += "<br>\n";
									graphPage += "<table width=\"100%\">\n";
										graphPage += "<tr>\n";
											graphPage += "<td align=\"left\" class=\"smalltext\">&nbsp;Infitialis Traffic Grapher</td>\n";
											graphPage += "<td align=\"right\" class=\"smalltext\">Page Generated on ";
											if (day < 10){
												graphPage += "0";
											}
											graphPage += to_string(day);
											graphPage += "/";
											if (month < 10){
												graphPage += "0";
											}
											graphPage += to_string(month);
											graphPage += "/";
											graphPage += to_string(year);
											graphPage += " ";
											int hour;
											if (currtime->tm_hour > 12){
												hour = currtime->tm_hour - 12;
											} else {
												hour = currtime->tm_hour;
											}
											if (hour < 10){
												graphPage += "0";
											}
											graphPage += to_string(hour);
											graphPage += ":";
											if (currtime->tm_min < 10){
												graphPage += "0";
											}
											graphPage += to_string(currtime->tm_min);
											graphPage += ":";
											if (currtime->tm_sec < 10){
												graphPage += "0";
											}
											graphPage += to_string(currtime->tm_sec);
											if (currtime->tm_hour > 12){
												graphPage += "PM";
											} else {
												graphPage += "AM";
											}
											graphPage += "&nbsp;</td>\n";
										graphPage += "</tr>\n";
									graphPage += "</table>\n";
								graphPage += "</td>\n";
							graphPage += "</tr>\n";
						graphPage += "</table>\n";
					graphPage += "</td>\n";
					graphPage += "<td width=\"150\" class=\"noprint\">&nbsp;</td>\n";
				graphPage += "</tr>\n";
			graphPage += "</table>\n";
		graphPage += "</body>\n";
	graphPage += "</html>";
	
	return graphPage;
}