#! /bin/sh

: ${script:="$(basename $0)[$$]"}
: ${facility:=daemon}

#
# logging
#

# $1 = priority
# $2 = message
log()
{
    logger -s -t "$script" -p $facility.$1 "$2"
}

# $1 = message
log_error()
{
    log err "$1"
    return 1
}

# $1 = message
log_warn()
{
    log warning "$1"
    return 0
}

# $1 = message
log_info()
{
    log notice "$1"
    return 0
}
