partitionboot_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/partitionboot-$last_x.png --title "Boot-Partition - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Boot-Partition">' \
        -l 0 $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Bytes" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:free=$rrd_dbpath/df-boot/df_complex-free.rrd:value:AVERAGE \
        DEF:used=$rrd_dbpath/df-boot/df_complex-used.rrd:value:AVERAGE \
        DEF:res=$rrd_dbpath/df-boot/df_complex-reserved.rrd:value:AVERAGE \
        VDEF:free_last=free,LAST \
        VDEF:res_last=res,LAST \
        VDEF:used_last=used,LAST \
        AREA:used$rrd_col_red_light:"used \t\t" \
        GPRINT:used_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:res$rrd_col_yellow:"reserved \t":STACK \
        GPRINT:res_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:free$rrd_col_green:"free \t\t":STACK \
        GPRINT:free_last:"Letzter\: %4.0lf %SByte\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

partitiondata_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/partitiondata-$last_x.png --title "Data-Partition - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Data-Partition">' \
        -l 0 $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Bytes" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:free=$rrd_dbpath/df-data/df_complex-free.rrd:value:AVERAGE \
        DEF:used=$rrd_dbpath/df-data/df_complex-used.rrd:value:AVERAGE \
        DEF:res=$rrd_dbpath/df-data/df_complex-reserved.rrd:value:AVERAGE \
        VDEF:free_last=free,LAST \
        VDEF:res_last=res,LAST \
        VDEF:used_last=used,LAST \
        AREA:used$rrd_col_red_light:"used \t\t" \
        GPRINT:used_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:res$rrd_col_yellow:"reserved \t":STACK \
        GPRINT:res_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:free$rrd_col_green:"free \t\t":STACK \
        GPRINT:free_last:"Letzter\: %4.0lf %SByte\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

partitionroot_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/partitionroot-$last_x.png --title "ROOT-Partition - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="ROOT-Partition">' \
        -l 0 $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Bytes" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:free=$rrd_dbpath/df-root/df_complex-free.rrd:value:AVERAGE \
        DEF:used=$rrd_dbpath/df-root/df_complex-used.rrd:value:AVERAGE \
        DEF:res=$rrd_dbpath/df-root/df_complex-reserved.rrd:value:AVERAGE \
        VDEF:free_last=free,LAST \
        VDEF:res_last=res,LAST \
        VDEF:used_last=used,LAST \
        AREA:used$rrd_col_red_light:"used \t\t" \
        GPRINT:used_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:res$rrd_col_yellow:"reserved \t":STACK \
        GPRINT:res_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:free$rrd_col_green:"free \t\t":STACK \
        GPRINT:free_last:"Letzter\: %4.0lf %SByte\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

partitionopt_graph ()
{
    last_x="$1"
    /usr/bin/rrdtool graph /srv/www/img/rrd/partitionopt-$last_x.png --title "OPT-Partition - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OPT-Partition">' \
        -l 0 $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Bytes" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:free=$rrd_dbpath/df-opt/df_complex-free.rrd:value:AVERAGE \
        DEF:used=$rrd_dbpath/df-opt/df_complex-used.rrd:value:AVERAGE \
        DEF:res=$rrd_dbpath/df-opt/df_complex-reserved.rrd:value:AVERAGE \
        VDEF:free_last=free,LAST \
        VDEF:res_last=res,LAST \
        VDEF:used_last=used,LAST \
        AREA:used$rrd_col_red_light:"used \t\t" \
        GPRINT:used_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:res$rrd_col_yellow:"reserved \t":STACK \
        GPRINT:res_last:"Letzter\: %4.0lf %SByte\l" \
        AREA:free$rrd_col_green:"free \t\t":STACK \
        GPRINT:free_last:"Letzter\: %4.0lf %SByte\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
}

