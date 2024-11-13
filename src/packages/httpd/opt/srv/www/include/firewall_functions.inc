#!/bin/sh
#------------------------------------------------------------------------------
#                                                                  __FLI4LVER__
# /srv/www/include/firewall_functions.inc
# Creation:     15.05.2005 HH
# Last Update:  $Id$
#------------------------------------------------------------------------------
[ "$cgi_helper" ] || exit 1 # must not be called standalone
case $FORM_fwdebug in
yes)
    FWRULES_DO_DEBUG=yes # set firewall debugging
    reload ()            # don't reload site
    {
        return
    }
    ;;
esac

# Initialize main functions
. /etc/boot.d/networking.inc
. /etc/rc.d/fwrules-helper
SCRIPT=portfw.cgi

# helper functions for portforwarding
get_active_matches ()
{
    matches=prot
    match_nr=1
    for i in tmpl `set | sed -n -e "s/^\([a-z]\+\)_p='yes'.*/\1/p"`; do
        case $i in
            prot) ;;
            *)
                if grep -q "\(^\|[[:space:]]\)$i:" $iptables_rules/nat/PORTFW 2>/dev/null
                then
                    matches="$matches $i"
                    match_nr=`expr $match_nr + 1`
                fi
                ;;
        esac
    done
}
get_all_matches ()
{
    matches="tmpl `set | sed -n -e "s/^\([a-z]\+\)_p='yes'.*/\1/p"`"
    match_nr=`set | grep -c "_p='yes'"`
    match_nr=`expr $match_nr + 1`
}
pfw_fixup_ip ()
{
    name=$1
    eval ip=\$$name
    case $ip in
        any | 0.0.0.0/0) 
            eval $name= 
            ;;
    esac
}

matches=
match_nr=0
default_prot='tcp'

#------- Firewall Rule Parser -------------------------------------------------

# $1 = comment of a rule
# output:
#   $table, $chain, $index
parse_comment()
{
    comment="$(echo "$1" | sed -n 's/^[[:space:]]*\(PF6\?_\([A-Z]\+_\)\+[0-9]\+\(_[A-Z]\+_[0-9]\+\)\?: \)\?\(.*\)/\4/p')"
    dynrule="$(echo "$comment" | sed -n 's/^.*\( (PLACEHOLDER:\([0-9]\+\))\)$/\2/p')"
    [ -n "$dynrule" ] && comment="$(echo "$comment" | sed 's/^\(.*\)\( (PLACEHOLDER:\([0-9]\+\))\)$/\1/')"

    set -- $(echo "$1" | sed -n 's/^[[:space:]]*\(PF6\?\)_\(\([A-Z]\+_\)\+\)\([0-9]\+\)\(_[A-Z]\+_\([0-9]\+\)\)\?[:=].*/\1 \2 \4 \6/p')
    prefix=$1
    chain=${2%_}
    index=$3
    subindex=$4
    case "$chain" in
    OUTPUT_CT|PREROUTING_CT)
        table=raw
        ;;
    INPUT|FORWARD)
        table=filter
        ;;
    USR_CHAIN)
        table=filter
        var=${prefix}_USR_CHAIN_${index}_NAME
        eval $(grep "^$var=" /etc/rc.cfg)
        eval chain=\$$var
        index=$subindex
        ;;
    POSTROUTING)
        table=nat
        ;;
    PREROUTING)
        table=nat
        chain=PORTFW
        ;;
    *)
        table=
        ;;
    esac
}

handle_iptables_params()
{
    array="$1"
    eval $array=
    shift

    count=0
    param=
    values=
    while [ -n "$1" ]
    do
        case "$1" in
        --*)
            if [ -n "$param" ]
            then
                eval params=\$\{$array\}
                eval $array=\"$params $param\"
                eval ${array}_${param}=\"\$values\"
                values=
            fi
            param="$(echo ${1#--} | sed 's/-/_/g')"
            ;;
        -*)
            break
            ;;
        *)
            values="$values $1"
            ;;
        esac
        shift
        count=$((count+1))
    done

    if [ -n "$param" ]
    then
        eval params=\$\{$array\}
        eval $array=\"$params $param\"
        eval ${array}_${param}=\"\$values\"
    fi
}

do_parse_iptables_rule()
{
    chain=
    in=
    out=
    prot_neg=
    prot=
    src_neg=
    src=any
    sport=
    dst_neg=
    dst=any
    dport=

    neg=
    rule_matches=

    while [ -n "$1" ]
    do
        case "$1" in
        !)
            shift
            neg="! "
            continue
            ;;
        -A)
            shift
            chain=$1
            ;;
        -s)
            shift
            src=$1
            src_neg=$neg
            ;;
        -d)
            shift
            dst=$1
            dst_neg=$neg
            ;;
        -j)
            shift
            action=$1
            if echo "$action" | grep -q "^[A-Z]\+$"
            then
                handle_iptables_params "$@"
                shift $count
            fi
            ;;
        -p)
            shift
            prot=$1
            prot_neg=$neg
            ;;
        -m)
            shift
            match=$1
            rule_matches="$rule_matches $match"
            handle_iptables_params "$@"
            shift $count
            ;;
        -i)
            shift
            in=$1
            ;;
        -o)
            shift
            out=$1
            ;;
        *)
            ;;
        esac
        neg=
        shift
    done
}

parse_iptables_rule()
{
    eval do_parse_iptables_rule $(echo "$1" | sed "s/\\\\'/'/g;s/\`/\\\\\`/g;s/\\$/\\\\$/g")
}

#------- Firewall GUI Functions -------------------------------------------------------
init_vars ()
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

reset_rule_error ()
{
    rule_error=
}

check_rule ()
{
    init_vars
    do_rule nat PREROUTING A "$pf_rule" '' reset_rule_error > /dev/null 2>&1
    get_params
}

create_rule ()
{
    src=$1
    sport=$2
    dst=$3
    dport=$4
    rip=$5
    rport=$6
    [ "$src" ] && pf_rule="$pf_rule $src"
    [ "$sport" ] && sport=":$sport"
    [ "$dport" ] && dport=":$dport"
    [ "$rport" ] && rport=":$rport"
    pf_rule="$pf_rule$sport $dst$dport DNAT:$rip$rport"
}

get_params ()
{
    # restrictions:
    prot=$proto
    if_in=`echo $if_in_negopt$if_in | sed "s/ //g"`
    if_out=`echo $if_out_negopt$if_out | sed "s/ //g"`
    # still missing match-opts: state, mac, limit, length, ...
    
    src=${src_unmapped%:*}
    case $src_unmapped in
    *:*) sport=${src_unmapped#*:} ;;
    *)   sport= ;;
    esac

    dst=${dst_unmapped%:*}
    case $dst_unmapped in
    *:*) dport=${dst_unmapped#*:} ;;
    *)   dport= ;;
    esac

    mangle_ip_params `echo $action | sed -e 's/DNAT://'` keepslash nomap
    rip=$ip
    rport=$(echo $port | sed 's/:/-/')
}

show_rule_error ()
{
    show_html_header "$_PF_portforwarding"
    show_backlink
    echo "<br /><br />"
    show_error "$_MN_err" "<pre>`cat /tmp/pfcgi.$$ | htmlspecialchars`</pre>"
    show_html_footer
}
