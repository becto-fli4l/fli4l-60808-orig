#!/bin/bash
## ---------------------------------------------------------------------------
## getDefaultConfigs.sh - Script to collect all default configurations
##
## Creation:     2013-02-24 starwarsfan
## Last Update:  $Id$
##
## Copyright (c) 2013 fli4l Developer Team <team@fli4l.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
## ---------------------------------------------------------------------------

## ---------------------------------------------------------------------------
## show help
showHelp() {
    bName=`basename $0`
    ewarn ""
    ewarn "=============================================================================="
    ewarn "                                                                              "
    ewarn " This script collects the default configurations of all activated opts and    "
    ewarn " copies them to a new folder on the same level as the folder with your        "
    ewarn " personal configurations. The new folder will have the same name extended     "
    ewarn " by '-default'. After that you can compare your configurations with the       "
    ewarn " default ones using some sort of diff tool                                    "
    ewarn "                                                                              "
    ewarn "Usage: $bName [-c <config-file>] [options]                                    "
    ewarn "                                                                              "
    ewarn "-c <config-file>\t config file, default: updatefli4l.cfg                      "
    ewarn "-u | -U <rev> \t rev is a specific revision or HEAD                           "
    ewarn "                                                                              "
    ewarn "Example: $bName -c router.cfg                                                 "
    ewarn "         $bName -c router.cfg -u       : update to HEAD                       "
    ewarn "         $bName -c router.cfg -U 12345 : update to r12345                     "
    ewarn "                                                                              "
}



## ---------------------------------------------------------------------------
## svn update
check_svn() {
    etrace "checking SVN state"

    ebeginn "\tlocal revision is : "
    localRev=`svn info $svnDir | grep ^[rR]evision | $SED "s/[rR]evision:\ //"`
    retval=$?
    echo $localRev
    eend $retval

    if [ $SVNUP -eq 1 ]; then
        ebeginn "\tremote revision is: "
        remoteRev=`svn status $svnDir -u | grep [rR]evision | $SED "s/.*[rR]evision:\ \ //"`
        retval=$?
        echo $remoteRev
        eend $retval

        if [ "$newRev" == "HEAD" ]; then
            newRev=$remoteRev
        fi
        ebegin "\tnew revision is   : "$newRev

        if  [ "$newRev" != "$localRev" ]; then
            eend 1 einfo "\t\tupdated required"
            etrace "running svn update"
            ebegin "\tsvn up -r $newRev "
            svn up -r $newRev $svnDir > /dev/null
#             localRev=`svn info $svnDir | grep ^[rR]evision | $SED "s/[rR]evision:\ //"`
            localRev=$newRev
            eend $? einfo "\tupdated to revision $newRev"
        else
            eend 1 einfo "\t\tno updated needed"
        fi
    fi

}



copyDefaultConfigs ()
{
    if [ ! -e ${configDir}-default ] ; then
        mkdir -p ${configDir}-default
    fi

    # Remove existing default configs
    rm -rf ${configDir}-default/*.txt

    # Copy default config for every existing modified config
    for currentConfig in $unpack ; do
        if [ -e $svnDir/${currentConfig}/config/${currentConfig}.txt ] ; then
            $CP $svnDir/${currentConfig}/config/${currentConfig}.txt ${configDir}-default/
        fi
    done
}



## ---------------------------------------------------------------------------
## main
echo -e "\033[2J"
echo -e "\033[H"
echo " *  fli4l - distribution build preparation script                                "
echo " * ------------------------------------------------------------------------------"
echo " *                                                                               "
echo " * running some prechecks                                                        "
echo " *      checking for functions.sh...                                             "

if [ -f ./functions.sh ]
then
    source ./functions.sh
    eend $?
else
    echo -e "\tmissing -> exiting"
    exit 1
fi

#edebug "script parameter : "$0 $1 $2 $3 $4 $5
eend $? echo " *                                                                            "

## ---------------------------------------------------------------------------
## clear variables
SVNUP=0

## ---------------------------------------------------------------------------
## read parameters
ebegin "\treading parameters..."
TEMP=`getopt -o 'uU:c:bsOph' -l 'help' -n ' * updatefli4l.sh' -- "$@"`

## ---------------------------------------------------------------------------
## exit if getopt returns an error
if [ $? != 0 ] ; then
eend 1
showHelp
eerror "Invalid parameter or option found... Terminating..."
exit 1 ; fi

## ---------------------------------------------------------------------------
## Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

pcnt=0

while true ; do
    case $1 in
        -h|--help)
            showHelp
            exit 0 ;;
        -u|-U)
pcnt=`expr $pcnt + 1`
            SVNUP=1
            ## ---------------------------------------------------------------
            ## check if parameter argument for -U is numeric overwise set
            ## revision to HAED
            newRev=`echo $2 | grep "^[0-9]*$"`
            if [ -z $newRev ]; then
                newRev=HEAD
            fi
            einfo "\t\tfound -u|-U will run 'svn up -r $newRev' "
            if [ "$1" == "-U" ]; then
                shift
            fi
            shift ;;
        -c)
pcnt=`expr $pcnt + 1`
            CFG=`echo $2 | grep '^[^-]'`
            einfo "\t\tfound -c will use '$CFG' as config file"
            shift 2 ;;
        --)
            shift ;
            break ;;
    esac
done
eendxy $pcnt $? einfo " "

## ---------------------------------------------------------------------------
## prüfen ob cfg angegeben wurde, wenn nein default verwenden
ebegin "\tchecking for configuration file '$CFG'"
if [ -z "$CFG" ]; then
    CFG=`echo $0 | $SED 's/.sh$//' | $SED 's/$/.cfg/'`
    einfo "no cfg given - using $CFG as default"
fi

## ---------------------------------------------------------------------------
## prüfen ob cfg existiert
if [ -f $CFG  ]; then
    source ./fli4lbase.cfg
    source $CFG
    eend $?
    client=`echo $CFG | $SED 's/.\///' | $SED 's/.cfg$//'`
    einfo "\t\tusing '$client' as client name"
else
    eend $? eerror "config-file $CFG does't exist"
    showUsage=1
    exit
fi

## ---------------------------------------------------------------------------
## prüfen ob $fli4lBaseDir existiert
ebegin "\tchecking for \$fli4lBaseDir: '$fli4lBaseDir'"
if [ -d $fli4lBaseDir ]; then
    eend $?
else
    eend $? #eerror '$svnDir     does not'
fi

## ---------------------------------------------------------------------------
## prüfen ob $svnDir existiert
ebegin "\tchecking for \$svnDir      : '$svnDir'"
if [ -d $svnDir ]; then
    eend $?
else
    eend $? #eerror '$svnDir     does not'
fi

## ---------------------------------------------------------------------------
## prüfen ob Errors aufgetreten sind
ebegin "prechecks finished"
if [ $failed -ne 0 ]; then
    eend $failed eerror " "
    eerror "ERRORs found, check the top most error first"
    eerror " "
    if [ $showUsage ]; then
            showHelp
    fi
    exit 1
fi
eend $? einfo " "

## ---------------------------------------------------------------------------
##  svn check, svn version ermitteln und evtl. ein svn update ausführen
check_svn

## ---------------------------------------------------------------------------
## build
etrace "collecting default configurations"

source ./fli4lbase.cfg
source $CFG

copyDefaultConfigs
eend $?

exit $?
