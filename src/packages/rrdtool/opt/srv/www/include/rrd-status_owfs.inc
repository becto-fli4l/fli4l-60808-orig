temperature_graph ()
{
    last_x="$1"
    owfs_sensor="$2"
    if [ -f $rrd_dbpath/onewire-$owfs_sensor/temperature.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-$owfs_sensor-$last_x.png --title "OWFS-Temperatur-Sensor $owfs_sensor - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Temperatur °C" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_temp=$rrd_dbpath/onewire-$owfs_sensor/temperature.rrd:value:AVERAGE \
            VDEF:ow_temp_max=owfs_temp,MAXIMUM \
            VDEF:ow_temp_avg=owfs_temp,AVERAGE \
            VDEF:ow_temp_min=owfs_temp,MINIMUM \
            VDEF:ow_temp_last=owfs_temp,LAST \
            LINE1:owfs_temp$rrd_col_green:"TEMP \t" \
            GPRINT:ow_temp_max:"Maximum\: %2.1lf%s°C" \
            GPRINT:ow_temp_avg:"Durchschnitt\: %2.1lf%s°C" \
            GPRINT:ow_temp_min:"Minimum\: %2.1lf%s°C" \
            GPRINT:ow_temp_last:"Letzter\: %2.1lf%s\l°C" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    else
        show_error "$_MN_err" "no rrd-database for $owfs_sensor"
    fi
	
	if [ -f $rrd_dbpath/onewire-$owfs_sensor/humidity.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-humidity-$owfs_sensor-$last_x.png --title "OWFS-Luftfeuchte-Sensor $owfs_sensor - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --upper-limit 100 --lower-limit 0 \
            --vertical-label "Luftfeuchte %" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_humidity=$rrd_dbpath/onewire-$owfs_sensor/humidity.rrd:value:AVERAGE \
            VDEF:ow_humidity_max=owfs_humidity,MAXIMUM \
            VDEF:ow_humidity_avg=owfs_humidity,AVERAGE \
            VDEF:ow_humidity_min=owfs_humidity,MINIMUM \
            VDEF:ow_humidity_last=owfs_humidity,LAST \
            LINE1:owfs_humidity$rrd_col_green:"Luftfeuchte \t" \
            GPRINT:ow_humidity_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_humidity_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_humidity_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_humidity_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	if [ -f $rrd_dbpath/onewire-$owfs_sensor/humidex.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-humidex-$owfs_sensor-$last_x.png --title "OWFS-Humidex-Sensor $owfs_sensor - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Grad" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_humidex=$rrd_dbpath/onewire-$owfs_sensor/humidex.rrd:value:AVERAGE \
            VDEF:ow_humidex_max=owfs_humidex,MAXIMUM \
            VDEF:ow_humidex_avg=owfs_humidex,AVERAGE \
            VDEF:ow_humidex_min=owfs_humidex,MINIMUM \
            VDEF:ow_humidex_last=owfs_humidex,LAST \
            LINE1:owfs_humidex$rrd_col_green:"Grad \t" \
            GPRINT:ow_humidex_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_humidex_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_humidex_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_humidex_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	
	if [ -f $rrd_dbpath/onewire-$owfs_sensor/pressure.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-pressure-$owfs_sensor-$last_x.png --title "OWFS-Luftdruck-Sensor $owfs_sensor - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Luftdruck" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_pressure=$rrd_dbpath/onewire-$owfs_sensor/pressure.rrd:value:AVERAGE \
            VDEF:ow_pressure_max=owfs_pressure,MAXIMUM \
            VDEF:ow_pressure_avg=owfs_pressure,AVERAGE \
            VDEF:ow_pressure_min=owfs_pressure,MINIMUM \
            VDEF:ow_pressure_last=owfs_pressure,LAST \
            LINE1:owfs_pressure$rrd_col_green:"Luftdruck \t" \
            GPRINT:ow_pressure_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_pressure_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_pressure_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_pressure_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	
	if [ -f $rrd_dbpath/onewire-$owfs_sensor/voltage.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-voltage-$owfs_sensor-$last_x.png --title "OWFS-Spannungs-Sensor $owfs_sensor - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Volt" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_voltage=$rrd_dbpath/onewire-$owfs_sensor/voltage.rrd:value:AVERAGE \
            VDEF:ow_voltage_max=owfs_voltage,MAXIMUM \
            VDEF:ow_voltage_avg=owfs_voltage,AVERAGE \
            VDEF:ow_voltage_min=owfs_voltage,MINIMUM \
            VDEF:ow_voltage_last=owfs_voltage,LAST \
            LINE1:owfs_voltage$rrd_col_green:"Volt \t" \
            GPRINT:ow_voltage_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_voltage_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_voltage_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_voltage_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	if [ -f $rrd_dbpath/onewire-$owfs_sensor/illuminance.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-illuminance-$owfs_sensor-$last_x.png --title "OWFS-Helligkeit-Sensor $owfs_sensor - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Illuminance" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:owfs_illuminance=$rrd_dbpath/onewire-$owfs_sensor/illuminance.rrd:value:AVERAGE \
        VDEF:ow_illuminance_max=owfs_illuminance,MAXIMUM \
        VDEF:ow_illuminance_avg=owfs_illuminance,AVERAGE \
        VDEF:ow_illuminance_min=owfs_illuminance,MINIMUM \
        VDEF:ow_illuminance_last=owfs_illuminance,LAST \
        LINE1:owfs_illuminance$rrd_col_green:"Illuminance \t" \
        GPRINT:ow_illuminance_max:"Maximum\: %2.1lf%s" \
        GPRINT:ow_illuminance_avg:"Durchschnitt\: %2.1lf%s" \
        GPRINT:ow_illuminance_min:"Minimum\: %2.1lf%s" \
        GPRINT:ow_illuminance_last:"Letzter\: %2.1lf%s\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	if [ -f $rrd_dbpath/onewire-$owfs_sensor/dew_point.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-dew_point-$owfs_sensor-$last_x.png --title "OWFS-Taupunkt-Sensor $owfs_sensor - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Taupunkt Grad" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:owfs_dew_point=$rrd_dbpath/onewire-$owfs_sensor/dew_point.rrd:value:AVERAGE \
        VDEF:ow_dew_point_max=owfs_dew_point,MAXIMUM \
        VDEF:ow_dew_point_avg=owfs_dew_point,AVERAGE \
        VDEF:ow_dew_point_min=owfs_dew_point,MINIMUM \
        VDEF:ow_dew_point_last=owfs_dew_point,LAST \
        LINE1:owfs_dew_point$rrd_col_green:"Taupunkt \t" \
        GPRINT:ow_dew_point_max:"Maximum\: %2.1lf%s" \
        GPRINT:ow_dew_point_avg:"Durchschnitt\: %2.1lf%s" \
        GPRINT:ow_dew_point_min:"Minimum\: %2.1lf%s" \
        GPRINT:ow_dew_point_last:"Letzter\: %2.1lf%s\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	if [ -f $rrd_dbpath/onewire-$owfs_sensor/heat_index.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/rrd/owfs-sensors-heat_index-$owfs_sensor-$last_x.png --title "OWFS-Heat_Index-Sensor $owfs_sensor - $last_x" \
        --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_sensor">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Hitze-Index" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:owfs_heat_index=$rrd_dbpath/onewire-$owfs_sensor/heat_index.rrd:value:AVERAGE \
        VDEF:ow_heat_index_max=owfs_heat_index,MAXIMUM \
        VDEF:ow_heat_index_avg=owfs_heat_index,AVERAGE \
        VDEF:ow_heat_index_min=owfs_heat_index,MINIMUM \
        VDEF:ow_heat_index_last=owfs_heat_index,LAST \
        LINE1:owfs_heat_index$rrd_col_green:"Hitze-Index \t" \
        GPRINT:ow_heat_index_max:"Maximum\: %2.1lf%s" \
        GPRINT:ow_heat_index_avg:"Durchschnitt\: %2.1lf%s" \
        GPRINT:ow_heat_index_min:"Minimum\: %2.1lf%s" \
        GPRINT:ow_heat_index_last:"Letzter\: %2.1lf%s\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
}

temperature_grp_graph ()
{
    last_x="$1"
    owfs_grp="$2"
    owfs_grp_name=`echo "$owfs_grp" | cut -c 4-`
    owfs_path=/var/run/owfs_rrdtool_group
    def_str=""
    def_str=`echo "$def_str COMMENT:Sensorname\t\t"`
    def_str=`echo "$def_str COMMENT:Maximum\t\t"`
    def_str=`echo "$def_str COMMENT:Average\t\t"`
    def_str=`echo "$def_str COMMENT:Minimum\t\t"`
    def_str=`echo "$def_str COMMENT:Letzter\n"`
    for owfs_d_sensor in `ls -1 $owfs_path/*$owfs_grp | cut -d "-" -f1 | head -1`
    do
    owfs_d_sensor_name=`echo "$owfs_d_sensor" | cut -c 4-`
    if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-temperature.rrd ]
    then
    n=0
    for int in `ls -d $owfs_path/*$owfs_grp/*temperature.rrd | cut -d "/" -f6 | cut -d "-" -f1`
    do
        int_name=`echo "$int" | cut -c 4-`
        def_str=`echo "$def_str DEF:owfs_temp${n}=$owfs_path/$owfs_grp/$int-temperature.rrd:value:AVERAGE "`
        def_str=`echo "$def_str VDEF:ow_temp_${n}_max=owfs_temp${n},MAXIMUM "`
        def_str=`echo "$def_str VDEF:ow_temp_${n}_avg=owfs_temp${n},AVERAGE "`
        def_str=`echo "$def_str VDEF:ow_temp_${n}_min=owfs_temp${n},MINIMUM "`
        def_str=`echo "$def_str VDEF:ow_temp_${n}_last=owfs_temp${n},LAST "`
        case ${n} in
           0|8)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_green:$int_name\t "`
                ;;
            1|9)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_red:$int_name\t "`
                ;;
            2|10)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_blue:$int_name\t "`
                ;;
            3|11)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_yellow:$int_name\t "`
                ;;
            4|12)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_green_light:$int_name\t "`
                ;;
            5|13)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_red_light:$int_name\t "`
                ;;
            6|14)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_blue_light:$int_name\t "`
                ;;
            7|15)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_yellow_light:$int_name\t "`
                ;;
            *)
                def_str=`echo "$def_str LINE1:owfs_temp${n}$rrd_col_green:$int_name\t "`
                ;;
        esac
        def_str=`echo "$def_str GPRINT:ow_temp_${n}_max:%2.1lf%s°C\t\t "`
        def_str=`echo "$def_str GPRINT:ow_temp_${n}_avg:%2.1lf%s°C\t\t "`
        def_str=`echo "$def_str GPRINT:ow_temp_${n}_min:%2.1lf%s°C\t\t "`
        def_str=`echo "$def_str GPRINT:ow_temp_${n}_last:%2.1lf%s°C\n "`
        n=`expr $n + 1`
    done
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-$owfs_grp-$last_x.png --title "OWFS-Temperatur-Sensor $owfs_grp_name - $last_x" \
            --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_grp_name">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Temperatur °C" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            $def_str \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    else
        show_error "$_MN_err" "no rrd-database for $owfs_grp"
    fi
    if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-humidity.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-humidity-$owfs_d_sensor-$last_x.png --title "OWFS-Luftfeuchte-Sensor $owfs_d_sensor_name - $last_x" \
            --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_d_sensor_name">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --upper-limit 100 --lower-limit 0 \
            --vertical-label "Luftfeuchte %" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_humidity=$owfs_path/$owfs_grp/$owfs_d_sensor-humidity.rrd:value:AVERAGE \
            VDEF:ow_humidity_max=owfs_humidity,MAXIMUM \
            VDEF:ow_humidity_avg=owfs_humidity,AVERAGE \
            VDEF:ow_humidity_min=owfs_humidity,MINIMUM \
            VDEF:ow_humidity_last=owfs_humidity,LAST \
            LINE1:owfs_humidity$rrd_col_green:"Luftfeuchte \t" \
            GPRINT:ow_humidity_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_humidity_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_humidity_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_humidity_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-humidex.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-humidex-$owfs_d_sensor-$last_x.png --title "OWFS-Humidex-Sensor $owfs_d_sensor_name - $last_x" \
            --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_d_sensor_name">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Grad" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_humidex=$owfs_path/$owfs_grp/$owfs_d_sensor-humidex.rrd:value:AVERAGE \
            VDEF:ow_humidex_max=owfs_humidex,MAXIMUM \
            VDEF:ow_humidex_avg=owfs_humidex,AVERAGE \
            VDEF:ow_humidex_min=owfs_humidex,MINIMUM \
            VDEF:ow_humidex_last=owfs_humidex,LAST \
            LINE1:owfs_humidex$rrd_col_green:"Grad \t" \
            GPRINT:ow_humidex_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_humidex_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_humidex_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_humidex_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
	if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-pressure.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-pressure-$owfs_d_sensor-$last_x.png --title "OWFS-Luftdruck-Sensor $owfs_d_sensor_name - $last_x" \
            --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_d_sensor_name">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Luftdruck" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_pressure=$owfs_path/$owfs_grp/$owfs_d_sensor-pressure.rrd:value:AVERAGE \
            VDEF:ow_pressure_max=owfs_pressure,MAXIMUM \
            VDEF:ow_pressure_avg=owfs_pressure,AVERAGE \
            VDEF:ow_pressure_min=owfs_pressure,MINIMUM \
            VDEF:ow_pressure_last=owfs_pressure,LAST \
            LINE1:owfs_pressure$rrd_col_green:"Luftdruck \t" \
            GPRINT:ow_pressure_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_pressure_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_pressure_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_pressure_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi

	if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-voltage.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-voltage-$owfs_d_sensor-$last_x.png --title "OWFS-Spannungs-Sensor $owfs_d_sensor_name - $last_x" \
            --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_d_sensor_name">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Volt" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:owfs_voltage=$owfs_path/$owfs_grp/$owfs_d_sensor-voltage.rrd:value:AVERAGE \
            VDEF:ow_voltage_max=owfs_voltage,MAXIMUM \
            VDEF:ow_voltage_avg=owfs_voltage,AVERAGE \
            VDEF:ow_voltage_min=owfs_voltage,MINIMUM \
            VDEF:ow_voltage_last=owfs_voltage,LAST \
            LINE1:owfs_voltage$rrd_col_green:"Volt \t" \
            GPRINT:ow_voltage_max:"Maximum\: %2.1lf%s" \
            GPRINT:ow_voltage_avg:"Durchschnitt\: %2.1lf%s" \
            GPRINT:ow_voltage_min:"Minimum\: %2.1lf%s" \
            GPRINT:ow_voltage_last:"Letzter\: %2.1lf%s\l" \
            COMMENT:"Letztes update\:\t$rrd_last_update"
    fi

	if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-illuminance.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-illuminance-$owfs_d_sensor-$last_x.png --title "OWFS-Helligkeit-Sensor $owfs_d_sensor_name - $last_x" \
        --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_d_sensor_name">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Illuminance" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:owfs_illuminance=$owfs_path/$owfs_grp/$owfs_d_sensor-illuminance.rrd:value:AVERAGE \
        VDEF:ow_illuminance_max=owfs_illuminance,MAXIMUM \
        VDEF:ow_illuminance_avg=owfs_illuminance,AVERAGE \
        VDEF:ow_illuminance_min=owfs_illuminance,MINIMUM \
        VDEF:ow_illuminance_last=owfs_illuminance,LAST \
        LINE1:owfs_illuminance$rrd_col_green:"Illuminance \t" \
        GPRINT:ow_illuminance_max:"Maximum\: %2.1lf%s" \
        GPRINT:ow_illuminance_avg:"Durchschnitt\: %2.1lf%s" \
        GPRINT:ow_illuminance_min:"Minimum\: %2.1lf%s" \
        GPRINT:ow_illuminance_last:"Letzter\: %2.1lf%s\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
    fi

	if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-dew_point.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-dew_point-$owfs_d_sensor-$last_x.png --title "OWFS-Taupunkt-Sensor $owfs_d_sensor_name - $last_x" \
        --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_d_sensor_name">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Taupunkt Grad" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:owfs_dew_point=$owfs_path/$owfs_grp/$owfs_d_sensor-dew_point.rrd:value:AVERAGE \
        VDEF:ow_dew_point_max=owfs_dew_point,MAXIMUM \
        VDEF:ow_dew_point_avg=owfs_dew_point,AVERAGE \
        VDEF:ow_dew_point_min=owfs_dew_point,MINIMUM \
        VDEF:ow_dew_point_last=owfs_dew_point,LAST \
        LINE1:owfs_dew_point$rrd_col_green:"Taupunkt \t" \
        GPRINT:ow_dew_point_max:"Maximum\: %2.1lf%s" \
        GPRINT:ow_dew_point_avg:"Durchschnitt\: %2.1lf%s" \
        GPRINT:ow_dew_point_min:"Minimum\: %2.1lf%s" \
        GPRINT:ow_dew_point_last:"Letzter\: %2.1lf%s\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
    fi

	if [ -f $owfs_path/$owfs_grp/$owfs_d_sensor-heat_index.rrd ]
    then
        /usr/bin/rrdtool graph /srv/www/img/owfs-sensors-heat_index-$owfs_d_sensor-$last_x.png --title "OWFS-Heat_Index-Sensor $owfs_d_sensor_name - $last_x" \
        --imginfo '<IMG SRC="/img/%s" WIDTH="%lu" HEIGHT="%lu" ALT="OWFS-Sensor-Grafik $owfs_d_sensor_name">' \
        $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
        --end now --start end-${last_x} \
        --vertical-label "Hitze-Index" \
        --font "WATERMARK:8" \
        $rrd_grid_background \
        DEF:owfs_heat_index=$owfs_path/$owfs_grp/$owfs_d_sensor-heat_index.rrd:value:AVERAGE \
        VDEF:ow_heat_index_max=owfs_heat_index,MAXIMUM \
        VDEF:ow_heat_index_avg=owfs_heat_index,AVERAGE \
        VDEF:ow_heat_index_min=owfs_heat_index,MINIMUM \
        VDEF:ow_heat_index_last=owfs_heat_index,LAST \
        LINE1:owfs_heat_index$rrd_col_green:"Hitze-Index \t" \
        GPRINT:ow_heat_index_max:"Maximum\: %2.1lf%s" \
        GPRINT:ow_heat_index_avg:"Durchschnitt\: %2.1lf%s" \
        GPRINT:ow_heat_index_min:"Minimum\: %2.1lf%s" \
        GPRINT:ow_heat_index_last:"Letzter\: %2.1lf%s\l" \
        COMMENT:"Letztes update\:\t$rrd_last_update"
    fi
  done
}

