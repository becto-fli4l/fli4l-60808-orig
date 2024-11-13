qosup_graph ()
{
    last_x="$1"
    class="$2"
    rrddb="$rrd_dbpath/netlink-imq1/ipt_bytes-class-htb-20_${class}.rrd"
    if [ -f $rrddb ]
    then
        eval label='$QOS_CLASS_'$class'_LABEL'
        if [ "x$label" != "x" ]
        then
            titel=`echo "CLASS:$class [$label]"`
        else
            titel=`echo "CLASS:$class"`
        fi
        ln -s "$rrddb" /tmp/qos.rrd
        /usr/bin/rrdtool graph /srv/www/img/rrd/qosup-class-$class-$last_x.png --title "QOS Upload - ${titel} - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="QOS-Graph">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Bytes/s" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:var=/tmp/qos.rrd:value:AVERAGE \
            VDEF:var_max=var,MAXIMUM \
            VDEF:var_avg=var,AVERAGE \
            VDEF:var_min=var,MINIMUM \
            VDEF:var_last=var,LAST \
            LINE1:var$rrd_col_green:"Bytes/s \t" \
            GPRINT:var_max:"Maximum\: %2.1lf%s" \
            GPRINT:var_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:var_min:"Minimum\: %2.1lf%s" \
            GPRINT:var_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
        rm /tmp/qos.rrd
    else
        show_error "$_MN_err" "no rrd-database for QOS up class ${class}"
    fi
}

qosdown_graph ()
{
    last_x="$1"
    class="$2"
    rrddb="$rrd_dbpath/netlink-imq0/ipt_bytes-class-htb-10_${class}.rrd"
    if [ -f $rrddb ]
    then
        eval label='$QOS_CLASS_'$class'_LABEL'
        if [ "x$label" != "x" ]
        then
            titel=`echo "CLASS:$class [$label]"`
        else
            titel=`echo "CLASS:$class"`
        fi
        ln -s "$rrddb" /tmp/qos.rrd
        /usr/bin/rrdtool graph /srv/www/img/rrd/qosdown-class-$class-$last_x.png --title "QOS Download - ${titel} - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="QOS-Graph">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Bytes/s" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:var=/tmp/qos.rrd:value:AVERAGE \
            VDEF:var_max=var,MAXIMUM \
            VDEF:var_avg=var,AVERAGE \
            VDEF:var_min=var,MINIMUM \
            VDEF:var_last=var,LAST \
            LINE1:var$rrd_col_green:"Bytes/s \t" \
            GPRINT:var_max:"Maximum\: %2.1lf%s" \
            GPRINT:var_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:var_min:"Minimum\: %2.1lf%s" \
            GPRINT:var_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
        rm /tmp/qos.rrd
    else
        show_error "$_MN_err" "no rrd-database for QOS down class ${class}"
    fi
}

