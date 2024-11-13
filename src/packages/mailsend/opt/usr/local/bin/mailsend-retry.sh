#!/bin/sh
#------------------------------------------------------------------------------
# /usr/local/bin/mailsend-retry.sh                          __FLI4LVER__
#
# Creation:     2015-12-21  cspiess
# Last Update:  $Id$
#------------------------------------------------------------------------------

. /usr/local/bin/mailsend-helper.sh

#----------------------------------------------------
# Welche Dateien liegen vor ?
#----------------------------------------------------
for _file in `ls ${MAILSEND_SPOOL_DIR} | grep '^.*retry[0-9]\+$'`
do
	mailsend-send.sh "${MAILSEND_SPOOL_DIR}/${_file}"
	
	if [ -e "${MAILSEND_SPOOL_DIR}/${_file}" ]
	then
	    # Versand gescheitert, neuen Namen ermitteln
	    _try=`echo "${_file}" | sed 's/^\(.*\.retry\)\(.*\)$/\2/g' | sed 's/[^0-9]//g'`
	    _base=`echo "${_file}" | sed 's/^\(.*\.retry\)\(.*\)$/\1/g'`
	    _next=$((_try + 1))
	    mv "${MAILSEND_SPOOL_DIR}/${_file}" "${MAILSEND_SPOOL_DIR}/${_base}${_next}"
	fi
done
