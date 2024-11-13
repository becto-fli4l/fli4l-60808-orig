#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/string.inc
# Miscellaneous string helper functions.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$string_api" != yes ]
then

# Masks a shell argument such that eval()'ing it returns the original argument.
# This is done by prepending backslashes to shell operators.
#
# $1 <- shell string
# output -> masked shell string;
#           eval echo "$output" returns the original argument
mask_shell_string()
{
    echo "$1" | sed "s/\([\\$\"'\`()[{]\)/\\\\\1/g"
}

# Transforms a string into a valid shell identifier by converting
# non-conformant characters into underscores. Underscores already present in
# the input string are doubled.
# NOTE: The caller needs to ensure that the first character is not a number,
# for example by prepending an appropriate prefix.
#
# $1 = string to transform
# Output: transformed string
# Example: "to_shell_id target:1.2.3.4-xy-1::abcd" --> "target_1_2_3_4_xy_1__abcd"
to_shell_id()
{
    echo -n "$1" | sed 's/_/__/g' | tr -c 'a-zA-Z0-9_' '_'
}

# Packs arguments into a single string.
#
# Input:
#   $1 = name of variable receiving the arguments in such a way that
#        (re)evaluating them provides the original arguments
#   $2... = arguments
# Example:
#   func() {
#     local args
#     pack_args args "$@"
#     send_rpc func_impl "$args"
#   }
#   # (in another process)
#   receive_rpc() {
#     local func="$1"
#     local args="$2"
#     eval $func "$args"
#   }
pack_args()
{
    local _p _resvar=$1 _result= _value
    shift
    for _p
    do
        _result="$_result '${_p//'/'\"'\"'}'"
    done
    eval $_resvar=\${_result\# }
}

string_api='yes'

fi # $string_api != yes
