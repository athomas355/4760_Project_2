# Makefile for Project 2: shared memory
#
CC = "g++"
CFLAGS="-g"

master: *.cpp 
	$(CC) $(CFLAGS) $@.cpp -o $@

clean:
	rm master