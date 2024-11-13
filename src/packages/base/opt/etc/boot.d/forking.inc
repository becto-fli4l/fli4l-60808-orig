#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/forking.inc
# Functions for adjusting state after forking to the background.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$forking_api" != yes ]
then

# List of registered fork handlers.
base_helper_fork_handlers=

# Registers a fork handler. A fork handler is called by a background child
# process that was created while forking off a shell script. Such a handler is
# responsible for maintaining proper state. For example, all variables keep
# their values after a fork, but it is unwise to inherit a list of locks the
# parent script might have acquired as the child process runs in parallel to
# its parent due to its background nature.
#
# Input:
#   $1 = Name of handler function. No parameters are passed to the handler.
fork_add_handler()
{
    local handler=$1
    base_helper_fork_handlers="$base_helper_fork_handlers $handler"
}

# Calls all registered fork handlers. This function needs to be called by the
# child process immediately after forking.
fork_call_handlers()
{
    local handler
    for handler in $base_helper_fork_handlers
    do
        $handler
    done
}

forking_api='yes'

fi # $forking_api != yes
