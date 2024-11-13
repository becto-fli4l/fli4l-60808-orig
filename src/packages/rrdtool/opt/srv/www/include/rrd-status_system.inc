load_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/load-$last_x.png --title "Load - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Load">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Load" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:shortterm=$rrd_dbpath/load/load.rrd:shortterm:AVERAGE \
        DEF:midterm=$rrd_dbpath/load/load.rrd:midterm:AVERAGE \
        DEF:longterm=$rrd_dbpath/load/load.rrd:longterm:AVERAGE \
        VDEF:shortterm_l=shortterm,LAST \
        VDEF:midterm_l=midterm,LAST \
        VDEF:longterm_l=longterm,LAST \
        AREA:shortterm$rrd_col_red_light:" 1 Minute,  letzter\:" GPRINT:shortterm_l:"%5.2lf\n" \
        AREA:midterm$rrd_col_orange:" 5 Minuten, letzter\:" GPRINT:midterm_l:"%5.2lf\n" \
        AREA:longterm$rrd_col_yellow:"15 Minuten, letzter\:" GPRINT:longterm_l:"%5.2lf\n" \
        LINE1:midterm$rrd_col_orange:"" \
        LINE1:shortterm$rrd_col_red_light:"" \
        COMMENT:"\n" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
}

cpuall_graph ()
{
    last_x="$1"
    cpu_anz=`grep -cE '^[Pp]rocessor[[:space:]]+:' /proc/cpuinfo`
    cpu_anzr=`expr $cpu_anz - 1`
    def_str=""
    for n in `seq 0 $cpu_anzr`
    do
        if [ -d $rrd_dbpath/cpu-${n} ]
        then
            def_str=`echo "$def_str DEF:interrupt${n}=$rrd_dbpath/cpu-${n}/cpu-interrupt.rrd:value:AVERAGE "`
            def_str=`echo "$def_str DEF:nice${n}=$rrd_dbpath/cpu-${n}/cpu-nice.rrd:value:AVERAGE "`
            def_str=`echo "$def_str DEF:softirq${n}=$rrd_dbpath/cpu-${n}/cpu-softirq.rrd:value:AVERAGE "`
            def_str=`echo "$def_str DEF:steal${n}=$rrd_dbpath/cpu-${n}/cpu-steal.rrd:value:AVERAGE "`
            def_str=`echo "$def_str DEF:system${n}=$rrd_dbpath/cpu-${n}/cpu-system.rrd:value:AVERAGE "`
            def_str=`echo "$def_str DEF:user${n}=$rrd_dbpath/cpu-${n}/cpu-user.rrd:value:AVERAGE "`
            def_str=`echo "$def_str DEF:wait${n}=$rrd_dbpath/cpu-${n}/cpu-wait.rrd:value:AVERAGE "`
            def_str=`echo "$def_str CDEF:cpu${n}_1=interrupt${n},nice${n},+ "`
            def_str=`echo "$def_str CDEF:cpu${n}_2=cpu${n}_1,softirq${n},+ "`
            def_str=`echo "$def_str CDEF:cpu${n}_3=cpu${n}_2,steal${n},+ "`
            def_str=`echo "$def_str CDEF:cpu${n}_4=cpu${n}_3,system${n},+ "`
            def_str=`echo "$def_str CDEF:cpu${n}_5=cpu${n}_4,user${n},+ "`
            def_str=`echo "$def_str CDEF:cpu${n}=cpu${n}_5,wait${n},+ "`
            if [ $n -eq 0 ]
            then
                def_str=`echo "$def_str CDEF:cpu_sum_${n}=cpu${n},1,* "`
            else
                n1=`expr $n - 1`
                def_str=`echo "$def_str CDEF:cpu_sum_${n}=cpu_sum_${n1},cpu${n},+ "`
            fi
            case ${n} in
                0|8)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_red_light:cpu_${n}\t "`
                    ;;
                1|9)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_green_light:cpu_${n}\t "`
                    ;;
                2|10)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_blue_light:cpu_${n}\t "`
                    ;;
                3|11)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_yellow_light:cpu_${n}\n "`
                    ;;
                4|12)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_red:cpu_${n}\t "`
                    ;;
                5|13)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_green:cpu_${n}\t "`
                    ;;
                6|14)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_blue:cpu_${n}\t "`
                    ;;
                7|15)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_yellow:cpu_${n}\n "`
                    ;;
                *)
                    def_str=`echo "$def_str LINE1:cpu${n}$rrd_col_green:cpu_${n}\n "`
                    ;;
            esac
        fi
    done
    if [ $cpu_anz -gt 1 ]
    then
        def_str=`echo "$def_str LINE1:cpu_sum_${n}#000000:cpu_all:dashes=2 "`
    fi
    /usr/bin/rrdtool graph /srv/www/img/rrd/cpu-all-$last_x.png --title "$_STAT_SYS_cpuusage Core * - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="$_STAT_SYS_cpuusage">' \
        --vertical-label "jiffies [max. 100 per sec and core]" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        $def_str \
        COMMENT:"\n" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

