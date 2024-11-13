#! /bin/sh
get_parent()
{
    ppid=`sed -n -e 's/^PPid:[[:space:]]*//p' < /proc/$1/status`
}

log()
{
    indent=
    pname=
    case $prog in
        insmod)   get_parent $$ ; get_parent $ppid; indent="    " ;;
        modprobe) get_parent $$ ;;
    esac

    pname=`sed -n -e 's/^Name:[[:space:]]*//p' < /proc/$ppid/status`
    if [ -f /var/run/rc.script ]; then
        . /var/run/rc.script
        pname="$pname ($script)"
    fi

    {
        echo "$indent$prog $@$postfix"
        echo "$indent    ($pname)"
    }  >> /var/log/modules.log

    {
        echo "$indent$prog $@$postfix"
        echo "$indent    ($pname)"
    } | /usr/local/bin/logmsg modwrap
}

prog=`basename $0`
if [ -f /var/run/debug_modules_all ]; then
    log "$@"
    exec /usr/local/bin/wrapped/$prog "$@"
else
    if ! /usr/local/bin/wrapped/$prog "$@"; then
        postfix=" failed ..."
        /usr/local/bin/log "$@"
    fi
fi
