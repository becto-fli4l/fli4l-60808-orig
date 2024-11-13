AC_PREREQ([2.69])

AC_INIT([mmencode], [1.0])
AM_INIT_AUTOMAKE([foreign -Wall])

AC_CONFIG_SRCDIR([src/mmencode.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for header files.
AC_CHECK_HEADERS([stdio.h ctype.h sys/types.h sys/stat.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([floor getcwd memset regcomp select socket strerror strtol strtoul])

AC_CONFIG_FILES([Makefile src/Makefile])
                 
AC_OUTPUT

#_oOo_
