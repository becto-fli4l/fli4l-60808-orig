#!/bin/sh

channel=$1
phy=$2
if [ -z "$channel" ]
then
 channel=1
fi
if [ -z "$phy" ]
then
  phy="phy0"
fi
print_htcap ()
{
  ht_capab=''
  ht_dir='-'
  case $2 in
    1|2|3|4|5|6|7)
      ht_dir='+'
      ;;
    36|44|52|60)
      ht_dir='+'
      ;;
    100|108|116|124|132)
      ht_dir='+'
      ;;
    149|157)
      ht_dir='+'
      ;;
  esac
  if [ $(($1 & 1)) -eq 1 ]
  then
    ht_capab="$ht_capab[LDPC]"
  fi
  if [ $(($1 & 2)) -eq 2 ]
  then
    ht_capab="$ht_capab[HT40$ht_dir]"
  fi
  if [ $(($1 & 32)) -eq 32 ]
  then
    ht_capab="$ht_capab[SHORT-GI-20]"
  fi
  if [ $(($1 & 64)) -eq 64 ]
  then
    ht_capab="$ht_capab[SHORT-GI-40]"
  fi
  if [ $(($1 & 128)) -eq 128 ]
  then
    ht_capab="$ht_capab[TX-STBC]"
  fi
  if [ $(($1 & 768)) -eq 256 ]
  then
    ht_capab="$ht_capab[RX-STBC1]"
  fi
  if [ $(($1 & 768)) -eq 512 ]
  then
    ht_capab="$ht_capab[RX-STBC12]"
  fi
  if [ $(($1 & 768)) -eq 768 ]
  then
    ht_capab="$ht_capab[RX-STBC123]"
  fi
  if [ $(($1 & 4096)) -eq 4096 ]
  then
    ht_capab="$ht_capab[DSSS_CCK-40]"
  fi
  echo "$ht_capab"
}

iw phy $phy info |\
while read line
do
  case $line in
    *Capabilities:*)
      capab=`echo $line | sed -n 's/.*\(0x[0-9A-Fa-f]\{1,4\}\).*/\1/p'`
      ;;
    *MHz*)
      found=`echo $line | sed -n 's/.*MHz \[\([0-9]\{1,3\}\)\].*dBm.*/\1/p'`
      if [ "$found"  = "$channel" ]
      then
        print_htcap $capab $channel
      fi
    ;;
  esac
done
