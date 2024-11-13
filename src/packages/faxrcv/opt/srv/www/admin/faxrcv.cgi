#!/bin/sh
#-----------------------------------------------------------------------------
#                                                                 __FLI4LVER__
# /srv/www/admin/faxrcv.cgi - display received faxes from OPT_FAXRCV
#
# Creation:     28.09.2003  felix
# Last Update:  $Id$
#-----------------------------------------------------------------------------

# get main helper functions
. /srv/www/include/cgi-helper

# some necessary language variables
_faxrcvtitle="${_FAXRCV_TITLE}"
_showfaxes="${_FAXRCV_SHOW}"
_showcontrol="${_FAXRCV_CONTROL}"

# This script can be called with the following parameters
# ----------------------------------------------------------------------------
# Parameter    | Value                    | Meaning
# ----------------------------------------------------------------------------
# action       | showfaxes                | Show faxes in the tab "item"
#              | showcontrol              | Show Deamon-Control in the tab "item"
#              | deletefax                | Delete the fax in "item"
#              | viewfax                  | Display fax in "item" in browser
#              | downloadfax              | Download fax in "item"
#              | start, stop              | Start or stop the capifaxrcvd
# item         | depening on action       | The item "action" should process
# page         | number                   | Used in conjunction with viewfax
#              |                          | Returns given page as JPG
# number       | number                   | selects capifaxrcvd with specified number
# message      | string                   | some status message that is shown
#              |                          | on the generated page

## sanitize the input
FORM_action=`echo "$FORM_action"   | sed "s/[^a-zA-Z]//g"`
FORM_item=`echo "$FORM_item"       | sed "s/[^-_+.a-zA-Z0-9]//g"`
FORM_page=`echo "$FORM_page"       | sed "s/[^0-9]//g"`
FORM_number=`echo "$FORM_number"   | sed "s/[^0-9]//g"`

SEC_REALM='isdnfax'
case $FORM_action in
    deletefax) SEC_ACTION='delete' ;;
    start|stop) SEC_ACTION='startstop' ;;
    *) SEC_ACTION='view' ;;
esac

# Security
check_rights 'isdnfax' "$SEC_ACTION"

## get some internal variables
. /var/run/faxrcv.conf

: ${FORM_number:="1"}
: ${FORM_action:="showfaxes"}
eval FAXDIR='$FAXRCV_'$FORM_number'_DIRECTORY'
eval TMPDIR='$FAXRCV_'$FORM_number'_TEMP'
TMPNAME="fax"

MFAX_LOG="$FAXDIR""/mfax.log"
MFAX_TMP="$FAXDIR""/mfax.tmp"


## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
## show_page_header: output opening html-statements
## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#
# background: this function outputs not only html-headers such as title but
#     but also the selection box for the fax number and a user message
#     requested through http-get if any.
show_page_header ()
{
    show_html_header "$_faxrcvtitle"
    show_tab_header "" no
    cat <<EOF
      <form name="num_select" action="$myname">
        Faxnummer:
        <input type="hidden" name="action" value="$FORM_action">
        <input type="hidden" name="item" value="$FORM_item">
        <select name="number" class="inputs" onchange="JavaScript:submit()">
          <option disabled>FaxMSN &nbsp; &nbsp; &nbsp;Status Anz</option>
EOF
    FAX_STATUS_ALL=`ps | grep capifaxrcvd | grep -v grep`
    for NUMBER_IDX in `seq 1 $FAXRCV_N`
    do
        FAX_STATUS="$( [ "`echo "$FAX_STATUS_ALL" | grep -e "-l \`eval echo '$FAXRCV_'$NUMBER_IDX'_NUMBER'\`"`" ] && echo 'Ein' || echo 'Aus' )"
        FAX_COUNT="(`grep -c "" \`eval echo '$FAXRCV_'$NUMBER_IDX'_DIRECTORY'\`/mfax.log`)"
        echo "    <option `[ "$NUMBER_IDX" = "$FORM_number" ] && echo "selected " `value=\"$NUMBER_IDX\">\
        `echo \`eval echo '$FAXRCV_'$NUMBER_IDX'_NUMBER'\`........... | cut -c-12 | sed "s/\./\&nbsp;/g"`>$FAX_STATUS< &nbsp;$FAX_COUNT</option>"
    done
    echo "  </select>&nbsp;&nbsp;<input type=submit value=\" Zeige \"></form>"

    show_tab_footer

    if [ -n "$FORM_message" ]
    then
        echo "<br />"
        message_type=`echo "$FORM_message"   | cut -d"|" -f1 -s`
        message_header=`echo "$FORM_message" | cut -d"|" -f2 -s`
        message_body=`echo "$FORM_message"   | cut -d"|" -f3-`
        
        case $message_type in
            E) show_error "$message_header" "$message_body" ;;
            *) show_info  "$message_header" "$message_body" ;;
        esac
    fi
    
    echo '<br />'
    show_tab_header "$_showfaxes" "$myname?action=showfaxes&amp;number=$FORM_number" \
                    "$_showcontrol" "$myname?action=showcontrol&amp;number=$FORM_number"
}

## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
## show_page_footer: output closing html-statements on bottom of page
## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
show_page_footer ()
{
    show_tab_footer
    show_html_footer
}

## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
## ensure_compatibility: backwards compatibility with older version of faxrcv
## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# background: in earlier version of OPT_FAXRCV mfax.log always contained
#     something.sff in mfax.log even if the actual local file was compressed
#     using gzip (something.sff.gz). This behaviour changed as of 2.1.12 so
#     now we may have gzipped and non-gzipped files peacefully side by side.
#     this function may be used to make sure things dont break anyhow.
#
# @ params    filename      the filename to check
#             faxdir        where to look for the file (will _not_ be returned)
#                           may be empty
# @ returns   filename      if filename exists or filename.gz does not exists
#             filename.gz   otherwise (! -f filename && -f filename.gz)
ensure_compatibility ()
{
    filename="$1"
    faxdir="$2"
    
    case x"$faxdir" in
        x) fullname="$filename" ;;
        *) fullname="$faxdir/$filename" ;;
    esac
    
    if [ -n "$filename" -a ! -f "$fullname" -a -f "$fullname".gz ]
    then
        echo "$filename".gz
    else
        echo "$filename"
    fi
}

## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
## find_mfax_entry: return mfax-infoline for the respective fax file
## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# background: to preserve mgetty compatibility, faxrcv stores information on
#     the received files in mfax.log format. each line contains the following
#     information:
#       date_received time_received remote_stationid num_pages filename
#     this function returnes the matching lines from mfaxfile for the
#     respective faxfile. as filename used to be file.sff in earlier versions
#     regardless of gzip being used (in that case the actual filename was
#     file.sff.gz on the disk) we use some magic to allow for backwards
#     compatibility.
#
# @ params    mfaxfile      full path to the mfax.log file to read from
#             faxname       name of the filename on disk to look for
# @ returns   infoline      the full line as described above
find_mfax_entry ()
{
    mfaxfile="$1"
    faxname="$2"
    
    faxname_comp=`echo $faxname|sed 's/\.gz$//'`
    echo `grep "\($faxname\|$faxname_comp\)$" "$mfaxfile"`
}

## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
## delete_mfax_entry: deletes mfax entry for specified file
## ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# @ params    mfaxfile      full path to the mfax.log file to read from
#             faxname       name of the filename on disk to look for
# @ returns   0             if deletions have been made
#             1             if the entry for faxname could not be found or
#                           an error occurred
delete_mfax_entry ()
{
    mfaxfile="$1"
    mfaxfile_temp="`dirname "$mfaxfile"`/mfax.tmp"
    faxname="$2"
    faxname_sed=`echo "$faxname" | sed 's/\./\\\./g'` # escape "." for sed
    #    (its the only char we need to escape - see sanity-check above)
    
    # return if we dont have such an entry
    grep -qe " $faxname\$" "$mfaxfile" || return 1
    # remove the line corresponding with the deleted file
    sed "/ $faxname_sed\$/d" "$mfaxfile" > "$mfaxfile_temp"
    # replace mfax.log
    mv -f "$mfaxfile_temp" "$mfaxfile" || return 1
}

