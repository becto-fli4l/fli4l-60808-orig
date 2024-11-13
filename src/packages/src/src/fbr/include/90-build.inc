#!/bin/bash

unset targets
unset config_targets
unset build_targets
unset clean_targets
unset have_all
unset have_linux
unset legal_info

# $1 = configuration to split
split_configuration()
{
	line=$(grep -n "^# $fbr_own_packages_menu$" $1 | cut -d : -f 1 | head -n 1)
	if [ -n "$line" ] && [ $line -ge 3 ]
	then
		sed -n -e "1,$((line-2))p" $1 > $br_config
		sed -n -e "1,$((line-2))!p" $1 > $fbr_own_packages_config
	else
		cat $1 > $br_config
		> $fbr_own_packages_config
	fi
}

if [ -f $fbr_log ]
then
	# write EOT (end-of-transmission) to log file in order to separate
	# old data from new data
	echo -e "\004" >> $fbr_log
fi
(
	echo "=============================================================================="
	echo "FBR log started at $(date)"

	BR_VERSION=$(grep "\<BR2_VERSION\>" $br_wcdir/Makefile | head -n 1 |
		sed -e 's/^.*BR2_VERSION[[:space:]]*:=[[:space:]]*\(.*\)/\1/')
	echo "Version of underlying uClibc buildroot: $BR_VERSION"

	if [ -d "$fbr_root/.svn" ] && type -p svnversion >/dev/null 2>&1
	then
		echo "SVN Revision: $(LANG=C svnversion -n $fbr_root)"
	fi
	echo "=============================================================================="
) >> $fbr_log

set -- $@
for i in $(seq 1 $#)
do
	target="$1"
	shift

	if [ "$target" = "show-versions" ]
	then
		message "Configured packages" standout
		make_targets_no_log --no-print-directory show-versions
		continue
	elif [ "$target" = "show-archives" ]
	then
		message "Needed archives" standout
		make_targets_no_log --no-print-directory show-archives
		continue
	elif [ "$target" = "buildroot-menuconfig" ]
	then
		message "Configuring buildroot" standout

		tmpconfig=$br_wcout/config/$(basename $br_wc_config)
		mkdir -p $(dirname $tmpconfig)
		cat $br_config $fbr_own_packages_config > $tmpconfig 2>/dev/null
		make_targets_no_log CONFIG_DIR=$(dirname $tmpconfig) BR2_CONFIG=$tmpconfig menuconfig
		split_configuration $tmpconfig

		if [ $# -gt 0 -o -n "${targets# }" ]
		then
			message "Restarting $progname due to new buildroot configuration" standout
			"$prog" $targets "$@"
			exit $?
		fi
	elif echo "$target" | grep -q "^.*-menuconfig$"
	then
		pkg=$(echo "$target" | sed 's/^\(.*\)-menuconfig$/\1/')
		config_targets+=" $pkg"
		echo "$target" | grep -q "^linux-" ||
			clean_targets+=" $pkg"
	elif [ "$target" = "legal-info" ]
	then
		legal_info=" legal-info"
	elif [ "$target" = "linux" ]
	then
		have_linux=1
		build_targets+=" linux"
	elif [ "$target" = "world" ]
	then
		have_all=1
		have_linux=1
	elif echo "$target" | grep -q "^.*-clean$"
	then
		pkg="$(echo "$target" | sed -e 's/^\(.*\)-clean$/\1/')"
		clean_targets+=" $pkg"
	else
		build_targets+=" $target"
	fi
	targets+=" $target"
done

if [ -n "$clean_targets" ]
then
	fbr_to_uninstall=" $clean_targets"
	fbr_complete_uninstall_packages
	if [ -n "${fbr_to_uninstall% }" ]
	then
		message "Cleaning${fbr_to_uninstall% }" standout
		make_targets $(printf '%s\n' $fbr_to_uninstall | sort -u | sed -e 's/.\+/&-uninstall &-dirclean/')
	else
		message "Nothing to clean." standout
	fi
fi

BR2_LINUX_KERNEL_VERSION=$(sed -n 's/^BR2_LINUX_KERNEL_VERSION="\(.*\)"$/\1/p' ${br_wc_config})
BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=$(sed -n 's/^BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE="\(.*\)"$/\1/p' ${br_wc_config})

if [ -n "$config_targets" ]
then
	for target in $config_targets
	do
		message "Configuring ${target}" standout
		J=1 make_targets_no_log KCONFIG_FILE_DIR="$br_dir/" $target-menuconfig $target-update-config

		if [ "$target" = linux -a -n "$FBR_ADDITIONAL_KERNELS" ]
		then
			for k in $FBR_ADDITIONAL_KERNELS
			do
				sed -i "s,^BR2_LINUX_KERNEL_VERSION=.*,BR2_LINUX_KERNEL_VERSION=\"$k\"," ${br_wc_config}
				sed -i "s,^BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=.*,BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=\"$k\"," ${br_wc_config}

				message "Configuring ${target}" standout
				J=1 make_targets_no_log KCONFIG_FILE_DIR="$br_dir/" $target-menuconfig $target-update-config
			done

			sed -i "s,^BR2_LINUX_KERNEL_VERSION=.*,BR2_LINUX_KERNEL_VERSION=\"$BR2_LINUX_KERNEL_VERSION\"," ${br_wc_config}
			sed -i "s,^BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=.*,BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=\"$BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE\"," ${br_wc_config}
		fi
	done
fi

if [ -n "$have_all" ]
then
	build_targets=" world"
fi
if [ -n "$have_linux" ]
then
	build_targets+=" linux-modules"
fi

if [ -n "$build_targets" ]
then
	build_targets+=${legal_info}

	message "Building${build_targets}" standout
	make_targets $build_targets

	if [ -n "$FBR_ADDITIONAL_KERNELS" ]
	then
		for k in $FBR_ADDITIONAL_KERNELS
		do
			sed -i "s,^BR2_LINUX_KERNEL_VERSION=.*,BR2_LINUX_KERNEL_VERSION=\"$k\"," ${br_wc_config}
			sed -i "s,^BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=.*,BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=\"$k\"," ${br_wc_config}
			make_targets $build_targets
		done

		sed -i "s,^BR2_LINUX_KERNEL_VERSION=.*,BR2_LINUX_KERNEL_VERSION=\"$BR2_LINUX_KERNEL_VERSION\"," ${br_wc_config}
		sed -i "s,^BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=.*,BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE=\"$BR2_LINUX_KERNEL_CUSTOM_VERSION_VALUE\"," ${br_wc_config}
	fi
fi

(
	echo "=============================================================================="
	echo "FBR log finished at $(date)"
	echo "=============================================================================="
) >> $fbr_log

message "Build successful." standout
