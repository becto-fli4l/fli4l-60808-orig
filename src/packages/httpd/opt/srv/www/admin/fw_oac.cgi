#!/bin/sh
# set_debug=yes
# get main helper functions
. /srv/www/include/cgi-helper

check_rights "oac" "view"

_rshort=5 # short reload (5)
_rurl=$myname # reload-url

configfile=/var/run/oac.cfg
configfile_groups=/var/run/oac-groups.cfg

show_html_header "Online Access Control" "cssfile=status"

OAC_GROUP_N=0
if [ -f $configfile_groups ]
then
    . $configfile_groups
fi

filter=''
skip=0
group=0

if [ 0${FORM_group} -gt 0 -a 0${FORM_group} -le $OAC_GROUP_N ]
then
    group=${FORM_group}
fi

show_tab_header "$_OAC_tab_header" no
invisible="no"
tabsused=0
tabs=''
if [ $OAC_GROUP_N -gt 0 ]
then
    tabs=''
    for i in `seq 0 $OAC_GROUP_N`
    do
        if [ $i -eq 0  ]
        then
            if [ "$OAC_ALL_INVISIBLE" = "yes" ]
            then
                invisible='yes'
            else
                tabs="\"$_OAC_tab_all\""
                tabsused=`expr $tabsused + 1`
            fi
        else
            eval invisible=\$OAC_GROUP_${i}_INVISIBLE
            if [ "$invisible" != "yes" ]
            then
                eval tabname=\$OAC_GROUP_${i}_NAME
                tabs="$tabs \"$tabname\""
                tabsused=`expr $tabsused + 1`
            fi
        fi
        if [ "$invisible" != "yes" ]
        then
            if [ "$OAC_ALL_INVISIBLE" = "yes" -a "$group" -eq 0 ]
            then
                group=$i
            fi
            if [ $group = $i ]
            then
                tabs="$tabs \"no\""
            else
                tabs="$tabs \"$myname?group=$i\""
            fi
        fi
    done
    if [ "$tabsused" -gt 1 ]
    then
        eval "show_tab_header $tabs"
    fi
fi

if [ $group -gt 0 ]
then
    eval numclients=\$OAC_GROUP_${group}_CLIENT_N
    filter='^ '
    for i in `seq 1 $numclients`
    do
        eval client=\$OAC_GROUP_${group}_CLIENT_$i
        filter="$filter $client"
    done
    filter="$filter \$"
fi

eval `set | sed -n 's/^\(FORM_action\)\(\(un\|\)block\(group\|\)\)\(\(num\)\([0-9]\+\)\|\)\(delay[12]\|\)=.*/\1=\2;num=\7;mode=\8/p'`

# set | grep "^FORM_" | show_info "Environment"  
if [ -n "${FORM_action}" ]
then
    case ${FORM_action}
        in
        block|unblock) eval limit=\$FORM_limit_$num;;
        blockgroup|unblockgroup) limit=$FORM_limit;;
    esac
fi

# Reverse action if delayed action is selected
if [ "$mode" = "delay1" -o "$mode" = "delay2" ]
then
  if [ "$mode" != "delay1" -o "$limit" -gt 0 ]
  then
    case ${FORM_action} in
      block) FORM_action="unblock";;
      unblock) FORM_action="block";;
      blockgroup) FORM_action="unblockgroup";;
      unblockgroup) FORM_action="blockgroup";;
    esac
  fi
fi

if [ -n "${FORM_action}" ]
then
    case ${FORM_action}
        in
        block|unblock) /usr/local/bin/oac.sh $FORM_action limit=$limit $num >/dev/null 2>&1;;
        blockgroup|unblockgroup) /usr/local/bin/oac.sh $FORM_action limit=$limit $FORM_group >/dev/null 2>&1;;
    esac
fi

limits=`sed "s/\\$_OAC_text_hours/$_OAC_text_hours/" /var/run/oac-limits.cfg`

echo "<form action=\"fw_oac.cgi\" method=\"post\"><input type=\"hidden\" name=\"group\" value=\"${group}\"/>"

cat <<EOF
<table class="normtable"><tr>
<th align="center">$_OAC_tab_host</th>
<th align="center">$_OAC_tab_ip</th>
<th align="center">$_OAC_tab_online</th>
<th align="center">$_OAC_tab_state</th>
<th align="center">$_OAC_tab_action</th>
<th align="center">$_OAC_tab_limit</th>
<th align="center">$_OAC_tab_delayed</th></tr>
EOF

tooltipTitle="${_OAC_tt_title}"

