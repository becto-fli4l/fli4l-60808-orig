#!/bin/bash
#
# Last Update:  $Id$
#------------------------------------------------------------------------------

lang=$1

# language constants

case $lang in
    deutsch) 
        _MDL_title="Links zu Einzel-Dokumentationen"
        _MDL_package="Paket"
        _MDL_format="Format"
        _MDL_base="Basis-Paket"
        _MDL_dev="Entwickler-Dokumentation"
        _MDL_src="Sourcen"
        _MDL_weblang=""
        ;;
    english)
        _MDL_title="Links to individual documentations"
        _MDL_package="Package"
        _MDL_format="Format"
        _MDL_base="Basic Package"
        _MDL_dev="Developer Documentation"
        _MDL_src="Sources"
        _MDL_weblang="en/"
        ;;
    french)
        _MDL_title="Liens sur les documentations individuelles"
        _MDL_package="Paquetage"
        _MDL_format="Format"
        _MDL_base="Paquetage Base"
        _MDL_dev="Documentation développeur"
        _MDL_src="Sources"
        _MDL_weblang="fr/"
        ;;
esac    

head ()
{
    cat <<EOF
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html> 
  <head>
    <title>$_MDL_title</title>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name="author" content="Fli4l-Team">
  </head>
<body bgcolor="#EEEEFF">
<a href="http://www.fli4l.de/$_MDL_weblang"><img src="logo/fli4l.gif" height="80" width="211" align="right" border="0" alt="Logo fli4l"></a>
<h2>$_MDL_title</h2>
<table width="100%">
<tr align="left">
    <th>$_MDL_package</th>
    <th colspan="2">$_MDL_format</th>
</tr>
<tr align="left">
    <th></th>
    <th>HTML</th>
    <th>PDF</th>
</tr>
EOF
}

tail ()
{
    cat<<EOF
</table>
</body> </html>
EOF
}

entry ()
{
    cat <<EOF
<tr align="left">
    <td>$1</td>
    <td><a href="doc/$lang/html/$2/index.html">$2/index.html</a></td>
    <td><a href="doc/$lang/pdf/$2.pdf">$2.pdf</a></td>
</tr>

EOF
}

base ()
{
    if [ ! "`grep doc/$lang/tex/base/Makefile packages/base/files.txt`" = "" ]
    then
        entry "$_MDL_base" base
    fi
    if [ ! "`grep doc/$lang/tex/dev/Makefile packages/base/files.txt`" = "" ]
    then
        entry "$_MDL_dev" dev
    fi
    if [ ! "`grep doc/$lang/tex/src/Makefile packages/src/files.txt`" = "" ]
    then
        entry "$_MDL_src" src
    fi
}

opt ()
{
    for f in $(ls -1 packages/*/files.txt | sort)
    do
        i=${f%/files.txt}
        grep -q "^. doc/$lang/tex/.*_main" $f || continue
        if [ -d $i/doc/$lang/tex ]
        then
            case $i in
                base|src|doc) ;;
                *)
                    if grep $i _RELEASE/release.conf > /dev/null
                    then
                        entry $i $i
                    fi
                    ;;
            esac
        fi
    done
}

head
base
opt
tail
