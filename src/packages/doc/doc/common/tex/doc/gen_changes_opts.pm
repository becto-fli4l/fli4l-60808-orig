@dir = (
        "advanced_networking",
        "base",
        "cert", "chrony",
        "dhcp_client", "dns_dhcp", "dsl", "dyndns",
        "easycron",
        "hd",
        "httpd",
        "ipv6", "isdn",
        "lcd", "lpdsrv",
        "openvpn",
        "pcmcia", "ppp", "proxy",
        "qos",
        "sshd",
        "tools",
        "umts", "usb",
        "wlan"
       );

@ignored = ("OPT_ADDMODULES", "ADDMODULES", 
	    "OPT_HDINSTALL_TEST", "HDIT_SIZES", "HDIT_DATA", "HDIT_POWEROFF",
	    "OPT_GUI_CONFIG", "GUI_CONFIG_SAVEPATH",
	    "DHCP_AUTHORITATIVE",
	    "PPPD_VERSION", "BUG_CHECK");

@ignored{@ignored} = (1) x @ignored; #create a hash from values in array

# list of renamed variables:
#       old varname                    new varname
%alias = qw(
        ETH_DRV_N                      NET_DRV_N
        ETH_DRV_x                      NET_DRV_x
        ETH_DRV_x_OPTION               NET_DRV_x_OPTION
        IP_ETH_N                       IP_NET_N
        IP_ETH_x_NAME                  IP_NET_x_DEV
        IP_ETH_x_IPADDR                IP_NET_x
        IP_ETH_x_NETWORK               IP_NET_x
        IP_ETH_x_NETMASK               IP_NET_x
        IP_DEFAULT_GATEWAY             IP_ROUTE_N:IP_ROUTE_x
        MASQ_FTP_PORT_N                MASQ_MODULE_x_OPTION
        MASQ_FTP_PORT_x                MASQ_MODULE_x_OPTION
        OPT_PORTFW                     PORTFW_N
        PORTFW_x                       PORTFW_x_TARGET:PORTFW_x_NEW_TARGET:PORTFW_x_PROTOCOL
        FORWARD_DENY_HOST_N            FORWARD_HOST_N:PF_NEW_CONFIG
        FORWARD_DENY_HOST_x            FORWARD_HOST_x:PF_NEW_CONFIG
        FORWARD_TRUSTED_NETS           TRUSTED_NETS:PF_NEW_CONFIG
        FIREWALL_DENY_PORT_N           INPUT_ACCEPT_PORT_N:PF_NEW_CONFIG
        FIREWALL_DENY_PORT_x           INPUT_ACCEPT_PORT_x:PF_NEW_CONFIG
        FIREWALL_DENY_ICMP             DENY_ICMP:PF_NEW_CONFIG
        FIREWALL_LOG                   PACKETFILTER_LOG:PF_NEW_CONFIG
        START_DNS                      OPT_DNS
        HOSTS_N                        HOST_N
        HOST_x                         HOST_x_NAME:HOST_x_IP4:HOST_x_IP6:HOST_x_MAC:HOST_x_ALIAS_N:HOST_x_ALIAS_x:HOST_x_DHCPTYP
        DNS_N                          DNS_SPECIAL_N
        DNS_x                          DNS_SPECIAL_x
        OPT_HDTUNE                     OPT_HDSLEEP
        HDTUNE_SLEEP_x                 HDSLEEP_TIMEOUT
        HDTUNE_SLEEPTIME_x             HDSLEEP_TIMEOUT
        OPT_RESCUE                     OPT_RECOVER
        HDINSTALL_RECOVERYTIME         OPT_RECOVER
        OPT_CDROM                      OPT_HDDRV
        HTTPD_USER_x                   HTTPD_USER_x_USERNAME
        HTTPD_PASS_x                   HTTPD_USER_x_PASSWORD
        HTTPD_RIGHTS_x                 HTTPD_USER_x_RIGHTS
        ISDN_CIRCUITS_N                ISDN_CIRC_N
        ISDN_CIRC_x_NETMASK            ISDN_CIRC_x_REMOTE
        ISDN_CIRC_x_NETMASKBITS        ISDN_CIRC_x_REMOTE
        ISDN_CIRC_x_ROUTE              ISDN_CIRC_x_ROUTE_N:ISDN_CIRC_x_ROUTE_x
        ISDN_CIRC_x_COMPRESSION        ISDN_CIRC_x_HEADERCOMP
        OPT_CRON                       OPT_EASYCRON
        INPUT_ACCEPT_DEF               PF_INPUT_ACCEPT_DEF
        INPUT_LIST_N                   PF_INPUT_N
        INPUT_LIST_x                   PF_INPUT_x
        INPUT_LOG                      PF_INPUT_LOG
        INPUT_LOG_LIMIT                PF_INPUT_LOG_LIMIT
        INPUT_REJ_LIMIT                PF_INPUT_REJ_LIMIT
        INPUT_UDP_REJ_LIMIT            PF_INPUT_UDP_REJ_LIMIT
        NEW_FW_CONFIG                  PF_NEW_CONFIG
        PF_ORIG_CONFIG                 PF_NEW_CONFIG
        ORIG_FW_CONFIG                 PF_NEW_CONFIG
        PF_ORIG_FW_CONFIG              PF_NEW_CONFIG
        POSTROUTING_LIST_N             PF_POSTROUTING_N
        POSTROUTING_LIST_x             PF_POSTROUTING_x
        PREROUTING_LIST_N              PF_PREROUTING_N
        PREROUTING_LIST_x              PF_PREROUTING_x
        USER_LIST_N                    PF_USR_CHAIN_N
        USER_LIST_x                    PF_USR_CHAIN_x_NAME:PF_USR_CHAIN_x_RULE_N:PF_USR_CHAIN_x_RULE_x:PF_USR_CHAIN_x_RULE_x_COMMENT
        FORWARD_ACCEPT_DEF             PF_FORWARD_ACCEPT_DEF
        FORWARD_LIST_N                 PF_FORWARD_N
        FORWARD_LIST_x                 PF_FORWARD_x
        FORWARD_LOG                    PF_FORWARD_LOG
        FORWARD_LOG_LIMIT              PF_FORWARD_LOG_LIMIT
        FORWARD_POLICY                 PF_FORWARD_POLICY
        FORWARD_REJ_LIMIT              PF_FORWARD_REJ_LIMIT
        FORWARD_UDP_REJ_LIMIT          PF_FORWARD_UDP_REJ_LIMIT
        OPENVPN_DEFAULT_FORWARD_LOG    OPENVPN_DEFAULT_PF_FORWARD_LOG
        OPENVPN_DEFAULT_FORWARD_POLICY OPENVPN_DEFAULT_PF_FORWARD_POLICY
        OPENVPN_DEFAULT_INPUT_LOG      OPENVPN_DEFAULT_PF_INPUT_LOG
        OPENVPN_DEFAULT_INPUT_POLICY   OPENVPN_DEFAULT_PF_INPUT_POLICY
        OPENVPN_DEFAULT_MANAGMENT_LOG_CACHE   OPENVPN_DEFAULT_MANAGEMENT_LOG_CACHE
        OPENVPN_x_MANAGMENT_LOG_CACHE  OPENVPN_x_MANAGEMENT_LOG_CACHE
        OPENVPN_x_MANAGMENT_PORT       OPENVPN_x_MANAGEMENT_PORT
        OPENVPN_x_FORWARD_LIST_N       OPENVPN_x_PF_FORWARD_N
        OPENVPN_x_FORWARD_LIST_x       OPENVPN_x_PF_FORWARD_x
        OPENVPN_x_FORWARD_LOG          OPENVPN_x_PF_FORWARD_LOG
        OPENVPN_x_FORWARD_POLICY       OPENVPN_x_PF_FORWARD_POLICY
        OPENVPN_x_INPUT_LIST_N         OPENVPN_x_PF_INPUT_N
        OPENVPN_x_INPUT_LIST_x         OPENVPN_x_PF_INPUT_x
        OPENVPN_x_INPUT_LOG            OPENVPN_x_PF_INPUT_LOG
        OPENVPN_x_INPUT_POLICY         OPENVPN_x_PF_INPUT_POLICY
        OPENVPN_x_POSTROUTING_LIST_N   OPENVPN_x_PF_POSTROUTING_N
        OPENVPN_x_POSTROUTING_LIST_x   OPENVPN_x_PF_POSTROUTING_x
        OPENVPN_x_PREROUTING_LIST_N    OPENVPN_x_PF_PREROUTING_N
        OPENVPN_x_PREROUTING_LIST_x    OPENVPN_x_PF_PREROUTING_x
        OPENVPN_x_REMOTE_HOSTS_N       OPENVPN_x_REMOTE_HOST_N
        OPENVPN_x_DNSIP_x              OPENVPN_x_ROUTE_x_DNSIP
        OPENVPN_x_DOMAIN_x             OPENVPN_x_ROUTE_x_DOMAIN
        EAGLE_WAITSECONDS              USB_MODEM_WAITSECONDS
        USBMODEM_WAITECONDS            USB_MODEM_WAITSECONDS
        LCD_OPTION                     LCD_ADDR_TYPE
        LCD_TYPE                       LCD_ADDR_TYPE
        HOST_x_IP                      HOST_x_IP4
        HOSTS_EXTRA_N                  HOST_EXTRA_N
        HOST_EXTRA_x_IP                HOST_EXTRA_x_IP4
        EXTMOUNT_x_DEVICE              EXTMOUNT_x_VOLUMEID
        SER_BEEP                       BEEP
        POWEROFF_ON_HALT               POWERMANAGEMENT
        REAL_MODE_POWEROFF             POWERMANAGEMENT
        BONDING_DEV_x_ARP_IP_TARGETS_N BONDING_DEV_x_ARP_IP_TARGET_N
        SSHD_PUBLIC_KEYS_N             SSHD_PUBLIC_KEY_N
        SSHD_PUBLIC_KEYFILES_N         SSHD_PUBLIC_KEYFILE_N
        SSHD_PRIVATE_KEYFILES_N        SSH_CLIENT_PRIVATE_KEYFILE_N
        SSHD_PRIVATE_KEYFILE_x         SSH_CLIENT_PRIVATE_KEYFILE_x
        COMP_TYPE_KERNEL               COMPRESS_KERNEL
        COMP_TYPE_ROOTFS               COMPRESS_ROOTFS
       );

# foreach $key (keys(%alias)) {
#     print "$key => $alias{$key}\n";
# }

# exit;
