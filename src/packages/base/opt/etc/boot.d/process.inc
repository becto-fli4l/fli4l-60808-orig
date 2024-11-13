#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/process.inc
# Functions for process inspection and manipulation.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$process_api" != yes ]
then

# Maps a PID to the process name.
#
# $1 = PID
# Output: the process name or "<unknown>" if it could not be determined
map_pid_to_process()
{
    local proc=
    read proc < /proc/$1/comm
    [ -n "$proc" ] && echo "$proc" || echo "<unknown>"
}

process_api='yes'

fi # $process_api != yes
