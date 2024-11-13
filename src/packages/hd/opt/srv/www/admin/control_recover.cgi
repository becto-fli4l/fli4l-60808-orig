#!/bin/sh
#----------------------------------------------------------------------------
# control_recover.cgi
#
# Creation:	2007-02-24 lanspezi
# Last Update:  $Id$
#----------------------------------------------------------------------------

format_output()
{
    htmlspecialchars | while read line
    do
        dat=`echo $line | cut -d "|" -f 1`
        event=`echo $line | cut -d "|" -f 2`

        echo "<tr>"
        echo "<td>$dat</td>"
        echo "<td>$event</td>"
        echo "</tr>"
    done
}

get_one_var()
{
    var=$1
    name=$2
    file=$3
    eval $var=`sed -n -e "s/^$name='\(.*\)'/\1/p" $file`
}

show_version_info ()
{
    get_one_var var_version FLI4L_VERSION   $2
    get_one_var var_date    FLI4L_BUILDDATE $2
    get_one_var var_time    FLI4L_BUILDTIME $2
    get_one_var var_kernel  KERNEL_VERSION $2
    get_one_var var_arch    ARCH $2
    
    var_typ="&nbsp;"
    if [ "$2" = "/etc/rc.cfg" ]
    then
        var_typ="not defined"
        if grep -q "fli4l_mode=normal" /proc/cmdline
        then
            var_typ="$_RECOVER_normal_version"
        fi
        if grep -q "fli4l_mode=recover" /proc/cmdline
        then
            var_typ="$_RECOVER_recover_version"
        fi
        echo "<tr class=\"odd\">"
    else
        echo "<tr>"
    fi
    if [ "x$var_arch" = "x" ]
    then
        var_arch="N.A."
    fi
    
    echo "<td><center>$1</center></td>"
    echo "<td><center>$var_version</center></td>"
    echo "<td><center>$var_kernel</center></td>"
    echo "<td><center>$var_date</center></td>"
    echo "<td><center>$var_time</center></td>"
    echo "<td><center>$var_arch</center></td>"
    echo "<td><center>$var_typ</center></td>"
    echo "</tr>"
}

show_versions ()
{
    show_tab_header "$_RECOVER_version_info" no
    cat <<EOF 
<table class="normtable">
<tr>
<th></th><th>$_RECOVER_version</th><th>Kernel</th><th>$_RECOVER_date</th><th>$_RECOVER_time</th><th>$_RECOVER_arch</th><th>Typ</th>
</tr>
EOF
    echo "<tr><td colspan=7></td></tr>"
    show_version_info "$_RECOVER_current_version" /etc/rc.cfg
    echo "<tr><td colspan=7></td></tr>"
    show_version_info "$_RECOVER_normal_version" /boot/rc.cfg
    if [ -f /boot/kernel2 ]
    then
        show_version_info "$_RECOVER_recover_version" /boot/rc2.cfg
        value=ask
    fi
    echo "</table>"
    show_tab_footer
}

. /srv/www/include/cgi-helper
: ${FORM_action:=view}
check_rights "recover" "$FORM_action"

case $FORM_action in
    mkrecoverask)
        show_html_header "$_RECOVER_title"
        show_tab_header "$_RECOVER_titledo" no
        echo '<br />'
        show_error "" "$_RECOVER_msg1ask"

        show_versions

        cat <<EOF
<br /><br />
$_RECOVER_msg2ask
<br />
<form action="$myname" method="GET">
<br />&nbsp;
<input id="recoverno" type="submit" value="$_RECOVER_no" name="action" class="actions">
&nbsp;
<input id="recoveryes" type="submit" value="$_RECOVER_yes" name="action" class="actions">
<br />
</form>
EOF
        show_tab_footer
        show_html_footer
        ;;
    mkrecover|$_RECOVER_yes)
        show_html_header "$_RECOVER_title" "refresh=5;url=control_recover.cgi"
        show_tab_header "$_RECOVER_titledo" no
        echo '<br />'
        echo "$_RECOVER_output"
        show_info "" "<pre>`/usr/sbin/mkrecover.sh -webgui`</pre>"
        echo '<br />'
        show_tab_footer
        show_html_footer
    ;;
    mkrestore)
        show_html_header "$_RECOVER_title" "refresh=5;url=control_recover.cgi"
        show_tab_header "$_RECOVER_titledorest" no
        echo '<br />'
        echo "$_RECOVER_output"
        show_info "" "<pre>`/usr/sbin/mkrecover.sh -webgui -restore`</pre>"
        echo '<br />'
        show_tab_footer
        show_html_footer    
    ;;
    view|$_RECOVER_no)
        show_html_header "$_RECOVER_title"

        show_tab_header "$_RECOVER_titleshow" no

        show_versions
        
        if grep -q "fli4l_mode=normal" /proc/cmdline
        then
            cat <<EOF
&nbsp;&nbsp;$_RECOVER_desc1 &nbsp;&nbsp;
<br />	
&nbsp;&nbsp;$_RECOVER_desc2 &nbsp;&nbsp;
<br />
<form action="$myname" method="GET">
<br />&nbsp;
<input id="recover" type="submit" value="$_RECOVER_action" class="actions">
<input id="action" type="hidden" value="mkrecover$value" name="action" class="actions">
<br /><br />
</form>
EOF
        fi
        
        if grep -q "fli4l_mode=recover" /proc/cmdline
        then
            cat <<EOF
