#!/bin/sh
#------------------------------------------------------------------------------
# /srv/www/admin/ow.cgi - httpd Frontend                           __FLI4LVER__
#
# Creation:     04.02.2009 kmw <news4kmw@web.de>
# Modification: 22.01.2012 Roland Franke
# Last Update:  $Id$
#------------------------------------------------------------------------------

. /srv/www/include/cgi-helper
: ${OW_OPT_CONF_FILE:='/var/run/ow.conf'}
. $OW_OPT_CONF_FILE
[ -f $OW_DEVICE_LIB ] && . $OW_DEVICE_LIB

# The following routines provide the structure for all metadata.
# The data itself is inserted through the refresh routine.

StatusLED ()
  {
  local id=$1
  local status=$2
  #--------------
  if [ -n "$id" ]
  then
    echo '<img id="led-'$id'" src="../img/ow-led-grey.gif">'
  fi
  }

Text ()
  {
  local id=$1
  shift
  local text=$*
  #--------------
  if [ -n "$id" ]
  then
    echo '<input id="txt-'$id'" class=ow_unboxed type=text readonly value="">'
  fi
  }

InputField ()
  {
  local id=$1
  local value=$2
  local unit=$3
  #--------------
  if [ -n "$id" ]
  then
    echo '<input type=hidden name=command value="'$id'">'\
         '<input id="ifd-'$id'" type=text name=value value="">'"$unit"
  fi
  }

Command ()
  {
  local id=$1
  local command=$2
  #--------------
  if [ -n "$id" -a -n "$command" ]
  then
    echo '<input id="cmd-'$id'" name=command type=hidden value="">'\
         '<input id="img-'$id'" name=image type=image src="../img/ow-toggle-empty.gif" class=ow_unboxed>'
  fi
  }

# Command button execution

ParseCommand ()
  {
  local command="$*"
  #--------------
  if [ -n "$command" ]
  then
    if [ -n "`CheckUserRight admin`" -o -n "`CheckUserRight exec`" ]
    then
      local file=
      local value=
      eval `echo "$command" | sed -n 's|^\([0-9A-Fa-f]\{2\}\)\([0-9A-Fa-f]\{12\}\)[0-9A-Fa-f]\{2\}-\([^-]*\)-\(.*\)$|file="'$OW_OWFS_PATH'/\1.\2/\3";value="\4"|p'`
      if [ -n "$file" -a -f $file ]; then echo -n "$value" > $file; fi
    fi
  fi
  }

# Basic interface routines

CheckUserRight()
  {
  local required_right=$1
  #--------------
  local result=
  if [ -n "$REMOTE_USER" ]
  then
    local right=
    local sectionrights=
    local globalrights=
    for right in `cat /etc/httpd/rights | sed -n '\|^'"$REMOTE_USER"'=|{s|^[^=]*=||;p}'`
    do
      case "`echo $right | sed -n 's|^\([^:]*\):.*$|\1|p'`" in
        "$OW_RIGHTS_SECTION")
          sectionrights="`echo $right | sed 's|^[^:]*:||'`"
          : ${sectionrights:="none"}
          ;;
        "") globalrights=$right;;
      esac
    done
    local rights=$sectionrights
    : ${rights:=$globalrights}
    : ${rights:="all"}
    for right in `echo $rights | tr ',' ' '`
    do
      case $right in
        all) result="yes";;
        $required_right) result="yes";;
      esac
    done
  else
    result="yes"
  fi
  echo $result
  }

ReadDeviceTree ()
  {
  local device=
  ls -1 $OW_OWFS_PATH | sed -n '\|^[0-9A-Fa-f]\{2\}\.[0-9A-Fa-f]\{12\}$|p' | while read device
  do
    local device_type=`cat $OW_OWFS_PATH/$device/type`
    if type OWDevice_$device_type > /dev/null
    then
      OWDevice_$device_type "$OW_OWFS_PATH/$device" | sed 's|^|/'"$device"'/|'
    else
      echo "/$device/$device_type : '<div class=ow_error>'$_ow_error_unknown_device_type'</div>'"
    fi
  done
  }

AliasDeviceTree ()
  {
  local aliases='s|.*|&|'
  if [ -f $OW_ALIAS_FILE ]
  then
    aliases=`cat $OW_ALIAS_FILE`
  fi
  local adminfilter=
  if [ -z "`CheckUserRight admin`" ]
  then
    adminfilter=';\|/[Aa][Dd][Mm][Ii][Nn]/|d'
  fi
  local line=
  while read line
  do
    echo $line | sed '{'"$aliases"'}; \|/![^/:]* :|d'"$adminfilter"
  done
  }

