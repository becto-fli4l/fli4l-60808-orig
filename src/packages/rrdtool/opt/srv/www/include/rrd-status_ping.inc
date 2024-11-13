rrdping_graph ()
{
    last_x="$1"
    host="$2"
    if [ -f $rrd_dbpath/ping/ping-$host.rrd ]
    then
        ds_name=`rrdtool info $rrd_dbpath/ping/ping-$host.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
        
        /usr/bin/rrdtool graph /srv/www/img/rrd/ping-$host-$last_x.png --title "Ping $host - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Ping $host">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Time [ms]" \
            --units-exponent 0 \
            --alt-y-grid \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:time=$rrd_dbpath/ping/ping-$host.rrd:${ds_name}:AVERAGE \
            VDEF:time_max=time,MAXIMUM \
            VDEF:time_avg=time,AVERAGE \
            VDEF:time_min=time,MINIMUM \
            VDEF:time_last=time,LAST \
            LINE1:time$rrd_col_green:"Time [ms]\t" \
            GPRINT:time_max:"Maximum\: %2.2lf" \
            GPRINT:time_avg:"Durchschnitt\: %2.2lf" \
            GPRINT:time_min:"Minimum\: %2.2lf" \
            GPRINT:time_last:"Letzter\: %2.2lf\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    else
        show_error "$_MN_err" "no rrd-database for host $host"
    fi
}

