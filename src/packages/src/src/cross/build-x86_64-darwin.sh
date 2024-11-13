#!/bin/bash

ROOT=$(dirname $(readlink -f $0))

OSXSDK_VERSION=10.8
DARWIN_VERSION=darwin9
ODCCTOOLS_VERSION=20090808
GMP_VERSION=6.0.0
GMP_REVISION=a
MPFR_VERSION=3.1.2
MPC_VERSION=1.0.2
GCC_VERSION=4.9.2

OSXSDK_URI=
ODCCTOOLS_URI=http://rsync.macports.org/odcctools
GMP_URI=ftp://ftp.gmplib.org/pub
MPFR_URI=http://www.mpfr.org
MPC_URI=http://www.multiprecision.org/mpc/download
GCC_URI=ftp://gcc.gnu.org/pub/gcc/releases

TARGET=x86_64-apple-$DARWIN_VERSION
PACKAGES="osxsdk odcctools gmp mpfr mpc gcc"

run_osxsdk_pass1()
{
	file=MacOSX${OSXSDK_VERSION}.sdk.tar.bz2
	extract osxsdk $DL/$file || return 2
	cp -a $BUILD/osxsdk/src/* $SYSROOT/ || return 3
}

run_osxsdk_pass2()
{
	# nothing to do
	:
}

run_odcctools_pass1()
{
	file=odcctools-${ODCCTOOLS_VERSION}.tar.bz2
	download "$ODCCTOOLS_URI" $file || return 2
	extract odcctools $DL/$file || return 3
	( cd $BUILD/odcctools/src &&
		for patch in "$ROOT"/darwin-patches/odcctools-*.patch; do \
			[ -f "$patch" ] || continue; \
			echo "Applying $patch..."; \
			patch -p1 < "$patch" || exit 1; \
		done \
	) || return 4
	mkdir -p $BUILD/odcctools/out
	cd $BUILD/odcctools/out &&
		CFLAGS="-O2 -m32" LDFLAGS="-O2 -m32" ../src/configure --target=$TARGET --prefix=$PREFIX --with-sysroot=$SYSROOT &&
		make $MAKEFLAGS &&
		make install
}

run_odcctools_pass2()
{
	# nothing to do
	:
}

run_gmp_pass1()
{
	file=gmp-${GMP_VERSION}${GMP_REVISION}.tar.xz
	download "$GMP_URI/gmp-${GMP_VERSION}" $file || return 2
	extract gmp $DL/$file || return 3
}

run_gmp_pass2()
{
	# nothing to do
	:
}

run_mpfr_pass1()
{
	file=mpfr-${MPFR_VERSION}.tar.xz
	download "$MPFR_URI/mpfr-${MPFR_VERSION}" $file || return 2
	extract mpfr $DL/$file || return 3
}

run_mpfr_pass2()
{
	# nothing to do
	:
}

run_mpc_pass1()
{
	file=mpc-${MPC_VERSION}.tar.gz
	download "$MPC_URI" $file || return 2
	extract mpc $DL/$file || return 3
}

run_mpc_pass2()
{
	# nothing to do
	:
}

run_gcc_pass1()
{
	file=gcc-${GCC_VERSION}.tar.bz2
	download "$GCC_URI/gcc-${GCC_VERSION}" $file || return 2
	extract gcc $DL/$file || return 3
	( cd $BUILD/gcc/src &&
		for patch in "$ROOT"/ct-ng-patches/gcc/${GCC_VERSION}/*.patch; do \
			[ -f "$patch" ] || continue; \
			echo "Applying $patch..."; \
			patch -p1 < "$patch" || exit 1; \
		done \
	) || return 4
	ln -s ../../gmp/src $BUILD/gcc/src/gmp
	ln -s ../../mpfr/src $BUILD/gcc/src/mpfr
	ln -s ../../mpc/src $BUILD/gcc/src/mpc
	mkdir -p $BUILD/gcc/out
	cd $BUILD/gcc/out &&
		CFLAGS_FOR_TARGET="-O2" CXXFLAGS_FOR_TARGET="-O2" \
		../src/configure --target=$TARGET --prefix=$PREFIX --with-sysroot=$SYSROOT \
			--disable-bootstrap --disable-nls \
			--enable-languages=c,c++ --enable-version-specific-runtime-libs \
			--with-arch=nocona --with-tune=generic \
			--with-gxx-include-dir=$PREFIX/include/c++/4.2.1 \
			--with-as=$PREFIX/bin/x86_64-apple-$DARWIN_VERSION-as \
			--with-ld=$PREFIX/bin/x86_64-apple-$DARWIN_VERSION-ld &&
		make $MAKEFLAGS &&
		make install
}

run_gcc_pass2()
{
	# nothing to do
	:
}

. $(dirname $0)/build-generic.sh
