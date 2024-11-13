CFLAGS=-x c -arch ppc -pipe -Wno-trigraphs -fasm-blocks -fpascal-strings \
	-Wno-four-char-constants -Wmost -Wno-four-char-constants -Wno-unknown-pragmas \
	-fmessage-length=0 -Wall -g -O0

LDFLAGS=-framework System -framework CoreServices -framework IOKit /usr/lib/crt1.o

all: FindDrive

clean:
	rm -f FindDrive *.o

FindDrive: FindDrive.o
	$(LD) $(LDFLAGS) $^ -o $@

