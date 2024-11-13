#!/bin/sh
export PATH=/usr/local/sbin:/usr/sbin:/sbin:/usr/local/bin:/usr/bin:/bin

timewarp=$1

notify_timewarp ()
{
    # Call all custom notify scripts
    for j in /etc/chrony.d/timewarp[0-9][0-9][0-9].*
    do
        if [ -f $j ]
    then
        . $j
    fi
    done
}

notify_timewarp 2>&1 | logmsg timewarp

exit 0
