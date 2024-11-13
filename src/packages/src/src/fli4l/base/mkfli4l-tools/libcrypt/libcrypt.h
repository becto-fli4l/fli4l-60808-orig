/* prototypes for internal crypt functions
 *
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef __LIBCRYPT_H__
#define __LIBCRYPT_H__

extern char *__sha256_crypt(const char *pw, const char *salt);

#endif
