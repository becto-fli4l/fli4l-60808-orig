#! /bin/sh
#------------------------------------------------------------------------------
# pf.cgi - show the firewall config
#
# Creation:     05.08.2005  jw5
# Last Update:  $Id$
#
#------------------------------------------------------------------------------

# Functions
dump_rules ()
{
    rules="$1"
    f="echo \"Table filter\"; iptables --line-numbers -nvL"
    n="echo \"Table nat\"; iptables --line-numbers -t nat -nvL"
    r="echo \"Table raw\"; iptables --line-numbers -t raw -nvL"
    case $rules in
        INPUT | FORWARD | OUTPUT)
            eval $f $1-head
            eval $f $1-middle
            eval $f $1-tail
            ;;
        PORTFW)
            eval $n PORTFW
            ;;
        all)
            eval $f
            eval $n
            eval $r
            ;;
        *[,:]*)
            for p in `echo $rules | sed -e 's/,/ /g'`
            do
                set `echo $p | sed -e 's/:/ /g'`
                case $# in
                    1)
                        eval $f $1
                        ;;
                    2)
                        echo "Table $1"
                        iptables --line-numbers -t $1 -nvL $2
                        ;;
                esac
            done
            ;;
        *)
            eval $f INPUT-middle
            eval $f FORWARD-middle
            eval $f OUTPUT-middle
            eval $n POSTROUTING-middle
            ;;
    esac
}

format_output ()
{
    while read line
    do
        case "$line" in
            '') ;;
            Table*)
                table=$(echo "$line" | sed 's/^Table[[:space:]]*//')
                ;;
            Chain*)
                 case $head in
                     yes)
                         echo '</table>'
                         show_tab_footer
                     ;;
                 esac
                 show_tab_header "$line [$table]" no
                 echo '    <table class="normtable">'
                head=yes
                ;;
            num*)
                echo "$line" | sed -e 's/^/<tr><th>/;s/[[:space:]]\+/<\/th><th>/g'
                echo "</th><th>additional restrictions</th>" #<th>config source</th></tr>"
                echo "<th>comment</th></tr>"
                ;;
            *)
                set -f
                set -- $line
                def=`echo "$1 $2 $3 $4 $5 $6 $7 $8 $9 ${10}" | sed -e 's/^/<td>/;s/[[:space:]]\+/<\/td><td>/g;s/$/<\/td>/;s#<td>\(all\|[*]\|--\|0\.\0\.\0\.0/0\)</td>#<td>\&nbsp;</td>#g'`
                 target=$4
                shift 10
                restr=`echo "<td>$*</td>" | sed -e 's,/[*].*[*]/,,;s#<td></td>#<td>\&nbsp;</td>#g'`
                if echo "$*" | grep -q '/\*.*\*/'; then
                    comment="`echo "$*" | sed -e 's#.*/\*\(.*\)\*/[^*/]*#\1#'`"
                else
                    comment=""
                fi
                echo "<tr title=\"$comment\" class=\"$target\">$def $restr <td>$comment</td></tr>"
                set +f
                ;;
        esac
    done
    echo '</table>'
    show_tab_footer
}

# get main helper functions
. /srv/www/include/cgi-helper

# Security
check_rights 'support' 'systeminfo'

# some variable hacks for proper tab-selection
_overview=$_MP_ov; _all=$_MN_all
_INPUT="Input"; _FORWARD="Forward"; _OUTPUT="Output"; _PORTFW="Port-Forward"

: ${FORM_action:=overview}
myname=`basename $0`
FORM_action="$(echo "$FORM_action" | cleanup_for_eval)"

show_html_header "$_MP_pf-`eval echo '$_'$FORM_action`" "cssfile=pf"

show_tab_header "$_overview" "$myname" \
                "$_INPUT" "$myname?action=INPUT" \
                "$_FORWARD" "$myname?action=FORWARD" \
                "$_OUTPUT" "$myname?action=OUTPUT" \
                "$_PORTFW" "$myname?action=PORTFW" \
                "$_all" "$myname?action=all"

case $FORM_format in
    raw)
        echo "<pre>"
        dump_rules $FORM_action
        echo "</pre>"
        ;;
    *)   dump_rules $FORM_action | format_output ;;
esac

show_tab_footer
show_html_footer
