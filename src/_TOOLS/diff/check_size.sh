#! /bin/sh

if [ $# -ne 1 ]; then
    echo "usage: $0 <file>"
    exit 1
fi
last_size=0
last_rev=HEAD
for i in `svn log $1 | grep ^r[0-9] | sed -e 's/r\([0-9]\{1,\}\).*/\1/'`; do
	size=`svn cat -r $i $1 | wc -c`
	diff=`expr $last_size - $size`
	if [ $diff -ne 0 -a $last_size -ne 0 ]; then
		echo "Diff: $diff (orig: $size, new: $last_size)"
		svn log -r $last_rev
	fi
	last_size=$size
	last_rev=$i
done
