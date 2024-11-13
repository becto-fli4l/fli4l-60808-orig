#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/lazy.inc
# Functions for lazy execution.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$lazy_api" != yes ]
then

lazy_api='yes'

lazy_nesting_counter=0

lazy_early_cmd_list_counter=0
lazy_normal_cmd_list_counter=0
lazy_late_cmd_list_counter=0

# Starts a lazy block.
lazy_begin()
{
    if [ $lazy_nesting_counter -eq 0 ]
    then
        lazy_early_cmd_list_counter=0
        lazy_normal_cmd_list_counter=0
        lazy_late_cmd_list_counter=0
    fi
    lazy_nesting_counter=$((lazy_nesting_counter + 1))
}

# Ends a lazy block. If this is the top-level block, all delayed executions
# registered by lazy_executed() are performed.
lazy_end()
{
    lazy_nesting_counter=$((lazy_nesting_counter - 1))
    if [ $lazy_nesting_counter -eq 0 ]
    then
        lazy_execute_commands early
        lazy_execute_commands normal
        lazy_execute_commands late
    fi
}

# Executes a command lazily, i.e. its execution is delayed until the top-level
# lazy block ends. If there is no surrounding lazy block, the command is
# executed immediately.
#
# If a lazy command is idempotent, it is executed only once within its lazy
# block, even if lazy_execute() has been called multiple times. For this to
# be effective, the first parameter to this function must be "--idempotent",
# and all command invocations must coincide in both the command and its
# arguments. Compared to other lazy commands, the execution of an idempotent
# lazy command is linked to the first call of lazy_execute(), not to the last
# call.
#
# All commands registered by this function are executed before all normal
# and late commands.
#
# Input:
#   $1 = command;
#        if this argument equals "--idempotent", the command's execution is
#        marked as "idempotent" (see above), and the command and its arguments
#        start at the second parameter.
#   $2... = command arguments
lazy_execute_early()
{
    lazy_execute_common early "$@"
}

# Executes a command lazily, i.e. its execution is delayed until the top-level
# lazy block ends. If there is no surrounding lazy block, the command is
# executed immediately.
#
# If a lazy command is idempotent, it is executed only once within its lazy
# block, even if lazy_execute() has been called multiple times. For this to
# be effective, the first parameter to this function must be "--idempotent",
# and all command invocations must coincide in both the command and its
# arguments. Compared to other lazy commands, the execution of an idempotent
# lazy command is linked to the first call of lazy_execute(), not to the last
# call.
#
# All commands registered by this function are executed after all early and
# before all late commands.
#
# Input:
#   $1 = command;
#        if this argument equals "--idempotent", the command's execution is
#        marked as "idempotent" (see above), and the command and its arguments
#        start at the second parameter.
#   $2... = command arguments
lazy_execute()
{
    lazy_execute_common normal "$@"
}

# Executes a command lazily, i.e. its execution is delayed until the top-level
# lazy block ends. If there is no surrounding lazy block, the command is
# executed immediately.
#
# If a lazy command is idempotent, it is executed only once within its lazy
# block, even if lazy_execute() has been called multiple times. For this to
# be effective, the first parameter to this function must be "--idempotent",
# and all command invocations must coincide in both the command and its
# arguments. Compared to other lazy commands, the execution of an idempotent
# lazy command is linked to the first call of lazy_execute(), not to the last
# call.
#
# All commands registered by this function are executed after all early and
# normal commands.
#
# Input:
#   $1 = command;
#        if this argument equals "--idempotent", the command's execution is
#        marked as "idempotent" (see above), and the command and its arguments
#        start at the second parameter.
#   $2... = command arguments
lazy_execute_late()
{
    lazy_execute_common late "$@"
}

# Input:
#   $1 = command list to use ("early", "normal", or "late")
#   $2 = command
#   $3... = arguments
lazy_execute_common()
{
    local list=$1 cmd=$2 idempotent
    shift 2

    if [ "$cmd" = "--idempotent" ]
    then
        # idempotent command: don't add command to list if it is already there
        # (and the arguments concide)
        idempotent=idempotent
        cmd=$1
        shift
    fi

    if [ $lazy_nesting_counter -eq 0 ]
    then
        "$cmd" "$@"
    else
        local args
        pack_args args "$@"
        lazy_add_command $list "$cmd" "$args" $idempotent
    fi
}

# Adds a command to some command list (internal).
# Input:
#   $1 = command list to use ("early", "normal", or "late")
#   $2 = command
#   $3 = arguments (packed)
#   $4 = non-empty if command is idempotent
lazy_add_command()
{
    local list=$1 cmd=$2 args=$3 idempotent=$4 num

    eval num=\$lazy_${list}_cmd_list_counter
    if [ -n "$idempotent" ]
    then
        # check whether the command (and its arguments) are already in the
        # list of commands to be executed
        local i
        for i in $(seq 1 $num)
        do
            eval local old_cmd=\$lazy_${list}_cmd_list_${i}_cmd
            eval local old_args=\$lazy_${list}_cmd_list_${i}_args
            [ "$cmd" = "$old_cmd" -a "$args" = "$old_args" ] && return 0
        done
    fi

    num=$((num + 1))
    eval lazy_${list}_cmd_list_counter=\$num
    eval lazy_${list}_cmd_list_${num}_cmd=\$cmd
    eval lazy_${list}_cmd_list_${num}_args=\$args
}

# Executes the registered commands (internal).
# Input:
#   $1 = command list to use ("early", "normal", or "late")
lazy_execute_commands()
{
    local list=$1 i
    eval local num=\$lazy_${list}_cmd_list_counter
    for i in $(seq 1 $num)
    do
        eval local cmd=\$lazy_${list}_cmd_list_${i}_cmd
        eval local args=\$lazy_${list}_cmd_list_${i}_args
        eval $cmd "$args"
    done
}

fi # $lazy_api != yes