cpu_graph ()
{
    last_x="$1"
    cpu_n=`echo "$2" | cut -c 4-`
    /usr/bin/rrdtool graph /srv/www/img/rrd/cpu-${cpu_n}-$last_x.png --title "$_STAT_SYS_cpuusage Core ${cpu_n} - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="$_STAT_SYS_cpuusage">' \
        --vertical-label "jiffies [max. 100 per sec]" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        DEF:interrupt=$rrd_dbpath/cpu-${cpu_n}/cpu-interrupt.rrd:value:AVERAGE \
        DEF:nice=$rrd_dbpath/cpu-${cpu_n}/cpu-nice.rrd:value:AVERAGE \
        DEF:softirq=$rrd_dbpath/cpu-${cpu_n}/cpu-softirq.rrd:value:AVERAGE \
        DEF:steal=$rrd_dbpath/cpu-${cpu_n}/cpu-steal.rrd:value:AVERAGE \
        DEF:system=$rrd_dbpath/cpu-${cpu_n}/cpu-system.rrd:value:AVERAGE \
        DEF:user=$rrd_dbpath/cpu-${cpu_n}/cpu-user.rrd:value:AVERAGE \
        DEF:wait=$rrd_dbpath/cpu-${cpu_n}/cpu-wait.rrd:value:AVERAGE \
        AREA:interrupt$rrd_col_blue:"interrupt" \
        AREA:nice$rrd_col_red:"nice     \n":STACK \
        AREA:softirq$rrd_col_red_dark:"softirq  ":STACK \
        AREA:steal$rrd_col_blue:"steal    \n":STACK \
        AREA:system$rrd_col_green:"system   ":STACK \
        AREA:user$rrd_col_green_light:"user     \n":STACK \
        AREA:wait$rrd_col_red_light:"wait     \n":STACK \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

uptime_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/uptime-$last_x.png --title "Uptime - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Uptime">' \
        -l 0 $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Tage" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:value=$rrd_dbpath/uptime/uptime.rrd:value:AVERAGE \
        CDEF:valday=value,86400,/ \
        AREA:valday$rrd_col_blue:"uptime \n" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

memory_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/memory-$last_x.png --title "Memory - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Memory">' \
        -l 0 $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Byte" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:buffered=$rrd_dbpath/memory/memory-buffered.rrd:value:AVERAGE \
        DEF:cached=$rrd_dbpath/memory/memory-cached.rrd:value:AVERAGE \
        DEF:free=$rrd_dbpath/memory/memory-free.rrd:value:AVERAGE \
        DEF:used=$rrd_dbpath/memory/memory-used.rrd:value:AVERAGE \
        AREA:used$rrd_col_red_light:"used " \
        AREA:cached$rrd_col_orange:"cached ":STACK \
        AREA:buffered$rrd_col_yellow:"buffered ":STACK \
        AREA:free$rrd_col_green_light:"free \n":STACK \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

processes_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/processes-$last_x.png --title "Processes - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Processes">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Anzahl" \
        --font "WATERMARK:8" \
        --logarithmic \
        --units=si \
        $rrd_grid_background \
        DEF:blocked=$rrd_dbpath/processes/ps_state-blocked.rrd:value:AVERAGE \
        DEF:paging=$rrd_dbpath/processes/ps_state-paging.rrd:value:AVERAGE \
        DEF:running=$rrd_dbpath/processes/ps_state-running.rrd:value:AVERAGE \
        DEF:sleeping=$rrd_dbpath/processes/ps_state-sleeping.rrd:value:AVERAGE \
        DEF:stopped=$rrd_dbpath/processes/ps_state-stopped.rrd:value:AVERAGE \
        DEF:zombies=$rrd_dbpath/processes/ps_state-zombies.rrd:value:AVERAGE \
        DEF:fork=$rrd_dbpath/processes/fork_rate.rrd:value:AVERAGE \
        LINE1:blocked$rrd_col_blue:"blocked  " \
        LINE1:paging$rrd_col_yellow:"paging  " \
        LINE1:running$rrd_col_green:"running  " \
        LINE1:sleeping$rrd_col_green_light:"sleeping " \
        LINE1:stopped$rrd_col_orange:"stopped  " \
        LINE1:zombies$rrd_col_red_light:"zombies  " \
        LINE1:fork$rrd_col_red:"fork-rate  \n" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

entropy_graph ()
{
    last_x="$1"
    ds_name=`rrdtool info $rrd_dbpath/entropy/entropy.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
    /usr/bin/rrdtool graph /srv/www/img/rrd/entropy-$last_x.png --title "Entropy - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Entropy">' \
        -l 0 $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Anzahl" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:value=$rrd_dbpath/entropy/entropy.rrd:${ds_name}:AVERAGE \
        VDEF:value_max=value,MAXIMUM \
        VDEF:value_avg=value,AVERAGE \
        VDEF:value_min=value,MINIMUM \
        VDEF:value_last=value,LAST \
        LINE1:value$rrd_col_blue:"Entropy \t" \
        GPRINT:value_max:"Maximum\: %2.0lf" \
        GPRINT:value_avg:"Durchschnitt\: %2.0lf" \
        GPRINT:value_min:"Minimum\: %2.0lf" \
        GPRINT:value_last:"Letzter\: %2.0lf\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}
