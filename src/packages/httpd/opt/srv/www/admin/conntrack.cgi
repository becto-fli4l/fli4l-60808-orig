#!/bin/sh
#------------------------------------------------------------------------------
# conntrack.cgi - show conntrack table
#
# Creation:     17.10.2005  hh
# Last Update:  $Id$
#
#------------------------------------------------------------------------------
# get main helper functions
. /srv/www/include/cgi-helper
. /etc/boot.d/list.inc

LOCALHOST4="127.0.0.1"
LOCALHOST6="0000:0000:0000:0000:0000:0000:0000:0001"

ip_to_dnsname ()
{   # lookup unknown IPs
    for ip
    do
        get_dns_name $ip
        [ "$res" ] && echo "s/>`netcalc network $ip`</ title=\"$ip\">$res</g" >> /tmp/sed.$$
    done
}

ip_to_wan ()
{   # replace IP of default route interface in conntrack with "WAN-IP"
    if [ -f /etc/default-route-interface ]
    then
        dev=`cat /etc/default-route-interface`
        if [ -f /var/run/$dev.ip ]
        then
            wanip=`cat /var/run/$dev.ip`
            echo "s/>$wanip</ title=\"$wanip\">WAN-IP</g" >> /tmp/sed.$$
        fi
    fi
}

