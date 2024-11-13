conntrack_graph ()
{
    last_x="$1"
    if [ -f $rrd_dbpath/conntrack/conntrack.rrd ]
    then
        ds_name=`rrdtool info $rrd_dbpath/conntrack/conntrack.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
        /usr/bin/rrdtool graph /srv/www/img/rrd/conntrack-$last_x.png --title \
            "Anzahl Verbindungen(Conntrack) - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Conntrack">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Anzahl" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:conn=$rrd_dbpath/conntrack/conntrack.rrd:${ds_name}:AVERAGE \
            VDEF:conn_max=conn,MAXIMUM \
            VDEF:conn_avg=conn,AVERAGE \
            VDEF:conn_min=conn,MINIMUM \
            VDEF:conn_last=conn,LAST \
            LINE1:conn$rrd_col_green:"Verbindungen \t" \
            GPRINT:conn_max:"Maximum\: %2.0lf" \
            GPRINT:conn_avg:"Durchschnitt\: %2.0lf" \
            GPRINT:conn_min:"Minimum\: %2.0lf" \
            GPRINT:conn_last:"Letzter\: %2.0lf\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    else
        show_error "$_MN_err" "no rrd-database for Conntrack"
    fi
}

