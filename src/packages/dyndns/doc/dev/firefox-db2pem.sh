#!/bin/sh
# ***************************************************************************
# *                                  _   _ ____  _
# *  Project                     ___| | | |  _ \| |
# *                             / __| | | | |_) | |
# *                            | (__| |_| |  _ <| |___
# *                             \___|\___/|_| \_\_____|
# *
# * Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
# *
# * This software is licensed as described in the file COPYING, which
# * you should have received as part of this distribution. The terms
# * are also available at http://curl.haxx.se/docs/copyright.html.
# *
# * You may opt to use, copy, modify, merge, publish, distribute and/or sell
# * copies of the Software, and permit persons to whom the Software is
# * furnished to do so, under the terms of the COPYING file.
# *
# * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# * KIND, either express or implied.
# *
# * $Id$
# ***************************************************************************
# This shell script creates a fresh ca-bundle.crt file for use with libcurl. 
# It extracts all ca certs it finds in the local Firefox database and converts
# them all into PEM format.
#

# rewritten by Raphael Gradenwitz to match for curl-use in fli4l opt dyndns

# READ THIS PLEASE: BITTE LESEN:
#==============================
# Dieses Skript ist speziell umgeschrieben um nur die Zertifikate aus firefox
# zu exportieren die fuer das opt_dyndns vom fli4l gebraucht werden.
# Voraussetzungen: Ein Linux Desktopsystem (bei mir Ubuntu) mit installiertem
# Firefox und 'certutil'
# Certifikate die nicht im "Builtin Object Token:" (von Firefox mitgeliefert)
# sind so wie beispielsweise IN-Berlin muessen zunaechst im Firefox importiert
# werden. Dazu einfach auf die Seite gehen welche im opt zum updaten aufgerufen
# werden sollen. Dazu sind keine Login-Daten notwendig, eine 404-Verbindung
# reicht aus. Firefox meckert wenn er das Root-Certifikat (noch) nicht hat.
# Einfach bestaetigen das es heruntergeladen werden soll und es sich genau
# anschauen damit euch der Name bekannt ist und hier eventuell hinzugefuegt
# werden kann.
# Mein Firefox ist deutsch, da heisst der NICHT-"Builtin Object Token:" Part
# "Software-Sicherheitsmodul:". Eventuell muesst ihr das auch noch anpassen wenn
# euer firefox z.B. englisch ist.
# der Rest spricht wohl fuer sich denke ich.
# Raphael, 08.12.2008


# Falls -x als erstes Argument uebergeben wird, wird ein Trace vom Skript in
# einer Datei auf dem Desktop abgelegt. Dies hilft bei der Fehlersuche:
if [ "$1" = "-x" ]
then
    debug=1
#    tracefile="~/Desktop/$(basename $0 .sh)_trace$$.log"
    tracefile="~/Desktop/$(basename $0 .sh)_trace$(date +%y%m%d%H%M%S).log"
    exec 2> $tracefile
    set -vx
    shift
fi

# Hier die Namen der gewuenschten Zertifikate einfuegen.
# Wichtig: Leerzeichen hier mit '_' ersetzen!
wanted_certs="\"Thawte_Server_CA\"\
              \"Equifax_Secure_Global_eBusiness_CA\"\
              \"Go_Daddy_Class_2_CA\"\
              \"IN-Berlin_Root_CA_(G2)_-_Individual_Network_Berlin_e.V.\"\
              \"GlobalSign_Root_CA\"\
              \"Equifax_Secure_CA\"\
              \"Thawte_Premium_Server_CA\""

db=`ls -1d $HOME/.mozilla/firefox/*default`  
out=$1

if test -z "$out"; then
  out="$HOME/Desktop/ca-bundle.crt" # use a sensible default
fi

find_match()
{
    r=0
    for cert in $wanted_certs
    do
        cert=`echo $cert | sed -e "s/_/ /g"`
        if [ "$nick" = "$cert" ]
        then
            r=1
        fi
    done
    echo $r
}

currentdate=`date`

cat >$out <<EOF
##
## Bundle of CA Root Certificates for fli4l opt_dyndns
##
## Converted at: ${currentdate}
## These were converted from a local Firefox directory by
## a modified firefox-db2pem script.
##
EOF

certutil -L -h 'Software-Sicherheitsmodul' -d $db | \
grep ' *[CcGTPpu]*,[CcGTPpu]*,[CcGTPpu]* *$' | \
sed -e 's/ *[CcGTPpu]*,[CcGTPpu]*,[CcGTPpu]* *$//' -e 's/\(.*\)/"\1"/' | \
sort | \
while read nickname; \
 do nick=`echo $nickname | sed -e "s/Software-Sicherheitsmodul://g"`; \
 if [ $(find_match) -eq 1 ] ; \
 then echo $nick; \
 eval certutil -d $db -L -n "$nickname" -a ; \
  fi ; \
done >> $out

certutil -L -h 'Builtin Object Token' -d $db | \
grep ' *[CcGTPpu]*,[CcGTPpu]*,[CcGTPpu]* *$' | \
sed -e 's/ *[CcGTPpu]*,[CcGTPpu]*,[CcGTPpu]* *$//' -e 's/\(.*\)/"\1"/' | \
sort | \
while read nickname; \
 do nick=`echo $nickname | sed -e "s/Builtin Object Token://g"`; \
 if [ $(find_match) -eq 1 ] ; \
 then echo $nick; \
eval certutil -d $db -L -n "$nickname" -a ; \
  fi ; \
done >> $out

echo "Schauen Sie sich das Erbenis an unter \"$out\""
if [ $debug ]
then set +vx
    echo "Ein trace dieses Skriptlaufes liegt unter \"$tracefile\""
else echo "Im Fehlerfall nochmal mit Argument \"-x\" versuchen das erzeugt ein trace"
fi



