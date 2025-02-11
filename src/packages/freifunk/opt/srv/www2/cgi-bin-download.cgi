#!/bin/sh
case $QUERY_STRING in
	g)
		MID=-g
		EXT=bin
	;;
	gs)
		MID=-gs
		EXT=bin
	;;
	gs40)
		MID=-gs40
		EXT=bin
	;;
	g3g)
		MID=-g3g
		EXT=bin
	;;
	a)
		MID=-all0277
		EXT=bin
	;;
	moto)
		MID=-moto
		EXT=bin
	;;
	se505)
		MID=-se505
		EXT=bin
	;;
	*)
		MID=
		EXT=trx
		QUERY_STRING=trx
	;;
esac
echo "Content-type: application/octet-stream"
echo "Content-Disposition: attachment; filename=\"openwrt$MID-freifunk-1.4.2-de.$EXT\""
echo
exec ./cgi-bin/freifunk-upload -$QUERY_STRING /dev/mtd/1
