get_cert_names ()
{
    cat <<EOF
Equifax Secure CA
Equifax Secure Global eBusiness CA
GlobalSign Root CA
GlobalSign Root CA - R2
Go Daddy Class 2 CA
Thawte Premium Server CA
Thawte Server CA
EOF
}

extract_cert ()
{
    local dump=
    local end=
    while read line; do
        case $line in
            $1) dump=yes ;;
            '-----END CERTIFICATE-----') [ "$dump" ] && end=yes ;;
        esac
        [ "$dump" ] && echo "$line"
        [ "$end" ] && return
    done < ca-bundle.crt
}

get_cert ()
{
    REMHOST=$1
    REMPORT=${2:-443}

    echo |\
        openssl s_client -connect ${REMHOST}:${REMPORT} 2>&1 |\
        sed -ne '/-BEGIN CERTIFICATE-/,/-END CERTIFICATE-/p'
}

usage ()
{
    echo "`basename $0` [ -h | --help ] [ -d <dyndns dir> ]"
    echo "   Updates the curl bundle for the dyndns packet using wget, openssl and perl"
    exit
}
exec_dir=`dirname $0`

if ! which openssl > /dev/null; then
    echo "Missing openssl"
    usage
fi
    
while [ "$1" ]; do
    case "$1" in
	-h | --help)
	    usage
	    ;;
	*)  
	    [ "$dyndns" ] && usage
	    dyndns="$1"
	    ;;
    esac
done

if [ ! "$dyndns" ]; then
    for dyndns in dyndns . ../.. ; do
        [ -d $dyndns/opt/files/usr/share/curl ] && break
    done
fi
if [ ! -d "$dyndns/opt/files/usr/share/curl" ]; then
    echo "Invalid dyndns dir, unable to locate certificate directory '$dyndns/opt/files/usr/share/curl'"
    usage
fi

dir=$dyndns/opt/files/usr/share/curl
echo "Updating certificate bundle in '$dir'"

set -e
echo " -> Retriving mozilla certificates" 
perl $exec_dir/mk-ca-bundle.pl

cat > fli4l-ca-bundle.crt <<EOF
##
## Bundle of CA Root Certificates for fli4l opt_dyndns
##
## Converted at: `date +%c`
## using http://curl.haxx.se/lxr/source/lib/mk-ca-bundle.pl 
## and openssl http://www.madboa.com/geek/openssl/
##
IN-Berlin Root CA (G2) - Individual Network Berlin e.V.
=======================================================
EOF

echo " -> retrieving certificate from www.in-berlin.de"
wget -q -O - http://ca.in-berlin.de/cgi-bin/rootca-G3 |\
    openssl x509 -inform DER > in-berlin.crt
fp=`openssl x509 -noout -fingerprint < in-berlin.crt | sed -e 's/.*=//'`
ib_fp='D9:85:D2:4B:AB:E5:34:A3:CC:0B:2B:D0:23:47:24:D9:AB:13:89:19'
if [ "$fp" = "$ib_fp" ]; then
    cat in-berlin.crt >> fli4l-ca-bundle.crt
else
    echo "Wrong fingerprint for in-berlin certificate, aborting"
    echo "Expected: $ib_fp"
    echo "Got:      $fp"

    exit 1
fi

# get_cert www.in-berlin.de >> fli4l-ca-bundle.crt

echo " -> extracting needed certificates from mozilla bundle"
get_cert_names | while read cert; do 
    extract_cert "$cert"
done >> fli4l-ca-bundle.crt

rm -f ca-bundle.crt certdata.txt
echo " -> moving certificates to $dir/ca-bundle.crt"
mv fli4l-ca-bundle.crt $dir/ca-bundle.crt
