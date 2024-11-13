#!/bin/bash

# ConvertTableCSV2HTML.bash
# Fri Mar 12 14:45:18 PST 2010
# originally based on the script from
# Chieh Cheng
# http://www.CynosureX.com/

# GNU General Public License (GPL) Version 2, June 1991

  scriptDir=`dirname "$0"`
  scriptName=`basename "$0"`

function process ()
{
  file="$1"

  if [ -f "${file}" ]
  then
    printf "<table rules=\"all\" border=\"1\">"
    printf "<tr bgcolor=\"#B8B8B8\">"
    for kernel in $(head -n 1 ${file})
    do
      kernel=${kernel##*/}
      printf "<td valign="middle" align="center">${kernel}</td>"
    done
    printf "<td valign="middle" align="center">Bus</td><td valign="middle" align="center">Driver</td><td valign="middle" align="center">Explanation</td></tr>"
    while read line
    do
      line=`echo ${line} | sed 's/#/<\/td><td valign="middle" align="center">/g' | sed -e 's/x</\&#10004;</' | sed -e 's/>x</>\&#10004;</g'`
      printf "<tr><td valign="middle" align="center">${line}</td></tr>"
      printf "<!-- Kommentar -->"
    done < <(sed 1d ${file})
    printf "</table>"
    echo
  else
    # send error to stderr
    echo "${file} does not exist!" >&2
  fi
}

function usage ()
{
  echo "  Usage:   ${scriptName} \"file\""
  echo
  echo "           file is the file to be processed"
  echo
}

  if [ $# -gt 0 ]
  then
    process "$@"
  else
    usage
  fi 
