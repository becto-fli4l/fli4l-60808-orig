#!/bin/bash

CROSSTOOL_NG_VERSION=1.21.0
CROSSTOOL_NG_URI=http://crosstool-ng.org/download/crosstool-ng

PACKAGES="crosstool_ng"
PATCHDIR=$(dirname $(readlink -f "$0"))/ct-ng-patches

run_crosstool_ng_pass1()
{
	file=crosstool-ng-${CROSSTOOL_NG_VERSION}.tar.bz2
	download "$CROSSTOOL_NG_URI" $file || return 2
	extract crosstool-ng $DL/$file || return 3
	if [ -f $PATCHDIR/$TARGET.patch ]; then
		( cd $BUILD/crosstool-ng/src; patch -p1 < $PATCHDIR/$TARGET.patch ) ||
			return 3
	fi

	config=$BUILD/crosstool-ng/src/.config
	cp -p $(dirname $0)/ct-ng-config.$TARGET $config
	sed -i -e "s,^\(CT_LOCAL_TARBALLS_DIR\)=.*,\1=\"$DL\"," $config
	sed -i -e "s,^\(CT_WORK_DIR\)=.*,\1=\"$BUILD\"," $config
	sed -i -e "s,^\(CT_PREFIX_DIR\)=.*,\1=\"$PREFIX\"," $config
	sed -i -e "s,^\(CT_LOCAL_PATCH_DIR\)=.*,\1=\"$PATCHDIR\"," $config

	pushd $BUILD/crosstool-ng/src
	./bootstrap || return 4
	./configure --enable-local || return 5
	make || return 6
	./ct-ng oldconfig || return 7
	./ct-ng build || return 8
	popd
}

run_crosstool_ng_pass2()
{
	# nothing to do
	:
}

. $(dirname $0)/build-generic.sh