ip_to_hostnames ()
{   # replace IPs of hosts in conntrack with names
    hosts=' '
    sed -n -e '/^[0-9a-z]/s/^\([^[:space:]]\+\)[[:space:]]\+\([^[:space:]]\+\).*/\1 \2/p' /etc/hosts /etc/hosts.d/* | while read ip name; do
        case "$hosts" in
            *" $ip "*) continue ;;
        esac
        hosts="$hosts$ip "

        sip=$(netcalc network $ip)
        echo "s/>$sip</ title=\"$ip\">$name</g"
    done >> /tmp/sed.$$
    case $OPT_DHCP_$DHCP_TYPE in
        yes_dnsmasq)
            set -f
            while read line
            do
                set -- $line
                case $4 in
                    "*") continue ;;
                esac
		case "$hosts" in
		    *" $4 "*) continue ;;
		esac
		hosts="$hosts$4 "
		echo "s/>$3</ title=\"$3\">$4</g"
            done < $DHCP_LEASES_DIR/dnsmasq.leases >> /tmp/sed.$$
            set +f
        ;;
    esac
}

expand_ipv6 ()
{
    echo $1 | sed -e 's/^:/0000:/;s/:$/:0000/' \
-e 's/^\(\([^:]\+:\)\{1\}\)\(\(:[^:]\+\)\{1,5\}\)$/\10000:\3/' \
-e 's/^\(\([^:]\+:\)\{2\}\)\(\(:[^:]\+\)\{1,4\}\)$/\10000:\3/' \
-e 's/^\(\([^:]\+:\)\{3\}\)\(\(:[^:]\+\)\{1,3\}\)$/\10000:\3/' \
-e 's/^\(\([^:]\+:\)\{4\}\)\(\(:[^:]\+\)\{1,2\}\)$/\10000:\3/' \
-e 's/^\(\([^:]\+:\)\{5\}\)\(\(:[^:]\+\)\{1\}\)$/\10000:\3/' \
-e 's/::/:0000:/' \
-e 's/:\(\([^:]\)\{1\}\):/:000\1:/' \
-e 's/:\(\([^:]\)\{1\}\):/:000\1:/' \
-e 's/:\(\([^:]\)\{1\}\):/:000\1:/' \
-e 's/:\(\([^:]\)\{1\}\):/:000\1:/' \
-e 's/:\(\([^:]\)\{1\}\):/:000\1:/' \
-e 's/:\(\([^:]\)\{1\}\):/:000\1:/' \
-e 's/:\(\([^:]\)\{2\}\):/:00\1:/' \
-e 's/:\(\([^:]\)\{2\}\):/:00\1:/' \
-e 's/:\(\([^:]\)\{2\}\):/:00\1:/' \
-e 's/:\(\([^:]\)\{2\}\):/:00\1:/' \
-e 's/:\(\([^:]\)\{2\}\):/:00\1:/' \
-e 's/:\(\([^:]\)\{2\}\):/:00\1:/' \
-e 's/:\(\([^:]\)\{3\}\):/:0\1:/' \
-e 's/:\(\([^:]\)\{3\}\):/:0\1:/' \
-e 's/:\(\([^:]\)\{3\}\):/:0\1:/' \
-e 's/:\(\([^:]\)\{3\}\):/:0\1:/' \
-e 's/:\(\([^:]\)\{3\}\):/:0\1:/' \
-e 's/:\(\([^:]\)\{3\}\):/:0\1:/' \
-e 's/^\(\([^:]\)\{1\}\):/000\1:/g' \
-e 's/^\(\([^:]\)\{2\}\):/00\1:/g' \
-e 's/^\(\([^:]\)\{3\}\):/0\1:/g' \
-e 's/:\(\([^:]\)\{1\}\)$/:000\1/g' \
-e 's/:\(\([^:]\)\{2\}\)$/:00\1/g' \
-e 's/:\(\([^:]\)\{3\}\)$/:0\1/g'
}

ipv6_short ()
{
    for ip in $(list_unique `grep -o -E "(([0-9a-f]{4}:){7}([0-9a-f]{4}))" /tmp/conntrack.$$`)
    do
        short_ip=`netcalc network $ip`
        echo "s/$ip/$short_ip/g" >> /tmp/sed.$$
    done
}

# calculates a modulo b
# $1 = first argument
# $2 = second argument
mod ()
{
	expr $1 - \( $1 / $2 \) \* $2
}

# adds a regex for a single-digit range to $result
# $1 = first digit
# $2 = second digit
# $3 = ignore leading zero
gen_ipv4_range_filter_digits ()
{
#	echo "filter_digits: first=$1 last=$2 ignore=$3" >&2
	local begin=$1
	if [ $1 -eq 0 -a $3 -ne 0 ]; then
		[ $2 -eq 0 ] && return 1 || begin=1
	fi
	if [ $begin -eq $2 ]; then
		result="$result$begin"
	elif [ `expr $begin + 1` -lt $2 ]; then
		result="$result[$begin-$2]"
	else
		result="$result[$begin$2]"
	fi
	[ $1 -eq 0 -a $3 -ne 0 ] && result="$result\?"
	return 0
}

# adds a regex for a range with a given arity to $result
# $1 = first digit
# $2 = second digit
# $3 = arity
# $4 = ignore leading zero
gen_ipv4_range_filter_complete_range ()
{
#	echo "filter_complete_range: first=$1 last=$2 arity=$3 ignore=$4" >&2
	[ $1 -eq 0 -a $3 -gt 1 -a $4 -ne 0 ] && local ignore=1 || local ignore=0
	gen_ipv4_range_filter_digits $1 $2 $ignore
	local i=$3
	while [ $i -gt 1 ]
	do
		[ $i -gt 10 ] || ignore=0
		gen_ipv4_range_filter_digits 0 9 $ignore
		i=`expr $i / 10`
	done
}

# adds a regex for a range with a given arity to $result
# $1 = start of range
# $2 = end of range
# $3 = arity (1, 10, 100, ...)
# $4 = 1 if suppress leading zeros, 0 otherwise
# $5 = 1 if bracket expression, 0 if it is optional
gen_ipv4_range_filter ()
{
	local f1=`expr $1 / $3`
	local r1=`mod $1 $3`
	local f2=`expr $2 / $3`
	local r2=`mod $2 $3`
	local limit=`expr $3 - 1`
#	echo "filter: first=$1 last=$2 arity=$3 ignore=$4 bracket=$5 f1=$f1 r1=$r1 f2=$f2 r2=$r2 limit=$limit" >&2

	if [ $f1 -eq $f2 ]; then
		[ $f1 -eq 0 -a $4 -ne 0 ] && local ignore=1 || local ignore=0
		gen_ipv4_range_filter_digits $f1 $f1 $ignore
		gen_ipv4_range_filter $r1 $r2 `expr $3 / 10` $ignore $5
	else
		local parts=0
		local old="$result"
		result=""
		ignore=$4
		if [ $r1 -ne 0 ]; then
			[ $f1 -eq 0 -a $4 -ne 0 ] && local ignore=1 || local ignore=0
			gen_ipv4_range_filter_digits $f1 $f1 $ignore
			[ $? -eq 0 ] && local bracket=1 || local bracket=0
			gen_ipv4_range_filter `mod $1 $3` $limit `expr $3 / 10` $ignore $bracket
			f1=`expr $f1 + 1`
			parts=1
		fi
		local prefix="$result"
		result=""
		if [ $r2 -ne $limit ]; then
			[ $f2 -eq 0 -a $4 -ne 0 ] && local ignore=1 || local ignore=0
			gen_ipv4_range_filter_digits $f2 $f2 $ignore
			[ $? -eq 0 ] && local bracket=1 || local bracket=0
			gen_ipv4_range_filter 0 `mod $2 $3` `expr $3 / 10` $ignore $bracket
			f2=`expr $f2 - 1`
			parts=`expr $parts + 1`
		fi
		local suffix="$result"
		result=""
		if [ $f1 -le $f2 ]; then
			[ $f1 -eq 0 -a $4 -ne 0 ] && local ignore=1 || local ignore=0
			gen_ipv4_range_filter_complete_range $f1 $f2 $3 $ignore
			parts=`expr $parts + 1`
		fi
		local middle="$result"
		[ -n "$suffix" -a -n "$prefix$middle" ] && suffix="\|$suffix"
		[ -n "$middle" -a -n "$prefix" ] && middle="\|$middle"

		result="$old"
		[ $parts -gt 1 -a $5 -ne 0 ] && result="$result\("
		result="$result$prefix$middle$suffix"
		[ $parts -gt 1 -a $5 -ne 0 ] && result="$result\)"
	fi
}

# creates a regex for all hosts within a subnet
# $1 = net
# $2 = subnet mask (number of bits to use from the left, between 0 and 32)
create_ipv4_submask_filter ()
{
	local net="$1"
	local bits="$2"
	local missing=`expr 32 - $bits`
	local suffix=""
	while [ $missing -ge 8 ]
	do
		[ -n "$suffix" ] && suffix="$suffix\\."
		suffix="$suffix[^ ]*"
		missing=`expr $missing - 8`
	done
	local prefix=""
	while [ $bits -ge 8 ]
	do
		[ -n "$prefix" ] && prefix="$prefix\\."
		prefix="$prefix${net%%.*}"
		net="${net#*.}"
		bits=`expr $bits - 8`
	done
	local middle=""
	if [ $bits -gt 0 ]
	then
		local next="${net%%.*}"
		local w=256
		while [ $bits -gt 0 ]
		do
			w=`expr $w / 2`
			bits=`expr $bits - 1`
		done
		local last=`expr $next + $w - 1`
		result=""
		gen_ipv4_range_filter $next $last 100 1 1
		middle="$result"
		[ -n "$prefix" ] && prefix="$prefix\\."
	fi
	[ -n "$prefix$middle" -a -n "$suffix" ] && suffix="\\.$suffix"
	echo "$prefix$middle$suffix"
}

create_gen_filter ()
{
    # [ -f /var/run/hc_no_generic_filter ] && return 1
    # if [ -f /var/run/hc_generic_filter ]; then
    #	read host_expr < /var/run/hc_generic_filter
    #	return
    # fi

    dri_ips=' '
    dri=`ip r s | sed -n -e '/^default/s/.*dev //p'`
    [ "$dri" ] && for dri_ip in `ip a s $dri | sed -n -e 's/^ *inet6\? \([^ ]*\).*/\1/p'`
    do
        dri_ips="$dri_ips$dri_ip "
    done
    for net in `ip a s | sed -n -e 's/^ *inet6\? \([^ ]*\).*/\1/p'`; do
        case "$dri_ips" in
            *" $net "*) continue ;;
        esac
        [ "$dri_ip" = "$net" ] && continue
        filter="create_ipv4_filter"
        case $net in
        */*)
            mask=${net##*/}             # 127.0.0.1/8 -> 8
            ;;
        *:*/*)
            mask=${net##*/}             # 2001::1/64 -> 64
            filter="create_ipv6_filter"
            ;;
        *.*)
            mask=32
            ;;
        *:*)
            mask=128
            filter="create_ipv6_filter"
            ;;
        esac
        net=`netcalc network $net` # normalize network
        if ! eval $filter $net $mask; then
            return 1
        fi
    done
    # echo "$host_expr" > /var/run/hc_generic_filter
    return 0
}

