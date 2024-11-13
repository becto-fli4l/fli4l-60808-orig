#!/bin/sh
##------------------------------------------------------------------------------
## c3Surf - login for services
## generate voucher
## parameter: none
## Creation:    2009 fs
## Last Update: $Id$
##-------------------------------------------------------------------------------
## Licence and conditions look at ~/config/c3surf.txt
##-------------------------------------------------------------------------------
# testing only
#DEBUG_INFO="+"

. /var/run/c3surf.conf

[ "$DEBUG_INFO" ] && echo "c3surf_voucher_gen starts ..."

# write a csv of the new vouchers
voucher_print_csv()
{
  # write to list
  echo "$voucher_usr, $voucher_pwd, $voucher_time, $v_valid, $voucher_lives" >> "$C3SURF_VOUCHER_PRINTFILE"
}


# generate one voucher_user / voucher_pwd combination
voucher_gen_randomuser()
{
  # random generator
  # voucher_seed=$(date +%s)
  # es gibt RANDOM
  # account basis generieren
  gen=`/bin/dd if=/dev/urandom count=1 2> /dev/null | /usr/bin/md5sum | /usr/bin/cut -f1 -d" "`
  # ergibt z.B.: 8fd4b690fe8b7861528903f18c2db232
  # 32 Stellen
  # usr range: (ersten 12) Format <index>-xxxx-xxxx-xxxx
  # index = index im config file.
  # pwd range: firstChar: 13-17 Length: 6 - 8 (bis 25)
  # + zufall upper char
  my_usrlen=$C3SURF_VOUCHER_USRLEN
  if [ $my_usrlen -gt 4 ]
  then
    voucher_usr1=`/usr/bin/expr substr "$gen" 1 4`
    if [ $C3SURF_VOUCHER_USRCAP = "random" -a $RANDOM -gt 16383 -o $C3SURF_VOUCHER_USRCAP = "yes" ]
    then
      voucher_usr1="$(echo "$voucher_usr1" | /bin/sed -e 'y/abcdef/ABCDEF/')"
    fi
    my_usrlen=`expr $my_usrlen - 4`
    if [ $my_usrlen -gt 4 ]
    then
      voucher_usr2=`/usr/bin/expr substr "$gen" 5 4`
      if [ $C3SURF_VOUCHER_USRCAP = "random" -a $RANDOM -gt 16383 -o $C3SURF_VOUCHER_USRCAP = "yes" ]
      then
        voucher_usr2="$(echo "$voucher_usr2" | /bin/sed -e 'y/abcdef/ABCDEF/')"
      fi
      my_usrlen=`expr $my_usrlen - 4`
      if [ $my_usrlen -gt 4 ]
      then
        voucher_usr3=`/usr/bin/expr substr "$gen" 9 4`
        if [ $C3SURF_VOUCHER_USRCAP = "random" -a $RANDOM -gt 16383 -o $C3SURF_VOUCHER_USRCAP = "yes" ]
        then
          voucher_usr3="$(echo "$voucher_usr3" | /bin/sed -e 'y/abcdef/ABCDEF/')"
        fi
        my_usrlen=`expr $my_usrlen - 4`
        voucher_usr4=`/usr/bin/expr substr "$gen" 13 $my_usrlen`
        if [ $C3SURF_VOUCHER_USRCAP = "random" -a $RANDOM -gt 16383 -o $C3SURF_VOUCHER_USRCAP = "yes" ]
        then
          voucher_usr4="$(echo "$voucher_usr4" | /bin/sed -e 'y/abcdef/ABCDEF/')"
        fi
        my_usrcount=4
      else
        voucher_usr3=`/usr/bin/expr substr "$gen" 9 $my_usrlen`
        if [ $C3SURF_VOUCHER_USRCAP = "random" -a $RANDOM -gt 16383 -o $C3SURF_VOUCHER_USRCAP = "yes" ]
        then
          voucher_usr3="$(echo "$voucher_usr3" | /bin/sed -e 'y/abcdef/ABCDEF/')"
        fi
        my_usrcount=3
      fi
    else
      voucher_usr2=`/usr/bin/expr substr "$gen" 5 $my_usrlen`
      if [ $C3SURF_VOUCHER_USRCAP = "random" -a $RANDOM -gt 16383 -o $C3SURF_VOUCHER_USRCAP = "yes" ]
      then
        voucher_usr2="$(echo "$voucher_usr2" | /bin/sed -e 'y/abcdef/ABCDEF/')"
      fi
      my_usrcount=2
    fi
  else
    voucher_usr1=`/usr/bin/expr substr "$gen" 1 $my_usrlen`
    if [ $C3SURF_VOUCHER_USRCAP = "random" -a $RANDOM -gt 16383 -o $C3SURF_VOUCHER_USRCAP = "yes" ]
    then
      voucher_usr1="$(echo "$voucher_usr1" | /bin/sed -e 'y/abcdef/ABCDEF/')"
    fi
    my_usrcount=1
  fi
  # voucher_usr2=`/usr/bin/expr substr "$gen" 5 4`
  # voucher_usr3=`/usr/bin/expr substr "$gen" 9 4`
  # voucher_usr4=`/usr/bin/expr substr "$gen" 13 4`

  # pwd offset: 0-4 + position 13
  # pwd_off=`expr \( $voucher_seed % 5 \) + 13`
  # pwd_off=`/usr/bin/expr \( $RANDOM % 5 \) + 13`
  pwd_off=17
  # pwd len: 0-2 + 6 = len range 6 - 8
  # pwd_len=`expr \( $voucher_seed % 3 \) + 6`
  # pwd_len=`/usr/bin/expr \( $RANDOM % 3 \) + 6`
  pwd_len=`/usr/bin/expr \( $RANDOM % $C3SURF_VOUCHER_PWDMOD \) + $C3SURF_VOUCHER_PWDLEN`
  voucher_pwd=`/usr/bin/expr substr "$gen" $pwd_off $pwd_len`
  # toupper=`expr $RANDOM % 100`
  # if [ $toupper -gt 49 ]


#  if [ $RANDOM -gt 16383 ]
#  then
#    voucher_usr1="$(echo "$voucher_usr1" | /bin/sed -e 'y/abcdef/ABCDEF/')"
#  fi
#  if [ $RANDOM -gt 16383 ]
#  then
#    voucher_usr2="$(echo "$voucher_usr2" | /bin/sed -e 'y/abcdef/ABCDEF/')"
#  fi
#  if [ $RANDOM -gt 16383 ]
#  then
#    voucher_usr3="$(echo "$voucher_usr3" | /bin/sed -e 'y/abcdef/ABCDEF/')"
#  fi
#  if [ $RANDOM -gt 16383 ]
#  then
#    voucher_usr4="$(echo "$voucher_usr4" | /bin/sed -e 'y/abcdef/ABCDEF/')"
#  fi


  case $my_usrcount in
    4)
      voucher_usr="$voucher_usr1-$voucher_usr2-$voucher_usr3-$voucher_usr4"
    ;;
    3)
      voucher_usr="$voucher_usr1-$voucher_usr2-$voucher_usr3"
    ;;
    2)
      if [ $C3SURF_VOUCHER_USRLEN -lt 8 ]
      then
        voucher_usr="$voucher_usr1$voucher_usr2"
      else
        voucher_usr="$voucher_usr1-$voucher_usr2"
      fi
    ;;
    1)
      voucher_usr="$voucher_usr1"
    ;;
  esac

  # index vor den account schreiben
  [ -n "$1" ] && voucher_usr="$1-$voucher_usr"

  if [ $C3SURF_VOUCHER_PWDCAP = "random" -a $RANDOM -lt 16383 -o $C3SURF_VOUCHER_PWDCAP = "yes" ]
  then
    voucher_pwd="$(echo "$voucher_pwd" | /bin/sed -e 'y/abcdef/ABCDEF/')"
  fi

  # finished voucher
  [ "$DEBUG_INFO" ] && echo "$voucher_usr ; $voucher_pwd"
}


