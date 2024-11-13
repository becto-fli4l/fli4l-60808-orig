#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/include/OpenVPN_functions.inc
# Creation:     15.05.2005 HH
# Last Update:  $Id$
#------------------------------------------------------------------------------
[ "$gui_started" != "true" ] && exit 1 # must not be called standalone
#------- Helper Functions -------------------------------------------------------
icon ()
{ # Display icon with description
    id=
    [ -z "$2" ] && eval alt='$_VPN_'$1 || eval alt='$_VPN_'$2
    [ -z "$3" ] || id="id=\"$3\""     # use id for reload
    echo "<img $id src=\"../img/$1.gif\" width=\"24\" height=\"24\" alt=\"$alt\" title=\"$alt\">"
}

ocontrol ()
{ # Control ovpn-management
    if [ -f /var/run/openvpn/$name/mport ]
    then
        mport=`cat /var/run/openvpn/$name/mport`
        ovpnctrl "$1" "^(END$|SUCCESS:|ERROR:)" localhost $mport 2>/dev/null >/tmp/ovpnstate.$$
        case "$?" in
            0)
                case "$1" in
                    state|STATE)
                        if cat /tmp/ovpnstate.$$ | grep -q HOLD
                        then
                            echo "HOLD"
                        else
                            cat /tmp/ovpnstate.$$ | grep , | cut -d',' -f2
                        fi
                    ;;
                    *)
                        cat /tmp/ovpnstate.$$ | grep -v -E "^(END$|SUCCESS:|ERROR:)"
                    ;;
                esac
            ;;
            *)
                echo "STOPPED"
            ;;
        esac
        rm -f /tmp/ovpnstate.$$
    else
        echo "STOPPED"
    fi
}

ctime ()
{ # convert unix-time to human readable
    # convert format string
    format=`echo $2 | sed 's°dd°\\\3°;s°mm°\\\2°;s°jjjj°\\\1°;s°hh°\\\4°;s°mm°\\\5°;s°ss°\\\6°'`
    # use format string to convert output of hconv
    hconv time_t $1 | sed "s°^\([^-]*\)-\([^-]*\)-\([^T]*\)T\([^:]*\):\([^:]*\):\([^$]*\)°$format°"
}

cnum ()
{ # convert number in human readable format
    hconv hnum $1
}

check_variables ()
{ # check given variables for security reason
    case $FORM_sec in
        status)
            case $FORM_action in
                show|start|stop|reload|hold|release)
                    #everything is fine
                ;;
                *)
                    FORM_sec="exit"
                 ;;
            esac
        ;;
        detail)
            case $FORM_action in
                secret)
                    case $FORM_saction in
                        create|show|download|save|restore)
                        #everythings fine
                        ;;
                        *)
                            FORM_sec="exit"
                         ;;
                    esac
                ;;
                config|dlog|dsupp|stats|packet|bridge|log|supp)
                ;;
                *)
                    FORM_sec="exit"
                 ;;
            esac
        ;;
        *)
            FORM_sec="exit"
        ;;
    esac
    case $FORM_sec in
        exit)
            show_header "fatal error - wrong parameters"
            show_footer
            exit 1
        ;;
    esac

}

#------- Action Functions -------------------------------------------------------

start_ovpn ()
{ # start ovpn session
    if ! ps | grep -qe "[n]obody.*$name.conf" # do not start openvpn twice
    then
        tmp_dir=`pwd`
        cd /etc/openvpn/
        case $1 in
            debug) openvpn --config $name.conf --verb 4 & ;;
            *)
                ulimit -l unlimited
                ulimit -c unlimited
                openvpn --config $name.conf --daemon openvpn-$name > /dev/null
                reload
            ;;
        esac
        cd $tmp_dir
    fi
}

stop_ovpn ()
{ # stop ovpn session
    case `state_ovpn` in              # only kill if ovpn-binary is running
        STOPPED) ;;
        *)
            ocontrol "signal SIGTERM"
            # check if ovpn is killed
            while ps | grep -qe "[n]obody.*$name.conf"
            do
                sleep 1
            done
            rm -f /var/run/openvpn/$name/mport
            rm -f /var/run/openvpn/$name/pid
        ;;
    esac
    case $1 in debug);; *) reload ;; esac
}

reload_ovpn ()
{ # reload ovpn session
    ocontrol "signal SIGUSR1"
    reload
}

hold_ovpn ()
{ # reload ovpn session
    ocontrol "hold on"
    ocontrol "signal SIGUSR1"
    reload
}

release_ovpn ()
{ # reload ovpn session
    ocontrol "hold off"
    ocontrol "hold release"
    reload
}

state_ovpn ()
{ # state of ovpn session
    ocontrol state
}

show_ovpn ()
{ # state of ovpn session
    show_header "$_VPN_head_state" "$_VPN_connections" no
    show_connections
    show_footer
}

download_secret ()
{   # download secret to client
    echo "Content-Type: text/plain"
    echo "Content-Disposition: attachment; filename=$keyfile"
    echo
    if [ -f /tmp/$keyfile ]
    then
        keyfile=/tmp/$keyfile
    else
        keyfile=/etc/openvpn/$keyfile
    fi
    sed "s/$/`echo -e \\\r`/" $keyfile # convert secret file to dos
}

save_secret ()
{ # save secret
    # save secret if not yet done so
    [ -f /etc/openvpn/$keyfile.orig ] || cp -f /etc/openvpn/$keyfile /etc/openvpn/$keyfile.orig
    # move teporary secret if it exists
    [ -f /tmp/$keyfile ] && mv -f /tmp/$keyfile /etc/openvpn/$keyfile
}

restore_secret ()
{ # save secret
    # restore secret if file exists
    [ -f /etc/openvpn/$keyfile.orig ] && mv -f /etc/openvpn/$keyfile.orig /etc/openvpn/$keyfile
    [ -f /tmp/$keyfile ] && rm -f /tmp/$keyfile
}

create_secret ()
{ # create secret
    openvpn --genkey --secret /tmp/$keyfile
}
