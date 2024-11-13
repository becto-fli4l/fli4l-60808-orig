#!/bin/sh

# read config
. /var/run/dsltool.conf

# check index argument
if [ "$#" -lt 1 ]
then
  echo "Syntax: $0 <index> ..."
  exit 1
fi

if [ "$1" -lt 1 -o "$1" -gt "$DSLTOOL_N" ]
then
  echo "invalid index, must be in range of 1 .. $DSLTOOL_N"
  exit 1
fi

# index
_IDX=$1
shift

# optional arguments
_OPTARG="$*"
 
eval _MODEM='$DSLTOOL_'$_IDX'_MODEM'
eval _PROTOCOL='$DSLTOOL_'$_IDX'_PROTOCOL'
eval _PORT='$DSLTOOL_'$_IDX'_PORT'
eval _ETHTYPE='$DSLTOOL_'$_IDX'_ETHTYPE'
eval _HOST='$DSLTOOL_'$_IDX'_HOST'
eval _USER='$DSLTOOL_'$_IDX'_USER'
eval _PASS='$DSLTOOL_'$_IDX'_PASS'
eval _LOG='$DSLTOOL_'$_IDX'_LOG'
eval _SYSLOG='$DSLTOOL_'$_IDX'_SYSLOG'
 
# set port argument
if [ "x$_PORT" = "x" ]
then
    _PORTARG=""
else
    _PORTARG="-N$_PORT"
fi

# select ethertype argument
_ETHARG=""
case $_ETHTYPE in
    IPv4)
        _ETHARG="-4"
        ;;
    IPv6)
        _ETHARG="-6"
        ;;
esac

# select logfile
_LOGARG=""
if [ "$_LOG" = "yes" ]
then
    if [ "$_SYSLOG" = "yes" ]
    then
        _LOGARG="-l#"
    else
         _LOGARG="-l"
    fi
fi

# dsltool(d) arguments
_ARGS="-m$_MODEM -C$_PROTOCOL $_PORTARG $_ETHARG -H$_HOST -U$_USER -P$_PASS $_LOGARG $_OPTARG"

# dsltool outputdirectory
_DIR="-f/srv/www/img"