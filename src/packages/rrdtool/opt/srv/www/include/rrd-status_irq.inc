irq_graph ()
{
    last_x="$1"
    irq="$2"
    if [ -f $rrd_dbpath/irq/irq-$irq.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/irq-$irq-$last_x.png --title "Interrupt $irq - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Interrupt $irq">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Anzahl" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:anz=$rrd_dbpath/irq/irq-$irq.rrd:value:AVERAGE \
            VDEF:anz_max=anz,MAXIMUM \
            VDEF:anz_avg=anz,AVERAGE \
            VDEF:anz_min=anz,MINIMUM \
            VDEF:anz_last=anz,LAST \
            LINE1:anz$rrd_col_green:"Anzahl\t" \
            GPRINT:anz_max:"Maximum\: %2.0lf" \
            GPRINT:anz_avg:"Durchschnitt\: %2.0lf" \
            GPRINT:anz_min:"Minimum\: %2.0lf" \
            GPRINT:anz_last:"Letzter\: %2.0lf\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    else
        show_error "$_MN_err" "no rrd-database for irq:$irq"
    fi
}

