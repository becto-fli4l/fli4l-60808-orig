#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/admin/OpenVPN.cgi
# Creation:     26.03.2005 HH
# Last Update:  $Id$
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

# Get libs
gui_started="true"
. /srv/www/include/status_OpenVPN_functions.inc
. /srv/www/include/status_OpenVPN_output.inc

# Define important variables
myname=`basename $0`
name="$FORM_name"

: ${FORM_sec:=status}                  # standard section is show status
case $FORM_sec in
    status)
        : ${FORM_action:=show}         # standard action in sec status is show
    ;;
    detail)
        : ${FORM_action:=stats}        # standard action in sec detail is statistics
    ;;
esac
: ${FORM_showtext:=yes}                # normally show connection status as text
: ${FORM_lang:=$HTTPD_GUI_LANG}      # standard language
: ${FORM_nrlog:=20}                    # default number of rows for log is 20

# Security
if [ "$FORM_action" = "show" ]
then
    sec_action="view"
else
    sec_action="$FORM_action"
fi
check_rights "openvpn" "$sec_action"

# Get language

check_variables

#------- Main part --------------------------------------------------------------
case $FORM_sec in
    status)
        # start,stop,hold,release,reload ovpn session
        eval $FORM_action'_ovpn'
    ;;

    detail)
        case $FORM_action in
            secret)
                # get keyfile
                keyfile=`cat /etc/openvpn/$name.conf | grep ^secret | cut -d" " -f2 | sed 's%/etc/openvpn/%%'`
                case $FORM_saction in
                    show)
                    ;;
                    download)
                        download_secret
                        exit 0
                    ;;
                    *)
                        eval $FORM_saction'_secret'
                    ;;
                esac
            ;;
        esac
        # initialize menu
        mod_prefx="VPN_" # this is to tell show_tab_header about a prefix in language variables
        eval header="\"\$_VPN_head_$FORM_action $name\""
        link="$myname?lang=$FORM_lang&amp;sec=$FORM_sec&amp;showtext=$FORM_showtext&amp;"
        statistics="$_VPN_stats ${link}action=stats&amp;name=$name"
        config="$_VPN_config ${link}action=config&amp;name=$name"
        # show bridge config instead of packet filter if bridged connection is used
        if cat /etc/openvpn/$name.conf | grep -q "^dev tun"
        then
            packetfilter="$_VPN_packet ${link}action=packet&amp;name=$name"
        else
            packetfilter="$_VPN_bridge ${link}action=bridge&amp;name=$name"
        fi
        case `state_ovpn` in
            STOPPED)
                debug="d"
            ;;
            *)
                if [ -f /var/lock/ovpn-debug.lck ]
                then
                    rm -f /var/lock/ovpn-debug.lck
                    debug="d"
                fi
            ;;
        esac
	eval log="\"\$_VPN_${debug}log ${link}action=${debug}log&amp;name=$name\""
        eval support="\"\$_VPN_${debug}supp ${link}action=${debug}supp&amp;name=$name\""
        secret="$_VPN_secret ${link}action=secret&amp;saction=show&amp;name=$name"

        show_header "$header" $statistics $log $packetfilter $config $secret $support
        # Show config,[stats],[packet],[bridge],[d]supp,[d]log of current connection
        eval 'show_'$FORM_action
        show_footer
    ;;
esac
