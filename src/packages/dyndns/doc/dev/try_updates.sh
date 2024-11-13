http_update ()
{
    for port in $provider_port; do
        {
            echo "GET $provider_url HTTP/1.1"
            echo "Host: $provider_host"
            if [ "$provider_auth" ]; then
                echo "Authorization: Basic $provider_auth"
            fi
            echo "User-Agent: foo/bar foobar@null.bogus"
            echo ""
        } | netcat -w "$dyn_timeout" "$provider_host" "$port"
        if [ $? -eq 0 ]; then
            return 0
        fi
    done
    return 1
}

host=foo.bar
ip=1.2.3.4
user=foo
pass=bar
dyn_timeout=30
dir=opt/etc/dyndns

if [ ! -d $dir ]; then
    echo "Please invoke $0 from the dyndns base packet dir as doc/dev/try_updates.sh"
    exit 1
fi

mkdir -p $dir/prot

for i in $dir/*[A-Z]; do 
    if [ -f $i ] && \
        ! grep -q -e  '^[^#].*_protocol' -e '^[^#].*_success' \
        -e '^provider_update_method.*dip' $i || grep -q "^provider_success='no_check'" $i; then 

        log=$dir/prot/${i#*\.}
        echo -e "\n#\n# $i\n#"
        . $i
        echo "running update"
        http_update > $log
	if [ -x doc/dev/decode_chunked ] && grep -q '^Transfer-Encoding: chunked' $log; then
            mv $log $log.orig
            doc/dev/decode_chunked < $log.orig > $log
	fi
        cat $log
    fi
done
