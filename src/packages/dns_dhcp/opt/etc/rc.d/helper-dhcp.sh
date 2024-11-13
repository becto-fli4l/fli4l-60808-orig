#!/bin/sh
#------------------------------------------------------------------------------
# /etc/rc.d/helper-dhcp - common functions to generate dhcp-config
#
# Creation:     2008-10-10 lanspezi
# Last Update:  $Id$
#------------------------------------------------------------------------------

# lease-file - check for writeable directory
lease_file ()
{
    case "x$DHCP_LEASES_DIR" in
        x|xauto) DHCP_LEASES_DIR=/var/lib/persistent/dhcp ;;
    esac

    if [ -d "$DHCP_LEASES_DIR" ]
    then
        sh -c "> ${DHCP_LEASES_DIR}/dhcp_testfile" 2> /dev/null
        if [ ! -f "${DHCP_LEASES_DIR}/dhcp_testfile" ]
        then
            log_error "cannot write to DHCP_LEASES_DIR"
            return 1
        fi
        rm "${DHCP_LEASES_DIR}/dhcp_testfile"
    else
        if ! mkdir -p "$DHCP_LEASES_DIR"; then
            log_error "could not create directory for the leases-file"
            return 1
        fi
    fi
    return 0
}

check_lease_file ()
{
    if [ ! -f "${DHCP_LEASES_DIR}/$1.leases" ]
    then
        if [ "$DHCP_LEASES_VOLATILE" != yes ]; then
            log_warn "dhcp-leases was not found!"
            log_warn "If this is your first boot with OPT_DHCP enabled you can"
            log_warn "ignore this message. Otherwise your dhcp-leases-file is not"
            log_warn "saved permanently. In this case you should read the documentation again!"
        fi
        > ${DHCP_LEASES_DIR}/$1.leases
    fi
}

# get parameters for an DNS-, NTP- or WIN-server
dhcpd_server_option ()
{
    var=$1
    service=$2
    idx=$3
    
    eval _server="\$${var}_${idx}_${service}_SERVERS"
    if [ -z "$_server" ]
    then
        # no DHCP_RANGE_idx_service_SERVERS
        eval _server="\$DHCP_${service}_SERVERS"
        if [ -z "$_server" ]
        then
            # no DHCP_service_SERVERS
            case ${service} in
            DNS) 
                if [ "$OPT_DNS" != "yes" ]
                then
                    #  no DNS server available
                    _server="none"
                fi
                ;;
            WINS)
                if [ "$OPT_SAMBA" = "yes" -a "$OPT_NMBD" = "yes" ]
                then
                    if [ "$NMBD_WINSSERVER" = "yes" -o -n "$NMBD_EXTWINSIP" ]
                    then
                        if [ -n "$NMBD_EXTWINSIP" ]
                        then
                            _server=$NMBD_EXTWINSIP
                        fi
                    else
                        #  no WINS server available
                        _server="none"
                    fi
                else
                    #  no WINS server available
                    _server="none"
                fi
                ;;
            NTP)
                if [ "$OPT_CHRONY" != "yes" -a "$OPT_NTP" != "yes" ]
                then
                    #  no NTP server available
                    _server="none"
                fi
                ;;
            esac
        fi
    fi
    
    if [ -z "$_server" ]
    then
        eval "_${service}_server"=$ip_local
    else
        eval "_${service}_server"=`echo $_server | tr ' ' ','`
    fi
}
