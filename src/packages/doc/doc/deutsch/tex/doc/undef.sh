#! /bin/sh
if [ ! -f doc.log ]
then
	echo "missing doc.log, please run latex first"
	echo "    make realclean ; make doc.tex; make ps"
	exit 1
fi
grep -i "reference.*undefined" doc.log |\
	awk '{ print $4}' | sed -e 's/[^[:alpha:]:-]//g' |\
	while read var 
	do 
		echo "Undefined: $var"
		grep -w $var *.tex
	done | tee undef.log |\
		sed -e 's/.*jump{.*{\([\\_[:alpha:]]*\)}.*/\1/g ; s/\\//g ; s/^Undefined: [A-Z].*//g; s/^Undefined: \(.*\)/Undefined Label: \1/g' |\
			sort -u
