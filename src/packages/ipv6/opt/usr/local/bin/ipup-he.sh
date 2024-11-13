#!/bin/sh
# $1 = dynamically assigned IPv4 address

result=$(wget -q -O - "http://ipv4.tunnelbroker.net/ipv4_end.php?ip=${1%/*}&pass=${tun6in4_he_password_md5}&apikey=${circ_tun6in4_he_userid}&tid=${circ_tun6in4_he_tunnelid}")
rc=$?
if [ $rc -eq 0 ] && echo "$result" | grep -qe "^+OK"
then
    exit 0
else
    logger -t "ipup-he[$$]" -p daemon.err "error while updating local IPv4 tunnel endpoint to $1 (reason: $result, exit code: $rc)"
    exit 1
fi
