#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/set.inc
# Functions for working with sets.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$set_api" != yes ]
then

. /etc/boot.d/list.inc

# Computes the union of two sets.
# Input:
#   $1 = first set (as single argument!)
#   $2... = second set
# Output:
#   A set with all elements of the first and second sets.
set_union()
{
    local set=$1
    shift
    list_unique $set $*
}

set_api='yes'

fi # $set_api != yes
