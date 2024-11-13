#! /bin/sh

# [ "$dir" ] || dir=opt/etc/dyndns
: ${dir:=opt/etc/dyndns}
if [ ! -f "$dir"/provider.patterns ]; then
    echo "usage: $0 <path to provider files> # eg. $0 opt/etc/dyndns"
    exit 1
fi

for i in $dir/*[A-Z]; do 
    if [ -f $i ] && ! grep -q -e  '^[^#].*_protocol' -e '^[^#].*_success' \
        -e '^provider_update_method.*dip' $i || grep -q "^provider_success='no_check'" $i; then 
        echo $i; 
    fi
done
