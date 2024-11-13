CC	=	gcc
CFLAGS	=	-Wall -O2 -finline-functions

all:		dtest

dtest:	test.o
	$(CC) -o dtest test.o

clean:
	rm -f dtest *.o
