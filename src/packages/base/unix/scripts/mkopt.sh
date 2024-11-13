#! /bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# Creates a new opt.img and a new rootfs.img concerning to the configuration
# in config/
#
# Last Update:    $Id$
#------------------------------------------------------------------------------

. ./unix/scripts/bootloader.sh

# array holding files/directories to be installed
# each entry consists of three strings:
# - the source file (qualified)
# - the target file (unqualified, as $dir_image/ is prepended automatically)
# - 1 if checksum should be computed, 0 otherwise
file_list=

cleanup_mkopt()
{
    rm -rf "$dir_build/tmp"
}

gen_md5_sums()
{
    for i; do
        name=`echo $i | sed -e 's/[^a-zA-Z0-9_]/_/g' | tr a-z A-Z`
        sum=
        if [ -f "$dir_image/$i" ]; then
            set `md5sum -b "$dir_image/$i"`
            sum=$1
        fi
        echo "FLI4L_MD5_$name='$sum'"
    done
}

get_kernel_version()
{
    kernel_version=
    if [ -f "$dir_config"/_fli4l.txt ]; then
        get_one_val kernel_version   KERNEL_VERSION   "$dir_config"/_fli4l.txt "" cont || true
    fi
    if [ -z "$kernel_version" ]; then
        get_one_val kernel_version   KERNEL_VERSION   "$dir_config"/base.txt
    fi
    if [ -z "$kernel_version" ]; then
        log_error "Can't determine kernel version"
        exit 1
    fi
}
gen_kernel_package()
{
    show_header "--- generating kernel package "
    get_kernel_version
    dep=opt/lib/modules/$kernel_version/modules.dep

    if [ ! -f $dep ]; then
        log_error "Unable to open $dep, check kernel version"
        show_end
        exit 1
    fi

    cat <<EOF > "$dir_config"/_kernel.txt
#
# generated for kernel version __${kernel_version}__
#
COMPLETE_KERNEL='no'
COMPLETE_KERNEL_VERSION='$kernel_version'
EOF
    cat <<EOF > check/_kernel.txt
COMPLETE_KERNEL         -               - YESNO
COMPLETE_KERNEL_VERSION COMPLETE_KERNEL - NOTEMPTY
EOF

    {
        echo "opt_format_version 1 -"
        sed -n -e 's#^\([^[:space:]]*[^/]\{1,\}\)\.ko:.*#\1#p' opt/lib/modules/$kernel_version/modules.dep |\
            while read mod; do
                case $mod  in
                    extra/*) ;; # ignore extra modules
                    *) echo "complete_kernel yes `basename $mod`"
                esac
        done
    } > opt/_kernel.txt
    show_end
    exit
}
check_kernel_package()
{
    get_one_val opt_complete_kernel COMPLETE_KERNEL "$dir_config"/_kernel.txt > /dev/null
    if [ "$opt_complete_kernel" != no ]; then
        show_header "--- checking kernel package "
        get_one_val complete_kernel_version COMPLETE_KERNEL_VERSION "$dir_config"/_kernel.txt
        get_kernel_version

        if [ "$kernel_version" != "$complete_kernel_version" ]; then
            log_error "Mismatch between kernel version ('$kernel_version') and generated kernel package ('$complete_kernel_version')"
            show_end
            exit 1
        fi
        show_end
    fi
}
update_version ()
{
    show_header "--- updating fli4l version string "
    ver=`cat version.txt`
    version_file=opt/etc/version
    version=`cat $version_file`
    case $version in
        __FLI4LVER__)
            cwd=`pwd`
            if [ -L $version_file ]; then
                link=`readlink $version_file`
                cd `dirname $link`
                version_file=version
            fi
            if svn info $version_file > /tmp/svnver.$$ ; then
                set `grep Revision /tmp/svnver.$$`
                cd $cwd
                echo "  setting version to '$ver-rev$2'"
                echo "$ver-rev$2" > $dir_config/etc/version
            else
                cd $cwd
                echo "   ... unable to determine svn version, leaving version unchanged"
            fi
            ;;
    esac
    show_end
}

get_one_val()
{
    var=$1
    token=$2
    file="$3"
    default=$4
    cont=$5
    _tmp=`sed -n -e "s/^[[:space:]]*$token=[\"']\([^\"']*\)[\"'].*/\1/p" "$file"`
    case x$_tmp in
        x)
            if [ "$default" ]; then
                eval $var=$default
                echo "  $token='$default'"
            else
        if [ "$cont" ]; then
            return 1
        else
            abort "unable to lookup $token (and no default value present)"
        fi
            fi
            ;;
        *)
            eval $var=$_tmp
            echo "  $token='$_tmp'"
            ;;
    esac
}
get_config_values()
{
    cfg="$dir_image/rc.cfg"
    show_header "--- extracting some information from $cfg "

    get_one_val arch  ARCH  "$cfg"
    get_one_val kernel_version  KERNEL_VERSION  "$cfg"
    kernel_version_id=$(echo $kernel_version | sed -e 's/[.-]/_/g' | tr "[[:lower:]]" "[[:upper:]]")

    get_one_val comp_type_rootfs COMP_TYPE_ROOTFS "$cfg" gzip
    get_one_val comp_type_opt COMP_TYPE_OPT "$cfg" lzma

    if [ ! "$remotehostname" ]
    then
        get_one_val remotehostname HOSTNAME "$cfg"
    fi
    get_one_val bool_recover     OPT_RECOVER      "$cfg" no
    show_end
}