create_ipv4_filter ()
{
	expr=
	case $2 in
	    8)  ip=${1%%.*}  ;; # 127.0.0.1 -> 127
	    16) ip=${1%.*.*} ;; # 127.0.0.1 -> 127.0
	    24) ip=${1%.*}   ;; # 127.0.0.1 -> 127.0.0
	    32) ip=${1%.*} ; expr="\\.${1##*.}"  ;; # 127.0.0, .1
	    *) ip=""; expr="`create_ipv4_submask_filter $1 $2`" ;;
	esac
	[ "$expr" ] || expr='\.[^ ]*'

	ip=`echo $ip | sed -e 's/\./\\\\./g'`
	if [ "$host_expr" ]; then
	    host_expr="$host_expr\|$ip$expr"
	else
	    host_expr="$ip$expr"
	fi
}

create_ipv6_filter ()
{
	# TODO: adapt create_ipv4_submask_filter to work with hexadecimal numbers
	return 1
}

create_filter ()
{
    if ! create_gen_filter; then
        host_expr=
        for ip in $(list_unique $LOCALHOST4 $LOCALHOST6 `sed -n -e '/^[0-9a-z]/s/[[:space:]].*//p' /etc/hosts /etc/hosts.d/*`)
        do
            case $ip in
            *:*) ip=`expand_ipv6 $ip` ;;
            esac
            if [ "$host_expr" ]; then
                host_expr="$host_expr\|$ip"
            else
                host_expr="$ip"
            fi
        done
    fi
    port_expr='53'
    [ "$HTTPD_PORT" ]   && port_expr="$port_expr\|$HTTPD_PORT"
    [ "$IMOND_PORT" ]   && port_expr="$port_expr\|$IMOND_PORT"
    [ "$TELMOND_PORT" ] && port_expr="$port_expr\|$TELMOND_PORT"

    fcmd="\(\(src\|dst\)=\($host_expr\) \)\{2\}.*dport=\($port_expr\) "
}

