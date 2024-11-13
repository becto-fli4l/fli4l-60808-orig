ntp_main_graph ()
{
    last_x="$1"
    ds_name_error=`rrdtool info $rrd_dbpath/ntpd/time_offset-error.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
    ds_name_loop=`rrdtool info $rrd_dbpath/ntpd/time_offset-loop.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
    
    /usr/bin/rrdtool graph /srv/www/img/rrd/ntp-main-$last_x.png --title "NTP Gesamtuebersicht - $last_x" \
	--imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="NTP Gesamtuebersicht">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
	--end now --start end-${last_x} \
	--vertical-label "Sekunden" \
	DEF:time_offset_error=$rrd_dbpath/ntpd/time_offset-error.rrd:${ds_name_error}:AVERAGE \
	DEF:time_offset_loop=$rrd_dbpath/ntpd/time_offset-loop.rrd:${ds_name_loop}:AVERAGE \
	VDEF:time_offset_error_max=time_offset_error,MAXIMUM \
	VDEF:time_offset_error_avg=time_offset_error,AVERAGE \
	VDEF:time_offset_error_min=time_offset_error,MINIMUM \
	VDEF:time_offset_error_last=time_offset_error,LAST \
	VDEF:time_offset_loop_max=time_offset_loop,MAXIMUM \
	VDEF:time_offset_loop_avg=time_offset_loop,AVERAGE \
	VDEF:time_offset_loop_min=time_offset_loop,MINIMUM \
	VDEF:time_offset_loop_last=time_offset_loop,LAST \
	COMMENT:"                 \t\t\g" \
	COMMENT:"     Maximum" \
	COMMENT:"Durchschnitt" \
	COMMENT:"     Minimum" \
	COMMENT:"     Letzter\l" \
	LINE1:time_offset_error$rrd_col_blue_light:"time-offset-error\t\t\g" \
	GPRINT:time_offset_error_max:"  % 8.3lf s" \
	GPRINT:time_offset_error_avg:"  % 8.3lf s" \
	GPRINT:time_offset_error_min:"  % 8.3lf s" \
	GPRINT:time_offset_error_last:"  % 8.3lf s\l" \
	LINE1:time_offset_loop$rrd_col_red_light:"time-offset-loop\t\t\g" \
	GPRINT:time_offset_loop_max:"  % 8.3lf s" \
	GPRINT:time_offset_loop_avg:"  % 8.3lf s" \
	GPRINT:time_offset_loop_min:"  % 8.3lf s" \
	GPRINT:time_offset_loop_last:"  % 8.3lf s\l" \
	COMMENT:"Letztes Update\:\t$rrd_last_update"
}

