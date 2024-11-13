#!/bin/sh
#------------------------------------------------------------------------------
# /usr/local/bin/ow-user-script.sh                                 __FLI4LVER__
#
# Creation:     04.02.2009 kmw <news4kmw@web.de>
# Modification: 22.01.2012 Roland Franke
# Last Update:  $Id$
#------------------------------------------------------------------------------

# This BASH script is used to hold your input and output routines, therefore
# your process plan. It is run in background with intervals of one second
# between loops to give OWFS some time to update it's cache.

# Load configuration
. /var/run/ow.conf

###############################
####### COMMON ROUTINES #######
###############################
# There is a set of routines available, you can use to access OWFS contents.

# OWFS_running () -- See if OWFS is currently executed
# ----------------------------------------------------
# Syntax:  $(OWFS_running)
# Returns: not empty string if running, otherwise nothing
OWFS_running ()
  {
  if [ -s "$OW_OWFS_PID_FILE" ]
  then
    ps | grep "^[ ]*$(cat $OW_OWFS_PID_FILE).*$(echo $OW_OWFS_BINARY | sed 's|.*/||;s| .*||') .*"
  fi
  }

# OWFS_read ()  -- Read file from OWFS
# --------------------------------------------
# Syntax:  $(OWFS_read {DeviceID}/{FileName})
# Example: $(OWFS_read 29.57D305000000/PIO.3)
# Returns: content of given file, if it exists
OWFS_read ()
  {
  local filename=$1
  #-----------------
  if [ -s "$OW_OWFS_PATH/$filename" ]
  then
    cat $OW_OWFS_PATH/$filename
  fi
  }

# OWFS_write () -- Write to file in OWFS
# --------------------------------------------
# Syntax:  $(OWFS_write {DeviceID}/{FileName} {Value})
# Example: $(OWFS_write 29.57D305000000/PIO.3 1)
# Returns: 'ok', if successful
OWFS_write ()
  {
  local filename=$1
  local value=$2
  #-----------------
  if [ -f "$OW_OWFS_PATH/$filename" -a -w "$OW_OWFS_PATH/$filename" ]
  then
    echo -n $value > $OW_OWFS_PATH/$filename
    echo 'ok'
  fi
  }

# OWFS_analog () -- Convert float to integer
# --------------------------------------------
# Syntax:  $(OWFS_analog {Value} [{Decimals}])
# Example: $(OWFS_analog 12.95756 2) results 1296
# Returns: Rounded product of {Value} times 10 to the power of {Decimals}
OWFS_analog ()
  {
  local value=$1
  local decimals=$2
  #-----------------
  : ${value:=0}
  : ${decimals:=0}
  decimals=$(expr $decimals + 1)
  expr $(echo $value | sed '/\./ !{s|$|0|} /\./ {s|$|00000|;s|\.\(.\{'$decimals'\}\).*|\1 + 5|'}) | sed 's|.$||'
  }

# OWFS_timer () -- Set or get timer
# --------------------------------------------
# Syntax:  $(OWFS_timer {Name} [{Interval}])
# Example: $(OWFS_timer MyTimer 15) -> sets MyTimer to 15 seconds.
#          $(OWFS_timer MyTimer) -> returns 'yes' if timer expired or nothing.
# Returns: Sets a timer with the given name and interval or returns 'yes', if
#          no interval is given and the timer has expired.
# {Name} can be any alphanumeric string, but mustn't contain slashes (/) and
# blanks ( ).
OWFS_timer ()
  {
  local id=$1
  local seconds=$2
  #-----------------
  : ${OW_TIMER_FILE_BASE:='/tmp/ow-timer'}
  if [ -n "$seconds" ]
  then
    expr $seconds + `date +%s` 1> $OW_TIMER_FILE_BASE.$id
  else
    if [ -f $OW_TIMER_FILE_BASE.$id ]
    then
      if [ `cat $OW_TIMER_FILE_BASE.$id` -le `date +%s` ]
      then
        rm $OW_TIMER_FILE_BASE.$id
        echo 'yes'
      fi
    else
      echo 'yes'
    fi
  fi
  }

###############################
########## ALIASES ############
###############################
# To enhance readability, you can define aliases for signals (e.g. ports) or
# even state settings. Make sure they are defined 'local'!

local Temperature=20.2AD304000000/temp
local Heater_state=29.57D305000000/PIO.3
local Heater_on="$Heater_state 1"
local Heater_off="$Heater_state 0"

###############################
### MAIN SCRIPT STARTS HERE ###
###############################

# Check if OWFS is activated
if [ -n "$(OWFS_running)" ]
then
  # Read Temperature and see if it is less than 21.5 units (e.g. Centigrade)
  if [ $(OWFS_analog $(OWFS_read $Temperature) 1) -lt 215 ]
  then
    # Chech if heater wasn't on for 300 seconds
    if [ "$(OWFS_read Heater_state)" = 0 -a -n "$(OWFS_timer Heater)" ]
    then
      # Temperature too low, so turn on the heater
      OWFS_write $Heater_on  > /dev/null
    fi
  else
    # Check if heater is on
    if [ "$(OWFS_read Heater_state)" = 1 ]
    then
      # Temperature cosy, so turn off the heater
      if [ "$(OWFS_write $Heater_off)" = "ok" ]
      then
        # Start hold-off timer
        OWFS_timer Heater 300
      else
        # Emergency message, if turning off the heater fails
        logger -s -p local1.err -t OWFS-UserScript "EMERGENCY: Heater cannot be turned off!"
      fi
    fi
  fi
fi
