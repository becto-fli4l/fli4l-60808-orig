#!/bin/bash
##----------------------------------------------------------------------------
## updatefli4l.sh - script to build a new fli4l distribution
##
## !!! be carefull the script isn't fail save !!!
##
## Creation:     22.03.2006  frudolph
##
## Copyright (c) 2013 Frank Rudolph <fli4l@fr-net.de>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##----------------------------------------------------------------------------

##----------------------------------------------------------------------------
## show help
##----------------------------------------------------------------------------
showHelp() {
    bName=`basename $0`
    ewarn ""
    ewarn "=============================================================================="
    ewarn "                                                                              "
    ewarn "Usage: $bName [-c <config-file>] [options]                                    "
    ewarn "                                                                              "
    ewarn "-c <config-file>\t config file, default: updatefli4l.cfg                      "
    ewarn "-u | -U <rev> \t rev is a specific revision or HEAD                           "
    ewarn "-b \t\t\t run mkfli4l                                                         "
    ewarn "-s \t\t\t create <hostname>-latest symlink, only when -b is also set          "
    ewarn "-O \t\t\t OVERWRITE an existing build                                         "
    ewarn "-p \t\t\t run patch (files in : $fli4lBaseDir/patches/patches.$client)        "
    ewarn "                                                                              "
    ewarn " --kernel \t\t update Kernel Version in base.txt to latest version            "
    ewarn " --copyconfig \t copy missing config/*.txt to config-dir                      "
    ewarn " --clean \t\t cleanup build directory                                         "
    ewarn "                                                                              "
    ewarn "Example: $bName -c router.cfg                                                 "
    ewarn "         $bName -c router.cfg -u         : will update to HEAD                "
    ewarn "         $bName -c router.cfg -U 12345   : will update to r12345              "
    ewarn "                                                                              "
}

##----------------------------------------------------------------------------
## copy all opt form opt db into the current fli4l distribution
##----------------------------------------------------------------------------
run_unpackOpt() {
    einfo "\tunpacking opt-files from opt-db"

    if [ -d $optfilesDir ] # check for existing directory
    then
        for item in $optfilesDir*
        do
        type=`file -L $item | cut -d " " -f 2`
        edebug "\t\t$item : type `file -L $item `"

        case $type
        in
            [Zz][Ii][Pp])
                ebegin "\t\t[unzip  ] $item"
                    unzip -qod $tarballDir $item
                eend $?
                ;;
            bzip2)
                # this a little evil code,
                # file type is bzip2, normally it shoud be a tbz file
                # so just untar/ungzip file
                ebegin "\t\t[tbz    ] $item"
                    ewarn "\t\t\tthis may be a tar.bz2/bgz2 file"
                    tar -xjf $item -C $tarballDir
                eend $?
                ;;
            gzip)
                # this a little evil code,
                # file type is gzip, normally it shoud be a tgz file
                # so just untar/ungzip file
                ebegin "\t\t[tgz    ] $item"
                    ewarn "\t\t\tthis may be a tar.gz/tgz file"
                    tar -xzf $item -C $tarballDir
                eend $?
                ;;
            directory)
                ebegin "\t\t[copy   ] $item"
                    cd $item
                    find . -not -type d -exec $CP --parents '{}' $tarballDir \;
                    cd - >> /dev/null
                eend $?
                ;;
            *) ewarn "\t\tunknown format $item";;
        esac
        done
    else
        eerror "\t\t$optfilesDir does not exist"
    fi
}

##----------------------------------------------------------------------------
## add a paket to the latest/given fli4l distribution (not implemented yet)
##----------------------------------------------------------------------------
add_to_distrib() {
    cd $svnDir/src

    einfo "\t\t$OPTARG"
    ./mktarball packages/$OPTARG $tarballDir $rev >/dev/null
    eend $?

    cd - >/dev/null
}