copy_kernel ()
{
    file=opt/img/linux-$kernel_version/kernel

    if [ -f "$dir_config"/$file ]
    then
        cp "$dir_config"/$file "$dir_image"/
    elif [ -f $file ]
    then
        cp $file "$dir_image"/
    else
        abort "Can't find $file! Either use a different kernel version or a different compression method."
    fi
}

copy_dtbs ()
{
    dtbdir="$dir_image/dtbs"
    for dtb in opt/img/linux-$kernel_version/*.dtb
    do
        [ -f "$dtb" ] || continue
        mkdir -p "$dtbdir"
        if [ -f "$dir_config"/$dtb ]
        then
            cp "$dir_config"/$dtb "$dtbdir"/
        else
            cp $dtb "$dtbdir"/
        fi
    done
}

set -e

# include function to check command-line
. ./unix/scripts/parse_cmd.sh

# check parameters from commandline and current env
parse_cmdline $*

case $bool_no_squeeze in
    true) squeeze_file=--no-squeeze ;;
    *)    squeeze_file= ;;
esac

mkdir -p "$dir_config/etc"

# remove all old versions of fli4l-file in build-dir
. ./unix/scripts/_lib_cleanup.sh
cleanup_fli4lfiles

echo -n "" > "$dir_config/etc/rc.cfg"

mkdir -p "$dir_image"

if [ "$mk_pkg" ]; then
    gen_kernel_package
elif [ -f "$dir_config"/_kernel.txt ]; then
        check_kernel_package
fi

show_header "--- check configuration in directory \"$dir_config\" and create archives "
mkdir -p "$dir_build/tmp"
if ! $(lookup_tool mkfli4l) $mkfli4l_debug_option -c "$dir_config" -t "$dir_build/tmp" -b "$dir_image" -l "$dir_build/mkfli4l.log" $squeeze_file
then
    append_error  "$dir_build"/mkfli4l.log
    cleanup_mkopt
    abort
else
    show_end

    cp "$dir_config/etc/rc.cfg" "$dir_image/rc.cfg"

    get_config_values

    case $bool_update_ver in
        true) update_version ;;
    esac

    copy_kernel
    copy_dtbs
    boot_loader_post_build $arch "$dir_image"

    if [ "$bool_test" = false ]; then
        file_list="$dir_image/rc.cfg rc.cfg 1"
        file_list="$file_list $dir_image/kernel kernel 1"
        file_list="$file_list $dir_image/rootfs.img rootfs.img 1"
    else
        file_list="$dir_image/rc.cfg rc-test.cfg 1"
        file_list="$file_list $dir_image/kernel kernel3 1"
        file_list="$file_list $dir_image/rootfs.img rootfs3.img 1"
    fi
    case $bool_recover in
        yes)
            file_list="$file_list img/boot.msg boot.msg 0"
            file_list="$file_list img/boot_s.msg boot_s.msg 0"
            file_list="$file_list img/boot_z.msg boot_z.msg 0"
            ;;
    esac
    if [ -f "$dir_image/opt.img" ]
    then
        if [ "$bool_test" = false ]; then
            file_list="$file_list $dir_image/opt.img opt.img 1"
        else
            file_list="$file_list $dir_image/opt.img opt-test.img 1"
        fi
    fi
    if [ -d "$dir_image/dtbs" ]
    then
        file_list="$file_list $dir_image/dtbs dtbs 0"
    fi
    file_list="$file_list $(boot_loader_get_entries $arch "$dir_image")"

    targets=
    md5targets=
    set -- $file_list
    while [ -n "$1" ]
    do
        source=$1 target=$2 do_md5=$3
        shift 3
        [ $do_md5 -ne 0 ] && md5targets="$md5targets $target"
        targets="$targets $target"
        target=$dir_image/$target

        [ "$source" = "$target" ] && continue
        [ -d "$target" ] && rm -rf "$target"

        case $source in
        $dir_image/*)
            mv "$source" "$target"
            res=$?
            ;;
        *)
            cp -a "$source" "$target"
            res=$?
            ;;
        esac
        if [ $res -ne 0 ]
        then
            log_error "could not copy/move $source to $target"
            cleanup_mkopt
            abort
        fi
    done

    show_header "--- generating md5 sums"
    gen_md5_sums $md5targets >> "$dir_image/rc.cfg"
    grep ^FLI4L_MD5_ "$dir_image/rc.cfg"
    cleanup_mkopt
    show_end

fi

echo "creation of all build-files finished..."
echo ""
mkopt_already_run=1

# be paranoid and set tmp_dir again
# vim: set ts=8 sw=4 sts=4:
