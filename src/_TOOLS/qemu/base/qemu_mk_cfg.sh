#! /bin/sh

usage ()
{
    if [ "$1" ]; then
        echo "$1"
    else
        cat <<EOF
usage: $0 [ -r | --refresh | -h | --help | -t <template> ] <dir> [ <packet list> ]

The script creates a directory containing base, dns_dhcp, hd, sshd,
dhcp_client, tools and packages specified in <packet list>.

It then adds an initial _fli4l.txt which may be used to boot fli4l
using qemu.

-r | --refresh: refresh links in directory and add missing ones
-h | --help:    print this help text
-t <template> : create config using template
EOF
    fi
    exit 1
}

tmpl_pkgs=
while true; do
    case "$1" in
        -h | --help) usage ;;
        -r | --refresh) refresh=yes ;;
        -t)
            tmpl="$2"
            [ -f "$tmpl" ] || usage
            case "$tmpl" in
                /*) ;;
                *)  tmpl="`pwd`/$2" ;;
            esac
                    
            tmpl_pkgs=`sed -ne '/^#[[:space:]]*Uses:/{s/.*:[[:space:]]*//;s/[[:space:]]\+$//;p}' $tmpl`
            ifaces=`sed -ne '/^#[[:space:]]*Interfaces:/{s/.*:[[:space:]]*//;s/[[:space:]]\+$//;p}' $tmpl`

            case "$tmpl_pkgs" in
                '')
                    echo "Missing '# Uses:' line ..."
                    exit 1
                    ;;
                none)
                    echo "Template '$2' uses no additional packages"
                    tmpl_pkgs= 
                    ;;
                *)
                    echo "Template '$2' adds '$tmpl_pkgs' to list of default packages"
                    ;;
            esac
            grep "^# Comment:"  $tmpl
            shift
            ;;
        *) break ;;
    esac
    shift
done

[ $# -ge 1 ]  || usage
dir=$1
shift

[ -d config ] || usage "missing config dir"
if [ -e $dir ]; then
    [ "$refresh" = yes ] || usage "'$dir' already exists"
else
    mkdir $dir || usage "unable to create '$dir'"
fi
cd $dir

for i in base dns_dhcp hd sshd dhcp_client tools $tmpl_pkgs $*; do
    file=../config/$i.txt
    if [ -e $file ]; then
        name=`basename $file`
        [ -L $name ] && rm -f $name
        ln -s $file || echo "unable to create link for package '$i'"
    else
        echo "unknown package '$i'"
    fi
done

if [ -f _fli4l.txt ]; then
    echo "_fli4l.txt already exists, skip generation of _fli4l.txt"
    exit
fi

if [ "$ifaces" ]; then
    ifaces="# qemu_interfaces: $ifaces"
else
    ifaces="# qemu_interfaces: -nu -nu"
fi
cat <<EOF > _fli4l.txt
$ifaces

#
# Common debug options
#
# DEBUG_STARTUP='yes'
# DEBUG_MDEV='yes'
# DEBUG_ENABLE_CORE='yes'
# DEBUG_IPUP='yes'
# DEBUG_MODULES='yes'

#
# common options for qemu cd boot
#
SER_CONSOLE='yes'
BOOT_TYPE='cd'
MOUNT_BOOT='ro'

OPT_HDDRV='yes'
HDDRV_N='1'
HDDRV_1='ide-cd'

IP_NET_N='2'
IP_NET_2='dhcp'
IP_NET_2_DEV='eth1'

PF_INPUT_N='2'
PF_INPUT_1='tmpl:ssh ACCEPT'
PF_INPUT_2='tmpl:http ACCEPT'

#
# dhcp client
#
OPT_DHCP_CLIENT='yes'
DHCP_CLIENT_TYPE='dhcpcd'
DHCP_CLIENT_N='1'
DHCP_CLIENT_1_IF='IP_NET_2_DEV'
DHCP_CLIENT_1_ROUTE='none'
DHCP_CLIENT_1_USEPEERDNS='yes'

OPT_KLOGD='yes'
OPT_SYSLOGD='yes'
SYSLOGD_DEST_N='3'
SYSLOGD_DEST_3='*.* /var/log/foo/messages'

OPT_STRACE='yes'
OPT_NETSTAT='yes'

#
# sshd
#
OPT_SSHD='yes'
OPT_SCP='yes'
OPT_SSH_CLIENT='yes'
EOF

if [ -e ../config/etc/ssh/root@fli4l ]; then
    cat <<EOF >> _fli4l.txt
SSHD_PUBLIC_KEYFILES_N='1'
SSHD_PUBLIC_KEYFILE_1='root@fli4l'
EOF
    mkdir -p etc/ssh
    cp ../config/etc/ssh/root@fli4l etc/ssh
fi

if [ "$tmpl" ]; then
    first=yes
    cat <<EOF >> _fli4l.txt

#
# Values added by template $tmpl
#
EOF
    while read line; do
        var=${line%%=*}
        if grep -q "^$var=" _fli4l.txt; then
            [ "$first" ] && echo "Replacing template values:"
            echo -n "    - "
            grep "^$var=" _fli4l.txt
            echo "    + $line"
            sed -i -e "/$var=/s/^/# /" _fli4l.txt
        fi
        echo "$line" >> _fli4l.txt
    done < "$tmpl"
fi