##----------------------------------------------------------------------------
## build the configured fli4l distribution
##----------------------------------------------------------------------------
run_mktarball() {
    cd $svnDir/src
    
    einfo "\tcopying 'base'-packages"
    if [ -n "$unpack" ]; then
        for i in $unpack
        do
            ebegin "\t\t[tarball] $i"
            if [ -d packages/$i ] ; then # check for existing directory
                ./mktarball packages/$i $tarballDir $rev >/dev/null
                eend $?
            else
                eend 1 ewarn "\t\t[package] ^ does not exist"
            fi
        done
        echo "$rev" > $tarballDir/version.txt
        echo "$arch" > $tarballDir/arch.txt
    else
        eend 1 eerror "\t\tno packages selected, as minimum 'base' is needed"
    fi

    if [ -n "$contrib" ]; then
        einfo "\tcopying contrib-files"
        for i in $contrib
        do
            ebegin "\t\t[tarball] $i"
            if [ -d packages/$i ] ; then # check for existing directory
                ./mktarball packages/$i $tarballDir $rev >/dev/null
                eend $?
            else
                eend 1 ewarn #"\t\t$i does not exist"
            fi
        done
    fi

    if [ -n "$avm" ]; then
        einfo "\tcopying avm-drivers"
        for i in $avm
        do
            ebegin "\t\t[tarball] $i"
            if [ -d packages/$i ] ; then # check for existing directory
                ./mktarball packages/$i $tarballDir $rev >/dev/null
                eend $?
            else
                eend 1 ewarn #"\t\t$i does not exist"
            fi
        done
    fi

    cd - >/dev/null
}

##----------------------------------------------------------------------------
## link the configuration files to the current build
##----------------------------------------------------------------------------
link_config() {
    einfo "\tlink in config dir"
    ebegin "\t\t[mv     ] $tarballDir/config -> $tarballDir/config.orig"
    mv $tarballDir/config $tarballDir/config.orig
    eend $?

    ebegin "\t\t[ln -s  ] $configDir -> $tarballDir/config"
    ln -s $configDir $tarballDir/config
    eend $?
    ebegin "\t\t[rm     ] $tarballDir/config/kernel*.txt"
    rm $tarballDir/config/kernel*.txt
    eend $?
    ebegin "\t\t[cp     ] $tarballDir/config.orig/kernel*.txt -> $tarballDir/config"
    cp $tarballDir/config.orig/kernel*.txt $tarballDir/config
    eend $?
    ebegin "\t\t[rm     ] $tarballDir/config/base_nic_kernel_*.list"
    rm $tarballDir/config/base_nic_kernel_*.list
    eend $?
    ebegin "\t\t[cp     ] $tarballDir/config.orig/base_nic_kernel_*.list -> $tarballDir/config"
    cp $tarballDir/config.orig/base_nic_kernel_*.list $tarballDir/config
    eend $?

}

unlink_config() {
    rm $tarballDir/config
    mv $tarballDir/config.orig $tarballDir/config
}

##----------------------------------------------------------------------------
##
##----------------------------------------------------------------------------
update_config() {
    einfo "\tupdating config (copy new config files)"
    cd  $tarballDir
    $CP -Rvn ./config.orig/* $configDir | $SED "s/\(.*\)/`einfo "\t\t\[copy   \] "`\1/"
    eend $?
    cd - >/dev/null
}

##----------------------------------------------------------------------------
##
##----------------------------------------------------------------------------
run_cleanup() {
    einfo "\tcleanup old build directory"
    BUILDDIR=
    eval $(grep BUILDDIR $configDir/mkfli4l.conf 2>/dev/null)
    eval $(grep BUILDDIR $configDir/mkfli4l.txt 2>/dev/null)
    builddir=$tarballDir/${BUILDDIR:-config/build}
    einfo "\t\t"$builddir

    if [ "$(ls -A $builddir)" ]; then
        rm $builddir/*
    fi
    eend $?
}

##----------------------------------------------------------------------------
##
##----------------------------------------------------------------------------
run_kernel() {
    einfo "\tapplying new kernel version to base.txt"
    KERNELVERS=$(ls ${tarballDir}/opt/lib/modules)
    OLDPS3=$PS3
    PS3="select Kernel Version: "
    select KERNELVER in ${KERNELVERS}; do
        if [ "x" != "x$KERNELVER" ]; then
            ebegin "\t\tnew kernel version is : "$KERNELVER
            $SED -i "s/^\(KERNEL_VERSION=\)'.*'/\1'${KERNELVER}'/" ${tarballDir}/config/base.txt
            eend $?
        fi
        break
    done
    PS3=$OLDPS3
}

##----------------------------------------------------------------------------
## extracts the variable $src ($3) from a file $dat ($1) into
## the variable $var ($2)
##----------------------------------------------------------------------------
extract_value () {
    dat=$1
    var=$2
    src=$3
    cond_val="$4"
    true_val="$5"
    false_val="$6"

    val=`$SED -n -e "s/^[[:space:]]*$src='\(.*\)'.*/\1/p" "$configDir/$dat"`
    case x$val in
        x) return ;;
    esac
    case  x$cond_val in
        x)    eval "$var='$val'" ;;
        *)    case  $cond_val in
                $val) eval "$var='$true_val'"  ;;
                *)    eval "$var='$false_val'" ;;
            esac
            ;;
    esac
}

