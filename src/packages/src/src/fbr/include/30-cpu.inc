#!/bin/bash

num_cores()
{
	echo $((1 + $(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)))
}

# set number of (logical) processor cores to use
if [ -n "$J" ]
then
	NJ=$(expr $J + 0 2>/dev/null)
	if [ $? -gt 1 ] || [ $NJ -lt 1 ]
	then
		warning "Number of cores ($J) invalid, setting to 1"
		J=1
	else
		J=$NJ
	fi
else
	J=$(num_cores)
fi

message "Using at most $J processor cores simultaneously for building" standout