get_conntrack_data()
{
    conntrack -L -f ipv4 | sed 's/.*/ipv4 2 &/'
    conntrack -L -f ipv6 | sed 's/.*/ipv6 10 &/'
}

# Security
check_rights "conntrack" "view"

show_html_header "$_MP_contrck"

if [ -f /srv/www/admin/status_rrdconntrack.cgi ]
then
    show_tab_header "$_CONT_aconn" no "$_MP_contrckrrd" status_rrdconntrack.cgi
else
    show_tab_header "$_CONT_aconn" no
fi

case $FORM_filter in yes) fchecked="checked" ;; esac
case $FORM_dns in yes) dchecked="checked" ;; esac

cat <<EOF
<form action="$myname" method="GET">
<input type="hidden" name="sort" value="$(echo "$FORM_sort" | htmlspecialchars quot)">
<script type="text/javascript">
    document.write("<input onChange=\"submit()\" type=\"checkbox\" name=\"filter\" value=\"yes\" $fchecked> $_CONT_filterlocal")
</script>
<noscript>
    <input type="checkbox" name="filter" value="yes" $fchecked> $_CONT_filterlocal
    <input class="sbutton" type="submit">
</noscript>
<br>
<script type="text/javascript">
    document.write("<input onChange=\"submit()\" type=\"checkbox\" name=\"dns\" value=\"yes\" $dchecked> $_CONT_dns")
</script>
<noscript>
    <input type="checkbox" name="dns" value="yes" $dchecked> $_CONT_dns
    <input class="sbutton" type="submit">
</noscript>

</form>
EOF

case $FORM_sort in
    src) srcimg='<img alt="sort" src="../img/snormal.gif">' ;;
    dst) dstimg='<img alt="sort" src="../img/snormal.gif">' ;;
esac

# create html table
cat >/tmp/sed.$$ <<EOF
/tcp/s/^\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\(\([^ ]\+\) \)\{2,4\}src=\([^ ]*\) dst=\([^ ]*\) sport=\([^ ]*\) dport=\([^ ]*\) .*$/\1	\3	\6	\8	\7	\9	\4/
/udp/s/^\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\(\([^ ]\+\) \)\{2,4\}src=\([^ ]*\) dst=\([^ ]*\) sport=\([^ ]*\) dport=\([^ ]*\) .*$/\1	\3	\6	\8	\7	\9	/
/icmp/s/type=8/type=echo-req/;/icmp/s/type=0/type=echo-reply/
/icmp/s/type=13/type=timestamp/;/icmp/s/type=14/type=timestamp-repl/
/icmp/s/type=15/type=info-req/;/icmp/s/type=16/type=info-reply/
/icmp/s/type=17/type=address/;/icmp/s/type=18/type=address-reply/
/icmpv6/s/type=128/type=echo-req/;/icmpv6/s/type=129/type=echo-reply/
/icmp/s/^\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\(\([^ ]\+\) \)\{2,4\}src=\([^ ]*\) dst=\([^ ]*\) type=\([^ ]*\) .* type=\([^ ]*\) .*$/\1	\3	\6	\8	\7	\9	/
/icmpv6/s/^\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\(\([^ ]\+\) \)\{2,4\}src=\([^ ]*\) dst=\([^ ]*\) type=\([^ ]*\) .* type=\([^ ]*\) .*$/\1	\3	\6	\8	\7	\9	/
/unknown/s/^\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\(\([^ ]\+\) \)\{2,4\}src=\([^ ]*\) dst=\([^ ]*\).*$/\1	\3	\6		\7		/
s/	/<\/td><td>/g
s/^/<tr><td>/
s%$%</td></tr>%
EOF


