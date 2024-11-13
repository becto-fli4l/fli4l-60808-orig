/**
 * @file    fdrecv.c
 * Waits for a pair of FDs on a UNIX socket and then executes a command with
 * <STDIN> and <STDOUT> set to the FDs received.
 * @author  Christoph Schulz
 * @since   2015-02-16
 * @version $Id$
 */

#include "magic.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

extern char **environ;

ssize_t
sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd1, int *fd2)
{
    ssize_t       size;
    struct msghdr msg;
    struct iovec  iov;
    char          control[2 * CMSG_SPACE(sizeof(int))];

    iov.iov_base = buf;
    iov.iov_len = bufsize;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    size = recvmsg(sock, &msg, 0);
    if (size == bufsize) {
        struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

        if (cmsg &&
                cmsg->cmsg_len == CMSG_LEN(sizeof(int) * 2) &&
                cmsg->cmsg_level == SOL_SOCKET &&
                cmsg->cmsg_type == SCM_RIGHTS) {
            int *pfd = (int *) CMSG_DATA(cmsg);
            *fd1 = *pfd++;
            *fd2 = *pfd++;
        } else {
            size = 0;
        }
    }
    return size;
}

int main(int argc, char *argv[])
{
    int sock, conn;
    int timeout;
    struct sockaddr_un addr;
    unsigned magic;
    struct pollfd pollfd;
    int pollnum;
    int fd1, fd2;

    if (argc < 4 || strlen(argv[1]) >= sizeof(addr.sun_path)) {
        fprintf(stderr, "Usage: %s <socket-path> <timeout-sec> <program> [<args...>]\n", argv[0]);
        return 1;
    }

    timeout = atoi(argv[2]);
    if (timeout < 0) {
        timeout = 0;
    } else if (timeout > 60) {
        timeout = 60;
    }

    if (remove(argv[1]) < 0 && errno != ENOENT) {
        perror("remove");
        return 2;
    }

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 3;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, argv[1]);

    if (bind(sock, (struct sockaddr const *) &addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        remove(argv[1]);
        return 4;
    }

    if (listen(sock, SOMAXCONN) < 0) {
        perror("listen");
        close(sock);
        remove(argv[1]);
        return 5;
    }

    pollfd.fd = sock;
    pollfd.events = POLLIN;
    pollnum = poll(&pollfd, 1, timeout * 1000);
    if (pollnum != 1) {
        if (pollnum < 0) {
            perror("poll");
        }
        close(sock);
        remove(argv[1]);
        return 6;
    }

    conn = accept(sock, NULL, NULL);
    if (conn < 0) {
        perror("accept");
        close(sock);
        remove(argv[1]);
        return 7;
    }

    pollfd.fd = conn;
    pollnum = poll(&pollfd, 1, timeout * 1000);
    if (pollnum != 1) {
        if (pollnum < 0) {
            perror("poll");
        }
        close(conn);
        close(sock);
        remove(argv[1]);
        return 8;
    }

    if (sock_fd_read(conn, &magic, sizeof(magic), &fd1, &fd2) == 0 ||
            magic != MAGIC) {
        close(conn);
        close(sock);
        remove(argv[1]);
        return 9;
    }

    close(conn);
    close(sock);
    remove(argv[1]);
    dup2(fd1, 0);
    dup2(fd2, 1);
    close(fd1);
    close(fd2);
    execve(argv[3], &argv[3], environ);
    return 10;
}
