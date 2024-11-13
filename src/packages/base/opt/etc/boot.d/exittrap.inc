#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/exittrap.inc
# Functions for managing shell exit hooks (exit traps).
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$exittrap_api" != yes ]
then

. /etc/boot.d/string.inc
. /etc/boot.d/forking.inc

# signals to handle
exit_trap_signals="HUP:1 INT:2 QUIT:3 ILL:4 ABRT:6 FPE:8 USR1:10 SEGV:11 USR2:12 PIPE:13 ALRM:14 TERM:15"

# Adds an exit trap. It is executed when the shell exits, and before all other
# exit traps that might have been registered in the past.
#
# Signals cause the shell script to exit, such that exit traps are also called
# when the script exits abnormally. To prevent this, install a special signal
# trap (see exit_trap_install_for_signal() below).
#
# Input:
#   $1    = name of variable receiving the trap index
#   $2... = command to execute when the shell exits.
exit_trap_add_with_index()
{
    local res=$1
    shift
    [ -n "$*" ] || return 1
    eval $res=\$exit_trap_num
    pack_args exit_trap_${exit_trap_num} "$@"
    exit_trap_num=$((exit_trap_num+1))
    return 0
}

# Adds an exit trap without returning its index. Use this function if you never
# plan to remove the exit trap via exit_trap_remove().
#
# Input:
#   $1... = command to execute when the shell exits.
exit_trap_add()
{
    local dummy
    exit_trap_add_with_index dummy "$@"
}

# Installs a signal trap for a given signal. It is executed when the shell has
# received that signal. Only one signal trap can be installed for a given
# signal at the same time.
#
# If the signal trap returns zero, the default action is performed afterwards,
# i.e. the script is terminated via "exit".
#
# In addition to a command to be executed, the action can be "-", which means
# "install default action" (i.e. uninstall a previously installed signal trap),
# or "", which means "ignore this signal".
#
# Input:
#   $1    = signal name (HUP, INT, TERM etc.)
#   $2... = command to execute when the signal is received.
exit_trap_install_for_signal()
{
    local signal=$1
    shift
    case "$*" in
    "")
        trap "" $signal
        ;;
    *)
        if [ "$*" = "-" ]
        then
            unset exit_trap_${signal}
        else
            pack_args exit_trap_${signal} "$@"
        fi

        local sig
        for sig in $exit_trap_signals
        do
            case $sig in
            $signal:*)
                local sigidx=${sig#*:}
                trap "exit_trap_call_on_signal $sigidx $signal" $sigidx
                break
                ;;
            esac
        done
        ;;
    esac
}

# Installs for each signal a trap that returns a non-zero value.
exit_trap_ignore_all_signals()
{
    local sig
    for sig in $exit_trap_signals
    do
        local signame=${sig%%:*}
        exit_trap_install_for_signal $signame ""
    done
}

# Removes an exit trap.
#
# $1 = index of trap to remove
exit_trap_remove()
{
    [ -n "$1" ] || return 1
    unset exit_trap_$1
}

# (internal) Calls an old exit trap if existing.
#
# $1 = signal name
exit_trap_call_old()
{
    local signal=$1
    local old_trap=$(echo "$_old_traps" | sed -n "s/^trap -- '\(.*\)' $signal$/\1/p")
    [ -n "$old_trap" ] && eval "$old_trap"
    return 0
}

# (internal) Calls exit_trap_call when exiting.
# Don't call it by yourself, it is called by the shell.
exit_trap_call_on_exit()
{
    local i
    for i in $(seq -- $((exit_trap_num-1)) -1 0)
    do
        eval local cmdline=\$exit_trap_${i}
        [ -n "$cmdline" ] && eval "$cmdline"
    done
    exit_trap_num=0
    exit_trap_call_old EXIT
}

# (internal) Exits the script at receipt of a signal with the return code
# <signal number+128>. Calls a signal trap if installed, which may inhibit
# script termination by returning a non-zero value.
#
# Don't call it by yourself, it is called by the shell.
#
# $1 = signal number
# $2 = signal name
exit_trap_call_on_signal()
{
    local sigidx=$1 signame=$2 rc=0
    eval local cmdline=\$exit_trap_${signame}
    if [ -n "$cmdline" ]
    then
        eval "$cmdline"
        rc=$?
    fi
    if [ $rc -eq 0 ]
    then
        exit_trap_remove $signame
        exit_trap_call_old $signame
        exit $((128+$sigidx))
    fi
}

# Makes the exit_trap library usable by the current process.
exit_trap_init()
{
    # number of registered exit traps
    exit_trap_num=0

    _old_traps=$(trap)
    trap exit_trap_call_on_exit 0

    local sig
    for sig in $exit_trap_signals
    do
        local signame=${sig%%:*}
        local sigidx=${sig#*:}
        exit_trap_remove $signame
        trap "exit_trap_call_on_signal $sigidx $signame" $sigidx
    done
}

exittrap_api='yes'

fork_add_handler exit_trap_init
exit_trap_init

fi # $exittrap_api != yes
