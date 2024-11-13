network_graph ()
{
    last_x="$1"
    net_dev="$2"
    if [ -f $rrd_dbpath/netlink-$net_dev/if_octets.rrd ]
    then
        . /etc/boot.d/netalias.inc
        alias_name=""
        net_alias_lookup_name $net_dev alias_name
        if [ "x$alias_name" != "x" ]
        then
            dname=`echo "${alias_name} [$net_dev]"`
        else
            dname=$net_dev
        fi
        /usr/bin/rrdtool graph /srv/www/img/rrd/network-$net_dev-$last_x.png --title "Network - $dname - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="Network">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Bytes/s" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:rx=$rrd_dbpath/netlink-$net_dev/if_octets.rrd:rx:AVERAGE \
            DEF:tx=$rrd_dbpath/netlink-$net_dev/if_octets.rrd:tx:AVERAGE \
            VDEF:rx_max=rx,MAXIMUM \
            VDEF:rx_avg=rx,AVERAGE \
            VDEF:rx_min=rx,MINIMUM \
            VDEF:rx_last=rx,LAST \
            VDEF:tx_max=tx,MAXIMUM \
            VDEF:tx_avg=tx,AVERAGE \
            VDEF:tx_min=tx,MINIMUM \
            VDEF:tx_last=tx,LAST \
            LINE1:rx$rrd_col_green:"RX \t" \
            GPRINT:rx_max:"Maximum\: %2.1lf%s" \
            GPRINT:rx_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:rx_min:"Minimum\: %2.1lf%s" \
            GPRINT:rx_last:"Letzter\: %2.1lf%s\l" \
            LINE1:tx$rrd_col_red_light:"TX \t" \
            GPRINT:tx_max:"Maximum\: %2.1lf%s" \
            GPRINT:tx_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:tx_min:"Minimum\: %2.1lf%s" \
            GPRINT:tx_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    else
        show_error "$_MN_err" "no rrd-database for $net_dev"
    fi
}

