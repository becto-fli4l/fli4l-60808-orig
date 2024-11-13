#!/bin/sh
#------------------------------------------------------------------------------
# /usr/sbin/makekbl.sh - make a keyboard layout map                __FLI4LVER__
#
# Creation:     02.11.2005  arwin
# Last Update:  $Id$
#------------------------------------------------------------------------------

set +x
echo -e "\nMake a keyboard layout map"
echo ==========================

echo -e "The following groups of layouts are available:\n"
ls /usr/share/keymaps/i386 | grep -v "include"
echo -en "\nEnter the name of the desired group: "
read group

if [ ! -d "/usr/share/keymaps/i386/$group" ]
then
    echo "Error: The group '$group' does not exist"
    exit
fi

if [ "$group" = "" ]
then
    echo "Error: No group entered"
    exit
fi

echo -e "\nThe following keyboard layouts are available in the chosen group:\n"
cd /usr/share/keymaps/i386/"$group"
ls -C *.map.gz | sed "s/\.map\.gz/       /g"

echo -en "\nEnter the name of the desired layout: "
read mapname

if [ ! -f "/usr/share/keymaps/i386/$group/$mapname.map.gz" ]
then
    echo "Error: The map '$mapname' does not exist"
    exit
fi

loadkeys /usr/share/keymaps/i386/"$group"/"$mapname".map.gz 2>/dev/null
dumpkmap > /tmp/"$mapname".map
echo -e "\nYour new keyboard layout map '$mapname.map' is now available in /tmp.\n"
