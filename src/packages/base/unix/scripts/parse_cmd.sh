#!/bin/sh
# -------------------------------------------------------------------------
# parse command line                                           __FLI4LVER__
#
# Creation:       lanspezi 2004-10-25
# Last Update:    $Id$
# -------------------------------------------------------------------------

show_header ()
{
    echo -n "$1"
    if [ "${#1}" -lt 78 ]
    then
	len=`expr 78 - ${#1}`
	for i in `seq 1 $len`
	do
            echo -n '-'
	done
    fi
    echo
}

show_end ()
{
    case x$1 in
        x) ;;
        *)
            case x$verbose in
                x) echo ;;
            esac
            ;;
    esac
    echo "------------------------------------------------------------------------------"
    echo ""
}

log_error ()
{
    case $# in
    0)
        while read line
        do
            echo "  $line"
            echo "  $line" >> /tmp/mkfli4l_error.log
        done
        ;;
    *)
        echo "$*"
        echo "$*" >> /tmp/mkfli4l_error.log
        ;;
    esac
}

append_error ()
{
    cat "$1" >> /tmp/mkfli4l_error.log
}

abort ()
{
    case "$1" in
        '') ;;
        -) log_error ;;
        *) log_error "$*" ;;
    esac

    if [ -d "$dir_build" ];  then
        cat <<EOF > "$dir_build/mkfli4l_error.log"
An error occurred while creating fli4l-files.
=============================================
`cat /tmp/mkfli4l_error.log`
EOF
        echo -n "" > "$dir_build/mkfli4l_error.flg"
        cat "$dir_build/mkfli4l_error.log"
        echo ""
        if [ "$bool_batch" = "false" ]; then
            echo "hit RETURN to continue ..."
            read dummy
        fi
  else
        cat <<EOF
An error occurred while creating fli4l-files.
=============================================
`cat /tmp/mkfli4l_error.log`
EOF
    fi
    rm -f /tmp/mkfli4l_error.log
    cleanup_fli4lfiles
    exit 1
}

usage ()
{
  cat unix/scripts/mkfli4l.usage
}

set_defaults ()
{
  ### set default parameters
  # logic - boolean
  bool_error=true
  bool_filesonly=false
  bool_cleanup=false
  bool_imonc=false
  bool_rebuild=false
  bool_update_ver=false
  bool_test=false
  bool_batch=false
  bool_yes=false

  # pathnames
  dir_config=
  dir_build=
  dir_image=

  # others
  verbose=
  suffix=
  mkfli4l_debug_option=

  # scp - remoteupdate
  remoteusername=fli4l
  remotehostname=
  remotepathname=/boot
  remoteupdate=false
  remoteport=
  remoteremount=false

  # ssh - reboot
  reboot=false

  # tftpboot
  tftpboot_path=/tftpboot
  tftpboot_image=/fli4l.tftp
  pxe_subdir=fli4l

  # direct hd-install
  hdinstall_path=

  # create kernel package
  mk_pkg=
}

parse_options ()
{
    # parse command line
    dir_config=
    while [ "$1" ]; do
        case "$1" in
            -v|--verbose) verbose='-v' ;;
            --batch) bool_batch=true ;;
            --yes) bool_yes=true ;;
            -b|--build)
                dir_build=$2
                shift
                ;;
            -u|--update-ver) bool_update_ver=true ;;
            -t|--test-ver) bool_test=true ;;
            -c|--clean) bool_cleanup=true ;;
            -k|--kernel-pkg) mk_pkg=true ;;
            --rebuild) bool_rebuild=true ;;
            --filesonly) bool_filesonly=true ;;
            --remotehost)
                remotehostname=$2
                shift
                ;;
            --remoteuser)
                remoteusername=$2
                shift
                ;;
            --remotepath)
                remotepathname=$2
                shift
                ;;
            --remoteport)
                remoteport=$2
                shift
                ;;
            --remoteupdate)
                remoteupdate=true
                ;;
            --remoteremount)
                remoteremount=true
                ;;
            --tftpbootpath)
                tftpboot_path=$2
                shift
                ;;
            --tftpbootimage)
                tftpboot_image=$2
                shift
                ;;
            --pxesubdir)
                pxe_subdir=$2
                shift
                ;;
            --imonc)
                bool_imonc=true
                ;;
            --no-squeeze)
                bool_no_squeeze=true
                ;;
            --hdinstallpath)
                hdinstall_path=$2
                shift
                ;;
            -*|-h|--help)
                usage
                exit 0
                ;;
            *)
                if [ "$dir_config" ]; then
                    usage
                    exit 1
                fi
                dir_config="`echo "$1" | sed -e 's,/*$,,'`"
                ;;
        esac
        shift
    done

    # set default config-dir if non is parsed from the command-line
    if [ ! "${dir_config}" ]; then
        dir_config=config
    fi
}

