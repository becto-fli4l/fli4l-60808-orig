#!/bin/sh
#------------------------------------------------------------------------------
# support.cgi - give the user some support-infos
#
# Creation:     24.02.2001  tg
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

tmpfile=/tmp/support.txt

head ()
{
    show_tab_header "$1" no
    echo "<pre>"
}

foot ()
{
    echo "</pre>"
    show_tab_footer
    echo "<br>"
}

cmd ()
{
    local action="$(echo "$1" | cleanup_for_eval)"
    if [ 0$# -gt 2 ]
    then
        command=$3
    else
        command=$2
    fi

    if [ "$FORM_action" = "$action" ]
    then
        tabs="$tabs'$1' 'no' "
    else
        tabs="$tabs'$1' '$myname?section=show&amp;action=`url_encode $action`' "
    fi
    echo "$action:$command:eval $2 | htmlspecialchars" >> $tmpfile
}

: ${FORM_section:=view}

case $FORM_section in
    view) SEC_ACTION='view' ;;
    *) SEC_ACTION='systeminfo' ;;
esac

# Security
check_rights "support" "$SEC_ACTION"

case $FORM_section in
show)
    : ${FORM_action:=$(echo "$_SUP_ps" | cleanup_for_eval)}
    > $tmpfile
    tabs=
    show_html_header "$_SUP_sinf"
    show_backlink
    echo "<br /><br /><div class=\"show\">"

    . /usr/local/bin/support.inc
    
    eval show_tab_header $tabs
    doit=`grep "^$FORM_action:" $tmpfile | cut -d: -f3`
    echo "<h2>`grep "^$FORM_action:" $tmpfile | cut -d: -f2`</h2><pre>"
    $doit
    echo "</pre></div>"
    show_tab_footer
    show_html_footer
;;
download)
    http_header download "ctype=text/plain; filename=supportinfos.txt"
    ( . /usr/local/bin/support.sh )
    sed "s/$/`echo -e \\\r`/" /tmp/support.txt
;;
*)
    show_html_header "$_SUP_inf"
    echo '<div class="supon">'
    show_tab_header "$_SUP_doc (online)" no
    case $lang in
    de)
        echo '<a href="http://www.fli4l.de" target="_blank">'$_SUP_web' fli4l</a><br>'
        echo '<a href="http://www.fli4l.de/hilfe/dokumentation/" target="_blank">'$_SUP_doc'</a><br>'
        echo '<a href="http://www.fli4l.de/hilfe/faq/" target="_blank">FAQs</a><br>'
        echo '<a href="http://www.fli4l.de/hilfe/howtos/" target="_blank">HOW-TOs</a><br>'
    ;;
    en)
        echo '<a href="http://www.fli4l.de/en/" target="_blank">'$_SUP_web' fli4l</a><br>'
        echo '<a href="http://www.fli4l.de/en/help/documentation/" target="_blank">'$_SUP_doc'</a><br>'
        echo '<a href="http://www.fli4l.de/en/help/faq/" target="_blank">FAQs</a><br>'
        echo '<a href="http://www.fli4l.de/en/help/howtos/" target="_blank">HOW-TOs</a><br>'
    ;;
    fr)
        echo '<a href="http://www.fli4l.de/fr/" target="_blank">'$_SUP_web' fli4l</a><br>'
        echo '<a href="http://www.fli4l.de/fr/aide/documentation/" target="_blank">'$_SUP_doc'</a><br>'
        echo '<a href="http://www.fli4l.de/fr/aide/faq/" target="_blank">FAQs</a><br>'
        echo '<a href="http://www.fli4l.de/fr/aide/howtos/" target="_blank">HOW-TOs</a><br>'
    ;;
    *)
        echo '<a href="http://www.fli4l.de" target="_blank">'$_SUP_web' fli4l</a> ('$_SUP_de')<br>'
        echo '<a href="http://www.fli4l.de/en/" target="_blank">'$_SUP_web' fli4l</a> ('$_SUP_en')<br>'
        echo '<a href="http://www.fli4l.de/fr/" target="_blank">'$_SUP_web' fli4l</a> ('$_SUP_fr')<br>'
        echo '<a href="http://www.fli4l.de/hilfe/dokumentation/" target="_blank">'$_SUP_doc'</a> ('$_SUP_de')<br>'
        echo '<a href="http://www.fli4l.de/en/help/documentation/" target="_blank">'$_SUP_doc'</a> ('$_SUP_en')<br>'
        echo '<a href="http://www.fli4l.de/fr/aide/documentation/" target="_blank">'$_SUP_doc'</a> ('$_SUP_fr')<br>'
        echo '<a href="http://www.fli4l.de/hilfe/faq/" target="_blank">FAQs</a> ('$_SUP_de')<br>'
        echo '<a href="http://www.fli4l.de/en/help/faq/" target="_blank">FAQs</a> ('$_SUP_en')<br>'
        echo '<a href="http://www.fli4l.de/fr/aide/faq/" target="_blank">FAQs</a> ('$_SUP_fr')<br>'
        echo '<a href="http://www.fli4l.de/hilfe/howtos/" target="_blank">HOW-TOs</a> ('$_SUP_de')<br>'
        echo '<a href="http://www.fli4l.de/en/help/howtos/" target="_blank">HOW-TOs</a> ('$_SUP_en')<br>'
        echo '<a href="http://www.fli4l.de/fr/aide/howtos/" target="_blank">HOW-TOs</a> ('$_SUP_fr')<br>'
    ;;
    esac
    show_tab_footer
    echo '</div>'
    echo '<div class="supoff">'
    show_tab_header "$_SUP_lsup (offline)" no
    echo '<a href="'$myname'?section=show">'$_SUP_shsinf'</a><br>'
    echo '<a href="'$myname'?section=download">'$_SUP_dlsinf'</a><br>'
    show_tab_footer
    echo '</div>'
    show_html_footer
;;
esac
