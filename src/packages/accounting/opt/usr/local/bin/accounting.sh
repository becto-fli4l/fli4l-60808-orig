#!/bin/sh
##-----------------------------------------------------------------------------
## /usr/local/bin/accounting.sh - collect data from chains         __FLI4LVER__
##
## Creation:     05.06.2002  Michael Knipping <fli4l@knibo.de>
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

#------------------------------------------------------------------------------
# Abbruch wenn Datum in der Vergangenheit
#------------------------------------------------------------------------------
ACCOUNTING_MINDATE=20070300
if [ $(date +%Y%m%d) -lt $ACCOUNTING_MINDATE ]; then
    echo "date is in foretime"
    exit 1
fi

#------------------------------------------------------------------------------
# prüfen ob accounting.sh / acccal.sh schon läuft
#------------------------------------------------------------------------------
LOCKFILE=$workdir/accounting.pid
myname=`basename $0`
if [ -f "$LOCKFILE" ]; then
    echo -n "`cat $LOCKFILE | cut -d" " -f1` is running and working with your "
    echo "data (PID: `cat $LOCKFILE | cut -d" " -f2`)."
    echo "Please wait and try again."
    exit 1
fi
echo $myname $$ > $LOCKFILE                    # --- LOCK-Datei erzeugen

#------------------------------------------------------------------------------
# Funktion für bc Aufruf
#------------------------------------------------------------------------------
cat_bc()
{
cat <<EOF | /usr/bin/bc
$1
EOF
}

#------------------------------------------------------------------------------
# Variablen setzen & Monatsdatei einlesen
#------------------------------------------------------------------------------
monat=$(date +%m)
day=$(date +%d)
tag=$(date +%Y-%m-%d)
jahr=$(date +%Y)
outfile="$ACCOUNTING_DIR/$jahr/$jahr-$monat.acc.tmp"
datafile="$ACCOUNTING_DIR/$jahr/$jahr-$monat.acc"
[ -f "$datafile" ] && . $datafile

#------------------------------------------------------------------------------
# Verzeichnisse erzeugen
#------------------------------------------------------------------------------
mkdir -p $ACCOUNTING_DIR
mkdir -p $ACCOUNTING_DIR/$jahr

#------------------------------------------------------------------------------
# Daten auslesen
#------------------------------------------------------------------------------
/sbin/iptables -nvxL -Z accin | grep 'RETURN' >$workdir/accin.tmp
/sbin/iptables -nvxL -Z accout | grep 'RETURN' >$workdir/accout.tmp

cat /proc/net/dev | grep ':' > $workdir/int.tmp

count=1
while read line
do
    set -f
    set $line
    case $9 in
        0.0.0.0/0) ;; # Ausschließen von Traffic zwischen maskierten Netzen
        *)
            eval "acci$count"=$2
            eval "accip$count"=$9
            count=`expr $count + 1`
        ;;
    esac
done < $workdir/accin.tmp
count=1
while read line
do
    set -f
    set $line
    case $8 in
        0.0.0.0/0) ;; # Ausschließen von Traffic zwischen maskierten Netzen
        *)
            eval "acco$count"=$2
            count=`expr $count + 1`
        ;;
    esac
done < $workdir/accout.tmp
count=`expr $count - 1`

#------------------------------------------------------------------------------
# Index anlegen / aktualisieren
#------------------------------------------------------------------------------
echo "aktualisiere Index"
ACCOUNTING_HOST_n=0
ACCOUNTING_INT_n=0
[ -f "$ACCOUNTING_DIR/index.acc" ] && . $ACCOUNTING_DIR/index.acc
indexchange=0

# suche nach neuen Hosts
[ 0$count -eq 0 ] || for idx in `seq 1 $count`
do
    eval accip=\$accip$idx
    if ! grep -q "'$accip'" $ACCOUNTING_DIR/index.acc 2>/dev/null; then
        ACCOUNTING_HOST_n=`expr $ACCOUNTING_HOST_n + 1`
        eval "ACCOUNTING_HOST_"$ACCOUNTING_HOST_n"_IP"=$accip
        indexchange=1
    fi
done

