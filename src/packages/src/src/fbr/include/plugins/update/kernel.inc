#!/bin/bash
#
# $1 = "nodelete" if no cleaning up is to be performed

: ${MODINFO:=modinfo}

. "$fbr_root/include/plugins/update/kernel-nic-descriptions.txt"

# global firmware list
global_firmware_list="packages/firmware/files.txt"

# input for handlers:
#  $kernel_package    : name of kernel package (e.g. kernel_4_19)
#  $kernel_version    : full kernel version (e.g. 4.19.80)
#  $kernel_major      : kernel major version (e.g. 4.19)
#  $kernel_dir        : full path to kernel's directory
#  $modules_dir       : full path to kernel's module directory

update_files_txt()
{
	message "$kernel_version: Updating files.txt..."

	local dir=$binroot/packages/$kernel_package
	mkdir -p "$dir"
	filestxt="$dir/files.txt"
	while read file
	do
		echo "b ${fbr_arch}::opt/${file#$binroot/}"
	done < <(find "$kernel_dir/" -type f 2>/dev/null) > "$filestxt"
	while read file
	do
		echo "b ${fbr_arch}::opt/${file#$br_wcout/target/}"
	done < <(find "$modules_dir/" -type f \! -name 'modules.*.bin' 2>/dev/null) >> "$filestxt"
	find "$dir/" -type f -o -type l |
		sed "/^${dir////\\/}\/files.txt$/d;s,^${dir//,/\\,}/doc/,u doc/,;s,^${dir//,/\\,}/,d ," >> "$filestxt"
	echo "d opt/${kernel_package}.txt" >> "$filestxt"

	LC_ALL="C" sort -k 2 -u -o "$filestxt" "$filestxt"
}

# private function used by update_csv_nic_lists()
parse_driver_info()
{
	DRIVERNAME=${1##*/}
	DRIVERNAME=${DRIVERNAME%.ko}

	WIRELESS='NO'
	BUS=''
	case "$1" in
		*/wan/*)     return 1;;
		*/wimax/*)   return 1;;
		*wireless*)  WIRELESS='YES';;
	esac

	DESCRIPTION=$($MODINFO "$1" | sed -n 's/^description: *\([ a-zA-Z]\{1,\}\)/\1/p' | sed 's/ *[dD]river[ .]*$//;s/^ //;s/ *Driver Version.*$//;s/^\([dD]river\|[sS]upport\) for //;s/ISA //;s/ *\.$//' | head -n 1)
	case $DESCRIPTION in
		*FIR*)                            return 1;;
		*wireless*|*WIRELESS*|*Wireless*) WIRELESS='YES';;
		*wlan*|*WLAN*|*Wlan*)             WIRELESS='YES';;
	esac

	# let the description from nic-descriptions.txt (if any) override the
	# one found in the driver
	eval local desc=\"\$driver_${DRIVERNAME//-/_}\"
	[ -n "$desc" ] && DESCRIPTION="$desc"

	DEPENDS=$($MODINFO "$1" | sed -n 's/^depends: *\([ a-zA-Z]\{1,\}\)/\1/p')
	case $DEPENDS in
		*irda*|*IRDA*|*Irda*) return 1;;
		*80211*)              WIRELESS='YES';;
	esac

	IO=$($MODINFO "$1" | sed -n '/^parm: *io:.*/p')
	if [ -n "$($MODINFO "$1" | grep "^alias:")" ]
	then
		# it's a driver for PnP hardware
		BUS=$($MODINFO "$1" | sed -n 's/^alias: *\([a-z]\{1,\}\):.*/\1/p' | sed 's/^\(ssb\|bcma\)$/pci/' | sed 's/^pnp$/isa/' | LC_ALL="C" sort -u)
		BUS=$(echo $BUS | sed 's/ /,/')
		if [ -n "$IO" ]
		then
			case $BUS in
				isa*) ;;
				*)
					case $BUS in
						eisa) BUS='eisa,isa';;
						*)    BUS="isa,$BUS";;
					esac
				;;
			esac
			case $DRIVERNAME in
				de4x5) BUS='eisa,pci';;
			esac
		else
			case $DRIVERNAME in
				dummy) BUS='virtual';;
			esac
		fi
	else
		IRQ=$($MODINFO "$1" | sed -n '/^parm: *irq:.*/p')
		if [ -n "$IO$IRQ" ]
		then
			BUS='isa'
		else
			case $DRIVERNAME in
				mac80211_hwsim) BUS='virtual';;
				mlx4_en) BUS='pci';;
			esac
		fi
	fi
	[ -n "$BUS" ]
}

