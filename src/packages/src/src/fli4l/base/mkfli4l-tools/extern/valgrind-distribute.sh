#!/bin/sh

if [ "$1" = "" ]
then
	report_dir="."
else
	report_dir="$1"
fi

header=1
first=1
testcase=0

> "$report_dir/valgrind-header.xml"

while read line
do
	[ -n "$line" ] || continue
	if [ "$header" -eq 1 ]
	then
		echo "$line" >> "$report_dir/valgrind-header.xml"
		echo "$line" | grep -q "^</status>" && header=0
		continue
	else
		if [ "$first" -eq 1 ]
		then
			testcase=$(expr $testcase + 1)
			cat "$report_dir/valgrind-header.xml" > "$report_dir/valgrind-$testcase.xml"
		fi
		echo "$line" >> "$report_dir/valgrind-$testcase.xml"
		first=0

		echo "$line" | grep -q "^</valgrindoutput>" && first=1
	fi
done < "$report_dir/valgrind.xml"

rm -f "$report_dir/valgrind.xml" "$report_dir/valgrind-header.xml"
