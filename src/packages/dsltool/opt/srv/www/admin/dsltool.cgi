#!/bin/sh
#------------------------------------------------------------------------------
# opt/files/srv/www/admin/dsltool.cgi                      __FLI4LVER__
#
# Creation:     06.02.2013 cspiess
# Last Update:  $Id$
#------------------------------------------------------------------------------


. /srv/www/include/cgi-helper
#set_debug=yes

## get some internal variables
. /var/run/dsltool.conf


# Security
check_rights "dsltool" "view"

#globals
: ${FORM_action:=dsltool_info}
: ${FORM_idx:=1}

# per instance variables
eval dsltoolhost='$DSLTOOL_'$FORM_idx'_HOST'
eval dsltoolrrd='$DSLTOOL_'$FORM_idx'_RRD'
eval dsltooldebug='$DSLTOOL_'$FORM_idx'_DEBUG'

if [ "$dsltoolrrd" = "yes" ]
then
    . /srv/www/include/rrd-common.inc
    . /srv/www/include/rrd-dsltool.inc

    _dsltool_modemstate="dsltool_modemstate"
    _dsltool_attenuation="dsltool_attenuation"
    _dsltool_noisemargin="dsltool_noisemargin"
    _dsltool_txpower="dsltool_txpower"
    _dsltool_bandwidth="dsltool_bandwidth"
    _dsltool_errors="dsltool_errors"
    _dsltool_failures="dsltool_failures"

    : ${FORM_rrd_graphtime_dsltool_modemstate:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_dsltool_attenuation:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_dsltool_noisemargin:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_dsltool_txpower:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_dsltool_bandwidth:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_dsltool_errors:=$rrd_default_graphtime}
    : ${FORM_rrd_graphtime_dsltool_failures:=$rrd_default_graphtime}
fi

# functions
dsltool_info_status ()
{
    show_tab_header "$_DSLTOOL_INFO_STATUS" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th>$_DSLTOOL_INFO_DSLTYPE</th>
        <td>$dsltype</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_MODEMSTATE</th>
        <td>$modemstate_str</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_OPERATIONMODE</th>
        <td>$operationmode</td>
    </tr>
EOF
    case "$dsltype" in
        "ADSL"|"ADSL2"|"ADSL2+")
cat <<EOF
    <tr>
        <th></th>
        <td>$channelmode</td>
    </tr>
EOF
        ;;
        "VDSL"|"VDSL2")
cat <<EOF
    <tr>
        <th>$_DSLTOOL_INFO_PROFILE</th>
        <td>$profile</td>
    </tr>
EOF
        ;;
    esac
cat <<EOF
</table>
EOF
    show_tab_footer
}

dsltool_info_line ()
{
    case "$dsltype" in
        "ADSL"|"ADSL2"|"ADSL2+")
            show_tab_header "$_DSLTOOL_INFO_LINE" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th></th>
        <th>$_DSLTOOL_INFO_BITRATE</th>
        <th>$_DSLTOOL_INFO_NOISEMARGIN</th>
        <th>$_DSLTOOL_INFO_ATTENUATION</th>
        <th>$_DSLTOOL_INFO_TXPOWER</th>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_UPSTREAM</th>
        <td align='center'>$bandwidth_kbit_up</td>
        <td align='center'>$noisemargin_up</td>
        <td align='center'>$attenuation_up</td>
        <td align='center'>$txpower_up</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_DOWNSTREAM</th>
        <td align='center'>$bandwidth_kbit_down</td>
        <td align='center'>$noisemargin_down</td>
        <td align='center'>$attenuation_down</td>
        <td align='center'>$txpower_down</td>
    </tr>
</table>
EOF
            show_tab_footer
        ;;
        "VDSL"|"VDSL2")
            show_tab_header "$_DSLTOOL_INFO_LINE" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th></th>
        <th>$_DSLTOOL_INFO_BITRATE</th>
        <th>max. $_DSLTOOL_INFO_BITRATE</th>
        <th>$_DSLTOOL_INFO_TXPOWER</th>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_UPSTREAM</th>
        <td align='center'>$bandwidth_kbit_up</td>
        <td align='center'>$maxbandwidth_kbit_up</td>
        <td align='center'>$txpower_up</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_DOWNSTREAM</th>
        <td align='center'>$bandwidth_kbit_down</td>
        <td align='center'>$maxbandwidth_kbit_down</td>
        <td align='center'>$txpower_down</td>
    </tr>
