#ifndef CRYPT_CONFIG_H_
#define CRYPT_CONFIG_H_

#include <stddef.h>
#include <string.h>

#define attribute_hidden

#if defined(__MINGW32__)

#define __BYTE_ORDER __LITTLE_ENDIAN
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#else
#include <alloca.h>
#endif

#if defined(__APPLE__)
#undef stpncpy
#undef mempcpy
#endif

extern char *stpncpy (char *__restrict __dest,
                      __const char *__restrict __src, size_t __n);

extern void *mempcpy (void *__restrict __dest,
                      __const void *__restrict __src, size_t __n);

/* shut up gcc-4.x signed warnings */
#undef strcpy
#define strcpy(dst,src) strcpy((char*)dst,(char*)src)
#undef strlen
#define strlen(s) strlen((char*)s)
#undef strncat
#define strncat(dst,src,n) strncat((char*)dst,(char*)src,n)
#undef strncmp
#define strncmp(s1,s2,n) strncmp((char*)s1,(char*)s2,n)

#endif
