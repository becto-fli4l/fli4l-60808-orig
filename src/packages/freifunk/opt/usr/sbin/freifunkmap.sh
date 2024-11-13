#!/bin/sh


        # only one instance should be running at time
        # if exists an other one kill them and all wgets
        pid="$(cat /tmp/freifunkmap.pid 2> /dev/null)"
        if [ ! -z $pid ]; then
                kill $pid >/dev/null 2> /dev/null
        fi

        # get rid of wget-s hanging arround
        for f in /proc/[0-9]*/cmdline; do
                if grep -sq 'mygooglemapscoords.txt' "$f"; then
                        WGET_PID="${f#/proc/}"; WGET_PID="${WGET_PID%%/*}"
                        kill "${WGET_PID}" 2>/dev/null
                fi
        done

        echo "$$" > /tmp/freifunkmap.pid
	. /var/run/freifunk_register.conf
	#updatestring=$(nvram get ff_adm_gps)
	#updatestring="52.53612457671945, 13.412010669708252"
	updatestring="$FREIFUNK_LAT"", ""$FREIFUNK_LON"
        # set up the updateintervall
        if [ "$FREIFUNK_GUI_MAPINTERVAL" = "minute" ]; then
                updateiv=60
        elif [ "$FREIFUNK_GUI_MAPINTERVAL" = "hourly" ]; then
                updateiv=3600
        elif [ "$FREIFUNK_GUI_MAPINTERVAL" = "daily" ]; then
                updateiv=86400
        elif [ "$FREIFUNK_GUI_MAPINTERVAL" = "monthly" ]; then
                updateiv=2592000
        fi

        #set up olsr ip
        #OLSR_IP=$(nvram get wifi_ipaddr)
    cnt=0
    row=0
    lq=""
    ip=""
    # get the neighbor info from the olsrd info plugin
     for i in `
               wget -q -O - http://127.0.0.1:2006/neighbours`;
            do

             string=`echo $i|sed -ne'

         s/[a-z]//g
     s/[A-Z]//g
     s/://g
     p
          '
           `

  if [ "$i" = "Neighbors" ]
  then
       break
  fi

    if [ ! -z "$string" ]; then

        if [ $row -eq 1 ]; then
            nip=$string
        fi
    # Next data of neighbour

        if [ $row -eq 3 ]; then
            lq=$string
        fi
        if [ $row -eq 7 ]; then
            cnt=`/usr/bin/expr $cnt + 1`
            row=0
        else
        row=`/usr/bin/expr $row + 1`
        fi

        if [ ! -z $nip ]; then
            if [ ! -z $lq ]; then

                    # if possible get the googlemaps coords from the neighbor and add
                    # it to the updatestring for the freifunk map

                    ncoords="`wget -q -O - http://$nip/mygooglemapscoords.txt 2> /dev/null`"
                    
                    if [ ! -z "$ncoords" ]; then
                            updatestring=`echo $updatestring, $ncoords, $lq`

                    fi
                    nip=""
                    lq=""
            fi
        fi

    fi

done
string=""

        # get batman ip if possible
        #BATMAN_IP=$(nvram get ff_bat_ran_ip)
	BATMAN_IP=""
        # before exiting send the updatestring to the mapserver
        update=`echo -e "$updatestring" | sed s/\ /%20/g`

#        lines=`echo -E "$(FREIFUNK_GUI_NOTICE)" | wc -l `
	lines=1
        N=1
        note=""
        lines=`/usr/bin/expr $lines + 1`
        while [ $N -lt $lines ] ; do
                # remove linefeed from every line and replace with %0D.*$/\1%0D/g`
		line=`echo -E "$FREIFUNK_GUI_NOTICE" | head -n$N | sed -re 's/(.*).*$/\1%0D/g`
                note=`echo -n "${note}${line}" | sed  's/\ /%20/g'`
                note=`echo -n "${note}" | sed 's/&/%26/g'`
                note=`echo -n "${note}" | sed 's/"/%22/g'`
                N=`/usr/bin/expr $N + 1`
        done

#        result=`wget "$FREIFUNK_GUI_MAPSERVER?update=$update&updateiv=$updateiv&olsrip=$FREIFUNK_GUI_MAPIP&batmanip=$BATMAN_IP&note=$note" -qO -`

        result=`wget "$FREIFUNK_GUI_MAPSERVER?update=$update&updateiv=$updateiv&olsrip=$FREIFUNK_GUI_MAPIP&batmanip=$BATMAN_IP&note=$note" -qO -`

#        log DEBUG "wget \"$FREIFUNK_GUI_MAPSERVER?update=$update&updateiv=$updateiv&olsrip=$FREIFUNK_GUI_MAPIP&batmanip=$BATMAN_IP&note=$note\" -qO -"
#        log INFO "$result"

        # everything looks fine
        rm /tmp/freifunkmap.pid
        