ntp_frequency_graph ()
{
    last_x="$1"
    ds_name=`rrdtool info $rrd_dbpath/ntpd/frequency_offset-loop.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
    
    /usr/bin/rrdtool graph /srv/www/img/rrd/ntp-frequency-$last_x.png --title "NTP Frequenz in ppm - $last_x" \
	--imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="NTP Frequenz in ppm">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
	--end now --start end-${last_x} \
	--vertical-label "Frequenz" \
	DEF:frequency=$rrd_dbpath/ntpd/frequency_offset-loop.rrd:${ds_name}:AVERAGE \
	VDEF:frequency_max=frequency,MAXIMUM \
	VDEF:frequency_avg=frequency,AVERAGE \
	VDEF:frequency_min=frequency,MINIMUM \
	VDEF:frequency_last=frequency,LAST \
	COMMENT:"         \t\t\g" \
	COMMENT:"     Maximum" \
	COMMENT:"Durchschnitt" \
	COMMENT:"     Minimum" \
	COMMENT:"     Letzter\l" \
	LINE1:frequency$rrd_col_red_light:"frequency\t\t\g" \
	GPRINT:frequency_max:" % 7.1lf ppm" \
	GPRINT:frequency_avg:" % 7.1lf ppm" \
	GPRINT:frequency_min:" % 7.1lf ppm" \
	GPRINT:frequency_last:" % 7.1lf ppm\l" \
	COMMENT:"Letztes Update\: $rrd_last_update"
}

ntp_generic_graph ()
{
    last_x="$1"
    local clockname=$2

    # ntp plugin does not provide "offset" values for LOCAL clock as they are
    # always zero, so check for that special case
    if [ -f "$rrd_dbpath/ntpd/time_offset-$clockname.rrd" ]
    then
        ds_name_dispersion=`rrdtool info $rrd_dbpath/ntpd/time_dispersion-$clockname.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
        ds_name_offset=`rrdtool info $rrd_dbpath/ntpd/time_offset-$clockname.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
	/usr/bin/rrdtool graph /srv/www/img/rrd/ntp-$clockname-$last_x.png --title "$clockname Offset und Dispersion in Sekunden - $last_x" \
	    --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="$clockname Offset und Dispersion in Sekunden">' \
	    $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
	    --end now --start end-${last_x} \
	    --vertical-label "Sekunden" \
	    DEF:dispersion=$rrd_dbpath/ntpd/time_dispersion-$clockname.rrd:${ds_name_dispersion}:AVERAGE \
	    DEF:offset=$rrd_dbpath/ntpd/time_offset-$clockname.rrd:${ds_name_offset}:AVERAGE \
	    VDEF:dispersion_max=dispersion,MAXIMUM \
	    VDEF:dispersion_avg=dispersion,AVERAGE \
	    VDEF:dispersion_min=dispersion,MINIMUM \
	    VDEF:dispersion_last=dispersion,LAST \
	    VDEF:offset_max=offset,MAXIMUM \
	    VDEF:offset_avg=offset,AVERAGE \
	    VDEF:offset_min=offset,MINIMUM \
	    VDEF:offset_last=offset,LAST \
	    COMMENT:"          \t\t\g" \
	    COMMENT:"     Maximum" \
	    COMMENT:"Durchschnitt" \
	    COMMENT:"     Minimum" \
	    COMMENT:"     Letzter\l" \
	    LINE1:dispersion$rrd_col_red_light:"dispersion\t\t\g" \
	    GPRINT:dispersion_max:"   % 7.2lf s" \
	    GPRINT:dispersion_avg:"   % 7.2lf s" \
	    GPRINT:dispersion_min:"   % 7.2lf s" \
	    GPRINT:dispersion_last:"   % 7.2lf s\l" \
	    LINE1:offset$rrd_col_blue_light:"offset    \t\t\g" \
	    GPRINT:offset_max:"   % 7.2lf s" \
	    GPRINT:offset_avg:"   % 7.2lf s" \
	    GPRINT:offset_min:"   % 7.2lf s" \
	    GPRINT:offset_last:"   % 7.2lf s\l" \
	    COMMENT:"Letztes Update\: $rrd_last_update"
    else
       ds_name_dispersion=`rrdtool info $rrd_dbpath/ntpd/time_dispersion-$clockname.rrd |grep -e "^ds.*type" | cut -d "[" -f2 | cut -d "]" -f1`
	/usr/bin/rrdtool graph /srv/www/img/rrd/ntp-$clockname-$last_x.png --title "$clockname Dispersion in Sekunden - $last_x" \
	    --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="$clockname Dispersion in Sekunden">' \
	    $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
	    --end now --start end-${last_x} \
	    --vertical-label "Sekunden" \
	    DEF:dispersion=$rrd_dbpath/ntpd/time_dispersion-$clockname.rrd:${ds_name_dispersion}:AVERAGE \
	    VDEF:dispersion_max=dispersion,MAXIMUM \
	    VDEF:dispersion_avg=dispersion,AVERAGE \
	    VDEF:dispersion_min=dispersion,MINIMUM \
	    VDEF:dispersion_last=dispersion,LAST \
	    COMMENT:"          \t\t\g" \
	    COMMENT:"     Maximum" \
	    COMMENT:"Durchschnitt" \
	    COMMENT:"     Minimum" \
	    COMMENT:"     Letzter\l" \
	    LINE1:dispersion$rrd_col_red_light:"dispersion\t\t\g" \
	    GPRINT:dispersion_max:"   % 7.2lf s" \
	    GPRINT:dispersion_avg:"   % 7.2lf s" \
	    GPRINT:dispersion_min:"   % 7.2lf s" \
	    GPRINT:dispersion_last:"   % 7.2lf s\l" \
	    COMMENT:"Letztes Update\: $rrd_last_update"
    fi
}
