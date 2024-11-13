#!/bin/sh
#------------------------------------------------------------------------------
# /usr/local/bin/mailsend-send.sh                           __FLI4LVER__
#
# Creation:     2015-12-21  cspiess
# Last Update:  $Id$
#------------------------------------------------------------------------------

# helper functions and config variables
. /usr/local/bin/mailsend-helper.sh

send ()
{
    _file=$1
    _account=`basename ${_file} | cut -d '-' -f 1`
    _n=$(get_account $_account)

    eval _auth='${MAILSEND_'$_n'_SMTP_AUTH}'
    eval _host='${MAILSEND_'$_n'_POP3_SERVER}'
    eval _port='${MAILSEND_'$_n'_POP3_PORT}'
    eval _user='${MAILSEND_'$_n'_POP3_USERNAME}'
    eval _pass='${MAILSEND_'$_n'_POP3_PASSWORD}'

    # POP3 before smtp 
    if [ "$_auth" = "pop" ]
    then
        echo -e "user ${_user}\r\npass ${_pass}\r\nquit\r" | netcat -i 1 ${host} ${_port} | grep -q ^ERR
        
        if [ "$?" = "0" ]
        then
           log "error on pop-before-smtp, check server, port and authentication data"
        fi
    fi

    # send message
    cat ${_file} | /usr/bin/msmtp -a ${_account} -C /etc/msmtp.conf --read-recipients --read-envelope-from

    _error=$?
    if [ $_error -ne 0 ]
    then
        # send error
        _try=`echo "${_file}" | sed 's/^\(.*\.retry\)\(.*\)$/\2/g' | sed 's/[^0-9]//g'`
        log "error sending mail ${_file} ($_error), attempt ${_try}"
        return $_error
    else
        # o.k, delete file
        log "mail ${_file} sent successfully"
        rm "${_file}"
        return 0
    fi
}

send $1
exit $?