case $FORM_sort in
    src|dst)
        case $FORM_sort in
            src) sstr='\3:\5' ;;
            dst) sstr='\5:\3' ;;
        esac
        scmd='sed -f /tmp/sort.$$ | sort | sed -e "s/sort=[^ ]* *//" |'
        cat >/tmp/sort.$$ <<EOF
	s/\(.*\)\(src=\)\([0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\)\( dst=\)\([0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\)\(.*\)\(src=\)\(.*\)/sort=$sstr \1\2\3\4\5\6\7\8/
	s/sort=\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\):\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\)\.\([0-9]\{1,3\}\) \(.*\)/sort=00\1.00\2.00\3.00\4:00\5.00\6.00\7.00\8 \9/
	s/sort=0\{0,2\}\([0-9]\{3\}\.\)0\{0,2\}\([0-9]\{3\}\.\)0\{0,2\}\([0-9]\{3\}\.\)0\{0,2\}\([0-9]\{3\}:\)0\{0,2\}\([0-9]\{3\}.\)0\{0,2\}\([0-9]\{3\}.\)0\{0,2\}\([0-9]\{3\}.\)0\{0,2\}\([0-9]\{3\}.\)\(.*\)/sort=\1\2\3\4\5\6\7\8\9/
EOF
    ;;
    time)
        scmd='sed "s/\([a-z]\{3,4\} *[0-9]\{1,10\} \)\([0-9]\{1,10\}\)\(.*\)/\2 \1\2\3/" | sort -n | sed "s/^[0-9]\{1,10\} //" |'
    ;;
    *) scmd= ;;
esac

expr_est="tcp.*ESTABLISHED\|udp.*ASSURED"

case $FORM_filter in
    yes)
	create_filter
	get_conntrack_data 2>/dev/null | grep -e "$expr_est" | grep -v -e "$fcmd"
	;;
    *)
	get_conntrack_data 2>/dev/null | grep -e "$expr_est"
	;;
esac > /tmp/conntrack.$$

ipv6_short
ip_to_wan
ip_to_hostnames
case $FORM_dns in
    yes)
        ip_to_dnsname $(list_unique `cat /tmp/conntrack.$$ | sed "s/^\([^ ]*\) *\([^ ]*\) *\([^ ]*\) *\(\([^ ]\+\) \)\{2,4\}src=\([^ ]*\) dst=\([^ ]*\).*$/\6 \7/"`)
    ;;
esac

cat <<EOF
<p>$_CONT_numcon: `cat /tmp/conntrack.$$ | grep -c use`</p>
<table class="normtable">
<tr><th>$_CONT_layer3</th><th>$_CONT_prot</th><th colspan="2"><a href="$myname?filter=$(url_encode "$FORM_filter")&amp;sort=src&amp;dns=$(url_encode "$FORM_dns")">$_CONT_src$srcimg</a></th><th colspan="2"><a href="$myname?filter=$(url_encode "$FORM_filter")&amp;sort=dst&amp;dns=$(url_encode "$FORM_dns")">$_CONT_dest$dstimg</a></th><th>$_CONT_state</th></tr>
<tr><th>&nbsp;</th><th>&nbsp;</th><th>$_CONT_IP</th><th>$_CONT_port</th><th>$_CONT_IP</th><th>$_CONT_port</th><th></th></tr>
EOF

eval 'cat /tmp/conntrack.'$$' | '$scmd ' sed -f /tmp/sed.'$$

rm /tmp/sort.$$ /tmp/sed.$$ /tmp/conntrack.$$ 2>/dev/null

echo '</table>'
show_tab_footer
show_html_footer
