#!/bin/sh
#------------------------------------------------------------------------------
# /usr/bin/pcengines-alix-cron.sh                                  __FLI4LVER__
#
# Creation:     08.02.2015 cspiess
# Last Update:  $Id$
#------------------------------------------------------------------------------

#reload lm90 driver
modprobe -r lm90
sleep 2
modprobe lm90
