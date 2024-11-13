#----------------------------------------------------------------------------
# /www/srv/admin/config-helper.inc
#
# Creation:     2006-09-22 hh
# Last Update:  $Id: cgi-helper 11868 2006-08-21 23:25:15Z hh $
#----------------------------------------------------------------------------
[ "$cgi_helper" ] || exit 1 # must not be called standalone

. /var/run/guiconfig.conf

read_var ()
{
    if grep -q $1= $CONF_FILE
    then
        eval `grep $1= $CONF_FILE`
    else
        eval `grep $1= /etc/rc.cfg`
    fi
}

write_var ()
{
    grep -v $1= $CONF_FILE /tmp/$CONF_FILE.$$
    mv /tmp/$CONF_FILE.$$ $CONF_FILE
    eval echo $1='\"$'$1'\"' >> $CONF_FILE
}
