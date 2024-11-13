rrdpeerping_graph ()
{
    last_x="$1"
    peer="$2"
    if [ -f $rrd_dbpath/peerping/ping-avg-$peer.rrd ]
    then
        ds_name_avg=`rrdtool info $rrd_dbpath/peerping/ping-avg-$peer.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
        ds_name_min=`rrdtool info $rrd_dbpath/peerping/ping-min-$peer.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
        ds_name_max=`rrdtool info $rrd_dbpath/peerping/ping-max-$peer.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
        /usr/bin/rrdtool graph /srv/www/img/rrd/ping-$peer-$last_x.png --title "Peerping $peer - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Peerping $peer">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Time [ms]" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:timeavg=$rrd_dbpath/peerping/ping-avg-$peer.rrd:${ds_name_avg}:AVERAGE \
            DEF:timemin=$rrd_dbpath/peerping/ping-min-$peer.rrd:${ds_name_min}:AVERAGE \
            DEF:timemax=$rrd_dbpath/peerping/ping-max-$peer.rrd:${ds_name_max}:AVERAGE \
            VDEF:timeavg_last=timeavg,LAST \
            VDEF:timemin_last=timemin,LAST \
            VDEF:timemax_last=timemax,LAST \
            AREA:timemax#FF0000 \
            AREA:timeavg#FF9900 \
            AREA:timemin#FFFF00 \
            LINE1:timemax#FF0000:"max \: " \
            GPRINT:timemax_last:"%2.2lf ms\l" \
            LINE1:timeavg#FF9900:"avg \: " \
            GPRINT:timeavg_last:"%2.2lf ms\l" \
            LINE1:timemin#FFFF00:"min \: " \
            GPRINT:timemin_last:"%2.2lf ms\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    else
        show_error "$_MN_err" "no rrd-database for Peer $peer"
    fi
}

