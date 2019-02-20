CC = gcc
CSOFLAGS = -Wall -std=c11 -pedantic -shared

LISTTARGET = bin/list.so

all:
	gcc -I include src/LinkedList.c -Wall -pedantic -std=c11 -shared -fPIC -o bin/liblist.so
	gcc -I include src/CalendarParser.c -Wall -pedantic -std=c11 -shared -fPIC -o bin/libcal.so

list:
	gcc -I include src/LinkedList.c -Wall -pedantic -std=c11 -shared -fPIC -o bin/liblist.so

parser:
	gcc -I include src/CalendarParser.c -Wall -pedantic -std=c11 -shared -fPIC -o bin/libcal.so

clean:
	rm -f *bin/*