vbox_init_touchtones

set seq_next "*1#"
set seq_prev "*2#"
set seq_quit "*9#"

vbox_breaklist rem all
vbox_breaklist add $seq_next
vbox_breaklist add $seq_prev
vbox_breaklist add $seq_quit

set USER [ exec /usr/local/bin/vboxinfouser2user $vbox_user_name ]

set DOEXIT "0"
set TOGGLED "0"
for { set ID "1" } { "$DOEXIT" == "0" } { incr ID } {
	for { set SUBID 1 } { "$DOEXIT" == "0" } { incr SUBID } {
		set FILE "$vbox_var_spooldir/incoming/$ID-$SUBID.msg"
		if { [ file exists "$FILE" ] } {
			set RC [ vbox_put_message "$FILE" ]
			if { "$RC" == "HANGUP" } {
				set DOEXIT "1"
				break
			}
			if { "$RC" == $seq_next } {
				break
			}
			if { "$RC" == $seq_prev } {
				incr ID -1
				if { "$ID" != "0" } {
					incr ID -1
				}
				break
			}
			if { "$RC" == $seq_quit } {
				set DOEXIT "1"
				break
			}

			set ORIGFILE [ file readlink "$FILE" ]
			if { [ string match "*/incoming/*" "$ORIGFILE" ] } {
				set CREATED [ exec /usr/local/bin/vboxgetmsgfield2 "$ORIGFILE" "M" ]
				if { "$CREATED" != "0" } {
					exec /usr/local/bin/vboxtogglemsg "$ORIGFILE"
				}
			}
		} else {
			if { "$SUBID" == "1" } {
				set DOEXIT "1"
			}
			break
		}
	}
	vbox_pause 2000
}
