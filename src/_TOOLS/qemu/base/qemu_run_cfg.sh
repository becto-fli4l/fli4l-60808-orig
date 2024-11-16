#! /bin/sh

port_base=10000
update_version=
output=-nographic
vnc_disp=1

extract_value ()
{
    sed -n -e "s/^$1='\([^']*\)'.*/\1/p" $2/build/rc.cfg
}

port_used ()
{
    netstat -lntp 2> /dev/null | grep -n :$1
}

qemu_nics=
qemu_vlan=1
add_nic ()
{
    type=$1
    case $2 in
        *=*) port=`echo $2 | sed -e 's/-n.=//'` ;;
        *) port=12345 ;;
    esac
    case $type in
        connect)
            if ! port_used $port; then
                usage "No one is listening on port $port, please start server first."
            fi
            type="socket,vlan=${qemu_vlan},${type}=127.0.0.1:$port"
            ;;
        listen)
            if port_used $port; then
                usage "Someone is already listening on port $port, please use a different port."
            fi
            type="socket,vlan=${qemu_vlan},${type}=127.0.0.1:$port"
            ;;
        user)
            type="user,vlan=${qemu_vlan},hostname=if${qemu_vlan}"
            ;;
        mcast)
            type="socket,vlan=${qemu_vlan},mcast=230.0.0.1:1234"
            ;;
    esac
    qemu_nics="$qemu_nics -net nic,vlan=${qemu_vlan},macaddr=52:54:00:12:34:5$qemu_vlan -net $type"
    qemu_vlan=`expr $qemu_vlan + 1`
}

usage ()
{
    [ "$1" ] && echo "Error: $1"
    cat <<EOF
usage:
    $0 [ -p <port base> ] [ -ut ] [ -nl=<port> ] [ -nc=<port> ] [ -nu ] [ -d[=<dir>]] [ -b hd ] [ -vnc [<disp>] ] <config dir>
        -p :           port base, all service ports are relocated using this offset
        -u :           set fli4l revision if installed from svn repository
        -t :           test version
        -nc :          connect to other qemu instance
        -nl :          listen for incoming connections from other qemu instances
        -nu :          user device
        -nm :          connect to other qemu instances using multicast
        -b hd :        boot from hard disk
        -d :           disk is located in <config>/hda.img
        -d=<file.img> : disk is located in <file.img>
	-usbdev :      enable usb and add usb device, e.g. 
			-usbdev host:13fe:3123 (taken from lsusb, user needs 
					     r/w access)
			-usbdev disk:<file.img>
        -snd : 	       add sound hardware (ess1370)
        -vnc [<disp>] : use vnc as display
        -xterm :        use xterm as display 
    possible config dirs:
EOF
    for i in */base.txt; do
        if grep -q "HOSTNAME=" $i 2> /dev/null; then
            echo "        `dirname $i`"
        fi
    done
    exit 1
}
handle_nic ()
{
    case $1 in
        -nl*)   add_nic listen $1
                ;;
        -nc*)   add_nic connect $1
                ;;
        -nu)    add_nic user
                ;;
        -nm)    add_nic mcast
                ;;
        *)      return 1
    esac
    return 0
}

while [ "$1" ]; do
    case $1 in
        -t)
                test_version=-t
                ;;
        -u)
                update_version=-u
                ;;
        -p)
                shift
                if [ "$1" ]; then
                    if echo "$1" | grep -q -e '^[0-9]\+$'; then
                        port_base=$1
                    else
                        usage "invalid port base '$1'"
                    fi
                else
                    usage "missing port base"
                fi
                ;;
        -xterm)
		output=
		;;
        -vnc)
                case $2 in
                        [0-9])
                                vnc_disp=$2
                                shift
                                ;;
			*)
				vnc_disp=0
				;;
                esac
                output="-vnc :$vnc_disp -k de"
                ;;
        -b)
                case $2 in
                    hd)
                        boot=c
                        use_hd=yes
                        ;;
                    *)  usage "unknown boot device $2" ;;
                esac
                shift
                ;;
        -d)
                use_hd=yes
                ;;
        -d=*)
                use_hd=yes
                name=`echo $1 | sed -e 's/-d=//'`
                disk_img=$name
                ;;
        -n*)    handle_nic $1 || usage ;;
        -usbdev)
                : ${usb:=-usb}
                usb="$usb -usbdevice $2"
                shift
                ;;
        -snd)
                sound="-soundhw es1370"
                ;;
        *)
                if [ -z "$config" ]; then
                    config=`echo $1 | sed -e 's#/*$##'`
                else
                    usage
                fi
                ;;
    esac
    shift
done

if [ ! -f $config/base.txt ]; then
    usage
fi