&nbsp;&nbsp;$_RECOVER_restdesc1 &nbsp;&nbsp;
<br />
<form action="$myname" method="GET">
<br />&nbsp;
<input id="recover" type="submit" value="$_RECOVER_actionrest" class="actions">
<input id="action" type="hidden" value="mkrestore" name="action" class="actions">
<br /><br />
</form>
EOF
        fi
        show_tab_footer

## new section for switch version for next boot - boot with syslinux
        if [ -f /boot/ldlinux.sys ]
        then
            if grep -q "DEFAULT n" /boot/syslinux.cfg
            then
                next_boot=$_RECOVER_normal_version
            fi
            if grep -q "DEFAULT r" /boot/syslinux.cfg
            then
                next_boot=$_RECOVER_recover_version
            fi
            echo "<br />"
            show_tab_header "$_RECOVER_bootnext" no
            cat << EOF
<br />
 &nbsp;&nbsp;&nbsp;&nbsp;$_RECOVER_bootnext : <b>$next_boot &nbsp;&nbsp;&nbsp;&nbsp;</b>
<br />
<form action="$myname" method="GET">
<br />&nbsp;
<input id="switch" type="submit" value="$_RECOVER_switch" class="actions">
<input id="action" type="hidden" value="mkrecoverswitch" name="action" class="actions">
&nbsp;&nbsp;&nbsp;&nbsp;<br /><br />
</form>
EOF
            show_tab_footer
        fi

## new section for switch version for next boot - boot with pygrub or grub
        if [ -f /boot/grub/menu.lst ]
        then
            if grep -q "default 0" /boot/grub/menu.lst
            then
                next_boot=`grep title /boot/grub/menu.lst | head -n 1 | cut -d " " -f 2-`
            fi
            if grep -q "default 1" /boot/grub/menu.lst
            then
                next_boot=`grep title /boot/grub/menu.lst | head -n 2 | tail -n1 | cut -d " " -f 2-`
            fi
            echo "<br />"
            show_tab_header "$_RECOVER_bootnext" no
            cat << EOF
<br />
 &nbsp;&nbsp;&nbsp;&nbsp;$_RECOVER_bootnext : <b>$next_boot &nbsp;&nbsp;&nbsp;&nbsp;</b>
<br />
<form action="$myname" method="GET">
<br />&nbsp;
<input id="switch" type="submit" value="$_RECOVER_switch" class="actions">
<input id="action" type="hidden" value="mkrecoverswitch" name="action" class="actions">
&nbsp;&nbsp;&nbsp;&nbsp;<br /><br />
</form>
EOF
            show_tab_footer
        fi

        ## LOG of mkrecover.sh
        if [ -f /var/lib/persistent/hd/recover.log ]
        then
            echo "<br />"
            show_tab_header "${_RECOVER_title}-LOG (30 $_MN_rows)" no
            echo '<table class="normtable">'
            echo "<tr>"
            echo "<th>$_RECOVERLOG_date</th>"
            echo "<th>$_RECOVERLOG_event</th>"
            echo "</tr>"
            cat /var/lib/persistent/hd/recover.log | do_tail 30 | sed '1!G;h;$!d' | format_output
            echo "</table>"
            show_tab_footer
        fi
        
        show_html_footer
        ;;
    mkrecoverswitch)
        show_html_header "$_RECOVER_title" "refresh=5;url=control_recover.cgi"
        show_info "$_RECOVER_title" "$_RECOVER_switch"
        echo "<br /><br />"
        err=0
        # check for /boot
        if ! grep -q /boot /proc/mounts
        then
            show_error "MOUNT-ERROR" "/boot is not mounted!<br /><br /><pre>`mount`</pre>"
            err=1
        fi

        # check mount state of boot-partition
        if ! grep /boot /proc/mounts | grep -q rw 
        then
            show_error "MOUNT-ERROR" "/boot is readonly!<br /><br /><pre>`grep /boot /proc/mounts`</pre>"
            err=1
        fi
        if [ $err != "1" ]
        then
            if [ -f /boot/ldlinux.sys ]
            then
                wert=`sed -n 's/^DEFAULT \([rn]\).*/\1/p' /boot/syslinux.cfg`
                case $wert in
                    n) sed -i 's/^DEFAULT [rn].*/DEFAULT r/' /boot/syslinux.cfg;;
                    *) sed -i 's/^DEFAULT [rn].*/DEFAULT n/' /boot/syslinux.cfg;;
                esac
            fi
            
            #todo - code to switch version for pygrub/grub based systems
            if [ -f /boot/grub/menu.lst ]
            then
                wert=`sed -n 's/^default \([01]\).*/\1/p' /boot/grub/menu.lst`
                case $wert in
                    0) sed -i 's/^default [01].*/default 1/' /boot/grub/menu.lst;;
                    1) sed -i 's/^default [01].*/default 0/' /boot/grub/menu.lst;;
                esac
            fi
        fi

        show_html_footer
        ;;
esac
