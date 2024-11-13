/* read 1 Byte from stdin and output ascii code to stdout */
/* it's used to get a randomized integer value (0-255)    */
/* example: rand=`b2d < /dev/urandom`                     */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  unsigned char value;
  read(0, &value, sizeof(value));
  printf("%d\n",value);
  return 0;
}
