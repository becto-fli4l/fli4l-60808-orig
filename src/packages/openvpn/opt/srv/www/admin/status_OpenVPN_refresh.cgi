#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/admin/OpenVPN_refresh.cgi
# Creation:     15.05.2005 HH
# Last Update:  $Id$
#------------------------------------------------------------------------------
# Debug info to debug2.log
#exec 2>/srv/www/debug2.log
#exec 2>/dev/console
#set -x

# Parse variables...
eval "`proccgi $*`"
: ${FORM_refresh:=4}             # refresh every 4 seconds
# Get libs
gui_started="true"
. /srv/www/include/status_OpenVPN_functions.inc

# Get language
if [ -f ../lang/status_OpenVPN.$FORM_lang ]
then
    . ../lang/status_OpenVPN.$FORM_lang
fi

# Get all ovpn names
cd /etc/openvpn/
ls *.conf | cut -d. -f 1 > /tmp/ovpnnames.$$
ref=
noconn="true"
while read name;
do
    state=`state_ovpn`
    case $state in
        STOPPED)
          symbol="stopped"
          ;;
        CONNECTED)
          symbol="running"
          noconn="false"
          ;;
        *)
          symbol="noconn"
          noconn="false"
          ;;
    esac
    eval alt='$_VPN_'$state
    ref="$ref parent.document.getElementById(\"state-$name\").src=\"../img/$symbol.gif\";\
              parent.document.getElementById(\"state-$name\").title=\"$alt\";\
              parent.document.getElementById(\"state-$name\").alt=\"$alt\";"
    case $FORM_showtext in
        yes)
            ref="$ref parent.document.getElementById(\"statetext-$name\").value=\"$alt\";"
        ;;
    esac
done < /tmp/ovpnnames.$$
rm -f /tmp/ovpnnames.$$
if [ "$noconn" = "true" ] # reduce refresh rate, if ther is no active connection
then
    FORM_refresh=30
else
    FORM_refresh=4
fi
echo "Content-Type: text/html; charset=utf-8"
echo "Cache-control: no-store" # HTTP/1.1 (or no-cache?)
echo "Pragma: no-cache"        # HTTP/1.0
echo "Expires: `date -Ru`"     # Expires now!
echo '
<!doctype html public "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>'
echo "<meta http-equiv=\"Refresh\" content=\"$FORM_refresh; url=status_OpenVPN_refresh.cgi?lang=$FORM_lang&amp;refresh=$FORM_refresh&amp;showtext=$FORM_showtext\">"
echo '
<title>OpenVPN-refresh</title>
</head>
<body>
<script language="JavaScript" type="text/javascript">
parent.document.getElementById("refresh").style.display="block";'
echo "
function hideit()
{
    parent.document.getElementById(\"refresh\").style.display=\"none\";
} "
echo "$ref"
echo 'setTimeout("hideit()",1000);'
echo '
</script>
</body>
</html> '
