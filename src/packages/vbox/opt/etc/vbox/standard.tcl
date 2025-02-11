
# First we clear the touchtone sequences and remove all entries from
# the callerid breaklist.

vbox_init_touchtones

vbox_breaklist rem all

# If variable VBOX_FLAG_STANDARD is TRUE we must play the standard
# message...

if { "$vbox_flag_standard" == "TRUE" } {

   set RC [ vbox_put_message $vbox_msg_standard ]

   vbox_pause 500

   if { "$RC" == "HANGUP" } {
      return
   }
}

# If variable VBOX_FLAG_BEEP is TRUE we must play the beep
# message...

if { "$vbox_flag_beep" == "TRUE" } {

   set RC [ vbox_put_message $vbox_msg_beep ]

   vbox_pause 500

   if { "$RC" == "HANGUP" } {
      return
   }
}

# If variable VBOX_FLAG_RECORD is TRUE we must record a new
# message...

if { "$vbox_flag_record" == "TRUE" } {

   set VBOX_NEW_MESSAGE "$vbox_var_spooldir/incoming/$vbox_var_savename"

   set RC [ vbox_get_message $VBOX_NEW_MESSAGE $vbox_var_rectime ]

   vbox_pause 1000

#   exec -- /usr/bin/vboxnotify.sh "$VBOX_NEW_MESSAGE" "$vbox_caller_id" "$vbox_caller_name" "$vbox_user_name" "$vbox_var_savename" "$vbox_var_spooldir" "$vbox_caller_phone"

   if { "$RC" == "HANGUP" } {
      return
   }

   if { "$RC" == "TIMEOUT" } {

      vbox_put_message $vbox_msg_timeout

      vbox_pause 500
   }
}