##----------------------------------------------------------------------------
## svn update
##----------------------------------------------------------------------------
check_svn() {
    einfo "checking SVN state"

    remoteUrl=$(svn info --xml $svnDir 2>/dev/null | xmllint --xpath 'string(/info/entry/url)' - 2>/dev/null)
    ebeginn "\tURL used          : "
    echo "$remoteUrl"
    [ -n "$remoteUrl" ]
    eend $?

    ebeginn "\tlocal revision is : "
    localRev=$(svnversion -c $svnDir)
    localRev=${localRev##*:}
    echo $localRev
    [ -n "$localRev" ]
    eend $?

    if [ $SVNUP -eq 1 ]; then
        ebeginn "\tremote revision is: "
        remoteRev=$(svn log -l 1 --xml "$remoteUrl" 2>/dev/null | xmllint --xpath 'string(/log/logentry/@revision)' - 2>/dev/null)
        echo $remoteRev
        [ -n "$remoteRev" ]
        eend $?

        if [ "$newRev" == "HEAD" ]; then
            newRev=$remoteRev
        fi
        ebegin "\tnew revision is   : "$newRev

        if  [ "$newRev" != "${localRev%[A-Z]}" ]; then
            eend 0 ewarn "\t\tupdated required"
            einfo "running svn update"
            ebegin "\tsvn up -r $newRev "
            svn up -r $newRev $svnDir > /dev/null
            retval=$?
            localRev=$(svnversion -c $svnDir)
            localRev=${localRev##*:}
            newRev=$localRev
            eend $? einfo "\tupdated to revision $newRev"
        else
            eend 0 ewarn "\t\tno updated needed"
        fi
    fi

}

##----------------------------------------------------------------------------
## run_patch()    | patch files
##----------------------------------------------------------------------------
run_patch() {
        #echo
        einfo "starting patch (patch -p0 << <patchfile>)"

        cd $tarballDir
        echo  > patchlog
        for i in `ls  $fli4lBaseDir/patches/patches.$client/*.patch`
        do
            ebegin "\tapply patch $i"
            patch -l -p0 < $i >> patchlog
            eend $?
        done
#        eend 0 einfo "finished - patch"
}

##----------------------------------------------------------------------------
## run_mkfli4l()    | build fli4l distribution
##----------------------------------------------------------------------------
run_mkfli4l() {
    einfo "starting build "
    cd $tarballDir
    sleep 2
    ./mkfli4l.sh
    if [ "$?" != "0" ]; then
        eend 1 eerror "error during build"
        exit 1
    fi

    einfo "finished - build of fli4l $client-$rev\n"

    ##--------------------------------------------------------------------
    ## create <hostname>-latest SYMLINK
    if [ $SYMLINK -eq 1 ]; then
        extract_value base.txt remotehostname HOSTNAME
        einfo " creating SYMLINK "$client"-latest"
        if [ -e $fli4lBaseDir/$client-latest ]; then
            rm $fli4lBaseDir/$client-latest
        fi
        if [ -e $fli4lBaseDir/$client/latest ]; then
            rm $fli4lBaseDir/$client/latest
        fi
        ln -s $tarballDir $fli4lBaseDir/$client-latest
        ln -s $tarballDir $fli4lBaseDir/$client/latest
        eend $?
    fi
}



##----------------------------------------------------------------------------
## main
##----------------------------------------------------------------------------
echo -e "\033[2J"
echo -e "\033[H"
echo " *  fli4l - distrib build script                                                 "
echo " * ------------------------------------------------------------------------------"
echo " *                                                                               "
echo " * running some prechecks                                                        "
echo " *      checking for functions.sh...                                             "

if [ -f ./functions.sh ]
then
    source ./functions.sh
    eend $?
else
    echo -e "\tfunctions.sh is missing -> exiting"
    exit 1
fi
eend $? echo " *                                                                            "

edebug "script parameter : "$0 $*

##----------------------------------------------------------------------------
## clear variables
SVNUP=0
BUILD=0
SYMLINK=0
OVERWRITE=0
DOPATCH=0
DOKERNEL=0
DOCONFIGCOPY=0
DOCLEAN=0

##----------------------------------------------------------------------------
## for later use of $SED (MacOS vs. Linux sed)
type -p gsed >/dev/null && SED=gsed || SED=sed
## and for cp also
type -p gcp >/dev/null && CP=gcp || CP=cp

if ! type xmllint >/dev/null 2>&1
then
    eend 1 eerror "This program needs 'xmllint', often provided by the 'libxml2-utils' package."
    exit 1
fi
if ! type tput >/dev/null 2>&1
then
    eend 1 eerror "This program needs 'tput', often provided by the 'ncurses' package."
    exit 1
fi

##----------------------------------------------------------------------------
## default parameter
##DEFAULTPARAMS="--kernel --copyconfig --clean -O"

##----------------------------------------------------------------------------
## read parameters
ebegin "\treading parameters..."
TEMP=`getopt -o 'uU:c:bsOph' -l 'help,copyconfig,kernel,clean' -n ' * updatefli4l.sh' -- $DEFAULTPARAMS "$@"`

##----------------------------------------------------------------------------
## exit if getopt returns an error
if [ $? != 0 ] ; then
    eend 1
    showHelp
    eerror "Invalid parameter or option found... Terminating..."
    exit 1
fi

##----------------------------------------------------------------------------
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
            ##----------------------------------------------------------------
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
#         a)  add=true
#            addpkg=$OPTARG
#            ;;
        -c)
pcnt=`expr $pcnt + 1`
            CFG=`echo $2 | grep '^[^-]'`
            einfo "\t\tfound -c will use '$CFG' as config file"
            shift 2 ;;
        -b)