# suche nach neuen Interfaces
while read line
do
    intn=$(echo $line | cut -d':' -f1)
    if ! grep -q "'$intn'" $ACCOUNTING_DIR/index.acc 2>/dev/null; then
        ACCOUNTING_INT_n=`expr $ACCOUNTING_INT_n + 1`
        eval "ACCOUNTING_INT_"$ACCOUNTING_INT_n"_NAME"=$intn
        indexchange=1
    fi
done < $workdir/int.tmp

# Index neu schreiben
if [ $indexchange != 0 ]; then
    echo "ACCOUNTING_HOST_n='"$ACCOUNTING_HOST_n"'" > $ACCOUNTING_DIR/index.acc
    [ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
    do
        eval acch="\$ACCOUNTING_HOST_"$idx"_NAME"
        eval accip="\$ACCOUNTING_HOST_"$idx"_IP"
        case $accip in
            "") ;;
            *) echo "ACCOUNTING_HOST_"$idx"_IP='"$accip"'" >> $ACCOUNTING_DIR/index.acc ;;
        esac
        case $acch in
            "") ;;
            *) echo "ACCOUNTING_HOST_"$idx"_NAME='"$acch"'" >> $ACCOUNTING_DIR/index.acc ;;
        esac
    done
    echo "ACCOUNTING_INT_n='"$ACCOUNTING_INT_n"'" >> $ACCOUNTING_DIR/index.acc
    [ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
    do
        eval accint="\$ACCOUNTING_INT_"$idx"_NAME"
        echo "ACCOUNTING_INT_"$idx"_NAME='"$accint"'" >> $ACCOUNTING_DIR/index.acc
    done
    rm -f $workdir/sorthosts
fi

# Hosts nach IP sortieren
if [ ! -f $workdir/sorthosts ]; then
    [ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
    do
        eval accip="\$ACCOUNTING_HOST_"$idx"_IP"
        eval acch="\$ACCOUNTING_HOST_"$idx"_NAME"
        case $accip$acch in
            "") ;;
            *)
                case $accip in
                    "") echo $acch $idx >> $workdir/sorthosts.tmp ;;
                    *) echo $accip $idx >> $workdir/sorthosts.tmp ;;
                esac
            ;;
        esac
    done
    sed -e "s/\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\)/00\1.00\2.00\3.00\4~\1.\2.\3.\4/" \
        -e "s/0\{0,2\}\([0-9]\{3\}\.\)0\{0,2\}\([0-9]\{3\}\.\)0\{0,2\}\([0-9]\{3\}\.\)0\{0,2\}\([0-9]\{3\}\)/\1\2\3\4/" \
        $workdir/sorthosts.tmp | sort | cut -d" " -f2 > $workdir/sorthosts
    rm $workdir/sorthosts.tmp
fi

#------------------------------------------------------------------------------
# Hostdaten anlegen / aktualisieren
#------------------------------------------------------------------------------
echo "aktualisiere Hostdaten"
eval monthi="\$ACCOUNTING_Y"$jahr"_M"$monat"_i"
eval montho="\$ACCOUNTING_Y"$jahr"_M"$monat"_o"
: ${monthi:=0}
: ${montho:=0}
[ 0$count -eq 0 ] || for idx in `seq 1 $count`
do
    eval accip=\$accip$idx
    eval acci=\$acci$idx
    eval acco=\$acco$idx
    : ${acci:=0}
    : ${acco:=0}
    case $acci$acco in
        00) ;;
        *)
            ACCOUNTING_HOST_NR=$(grep "'$accip'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
            eval dayhosti="\$ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_D"$day"_i"
            eval dayhosto="\$ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_D"$day"_o"
            eval monthhosti="\$ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_i"
            eval monthhosto="\$ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_o"
            case $dayhosti in
                ""|0) dayhosti=$acci ;;
                *) dayhosti=$(cat_bc "$dayhosti + $acci") ;;
            esac
            case $dayhosto in
                ""|0) dayhosto=$acco ;;
                *) dayhosto=$(cat_bc "$dayhosto + $acco") ;;
            esac
            case $monthhosti in
                ""|0) monthhosti=$acci ;;
                *) monthhosti=$(cat_bc "$monthhosti + $acci") ;;
            esac
            case $monthhosto in
                ""|0) monthhosto=$acco ;;
                *) monthhosto=$(cat_bc "$monthhosto + $acco") ;;
            esac
            monthi=$(cat_bc "$monthi + $acci")
            montho=$(cat_bc "$montho + $acco")
            eval "ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_D"$day"_i"=$dayhosti
            eval "ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_D"$day"_o"=$dayhosto
            eval "ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_i"=$monthhosti
            eval "ACCOUNTING_HOST_"$ACCOUNTING_HOST_NR"_Y"$jahr"_M"$monat"_o"=$monthhosto
        ;;
    esac