ShowDeviceTree ()
  {
  local group=$1
  local action=$2
  #--------------
  echo '<table class="normtable">'
    echo "<tr><th id=ow_column_group>$_ow_header_group</th><th id=ow_column_data>$_ow_header_data</th></tr>"
    local linecoloring=
    : ${group:="/"}
    local s_group="`echo \"$group\" | sed 's|/|\\\/|g;s|\&|\\\&|g;s|\*|\\\*|g;s|\.|\\\.|g;s|\[|\\\[|g;s|\]|\\\]|g;s|\^|\\\^|g'`"
    local s_grouproot="`echo \"$group\" | sed 's|[^/]*$||;h;s|^/\([^/]*\).*|/\1/|;s|/|\\\/|g;x;s|^/[^/]*/\(.*\)$|\1|;s|[^/]*/|[^\\\/]*[\\\/]*|g;x;G;s|\n||'`"
    local line=
    # sed:
    #   Entries starting with current selection only
    #     Print if current selection is a property ..
    #     .. else ..
    #       Print if current selection contains a property ..
    #       .. else it's a subgroup, so add a " to make it stick at the top of the list during sort
    #   Entries not starting with current selection: List root group only
    #   Delete everything else
    # echo: Add a line for each subgroup of the current selection, as to guarantee, there is at least one matching entry in the list. This is needed for browsing through subgroups.
    # sort: case-insensive and ignore any special characters
    # sed:  Remove any preciding " in subgroup names of the current selection, as they were added for sorting only (see above)
    # sed
    #   Delete duplicate lines. Now we're dealing with the content of the current selection and the roots of all other groups, only.
    # sed
    #   Write meta-data to file,
    # while read
    #   Execute meta-data
    # sed
    #   Duplicate line, strip data, replace irrelevant subgroup names with undeletable white space and store white space and property name in separate lines
    #   Line #1: /root/subgroup/property : data
    #   Line #2: wsp wsp
    #   Line #3: property
    # sed
    #   Duplicate first line and put copy as url-encoded group in separate line
    #   Line #1: /root/subgroup/property : data
    #   Line #2: wsp wsp
    #   Line #3: property
    #   Line #4: url-encoded group
    # sed
    #   If first line contains a data delimiter (therefore data), HTML-encode entire set of lines as property
    # sed
    #   If line isn't allready HTML-encoded ..
    #     If property is a subgroup of the current selection, HTML-encode entire set of lines as subgroup
    #     Else HTML-encode set of lines as subgroup pointing to the current selection
    # sed: Insert line coloring to every second line
    ReadDeviceTree | AliasDeviceTree | ( \
      sed '\
        /^'"$s_group"'/ {\
          /^'"$s_group"'[ ]* : / P;\
          /^'"$s_group"'[ ]* : / !{\
            /^'"$s_group"'[^\/]* : / P;\
            /^'"$s_group"'[^\/]* : / !{s|^'"$s_group"'\([^/]*/\).*|'"$s_group"'"\1|p}}};\
        /^'"$s_group"'/ !{s|^\(/[^/]*[/]\).*|\1|p};\
        D' && \
      echo -n "$group" | sed 's|/\(.*\)/[^/]*|\1|;s|/|\n|g' | sed -n -e '/^$/ !{x;G;s|\n|/|p;x}' | sed 's|.*|&/|' ) |
      sort -f -d |
      sed 's|^'"$s_group"'"\([^/]*/\).*|'"$s_group"'\1|p' |
      sed '$!N;\
        /^\(.*\)\n\1$/!P;\
        D' |
      sed '/ : / w '"$OW_REFRESH_TEMP" |
      while read line; do eval echo $line; done |
      sed 'h;\
        s|[ ]* : .*||;\
        s|\([^/]*\)[/]*$|\n\1|;\
        s|^/||;\
        s|[^/]*/|\&nbsp\&nbsp\&nbsp|g;\
        x;G' |
      sed 'h;\
        s|[ ]* : .*||;\
        s/%/%25/g;s/ /%20/g;s/!/%21/g;s/"/%22/g;s/#/%23/g;s/\$/%24/g;s/\&/%26/g;s/'\''/%27/g;s/(/%28/g;s/)/%29/g;s/\*/%2a/g;s/+/%2b/g;s/,/%2c/g;s/-/%2d/g;s/\./%2e/g;s/\//%2f/g;s/:/%3a/g;s/;/%3b/g;s/</%3c/g;s/\=/%3d/g;s/>/%3e/g;s/?/%3f/g;s/@/%40/g;s/\[/%5b/g;s/\\/%5c/g;s/\]/%5d/g;s/\^/%5e/g;s/_/%5f/g;s/`/%60/g;s/{/%7b/g;s/|/%7c/g;s/}/%7d/g;s/~/%7e/g;s/ö/%f6/g;s/ü/%fc/g;s/ä/%e4/g;s/Ö/%d6/g;s/Ü/%dc/g;s/Ä/%c4/g;\
        x;N;N;G' |
      sed '\
        / : / !{N;N;N}\
        / : / {N;N;N;s|\(.*[^ ]\) [ ]*: \(.*[^ ]\).*\n\(.*\)\n\(.*\)\n\(.*\)|<td>\3<img class=ow_bullet src=\"../img/ow-port.gif\">\4</td><td><form action="'"$myname"'" method="POST"><input type=hidden name=action value="'"$FORM_action"'"><input type=hidden name=group value="'"$FORM_group"'">\2</form></td></tr>|}' |
      sed '/^<td>/ !{\
          /^'"$s_grouproot"'$/ !{N;N;N;\
            s|\(.*[^ ]\).*\n\(.*\)\n\(.*\)\n\(.*\)|<td>\2<a href="'"$myname"'?action='"$action"'\&group=\4"><img class=ow_bullet src=\"../img/ow-plus.gif\">\3</a></td><td></td></tr>|}
          /^'"$s_grouproot"'$/ {N;N;N;\
              s|\(.*[^ ]\).*\n\(.*\)\n\(.*\)\n\(.*\)|<td>\2<a href="'"$myname"'?action='"$action"'\&group=\4"><img class=ow_bullet src=\"../img/ow-reload.gif\"><b>\3</b></a></td><td></td></tr>|}}' |
      sed '$!N; s|\(.*\)\n\(.*\)|<tr>\1\n<tr class=ow_linecoloring>\2|' |
      while read line; do echo "$line"; [ 1 = 2 ]; done &&
        echo "<tr><td colspan=2><b>$_ow_error_empty_tree</b></td></tr>" # || echo '<tr><th>'"$_ow_header_refresh_time"'</th><th><input id="RefreshTime" class=ow_unboxed type=text readonly value=""></th></tr>'
  echo "</table>"
  # Straighten metadata file
  cat $OW_REFRESH_TEMP | sed -n '/\$(/ s|.* : ||p' | sed 's|^[^$]*||;s|[^)]*$||;/^$/ !s|^|echo |' > $OW_REFRESH_TEMP.new
  if [ -s $OW_REFRESH_TEMP.new ]
  then
    (echo "#!/bin/sh" && cat $OW_REFRESH_TEMP.new) > $OW_REFRESH_TEMP
  else
    echo -n > $OW_REFRESH_TEMP
  fi
  rm $OW_REFRESH_TEMP.new
  chmod 555 $OW_REFRESH_TEMP
  }

ModuleStarted ()
  {
  local module=$1
  #--------------
  if [ -n "$module" ]
  then
    eval local pid_file='$OW_'$module'_PID_FILE'
    if [ -n "$pid_file" -a -s $pid_file ]
    then
      eval local task='$OW_'$module'_BINARY'
      task=`echo $task | sed 's|.*/||;s| .*||'`
      ps w| grep "^[ ]*$(cat $pid_file).*$task .*"
    fi
  fi
  }

TaskStarted ()
  {
  local file=$1
  #--------------
  if [ -n "$file" ]
  then
    ps w| grep " $file " | grep -v " grep "
  fi
  }

ShowAdmin ()
  {
  local command=$1
  local action=$2
  #--------------
  local module=
  case $command in
    owfs_*)    module=OWFS;;
    owshell_*) module=OWSHELL;;
    owhttpd_*) module=OWHTTPD;;
    uscript_*) module=uscript;;
    *)         module="";;
  esac
  if [ -n "$module" ]
  then
    case $command in
      owfs_stop|owshell_stop|owhttpd_stop)
        eval local task='$OW_'$module'_BINARY'
        task=`echo $task | sed 's|.*/||;s| .*||'`
        if [ -n "`ModuleStarted $module`" -a -n "$task" ]
        then
          killall -SIGKILL $task
          if [ $command = owfs_stop ]
          then
            sleep 1
            umount $OW_OWFS_PATH
          fi
        fi
      ;;
      owfs_start|owshell_start|owhttpd_start)
        if [ -z "`ModuleStarted $module`" ]
        then
          eval '$OW_'$module'_BINARY -c $OW_MODULE_CONF_FILE'
        fi
      ;;
      uscript_start)
        if [ -x "$OW_USER_SCRIPT" -a -x "$OW_SCRIPT_WRAPPER" -a -z "`TaskStarted $OW_SCRIPT_WRAPPER`" ]
        then
          if [ -f "$OW_USER_SCRIPT_STOP" ]; then rm $OW_USER_SCRIPT_STOP; fi
          /bin/sh $OW_SCRIPT_WRAPPER &
        fi
      ;;
      uscript_stop)
        if [ -n "$OW_USER_SCRIPT_STOP" ]
        then
          echo -n > $OW_USER_SCRIPT_STOP
          sleep 1 # Wait for next user script cycle
        fi
      ;;
    esac
  fi
  echo '<table class="normtable">'
  local button=
  for module in OWFS OWSHELL OWHTTPD
  do
    if [ "`eval echo '$OW_'$module`" = yes ]
    then
      eval local fake='$OW_'$module'_FAKE'
      eval local tester='$OW_'$module'_TESTER'
      echo -n
      button='<a href="'"$myname"'?action='"$action"'&command='`echo $module'_'|tr "A-Z" "a-z"`
      if [ -n "`ModuleStarted $module`" ]
      then
        button=$button'stop"><img src="../img/ow-toggle-down.gif"></a>'
        if [ -z "$fake" -a -z "$tester" ]
        then
          led=green
        else
          led=yellow
        fi
      else
        button=$button'start"><img src="../img/ow-toggle-up.gif"></a>'
        led=red
      fi
      local mode=
      if [ -z "$fake" ]
      then
        if [ -z "$tester" ]
        then
          eval mode='$OW_'$module'_DEV'
        else
          mode=$_ow_tester_mode
        fi
      else
        mode=$_ow_fake_mode
      fi
      echo '<tr><th>'$module'</th><td><img src="../img/ow-led-'$led'.gif">'$button'</td><td>'"$mode"'</td></tr>'
    fi
  done
  if [ -n "$OW_USER_SCRIPT" ]
  then
    button='<a href="'"$myname"'?action='"$action"'&command=uscript_'
    if [ -n "`TaskStarted $OW_SCRIPT_WRAPPER`" ]
    then
      if [ -n "`ModuleStarted OWFS`" ]
      then
        led=green
      else
        led=yellow
      fi
      button=$button'stop"><img src="../img/ow-toggle-down.gif"></a>'
    else
      led=red
      button=$button'start"><img src="../img/ow-toggle-up.gif"></a>'
    fi
  else
    led=grey
    button=
  fi
  echo '<tr><th>'$_ow_header_user_script'</th><td><img src="../img/ow-led-'$led'.gif">'$button'</td><td>'$OW_USER_SCRIPT'</td></tr>'
  echo '</table>'
  }

