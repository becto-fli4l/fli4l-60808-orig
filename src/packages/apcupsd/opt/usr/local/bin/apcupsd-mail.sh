#!/bin/sh
#
# This shell script if placed in /etc/apcupsd
# will be called by /etc/apcupsd/apccontrol when apcupsd
# detects that the battery should be replaced.
# We send an email message to root to notify him.
#
. /etc/apcupsd/mail.conf

HOSTNAME=`hostname`
MSG="$HOSTNAME $1"
(
   echo "$MSG"
   echo " "
   apcaccess status
) | mailsend -A "${APCUPSD_EVENTMAIL_ACCOUNT}" -t "${APCUPSD_EVENTMAIL_TO}" -s "${MSG}"

