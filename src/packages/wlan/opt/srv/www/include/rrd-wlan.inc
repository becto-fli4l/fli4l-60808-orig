wlan_graph ()
{
    last_x="$1"
    if [ -f $rrd_dbpath/wireless-$dev/signal_noise.rrd -a \
         -f $rrd_dbpath/wireless-$dev/signal_power.rrd -a \
         -f $rrd_dbpath/wireless-$dev/signal_quality.rrd ]
    then
        max_label=$(printf "%-11.11s" "${_MN_rrd_max}")
        avg_label=$(printf "%-11.11s" "${_MN_rrd_avg}")
        min_label=$(printf "%-11.11s" "${_MN_rrd_min}")
        last_label=$(printf "%-11.11s" "${_MN_rrd_last}")
        update_label=$(printf "%-15.15s" "${_MN_rrd_update}")
        var1_label=$(printf "%-15.15s" "${_WLAN_NOISE}")
        var2_label=$(printf "%-15.15s" "${_WLAN_POWER}")
        var3_label=$(printf "%-15.15s" "${_WLAN_QUALITY}")
        /usr/bin/rrdtool graph /srv/www/img/rrd/wireless-$dev-$last_x.png --title "WLAN ${_WLAN_STATISTICS} - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="WLAN ${_WLAN_STATISTICS}">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "${_WLAN_LABEL}" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:var1=${rrd_dbpath}/wireless-${dev}/signal_noise.rrd:value:AVERAGE \
            DEF:var2=${rrd_dbpath}/wireless-${dev}/signal_power.rrd:value:AVERAGE \
            DEF:var3=${rrd_dbpath}/wireless-${dev}/signal_quality.rrd:value:AVERAGE \
            VDEF:var1_max=var1,MAXIMUM \
            VDEF:var1_avg=var1,AVERAGE \
            VDEF:var1_min=var1,MINIMUM \
            VDEF:var1_last=var1,LAST \
            VDEF:var2_max=var2,MAXIMUM \
            VDEF:var2_avg=var2,AVERAGE \
            VDEF:var2_min=var2,MINIMUM \
            VDEF:var2_last=var2,LAST \
            VDEF:var3_max=var3,MAXIMUM \
            VDEF:var3_avg=var3,AVERAGE \
            VDEF:var3_min=var3,MINIMUM \
            VDEF:var3_last=var3,LAST \
            COMMENT:"\t\t\t\g" \
            COMMENT:"${max_label}\t\g" \
            COMMENT:"${avg_label}\t\g" \
            COMMENT:"${min_label}\t\g" \
            COMMENT:"${last_label}\l" \
            LINE1:var1#FF0000:"${var1_label}\t\g" \
            GPRINT:var1_max:"%.2lf\t\t\g" \
            GPRINT:var1_avg:"%.2lf\t\t\g" \
            GPRINT:var1_min:"%.2lf\t\t\g" \
            GPRINT:var1_last:"%.2lf\l" \
            LINE1:var2#FFFF00:"${var2_label}\t\g" \
            GPRINT:var2_max:"%.2lf\t\t\g" \
            GPRINT:var2_avg:"%.2lf\t\t\g" \
            GPRINT:var2_min:"%.2lf\t\t\g" \
            GPRINT:var2_last:"%.2lf\l" \
            LINE1:var3#00FF00:"${var3_label}\t\g" \
            GPRINT:var3_max:"%.2lf\t\t\g" \
            GPRINT:var3_avg:"%.2lf\t\t\g" \
            GPRINT:var3_min:"%.2lf\t\t\g" \
            GPRINT:var3_last:"%.2lf\l" \
            COMMENT:"\n" \
            COMMENT:"${update_label}\t${rrd_last_update}"
    else
        show_error "$_MN_err" "no rrd-database for WLAN ${_WLAN_STATISTICS}"
    fi
}