done
eval "ACCOUNTING_Y"$jahr"_M"$monat"_i"=$monthi
eval "ACCOUNTING_Y"$jahr"_M"$monat"_o"=$montho

#------------------------------------------------------------------------------
# Interfacedaten anlegen / aktualisieren
#------------------------------------------------------------------------------
echo "aktualisiere Interfacedaten"
[ -f "$workdir/last.int" ] && mv $workdir/last.int $workdir/last.int.tmp
while read line
do
    set $(echo $line | cut -d':' -f2)
    intin=$1
    intout=$9
    : ${intin:=0}
    : ${intout:=0}
    case $intin$intout in
        00) ;;
        *)
            intname=$(echo $line | cut -d':' -f1)
            if grep -q "'$intname'" $workdir/last.int.tmp 2>/dev/null; then
                set $(grep "'$intname'" $workdir/last.int.tmp)
                lastin=$2
                lastout=$3
            else
                lastin=0
                lastout=0
            fi
            : ${lastin:=0}
            : ${lastout:=0}

            # Interfaceüberlauf erkennen
            case $(cat_bc "$intin < $lastin") in
                1)
                    case $ACCOUNTING_MAXINT in
                        ""|0) newinti=$intin ;;
                        *) newinti=$(cat_bc "$intin + $ACCOUNTING_MAXINT - $lastin") ;;
                    esac
                ;;
                *) newinti=$(cat_bc "$intin - $lastin") ;;
            esac
            case $(cat_bc "$intout < $lastout") in
                1)
                    case $ACCOUNTING_MAXINT in
                        ""|0) newinto=$intout ;;
                        *) newinto=$(cat_bc "$intout + $ACCOUNTING_MAXINT - $lastout") ;;
                    esac
                ;;
                *) newinto=$(cat_bc "$intout - $lastout") ;;
            esac

            echo "'"$intname"'" $intin $intout >> $workdir/last.int
            ACCOUNTING_INT_NR=$(grep "'$intname'" $ACCOUNTING_DIR/index.acc | cut -d'_' -f3)
            eval dayinti="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_D"$day"_i"
            eval dayinto="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_D"$day"_o"
            eval monthinti="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_i"
            eval monthinto="\$ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_o"
            case $dayinti in
                ""|0) dayinti=$newinti ;;
                *) dayinti=$(cat_bc "$dayinti + $newinti") ;;
            esac
            case $dayinto in
                ""|0) dayinto=$newinto ;;
                *) dayinto=$(cat_bc "$dayinto + $newinto") ;;
            esac
            case $monthinti in
                ""|0) monthinti=$newinti ;;
                *) monthinti=$(cat_bc "$monthinti + $newinti") ;;
            esac
            case $monthinto in
                ""|0) monthinto=$newinto ;;
                *) monthinto=$(cat_bc "$monthinto + $newinto") ;;
            esac
            eval "ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_D"$day"_i"=$dayinti
            eval "ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_D"$day"_o"=$dayinto
            eval "ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_i"=$monthinti
            eval "ACCOUNTING_INT_"$ACCOUNTING_INT_NR"_Y"$jahr"_M"$monat"_o"=$monthinto
            case $ACCOUNTING_DEBUG_INT in
                yes) echo $tag $intname': '$intin $intout'  last: '$lastin $lastout'  new: '$newinti $newinto >>$ACCOUNTING_DIR/int.log ;;
            esac
        ;;
    esac
done < $workdir/int.tmp

#------------------------------------------------------------------------------
# Daten in Datei scheiben
#------------------------------------------------------------------------------
echo "schreibe Daten"
{
    echo "#------------------------------------------------------------------------------"
    echo "# Accounting data $jahr $monat"
    echo "# Creation: $tag  OPT_ACCOUNTING"
    echo "#------------------------------------------------------------------------------"
} > $outfile

