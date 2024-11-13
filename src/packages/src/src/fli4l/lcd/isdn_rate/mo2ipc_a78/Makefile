LD      = ld
TARGETS = mo2ipc_a78

all:	$(TARGETS)

clean:
	rm -f $(TARGETS) mo2ipc_a78.o

mo2ipc_a78: mo2ipc_a78.asm
	nasm -f elf mo2ipc_a78.asm
	$(LD) $(LDFLAGS) -o mo2ipc_a78 mo2ipc_a78.o
