CC = gcc

CFLAGS = -Wall
CPPFLAGS = -D_BSD_SOURCE $(shell pkg-config libevdev --cflags)

SRCS   := hwsuppd.c
SRCS   += log.c
SRCS   += button_gpio.c
SRCS   += button_evdev.c

OBJS   := $(SRCS:%.c=%.o)

hwsuppd: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -levdev

$(OBJS): hwsuppd.h

clean:
	rm -f *.o hwsuppd