</table>
<table class='normtable'>
    <tr>
        <th></th>
        <th>$_DSLTOOL_INFO_NOISEMARGIN</th>
        <th>$_DSLTOOL_INFO_ATTENUATION</th>
    </tr>
EOF
            for i in `seq 0 5`
            do
                if [ $dslband -gt $i ]
                then
                    if [ $i -eq 0 ]
                    then
                        eval nm_up='$noisemargin_up'
                        eval at_up='$attenuation_up'
                        eval nm_down='$noisemargin_down'
                        eval at_down='$attenuation_down'
                    else
                        eval nm_up='$noisemargin'$i'_up'
                        eval at_up='$attenuation'$i'_up'
                        eval nm_down='$noisemargin'$i'_down'
                        eval at_down='$attenuation'$i'_down'
                    fi
                    if [ -n $nm_up -o -n $at_up ]  
                    then
cat <<EOF
    <tr>
        <th>$_DSLTOOL_INFO_UPSTREAM $i</th>
        <td align='center'>$nm_up</td>
        <td align='center'>$at_up</td>
    </tr>
EOF
                    fi
                    if [ -n $nm_down -o -n $at_down ]
                    then
cat <<EOF
        <tr>
        <th>$_DSLTOOL_INFO_DOWNSTREAM $i</th>
        <td align='center'>$nm_down</td>
        <td align='center'>$at_down</td>
    </tr>
EOF
                    fi
                fi
            done
cat <<EOF
</table>
EOF
            show_tab_footer
        ;;
    esac
}

dsltool_info_errors ()
{
    show_tab_header "$_DSLTOOL_INFO_ERRORS" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th></th>
        <th>$_DSLTOOL_INFO_FEC</th>
        <th>$_DSLTOOL_INFO_CRC</th>
        <th>$_DSLTOOL_INFO_HEC</th>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_RX</th>
        <td align='center'>$error_rx_FEC</td>
        <td align='center'>$error_rx_CRC</td>
        <td align='center'>$error_rx_HEC</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_TX</th>
        <td align='center'>$error_tx_FEC</td>
        <td align='center'>$error_tx_CRC</td>
        <td align='center'>$error_tx_HEC</td>
    </tr>
</table>
EOF
    show_tab_footer
}

dsltool_info_failures ()
{
    show_tab_header "$_DSLTOOL_INFO_FAILURES" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th></th>
        <th>$_DSLTOOL_INFO_ERRSEC</th>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_15M</th>
        <td align='center'>$error_secs_15min</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_DAY</th>
        <td align='center'>$error_secs_day</td>
    </tr>
</table>
EOF
    show_tab_footer
}

dsltool_info_atm ()
{
    case "$dsltype" in
        "ADSL"|"ADSL2"|"ADSL2+")
            show_tab_header "$_DSLTOOL_INFO_ATM" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th>$_DSLTOOL_INFO_VPI</th>
        <th>$_DSLTOOL_INFO_VCI</th>
    </tr>
    <tr>
        <td align='center'>$ATM_VPI</td>
        <td align='center'>$ATM_VCI</td>
    </tr>
</table>
EOF
            show_tab_footer
        ;;
    esac
}

dsltool_info_xtu ()
{
    case "$dsltype" in
        "ADSL"|"ADSL2"|"ADSL2+")
            show_tab_header "$_DSLTOOL_INFO_ATU" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th></th>
        <th>$_DSLTOOL_INFO_VENDOR</th>
        <th>$_DSLTOOL_INFO_VENDSPEC</th>
        <th>$_DSLTOOL_INFO_REVISION</th>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_ATUC</th>
        <td align='center'>$ATU_C_vendor</td>
        <td align='center'>$ATU_C_vendspec</td>
        <td align='center'>$ATU_C_revision</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_ATUR</th>
        <td align='center'>$ATU_R_vendor</td>
        <td align='center'>$ATU_R_vendspec</td>
        <td align='center'>$ATU_R_revision</td>
    </tr>
</table>
EOF
            show_tab_footer
        ;;
        "VDSL"|"VDSL2")
            show_tab_header "$_DSLTOOL_INFO_VTU" "no"
cat <<EOF
<table class='normtable'>
    <tr>
        <th></th>
        <th>$_DSLTOOL_INFO_VENDOR</th>
        <th>$_DSLTOOL_INFO_VENDSPEC</th>
        <th>$_DSLTOOL_INFO_REVISION</th>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_VTUO</th>
        <td align='center'>$ATU_C_vendor</td>
        <td align='center'>$ATU_C_vendspec</td>
        <td align='center'>$ATU_C_revision</td>
    </tr>
    <tr>
        <th>$_DSLTOOL_INFO_VTUR</th>
        <td align='center'>$ATU_R_vendor</td>
        <td align='center'>$ATU_R_vendspec</td>
        <td align='center'>$ATU_R_revision</td>
    </tr>
