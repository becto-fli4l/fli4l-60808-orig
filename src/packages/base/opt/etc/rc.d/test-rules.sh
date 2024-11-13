#!/bin/sh
script_name=test-fules.sh

. ./test.inc
indent='    '

usage ()
{
    cat <<EOF
usage: $0
            - runs pre-defined tests
       $0 -f <file>
            - runs test contained in <file>
            - '-f -' reads from stdin
       $0 "rule"
            - tests given rules
EOF
    exit 1
}

if [ $# -eq 1 -a "$1" = "-h" ]; then
    usage
fi

if [ $# -eq 0 ] || [ $# -eq 2 -a "$1" = "-f" ]
then
    if [ $# -eq 2 ]
    then
        case "$2" in
            -)
                cat
                ;;
            *)
                if [ -f "$2" ]
                then
                    cat "$2"
                else
                    echo "can't open '$2'"
                    usage 1
                fi
                ;;
        esac
    else
        cat <<EOF
#
# accept all
#
ACCEPT

#
# destination defaults to any
#

1.2.3.4                 ACCEPT
1.2.3.4/24              ACCEPT
1.2.3.4/24              ACCEPT BIDIRECTIONAL

#
# source and destination default to any
#

22                      ACCEPT
22-44                   ACCEPT
any:22                  ACCEPT
any:22-44               ACCEPT

#
# source defaults to any
#

1.2.3.4:22              ACCEPT
1.2.3.4/24:22-44        ACCEPT

#
# no defaults
#
any             any     ACCEPT
any:22          any     ACCEPT
any             any:22  ACCEPT
1.2.3.4/24      any     ACCEPT
any             1.2.3.4/24 ACCEPT
5.6.7.8/24      1.2.3.4/24 ACCEPT
any             1.2.3.4/24 ACCEPT BIDIRECTIONAL
5.6.7.8/24:22-24        1.2.3.4/24:26-28 ACCEPT BIDIRECTIONAL

#
# icmp types
#
prot:icmp               ACCEPT
prot:icmp:echo-request  ACCEPT
prot:!icmp              ACCEPT

#
# mac
#
mac:00:ab:cd:de:f0:19   ACCEPT
mac:!00:ab:cd:de:f0:19  ACCEPT

#
# if
#
if:IP_NET_1_DEV:IP_NET_1_DEV ACCEPT

#
# SNAT
#
SNAT:1.2.3.4
SNAT:1.2.3.4-1.2.3.5
SNAT:IP_NET_1
SNAT:IP_NET_1:1
SNAT:IP_NET_1:1-9
prot:tcp SNAT:IP_NET_1:1-9

#
# DNAT
#
DNAT:1.2.3.4-1.2.3.5:1-9
prot:tcp DNAT:IP_NET_1:1-9

#
# LOG
#
LOG
LOG:test-prefix
ACCEPT LOG:test-prefix
DNAT:1.2.3.4-1.2.3.5:1-9 LOG:dnat-prefix
SNAT:1.2.3.4 LOG:snat-prefix

EOF
    fi | sed -e 's/[[:space:]]\+/ /g' | while read y
    do
            # skip comments and empty lines
        if echo "$y" | grep -q -e '^[[:space:]]*#.*$' -e '^[[:space:]]*$'; then
            echo "$y"
        else
            echo "add_rule '$y'"
            add_rule filter FORWARD "$y"
        fi
    done
else
    case $2 in
    del)
        echo "del_rule filter FORWARD '$1'"
        del_rule filter FORWARD "$1" "$3"
        ;;
    ins)
        echo "ins_rule filter FORWARD '$1' $3"
        ins_rule filter FORWARD "$1" "$3" "$4"
        ;;
    *)
        echo "add_rule filter FORWARD '$1'"
        add_rule filter FORWARD "$1" "$3"
        ;;
    esac
fi
