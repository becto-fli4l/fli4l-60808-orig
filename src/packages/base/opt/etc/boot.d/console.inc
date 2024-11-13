#!/bin/sh
#----------------------------------------------------------------------------
# /etc/boot.d/console.inc
# Functions for printing to the console.
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

if [ "$console_api" != yes ]
then

etracen() {
    echo -ne " ${TRACE}*${NORMAL} $*"
    return 0
}

etrace() {
    etracen "$*\n"
    return 0
}

edebug() {
    if [ $DEBUG -eq 1 ]; then
        echo -e " ${DBUG}*${NORMAL} $*"
    return 0
    fi
}

eprefixedinfo() {
    echo -ne " ${INFO}*${NORMAL} ${DBUG}$1${NORMAL} "
    shift
    echo -e "$*"
    return 0
}

einfon() {
    echo -ne " ${INFO}*${NORMAL} $*"
    return 0
}

einfo() {
    einfon "$*\n"
    return 0
}

ewarnn() {
    echo -ne " ${WARN}*${NORMAL} $*"
    return 0
}

ewarn() {
    ewarnn "$*\n"
    return 0
}

eerrorn() {
    echo -ne " ${BAD}*${NORMAL} $*"
    return 0
}

eerror() {
    eerrorn "$*\n"
    return 0
}

ebeginn() {
    echo -ne " ${GOOD}*${NORMAL} $*"
    return 0
}

ebegin() {
    ebeginn "$*\n"
    return 0
}

eynbn() {
    echo -ne " ${DBUG}*${NORMAL} $* [${BAD}YES${NORMAL}/${GOOD}no${NORMAL}/backup] "
}

eynn() {
    echo -ne " ${DBUG}*${NORMAL} $* [${BAD}YES${NORMAL}/${GOOD}no${NORMAL}] "
}

eYnn() {
    echo -ne " ${DBUG}*${NORMAL} $* [${GOOD}YES${NORMAL}/${BAD}no${NORMAL}] "
}

eendxy() {
    cnt=$1
    tput cuu $cnt
    shift
    eend $1
    tput cud $cnt
    shift

    efunc=${1:-eerror}
    shift
    ${efunc} "$*"
}

eend() {
    local retval=${1:-0} efunc=${2:-eerror} msg
    shift; shift

    if [[ ${retval} == 0 ]]; then
        msg="${BRACKET}[ ${GOOD}OK${BRACKET} ]${NORMAL}"
        echo -e "${ENDCOL}  ${msg}"
        if [[ -n "$*" ]]; then
            ${efunc} "$*"
        fi
    else
        if [[ $efunc == "ewarn" ]]; then
            msg="${BRACKET}[ ${WARN}!!${BRACKET} ]${NORMAL}"
            warned=1
        elif [[ $efunc == "einfo" ]]; then
            msg="${BRACKET}[ ${GOOD}OK${BRACKET} ]${NORMAL}"
        else
            msg="${BRACKET}[ ${BAD}!!${BRACKET} ]${NORMAL}"
            failed=$retval
        fi
        echo -e "${ENDCOL}  ${msg}"
        if [[ -n "$*" ]]; then
            ${efunc} "$*"
        fi
    fi

    return 0
}

failed=0
warned=0

COLS=$(stty size 2>/dev/null | cut -d' ' -f2)
# use a sensible default when stty does not yield a meaningful value
: ${COLS:=80}
ENDCOL=$'\e[A\e['$(( COLS - 9 ))'C'

TRACE=$'\e[34;01m'              # blue bold
DBUG=$'\e[36;01m'               # bold cyan
INFO=$'\e[32;01m'               # blue green
WARN=$'\e[33;01m'               # bold yellow
BAD=$'\e[31;01m'                # bold red

GOOD=$'\e[32;01m'               # bold green
NORMAL=$'\e[0m'                 #
BRACKET=$'\e[34;01m'            # blue bold

DEBUG=0

console_api='yes'

fi # $console_api != yes
