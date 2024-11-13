#!/bin/sh
#------------------------------------------------------------------------------
# ntp-status.cgi - Display Status of NTP
#
# Creation:     27.07.2009 babel
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
#set_debug=yes
. /srv/www/include/cgi-helper
. /etc/rc.cfg

if [ "$RRDTOOL_NTP" = "yes" ]
then
. /srv/www/include/rrd-common.inc
. /srv/www/include/rrd-status_ntp.inc
fi

# Security
check_rights "NTP" "view"

myname=`basename $0`

ntp_assoc_select_val_0="reject"
ntp_assoc_select_val_0_colour="red"
ntp_assoc_select_val_1="falsetick"
ntp_assoc_select_val_1_colour="red"
ntp_assoc_select_val_2="excess"
ntp_assoc_select_val_2_colour="red"
ntp_assoc_select_val_3="outlyer"
ntp_assoc_select_val_3_colour="red"
ntp_assoc_select_val_4="candiate"
ntp_assoc_select_val_4_colour="yellowgreen"
ntp_assoc_select_val_5="backup"
ntp_assoc_select_val_5_colour="yellow"
ntp_assoc_select_val_6="sys.peer"
ntp_assoc_select_val_6_colour="limegreen"
ntp_assoc_select_val_7="pps.peer"
ntp_assoc_select_val_7_colour="limegreen"

# Maps an association to a colour.
#
# Input:
#   $1 = association status (hexadecimal number without 0x prefix)
# Output:
#   HTML colour to use
map_assoc_select_to_colour()
{
    local value=$(printf "%d" 0x$1)
    local index=$(((value >> 8) & 7))
    eval echo \$ntp_assoc_select_val_${index}_colour
}

# Converts a generic property list (name: value) into a HTML table.
#
# Input:
#   <stdin> = input lines
# Output:
#   HTML table code
genericproplist()
{
    sed -n 's,^\([^:]\+:\)[[:space:]]*\(.*\)$,\1\2,p' |
    ( IFS=:; while read name value
    do
        echo "<tr><td>$name</td><td>$value</td></tr>"
    done )
}

# Prints a table with properties for a list of associations.
#
# Input:
#   $1 = command
#   $2 = association ID
#   $3... = fields
# Output:
#   HTML table code
readlist_single_assoc ()
{
    local cmd=$1 assid=$2
    shift 2

    local cmdline= field
    for field
    do
        cmdline="$cmdline -c \"$cmd $assid $field\""
    done

    echo "<table class='normtable'>"
    echo "<tr><th>$_NTP_VARIABLE</th><th>$_NTP_VALUE</th></tr>"
    eval ntpq 2>/dev/null "$cmdline" \| \
        sed -n "'"'/^$/d;s,^\([^=]\+\)=\"\?\([^"]*\)\"\?$,\1 \2,p'"'" \| \
        while read name value\; \
        do \
            echo "\"<tr><td>\$name</td><td>\$value</td></tr>\""\; \
        done
    echo "</table>"
}

# Prints a table with properties for a list of associations.
#
# Input:
#   $1 = command
#   $2 = association IDs
#   $3 = association colours
#   $4... = fields
# Output:
#   HTML table code
readlist_multi_assoc ()
{
    local cmd=$1 assoc_ids=$2 assoc_colours=$3 field assid
    shift 3

    echo "<table class='normtable'>"

    echo -n "<tr><th/>"
    for assid in $assoc_ids
    do
        echo "<th>$assid</th>"
    done
    echo "</tr>"

    for field
    do
        local break= cmdline=
        if [ "${field%:}" != "$field" ]
        then
            break=1
            field=${field%:}
        fi

        for assid in $assoc_ids
        do
            cmdline="$cmdline -c \"$cmd $assid ${field%:}\""
        done

        echo -n "<tr><td>$field</td>"
        set -- $assoc_colours
        eval ntpq 2>/dev/null "$cmdline" \| \
            sed "'"'/^$/d;s,^[^=]\+=\"\?\([^"]*\)\"\?$,\1,'"'" \| \
            while read value\; \
            do \
                [ -n "\"\$break\"" ] \&\& value=\$\(echo "\"\$value\"" \| sed "'s, ,<br/>,'"\)\; \
                echo -n "\"<td style='background-color:\$1'>\$value</td>\""\; \
                shift\; \
            done
        echo "</tr>"
    done
    echo "</table>"
}

