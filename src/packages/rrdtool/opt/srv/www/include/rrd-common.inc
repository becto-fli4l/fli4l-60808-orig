# RRD Konfiguration fuer Farben usw. holen
. /etc/rrd/settings.inc

if [ ! -d /srv/www/img/rrd ]
then
    mkdir -p /srv/www/img/rrd
else
    find /srv/www/img/rrd/ -mmin +1 -exec rm {} \;
fi

find /var/run/ -name rrd_currentgraphtime -mmin +5 -exec rm {} \;

if [ -f /var/run/rrd_currentgraphtime ]
then
    rrd_default_graphtime=`cat /var/run/rrd_currentgraphtime`
fi

if [ -d ${rrd_dbpath}/ntpd ]
then
    find ${rrd_dbpath}/ntpd/ -mtime +2 -exec rm {} \;
fi

rrd_open_tab_list ()
{
    rrd_source=$1
    local var_name=`echo $1 | sed -e 's/[.-]/_/g'`

    local subtab=
    eval local current_tab='$FORM_rrd_graphtime_$var_name'

    local rrd_other_graphtime_values
    local rrd_all_graphtime_values=`set | grep "^FORM_rrd_graphtime_$var_name"`
    for idx in $rrd_all_graphtime_values
    do
        local var=`echo $idx|sed -e 's/^FORM_rrd_graphtime_\([^=]\+\).*/\1/'`
        local value=`echo $idx|sed -e "s/^FORM_rrd_graphtime_[^=]\+='\([^']\+\)'/\1/"`
        if [ "$rrd_source" != "$var" ]
        then
            rrd_other_graphtime_values="rrd_graphtime_$var=$value&amp;$rrd_other_graphtime_values"
        fi
    done

    for idx in $rrd_graphtime_list
    do
        if [ "$value" = "$idx" ]
        then
            subtab="$subtab $idx no"
    else
            subtab="$subtab $idx $myname?action=$FORM_action&amp;rrd_graphtime_$var_name=$idx"
        fi
    done
    show_tab_header $subtab
}

rrd_close_tab_list ()
{
    show_tab_footer
}

rrd_render_graph ()
{
    local rrd_typ=$1
    local rrd_source=$2
    local var_name=`echo $2 | sed -e 's/[.-]/_/g'`
    local render_list=

    if [ "$rrd_source" = "" ]
    then
        eval render_list='$FORM_rrd_graphtime_'$rrd_typ
    else
        eval render_list='$FORM_rrd_graphtime_'$var_name
    fi
    
    echo "$render_list" > /var/run/rrd_currentgraphtime
    
    if [ "$render_list" = "all" ]
    then
        render_list=`echo $rrd_graphtime_list|sed -e 's/ all$//'`
    fi
    for graphtime in $render_list
    do
        ${rrd_typ}_graph $graphtime $rrd_source
        echo "<br>"
    done
}