faxblink_off ()
{
    if [ -f /usr/local/bin/faxrcv_setleds ]
    then 
        /usr/local/bin/faxrcv_setleds off > /dev/null 2>&1
    fi
}

# Main stuff
case "$FORM_action" in

showfaxes)
### SHOWFAXES #################################################################
    case "$FORM_item" in
        "") MENU_SELECTED="Faxe" ;;
        *) MENU_SELECTED="$FORM_item" ;;
    esac

    show_page_header

    if [ -s "$MFAX_LOG" ]
    then
        cat <<EOF
<table class="normtable">
  <tr>
    <th>${_FAXRCV_TAB_DATE}</th>
    <th>${_FAXRCV_TAB_FAXID}</th>
    <th>${_FAXRCV_TAB_PAGES}</th>
    <th>${_FAXRCV_TAB_FILE}</th>
    <th>${_FAXRCV_TAB_ACTIONS}</th>
  </tr>
EOF
        sed '1!G;h;$!d' "$MFAX_LOG" | while read d z i p f
        do
            # + is not allowed in QUERY_STRING but in filenames
            qf=`echo "$f" | sed "s/\+/%2B/g"`
            # "_" is a replacement for " "
            # i=`echo "$i" | sed "s/_/\&nbsp;/g"`
            cat <<EOF
<tr class="listitem$ITERATE">
  <td align="center">$d/$z</td>
  <td><b>$i</b></td>
  <td align="center">$p</td>
  <td>$f</td>
  <td align="center">
    <a href="$myname?action=viewfax&amp;number=$FORM_number&amp;item=$qf">
      <img src="/img/fax_view.gif" border="0" width="21" height="16" alt="${_FAXRCV_ACT_VIEW}" title="${_FAXRCV_ACT_VIEW}">
    </a>
        <a href="$myname?action=downloadfax&amp;number=$FORM_number&amp;item=$qf">
      <img src="/img/fax_download.gif" border="0" width="21" height="16" alt="${_FAXRCV_ACT_DOWNLOAD}" title="${_FAXRCV_ACT_DOWNLOAD}">
    </a>
    <a href="$myname?action=deletefax&amp;number=$FORM_number&amp;item=$qf" onclick="return confirm(${_FAXRCV_CONF_DELETE});">
      <img src="/img/fax_delete.gif" border="0" width="21" height="16" alt="${_FAXRCV_ACT_DELETE}" title="${_FAXRCV_ACT_DELETE}">
    </a>
  </td>
</tr>
EOF

            case "$ITERATE" in
                "") ITERATE="2" ;;
                *) ITERATE="" ;;
            esac
        done
        echo "</table>"
    else
        echo "${_FAXRCV_NOFAX}"
    fi
    show_page_footer
;;

showcontrol)
### SHOWCONTROL ###############################################################
    show_page_header

    cat <<EOF
<table class="normtable">
  <tr><th>
EOF
    echo "${_FAXRCV_RECEIVE} `eval echo '$FAXRCV_'$FORM_number'_NUMBER'` ${_FAXRCV_IS} "

    if ps | grep capifaxrcvd | grep -v grep | grep -qe "-l `eval echo '$FAXRCV_'$FORM_number'_NUMBER'`"
    then
        echo "<span class=\"faxrcvon\">${_FAXRCV_ON}</span>"
    else
        echo "<span class=\"faxrcvoff\">${_FAXRCV_OFF}</span>"
    fi
    cat <<EOF
    </th>
  </tr>
  <tr>
    <td align="center">
      [<a href="$myname?action=start&amp;number=$FORM_number">${_FAXRCV_ENABLE}</a>]&nbsp;
      [<a href="$myname?action=stop&amp;number=$FORM_number">${_FAXRCV_DISABLE}</a>]
    </td>
  </tr>
</table>
EOF
    show_page_footer
;;

