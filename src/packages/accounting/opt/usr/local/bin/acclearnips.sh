#!/bin/sh
##-----------------------------------------------------------------------------
## /usr/local/bin/acclearnips.sh - learn IPs from ARP Cache        __FLI4LVER__
##
## Creation:     15.12.2005  Michael Knipping <fli4l@knibo.de>
## Last Update:  $Id$
##
## Copyright (c) 2002-2009 - Michael Knipping <fli4l@knibo.de>
## Copyright (c) 2009-2016 - fli4l-Team <team@fli4l.de>
##-----------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Config Datei einlesen, Abbruch wenn diese nicht existiert
#------------------------------------------------------------------------------
if [ -f "/etc/accounting.conf" ]; then
    . /etc/accounting.conf
else
    echo "/etc/accounting.conf not found"
    exit 1
fi
iplist="$workdir/iplist"

#------------------------------------------------------------------------------
# Regeln für neue IPs erzeugen
#------------------------------------------------------------------------------
addipsfromarp()
{
    while read line
    do
        set -f
        set $line
        case "$line" in
            '') ;;
            IP*) ;;
            *)
                case "$4" in
                    '00:00:00:00:00:00') ;; # ungültige MAC Adresse
                    *)
                        case "$ACCOUNTING_LEARNFROMINT" in
                            *$6*) INTOK=1 ;;
                            '') INTOK=1 ;;
                            *) INTOK=0 ;;
                        esac
                        case "$INTOK" in
                            1)
                                if ! grep -q "^${1}$" $iplist 2>/dev/null; then
                                    /sbin/iptables -A accin  -d $1 -j RETURN -m comment --comment "add by acclearnips.sh"
                                    /sbin/iptables -A accout -s $1 -j RETURN -m comment --comment "add by acclearnips.sh"
                                    /sbin/iptables -A accinlive  -d $1 -j RETURN -m comment --comment "add by acclearnips.sh"
                                    /sbin/iptables -A accoutlive -s $1 -j RETURN -m comment --comment "add by acclearnips.sh"
                                    echo "${1}" >> $iplist
                                fi
                            ;;
                        esac
                    ;;
                esac
            ;;
        esac
    done
}

cat /proc/net/arp | addipsfromarp
