#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/logging.inc
# Functions for logging, especially while booting.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$logging_api" != yes ]
then

. /etc/boot.d/console.inc

do_log () {
    : ${base_log_file:=/bootmsg.txt}
    : ${log_func:=einfo}
    local script=$(basename "$script")

    case $# in
    0)
        while read line; do
            if [ "$booting" = "yes" -o "$shuttingdown" = "yes" ]
            then
                $log_func "$SCRIPT_INDENT$line" 1>&2
                case $log_boot$cons_boot in
                    yesyes) echo "[$script] $log_prefix $line" >> $base_log_file ;;
                esac
            else
                echo "$log_prefix $line" 1>&2
            fi
        done
        ;;
    *)
        if [ "$booting" = "yes" -o "$shuttingdown" = "yes" ]
        then
            $log_func "$SCRIPT_INDENT$*" 1>&2
            case $log_boot$cons_boot in
                yesyes) echo "[$script] $log_prefix $*" >> $base_log_file ;;
            esac
        else
            echo "$log_prefix $*" 1>&2
        fi
        ;;
    esac
}

log_info()
{
    log_boot=
    log_prefix="  "
    log_func=einfo

    case $# in
        0) do_log ;;
        *) do_log "$@" ;;
    esac
    return 0
}

log_warn()
{
    log_boot=yes
    log_prefix="WARN:"
    log_func=ewarn
    do_log "$@"
    SCRIPT_RESULT=1
    return 0
}

# show an error message and write it to /bootmsg.txt
log_error()
{
    log_boot=yes
    log_prefix="ERR:"
    log_func=eerror
    do_log "$@"
    SCRIPT_RESULT=1
    SCRIPT_WARNONLY=
    return 1
}

case "`ls -l /proc/$$/fd/1 2> /dev/null`" in
    */dev/console*) cons_boot=yes ;;
    *) cons_boot= ;;
esac

logging_api='yes'

fi # $logging_api != yes