update_csv_nic_lists()
{
	message "$kernel_version: Updating raw list of supported NICs..."

	outfile_dir="$binroot/packages/$kernel_package/doc/common/tex/$kernel_package"
	outfile_lan="$outfile_dir/niclist_${kernel_package}_lan.csv"
	outfile_wlan="$outfile_dir/niclist_${kernel_package}_wlan.csv"
	mkdir -p "$outfile_dir"
	> "$outfile_lan"
	> "$outfile_wlan"

	local driver
	find $modules_dir/kernel/drivers/net -type f -name '*.ko' | LC_ALL="C" sort -u | while read driver
	do
		parse_driver_info "$driver" || continue
		BUS=$(echo $BUS | sed 's/\(platform\|spi\|sdio\|mdio\|vmbus\|devname\|of\)//g;s/^,//;s/,$//;s/,\{1,\}/,/g')
		[ -n "$BUS" ] || continue;

		case $WIRELESS in
		NO)
			outfile="$outfile_lan"
			;;
		YES)
			outfile="$outfile_wlan"
			;;
		esac
		echo "$BUS#$DRIVERNAME#$DESCRIPTION" >> "$outfile"
	done
	LC_ALL="C" sort -u -t '#' -k 2 -o "$outfile_lan" "$outfile_lan"
	LC_ALL="C" sort -u -t '#' -k 2 -o "$outfile_wlan" "$outfile_wlan"
	sed -i "1 ikernel version#$kernel_version" "$outfile_lan"
	sed -i "1 ikernel version#$kernel_version" "$outfile_wlan"
}

# private function used by update_doc_nic_lists()
write_nic_doc_entries()
{
	local old_IFS=$IFS
	IFS='#'
	while read BUS DRIVER DESCRIPTION
	do
		printf "| %-15s | %-10s | %s\n" $DRIVER $BUS
		printf "| %-15s | %s\n" " " "$DESCRIPTION"
		echo "-------------------------------------------------------------------------------"
	done
	IFS=$old_IFS
}

update_doc_nic_lists()
{
	message "$kernel_version: Updating pretty list of supported NICs..."
	outfile="$binroot/packages/$kernel_package/config/base_nic_${kernel_package}.list"
	mkdir -p "$(dirname "$outfile")"
	{
		cat << EOF
List of LAN drivers supported by fli4l
--------------------------------------

| drivername      | bus
|                 | description of the driver
===============================================================================
EOF
		write_nic_doc_entries < "$binroot/packages/$kernel_package/doc/common/tex/$kernel_package/niclist_${kernel_package}_lan.csv"

		cat << EOF


List of WLAN drivers supported by fli4l
---------------------------------------

| drivername      | bus
|                 | description of the driver
===============================================================================
EOF
		write_nic_doc_entries < "$binroot/packages/$kernel_package/doc/common/tex/$kernel_package/niclist_${kernel_package}_wlan.csv"
	} > "$outfile"
}

get_module_clients()
{
	# NOTE: as depmod generates a modules.dep being fully recursive, we do
	# not need to call ourselves recursively on each client
	sed -n "s,^[^ ]\+/\([^/]\+\.ko\): .*/$1\( .*\)\?$,\1,p" "$modules_dir/modules.dep" | LC_ALL="C" sort -u
}

