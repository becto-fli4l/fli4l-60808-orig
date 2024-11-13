#!/bin/sh
# -------------------------------------------------------------------------
# functions to remove not needed files                         __FLI4LVER__
#
# Creation:       lanspezi 2004-10-25
# Last Update:    $Id$
# -------------------------------------------------------------------------

cleanup_fli4lfiles ()
{
    [ -d "$dir_image" ] && rm -rf "$dir_image"
    for f in fli4l.iso mkfli4l.log mkfli4l_build.log mkfli4l_error.log mkfli4l_error.flg
    do
        rm -f "$dir_build/$f"
    done
    rm -f "${dir_config}/etc/rc.cfg"
}

cleanup_mkfli4l ()
{
    make -C src clean
}
