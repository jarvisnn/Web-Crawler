CC = g++
CFLAGS = -std=c++11 -g -Wall

all: remove-output crawler run clean

file-output: remove-output crawler run-o clean

crawler: crawler.o clientSocket.o parser.o
	$(CC) $(CFLAGS) -o crawler crawler.o clientSocket.o parser.o

crawler.o: crawler.cpp clientSocket.h parser.h
	$(CC) $(CFLAGS) -c crawler.cpp

clientSocket.o: clientSocket.cpp parser.h
	$(CC) $(CFLAGS) -c clientSocket.cpp	

parser.o: parser.cpp
	$(CC) $(CFLAGS) -c parser.cpp

run:
	./crawler

run-o:
	./crawler > statistics.txt

clean:
	rm -f crawler *.o

remove-output:
	rm -f statistics.txt