update_firmware_list()
{
	message "$kernel_version: Updating list of needed firmware files..."
	outfile="$binroot/packages/${kernel_package}/opt/${kernel_package}.txt"
	mkdir -p "$(dirname "$outfile")"
	{
		echo "#----------------------------------------------------------------------------"
		echo "#  fli4l __FLI4LVER__ - firmware files for kernel ${kernel_package}"
		echo "#"
		echo "#  Last Update: \$Id\$"
		echo "#"
		echo "#  Copyright (c) 2001-2016 - Frank Meyer, fli4l-Team <team@fli4l.de>"
		echo "#----------------------------------------------------------------------------"
		echo "#"
		echo "#"
		echo "opt_format_version 1 -"

		echo
		echo "#----------------------------------------------------------------------------"
		echo "# firmware files"
		echo "#----------------------------------------------------------------------------"

		local entries=
		local weak_vars=
		local driver
		while read driver
		do
			local driver_rel=${driver#$modules_dir/}
			local firmware
			for firmware in $($MODINFO $driver | sed -n "s/^firmware:[[:space:]]*\(.*\)/\1/p" | LC_ALL="C" sort -u)
			do
				local path="$br_wcout/target/lib/firmware/$firmware"
				if [ -f $path ]
				then
					local fwspec="lib/firmware/$firmware"
				else
					unset path
				fi

				local name=$(basename "$driver")
				local regex=${name//./\\.}
				local client
				for client in $(get_module_clients $name)
				do
					regex+="\|${client//./\\.}"
				done

				while read var expr prefix
				do
					if [ -n "$path" ]
					then
						entries+=" $var $expr ${prefix:--} $fwspec"
						weak_vars+=" $var"
					elif ! grep -q "^$driver_rel: $firmware$" "$fbr_root/include/plugins/update/kernel-skipped-firmware.txt"
					then
						echo "WARNING: $driver_rel: firmware $firmware not found" >&2
						break
					fi
				done < <(sed -n "s/^[[:space:]]*\([^ ]\+\)[[:space:]]\+\([^ ]\+\)[[:space:]]\+\(rootfs:\)\?\($regex\)\>.*/\1 \2 \3/p" "$pkgroot"/*/opt/*.txt)
			done
		done < <(find "$modules_dir/kernel/drivers" -type f -name '*.ko' | LC_ALL="C" sort)

		for v in $weak_vars
		do
			grep -qi "^[+]*$v[[:space:]]" "$pkgroot/base/check/base.txt" || echo "weak $v -"
		done | LC_ALL="C" sort -u

		set -- $entries
		while [ $# -ne 0 ]
		do
			echo "$1 $2 $3 $4"
			shift 4
		done <<< $entries | LC_ALL="C" sort -u | (
			last=
			while read var expr prefix file opt
			do
				[ "$var" = "$last" ] || echo
				last=$var
				[ "$prefix" = "-" ] && prefix=
				echo "$var $expr $prefix$file"
				flock "$binroot/$global_firmware_list" echo "b opt/$file" >> "$binroot/$global_firmware_list"
			done
		)
	} > "$outfile"
}

cleanup_kernels()
{
	message "    Cleaning up kernel packages..."
	rm -rf $binroot/packages/kernel_*
	rm -f $binroot/packages/base/opt/base_*.txt

	local filestxt=$binroot/packages/base/files.txt
	if [ -f "$filestxt" ]
	then
		sed -i "/^. opt\/base_.*\.txt$/d" $filestxt
	fi

	> "$binroot/$global_firmware_list"
}

update_kernel()
{
	if [ "$1" != "nodelete" ]
	then
		cleanup_kernels
	fi

	for kernel_dir in $binroot/img/linux-*
	do
		local kernel=$kernel_dir/kernel
		[ -f "$kernel" ] || continue

		(
		kernel_version=${kernel_dir#$binroot/img/linux-}

		modules_dir=$br_wcout/target/lib/modules/$kernel_version
		if [ ! -d "$modules_dir" ]
		then
			warning "$kernel_version: Can't find $modules_dir, skipping $kernel_version"
			continue
		fi

		kernel_major=$(echo $kernel_version | sed 's/^\([0-9]\+\.[0-9]\+\).*$/\1/')

		kernel_pkg_version=${kernel_major//./_}
		kernel_package="kernel_${kernel_pkg_version}"

		if [ ! -d $pkgroot/$kernel_package ]
		then
			warning "$kernel_version: Can't find package $kernel_package, skipping $kernel_version"
			continue
		fi

		update_csv_nic_lists
		update_doc_nic_lists
		update_firmware_list
		update_files_txt
		) &
	done
	wait

	LC_ALL="C" sort -u -o "$binroot/$global_firmware_list" "$binroot/$global_firmware_list"
}

update_plugin_messages+="      --update-kernel-files    update kernel packages
"

if [ "$update_kernel_files" = yes ]
then
	update_plugins+=" update_kernel"
fi
