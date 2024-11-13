#!/bin/sh
#------------------------------------------------------------------------------
# opt/files/srv/www/admin/control_circuits.cgi                     __FLI4LVER__
#
# Creation: sklein 1.11.2013
# Last Update:  $Id: control_circuits.cgi 28543 2013-11-02 08:14:03Z sklein $
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Ein wenig Doku für die Übersicht:
#
# Var               Inhalt
# FORM_id           id des Circuit
# FORM_state        Status des Circuits
# FORM_mode         (global)dial mode
#
#------------------------------------------------------------------------------
# set_debug='yes'
. /srv/www/include/cgi-helper
. /etc/boot.d/string.inc
. /etc/boot.d/env.inc
. /usr/share/circuits/api
check_rights "status" "change"

#------------------------------------------------------------------------------
# global
#------------------------------------------------------------------------------
control_globaldialmode(){
    show_html_header "Circuits" "refresh=5;url=control_circuits.cgi"
    FORM_mode=`circuit_get_global_dialmode`
    case $FORM_mode in
        auto)
            mode=`echo "<option selected value=\"auto\">"auto"</option><option value=\"manual\">"manual"</option><option value=\"off\">"off"</option>"`
            ;;
        manual)
            mode=`echo "<option value=\"auto\">"auto"</option><option selected value=\"manual\">"manual"</option><option value=\"off\">"off"</option>"`
            ;;
        off)
            mode=`echo "<option value=\"auto\">"auto"</option><option value=\"manual\">"manual"</option><option selected value=\"off\">"off"</option>"`
            ;; 
    esac
    show_tab_header "Globaldialmode" no
        echo "<form name=\"global\" action="$myname" method=\"post\">"
        echo '<select name="global_mode" size="1" onchange="this.form.submit()">'
        echo $mode
        echo "</select>&nbsp;</td>"
        echo "</form>"
    show_tab_footer
    echo "<br/>"
}

#------------------------------------------------------------------------------
# circuits
#------------------------------------------------------------------------------

