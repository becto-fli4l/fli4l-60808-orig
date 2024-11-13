#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/env.inc
# Functions for environment inspection.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$env_api" != yes ]
then

get_value ()
{
    local name=$1
    eval res=\$$name
    case "x$res" in
    x)
        set | grep -q "^$name="
        ;;
    *)
        return 0
        ;;
    esac
}

# Determine the names of variables. Expects a list of variable assignments to
# be passed through <stdin>, like the list produced by "set". The keyword
# "local" can prepend the assignments and is ignored.
#
# Output:
#   String with the names of all variables in the list.
extract_variable_names()
{
    local vars= line var skip=0
    while read -r line
    do
        if [ $skip -eq 0 ]
        then
            var=${line#local }
            var=${var%%=*}
            vars="$vars $var"
        fi

        local quotes=$(printf "%s" "$line" | sed "s/'\"'\+\"/'/g;s/[^']//g;s/''//g")
        [ -n "$quotes" ] && skip=$((1-skip))
    done
    echo $vars
}

env_api='yes'

fi # $env_api != yes
