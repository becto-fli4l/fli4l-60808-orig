/* gethostname */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
extern int h_errno;

int main (int argc, char **argv)
{
   struct hostent* h = NULL;
   int i;
   int j;
   struct in_addr inp;

   if (argc != 2) {
     puts ("\nUsage: gethostname <name>\n");
     return 1;
   }
   if (!inet_aton (argv[1], &inp))
     return 1;
   h = gethostbyaddr((char *)&inp.s_addr, sizeof(inp.s_addr), AF_INET);

   if (!h)
     return 1;
   if (h->h_name)
     printf ("%s\n",h->h_name);
   return 0;
}