[ -f  $config/_fli4l.txt ] && type=`sed -n -e "s/^BOOT_TYPE='\([^']*\)'.*/\1/p" $config/_fli4l.txt`
[ ! "$type" ] && type=`sed -n -e "s/^BOOT_TYPE='\([^']*\)'.*/\1/p" $config/base.txt`
case $type in
    fd*) boot=a ;;
    hd*) boot=c ;;
    cd*) boot=d ;;
    *)   echo "Unknown or unhandled boot type '$type'" ; exit 1 ;;
esac

if [ ! "$qemu_nics" ]; then
    interfaces=`sed -n -e 's/# qemu_interfaces://p' $config/_fli4l.txt`
    if [ "$interfaces" ]; then
        echo "using interface definition from config file: $interfaces"
        for i in $interfaces; do
            handle_nic $i || usage
        done
    fi
fi


: ${qemu_nics:='-net nic,vlan=0 -net user,vlan=0,hostname=if1 -net nic,vlan=1 -net user,vlan=1,hostname=if2 -net nic,vlan=2 -net user,vlan=2,hostname=if3'}

if [ "$boot" == 'd' ]; then
    rm -f $config/build/fli4l.iso
    sh mkfli4l.sh --no-squeeze $update_version $test_version $config || exit
elif [ "$boot" == 'a' ]; then
    floppy="-fda $config/build/floppy.img"
    export MTOOLSRC="$config/build/mtoolsrc"
    echo "drive a: file=\"$config/build/floppy.img\"" > $MTOOLSRC
    sh mkfli4l.sh $update_version $test_version $config || exit
fi

echo "boot configuration: "
{
    for i in BOOT_TYPE OPT_HDDRV HDDRV_ SER_CONSOLE= DEBUG_STARTUP DEBUG_LOG; do
        grep -e "^$i" $config/build/rc.cfg
    done
} | while read line; do
    echo "    $line"
done
echo

if [ "$boot" = 'd' ]; then
    if ! grep -q "^HDDRV_[0-9]='ide-cd'" $config/build/rc.cfg; then
        cat <<EOF
Incomplete boot configuration, please add ide-cd to your hd driver section,
for instance like follows"
    OPT_HDDRV='yes'
    HDDRV_N='1'
    HDDRV_1='ide-cd'
EOF
        exit
        if [ ! -f $config/build/fli4l.iso ]; then
            if ! grep -q "^BOOT_TYPE='cd'" $config/build/rc.cfg; then
                echo "Missing fli4l iso image, please add BOOT_TYPE='cd' to your configuration"
            fi
            exit
        fi
    fi
fi

if ! grep -q "^SER_CONSOLE='yes'" $config/build/rc.cfg; then
    case $output in
        -nographic)
                echo "Please add SER_CONSOLE='yes' to your configuration"
                exit
                ;;
    esac
fi

echo "Port redirections:"
redir_msg="    NONE"
redir=
for i in 37 53 udp:53 SSHD_PORT HTTPD_PORT IMOND_PORT; do
    prot=tcp
    case $i in
        [A-Z]*)
            case $i in
                tcp* | udp*)
                    set `echo $i | sed -e 's/:/ /'`
                    prot=$1
                    port=$2
                    ;;
                *)  port=`extract_value $i $config` ;;
            esac
            ;;
        *)  port=$i ;;
    esac
    case $port in
        '') ;;
        *)
            redir_msg=
            rport=`expr $port + $port_base`
            if port_used $rport; then
                usage "invalid port base $port_base, port $rport already used by someone else"
            else
                redir="$redir -redir $prot:$rport::$port"
                printf "    %-15s: $rport, $prot\n" $i
            fi
            ;;
    esac
done
echo "$redir_msg"

if [ -f $config/build/fli4l.iso ]; then
    cdrom="-cdrom $config/build/fli4l.iso"
fi

if [ "$use_hd" ]; then
    if [ ! "$disk_img" ]; then
        disk_img=$config/hda.img
    fi
    hd="-hda $disk_img"

    if [ ! -f $disk_img ]; then
        usage "missing disk image '$disk_img'"
    fi
fi

echo qemu -m 256 $floppy $cdrom $hd -boot $boot \
	-drive file=$config/build/scsi.dsk,if=scsi,bus=0,unit=6 \
	$sound	       			       \
        $usb                                   \
        $output                                \
        $qemu_nics                             \
        $redir                                 \

qemu -m 256 $floppy $cdrom $hd -boot $boot \
	$sound  			  \
        $usb                              \
        $output                           \
        $qemu_nics                        \
        $redir                            \
    | tee $config/build/$config.log

# -drive file=$config/build/scsi.dsk,if=scsi,bus=0,unit=6 \
exit 0
