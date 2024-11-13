/**
 * @file    fdsend.c
 * Sends <STDIN> and <STDOUT> to another process over a UNIX socket.
 * @author  Christoph Schulz
 * @since   2015-02-16
 * @version $Id$
 */

#include "magic.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

ssize_t
sock_fd_write(int sock, void *buf, ssize_t buflen, int fd1, int fd2)
{
    struct msghdr msg;
    struct iovec  iov;
    char control[CMSG_SPACE(sizeof(int))];
    struct cmsghdr *cmsg;
    int *pfd;

    iov.iov_base = buf;
    iov.iov_len = buflen;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(sizeof(int) * 2);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    pfd = (int *) CMSG_DATA(cmsg);
    *pfd++ = fd1;
    *pfd++ = fd2;

    return sendmsg(sock, &msg, 0);
}

int main(int argc, char *argv[])
{
    int sock;
    int retries;
    struct sockaddr_un addr;
    ssize_t size;
    unsigned magic = MAGIC;

    if (argc != 3 || strlen(argv[1]) >= sizeof(addr.sun_path)) {
        fprintf(stderr, "Usage: %s <socket-path> <num-retries>\n", argv[0]);
        return 1;
    }

    retries = atoi(argv[2]);
    if (retries < 0) {
        retries = 0;
    } else if (retries > 60) {
        retries = 60;
    }

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        return 2;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);

    while (connect(sock, (struct sockaddr const *) &addr, sizeof(addr)) < 0) {
        if (errno == ECONNREFUSED || errno == ENOENT) {
            if (retries-- == 0) {
                close(sock);
                return 4;
            } else {
                sleep(1);
            }
        } else {
            close(sock);
            return 3;
        }
    }

    size = sock_fd_write(sock, &magic, sizeof(magic), 0, 1);
    close(sock);
    return size == sizeof(magic) ? 0 : 5;
}
