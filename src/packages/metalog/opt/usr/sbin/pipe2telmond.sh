#!/bin/sh
# /usr/sbin/pipe2telmond
# For metalog -- emulate a named pipe
# generate a syslogd like output

[ -p /var/run/kernel-info ] && echo "$1 local $2: $3" >> /var/run/kernel-info
