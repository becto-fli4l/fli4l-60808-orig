#!/bin/sh
#------------------------------------------------------------------------------
# dyndns.cgi - display the answers from the dyndns services
# part of Package DYNDNS __FLI4LVER__, see documentation for licence
#
# (c) copyright 2001-2002 Thomas Mueller
# (c) copyright 2002-2003 Tobias Gruetzmacher
#
# Creation:    27.08.2001 Thomas Mueller
# Last update: $Id$
#------------------------------------------------------------------------------

# set_debug=yes

sanitize_log ()
{
    local sed_lang=/srv/www/lang/status_dyndns.sed.$lang
    if [ -f $sed_lang ]
    then
        sed -f /srv/www/admin/status_dyndns.sed $1 | sed -f $sed_lang
    else
        sed -f /srv/www/admin/status_dyndns.sed $1
    fi
}

show_history ()
{
    host=$1
    show_html_header "DynDNS-Status $host"

    show_tab_header "$_DYNDNS_tab_header" no
    sanitize_log $dyn_histdir/$host
}

show_status ()
{
    hist=$1

    show_html_header "DynDNS-Status"
    show_tab_header "$_DYNDNS_tab_header" no
    cat <<EOF
<table class="normtable">
EOF

    for host in $dyn_hostdir/*.conf
    do
	host=`basename $host|sed -e 's/\.conf$//'`
        dyn_runfile="$dyn_rundir/$host"
        dyn_disabled="${dyn_runfile}.disabled"
        dyn_logfile="$dyn_logdir/$host"
        dyn_histfile="$dyn_histdir/$host"
	arg=

        if [ -n "$hist" ]
	then
            [ "$hist" != "$host" ] && continue
            dyn_logfile="$dyn_histfile"
        fi

        if [ -f $dyn_runfile ]
	then
            . $dyn_runfile
            last="`date -d $registered_date -D '%s' '+%d.%m.%Y %H:%M'`"
            state="Update not needed"
        else
            last=unknown
            state=unknown
        fi

        if [ -f $dyn_disabled ]
	then
            action=enable
            text="$_DYNDNS_action_enable"
            state="Update disabled"
        else
            if [ -f $dyn_runfile.pid ]
	    then
                action=cancel
                text="$_DYNDNS_action_cancel"
                state="Update running"
            else
                action=update
                text="$_DYNDNS_action_update"
		arg=
                img="<img src=\"/img/execute.png\" width=\"16\" height=\"16\" title=\"$text\" alt=\"$text\"/>"
            fi
        fi

        #
        # header
        #
        cat <<EOF
<tr>
  <th colspan="2">$host, Current state: $state, Last succesful update: $last</th>
</tr>
<tr>
  <td valign="top" align="left"><!-- left column start -->
      <ul>
        <li>
          <a href="status_dyndns.cgi?action=$action&host=$host&arg=$arg">
            $img
            $text
          </a>
        </li>
EOF
	# if the current action is the update action, add a button to force a dyndns update
	# regardless of current state
	if [ "$action" = "update" ]
	then
            text="$_DYNDNS_action_update_force"
	    arg="-f"
            cat <<EOF
        <li>
          <a href="status_dyndns.cgi?action=$action&host=$host&arg=$arg">
            $img
            $text
          </a>
        </li>
EOF
        fi

        echo -e "        <li>$_DYNDNS_methods\n          <ul>"
        . /etc/dyndns/provider.d/$provider
        for method in $provider_update_method
	do
            if [ -f $dyn_disabled.$method ]
	    then
                echo "          <li><a href=\"status_dyndns.cgi?action=enable_method&host=$host&method=$method\" title=\"$_DYNDNS_action_enable_method $method\">$method</a></li>"
            else
                echo "          <li>$method</li>"
            fi
        done
        echo -e '</ul></li>\n'

        echo -e '<li>\n\n'
        if [ -n "$hist" ]
	then
            echo -e "        $_DYNDNS_click_begin <a href=$_rurl>$_DYNDNS_done_url</a>$_DYNDNS_click_end"
	else
	    echo -e "        <a href=\"status_dyndns.cgi?action=history&host=$host\">$_DYNDNS_update_history</a>"
	fi

        echo -e '</li>\n  </td><!-- left column end -->'
        

        #
        # right column
        #
        echo '  <td><!-- right column start -->'
        [ -f $dyn_logfile ] && sanitize_log $dyn_logfile
        echo '  </td><!-- right column end -->'

        #
        # close row
        #
        echo '</tr>'
    done
    echo "</table>"
    show_tab_footer
}

update_host ()
{
    local host=$1
    local arg=$2
    local pid=
    local tmp=/tmp/dyncgi.$$
    local res=

    if [ ! -f $dyn_hostdir/$host.conf ]
    then
        show_error "error" "Missing config for host '$host' in config directory!"
        return 1
    fi

    dyndns-update $deb_flag $arg $host > $tmp 2>&1 &

    pid=$!
    for i in 1 2 3 4 5
    do
        kill -0 $pid 2> /dev/null || break
        sleep 1
    done
    if [ -s $tmp ]
    then
        if grep "Update successful" $tmp
	then
            res=0
        else
            cat  $tmp
            res=1
        fi
    else
        msg="$_DYNDNS_update_triggered"
        res=0
    fi
    rm -f $tmp
    return $res
}

dyndns_debug () {
    val=$1
    if [ "$val" ]
    then
        file=$dyn_rundir/inv_$val
        if [ -f $file ]
        then
            rm -f $file
        else
            > $file
        fi
    fi
    show_html_header "DynDNS - Debug Provider"
    echo "<ul>"
    for val in user pass host IP
    do
        file=$dyn_rundir/inv_$val
        if [ -f $file ]
        then
            txt="use correct $val"
        else
            txt="use wrong $val"
        fi
        echo "  <li><a href=\"status_dyndns.cgi?action=debug_provider&host=$val\">$txt</a>"
    done
    echo '<li>dyndns-update debugging<ul>'
    for i in debug verb_debug
    do
        name=$dyn_rundir/inv_$i
        if [ -f $name ]
        then
            txt=disable
        else
            txt=enable
        fi
        case $i in 
            verb_debug) opt=verbose ;;
        esac
        echo "  <li><a href=\"status_dyndns.cgi?action=debug_provider&host=$i\">$txt $opt debugging of dyndns-update</a>"
    done
    echo "</ul><li>Traces<ul>"
    cd /var/tmp
    for i in `ls -t`
    do
        echo "<li><a href=\"status_dyndns.cgi?action=debug_trace&host=$i\">$i</a></li>"
    done
    echo "</ul></ul>"
}

debug_trace () 
{
    show_html_header "DynDNS - Debug Trace $1"
    {
        echo "<pre>"
        cat /var/tmp/$1
        echo "</pre>"
    } | show_error "$1" 
}

# get main helper functions
. /srv/www/include/cgi-helper
: ${FORM_action:=status}

# Security
check_rights "dyndns" "view"

dyn_hostdir="/etc/dyndns/host.d"
dyn_logdir="/var/log/dyndns"
dyn_histdir="${dyn_logdir}/history"
dyn_rundir="/var/run/dyndns"

_rurl=$myname # reload-url
_rshort=5 # short reload (5)
[ "$set_debug" = yes ] && _rshort=1800 # use a longer reload for the debug case

action=$FORM_action
host=$FORM_host
method=$FORM_method
arg=$FORM_arg
msg=

if [ $action = status -o $action = history ]
then
    show_status $host
elif [ $action = debug_provider ]
then
    dyndns_debug $host
elif [ $action = debug_trace ]
then
    debug_trace $host
else
    if [ -f $dyn_rundir/inv_verb_debug ]
    then
        deb_flag=-vx
    else
        [ -f $dyn_rundir/inv_debug ] && deb_flag=-x
    fi
    case $action in
        update) 
            update_host $host $arg ;;
        enable_method)
            dyndns-update $deb_flag enable_method $host $method ;;
        enable) 
            dyndns-update $deb_flag enable $host ;;
        cancel)
            dyndns-update $deb_flag cancel $host ;;
        *) 
            echo "$_DYNDNS_unknown_action_msg: '$action'"
            action=unknown_action
            false
        ;;
    esac > /tmp/status_dyndns.cgi.$$ 2>&1
    
    res=$?

    eval header="\"\$_DYNDNS_${action}_header $host\""
    if [ $res -eq 0 ]
    then
        if true
        then 
            show_status
        else 
            show_html_header "$header" "refresh=$_rshort;url=$_rurl"
            [ "$msg" ] || eval msg="\"\$_DYNDNS_${action}_done_info\""
            {
		if [ -s  /tmp/status_dyndns.cgi.$$ ]
		then
                    echo '<pre>'
                    cat /tmp/status_dyndns.cgi.$$
                    echo '</pre>'
		fi
		echo "$_DYNDNS_done_message<a href=$_rurl>$_DYNDNS_done_url</a>."
            } | show_info "$msg" 
        fi
    else
        show_html_header "$header" # "refresh=$_rshort;url=$_rurl"
        {
            echo '<pre>'
            cat /tmp/status_dyndns.cgi.$$
            echo -e "</pre>\n<b>$_DYNDNS_click_begin <a href=$_rurl>$_DYNDNS_done_url</a>$_DYNDNS_click_end</b>"
        } | show_error "Error" 
    fi
fi

#
# one closing html footer for all
#
show_html_footer
