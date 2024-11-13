#!/bin/sh

# get main helper functions
. /srv/www/include/cgi-helper

check_rights 'support' 'systeminfo'

bootlog=/var/tmp/boot.log
myname=`basename $0`

format_bootlog()
{
    show_html_header "$_BOOTLOG_title"

    show_tab_header "$_BOOTLOG_show" no "$_BOOTLOG_download" "$myname?action=download" 
    cat <<EOF 
<table class="normtable">
<tr>
  <th>$_MN_date</th><th>$_MN_time</th><th></th>
</tr>
EOF

    cached=/var/tmp/boot.table
    if [ ! -f $cached -o $bootlog -nt $cached ]; then
	sed -f log_bootlog.sed $bootlog > $cached
    fi
    cat $cached
    echo "</table>"
    show_tab_footer
    show_html_footer
}

dump_bootlog()
{
    http_header download "ctype=text/plain;filename=boot.log"
    cat $bootlog
}

if [ -f $bootlog ]
then
    case $FORM_action in
        download) dump_bootlog ;;
	*)        format_bootlog ;;
    esac
else
    show_html_header "$_MN_err"
    show_error "$_MN_err" "$_BOOTLOG_enoexist"
    show_html_footer
fi
