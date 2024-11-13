#!/bin/sh
#----------------------------------------------------------------------------
# reload-radvd.sh - reloads/restarts radvd                       __FLI4LVER__
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

. /etc/boot.d/base-helper

pidfile=/var/run/radvd-legacy.pid

net_config_dir=/var/run/ipv6.nets
radvd_config_dir=/var/run/radvd/legacy
conffile=/etc/radvd-legacy.conf

devs=
for netfile in "$net_config_dir"/*.conf
do
    [ -f "$netfile" ] || continue
    . "$netfile"
    [ "$adv" = "yes" ] || continue

    netindex="$(basename "$netfile" .conf)"
    transformed_dev=$(to_shell_id "$dev")
    if ! echo "$devs" | grep -q " $transformed_dev\( \|$\)"
    then
        devs="$devs $transformed_dev"
        eval dev_${transformed_dev}="\"\$dev\""
        eval nets_${transformed_dev}=
    fi

    eval nets_${transformed_dev}="\"\$nets_${transformed_dev} \$netindex\""
done

for transformed_dev in $devs
do
    eval dev=\$dev_${transformed_dev}
    echo "interface $dev {"
    echo "    IgnoreIfMissing on;"
    echo "    AdvSendAdvert on;"

    eval nets=\$nets_${transformed_dev}
    for netindex in $nets
    do
        input="$radvd_config_dir/$netindex.conf"
        [ -f $input ] || continue
        cat $input
    done

    echo "};"
done > $conffile

if [ -s $conffile ]
then
    if [ -f $pidfile ]
    then
        kill -HUP $(cat $pidfile)
    else
        radvd -C $conffile -p $pidfile -m syslog >/dev/null 2>&1 &
    fi
else
    if [ -f $pidfile ]
    then
        kill $(cat $pidfile)
    fi
fi
