#!/bin/sh
#------------------------------------------------------------------------------
# portfw6.cgi
# Last Update:  $Id$
#------------------------------------------------------------------------------

# get main helper functions
#set_debug=yes
. /srv/www/include/cgi-helper

# Get libs
. /srv/www/include/firewall6_functions.inc

# Security
: ${FORM_action:="show"}
case $FORM_action in
    show) sec_action="view" ;;
    *) sec_action="set" ;;
esac
check_rights "firewall-portfw" "$sec_action"

case $FORM_action in
show)
    show_html_header "$_PF6_portforwarding"

    case $FORM_type in
        complete)
            get_all_matches6
            show_tab_header "$_PF6_portforwarding - $_PF_simple_view" "$myname?type=simple" "$_PF6_portforwarding - $_PF_complete_view" no
            ;;
        *)
            get_active_matches6
            show_tab_header "$_PF6_portforwarding - $_PF_simple_view" no "$_PF6_portforwarding - $_PF_complete_view" "$myname?type=complete"
            ;;
    esac

    cat <<-EOF
        <table class="normtable">
        <tr>
        <th colspan="$match_nr">$_PF_matches</th>
        <th colspan="2" class="src">$_PF_src</th>
        <th colspan="2">$_PF_dst</th>
        <th colspan="2">$_PF_redirect_to</th>
        <th>$_PF_comment</th>
        <th>$_PF_action</th>
        </tr>
        <tr>
EOF
    for match in $matches
    do
            eval echo '\<th class=\"m_'$match'\"\>$_PF_m_'$match'\</th\>'
    done
    cat <<-EOF
        <th class="src">$_PF_host</th>
        <th class="src">$_PF_ports</th>
        <th>$_PF_host</th>
        <th>$_PF_ports</th>
        <th>$_PF_host</th>
        <th>$_PF_ports</th>
        <th>&nbsp;</th>
        <th></th>
        </tr>
EOF

    translate_ip6_net()
    { # replace function to do nothing, because we dont't want to see the translated values in the gui
        res=$1
    }

    src_used=no
    while read -r line
    do
        parse_iptables_rule "$line"
        [ -n "$comment_comment" ] || continue
        parse_comment "$comment_comment"
        [ "$table" = nat -a "$chain" = PORTFW ] || continue

        pf_rule="$(grep "^[^%]\+ %%% PF6_PREROUTING_${index}[:=]" $ip6tables_rules/$table/$chain | sed 's/\([^%]\+\) %%% .*/\1/')"
        pf_comment="$comment"

        if [ -n "$dynrule" ]
        then
            . $ip6tables_dynrules/$dynrule.rule
            pf_rule="$rule"
        fi

        check_rule6 "$pf_rule"
        [ "$rule_error" ] && continue # don't show faulty rules

        case $action in
        DNAT:*) ;;
        *)      continue ;;
        esac

        for match in $matches
        do
            val=`echo $pf_rule | sed -n -e "s/.*$match:\([^[:space:]]\+\).*/\1/p"`
            [ "$val" ] && pf_rule=`echo $pf_rule | sed -e "s/\(.*\)$match:[^[:space:]]\+\(.*\)/\1\2/"`
            eval echo '\<td class=\"m_'$match'\"\>$val\</td\>'
        done

        pfw6_fixup_ip src
        pfw6_fixup_ip dst
        [ -n "$src" -a "$src" != "any" -o -n "$sport" ] && src_used=yes
        cat <<-EOF
        <td class="src">$src</td>
        <td class="src">$sport</td>
        <td>$dst</td>
          <td>$dport</td>
          <td>$rip</td>
          <td>$rport</td>
          <td>$pf_comment</td>
          <td style="text-align: center">
          <a href="$myname?action=del_rule&amp;rulenum=$index">
          <img src="../img/delete.gif" border="0" title="$_PF_delete" alt="$_PF_delete"></a></td></tr>
EOF
    done <<EOF
$(ip6tables -t nat -S PORTFW  | grep "^-A")
EOF

    cat <<-EOF
        <tr><form name="create_rule" action="$myname" method="get">
EOF
    for match in $matches
    do
        eval echo '\<td class=\"m_'$match'\"\>\<input type=\"text\" name=\"'$match'\" size=\"15\" value=\"$default_'$match'\"\>\</td\>'
    done
    cat <<-EOF
        <td class="src"><input type="text" name="src" size="15" value="any"></td>
        <td class="src"><input type="text" name="sport" size="10"></td>
        <td><input type="text" name="dst" size="15" value="dynamic"></td>
        <td><input type="text" name="dport" size="10"></td>
        <td><input type="text" name="rip" size="15"></td>
        <td><input type="text" name="rport" size="10"></td>
        <td><input type="text" name="comment" size="15"></td>
        <td style="text-align: center">
            <input type="submit" name="action" value="$_PF_add" class="actions" title="$_PF_add">
        </td>
        </form></tr></table>
EOF
    # show src field only if necessary
    [ "$src_used" = "yes" -o "$FORM_type" = "complete" ] && echo "<style>.src{display: table-cell;}</style>"
    show_tab_footer
    show_html_footer
    ;;
# create forward rule
"$_PF_add")
    : ${FORM_dst:=dynamic}
    : ${FORM_src:=any}
    get_all_matches
    # add all transmitted matches to rule
    for match in $matches
    do
        eval val=\"\$FORM_$match\"
        [ "$val" ] && pf_rule="$pf_rule $match:$val"
    done

    create_rule6 "$FORM_src" "$FORM_sport" "$FORM_dst" "$FORM_dport" "$FORM_rip" "$FORM_rport"

    nextindex=$(get_next_rule_index6 nat PORTFW)
    if [ -z "$FORM_comment" ]
    then
        pf_comment="PF6_PREROUTING_${nextindex}='$pf_rule'"
    else
        pf_comment="PF6_PREROUTING_${nextindex}: $FORM_comment"
    fi

    fw_append_rule6 nat PORTFW "$pf_rule" "$pf_comment" exec_prerouting_rule6 > /tmp/pf6cgi.$$ 2>&1
    if [ -s /tmp/pf6cgi.$$ ]
    then
        show_rule_error6
    else
        reload
    fi
    rm -f /tmp/pf6cgi.$$
;;
# delete forward rule
del_rule)
    table=nat
    chain=PORTFW
    pf_line="$(grep "^[^%]\+ %%% PF6_PREROUTING_${FORM_rulenum}[:=]" $ip6tables_rules/$table/$chain)"
    pf_rule="$(echo "$pf_line" | sed 's/\([^%]\+\) %%% .*/\1/')"
    pf_comment="$(echo "$pf_line" | sed 's/[^%]\+ %%% \(.*\)/\1/')"

    fw_append_rule6 nat PORTFW "$pf_rule" "$pf_comment" exec_prerouting_rule6 > /tmp/pf6cgi.$$ 2>&1
    if [ -s /tmp/pf6cgi.$$ ]
    then
        show_rule_error6
    else
        reload
    fi
    rm -f /tmp/pf6cgi.$$
;;

esac
