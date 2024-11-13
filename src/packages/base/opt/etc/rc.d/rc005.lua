#!/bin/sh
#----------------------------------------------------------------------------
# /etc/rc.d/rc005.lua - Setup of Lua environment
#
# Last Update:  $Id$
#----------------------------------------------------------------------------

begin_script RC005_LUA "setting up Lua environment..."

cat >>/var/run/lua-environment <<EOF
export LUA_PATH="/usr/share/lua/${LUA_VERSION}/?.lua;/usr/share/lua/${LUA_VERSION}/?/init.lua"
export LUA_CPATH="/usr/lib/lua/${LUA_VERSION}/?.so"
EOF

. /var/run/lua-environment
ln -s /var/run/lua-environment /etc/profile.d/lua-paths

end_script
