#! /bin/sh
if [ "$1" ]
then
	if [ -f $1 ]
	then
		. $1
	elif [ -d $1 ]
	then
		packages=$1
	else
		case $1 in 
			-h)
				echo "usage: $0                           - creates a diff for main packages"
				echo "       $0 package                   - creates a diff for specific package"
				echo "       $0 <release conf file>       - create a diff for packages specified in conf file"
				echo "       packages='package list' $0   - creates a diff for packages specified in \$packages"
				;;
			*)
				echo "unable to open '$1'"
				;;
		esac
		exit 1
	fi
else
	if [ ! "$packages" ]
	then
		. _RELEASE/release.conf
	fi
fi

for p in $packages
do
	for d in config check etc opt unix windows
	do
		if [ -d $p/$d ]
		then
			sh _TOOLS/diff/svndiff 3.4.0 $p/$d
		fi
	done
done