pcnt=`expr $pcnt + 1`
            BUILD=1
            einfo "\t\tfound -b will run mkfli4l.sh afterwards"
            shift ;;
        -s)
pcnt=`expr $pcnt + 1`
            SYMLINK=1;
            einfo "\t\tfound -s will create ${CFG%*.*}-latest SYMLINK afterwards"
#            einfo "\t\tfound -s will create <hostname>-latest SYMLINK afterwards, only when -b is also set"
            shift ;;
        -O)
pcnt=`expr $pcnt + 1`
            OVERWRITE=1;
            einfo "\t\tfound -O will OVERWRITE existing build!"
            shift ;;
        -p)
pcnt=`expr $pcnt + 1`
            DOPATCH=1;
            einfo "\t\tfound -p will patch the build!"
            shift ;;
        --kernel)
pcnt=`expr $pcnt + 1`
            DOKERNEL=1;
            einfo "\t\tfound --kernel, setting Kernel Version in base.txt"
            shift ;;
        --copyconfig)
pcnt=`expr $pcnt + 1`
            DOCONFIGCOPY=1;
            einfo "\t\tfound --copyconfig, copying missing config files"
            shift ;;
        --clean)
pcnt=`expr $pcnt + 1`
            DOCLEAN=1;
            einfo "\t\tfound --clean, cleanup build directory"
            shift ;;
        --)
            shift ;
            break ;;
# should not happen
#       *)
#           eerror "\t\tfound invalid parameter -$OPTARG"
#        showUsage=1
#        eend 1
#        ;;
    esac
done
eendxy $pcnt $? einfo " "

##----------------------------------------------------------------------------
## check if given cfg exist, if not use udatefli4l.cfg
##----------------------------------------------------------------------------
    ebegin "\tchecking for configuration file '$CFG'"
    if [ -z "$CFG" ]; then
        CFG=`echo $0 | $SED 's/.sh$//' | $SED 's/$/.cfg/'`
        einfo "no cfg given - using $CFG as default"
    fi

    ##------------------------------------------------------------------------
    ## check if cfg-file exist, if not exit
    ##------------------------------------------------------------------------
    if [ -f $CFG  ]; then
        source ./fli4lbase.cfg
        source $CFG
        eend $?
        client=`echo $CFG | $SED 's/\/$//' | $SED 's/.cfg$//'`
        einfo "\t\tusing '$client' as client name"
    else
        eend $? eerror "config-file $CFG does't exist"
        showUsage=1
    fi

