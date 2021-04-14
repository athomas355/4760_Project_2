# Makefile for Project 2: shared memory
#

CFLAGS="-g"


bin_addr:%.c %h
	$(CC) -c $(CFLAGS) -o bin_addr $^

clean:
	rm main bin_addr
