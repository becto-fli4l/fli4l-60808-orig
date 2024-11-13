#!/bin/sh

echo $$ > /var/run/dsltool.pid

. /usr/bin/dsltool-helper.sh

logger -t "dsltool [$$]" "dsltoold started"

dsltoold $_ARGS -r`hostname` 

logger -t "dsltool [$$]" "dsltoold terminated"
