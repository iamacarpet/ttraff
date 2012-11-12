# ttraff Network Bandwidth Logger

The ttraff tool is an program to log how much bandwidth is moving through each interface and log it out to a database.

The traffic is logger per date and per interface.

An embedded webserver is also included to view graphs of the recorded data.

## WARNING

PLEASE NOTE: This software was written by me to help build a legal case against my old ISP - I won't go into details about that.

But it was written in a short period of time to be run on a very low RAM, x86 embedded Linux router running Ubuntu.

I usually like to value the security provisions taken in my code, but I have very little experiance of C++ and can almost guarantee there are lots of securit
y flaws with this code.

It didn't bother me as I was using it on a home network locked down to my IP - But please, DON'T use this on a production system unless you can verify there
is no exploitable code.

## Features

  * Traffic Bandwidth Totals Logging
  * SQLite Storage Backend
  * Embedded webserver
  * Pretty graphs

## Installation

On Ubuntu 10.04, make sure you install: apt-get install libsqlite3-dev libmicrohttpd-dev

Then cd into the src folder, make && make install

## Documentation & Usage

Run the program with the ttraffd command.

Visit it at http://serverip:8888/

View traffic graphs - That's about all there is to it.

## Open Source Projects Used

This was origionally based on the code from the DD-WRT project along with some other libs and SQLite.

I'll add full details here when I can.

## License

The GPL version 3, read it at [http://www.gnu.org/licenses/gpl.txt](http://www.gnu.org/licenses/gpl.txt)

##Contributing

Any help is always welcome, please contact sam [at] infitialis.com and we can discuss any help you would like to give.
