#!/bin/bash

# buildroot source directory
br_dir=$fbr_root/buildroot

# buildroot destination directory (working copy)
br_wcdir=$fbr_workdir/buildroot
br_wcout=$br_wcdir/output

# buildroot configuration
br_config=$br_dir/.config-$fbr_arch
br_wc_config=$br_wcdir/.config-$fbr_arch

# download directory
if [ -n "$FBR_DLDIR" ]
then
	fbr_dldir=$FBR_DLDIR
else
	fbr_dldir=$fbr_workdir/../dl
fi

# user-defined packages
fbr_own_packages=own
fbr_own_packages_path="package/$fbr_own_packages"
fbr_own_packages_menu="User-defined packages"

if [ -n "$FBR_OWNDIR" ]
then
	fbr_owndir=$FBR_OWNDIR
else
	fbr_owndir=$fbr_workdir/../$fbr_own_packages
fi

fbr_own_wc_dir=$fbr_workdir/$fbr_own_packages
fbr_own_packages_config=$fbr_owndir/.config-$fbr_arch
mkdir -p $fbr_owndir

# build log
fbr_log=$br_wcout/build.log

# other properties
fbr_tidy=${FBR_TIDY:-y}
fbr_nocheck=${FBR_NOCHECK:-n}

message "fbr-make started :-)" standout
message "Source branch:                   " "" -n
message "$fbr_category/$fbr_branch" yellow
message "Architecture:                    " "" -n
message "$fbr_arch" yellow
message "Working directory:               " "" -n
message "$fbr_workdir" yellow
message "Download directory:              " "" -n
message "$fbr_dldir" yellow
message "User-defined packages directory: " "" -n
message "$fbr_owndir" yellow
message "Deleting build artefacts:        " "" -n
case $fbr_tidy in
y)
	message "yes" yellow;;
k)
	message "yes (kernel only)" yellow;;
*)
	message "no" red;;
esac
message "Additional kernels to build:     " "" -n
message "${FBR_ADDITIONAL_KERNELS:-(none)}" yellow
if [ "$fbr_nocheck" = y ]
then
	message ""
	message "                **********************************************" red
	message "                * " red -n
	message "NO FBR DIRECTORY SYNCHRONISATION PERFORMED" yellow -n
	message " *" red
	message "                **********************************************" red
	message ""
fi
