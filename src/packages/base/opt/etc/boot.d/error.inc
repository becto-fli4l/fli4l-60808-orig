#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/error.inc
# Functions for handling error conditions.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$error_api" != yes ]
then

. /etc/boot.d/logging.inc

# sets an error flag and shows an error message
set_error ()
{
    case "x$1" in
        x) ;;
        *) log_error "$1" ;;
    esac
    base_error='yes'
    return 1
}

# checks and resets error flag set by set_error
is_error ()
{
    case "x$base_error" in
        x) return 1 ;;
        *) base_error= ; return 0 ;;
    esac
}

# clears error flag set by set_error
clear_error ()
{
    base_error=
}

base_error=
error_api='yes'

fi # $error_api != yes