deletefax)
### DELETEFAX #################################################################
    faxblink_off
    # only go ahead if there was an entry in mfax.log that we could delete
    # in the first place
    if delete_mfax_entry "$MFAX_LOG" "$FORM_item"
    then
        # if it exists, we must delete the entry from the
        # imonc-log as well
        delete_mfax_entry "$FAXRCV_IMONC_DIR/mfax.log" "$FAXDIR/$FORM_item"
        file_on_hd=`ensure_compatibility "$FORM_item" "$FAXDIR"`
        rm -f "$FAXDIR/$file_on_hd"
        if [ "$?" = 1 -a -f "$FAXDIR/$file_on_hd" ]
        then
            # loeschen war nicht erfolgreich und die datei existiert noch
            delete_error="${_FAXRCV_ERR_FILE} $FAXDIR/$file_on_hd ${_FAXRCV_ERR_NOTDELETED}"
        fi
    else
        delete_error="${_FAXRCV_ERR_FILE} $FAXDIR/$FORM_item ${_FAXRCV_ERR_DELNOTFOUND}"
    fi
    
    case $delete_error in
        "") message=`url_encode "I|${_FAXRCV_INF_DELETE}|${_FAXRCV_ERR_FILE} $FAXDIR/$FORM_item ${_FAXRCV_INF_FILEDELETED}"` ;;
        *)  message=`url_encode "E|${_FAXRCV_ERR_DELETE}|$delete_error"` ;;
    esac
    
    echo "Location: $myname?action=showfaxes&number=$FORM_number&message=$message"
    echo
;;

viewfax)
### VIEWFAX ###################################################################
    faxblink_off
    case $FORM_page in
    "")
        # convert the fax and
        # show the page where all the images are included
        mkdir -p $TMPDIR
        rm -f "$TMPDIR/$TMPNAME"*

        file_on_hd=`ensure_compatibility "$FORM_item" "$FAXDIR"`

        if [ -f "$FAXDIR/$file_on_hd" ]
        then
            show_html_header "$_faxrcvtitle"

            if echo $file_on_hd | grep -qe "\.gz$"
            then
                gzip -cd "$FAXDIR/$file_on_hd" > "$TMPDIR/$TMPNAME.sff" 2>/dev/console || show_error "${_FAXRCV_ERR_UNPACK}" "${_FAXRCV_ERR_UNPACK_LONG}"
            else
                cp "$FAXDIR/$FORM_item" "$TMPDIR/$TMPNAME.sff" 2>/dev/console || show_error "${_FAXRCV_ERR_COPY}" "${_FAXRCV_ERR_COPY_LONG}"
            fi
            case "`echo $file_on_hd | cut -b1-2`" in
                fn) conv_option="-d" ;;
                *) conv_option="-j" ;;
            esac
                
            # creates $TMPDIR/$TMPNAME.001.jpg $TMPNAME.002.jpg etc.
            sfftobmp -j -q -n  "$TMPDIR/$TMPNAME.sff" -o "$TMPDIR/$TMPNAME.jpg" >/dev/console 2>&1 || show_error "${_FAXRCV_ERR_SFF2MISC}" "${_FAXRCV_ERR_SFF2MISC_LONG}"
            
            INFOLINE=`find_mfax_entry "$MFAX_LOG" "$file_on_hd"`
            case "$INFOLINE" in
                "") show_error "${_FAXRCV_ERR_NOTFOUND}" "${_FAXRCV_ERR_NOTFOUND_LONG}" ;;
                *)
                    # d z i p f
                    set -- $INFOLINE
                    echo "<h1>$1 ($2) - ${_FAXRCV_FAXFROM} $3</h1><h2>$file_on_hd</h2>"
                    for idx in `seq 1 "$4"`
                    do
                        echo "<h3>${_FAXRCV_PAGE} $idx</h3><br>"
                        cat <<EOF
<div class="faxpage">
  <a href="$myname?action=viewfax&amp;number=$FORM_number&amp;item=&amp;page=$idx" target="_blank">
    <img align="center" src="$myname?action=viewfax&amp;number=$FORM_number&amp;page=$idx" alt="Seite $idx" width="500" border="0">
  </a>
