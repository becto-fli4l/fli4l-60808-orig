/**
 * sends one line of text to a given tcp-host/port and waits for an answer
 * Necessary because netcat can't do this in an easy way.
 *
 * Additionally this tool waits for the next second-change to allow
 * synchronizing shell-scripts with the clock
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 *
 * I don't accept liability nor provide warranty for this Software.
 * This material is provided "AS-IS" and at no charge.
 *
 * Version History
 *
 * 0.1         Initial Release
 *
 * rresch    (c) 2009 Robert Resch <fli4l@robert.reschpara.de>
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>

#define timeout 20000 /* 20msec */
#define defaultport 50290 /* Pollin AVR-NET-IO */
#define buffsize 256 /* buffer-size */

int write_help(char * progname)
{
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("-h <hostname>\ttarget-hostname or ip\n");
    printf("-p <port>\ttarget-port\t\t\tdefault: %d\n", defaultport);
    printf("-t <text>\ttext to send to host\n");
    printf("-l <lines>\tlines of answer waiting for.\tdefault: 1\n");
    printf("-a <file>:<skip>:<num>\tappends <num> bytes from file <file> to <text> and skips <skip> bytes from <file>\n");
    printf("-w\t\twait for next second-change\n");
    return 255;
}

int writelan(struct hostent *host, int port, char *text, char *append, int lines)
{
    int s, i, c, j, line;
    int a=0, skip=0, num=0, p=0;
    char buffer[buffsize];
    char buf2[1]="";
    struct timeval now, tv;
    struct timezone tz;
    struct sockaddr_in lcdAddr;

    s = socket(PF_INET, SOCK_STREAM, 6);
    memset(&lcdAddr, 0, sizeof(lcdAddr));
    lcdAddr.sin_family = AF_INET;
    lcdAddr.sin_port = htons(port);
    bcopy(host->h_addr_list[0], &(lcdAddr.sin_addr.s_addr), host->h_length);

    if (connect(s, (struct sockaddr *) &lcdAddr, sizeof(lcdAddr)) < 0) {
        return 253;
    }

    for (i = 0; text[i] && i < (buffsize - 2); i++) {
        if ((text[i] <= ' ') || (text[i] == (char) 255) || (text[i] == (char) 127)) {
            buffer[i] = ' ';
        } else {
            buffer[i] = text[i];
        }
    }
    if (append[0])
    {
	for (j=0,c=0;append[j];j++)
	{
	    if (append[j] == ':')
	    {
		append[j]='\0';
		if (p==0)
		{
		    a=open(append,O_RDONLY);
		    p=j;
		}
		else if (p!=0 && a)
		{
		    skip=atoi(&append[p+1]);
		    lseek(a,atoi(&append[p+1]),SEEK_SET);
		    num=atoi(&append[j+1]);
		    break;
		}
	    }
	}
    }
    if (a)
    {
	j=i;
        for (; i < (j+num) && i < (buffsize - 2); i++)
	{
	    c=read(a,buf2,1);
	    if (c)
	    {
		c=buf2[0];
	        if ((c <= ' ') || (c == (char) 255) || (c == (char) 127)) c=' ';
		buffer[i]=c;
	    }
	}
	close(a);
    }
    buffer[i] = '\n';
    buffer[i + 1] = 0;

    send(s, buffer, i + 1, 0);

    i = 0;
    line = 0;
    gettimeofday(&tv, &tz);

    while (line < lines && i < 20000) {
        if ( (j = recv(s, buffer, 1, MSG_DONTWAIT)) < 0 ) {
            gettimeofday(&now, &tz);
            if (( now.tv_usec - tv.tv_usec ) > timeout) {
            	 return 1;
            }
            usleep(2000);
        }
        if (j > 0) {
            i++;
            c = (int) buffer[0];
            tv.tv_usec = now.tv_usec;
            if (c == '\n') {
            	line++;
            }
            printf("%c", c);
        }
    }
    close(s);
    return 0;
}

void waitforsecondchange()
{
    time_t now;
    struct tm * tm;
    int sec;
    now = time((time_t *) NULL);
    tm = localtime(&now);
    sec = tm->tm_sec;
    while (tm->tm_sec == sec) {
        now = time((time_t *) NULL);
        tm = localtime(&now);
        usleep(50000);
    }
}

int main(int argc, char *argv[])
{
    char *text = "";
    char *append = "";
    struct hostent *host = NULL;
    int port = defaultport;
    int i;
    int hostset = 0;
    int textset = 0;
    int help = 0;
    int lines = 1;

    for (i = 1; i < argc; i++) {
        if (!strncmp("-p", argv[i], 2) && i + 1 < argc ) {
            i++;
            if (atoi(argv[i]) > 0 && atoi(argv[i]) < 65536) {
                port = atoi(argv[i]);
                continue;
            } else {
                printf("Invalid port-number\n");
                help = 1;
            }
        }
        if (!strncmp("-l", argv[i], 2) && i + 1 < argc ) {
            i++;
            if (atoi(argv[i]) > 0 && atoi(argv[i]) < 200) {
                lines = atoi(argv[i]);
                continue;
            } else {
                printf("Invalid line-count\n");
                help = 1;
            }
        }
        if (!strncmp("-h", argv[i], 2) && i + 1 < argc ) {
            i++;
            host = gethostbyname(argv[i]);
            if (host != NULL) {
                hostset = 1;
                continue;
            }
            printf("Unknown Hostname\n");
            return 251;
        }
        if (!strncmp("-t", argv[i], 2) && i + 1 < argc ) {
            i++;
            text = argv[i];
            textset = 1;
            continue;
        }
        if (!strncmp("-a", argv[i], 2) && i + 1 < argc ) {
            i++;
            append = argv[i];
            continue;
        }
        if (!strncmp("-w", argv[i], 2) && i < argc ) {
            waitforsecondchange();
            return 0;
        }
        help = 1;
    }
    if (!hostset || !textset) {
    	help = 1;
    }
    if (help) {
    	return write_help((char *) argv[0]);
    }
    return writelan(host, port, text, append, lines);
}