: ${FORM_action:=status}

# Check User Access Right
case $FORM_action in
  status) check_rights "$OW_RIGHTS_SECTION" "view";;
  *)      check_rights "$OW_RIGHTS_SECTION" "admin";;
esac

show_html_header "$OW_OPT_TITLE" "$OW_CSS_FILE"
  _status=$_ow_tab_status
  _admin=$_ow_tab_admin
  if [ -n "`CheckUserRight admin`" ]
  then
    show_tab_header "$_status" "$myname?action=status" "$_admin" "$myname?action=admin"
  else
    show_tab_header "$_status" "$myname?action=status"
  fi
  case $FORM_action in
    status)
      ParseCommand "`echo \"$FORM_command-$FORM_value\" | sed 's|-$||'`"
      ShowDeviceTree "$FORM_group" "$FORM_action"
    ;;
    admin)
      ShowAdmin "$FORM_command" "$FORM_action"
    ;;
    debug)
      ReadDeviceTree | sed 's|$|<br>|'
    ;;
    *) show_error "$_ow_title_error" "$_ow_error_unknown_action";;
  esac
show_tab_footer

case $FORM_action in
  status)
    if [ -s $OW_REFRESH_TEMP ]
    then
      echo '<div id="refresh"><iframe src="'$OW_REFRESH_FILE'">Input State</iframe></div>'
    fi
  ;;
esac

show_html_footer
