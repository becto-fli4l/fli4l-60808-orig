# $Id$

extra_cpufreq_governor ()
{
    local cpu=$1

    show_tab_header $(translate_label "${_HWSUPP_CPUFREQ_GOV}") no

    echo "<table class='normtable'>"
    local governor=`cat /sys/devices/system/cpu/${cpu}/cpufreq/scaling_governor`
    local available=`cat /sys/devices/system/cpu/${cpu}/cpufreq/scaling_available_governors`
    for g in performance ondemand conservative powersave userspace
    do
        if echo $available | grep -q $g
        then
            if [ "${governor}" = "${g}" ]
            then
                local label=$(translate_label "${_HWSUPP_CPUFREQ_ACTIVE}")
                echo "<tr class=\"odd\"><td>${g}</td><td>$label</td></tr>"
            else
                local label=$(translate_label "${_HWSUPP_CPUFREQ_ACTIVATE}")
                echo -n "<tr><td>${g}</td><td>"
                echo -n "<a href=\"hwsupp.cgi?action=${FORM_action}&amp;sub=${FORM_sub}&amp;command=governor&amp;activate=${g}\">"
                echo "<div>$label</div></a></td></tr>"
            fi
        fi
    done
    echo "</table>"

    show_tab_footer
}

extra_cpufreq_setspeed ()
{
    local cpu=$1
    
    local governor=`cat /sys/devices/system/cpu/${cpu}/cpufreq/scaling_governor`

    if [ "${governor}" = "userspace" ]
    then
        show_tab_header $(translate_label "${_HWSUPP_CPUFREQ_FREQ}") no
    
        echo "<table class='normtable'>"
        local label=$(translate_label "${_HWSUPP_CPUFREQ_MHZ}")
        echo "<tr><th>${label}</th><th/></tr>"
        
        local freq=`cat /sys/devices/system/cpu/${cpu}/cpufreq/scaling_setspeed`
        local available=`cat /sys/devices/system/cpu/${cpu}/cpufreq/scaling_available_frequencies`
        for f in ${available}
        do
            if [ "${freq}" = "${f}" ]
            then
                local label=$(translate_label "${_HWSUPP_CPUFREQ_ACTIVE}")
                echo "<tr class=\"odd\"><td>$((${f} / 1000))</td><td>$label</td></tr>"
            else
                local label=$(translate_label "${_HWSUPP_CPUFREQ_ACTIVATE}")
                echo -n "<tr><td>$((${f} / 1000))</td><td>"
                echo -n "<a href=\"hwsupp.cgi?action=${FORM_action}&amp;sub=${FORM_sub}&amp;command=setspeed&amp;activate=${f}\">"
                echo "<div>$label</div></a></td></tr>"
            fi
        done
        echo "</table>"
    
        show_tab_footer
    fi
}

extra_cpufreq_stat_line ()
{
    echo "<tr>"
    echo "<td>$(($1 / 1000))</td>"
    echo "<td style=\"text-align: right;\">$2</td>"
    echo "</tr>"
}

extra_cpufreq_stat ()
{
    local cpu=$1
    if [ -e /sys/devices/system/cpu/${cpu}/cpufreq/stats/time_in_state ]
    then
        local ts=`cat /sys/devices/system/cpu/${cpu}/cpufreq/stats/time_in_state`
        show_tab_header $(translate_label "${_HWSUPP_CPUFREQ_STAT}") no

        echo "<table class='normtable'>"
        local label=$(translate_label "${_HWSUPP_CPUFREQ_MHZ}")
        echo "<tr><th>${label}</th>"
        local label=$(translate_label "${_HWSUPP_CPUFREQ_TIME}")
        echo "<th>${label}</th></tr>"
        
        echo "${ts}" | while read line
        do
            extra_cpufreq_stat_line $line
        done
        
        echo "</table>"

        show_tab_footer
    fi
}

extra_cpufreq_trans_head ()
{
    local label=$(translate_label "${_HWSUPP_CPUFREQ_TO}")
    echo "<tr><th></th><th colspan=0>${label}</th><tr>"
    local label=$(translate_label "${_HWSUPP_CPUFREQ_FROM}")
    echo "<tr><th>${label}</th>"

    for i in $*
    do
        echo "<td style=\"width: 50px;\">$(($i / 1000))</td>"
    done

    echo "</tr>"
}

extra_cpufreq_trans_line ()
{
    local freq=`echo $1 | cut -d: -f1`
    local val=`echo $1 | cut -d: -f2`
    echo "<tr>"
    echo "<td>$(($freq / 1000))</td>"
    for i in $val
    do
        echo "<td style=\"text-align: right;\">$i</td>"
    done

    echo "</tr>"
}

extra_cpufreq_trans ()
{
    local cpu=$1
    if [ -e /sys/devices/system/cpu/${cpu}/cpufreq/stats/trans_table ]
    then
        local trans=`cat /sys/devices/system/cpu/${cpu}/cpufreq/stats/trans_table | sed '1,1d'`
        show_tab_header $(translate_label "${_HWSUPP_CPUFREQ_TRANS}") no

        echo "<table class='normtable'>"
        echo "${trans}" | sed '1q' | cut -d: -f2 |  while read line
        do 
            extra_cpufreq_trans_head "$line"
        done
        echo "${trans}" | sed '1,1d' | while read line
        do
            extra_cpufreq_trans_line "$line"
        done
        echo "</table>"

        show_tab_footer
    fi
}

extra_cpufreq ()
{
    cpus=`ls /sys/bus/cpu/devices | sed 's/cpu//g'`
    : ${FORM_sub:=cpu0}
    : ${FORM_command:=}
    local sub_list=""
    for cpu in $cpus
    do
        sub_label=$(translate_label "${_HWSUPP_RC_CPU} ${cpu}")
        if [ "$FORM_sub" = "cpu${cpu}" ]
        then
            sub_list=`echo "$sub_list $sub_label no"`
        else
            sub_list=`echo "$sub_list $sub_label $myname?action=$FORM_action&sub=cpu${cpu}"`
        fi
    done

    show_tab_header $sub_list

    case $FORM_command in
        governor)
            if [ -n $FORM_activate ]
            then
                echo $FORM_activate > /sys/devices/system/cpu/${FORM_sub}/cpufreq/scaling_governor
            fi
        ;;
        setspeed)
            if [ -n $FORM_activate ]
            then
                echo $FORM_activate > /sys/devices/system/cpu/${FORM_sub}/cpufreq/scaling_setspeed
            fi
        ;;
    esac
    extra_cpufreq_governor ${FORM_sub}
    extra_cpufreq_setspeed ${FORM_sub}
    extra_cpufreq_stat ${FORM_sub}
    extra_cpufreq_trans ${FORM_sub}
    
    show_tab_footer
}
# _oOo_