# Index Hosts
echo "ACCOUNTING_HOST_Y"$jahr"_M"$monat"_n='"$ACCOUNTING_HOST_n"'" >> $outfile
[ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
do
    eval accip="\$ACCOUNTING_HOST_"$idx"_IP"
    eval acch="\$ACCOUNTING_HOST_"$idx"_NAME"
    case $accip in
        "") ;;
        *) echo "ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_IP='"$accip"'" >> $outfile ;;
    esac
    case $acch in
        "") ;;
        *) echo "ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_NAME='"$acch"'" >> $outfile ;;
    esac
done

# Tagesdaten Hosts
[ -f $datafile ] && sed -n -e '/_D'$day'_/d' -e '/_HOST_.*_D[0-3][0-9]_/p' $datafile >> $outfile
[ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
do
    eval dayhosti="\$ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_D"$day"_i"
    eval dayhosto="\$ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_D"$day"_o"
    case $dayhosti in
        ""|0) ;;
        *) echo "ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_D"$day"_i='"$dayhosti"'" >> $outfile ;;
    esac
    case $dayhosto in
        ""|0) ;;
        *) echo "ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_D"$day"_o='"$dayhosto"'" >> $outfile ;;
    esac
done

# Monatsdaten Hosts
[ 0$ACCOUNTING_HOST_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_HOST_n`
do
    eval monthhosti="\$ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_i"
    eval monthhosto="\$ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_o"
    case $monthhosti in
        ""|0) ;;
        *) echo "ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_i='"$monthhosti"'" >> $outfile ;;
    esac
    case $monthhosto in
        ""|0) ;;
        *) echo "ACCOUNTING_HOST_"$idx"_Y"$jahr"_M"$monat"_o='"$monthhosto"'" >> $outfile ;;
    esac
done

# Monatsdaten Gesamt
eval monthi="\$ACCOUNTING_Y"$jahr"_M"$monat"_i"
eval montho="\$ACCOUNTING_Y"$jahr"_M"$monat"_o"
echo "ACCOUNTING_Y"$jahr"_M"$monat"_i='"$monthi"'" >> $outfile
echo "ACCOUNTING_Y"$jahr"_M"$monat"_o='"$montho"'" >> $outfile

# Index Interfaces
echo "ACCOUNTING_INT_Y"$jahr"_M"$monat"_n='"$ACCOUNTING_INT_n"'" >> $outfile
[ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
do
    eval intn="\$ACCOUNTING_INT_"$idx"_NAME"
    echo "ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_NAME='"$intn"'" >> $outfile
done

# Tagesdaten Interfaces
[ -f $datafile ] && sed -n -e '/_D'$day'_/d' -e '/_INT_.*_D[0-3][0-9]_/p' $datafile >> $outfile
[ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
do
    eval dayinti="\$ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_D"$day"_i"
    eval dayinto="\$ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_D"$day"_o"
    case $dayinti in
        ""|0) ;;
        *) echo "ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_D"$day"_i='"$dayinti"'" >> $outfile ;;
    esac
    case $dayinto in
        ""|0) ;;
        *) echo "ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_D"$day"_o='"$dayinto"'" >> $outfile ;;
    esac
done

# Monatsdaten Interfaces
[ 0$ACCOUNTING_INT_n -eq 0 ] || for idx in `seq 1 $ACCOUNTING_INT_n`
do
    eval monthinti="\$ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_i"
    eval monthinto="\$ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_o"
    case $monthinti in
        ""|0) ;;
        *) echo "ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_i='"$monthinti"'" >> $outfile ;;
    esac
    case $monthinto in
        ""|0) ;;
        *) echo "ACCOUNTING_INT_"$idx"_Y"$jahr"_M"$monat"_o='"$monthinto"'" >> $outfile ;;
    esac
done

echo "ACCOUNTING_Y"$jahr"_M"$monat"_DAYS='"$day"'" >> $outfile
mv $outfile $datafile
echo "Fertig"

#------------------------------------------------------------------------------
# Aufräumen
#------------------------------------------------------------------------------
rm -f $workdir/accin.tmp
rm -f $workdir/accout.tmp
rm -f $workdir/int.tmp
rm -f $workdir/$tag.acc
rm -f $workdir/$tag.int
rm -f $workdir/last.int.tmp
rm -f $workdir/last.int.tmp
rm -f $LOCKFILE 2>/dev/null       # --- LOCK-Datei wieder entfernen