</table>
EOF
            show_tab_footer
        ;;
    esac
}

dsltool_info ()
{
    rm /srv/www/img/dsltool-${dsltoolhost}.dat
    /usr/bin/dsltool-info.sh ${FORM_idx} -o9

    if [ -e /srv/www/img/dsltool-${dsltoolhost}.dat ]
    then
        . /srv/www/img/dsltool-${dsltoolhost}.dat
        dsltool_info_status
        dsltool_info_line
        dsltool_info_errors
        dsltool_info_failures
        dsltool_info_atm
        dsltool_info_xtu
    else
        show_error "$_MN_err" "$_DSLTOOL_ERR_NO_ANSWER ${dsltoolhost}"
    fi
}

dsltool_spectrum_show()
{
    if [ -f /srv/www/img/dsltool-${dsltoolhost}-$1.png ]
    then
        echo "<img src=\"/img/dsltool-${dsltoolhost}-$1.png\" alt=\"DSL-Tool $1\"/>"
    fi
}

dsltool_spectrum ()
{
    /usr/bin/dsltool-info.sh ${FORM_idx} -o9
    dsltool_spectrum_show bits
    echo "<br/>"
    dsltool_spectrum_show snr
    echo "<br/>"
    dsltool_spectrum_show char
    echo "<br/>"
}

dsltool_statistics ()
{
    if [ "$dsltoolrrd" = "yes" ]
    then
        tab_hdr=""
        tab_list=""
        if [ -f $rrd_dbpath/dsltool-${dsltoolhost}/modemstate.rrd ]
        then
            tab_hdr=`echo "$tab_hdr modemstate"`
        fi
        if [ -f $rrd_dbpath/dsltool-${dsltoolhost}/attenuation-down.rrd ]
        then
            tab_hdr=`echo "$tab_hdr attenuation"`
        fi
        if [ -f $rrd_dbpath/dsltool-${dsltoolhost}/noisemargin-down.rrd ]
        then
            tab_hdr=`echo "$tab_hdr noisemargin"`
        fi
        if [ -f $rrd_dbpath/dsltool-${dsltoolhost}/txpower-up.rrd -a \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/txpower-down.rrd ]
        then
            tab_hdr=`echo "$tab_hdr txpower"`
        fi
        if [ -f $rrd_dbpath/dsltool-${dsltoolhost}/bandwidth-kbit-up.rrd -a \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/bandwidth-kbit-down.rrd ]
        then
            tab_hdr=`echo "$tab_hdr bandwidth"`
        fi
        if [ -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-error-FEC-Rx.rrd -o \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-error-CRC-Rx.rrd -o \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-error-HEC-Rx.rrd -o \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-error-FEC-Tx.rrd -o \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-error-CRC-Tx.rrd -o \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-error-HEC-Tx.rrd ]
        then
            tab_hdr=`echo "$tab_hdr errors"`
        fi
        if [ -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-failure-errsec-15min.rrd -a \
             -f $rrd_dbpath/dsltool-${dsltoolhost}/statistics-failure-errsec-day.rrd ]
        then 
            tab_hdr=`echo "$tab_hdr failures"`
        fi
        for i in $tab_hdr
        do
            eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
            : ${rrd_source_time:=$rrd_default_graphtime}

            case $i in
                modemstate)
                    label=$(translate_label "${_DSLTOOL_MODEMSTATE}")
                    ;;
                attenuation)
                    label=$(translate_label "${_DSLTOOL_ATTENUATION}")
                    ;;
                noisemargin)
                    label=$(translate_label "${_DSLTOOL_NOISEMARGIN}")
                    ;;
                txpower)
                    label=$(translate_label "${_DSLTOOL_TXPOWER}")
                    ;;
                bandwidth)
                    label=$(translate_label "${_DSLTOOL_BANDWIDTH}")
                    ;;
                errors)
                    label=$(translate_label "${_DSLTOOL_ERRORS}")
                    ;;
                failures)
                    label=$(translate_label "${_DSLTOOL_FAILURES}")
                    ;;
            esac
            if [ "$FORM_action" = "dsltool_$i" ]
            then
                tab_list=`echo "$tab_list $label no"`
            else
                tab_list=`echo "$tab_list $label $myname?idx=${FORM_idx}&action=dsltool_$i"`
            fi
        done
        show_tab_header $tab_list
        rrd_open_tab_list $FORM_action
        rrd_render_graph $FORM_action
        rrd_close_tab_list
        show_tab_footer
    fi
}