parse_cmdline ()
{
    if [ "$_cmdline_parsed" = true ]; then
        return
    fi

    echo ""
    echo "fli4l __FLI4LVER__ - starting build process"
    echo "=============================================================================="
    echo ""

    # check for current dir on exec the script
    [ -f opt/base.txt ] ||  abort "- execute/run script mkfli4l.sh from your fli4l-basedir"

    set_defaults
    parse_options "$@"

    case "x$dir_build" in
        x) dir_build="$dir_config/build" ;;
    esac

    mkfli4l=$(lookup_tool mkfli4l)

    if [ "$bool_rebuild" = true -o ! -n "$mkfli4l" ] || \
         ! $mkfli4l -h > /dev/null 2>&1
    then
        show_header "--- rebuilding mkfli4l"
        if command -v gmake >/dev/null
        then
            make=gmake
        elif command -v make >/dev/null
        then
            make=make
        else
            log_error "Failed to (re)build mkfli4l as GNU make could not be found, aborting..."
            abort
        fi

        DEBUG=$verbose
        if ! $make -C src DEBUG=$verbose clean || ! $make -C src DEBUG=$verbose
        then
            log_error "Failed to (re)build mkfli4l (error code $?): cannot continue, aborting..."
            abort
        fi

        show_end
    fi

    echo "using configuration from directory '$dir_config'..."
    echo ""

    # check for existence of base.txt in config-dir
    if [ ! -f "$dir_config/base.txt" ]; then
        log_error "- directory $dir_config does not contain base.txt -- did you choose the right directory?"
        usage
        abort
    fi

    # check for existence of mkfli4l.conf in config-dir
    if [ -f "$dir_config/mkfli4l.conf" -a ! -f "$dir_config/mkfli4l.txt" ]; then
        echo "migrating mkfli4l.conf to mkfli4l.txt..."
        echo
        mv "$dir_config/mkfli4l.conf" "$dir_config/mkfli4l.txt"
    fi

    # check for existence of mkfli4l.txt in config-dir
    if [ -f "$dir_config/mkfli4l.txt" ]; then
          # check the conf-file
        . unix/scripts/parse_mkfli4l_conf.sh
        if ! parse_mkfli4l_conf; then
            abort
        fi
    fi

    dir_config=
    parse_options "$@"
    dir_image="$dir_build/image"

    drive=`echo $drive | sed -e 's/://'`

    # check for build-dir different from root-dir
    if [ "$dir_build" = "/" ]; then
        abort "- build-directory must be different from filesystem root"
    fi

    # check existence of tftpboot_path
    if [ ! -d "$tftpboot_path" -a "$boot_type" = "netboot" ] ||
        [ ! -d "$tftpboot_path" -a "$boot_type" = "pxeboot" ]; then
        abort  "- directory $tftpboot_path does not exist"
    fi

    # check exist of build-dir
    if [ ! -e "$dir_build" ]; then
        mkdir -p "$dir_build"
    else
        if [ ! -d "$dir_build" -o ! -w "$dir_build" ]; then
            abort "- build dir $dir_build is either not a directory or not writable"
        fi
    fi


    boot_type=
    # check boot_type from base.txt
    if [ -f "$dir_config/_fli4l.txt" ]; then
        boot_type=`sed -n -e "s/^[[:space:]]*BOOT_TYPE=[\"']\(.*\)[\"'].*/\1/p" "$dir_config/_fli4l.txt"`
    fi
    if [ -z "$boot_type" ]; then
        boot_type=`sed -n -e "s/^[[:space:]]*BOOT_TYPE='\(.*\)'.*/\1/p" "$dir_config/base.txt"`
    fi

    _cmdline_parsed=true
}

