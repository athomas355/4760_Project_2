# Makefile for Project 2: shared memory
#

CFLAGS="-g"


master: %.cpp %h
	#We gotta figure out how to do rules to name the cpp files in the line below
	$(CC) -c $(CFLAGS) -o master master.cpp bin_addr.cpp 

clean:
	rm master
