# Creation:     20.05.2020 chfritsch

generic_acpi_coretemp_frequency_graph ()
{
    last_x="$1"
    if [ -f $rrd_dbpath/cpufreq/cpufreq-0.rrd ]
    then
        max_label=$(printf "%-11.11s" "${_MN_rrd_max}")
        avg_label=$(printf "%-11.11s" "${_MN_rrd_avg}")
        min_label=$(printf "%-11.11s" "${_MN_rrd_min}")
        last_label=$(printf "%-11.11s" "${_MN_rrd_last}")
        update_label=$(printf "%-15.15s" "${_MN_rrd_update}")
        var1_label=$(printf "%-15.15s" "${_HWSUPP_RC_CPU} 0")
        /usr/bin/rrdtool graph /srv/www/img/rrd/generic_acpi_coretemp-frequency-$last_x.png --title "GENERIC-ACPI (coretemp) - ${_HWSUPP_RT_FREQUENCY} - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="FSC Futro S900 - ${_HWSUPP_RT_FREQUENCY}">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "${_HWSUPP_RL_FREQUENCY}" \
            --units-exponent 0 \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:var1_1=${rrd_dbpath}/cpufreq/cpufreq-0.rrd:value:AVERAGE \
            CDEF:var1=var1_1,1000000,/ \
            VDEF:var1_max=var1,MAXIMUM \
            VDEF:var1_avg=var1,AVERAGE \
            VDEF:var1_min=var1,MINIMUM \
            VDEF:var1_last=var1,LAST \
            COMMENT:"\t\t\t\g" \
            COMMENT:"${max_label}\t\g" \
            COMMENT:"${avg_label}\t\g" \
            COMMENT:"${min_label}\t\g" \
            COMMENT:"${last_label}\l" \
            LINE1:var1#FF0000:"${var1_label}\t\g" \
            GPRINT:var1_max:"%.1lf\t\t\g" \
            GPRINT:var1_avg:"%.1lf\t\t\g" \
            GPRINT:var1_min:"%.1lf\t\t\g" \
            GPRINT:var1_last:"%.1lf\l" \
            COMMENT:"\n" \
            COMMENT:"${update_label}\t${rrd_last_update}"
    else
        show_error "$_MN_err" "no rrd-database for GENERIC-ACPI-CORETEMP ${_HWSUPP_RT_FREQUENCY}"
    fi
}

generic_acpi_coretemp_coretemp_graph ()
{
    last_x="$1"
    if [ -f $rrd_dbpath/sensors-k10temp-pci-00c3/temperature-temp1.rrd ]
    then
        max_label=$(printf "%-11.11s" "${_MN_rrd_max}")
        avg_label=$(printf "%-11.11s" "${_MN_rrd_avg}")
        min_label=$(printf "%-11.11s" "${_MN_rrd_min}")
        last_label=$(printf "%-11.11s" "${_MN_rrd_last}")
        update_label=$(printf "%-15.15s" "${_MN_rrd_update}")
        var_temp2_label=$(printf "%-15.15s" "${_HWSUPP_RC_CPU} 0")
        /usr/bin/rrdtool graph /srv/www/img/rrd/generic_acpi_coretemp-coretemp-$last_x.png --title "GENERIC-ACPI (coretemp) - Core-${_HWSUPP_TEMP} - $last_x" \
            --imginfo '<IMG SRC="/img/rrd/%s" WIDTH="%lu" HEIGHT="%lu" ALT="FSC Futro S900 - Core-${_HWSUPP_TEMP}">' \
            $rrd_width $rrd_height $rrd_common_opts --watermark "$rrd_watermark" \
            --end now --start end-${last_x} \
            --vertical-label "Core-${_HWSUPP_RL_TEMP}" \
            --font "WATERMARK:8" \
            $rrd_grid_background \
            DEF:var_temp2=${rrd_dbpath}/sensors-k10temp-pci-00c3/temperature-temp1.rrd:value:AVERAGE \
            VDEF:var_temp2_max=var_temp2,MAXIMUM \
            VDEF:var_temp2_avg=var_temp2,AVERAGE \
            VDEF:var_temp2_min=var_temp2,MINIMUM \
            VDEF:var_temp2_last=var_temp2,LAST \
            COMMENT:"\t\t\t\g" \
            COMMENT:"${max_label}\t\g" \
            COMMENT:"${avg_label}\t\g" \
            COMMENT:"${min_label}\t\g" \
            COMMENT:"${last_label}\l" \
            LINE1:var_temp2#FF0000:"${var_temp2_label}\t\g" \
            GPRINT:var_temp2_max:"%.1lf\t\t\g" \
            GPRINT:var_temp2_avg:"%.1lf\t\t\g" \
            GPRINT:var_temp2_min:"%.1lf\t\t\g" \
            GPRINT:var_temp2_last:"%.1lf\l" \
            COMMENT:"\n" \
            COMMENT:"${update_label}\t${rrd_last_update}"
    else
        show_error "$_MN_err" "no rrd-database for GENERIC-ACPI-CORETEMP Core-${_HWSUPP_TEMP}"
    fi
}

# _oOo_
