#! /bin/sh

if [ $# -ne 1 ]; then
    echo "usage: $0 <file>"
    exit 1
fi

last=HEAD
for i in `svn log $1 | grep ^r[0-9] | sed -e 's/r\([0-9]\{1,\}\).*/\1/'`; do 
    svn diff -r $i:$last  $1
    last=$i
done