overview ()
{
    echo '<div class="ntp_msg">'
    if [ -e /var/run/ntp_firstsync ]
    then
        . /var/run/ntp_firstsync
        if [ "$ntp_firstsync" = "yes" ]
        then
            echo "<p class=\"error\">$_NTP_NTPBOX_NOT_READY</p>"
        else
            if [ "$NTP_REFCLOCK_TYPE" != "none" ]
            then
                [ -e /var/run/ntp_status_radioclock ] && . /var/run/ntp_status_radioclock
                if [ "$ntp_status_radioclock" = "on" ]
                then
                    echo "<p class=\"info\">$_NTP_RADIO_CLOCK_RECEIVING</p>"
                else
                    echo "<p class=\"error\">$_NTP_RADIO_CLOCK_NOT_RECEIVING</p>"
                fi
            fi

            [ -e /var/run/ntp_status_stratum ] && . /var/run/ntp_status_stratum
            if [ "$NTP_REFCLOCK_TYPE" != "none" ]
            then
                if [ 0"$ntp_status_stratum" -eq 1 ]
                then
                    echo "<p class=\"info\">$_NTP_SERVER_IS_STRATUM_1</p>"
                elif [ 0"$ntp_status_stratum" -gt 1 -a 0"$ntp_status_stratum" -lt 16 ]
                then
                    echo "<p class=\"error\">$_NTP_SERVER_LOST_HW $ntp_status_stratum</p>"
                else
                    echo "<p class=\"error\">$_NTP_SERVER_NOT_WORKING</p>"
                fi
            else
                if [ 0"$ntp_status_stratum" -gt 1 -a 0"$ntp_status_stratum" -lt 16 ]
                then
                    echo "<p class=\"info\">$_NTP_SERVER_IN_SYNC_WITH_OTHERS $ntp_status_stratum</p>"
                else
                    echo "<p class=\"error\">$_NTP_SERVER_NOT_WORKING</p>"
                fi
            fi
        fi
    else
        echo "<p class=\"error\">$_NTP_STATUS_ERROR</p>"
    fi
    echo '</div class="ntp_msg">'

    if [ "$RRDTOOL_NTP" = "yes" ]
    then
        rrd_open_tab_list ntp_main
        rrd_render_graph ntp_main
        rrd_close_tab_list
        echo "<br>"
        rrd_open_tab_list ntp_frequency
        rrd_render_graph ntp_frequency
        rrd_close_tab_list
    else
        show_error "" "$_NTP_NO_RRDTOOL"
    fi
}

readlist ()
{
    readlist_single_assoc readvar 0 \
        version processor system leap stratum precision rootdelay rootdisp \
        refid reftime clock peer tc mintc offset frequency sys_jitter \
        clk_jitter clk_wander
}