</div>
EOF
                    done
                ;;
            esac
            show_html_footer
        else
            # we dont have that file...
            echo "Location: $myname?action=showfaxes&number=$FORM_number&message=`url_encode "E|${_FAXRCV_ERR_FILENOTFOUND}|${_FAXRCV_ERR_FILE} ${FAXDIR}/${FORM_item} ${_FAXRCV_ERR_FILENOTEXIST}"`"
            echo
        fi
    ;;
    *)
        # return a specific page as JPG
        PADDED_PAGE=`echo "000$FORM_page" | sed "s/.*\([0-9]\{3\}\)/\1/"`
        echo "Content-Type: image/jpeg"
        echo
        cat "$TMPDIR/$TMPNAME.$PADDED_PAGE.jpg"
    ;;
    esac
;;

downloadfax)
### DOWNLOADFAX ###############################################################
    faxblink_off
    FORM_item=`ensure_compatibility "$FORM_item" "$FAXDIR"`
    filename=`echo "$FORM_item" | cut -d '.' -f 1`
    if [ -f "$FAXDIR/$FORM_item" ]
    then
        mkdir -p $TMPDIR
        rm -f "$TMPDIR/$TMPNAME"*
        case $FAXRCV_DOWNLOAD in
            sff)
                http_header download "ctype=application/download;filename=$filename.sff"
            ;;
            tiff)
                http_header download "ctype=image/tiff;filename=$filename.tif"
            ;;
            pdf)
                http_header download "ctype=application/pdf;filename=$filename.pdf"
            ;;
        esac
        if echo $FORM_item | grep -qe "\.gz$"
        then
            gzip -cd "$FAXDIR/$FORM_item" > "$TMPDIR/$TMPNAME.sff"
        else
            cp "$FAXDIR/$FORM_item" "$TMPDIR/$TMPNAME.sff"
        fi
        
        case $FAXRCV_DOWNLOAD in
            sff)
                cat "$TMPDIR/$TMPNAME.sff"
            ;;
            tiff)
                sfftobmp -t -q "$TMPDIR/$TMPNAME.sff" -o "$TMPDIR/$TMPNAME.tif" >/dev/console 2>&1
                cat "$TMPDIR/$TMPNAME.tif"
            ;;
            pdf)
                sfftobmp -t -q "$TMPDIR/$TMPNAME.sff" -o "$TMPDIR/$TMPNAME.tif" >/dev/console 2>&1
                tiff2pdf -o "$TMPDIR/$TMPNAME.pdf" "$TMPDIR/$TMPNAME.tif" >/dev/console 2>&1
                cat "$TMPDIR/$TMPNAME.pdf"
            ;;
        esac
    else
        echo "Location: $myname?action=showfaxes&number=$FORM_number&message=`url_encode "E|${_FAXRCV_ERR_FILENOTFOUND}|${_FAXRCV_ERR_FILE} ${FAXDIR}/${FORM_item} ${_FAXRCV_ERR_FILENOTEXIST}"`"
        echo
    fi
;;

start)
### START #####################################################################
    # remove ANSI colors from output
    result=`faxrcv.sh start $FORM_number`
    result=${result//[^a-z A-Z.0-9()]/} #remove Escape sequences from Var as they seem to fool sed
    result=`echo $result | sed "s|[0-9]*m||g"`
    case "$result" in
        "") result_message="";;
        *)  result_message="<pre>$result</pre>" ;;
    esac
    echo "Location: $myname?action=showcontrol&number=$FORM_number&message=`url_encode "$result_message"`"
    echo
;;

stop)
### STOP ######################################################################
    # remove ANSI colors from output
    result=`faxrcv.sh stop $FORM_number`
    result=${result//[^a-z A-Z.0-9()]/} #remove Escape sequences from Var as they seem to fool sed
    result=`echo $result | sed "s|[0-9]*m||g"`
    case "$result" in
        "") result_message="";;
        *)  result_message="<pre>$result</pre>" ;;
    esac
    echo "Location: $myname?action=showcontrol&number=$FORM_number&message=`url_encode "$result_message"`"
    echo
;;

*)
    echo "Location: $myname?action=showfaxes&number=$FORM_number&message=`url_encode "E|${_FAXRCV_ERR_INTERNAL}|${_FAXRCV_ERR_PARAM}"`"
    echo
;;

esac

