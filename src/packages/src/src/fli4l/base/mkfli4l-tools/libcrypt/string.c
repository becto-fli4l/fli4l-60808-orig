#include "config.h"
#include <string.h>

extern char *stpncpy (char *__restrict __dest,
                      __const char *__restrict __src, size_t __n)
{
	strncpy(__dest, __src, __n);
	if (__n > 0 && __dest[__n - 1] == '\0')
		return __dest + strlen(__dest);
	else
		return __dest + __n;
}

extern void *mempcpy (void *__restrict __dest,
                      __const void *__restrict __src, size_t __n)
{
	return (char *) memcpy(__dest, __src, __n) + __n;
}