peer_table()
{
    echo "<table class='normtable'>"
    local field \
        assoc_fields="index assid status conf reach auth condition last_event cnt" \
        peer_fields="select remote refid stratum type when poll reach delay offset jitter"

    echo -n "<tr>"
    for field in $assoc_fields $peer_fields
    do
        echo -n "<th>$field</th>"
    done
    echo "</tr>"

    local tmpfile=$(mktemp)
    ntpq -w -c peers -c lassociations 2>/dev/null | sed '1,/^=/d' > $tmpfile
    sed '1,/^=/d' $tmpfile > $tmpfile.assoc

    local cont= peer_num=0 i
    oldifs="$IFS"; IFS=
    while read -r line
    do
        if [ -z "$cont" ]
        then
            select=${line:0:1}
            line=${line:1}

            IFS="$oldifs"
            set -- $line
            remote="$1"
            if [ $# -le 2 ]
            then
                [ -n "$2" ] && remote="$remote $2"
                cont=1
                IFS=
                continue
            else
                shift
            fi
        else
            IFS="$oldifs"
            set -- $line
        fi
        peer_num=$((peer_num+1))
        eval select_${peer_num}=\$select
        eval remote_${peer_num}=\$remote
        eval refid_${peer_num}=\$1
        eval stratum_${peer_num}=\$2
        eval type_${peer_num}=\$3
        eval when_${peer_num}=\$4
        eval poll_${peer_num}=\$5
        eval reach_${peer_num}=\$6
        eval delay_${peer_num}=\$7
        eval offset_${peer_num}=\$8
        eval jitter_${peer_num}=\$9

        cont=
        IFS=
    done <<EOF
$(sed '/^$/,$d' $tmpfile)
EOF
    IFS="$oldifs"

    assoc_ids=
    assoc_colours=
    while read $assoc_fields
    do
        local colour=$(map_assoc_select_to_colour $status)
        local td="td style='background-color:$colour'"
        echo -n "<tr>"
        for field in $assoc_fields
        do
            eval local value=\$$field
            if [ "$field" = assid ]
            then
                assoc_colours="$assoc_colours $(map_assoc_select_to_colour $status)"
                assoc_ids="$assoc_ids $assid"
            fi
            echo -n "<$td>$value</td>"
        done

        local srcadr=$(ntpq -c "rv $assid srcadr" | sed 's/^srcadr=//') found=
        for i in $(seq 1 $peer_num)
        do
            eval local remote=\$remote_${i}
            if [ "$srcadr" = "$remote" ]
            then
                found=1
                break
            fi
        done

        if [ -n "$found" ]
        then
            for field in $peer_fields
            do
                eval local value=\$${field}_${i}
                if [ "$field" = select ]
                then
                    echo -n "<$td align='center'>$value</td>"
                else
                    echo -n "<$td>$value</td>"
                fi
            done
        else
            for field in $peer_fields
            do
                if [ "$field" = select ]
                then
                    echo -n "<$td align='center'>?</td>"
                else
                    echo -n "<$td>?</td>"
                fi
            done
        fi
        echo "</tr>"
    done < $tmpfile.assoc
    rm $tmpfile.assoc $tmpfile

    echo "</table>"
}

peerinfo()
{
    local assoc_ids assoc_colours
    # also looks up association IDs and colours
    peer_table
    echo "<p/>"

    readlist_multi_assoc readvar "$assoc_ids" "$assoc_colours" \
        srcadr srcport dstadr dstport leap stratum precision rootdelay \
        rootdisp refid reftime: rec: reach unreach hmode pmode hpoll ppoll \
        headway flash keyid offset delay dispersion jitter xleave
}

clockinfo()
{
    local dummy assid status assoc_ids assoc_colours clocks clock assid_and_clock
    while read dummy assid status
    do
        local data=$(ntpq -c "clocklist $assid" 2>/dev/null)
        if [ -n "$data" ]
        then
            assoc_colours="$assoc_colours $(map_assoc_select_to_colour $status)"
            assoc_ids="$assoc_ids $assid"
            clock=$(ntpq -c "rv $assid srcadr" | sed -n 's/srcadr=\(.*\)(.*)/\1/p')
            clocks="$clocks $assid:$clock"
        fi
    done <<EOF
$(ntpq -c associations 2>/dev/null | sed '1,/^=/d')
EOF

    readlist_multi_assoc clockvar "$assoc_ids" "$assoc_colours" \
        device timecode poll noreply badformat baddata stratum refid flags

    if [ "$RRDTOOL_NTP" = "yes" -a -n "$clocks" ]
    then
        echo "<p/>"
        local tab_list
        for assid_and_clock in $clocks
        do
            : ${FORM_section:=$assid_and_clock}
            assid=${assid_and_clock%%:*}
            clock=${assid_and_clock#*:}
            eval local rrd_source_time='$FORM_rrd_graphtime_'$rrd_source
            : ${rrd_source_time:=$rrd_default_graphtime}
            eval : \${FORM_rrd_graphtime_$clock:=$rrd_default_graphtime}
            if [ "$FORM_section" = "$assid_and_clock" ]
            then
                tab_list="$tab_list $assid&nbsp;($clock) no"
            else
                tab_list="$tab_list $assid&nbsp;($clock) $myname?action=clockinfo&section=$assid_and_clock"
            fi
        done

        show_tab_header $tab_list
        clock=${FORM_section#*:}
        rrd_open_tab_list $clock
        rrd_render_graph ntp_generic $clock
        rrd_close_tab_list
    else
        show_error "" "$_NTP_NO_CLOCK_GRAPHS"
    fi
}

kerninfo ()
{
    echo "<table class='normtable'>"
    echo "<tr><th>$_NTP_VARIABLE</th><th>$_NTP_VALUE</th></tr>"
    ntpq -c kerninfo 2>/dev/null | genericproplist
    echo "</table>"
}

sysinfo ()
{
    echo "<table class='normtable'>"
    echo "<tr><th>$_NTP_VARIABLE</th><th>$_NTP_VALUE</th></tr>"
    ntpq -c sysinfo 2>/dev/null | genericproplist
    echo "</table>"
}


: ${FORM_action:=overview}
: ${FORM_rrd_graphtime_ntp_main:=$rrd_default_graphtime}
: ${FORM_rrd_graphtime_ntp_frequency:=$rrd_default_graphtime}
_overview=$_MP_ov
_peerinfo=$_NTP_RRD_PEER
_clockinfo=$_NTP_RRD_CLOCKINFO
_radioclock=$_NTP_RRD_RADIOCLOCK
_readlist=$_NTP_RRD_READLIST
_kerninfo=$_NTP_RRD_KERNINFO
_sysinfo=$_NTP_RRD_SYSINFO

show_html_header "NTP-`eval echo '$_'$FORM_action`" "cssfile=status_ntp"

show_tab_header "$_overview" "$myname" \
    "$_peerinfo" "$myname?action=peerinfo" \
    "$_clockinfo" "$myname?action=clockinfo" \
    "$_readlist" "$myname?action=readlist" \
    "$_kerninfo" "$myname?action=kerninfo" \
    "$_sysinfo" "$myname?action=sysinfo"

$FORM_action

show_tab_footer

show_html_footer