# call c3surf_worker to insert one voucher
# globals: voucher_pwd, voucher_usr, voucher_time, voucher_counter, voucher_valid, voucher_lives
voucher_activate_user()
{
  # voucher_pwd voucher_usr forename="VOUCHER" name="VOUCHER" email="" voucher_time blocktime="-1" voucher_counter curfew="" module="voucher" voucher_valid voucher_lives
  /usr/local/bin/c3surf_worker.sh "doAdminUser" "$voucher_pwd" "$voucher_usr" \
                                  "VOUCHER" "VOUCHER" "" "$voucher_time" \
                                  "-1" "$voucher_counter" "" \
                                  "voucher" "$voucher_valid" "$voucher_lives"
}


# generate all vouchers
voucher_generator()
{
  for idx in `seq 1 $C3SURF_VOUCHER_N`
  do
    # set counter to 1
    my_n=1

    # count the vouchers
    for f in "$C3SURF_READ_PATH"/$idx-*.data-voucher
    do
      # schließe den Abfrage-String aus
      if [ "$f" != "$C3SURF_READ_PATH/$idx-*.data-voucher" ]
      then
        my_n=`/usr/bin/expr $my_n \+ 1`
      fi
    done

    eval voucher_time='$C3SURF_VOUCHER_TIME_'$idx
    eval voucher_days='$C3SURF_VOUCHER_DAYS_'$idx
    eval voucher_count='$C3SURF_VOUCHER_COUNT_'$idx
    eval voucher_lives='$C3SURF_VOUCHER_LIVES_'$idx

    [ "$DEBUG_INFO" ] && echo "voucher_time: $voucher_time, voucher_days: $voucher_days, voucher_count: $voucher_count, voucher_lives: $voucher_lives"

    if [ $voucher_days -eq 0 ]
    then
      voucher_valid=0
      v_valid="ultimo"
    else
      # v_days=`/usr/bin/expr $voucher_days "*" 3600 "*" 24`
      # voucher expire date in secs
      # voucher_valid=`/usr/bin/expr $v_now "+" $v_days`
      # jetzt auf 00:00 Uhr
      v_days=`/usr/bin/expr $voucher_days "*" 24`
      # voucher expire date in secs
      # next day 00:00 h
      voucher_valid=`/bin/date -d $v_days:1440 +%s`
      # calc datediff - it is different for summer/winter time.
      v_datediff=`/bin/date -d "1970.1.1-0:0:$voucher_valid" +%z`
      v_datediff=`/usr/bin/expr $v_datediff "*" 36`
      # for the valid sring subtract one minute=60 - it shows the day before
      v_zone=`/usr/bin/expr $voucher_valid "+" $v_datediff "-" 60`
      # v_valid=`/bin/date -d "1970.1.1-0:0:$v_zone" +%d.%m.%Y-%H:%M`
      v_valid=`/bin/date -d "1970.1.1-0:0:$v_zone" +%d.%m.%Y`
    fi

    [ "$DEBUG_INFO" ] && echo "voucher_valid: $v_valid"

    if [ $voucher_lives -eq 0 ]
    then
      voucher_counter=0
    else
      voucher_counter=-1
    fi

    [ "$DEBUG_INFO" ] && echo "voucher_counter: $voucher_counter"

    # generate all vouchers
    for i in `seq $my_n $voucher_count`
    do
      # generate usr / pwd
      voucher_gen_randomuser "$idx"
      # tell user-data to c3surf
      voucher_activate_user
      # put new values to the printer csv
      voucher_print_csv
    done

  done
}

# fix values for date calc
v_now=$(/bin/date +%s)

voucher_generator

[ "$DEBUG_INFO" ] && echo "...done"