oac.sh status | while read line
do
    eval $line
    case "$filter" in
        '' | *" $name "*) ;;
        *)           continue ;;
    esac


    case "$fwstat" in
        blocked)
            fwicon='world_delete.png'
            fwstatus="$_OAC_blocked"
            cssstatus='offline'
	    action='unblock'
            ;;
        *)
            fwicon='world_go.png'
            fwstatus="$_OAC_unblocked"
            cssstatus='online'
	    action='block'
    esac
    case "$state" in
        online)
            state_text="$_OAC_text_on"
            ;;
        *)
            state_text="$_OAC_text_off"
            ;;
    esac
    cat <<EOF
<tr class="$cssstatus">
  <td>&nbsp;$name&nbsp;</td>
  <td>&nbsp;$IP4&nbsp;</td>
  <td align="center">
    <img src="/img/$state.png" width="16" height="16" class="tooltip::Power Status;$state_text" alt="$state_text">
  </td>
  <td align="center">
    <img src="/img/$fwicon" width="16" height="16" class="tooltip::Status;$fwstatus" alt="$fwstatus">
  </td>
EOF
    if [ $limit != none ]
    then
	case $action in
	  block)
	    class1="tooltip::${tooltipTitle};${_OAC_tt_body_disable_t1} '$name' ${_OAC_tt_body_disable_t2}" 
	    class2="tooltip::${tooltipTitle};${_OAC_tt_body_disable_v1} '$name' ${_OAC_tt_body_disable_v2}" 
	    ;;
	  *)
            class1="tooltip::${tooltipTitle};${_OAC_tt_body_enable_t1} '$name' ${_OAC_tt_body_enable_t2}" 
            class2="tooltip::${tooltipTitle};${_OAC_tt_body_enable_v1} '$name' ${_OAC_tt_body_enable_v2}" 
	    ;;
	esac
	eval val1=\"\$\{_OAC_action_$action\}\"
	eval val2=\"\$\{_OAC_action_dellimit\}\"
        cat <<EOF
  <td>
    <input name="action${action}num${num}" type="submit" 
	    class="$class1"
	    value="$val1"/>&nbsp;&nbsp;&nbsp;&nbsp;
  </td>
  <td align="center">$limit ${_OAC_text_hours}</td>
  <td>
    <input name="action${action}num${num}delay2" type="submit" 
	    class="$class2"
	    value="$val2"/>&nbsp;&nbsp;&nbsp;&nbsp;
  </td>
</tr>
EOF
    else
	case $action in
	  block)
	    class1="tooltip::${tooltipTitle};${_OAC_tt_body_disable_t1} '$name' ${_OAC_tt_body_disable_t2}" 
	    class2="tooltip::${tooltipTitle};${_OAC_tt_body_disable_v1} '$name' ${_OAC_tt_body_disable_v2}" 
	    ;;
	  *)
            class1="tooltip::${tooltipTitle};${_OAC_tt_body_enable_t1} '$name' ${_OAC_tt_body_enable_t2}" 
            class2="tooltip::${tooltipTitle};${_OAC_tt_body_enable_v1} '$name' ${_OAC_tt_body_enable_v2}" 
	    ;;
	esac
	eval val1=\"\$\{_OAC_action_$action\}\"
	eval val2=\"\$\{_OAC_action_$action\}\"
        cat <<EOF
  <td>
    <input name="action${action}num${num}" type="submit" 
	    class="$class1"
	    value="$val1"/>&nbsp;&nbsp;&nbsp;&nbsp;
  </td>
  <td>
    <select name="limit_${num}" size="1"><option value="0">$_OAC_tab_limit_text</option>
    $limits
    </select>
  </td>
  <td>
    <input name="action${action}num${num}delay1" type="submit" 
	    class="$class2"
	    value="$val2"/>&nbsp;&nbsp;&nbsp;&nbsp;
  </td>
</tr>
EOF
    fi
done
if [ $group -gt 0 ]
then
    cat <<EOF
  <tr class="calling">
    <td colspan="3">&nbsp;$_OAC_tab_all&nbsp;</td>
    <td>
      <input name="actionblockgroup" type="submit" 
             class="tooltip::${tooltipTitle};$_OAC_action_block" 
             value="${_OAC_action_block}"/>&nbsp;&nbsp;&nbsp;&nbsp;
    </td>
    <td>
      <input name="actionunblockgroup" type="submit" 
             class="tooltip::${tooltipTitle};$_OAC_action_unblock" 
             value="${_OAC_action_unblock}"/>&nbsp;&nbsp;&nbsp;&nbsp;
    </td>
    <td>
      <select name="limit" size="1"><option value="0">$_OAC_tab_limit_text</option>
	$limits
      </select>
    </td>
    <td>&nbsp;</td>
  </tr>
EOF
fi
echo "</table></form>"

if [ "$tabsused" -gt 1 ]
then
    eval "show_tab_footer $tabs"
fi

show_tab_footer
show_html_footer