##----------------------------------------------------------------------------
## check if $fli4lBaseDir exist
##----------------------------------------------------------------------------
    ebegin "\tchecking for \$fli4lBaseDir: '$fli4lBaseDir'"
    if [ -d $fli4lBaseDir ]; then
        eend $?
    else
        eend $? #eerror '$svnDir     does not'
    fi

##----------------------------------------------------------------------------
## check if $svnDir exist
##----------------------------------------------------------------------------
    ebegin "\tchecking for \$svnDir      : '$svnDir'"
    if [ -d $svnDir ]; then
        eend $?
    else
        eend $? #eerror '$svnDir     does not'
    fi

##----------------------------------------------------------------------------
## check for errors
##----------------------------------------------------------------------------
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

##----------------------------------------------------------------------------
##  svn check, evaluate svn version and run svn update if necessary
##----------------------------------------------------------------------------
    check_svn

##----------------------------------------------------------------------------
## build process
##----------------------------------------------------------------------------
    einfo "building distribution"

    branch=$(LC_ALL=C svn info $svnDir | grep "^URL:" | $SED -e 's,^.*/\([^/]\+\)$,\1,')
    [ "$branch" = "trunk" ] && branch= || branch="-$branch"
    rev=`cat $svnDir/src/packages/base/version.txt`-r$localRev$branch
    einfo "\tupdate $client to $rev"
    einfo "\tArchitecture: $arch"
    source ./fli4lbase.cfg
    source $CFG

    ##------------------------------------------------------------------------
    ## clean old build
    if [ $DOCLEAN -eq 1 ]; then
        run_cleanup
    fi

    einfo "\tbuilding distrib in $tarballDir"
    ##------------------------------------------------------------------------
    ## check if $tarballDir allready exists
    if [ -d $tarballDir ]; then
        if [ $OVERWRITE -eq 1 ]; then
            answer=YES
        else
            eend 1 ewarn "$tarballDir already exists"
            eynbn "OVERWRITE? "
            read answer
        fi

        if [ "$answer" = "YES" ]; then
             rm -r $tarballDir
            warned=0
        elif [ "$answer" = "backup" ]; then
            mv $tarballDir "$tarballDir.`date +%Y%m%d%H%M`"
            warned=0
        elif [ "$answer" = "no" ]; then
            warned=0
            :
        else
            eend 1 eerror "unknown answer given"
        fi
    fi

    ##------------------------------------------------------------------------
    ## if tarballDir does not exist (no more)
    if [ ! -d $tarballDir ]; then
        mkdir -p $tarballDir
        export ARCH=$arch
        run_mktarball
        unset ARCH
        run_unpackOpt
        link_config
    else
        :
    fi

    ##------------------------------------------------------------------------
    ## copy missing config files
    if [ $DOCONFIGCOPY -eq 1 ]; then
        update_config
    fi

    ##------------------------------------------------------------------------
    ## setting kernel version
    ## TODO: check for more than one Kernel
    if [ $DOKERNEL -eq 1 ]; then
        run_kernel
    fi

    ##------------------------------------------------------------------------
    ## run patch
    if [ $DOPATCH -eq 1 ]; then
        run_patch
    fi

    ##------------------------------------------------------------------------
    ## check for errors during build and cancel also if $answer was 'no'
    einfo " "
    ebegin "checking for errors and warnings during build"
    if [ $failed -ne 0 ]; then
        eend $failed eerror " "
        eerror "ERRORs found, check the top most error first"
        eerror " "
        exit $failed
    elif [ $warned -ne 0 ]; then
        eend $warned ewarn " "
        ewarn "WARNINGs found, check the top most warning first"
        ewarn " "
        eYnn "CONTINUE? "
        IFS= read answer

        if [[ ( "/$answer/" = '//' || "/$answer/" = '/YES/' ) ]]; then
            :
        else
            exit 1
        fi
    else
        eend 0 einfo "finished - creation of fli4l $client-$rev\n"
    fi

    ##------------------------------------------------------------------------
    ## run mkfli4l.sh if parameter -b is set
    if [ $BUILD -eq 1 ]; then
      run_mkfli4l
    fi
exit $?
