#!/bin/sh
# call with 'faxrcv-mail.sh <index> <fax file>'
. /var/run/faxrcv.conf

# setup local variables
idx=$1
eval dir='$FAXRCV_'$idx'_DIRECTORY'
eval temp='$FAXRCV_'$idx'_TEMP'
eval mail_account='$FAXRCV_'$idx'_MAIL_ACCOUNT'
eval mail_attach='$FAXRCV_'$idx'_MAIL_ATTACH'
eval mail_to='$FAXRCV_'$idx'_MAIL_TO'
file=$(basename "$2")
name=`echo "$file" | cut -d '.' -f 1`
path="$dir/$file"
sff="$temp/$name.sff"
tiff="$temp/$name.tiff"
pdf="$temp/$name.pdf"

from=`echo $name | cut -c 14-`
pages=`echo $name | cut -c 3-5 | sed 's/^[0]*//'`

# create temporary directory
mkdir -p $temp

case ${mail_attach} in
    sff|tiff|pdf)
        if echo $path | grep -qe "\.gz$"
        then
            gzip -cd "$dir/$file" > "$sff"
        else
            cp "$dir/$file" "$temp/"
        fi
        ;;
esac

case ${mail_attach} in
    sff)
        attach="-a $sff"
        ;;
    tiff)
        sfftobmp -t -q "$sff" -o "$tiff"
        attach="-a $tiff"
        ;;
    pdf)
        sfftobmp -t -q "$sff" -o "$tiff"
        tiff2pdf -o "$pdf" "$tiff"
        attach="-a $pdf"
        ;;
esac

# build and send mail
subject="Fax from $from"
message="Fax with $pages pages from $from received"
mailsend -A "${mail_account}" -t "${mail_to}" -s "${subject}"\
  ${attach} "${message}"

# cleanup temporary directory
rm -f $temp/$name.*
