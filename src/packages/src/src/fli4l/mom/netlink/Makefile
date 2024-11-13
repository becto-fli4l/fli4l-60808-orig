CC=gcc

mom_netlink: mom_netlink.o env.o netlink.o type_basic.o type_address.o msg_ifinfo.o msg_ifaddr.o msg_prefix.o msg_rt.o
mom_netlink.o: msg_ifinfo.h msg_ifaddr.h msg_prefix.h msg_rt.h netlink.h env.h defs.h
msg_ifinfo.o: msg_ifinfo.h type_address.h type_basic.h netlink.h env.h defs.h
msg_ifaddr.o: msg_ifaddr.h type_address.h type_basic.h netlink.h env.h defs.h
msg_prefix.o: msg_prefix.h type_address.h type_basic.h netlink.h env.h defs.h
msg_rt.o: msg_rt.h type_address.h type_basic.h netlink.h env.h defs.h
type_address.o: type_address.h netlink.h env.h defs.h
type_basic.o: type_basic.h netlink.h env.h defs.h
netlink.o: netlink.h defs.h
env.o: env.h defs.h

clean:
	rm -f mom_netlink *.o

.PHONY: clean

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -pedantic -Werror -Wall -Wextra -ansi -D_GNU_SOURCE -D_POSIX_SOURCE -c -o $@ $<
