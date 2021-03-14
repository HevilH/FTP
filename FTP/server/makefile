CC = gcc
CFLAGS = -Wall
TARGET = server

$(TARGET) : server.o
	$(CC) $(CFLAGS) -o $(TARGET) server.o

server.o : ftp_header.h server.c
	$(CC) $(CFLAGS) -c server.c

clean :
	rm -rf *.o
	rm server
