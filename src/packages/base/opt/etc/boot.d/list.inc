#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/list.inc
# Functions for working with lists.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$list_api" != yes ]
then

# Checks whether an element is part of a list.
# Input:
#   $1 = element
#   $2... = list
# Return code:
#   0 if the element is part of the list, 1 otherwise
list_is_in()
{
    local e=$1 i
    shift
    for i
    do
        [ "$e" = "$i" ] && return 0
    done
    return 1
}

# Transforms a list into a set (without duplicate elements).
# Input:
#   $1... = list
# Output:
#   A set with the same elements as the list.
list_unique()
{
    local e result
    for e
    do
        list_is_in $e $result || result="$result $e"
    done
    echo ${result# }
}

# Computes the subtraction of two lists.
# Input:
#   $1 = first list (as single argument!)
#   $2... = second list
# Output:
#   All elements of the first list that are not in the second list.
list_sub()
{
    local e1 e2 result list=$1
    shift
    for e1 in $list
    do
        list_is_in $e1 $* || result="$result $e1"
    done
    echo ${result# }
}

# Computes an intersection of two lists.
# Input:
#   $1 = first list (as single argument!)
#   $2... = second list
# Output:
#   All elements of the first list that are also in the second list.
list_intersect()
{
    local e1 e2 result list=$1
    shift
    for e1 in $list
    do
        list_is_in $e1 $* && result="$result $e1"
    done
    echo ${result# }
}

# Returns the first element of a list
# Input:
#   $1... = list
# Output:
#   The first element in the list or nothing if the list is empty.
# Return code:
#   0 if the list is non-empty, 1 otherwise
list_first()
{
    [ -n "$1" ] || return 1
    echo $1
}

# Returns the last element of a list
# Input:
#   $1... = list
# Output:
#   The last element in the list or nothing if the list is empty.
# Return code:
#   0 if the list is non-empty, 1 otherwise
list_last()
{
    [ -n "$1" ] || return 1
    local num=$#
    eval echo \${$num}
}

# Executes a command on each list element. The command is eval'ed before being
# executed. It has to reference "$1" in order to get the list element passed.
# Input:
#   $1    = command to be executed on each list element
#   $2... = list
# Example 1:
#     list_foreach "echo [\$1]" a b c
#   results in the output:
#     [a]
#     [b]
#     [c]
# Example 2:
#     list_foreach "echo \${1%/*}" 1.2.3.4/24 5.6.7.8/32
#   results in the output:
#     1.2.3.4
#     5.6.7.8
# Example 3:
#     list_foreach "echo \$1 | sed 's/\\./\\\\./g'" $res
#   where $res is the result of example 2 yields the output:
#     1\.2\.3\.4
#     5\.6\.7\.8
#
list_foreach()
{
    local e cmd=$1
    shift
    for e
    do
        set -- "$e"
        eval $cmd
    done
}

# Filters a list. The filter is eval'ed before being executed. It has to
# reference "$1" in order to get the list element passed. Only if the filter
# returns zero the list element is printed.
# Input:
#   $1    = filter to be executed on each list element
#   $2... = list
# Example 1:
#     list_filter "[ -e \$1 ]" /etc /bla /root
#   results in the output:
#     /etc
#     /root
#   (provided there is no file or directory "/bla")
# Example 2:
#     list_filter "netcalc isipv4 \$1" 1.2.3.4 ::1:2:3
#   results in the output:
#     1.2.3.4
#
list_filter()
{
    local e filter=$1
    shift
    for e
    do
        set -- "$e"
        eval $filter >/dev/null 2>&1 && echo "$e"
    done
}

list_api='yes'

fi # $list_api != yes