dsltool_control ()
{
    echo "<form action=\"$myname\" method=\"get\">"
    echo "<input type=\"hidden\" name=\"action\" value=\"$FORM_action\" />"
    echo "<input type=\"submit\" value=\"$_DSLTOOL_CTRL_RESYNC\" name=\"command\" class=\"actions\" />"
    echo "<input type=\"submit\" value=\"$_DSLTOOL_CTRL_REBOOT\" name=\"command\" class=\"actions\" />"
    echo "</form>"
}

dsltool_debug ()
{
    /usr/bin/dsltool-dump.sh ${FORM_idx}
    
    http_header download "filename=ctype=application/download;filename=dsltool-dump.tgz"
    cat /tmp/dsltool-dump.tgz
}

if [ "$dsltooldebug" = "yes" -a "$FORM_action" = "dsltool_debug" ]
then
    dsltool_debug
else
    show_html_header "DSL-Tool"
    
    # dsltool instances
    if [ ${DSLTOOL_N} -gt 1 ]
    then
        tab_list=""
        for n in `seq 1 ${DSLTOOL_N}`
        do
            eval label='$DSLTOOL_'$n'_HOST'
            if [ "$FORM_idx" = "$n" ]
            then
                tab_list=`echo "$tab_list $label no"`
            else
                tab_list=`echo "$tab_list $label $myname?idx=$n"`
            fi
        done
        show_tab_header $tab_list
    fi
    
    tab_list=""
    if [ "$dsltoolrrd" = "yes" ]
    then
        tabs=`echo "info spectrum statistics control"`
    else
        tabs=`echo "info spectrum control"`
    fi
    if [ "$dsltooldebug" = "yes" ]
    then
        tabs=`echo "$tabs debug"`
    fi
    for i in $tabs
    do
        case $i in
            info)
                label=$(translate_label "${_DSLTOOL_INFO}")
                ;;
            spectrum)
                label=$(translate_label "${_DSLTOOL_SPECTRUM}")
                ;;
            statistics)
                label=$(translate_label "${_DSLTOOL_STATISTICS}")
                ;;
            control)
                label=$(translate_label "${_DSLTOOL_CONTROL}")
                ;;
            debug)
                label=$(translate_label "${_DSLTOOL_DEBUG}")
                ;;
        esac
        if [ "$FORM_action" = "dsltool_$i" ]
        then
            tab_list=`echo "$tab_list $label no"`
        elif [ "$i" = "statistics" ]
        then
            display="$myname?idx=${FORM_idx}&action=dsltool_$i"
            for a in modemstate attenuation noisemargin txpower bandwidth errors failures
            do
                if [ "$FORM_action" = "dsltool_$a" ]
                then
                    display="no"
                fi
            done
            tab_list=`echo "$tab_list $label $display"`
        else
            tab_list=`echo "$tab_list $label $myname?idx=${FORM_idx}&action=dsltool_$i"`
        fi
    done
    show_tab_header $tab_list
    
    case $FORM_action in
        dsltool_info)
            dsltool_info
            ;;
        dsltool_spectrum)
            FORM_action="dsltool_spectrum_bits"
            dsltool_spectrum
            ;;
        dsltool_spectrum_bits | dsltool_spectrum_snr | \
        dsltool_spectrum_char | dsltool_spectrum_all)
            dsltool_spectrum
            ;;
        dsltool_statistics)
            FORM_action="dsltool_modemstate"
            dsltool_statistics
            ;;
        dsltool_modemstate | dsltool_attenuation | \
        dsltool_noisemargin | dsltool_txpower | dsltool_bandwidth | \
        dsltool_errors | dsltool_failures)
            dsltool_statistics
            ;;
        dsltool_control)
            case "$FORM_command" in
                "$_DSLTOOL_CTRL_RESYNC")
                    /usr/bin/dsltool-resync.sh ${FORM_idx}
                    ;;
                "$_DSLTOOL_CTRL_REBOOT")
                    /usr/bin/dsltool-reboot.sh ${FORM_idx}
                    ;;
            esac
            dsltool_control
            ;;
    esac
    show_tab_footer

    # dsltool instances
    if [ ${DSLTOOL_N} -gt 1 ]
    then
        show_tab_footer
    fi
    
    show_html_footer
fi