control_circuits() {
show_tab_header "Circuits" no
    cat <<EOF
<table class="normtable"><tr>
<th align="center">&nbsp;Id&nbsp;</th>
<th align="center">&nbsp;Alias&nbsp;</th>
<th align="center">&nbsp;Type&nbsp;</th>
<th align="center">&nbsp;Interface&nbsp;</th>
<th align="center">&nbsp;Name&nbsp;</th>
<th align="center">&nbsp;Klassen&nbsp;</th>
<th align="center">&nbsp;Depends&nbsp;</th>
<th align="center">&nbsp;Dialmode&nbsp;</th>
<th align="center">&nbsp;State&nbsp;</th>
<th align="center" colspan="2">&nbsp;Change&nbsp;</th>
EOF
switch="odd"

    local _result
    if _result=$(mom_unicast_message_and_receive_reply \
                    circd get_all_states_circuit_message \
                    get_all_states_circuit_reply_message \
                    $circd_timeout)
    then
        local _varnames=$(echo "$_result" | extract_variable_names)
        local $_varnames
        eval "$_result"

        local i j
        for i in $(seq 1 $get_all_states_circuit_reply_message_circuit_n)
        do
            eval local circ_id=\$get_all_states_circuit_reply_message_circuit_${i}_id
            eval local circ_name=\$get_all_states_circuit_reply_message_circuit_${i}_name
            eval local circ_alias=\$get_all_states_circuit_reply_message_circuit_${i}_alias
            eval local circ_type=\$get_all_states_circuit_reply_message_circuit_${i}_type
            eval local circ_dev=\$get_all_states_circuit_reply_message_circuit_${i}_dev
            eval local circ_class_n=\$get_all_states_circuit_reply_message_circuit_${i}_class_n
            eval local circ_deps=\$get_all_states_circuit_reply_message_circuit_${i}_deps
            eval local circ_state=\$get_all_states_circuit_reply_message_circuit_${i}_state
            eval local circ_local_dialmode=\$get_all_states_circuit_reply_message_circuit_${i}_local_dialmode
            eval local circ_effective_dialmode=\$get_all_states_circuit_reply_message_circuit_${i}_effective_dialmode

            local circ_classes=
            for j in $(seq 1 $circ_class_n)
            do
                eval local circ_class=\$get_all_states_circuit_reply_message_circuit_${i}_class_${j}
                circ_classes="$circ_classes $circ_class"
            done

            FORM_id=$circ_id

            echo "<form name=\"status\" action="$myname" method=\"post\">"
            FORM_mode=$circ_local_dialmode
            case $FORM_mode in
                auto)
                    mode=`echo "<option selected value=\"auto\">"auto"</option><option value=\"manual\">"manual"</option><option value=\"off\">"off"</option>"`
                    ;;
                manual)
                    mode=`echo "<option value=\"auto\">"auto"</option><option selected value=\"manual\">"manual"</option><option value=\"off\">"off"</option>"`
                    ;;
                off)
                    mode=`echo "<option value=\"auto\">"auto"</option><option value=\"manual\">"manual"</option><option selected value=\"off\">"off"</option>"`
                    ;; 
            esac
            FORM_state=$circ_state
            FORM_mode_effective=$circ_effective_dialmode
            case $FORM_state in
                active)
                    case $FORM_mode_effective in
                        off)
                            statechange1=`echo '<input type="submit" value="dial" name="todo" class="actions" disabled="disabled" style="background:#A4A4A4; width:65px">'`
                            statechange2=`echo '<input type="submit" value="deactivate" name="todo" class="actions" style="width:65px">'`
                            ;;
                        *)
                            statechange1=`echo '<input type="submit" value="dial" name="todo" class="actions" style="width:65px">'`
                            statechange2=`echo '<input type="submit" value="deactivate" name="todo" class="actions" style="width:65px">'`
                            ;;
                    esac
                    ;;
                ready|semionline|online)
                    statechange1=`echo '<input type="submit" value="hangup" name="todo" class="actions" style="width:65px">'`                
                    statechange2=`echo '<input type="submit" value="deactivate" name="todo" class="actions" style="width:65px">'`
                    ;;
                failed)
                    statechange1=`echo '<input type="submit" value="dial" name="todo" class="actions" width:65px">'`
                    statechange2=`echo '<input type="submit" value="activate" name="todo" class="actions" style="width:65px">'`                
                    ;;
                *)
                    statechange1=`echo '<input type="submit" value="dial" name="todo" class="actions" disabled="disabled" style="background:#A4A4A4; width:65px">'`
                    statechange2=`echo '<input type="submit" value="activate" name="todo" class="actions" style="width:65px">'`
                    ;;
            esac
            echo "<tr class=\"$switch\"><td>&nbsp;$FORM_id&nbsp;</td><td>&nbsp;$circ_alias&nbsp;</td><td>&nbsp;$circ_type&nbsp;</td>"
            echo "<td>&nbsp;$circ_dev&nbsp;</td><td>&nbsp;$circ_name&nbsp;</td><td>&nbsp;$circ_classes&nbsp;</td>"
            echo "<td>&nbsp;$circ_deps&nbsp</td><td>&nbsp;"
            echo '<select name="mode_list" size="1" onchange="this.form.submit()">'
            echo $mode
            echo "</select>&nbsp;</td>"
            echo "<input type=\"hidden\" name=\"id\" value=\"$FORM_id\">"
            case $FORM_state in
                active)
                    echo '<td style="background-color:blue; color:white">&nbsp;<span style="font-weight:bold">'$FORM_state'</span>&nbsp;</td>'
                    ;;
                ready)
                    echo '<td style="background-color:cyan">&nbsp;<span style="font-weight:bold">'$FORM_state'</span>&nbsp;</td>'
                    ;;
                semionline)
                    echo '<td style="background-color:yellow">&nbsp;<span style="font-weight:bold">'$FORM_state'</span>&nbsp;</td>'
                    ;;
                online)
                    echo '<td style="background-color:green">&nbsp;<span style="font-weight:bold">'$FORM_state'</span>&nbsp;</td>'
                    ;;
                failed)
                    echo '<td style="background-color:red">&nbsp;<span style="font-weight:bold">'$FORM_state'</span>&nbsp;</td>'
                    ;;
                *)
                    echo '<td style="background-color:darkgray">&nbsp;<span style="font-weight:bold">'$FORM_state'</span>&nbsp;</td>'
                    ;;
            esac
            echo "<td>&nbsp;$statechange1&nbsp;</td>"
            echo "<td>&nbsp;$statechange2&nbsp;</td></tr>"
            echo "</form>"
            # change row-color
            if [ "$switch" = "odd" ]
            then
                switch="even"
            else
                switch="odd"
            fi
        done
    fi
    echo "</table><br/>"
show_tab_footer
show_html_footer                                             
}

#------------------------------------------------------------------------------
# Auswertung
#------------------------------------------------------------------------------

case $FORM_todo in
    dial)
        fli4lctrl dial $FORM_id
        ;;
    hangup)
        fli4lctrl hangup $FORM_id
        ;;
    activate)
        fli4lctrl up $FORM_id
        ;;
    deactivate)
        fli4lctrl down $FORM_id
        ;;
esac
case $FORM_mode_list in
    auto)
        fli4lctrl dialmode local $FORM_id auto
        ;;
    manual)
        fli4lctrl dialmode local $FORM_id manual
        ;;
    off)
        fli4lctrl dialmode local $FORM_id off
        ;;
esac
case $FORM_global_mode in
    auto)
        fli4lctrl dialmode global auto
        ;;
    manual)
        fli4lctrl dialmode global manual
        ;;
    off)
        fli4lctrl dialmode global off
        ;;
esac
control_globaldialmode
control_circuits
