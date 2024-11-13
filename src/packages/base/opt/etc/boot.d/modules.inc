#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/modules.inc
# Functions for working with kernel modules.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$modules_api" != yes ]
then

mod_ext=ko

begin_probe () {
    probe='yes'
}

end_probe () {
    probe=''
}

do_insmod () {
    log_error "invoking insmod (directly or via one of the do_insmod variants), please try to eleminate it or replace it with do_modprobe"
    /sbin/insmod $* > /tmp/insmod.res.$$ 2>&1
    res=$?
    if [ ! "$probe" -o "$debug_active" = 'yes' ]
    then
        case $res in
            0)  ;;
            *)
                log_error "insmod $module $* failed!"
                log_error < /tmp/insmod.res.$$
                ;;
        esac
        cat /tmp/insmod.res.$$
    fi
    rm -f /tmp/insmod.res.$$
    return $res
}

do_modprobe () {
    quiet=
    if [ "$1" = "-q" ]
    then
        quiet=-q
        shift
    fi
    /sbin/modprobe $* > /tmp/insmod.res.$$ 2>&1
    res=$?
    case $res in
        0)  ;;
        *)
            if [ -z "$quiet" ]
            then
                log_error "modprobe $* failed!"
                log_error < /tmp/insmod.res.$$
            fi
            ;;
    esac
    [ -z "$quiet" ] && cat /tmp/insmod.res.$$
    rm -f /tmp/insmod.res.$$
    return $res
}

# $1 = module_path relative to /lib/modules/kernel_version
do_modprobe_if_exists () {
    quiet=
    if [ "$1" = "-q" ]
    then
        quiet=-q
        shift
    fi
    local kernel_version=$(uname -r)
    if ! grep -q "^alias $2 " /lib/modules/$kernel_version/modules.alias
    then
        [ -f /lib/modules/$kernel_version/$1/$2.$mod_ext ] || return 0
    fi

    shift
    do_modprobe $quiet $*
}

do_insmod_once () {
    grep -q "^$1 " /proc/modules && return 0

    do_insmod $*
    # returns the result of do_insmod
}

# $1 = module_path relative to /lib/modules/kernel_version
do_insmod_if_exists () {
    local kernel_version=$(uname -r)
    if ! grep -q "^alias $2 " /lib/modules/$kernel_version/modules.alias
    then
        [ -f /lib/modules/$kernel_version/$1/$2.$mod_ext ] || return 0
    fi

    shift
    do_insmod $*
}

# $1 = module_path relative to /lib/modules/kernel_version
do_insmod_if_exists_once () {
    grep -q "^$2 " /proc/modules && return 0

    do_insmod_if_exists $*
}

modules_api='yes'

fi # $modules_api != yes
