#!/bin/sh
initlcd()
{
case $firmware in
  pollin) netecho -h $hostname -p 50290 -t "INITLCD" >/dev/null ;;
  ethersex)
  if [ -n "$userpass" ]
  then
    netecho -h $hostname -p 2701 -t "$userpass lcd reinit 0 0"
  else
    netecho -h $hostname -p 2701 -t "lcd reinit 0 0"
  fi
  ;;
esac
}

if [ -n "$1" ]
then
  hostname=$1
  n=0
  if [ -n "$2" ]
  then
    firmware=$2
    if [ -n "$3" -a -n "$4" ]
    then
      userpass="auth $3 $4"
    else
      userpass=""
    fi
  else
    firmware="pollin"
  fi
  initlcd
  sleep 5
  
  while true
  do
    n=`expr $n + 1`
    if [ $n -gt 59 ]
    then
      n=0
      initlcd
    fi
    case $firmware in
      pollin)
	netecho -h $hostname -p 50290 -t "WRITELCD 1." -a "/dev/vcs9:0:16" >/dev/null
	netecho -h $hostname -p 50290 -t "WRITELCD 2." -a "/dev/vcs9:80:16" >/dev/null
	;;
      ethersex)
	if [ -n "$userpass" ]
	then
	  netecho -h $hostname -p 2701 -t "$userpass lcd goto 0 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "$userpass lcd write " -a "/dev/vcs9:0:20" >/dev/null
	  netecho -h $hostname -p 2701 -t "$userpass lcd goto 1 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "$userpass lcd write " -a "/dev/vcs9:80:20" >/dev/null
	  netecho -h $hostname -p 2701 -t "$userpass lcd goto 2 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "$userpass lcd write " -a "/dev/vcs9:160:20" >/dev/null
	  netecho -h $hostname -p 2701 -t "$userpass lcd goto 3 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "$userpass lcd write " -a "/dev/vcs9:240:20" >/dev/null
	else
	  netecho -h $hostname -p 2701 -t "lcd goto 0 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "lcd write " -a "/dev/vcs9:0:20" >/dev/null
	  netecho -h $hostname -p 2701 -t "lcd goto 1 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "lcd write " -a "/dev/vcs9:80:20" >/dev/null
	  netecho -h $hostname -p 2701 -t "lcd goto 2 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "lcd write " -a "/dev/vcs9:160:20" >/dev/null
	  netecho -h $hostname -p 2701 -t "lcd goto 3 0" >/dev/null
	  netecho -h $hostname -p 2701 -t "lcd write " -a "/dev/vcs9:240:20" >/dev/null
	fi
	;;
    esac
    netecho.real -w	# Wait for next second arriving.
    # Use the real netecho tool because waiting needs no locking.
  done
fi
