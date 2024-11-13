#!/bin/sh
#------------------------------------------------------------------------------
# /usr/local/bin/mailsend-helper.sh                         __FLI4LVER__
#
# Creation:     2015-12-21  cspiess
# Last Update:  $Id$
#------------------------------------------------------------------------------

# read config variables
. /var/run/mailsend.conf

# log (string)
log ()
{
    local _text=$1
    local _date=`date`

    case "${MAILSEND_LOGFILE}"
    in
        "")
            ;;
        syslog)
            logger "mailsend: ${_text}"
            ;;
        *)
            echo "${_date} mailsend: ${_text}" >> ${MAILSEND_LOGFILE}
            ;;
    esac
    
    return 1
}

# get_acount (name)
get_account ()
{
    _index=1
    _name=$1
    for n in `seq 1 ${MAILSEND_N}`
    do
        eval _account='${MAILSEND_'$n'_ACCOUNT}'
        if [ "$_account" == "$_name" ] 
        then
            _index=$n
        fi
    done
    
    echo $_index
}