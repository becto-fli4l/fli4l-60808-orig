#! /bin/sh

cat <<EOF
#
# Ruleset
#
EOF
for i in INPUT FORWARD; do
    iptables -nvL $i | grep -e ^Chain -e pkts -e acc
done
for i in accin accinlive accout accoutlive; do
     iptables -nL $i
done

. /etc/accounting.conf
cat <<EOF
#
# List of known ips
#
EOF
cat $workdir/iplist
cat <<EOF
#
# accounting.conf
#
EOF
cat /etc/accounting.conf  
