#!/bin/bash

# colors used
if [ "$fbr_dumbtty" != yes ]
then
	standouton=$(tput smso)
	standoutoff=$(tput rmso)
	yellowon=$(tput setaf 3)$(tput bold)
	yellowoff=$(tput sgr0)
	redon=$(tput setaf 1)$(tput bold)
	redoff=$(tput sgr0)
else
	standouton=
	standoutoff=
	yellowon="*** "
	yellowoff=" ***"
	redon="!!! "
	redoff=" !!!"
fi

# Prints a message.
# $1 = message
# $2 = attribute (can be empty)
# $3 = additional echo options (can be empty)
message()
{
	local on=
	local off=
	if [ -n "$2" ]
	then
		eval on=\"\$$2on\"
		eval off=\"\$$2off\"
	fi
	echo $3 "$on$1$off" >&2
}

# Prints a qualified message.
# $1 = message
# $2 = attribute (can be empty)
q_message()
{
	message "$progname: $1" $2
}

# Prints an error message and aborts script execution.
# $1 = message
# $2 = exit code
error()
{
	q_message "$1" red
	[ -n "$2" ] && exit $2 || exit 1
}

# Prints a warning message.
# $1 = message
warning()
{
	q_message "$1" yellow
}

#
# functions for spinner
#

numspinnerstates=4
spinnerstate0='|'
spinnerstate1='/'
spinnerstate2='-'
spinnerstate3='\'

# Resets the spinner.
spinner_init()
{
	spinnerstate=0
}

# Lets the spinner rotate.
spinner_show()
{
	if [ "$fbr_dumbtty" != yes ]
	then
		eval local ch=\${spinnerstate${spinnerstate}}
		message "\033[s\033[K $ch \033[u" "" "-n -e"
		spinnerstate=$(((spinnerstate+1)%numspinnerstates))
	else
		message "." "" "-n"
	fi
}

# Removes the spinner and prints "done.".
spinner_done()
{
	if [ "$fbr_dumbtty" != yes ]
	then
		message "\033[K done." "" "-e"
	else
		message " done."
	fi
}

spinner_init

#
# functions for percentage display
#

# Shows a percentage value. If the TTY is dumb, only a dot is displayed.
# $1 = index
# $2 = maximum value (may not be zero)
percentage_show()
{
	local value=$(($1*100/$2))
	if [ "$fbr_dumbtty" != yes ]
	then
		message "\033[s\033[K $value%\033[u" "" "-n -e"
	else
		message "." "" "-n"
	fi
}

# Removes the percentage value and prints "done.".
percentage_done()
{
	spinner_done
}
