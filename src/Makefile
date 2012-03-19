all: ttraffd

ttraffd: main.o ttraff.o functions.o ttgraph.o httpd.o
	g++ -lmicrohttpd -lsqlite3 main.o ttraff.o functions.o ttgraph.o httpd.o -o ttraffd

main.o: main.cpp main.h
	g++ -c main.cpp

ttraff.o: ttraff.cpp ttraff.h
	g++ -c ttraff.cpp

functions.o: functions.cpp functions.h
	g++ -c functions.cpp

ttgraph.o: ttgraph.cpp ttgraph.h
	g++ -c ttgraph.cpp

httpd.o: httpd.cpp httpd.h
	g++ -c httpd.cpp

clean:
	rm -fr *.o ttraffd

install: ttraffd
	install -m 755 ttraffd /usr/local/bin/ttraffd
	mkdir -p /var/ttraff/db
