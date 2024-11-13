#!/bin/sh
#------------------------------------------------------------------------------
# catlog.cgi - show squid logs on the web.
#
# Creation:        22.02.2001  mb
# Last Update:  $Id$
#
#------------------------------------------------------------------------------


xname=`basename $0`

echo "Content-Type: text/html; charset=utf-8"
echo "Cache-control: no-store" # HTTP/1.1 (or no-cache?)
echo "Pragma: no-cache"        # HTTP/1.0
echo "Expires: `date -Ru`"     # Expires now!
echo


logdir=`cat /etc/squid/workdir`/logs


datei=$1

if [ "$datei" = "" ]
then
	datei="access.log"
else
        if [ "$datei" != "access.log" -a "$datei" != "cache.log" ]
	then
		datei="access.log"
	fi
fi

toshow=$logdir/$datei
echo '<html><head>'
echo '<title>FLI4L-Webserver - squid logs .. ('$datei')</title>'
echo '</head>'
echo '<body bgcolor=black text=white vlink=red link=red alink=yellow><font size=3>'
echo '<center><a href="'$xname'?access.log">access.log</a> | '
echo '<a href="'$xname'?cache.log">cache.log</a></center>'
echo '<pre>squid logfile: '$toshow':'
echo '===================================================================='
cat $toshow
echo '</pre></font></body></html>'

