#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  char *ptr;
  unsigned long sleeptime;

  sleeptime = atol(argv[1]);
  ptr = strrchr(argv[0], '/');
  if(ptr != NULL && 0 == strcmp(ptr+1, "msleep"))
    sleeptime *= 1000UL;
#if 0
  printf("%s %ld\n", argv[0], sleeptime);
#endif
  usleep(sleeptime);
  return (0);
}
