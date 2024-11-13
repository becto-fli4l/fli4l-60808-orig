#!/bin/sh
#----------------------------------------------------------------------------
# bootfiles.cgi
#
# Creation:	2007-02-27 lanspezi
# Last Update:  $Id$
#----------------------------------------------------------------------------
get_one_var()
{
    local var=$1
    local name=$2
    local file=$3
    eval $var=\"`sed -n -e "s/^FLI4L_$name='\(.*\)'/\1/p" $file`\"
}

show_file_info()
{
    local rc_cfg=$1
    local name=$2
    local file=$3

    if [ "$file" ]; then
	var_name=`echo $name | sed -e 's/\./_/g;s/2//' | tr a-z A-Z`
	get_one_var md5sum MD5_$var_name $rc_cfg
	case $file in
	    rc*) set `grep -v '^FLI4L_MD5_' $BOOT_DIR/$file | md5sum` ;;
            *)   set `md5sum $BOOT_DIR/$file` ;;
	esac
	real_md5sum=$1
	col_opt=
	if [ "$md5sum" -a "$md5sum" != "$real_md5sum" ]; then
	    col_opt="bgcolor=#FF3F00"
	fi
	cat <<EOF
<tr>
  <td align="center"><a href="$myname?file=$name">$name</a></td><td $col_opt>$md5sum</td><td $col_opt>$real_md5sum</td>
</tr>
EOF
    else
	cat <<EOF
<tr>
  <td align="center">$name not present</a></td><td></td>
</tr>
EOF
    fi

}

show_version_info ()
{
    bftype=$1
    rc_cfg=$2

    get_one_var ver     VERSION   $rc_cfg
    get_one_var date    BUILDDATE $rc_cfg
    get_one_var time    BUILDTIME $rc_cfg
    get_one_var cfg_dir BUILDDIR  $rc_cfg
    cat <<EOF
<tr>
  <td rowspan="$#">$bftype</td>
  <td>$ver</td>
  <td>$date</td>
  <td>$time</td>
</tr>
<tr>
  <th>$_BOOTFILES_files</th><th>md5 (rc.cfg, $cfg_dir)</th><th>md5 ($BOOT_DIR)</th>
</tr>
EOF

    shift 2
    while [ "$1" ]; do
	file=$1
	if [ ! -f $BOOT_DIR/$file ]; then
	    file=
	    case $1 in
		opt.img) [ -f $BOOT_DIR/opt.old ] && file=opt.old ;;
	    esac
	 fi
	 show_file_info $rc_cfg $1 $file
	 shift
    done
}

show_versions ()
{
	show_tab_header "$_BOOTFILES_version_info" no
	cat <<EOF
<table class="normtable">
<tr>
  <th></th><th>$_BOOTFILES_version</th><th>$_BOOTFILES_date</th><th>$_BOOTFILES_time</th>
</tr>
EOF
        show_version_info "$_BOOTFILES_std" /boot/rc.cfg $boot_files
        if [ -f /boot/kernel2 ]; then
            cat <<EOF
<tr>
  <th></th><th>$_BOOTFILES_version</th><th>$_BOOTFILES_date</th><th>$_BOOTFILES_time</th>
</tr>
EOF
	    show_version_info "$_BOOTFILES_rec" /boot/rc2.cfg $boot_files2
	fi
	echo "</table>"
	show_tab_footer
}

# get main helper functions
. /srv/www/include/cgi-helper

BOOT_DIR=/boot
boot_files='kernel rootfs.img opt.img rc.cfg syslinux.cfg'
boot_files2='kernel2 rootfs2.img opt2.img rc2.cfg'

check_rights 'support' 'systeminfo'

case $FORM_file in
    "")
        show_html_header "$_BOOTFILES_title"
	show_versions
        show_html_footer
    ;;
    *)
        file=
	name=
        for f in $boot_files $boot_files2; do
	    if [ "$FORM_file" == "$f" ]; then
		file=$f
	    fi
	done
	name=$file

        if [ ! -f $BOOT_DIR/$file ]
        then
	    case $file in
		opt.img) [ -f $BOOT_DIR/opt.old ] && file=opt.old ;;
	    esac
	fi

        if [ -f $BOOT_DIR/$file ]
        then
            ctype=application/download;filename=$name
            set -- `ls -l $BOOT_DIR/$file`
            length=$5
            echo "Content-Type: $ctype"
            echo "Content-Disposition: attachment; filename=$name"
            echo "Content-Length: $length"
            echo
            cat $BOOT_DIR/$file
        else
            show_html_header "File not found" "showmenu=no;"
            show_error "404 File not found" "The specified file was not found in the /boot directory!"
            show_html_footer
        fi
   ;;
esac
