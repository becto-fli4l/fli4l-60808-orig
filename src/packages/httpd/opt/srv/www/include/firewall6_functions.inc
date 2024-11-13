#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/include/firewall6_functions.inc
# Last Update:  $Id$
#------------------------------------------------------------------------------
[ "$cgi_helper" ] || exit 1 # must not be called standalone
case $FORM_fwdebug in
yes)
    FWRULES6_DO_DEBUG=yes # set firewall debugging
    reload ()            # don't reload site
    {
        return
    }
    ;;
esac

# Initialize main functions
. /srv/www/include/firewall_functions.inc
. /etc/rc.d/fwrules-helper.ipv6
SCRIPT=portfw6.cgi

# helper functions for portforwarding
get_active_matches6 ()
{
    matches=prot
    match_nr=1
    for i in tmpl `set | sed -n -e "s/^\([a-z]\+\)_p='yes'.*/\1/p"`; do
        case $i in
            prot) ;;
            *)
                if grep -q "\(^\|[[:space:]]\)$i:" $ip6tables_rules/nat/PORTFW 2>/dev/null
                then
                    matches="$matches $i"
                    match_nr=`expr $match_nr + 1`
                fi
            ;;
        esac
    done
}
get_all_matches6 ()
{
    matches="tmpl `set | sed -n -e "s/^\([a-z]\+\)_p='yes'.*/\1/p"`"
    match_nr=`set | grep -c "_p='yes'"`
    match_nr=`expr $match_nr + 1`
}
pfw6_fixup_ip ()
{
    name=$1
    eval ip=\$$name
    case $ip in
        any | ::/0) 
            eval $name= 
            ;;
    esac
}

matches=
match_nr=0
default_prot='tcp'

#------- Firewall GUI Functions -------------------------------------------------------
init_vars6 ()
{ #clear some variables
    prot=
    if_in=
    if_out=
    sport=
    dport=
    action=
    orig_tmpl_name=
    rule_error=yes
}

reset_rule_error6 ()
{
    rule_error=
}

check_rule6 ()
{
    init_vars6
    do_rule6 nat PREROUTING A "$pf_rule" '' reset_rule_error6 > /dev/null 2>&1
    get_params6
}

create_rule6 ()
{
    src=$1
    sport=$2
    dst=$3
    dport=$4
    rip=$5
    rport=$6
    [ "$src" ] || src=any
    [ "$sport" ] && sport=":$sport"
    [ "$dport" ] && dport=":$dport"
    [ "$rport" ] && rport=":$rport"
    pf_rule="$pf_rule [$src]$sport [$dst]$dport DNAT:[$rip]$rport"
}

get_params6 ()
{
    # restrictions:
    prot=$proto
    if_in=`echo $if_in_negopt$if_in | sed "s/ //g"`
    if_out=`echo $if_out_negopt$if_out | sed "s/ //g"`
    # still missing match-opts: state, mac, limit, length, ...

    src=$(echo "$src_unmapped" | sed 's/]:.*/]/;s/^\[//;s/]//')
    sport=$(echo "$src_unmapped" | sed -n 's/^.*]:\(.*\)/\1/p')

    dst=$(echo "$dst_unmapped" | sed 's/]:.*/]/;s/^\[//;s/]//')
    dport=$(echo "$dst_unmapped" | sed -n 's/^.*]:\(.*\)/\1/p')

    mangle_ip_params6 `echo $action | sed -e 's/DNAT://'` keepslash nomap
    rip=$ip
    rport=$(echo $port | sed 's/:/-/')
}

show_rule_error6 ()
{
    show_html_header "$_PF6_portforwarding"
    show_backlink
    echo "<br /><br />"
    show_error "$_MN_err" "<pre>`cat /tmp/pf6cgi.$$ | htmlspecialchars`</pre>"
    show_html_footer
}
