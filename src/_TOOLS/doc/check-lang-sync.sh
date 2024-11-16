#!/bin/bash

dirtarball=$1

cat << !!!
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
       "http://www.w3.org/TR/html4/loose.dtd">
<head
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
</head>
<html>
<title>fli4l - state of syncronisation for documentation</title>
<style type="text/css">
<!--
body {
padding:0px;
margin:0px;
}
table tr td{
border-style: solid;
border-width: 1px;
border-color: #cccccc;
border-spacing: 0px;
font-size:14px;
padding:0px;
margin:0px;
}
-->
</style>
<body>
!!!
if [ -d ${dirtarball}/ ]
then
   echo "<h3>list of translation detail information files:</h3>"
   for transf in ${dirtarball}/translation*.html
   do
       filef=`basename $transf`
       if [ "$filef" != "translation_syncstat.html" ]
       then
           echo "&nbsp;&nbsp;<a href=\"$filef\">$filef</a><br />"
       fi
   done
fi
echo "<h3>sync state for documentation tex-files - created: `date` </h3>"
echo "<table>"
echo "<tr><th>section</th><th>package</th><th>TEX-file</th><th>DE<br>rev of last change</th><th>FR<br>Sync-TAG</th><th>EN<br>Sync-TAG</th></tr>"

for packdir in src/packages/*
do
    packname=`basename $packdir`
    packcat="<b>N.A.</b>"
    for cat in distri non_gpl opt_db development internal obsolet
    do
        if [ "$cat" = "distri" ]
        then
             conffile=src/_RELEASE/release.conf
        else
             conffile=src/_RELEASE/release-${cat}.conf
        fi
        if grep -q $packname $conffile
        then
            packcat=$cat
        fi
    done
    if [ -d $packdir/doc/deutsch/ ]
    then
        if [ "$packname" != "doc" ]
        then        
            if [ ! -f $packdir/doc/deutsch/tex/$packname/$packname.tex ]
            then
                echo "<tr><td>$packcat</td><td>$packname</td><td colspan=\"4\"><b><font color=\"ff0000\">no TEX-documentation files found</b></td></tr>"
            fi
        fi
        for texfile_de in `find $packdir/doc/deutsch/ -name *.tex | sort`
        do
            linkneeded=0
            texfile=`basename $texfile_de`
            texfile_fr=`echo $texfile_de | sed -e 's/deutsch/french/'`
            texfile_en=`echo $texfile_de | sed -e 's/deutsch/english/'`
            rev_de=`svn info $texfile_de | grep "Rev:" | cut -d " " -f 4`
            if [ -f $texfile_fr ]
            then
                rev_fr=`sed -n '/^% Sync/p' $texfile_fr`
                if [ "x$rev_fr" = "x" ]
                then
                    rev_fr="no Sync Tag"
                fi
                case $rev_fr in
                    *$rev_de*) rev_fr="<font color=\"#00c000\">$rev_fr" ;;
                    *) rev_fr="<font color=\"#ff0000\">$rev_fr"; linkneeded=1 ;;
                 esac
            else
                rev_fr="no tex file"
            fi
            if [ -f $texfile_en ]
            then
                rev_en=`sed -n '/^% Sync/p' $texfile_en`
                if [ "x$rev_en" = "x" ]
                then
                    rev_en="no Sync Tag"
                fi
                case $rev_en in
                    *$rev_de*) rev_en="<font color=\"#00c000\">$rev_en" ;;
                    *) rev_en="<font color=\"#ff0000\">$rev_en"; linkneeded=1 ;;
                 esac
            else
                rev_en="no tex file"
            fi
            if [ "$linkneeded" = "1" ]
            then
                if [ "$packcat" != "distri" ]
                then
                    echo "<tr><td><b><a href=\"${packcat}/translation.html\">${packcat}</a></b></td><td>$packname</td><td>$texfile</td><td>$rev_de</td><td>$rev_fr</td><td>$rev_en</td></tr>"
                else
                    echo "<tr><td><b><a href=\"translation.html\">${packcat}</a></b></td><td>$packname</td><td>$texfile</td><td>$rev_de</td><td>$rev_fr</td><td>$rev_en</td></tr>"
                fi
            else
                echo "<tr><td>${packcat}</td><td>$packname</td><td>$texfile</td><td>$rev_de</td><td>$rev_fr</td><td>$rev_en</td></tr>"
            fi
        done
        echo "<tr><td colspan=\"6\"></td></tr>"
    else
        case $packname in
            kernel*) ;;
            *) 
                 echo "<tr><td>$packcat</td><td>$packname</td><td colspan=\"4\"><b><font color=\"#ff0000\">no directory for german documentation</b></td></tr>"
                 echo "<tr><td colspan=\"6\"></td></tr>"
                 ;;
        esac
    fi
done
echo "</table>"
echo "</body></html>"
