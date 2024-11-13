#!/bin/sh

# Prepares modules.aliases to be used in a simple script like hw.cgi

KERNEL="kernel_5_4"
: ${ARCH:=x86}
FILE='hw_detect.sed'
KERNEL_VERSION=`sed -n "s/^. $ARCH::opt\/lib\/modules\/\([^/]\+\)\/modules.alias$/\1/p" ../../../../../../bin/$ARCH/packages/$KERNEL/files.txt`

cat ../../../../../../bin/$ARCH/lib/modules/$KERNEL_VERSION/modules.alias |\
  sort |\
  sed -n '/^alias /p' |\
  sed -n '/[a-z]\*\{0,1\}:/p' |\
  sed 's#/#\\/#g' |\
  sed 's/\[0-9a-0\]/[0-9]/' |\
  sed 's/\*/.\*/g' |\
  sed 's/alias /\//' |\
  sed 's/ /\/s\/$\/ /' |\
  sed 's/$/\/p/' |\
  sed 's/usb_libusual/usb_storage/' > $FILE

grep pci $FILE >$FILE.pci
grep usb $FILE >$FILE.usb
grep -v pci $FILE | grep -v usb >$FILE.other

rm $FILE
