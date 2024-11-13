# Makefile for buildroot
#
# Copyright (C) 1999-2005 by Erik Andersen <andersen@codepoet.org>
# Copyright (C) 2006-2014 by the Buildroot developers <buildroot@uclibc.org>
# Copyright (C) 2014-2018 by the Buildroot developers <buildroot@buildroot.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#--------------------------------------------------------------
# Just run 'make menuconfig', configure stuff, then run 'make'.
# You shouldn't need to mess with anything beyond this point...
#--------------------------------------------------------------

# Delete default rules. We don't use them. This saves a bit of time.
.SUFFIXES:

# We need this for various package interdependencies where the definition of
# one package needs to access variables set by another package.
.SECONDEXPANSION:

# we want bash as shell
SHELL := $(shell if [ -x /bin/bash ]; then echo /bin/bash; \
	 else echo sh; fi)

# Set O variable if not already done on the command line;
# or avoid confusing packages that can use the O=<dir> syntax for out-of-tree
# build by preventing it from being forwarded to sub-make calls.
ifneq ("$(origin O)", "command line")
O := $(CURDIR)/output
endif

# Check if the current Buildroot execution meets all the pre-requisites.
# If they are not met, Buildroot will actually do its job in a sub-make meeting
# its pre-requisites, which are:
#  1- Permissive enough umask:
#       Wrong or too restrictive umask will prevent Buildroot and packages from
#       creating files and directories.
#  2- Absolute canonical CWD (i.e. $(CURDIR)):
#       Otherwise, some packages will use CWD as-is, others will compute its
#       absolute canonical path. This makes harder tracking and fixing host
#       machine path leaks.
#  3- Absolute canonical output location (i.e. $(O)):
#       For the same reason as the one for CWD.

# Remove the trailing '/.' from $(O) as it can be added by the makefile wrapper
# installed in the $(O) directory.
# Also remove the trailing '/' the user can set when on the command line.
override O := $(patsubst %/,%,$(patsubst %.,%,$(O)))
# Make sure $(O) actually exists before calling realpath on it; this is to
# avoid empty CANONICAL_O in case on non-existing entry.
CANONICAL_O := $(shell mkdir -p $(O) >/dev/null 2>&1)$(realpath $(O))

# gcc fails to build when the srcdir contains a '@'
ifneq ($(findstring @,$(CANONICAL_O)),)
$(error The build directory can not contain a '@')
endif

CANONICAL_CURDIR = $(realpath $(CURDIR))

REQ_UMASK = 0022

# Make sure O= is passed (with its absolute canonical path) everywhere the
# toplevel makefile is called back.
EXTRAMAKEARGS := O=$(CANONICAL_O)

# Check Buildroot execution pre-requisites here.
ifneq ($(shell umask):$(CURDIR):$(O),$(REQ_UMASK):$(CANONICAL_CURDIR):$(CANONICAL_O))
.PHONY: _all $(MAKECMDGOALS)

$(MAKECMDGOALS): _all
	@:

_all:
	@umask $(REQ_UMASK) && \
		$(MAKE) -C $(CANONICAL_CURDIR) --no-print-directory \
			$(MAKECMDGOALS) $(EXTRAMAKEARGS)

else # umask / $(CURDIR) / $(O)

# This is our default rule, so must come first
all:
.PHONY: all

# Set and export the version string
export BR2_VERSION := 2018.02.8
# Actual time the release is cut (for reproducible builds)
BR2_VERSION_EPOCH = 1543272000

# Save running make version since it's clobbered by the make package
RUNNING_MAKE_VERSION := $(MAKE_VERSION)

# Check for minimal make version (note: this check will break at make 10.x)
MIN_MAKE_VERSION = 3.81
ifneq ($(firstword $(sort $(RUNNING_MAKE_VERSION) $(MIN_MAKE_VERSION))),$(MIN_MAKE_VERSION))
$(error You have make '$(RUNNING_MAKE_VERSION)' installed. GNU make >= $(MIN_MAKE_VERSION) is required)
endif

# absolute path
TOPDIR := $(CURDIR)
CONFIG_CONFIG_IN = Config.in
CONFIG = support/kconfig
DATE := $(shell date +%Y%m%d)

# Compute the full local version string so packages can use it as-is
# Need to export it, so it can be got from environment in children (eg. mconf)
export BR2_VERSION_FULL := $(BR2_VERSION)$(shell $(TOPDIR)/support/scripts/setlocalversion)

# List of targets and target patterns for which .config doesn't need to be read in
noconfig_targets := menuconfig nconfig gconfig xconfig config oldconfig randconfig \
	defconfig %_defconfig allyesconfig allnoconfig alldefconfig silentoldconfig release \
	randpackageconfig allyespackageconfig allnopackageconfig \
	print-version olddefconfig distclean manual manual-%

# Some global targets do not trigger a build, but are used to collect
# metadata, or do various checks. When such targets are triggered,
# some packages should not do their configuration sanity
# checks. Provide them a BR_BUILDING variable set to 'y' when we're
# actually building and they should do their sanity checks.
#
# We're building in two situations: when MAKECMDGOALS is empty
# (default target is to build), or when MAKECMDGOALS contains
# something else than one of the nobuild_targets.
nobuild_targets := source %-source \
	legal-info %-legal-info external-deps _external-deps \
	clean distclean help show-targets graph-depends \
	%-graph-depends %-show-depends %-show-version \
	graph-build graph-size list-defconfigs \
	savedefconfig printvars
ifeq ($(MAKECMDGOALS),)
BR_BUILDING = y
else ifneq ($(filter-out $(nobuild_targets),$(MAKECMDGOALS)),)
BR_BUILDING = y
endif

# We call make recursively to build packages. The command-line overrides that
# are passed to Buildroot don't apply to those package build systems. In
# particular, we don't want to pass down the O=<dir> option for out-of-tree
# builds, because the value specified on the command line will not be correct
# for packages.
MAKEOVERRIDES :=

# Include some helper macros and variables
include support/misc/utils.mk

# Set variables related to in-tree or out-of-tree build.
# Here, both $(O) and $(CURDIR) are absolute canonical paths.
ifeq ($(O),$(CURDIR)/output)
CONFIG_DIR := $(CURDIR)
NEED_WRAPPER =
else
CONFIG_DIR := $(O)
NEED_WRAPPER = y
endif

# bash prints the name of the directory on 'cd <dir>' if CDPATH is
# set, so unset it here to not cause problems. Notice that the export
# line doesn't affect the environment of $(shell ..) calls.
export CDPATH :=

BASE_DIR := $(CANONICAL_O)
$(if $(BASE_DIR),, $(error output directory "$(O)" does not exist))

# To make sure that the environment variable overrides the .config option,
# set this before including .config.
ifneq ($(BR2_DL_DIR),)
DL_DIR := $(BR2_DL_DIR)
endif
ifneq ($(BR2_CCACHE_DIR),)
BR_CACHE_DIR := $(BR2_CCACHE_DIR)
endif

# Need that early, before we scan packages
# Avoids doing the $(or...) everytime
BR_GRAPH_OUT := $(or $(BR2_GRAPH_OUT),pdf)

BUILD_DIR := $(BASE_DIR)/build
BINARIES_DIR := $(BASE_DIR)/images
TARGET_DIR := $(BASE_DIR)/target
STAMP_DIR := $(BASE_DIR)/stamps
SANDBOX_DIR := $(BASE_DIR)/sandbox
DEBUG_DIR := $(BASE_DIR)/debug
# initial definition so that 'make clean' works for most users, even without
# .config. HOST_DIR will be overwritten later when .config is included.
HOST_DIR := $(BASE_DIR)/host
GRAPHS_DIR := $(BASE_DIR)/graphs

LEGAL_INFO_DIR = $(BASE_DIR)/legal-info
REDIST_SOURCES_DIR_TARGET = $(LEGAL_INFO_DIR)/sources
REDIST_SOURCES_DIR_HOST = $(LEGAL_INFO_DIR)/host-sources
LICENSE_FILES_DIR_TARGET = $(LEGAL_INFO_DIR)/licenses
LICENSE_FILES_DIR_HOST = $(LEGAL_INFO_DIR)/host-licenses
LEGAL_MANIFEST_CSV_TARGET = $(LEGAL_INFO_DIR)/manifest.csv
LEGAL_MANIFEST_CSV_HOST = $(LEGAL_INFO_DIR)/host-manifest.csv
LEGAL_WARNINGS = $(LEGAL_INFO_DIR)/.warnings
LEGAL_REPORT = $(LEGAL_INFO_DIR)/README

DIR_TARGETS = $(foreach dir,BUILD STAMP BINARIES SANDBOX DEBUG HOST,$($(dir)_DIR)/.root) $(STAMP_DIR)/.root-staging $(STAMP_DIR)/.root-target
SOURCES :=

BR2_CONFIG = $(CONFIG_DIR)/.config

# Pull in the user's configuration file
ifeq ($(filter $(noconfig_targets),$(MAKECMDGOALS)),)
-include $(BR2_CONFIG)
endif

# timezone and locale may affect build output
ifeq ($(BR2_REPRODUCIBLE),y)
export TZ = UTC
export LANG = C
export LC_ALL = C
export GZIP = -n
BR2_VERSION_GIT_EPOCH = $(shell GIT_DIR=$(TOPDIR)/.git $(GIT) log -1 --format=%at)
export SOURCE_DATE_EPOCH ?= $(if $(wildcard $(TOPDIR)/.git),$(BR2_VERSION_GIT_EPOCH),$(BR2_VERSION_EPOCH))
DEPENDENCIES_HOST_PREREQ += host-fakedate
endif

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands
ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  Q =
ifndef VERBOSE
  VERBOSE = 1
endif
export VERBOSE
else
  Q = @
endif

# kconfig uses CONFIG_SHELL
CONFIG_SHELL := $(SHELL)

export SHELL CONFIG_SHELL Q KBUILD_VERBOSE

ifndef HOSTAR
HOSTAR := ar
endif
ifndef HOSTAS
HOSTAS := as
endif
ifndef HOSTCC
HOSTCC := gcc
HOSTCC := $(shell which $(HOSTCC) || type -p $(HOSTCC) || echo gcc)
endif
HOSTCC_NOCCACHE := $(HOSTCC)
ifndef HOSTCXX
HOSTCXX := g++
HOSTCXX := $(shell which $(HOSTCXX) || type -p $(HOSTCXX) || echo g++)
endif
HOSTCXX_NOCCACHE := $(HOSTCXX)
ifndef HOSTCPP
HOSTCPP := cpp
endif
ifndef HOSTLD
HOSTLD := ld
endif
ifndef HOSTLN
HOSTLN := ln
endif
ifndef HOSTNM
HOSTNM := nm
endif
ifndef HOSTOBJCOPY
HOSTOBJCOPY := objcopy
endif
ifndef HOSTRANLIB
HOSTRANLIB := ranlib
endif
HOSTAR := $(shell which $(HOSTAR) || type -p $(HOSTAR) || echo ar)
HOSTAS := $(shell which $(HOSTAS) || type -p $(HOSTAS) || echo as)
HOSTCPP := $(shell which $(HOSTCPP) || type -p $(HOSTCPP) || echo cpp)
HOSTLD := $(shell which $(HOSTLD) || type -p $(HOSTLD) || echo ld)
HOSTLN := $(shell which $(HOSTLN) || type -p $(HOSTLN) || echo ln)
HOSTNM := $(shell which $(HOSTNM) || type -p $(HOSTNM) || echo nm)
HOSTOBJCOPY := $(shell which $(HOSTOBJCOPY) || type -p $(HOSTOBJCOPY) || echo objcopy)
HOSTRANLIB := $(shell which $(HOSTRANLIB) || type -p $(HOSTRANLIB) || echo ranlib)
SED := $(shell which sed || type -p sed) -i -e

export HOSTAR HOSTAS HOSTCC HOSTCXX HOSTLD
export HOSTCC_NOCCACHE HOSTCXX_NOCCACHE

# Determine the userland we are running on.
#
# Note that, despite its name, we are not interested in the actual
# architecture name. This is mostly used to determine whether some
# of the binary tools (e.g. pre-built external toolchains) can run
# on the current host. So we need to know if the userland we're
# running on can actually run those toolchains.
#
# For example, a 64-bit prebuilt toolchain will not run on a 64-bit
# kernel if the userland is 32-bit (e.g. in a chroot for example).
#
# So, we extract the first part of the tuple the host gcc was
# configured to generate code for; we assume this is our userland.
#
export HOSTARCH := $(shell LC_ALL=C $(HOSTCC_NOCCACHE) -v 2>&1 | \
	sed -e '/^Target: \([^-]*\).*/!d' \
	    -e 's//\1/' \
	    -e 's/i.86/x86/' \
	    -e 's/sun4u/sparc64/' \
	    -e 's/arm.*/arm/' \
	    -e 's/sa110/arm/' \
	    -e 's/ppc64/powerpc64/' \
	    -e 's/ppc/powerpc/' \
	    -e 's/macppc/powerpc/' \
	    -e 's/sh.*/sh/' )

# When adding a new host gcc version in Config.in,
# update the HOSTCC_MAX_VERSION variable:
HOSTCC_MAX_VERSION := 8

HOSTCC_VERSION := $(shell V=$$($(HOSTCC_NOCCACHE) --version | \
	sed -n -r 's/^.* ([0-9]*)\.([0-9]*)\.([0-9]*)[ ]*.*/\1 \2/p'); \
	[ "$${V%% *}" -le $(HOSTCC_MAX_VERSION) ] || V=$(HOSTCC_MAX_VERSION); \
	printf "%s" "$${V}")

# For gcc >= 5.x, we only need the major version.
ifneq ($(firstword $(HOSTCC_VERSION)),4)
HOSTCC_VERSION := $(firstword $(HOSTCC_VERSION))
endif

ifeq ($(BR2_NEEDS_HOST_UTF8_LOCALE),y)
# First, we try to use the user's configured locale (as that's the
# language they'd expect messages to be displayed), then we favour
# a non language-specific locale like C.UTF-8 if one is available,
# so we sort with the C locale to get it at the top.
# This is guaranteed to not be empty, because of the check in
# support/dependencies/dependencies.sh
HOST_UTF8_LOCALE := $(shell \
			( echo $${LC_ALL:-$${LC_MESSAGES:-$${LANG}}}; \
			  locale -a 2>/dev/null | LC_ALL=C sort \
			) \
			| grep -i -E 'utf-?8$$' \
			| head -n 1)
HOST_UTF8_LOCALE_ENV := LC_ALL=$(HOST_UTF8_LOCALE)
endif

# Make sure pkg-config doesn't look outside the buildroot tree
HOST_PKG_CONFIG_PATH := $(PKG_CONFIG_PATH)
unexport PKG_CONFIG_PATH
unexport PKG_CONFIG_SYSROOT_DIR
unexport PKG_CONFIG_LIBDIR

# Having DESTDIR set in the environment confuses the installation
# steps of some packages.
unexport DESTDIR

# Causes breakage with packages that needs host-ruby
unexport RUBYOPT

include package/pkg-utils.mk
include package/doc-asciidoc.mk

ifeq ($(BR2_HAVE_DOT_CONFIG),y)

################################################################################
#
# Hide troublesome environment variables from sub processes
#
################################################################################
unexport CROSS_COMPILE
unexport ARCH
unexport CC
unexport LD
unexport AR
unexport CXX
unexport CPP
unexport RANLIB
unexport CFLAGS
unexport CXXFLAGS
unexport GREP_OPTIONS
unexport TAR_OPTIONS
unexport CONFIG_SITE
unexport QMAKESPEC
unexport TERMINFO
unexport MACHINE
unexport O
unexport GCC_COLORS

GNU_HOST_NAME := $(shell support/gnuconfig/config.guess)

# all targets
TARGETS :=
# all enabled targets
PACKAGES :=
PACKAGES_ALL :=

# silent mode requested?
QUIET := $(if $(findstring s,$(filter-out --%,$(MAKEFLAGS))),-q)

# Strip off the annoying quoting
ARCH := $(call qstrip,$(BR2_ARCH))

KERNEL_ARCH := $(shell echo "$(ARCH)" | sed -e "s/-.*//" \
	-e s/i.86/i386/ -e s/sun4u/sparc64/ \
	-e s/arcle/arc/ \
	-e s/arceb/arc/ \
	-e s/arm.*/arm/ -e s/sa110/arm/ \
	-e s/aarch64.*/arm64/ \
	-e s/bfin/blackfin/ \
	-e s/or1k/openrisc/ \
	-e s/parisc64/parisc/ \
	-e s/powerpc64.*/powerpc/ \
	-e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
	-e s/sh.*/sh/ \
	-e s/microblazeel/microblaze/)

ZCAT := $(call qstrip,$(BR2_ZCAT))
BZCAT := $(call qstrip,$(BR2_BZCAT))
XZCAT := $(call qstrip,$(BR2_XZCAT))
LZCAT := $(call qstrip,$(BR2_LZCAT))
TAR_OPTIONS = $(call qstrip,$(BR2_TAR_OPTIONS)) -xf

# packages compiled for the host go here
HOST_DIR := $(call qstrip,$(BR2_HOST_DIR))

# Quotes are needed for spaces and all in the original PATH content.
BR_PATH = "$(HOST_DIR)/bin:$(HOST_DIR)/sbin:$(PATH)"

# Location of a file giving a big fat warning that output/target
# should not be used as the root filesystem.
TARGET_DIR_WARNING_FILE = $(TARGET_DIR)/THIS_IS_NOT_YOUR_ROOT_FILESYSTEM

ifeq ($(BR2_CCACHE),y)
CCACHE := $(HOST_DIR)/bin/ccache
BR_CACHE_DIR ?= $(call qstrip,$(BR2_CCACHE_DIR))
export BR_CACHE_DIR
HOSTCC := $(CCACHE) $(HOSTCC)
HOSTCXX := $(CCACHE) $(HOSTCXX)
else
export BR_NO_CCACHE
endif

# Scripts in support/ or post-build scripts may need to reference
# these locations, so export them so it is easier to use
export BR2_CONFIG
export BR2_REPRODUCIBLE
export TARGET_DIR
export STAGING_DIR
export HOST_DIR
export BINARIES_DIR
export BASE_DIR

################################################################################
#
# You should probably leave this stuff alone unless you know
# what you are doing.
#
################################################################################

all: world

# Include legacy before the other things, because package .mk files
# may rely on it.
include Makefile.legacy

include system/system.mk
include package/Makefile.in
# arch/arch.mk.* must be after package/Makefile.in because it may need to
# complement variables defined therein, like BR_NO_CHECK_HASH_FOR.
-include $(sort $(wildcard arch/arch.mk.*))
include support/dependencies/dependencies.mk

include $(sort $(wildcard toolchain/*.mk))
include $(sort $(wildcard toolchain/*/*.mk))

# Include the package override file if one has been provided in the
# configuration.
PACKAGE_OVERRIDE_FILE = $(call qstrip,$(BR2_PACKAGE_OVERRIDE_FILE))
ifneq ($(PACKAGE_OVERRIDE_FILE),)
-include $(PACKAGE_OVERRIDE_FILE)
endif

include $(sort $(wildcard package/*/*.mk))

include boot/common.mk
include linux/linux.mk
include fs/common.mk

# If using a br2-external tree, the BR2_EXTERNAL_$(NAME)_PATH variables
# are also present in the .config file. Since .config is included after
# we defined them in the Makefile, the values for those variables are
# quoted. We just include the generated Makefile fragment .br2-external.mk
# a third time, which will set those variables to the un-quoted values.
include $(BR2_EXTERNAL_FILE)

# Nothing to include if no BR2_EXTERNAL tree in use
include $(BR2_EXTERNAL_MKS)

# Now we are sure we have all the packages scanned and defined. We now
# check for each package in the list of enabled packages, that all its
# dependencies are indeed enabled.
#
# Only trigger the check for default builds. If the user forces building
# a package, even if not enabled in the configuration, we want to accept
# it.
#
ifeq ($(MAKECMDGOALS),)

define CHECK_ONE_DEPENDENCY
ifeq ($$($(2)_TYPE),target)
ifeq ($$($(2)_IS_VIRTUAL),)
ifneq ($$($$($(2)_KCONFIG_VAR)),y)
$$(error $$($(2)_NAME) is in the dependency chain of $$($(1)_NAME) that \
has added it to its _DEPENDENCIES variable without selecting it or \
depending on it from Config.in)
endif
endif
endif
endef

$(foreach pkg,$(call UPPERCASE,$(PACKAGES)),\
	$(foreach dep,$(call UPPERCASE,$($(pkg)_FINAL_ALL_DEPENDENCIES)),\
		$(eval $(call CHECK_ONE_DEPENDENCY,$(pkg),$(dep))$(sep))))

endif

# closure operator on package lists
# $(1) = suffixes that, appended to the uppercase package name, yield variables
#        containing other packages (e.g. DEPENDENCIES yields the transformation
#        "pkg" --> $(PKG_DEPENDENCIES))
# $(2) = list of packages
# $(3) = result package list, initially empty
PKG_CLOSURE = $(if $(filter-out $(3),$(2)),\
$(call PKG_CLOSURE,$(1),$(sort $(foreach pkg,$(2),$(pkg) $(foreach suffix,$(1),$($(call UPPERCASE,$(pkg))_$(suffix))))),$(2)),\
$(2))

# all (selected) targets with their direct and indirect dependencies
COMPLETE_TARGETS := $(strip $(call PKG_CLOSURE,DEPENDENCIES,$(PACKAGES) toolchain))

# all packages that build up the cross-compiling toolchain, including all
# selected host targets
TOOLCHAIN_TARGETS := $(strip $(sort $(call PKG_CLOSURE,DEPENDENCIES,toolchain) $(filter host-%,$(COMPLETE_TARGETS))))

# all (selected) non-toolchain non-host packages together with their non-host
# dependencies
TARGET_TARGETS := $(strip $(filter-out $(TOOLCHAIN_TARGETS),$(COMPLETE_TARGETS)))

# all (not necessarily selected) non-toolchain non-host packages together with
# their non-host dependencies
ALL_TARGET_TARGETS := $(strip $(filter-out host-% $(TOOLCHAIN_TARGETS),$(TARGETS)))

# returns all direct dependencies of a package, including "toolchain" for
# target packages (CONFIGURE dependencies are ignored)
# $(1) = package
GET_DEPS = $(strip $(sort $(if $(filter $(1),$(ALL_TARGET_TARGETS)),toolchain) $($(call UPPERCASE,$(1))_DEPENDENCIES)))

# returns all direct and indirect dependencies of a package (CONFIGURE
# dependencies are ignored)
# $(1) = package
GET_COMPLETE_DEPS = $(strip $(call PKG_CLOSURE,DEPENDENCIES,$(call GET_DEPS,$(1))))

# computes direct reverse dependencies of each package:
# for each package "pkg" PKG_DEPENDEES receives the packages that are directly
# depending on "pkg" (again excluding CONFIGURE dependencies)
CALC_REVERSE_DEPS = $(foreach p,$(sort $(TOOLCHAIN_TARGETS)),\
$(foreach dep,$($(call UPPERCASE,$(p))_DEPENDENCIES),\
$(eval $(call UPPERCASE,$(dep))_DEPENDEES += $(p)))) \
$(foreach p,$(ALL_TARGET_TARGETS),\
$(foreach dep,toolchain $($(call UPPERCASE,$(p))_DEPENDENCIES),\
$(eval $(call UPPERCASE,$(dep))_DEPENDEES += $(p))))

DEPENDENCIES_HOST_PREREQ_ALL := $(call PKG_CLOSURE,DEPENDENCIES,$(DEPENDENCIES_HOST_PREREQ))
HOST_MAKE_DEPS_ALL := $(call PKG_CLOSURE,DEPENDENCIES,host-make)

# all targets depend on their dependencies
$(eval $(foreach pkg,$(TARGETS),$(foreach dep,$($(call UPPERCASE,$(pkg))_FINAL_DEPENDENCIES),$($(call UPPERCASE,$(pkg))_TARGET_INSTALL_DEPS): $($(call UPPERCASE,$(dep))_TARGET_INSTALL)$(sep))))
# all patch targets depend on their patch dependencies (order-only dependency)
$(eval $(foreach pkg,$(TARGETS),$(foreach dep,$($(call UPPERCASE,$(pkg))_FINAL_PATCH_DEPENDENCIES),$($(call UPPERCASE,$(pkg))_TARGET_PATCH): | $($(call UPPERCASE,$(dep))_TARGET_INSTALL)$(sep))))

# all configure targets in DEPENDENCIES_HOST_PREREQ depend on "host-make", except host-make itself and its dependencies
$(eval $(foreach pkg,$(filter-out host-make $(HOST_MAKE_DEPS_ALL),$(DEPENDENCIES_HOST_PREREQ_ALL)),$($(call UPPERCASE,$(pkg))_TARGET_CONFIGURE): $(HOST_MAKE_TARGET_INSTALL_HOST)$(sep)))

# all install-host targets of host-make and its dependencies need to use the build host's make
$(eval $(foreach pkg,$(HOST_MAKE_DEPS_ALL),$($(call UPPERCASE,$(pkg))_TARGET_INSTALL_HOST): $(call UPPERCASE,$(pkg))_MAKE := $(MAKEORIG)$(sep)))

# all source targets not in DEPENDENCIES_HOST_PREREQ depend on "dependencies"
$(eval $(foreach pkg,$(filter-out $(DEPENDENCIES_HOST_PREREQ_ALL),$(TARGETS)),$($(call UPPERCASE,$(pkg))_TARGET_SOURCE): | dependencies $(HOST_DIR)/share/buildroot/toolchainfile.cmake $(HOST_DIR)/share/buildroot/Platform/Buildroot.cmake$(sep)))
$(eval $(foreach pkg,$(filter-out $(DEPENDENCIES_HOST_PREREQ_ALL),$(TARGETS)),$($(call UPPERCASE,$(pkg))_TARGET_RSYNC): | dependencies $(HOST_DIR)/share/buildroot/toolchainfile.cmake $(HOST_DIR)/share/buildroot/Platform/Buildroot.cmake$(sep)))

# all normal targets (e.g. non-toolchain non-host targets) depend on the target
# toolchain to be installed
$(eval $(foreach pkg,$(ALL_TARGET_TARGETS),$($(call UPPERCASE,$(pkg))_TARGET_INSTALL_DEPS): $(TOOLCHAIN_TARGET_INSTALL)$(sep)))

$(BUILD_DIR)/buildroot-config/auto.conf: PKG := BUILDROOT
$(BUILD_DIR)/buildroot-config/auto.conf: BUILDROOT_NAME := buildroot
$(BUILD_DIR)/buildroot-config/auto.conf: BUILDROOT_VERSION := $(BR2_VERSION)
$(BUILD_DIR)/buildroot-config/auto.conf: $(DIR_TARGETS) $(BR2_CONFIG)
	@$(call MESSAGE,Preparing)
	+$(MAKEORIG) $(EXTRAMAKEARGS) HOSTCC="$(HOSTCC_NOCCACHE)" HOSTCXX="$(HOSTCXX_NOCCACHE)" BR2_CONFIG="$(BR2_CONFIG)" silentoldconfig

.PHONY: world
world: target-post-image

.PHONY: sdk
sdk: world
	@$(call MESSAGE,"Rendering the SDK relocatable")
	$(TOPDIR)/support/scripts/fix-rpath host
	$(TOPDIR)/support/scripts/fix-rpath staging
	$(INSTALL) -m 755 $(TOPDIR)/support/misc/relocate-sdk.sh $(HOST_DIR)/relocate-sdk.sh
	echo $(HOST_DIR) > $(HOST_DIR)/share/buildroot/sdk-location

# Populating the staging with the base directories is handled by the skeleton package
$(STAMP_DIR)/.root-staging: PKG := BUILDROOT
$(STAMP_DIR)/.root-staging: BUILDROOT_NAME := buildroot
$(STAMP_DIR)/.root-staging: BUILDROOT_VERSION := $(BR2_VERSION)
$(STAMP_DIR)/.root-staging: BUILDROOT_SANDBOX_DIR := $(SANDBOX_DIR)/buildroot
$(STAMP_DIR)/.root-staging: $(HOST_DIR)/.root $(SANDBOX_DIR)/.root
	@$(call MESSAGE,Creating staging directories)
	@mkdir -p $(STAGING_DIR)
	@ln -snf $(STAGING_DIR) $(BASE_DIR)/staging
	$(call MARK_ORPHANS,BUILDROOT,staging,$(STAGING_DIR))
	@touch $@

RSYNC_VCS_EXCLUSIONS = \
	--exclude .svn --exclude .git --exclude .hg --exclude .bzr \
	--exclude CVS

$(STAMP_DIR)/.root-target: PKG := BUILDROOT
$(STAMP_DIR)/.root-target: BUILDROOT_NAME := buildroot
$(STAMP_DIR)/.root-target: BUILDROOT_VERSION := $(BR2_VERSION)
$(STAMP_DIR)/.root-target: BUILDROOT_SANDBOX_DIR := $(SANDBOX_DIR)/buildroot
$(STAMP_DIR)/.root-target: $(SANDBOX_DIR)/.root
	@$(call MESSAGE,Creating target directories)
	mkdir -p $(TARGET_DIR)

	mkdir -p $(TARGET_DIR)/etc $(TARGET_DIR)/usr/lib
	# Mandatory configuration file and auxiliary cache directory
	# for recent versions of ldconfig
	touch $(TARGET_DIR)/etc/ld.so.conf

	( \
		echo "NAME=Buildroot"; \
		echo "VERSION=$(BR2_VERSION_FULL)"; \
		echo "ID=buildroot"; \
		echo "VERSION_ID=$(BR2_VERSION)"; \
		echo "PRETTY_NAME=\"Buildroot $(BR2_VERSION)\"" \
	) >  $(TARGET_DIR)/usr/lib/os-release
	ln -sf ../usr/lib/os-release $(TARGET_DIR)/etc

	$(call MARK_ORPHANS,BUILDROOT,target,$(TARGET_DIR))
	@touch $@

$(BUILD_DIR)/.root: PKG := BUILDROOT
$(BUILD_DIR)/.root: BUILDROOT_NAME := buildroot
$(BUILD_DIR)/.root: BUILDROOT_VERSION := $(BR2_VERSION)
$(BUILD_DIR)/.root:
	@$(call MESSAGE,Creating output directory)
	mkdir -p $(@D)
ifeq ($(BR2_ENABLE_LOCALE_PURGE),y)
	rm -f $(LOCALE_WHITELIST)
	for i in $(LOCALE_NOPURGE) locale-archive; do echo $$i >> $(LOCALE_WHITELIST); done
endif
	@touch $@

$(HOST_DIR)/.root: PKG := BUILDROOT
$(HOST_DIR)/.root: BUILDROOT_NAME := buildroot
$(HOST_DIR)/.root: BUILDROOT_VERSION := $(BR2_VERSION)
$(HOST_DIR)/.root:
	@$(call MESSAGE,Creating directory for host packages)
	mkdir -p $(@D) $(@D)/lib
# Compatibility symlink in case a post-build script still uses $(HOST_DIR)/usr
	@ln -snf . $(HOST_DIR)/usr
	@case $(HOSTARCH) in \
		(*64) ln -snf lib $(@D)/lib64;; \
		(*)   ln -snf lib $(@D)/lib32;; \
	esac
	@touch $@

$(BINARIES_DIR)/.root: PKG := BUILDROOT
$(BINARIES_DIR)/.root: BUILDROOT_NAME := buildroot
$(BINARIES_DIR)/.root: BUILDROOT_VERSION := $(BR2_VERSION)
$(BINARIES_DIR)/.root:
	@$(call MESSAGE,Creating directory for images)
	mkdir -p $(@D)
	@touch $@

$(STAMP_DIR)/.root: PKG := BUILDROOT
$(STAMP_DIR)/.root: BUILDROOT_NAME := buildroot
$(STAMP_DIR)/.root: BUILDROOT_VERSION := $(BR2_VERSION)
$(STAMP_DIR)/.root:
	@$(call MESSAGE,Creating directory for stamps)
	mkdir -p $(@D)
	@touch $@

$(SANDBOX_DIR)/.root: PKG := BUILDROOT
$(SANDBOX_DIR)/.root: BUILDROOT_NAME := buildroot
$(SANDBOX_DIR)/.root: BUILDROOT_VERSION := $(BR2_VERSION)
$(SANDBOX_DIR)/.root:
	@$(call MESSAGE,Creating directory for sandboxes)
	mkdir -p $(@D)
	@touch $@

$(DEBUG_DIR)/.root: PKG := BUILDROOT
$(DEBUG_DIR)/.root: BUILDROOT_NAME := buildroot
$(DEBUG_DIR)/.root: BUILDROOT_VERSION := $(BR2_VERSION)
$(DEBUG_DIR)/.root:
	@$(call MESSAGE,Creating directory for debug information)
	mkdir -p $(@D)
	@touch $@

STRIP_FIND_CMD = find $(TARGET_DIR)
ifneq (,$(call qstrip,$(BR2_STRIP_EXCLUDE_DIRS)))
STRIP_FIND_CMD += \( $(call finddirclauses,$(TARGET_DIR),$(call qstrip,$(BR2_STRIP_EXCLUDE_DIRS))) \) -prune -o
endif
STRIP_FIND_CMD += -type f \( -perm /111 -o -name '*.so*' \)
# file exclusions:
# - libpthread.so: a non-stripped libpthread shared library is needed for
#   proper debugging of pthread programs using gdb.
# - ld.so: a non-stripped dynamic linker library is needed for valgrind
# - kernel modules (*.ko): do not function properly when stripped like normal
#   applications and libraries. Normally kernel modules are already excluded
#   by the executable permission check above, so the explicit exclusion is only
#   done for kernel modules with incorrect permissions.
STRIP_FIND_CMD += -not \( $(call findfileclauses,libpthread*.so* ld-*.so* *.ko $(call qstrip,$(BR2_STRIP_EXCLUDE_FILES))) \) -print0

ifeq ($(BR2_ECLIPSE_REGISTER),y)
define TOOLCHAIN_ECLIPSE_REGISTER
	./support/scripts/eclipse-register-toolchain `readlink -f $(O)` \
		$(notdir $(TARGET_CROSS)) $(BR2_ARCH)
endef
TARGET_FINALIZE_HOOKS += TOOLCHAIN_ECLIPSE_REGISTER
endif

# Generate locale data. Basically, we call the localedef program
# (built by the host-localedef package) for each locale. The input
# data comes preferably from the toolchain, or if the toolchain does
# not have them (Linaro toolchains), we use the ones available on the
# host machine.
ifeq ($(BR2_TOOLCHAIN_USES_GLIBC),y)
GLIBC_GENERATE_LOCALES = $(call qstrip,$(BR2_GENERATE_LOCALE))
ifneq ($(GLIBC_GENERATE_LOCALES),)
PACKAGES += host-localedef

define GENERATE_GLIBC_LOCALES
	$(Q)mkdir -p $(TARGET_DIR)/usr/lib/locale/
	$(Q)for locale in $(GLIBC_GENERATE_LOCALES) ; do \
		inputfile=`echo $${locale} | cut -f1 -d'.'` ; \
		charmap=`echo $${locale} | cut -f2 -d'.' -s` ; \
		if test -z "$${charmap}" ; then \
			charmap="UTF-8" ; \
		fi ; \
		echo "Generating locale $${inputfile}.$${charmap}" ; \
		I18NPATH=$(STAGING_DIR)/usr/share/i18n:/usr/share/i18n \
		$(HOST_DIR)/bin/localedef \
			--prefix=$(TARGET_DIR) \
			--$(call LOWERCASE,$(BR2_ENDIAN))-endian \
			-i $${inputfile} -f $${charmap} \
			$${locale} ; \
	done
endef
TARGET_FINALIZE_HOOKS += GENERATE_GLIBC_LOCALES
endif
endif

ifeq ($(BR2_ENABLE_LOCALE_PURGE),y)
LOCALE_WHITELIST = $(BUILD_DIR)/locales.nopurge
LOCALE_NOPURGE = $(call qstrip,$(BR2_ENABLE_LOCALE_WHITELIST))
endif

$(TARGETS_ROOTFS): $(BUILD_DIR)/.finalized

.PHONY: target-finalize
target-finalize: $(BUILD_DIR)/.finalized

$(BUILD_DIR)/.finalized: PKG := BUILDROOT
$(BUILD_DIR)/.finalized: BUILDROOT_NAME := buildroot
$(BUILD_DIR)/.finalized: BUILDROOT_VERSION := $(BR2_VERSION)
$(BUILD_DIR)/.finalized: $(foreach tgt,$(PACKAGES),$($(call UPPERCASE,$(tgt))_TARGET_INSTALL))
	@$(call MESSAGE,"Finalizing target directory")
	# Check files that are touched by more than one package
#	./support/scripts/check-uniq-files -t target $(BUILD_DIR)/packages-file-list.txt
#	./support/scripts/check-uniq-files -t staging $(BUILD_DIR)/packages-file-list-staging.txt
#	./support/scripts/check-uniq-files -t host $(BUILD_DIR)/packages-file-list-host.txt
	$(foreach hook,$(TARGET_FINALIZE_HOOKS),$($(hook))$(sep))

	@$(call MESSAGE,"Sanitizing RPATH in target tree")
	$(TOPDIR)/support/scripts/fix-rpath target

	@$(foreach d, $(call qstrip,$(BR2_ROOTFS_OVERLAY)), \
		$(call MESSAGE,"Copying overlay $(d)"); \
		rsync -a --ignore-times --keep-dirlinks $(RSYNC_VCS_EXCLUSIONS) \
			--chmod=u=rwX,go=rX --exclude .empty --exclude '*~' \
			$(d)/ $(TARGET_DIR)$(sep))

	@$(foreach s, $(call qstrip,$(BR2_ROOTFS_POST_BUILD_SCRIPT)), \
		$(call MESSAGE,"Executing post-build script $(s)"); \
		$(EXTRA_ENV) $(s) $(TARGET_DIR) $(call qstrip,$(BR2_ROOTFS_POST_SCRIPT_ARGS))$(sep))
	
	@touch $@

	touch $(TARGET_DIR)/usr

.PHONY: target-post-image
target-post-image: $(TARGETS_ROOTFS) $(BUILD_DIR)/.finalized
	@$(foreach s, $(call qstrip,$(BR2_ROOTFS_POST_IMAGE_SCRIPT)), \
		$(call MESSAGE,"Executing post-image script $(s)"); \
		$(EXTRA_ENV) $(s) $(BINARIES_DIR) $(call qstrip,$(BR2_ROOTFS_POST_SCRIPT_ARGS))$(sep))

.PHONY: source
source: $(foreach p,$(PACKAGES),$(p)-all-source)

.PHONY: _external-deps external-deps
_external-deps: $(foreach p,$(PACKAGES),$(p)-all-external-deps)
external-deps:
	@$(MAKEORIG) -j1 -Bs $(EXTRAMAKEARGS) _external-deps | sort -u

.PHONY: legal-info-clean
legal-info-clean:
	@rm -fr $(LEGAL_INFO_DIR)

.PHONY: legal-info-prepare
legal-info-prepare:
	@rm -fr $(LEGAL_INFO_DIR)
	@mkdir $(LEGAL_INFO_DIR)
	@mkdir $(REDIST_SOURCES_DIR_HOST)
	@mkdir $(REDIST_SOURCES_DIR_TARGET)
	@mkdir $(LICENSE_FILES_DIR_HOST)
	@mkdir $(LICENSE_FILES_DIR_TARGET)
	@cp $(BR2_CONFIG) $(LEGAL_INFO_DIR)/buildroot.config

.PHONY: legal-info
legal-info: legal-info-prepare $(foreach p,$(PACKAGES),$(p)-all-legal-info)
	@$(call MESSAGE,"Buildroot $(BR2_VERSION_FULL) Collecting legal info")
	@$(call legal-license-file,buildroot,buildroot,support/legal-info,COPYING,COPYING,HOST)
	@$(call legal-manifest,buildroot,$(BR2_VERSION_FULL),GPLv2+,COPYING,not saved,not saved,HOST)

	@sort -o $(LEGAL_MANIFEST_CSV_TARGET).tmp $(LEGAL_MANIFEST_CSV_TARGET)
	@: > $(LEGAL_MANIFEST_CSV_TARGET)
	@$(call legal-manifest,PACKAGE,VERSION,LICENSE,LICENSE FILES,SOURCE ARCHIVE,SOURCE SITE,TARGET)
	@cat $(LEGAL_MANIFEST_CSV_TARGET).tmp >> $(LEGAL_MANIFEST_CSV_TARGET)
	@rm $(LEGAL_MANIFEST_CSV_TARGET).tmp

	@sort -o $(LEGAL_MANIFEST_CSV_HOST).tmp $(LEGAL_MANIFEST_CSV_HOST)
	@: > $(LEGAL_MANIFEST_CSV_HOST)
	@$(call legal-manifest,PACKAGE,VERSION,LICENSE,LICENSE FILES,SOURCE ARCHIVE,SOURCE SITE,HOST)
	@cat $(LEGAL_MANIFEST_CSV_HOST).tmp >> $(LEGAL_MANIFEST_CSV_HOST)
	@rm $(LEGAL_MANIFEST_CSV_HOST).tmp

	@cat support/legal-info/README.header >>$(LEGAL_REPORT)
	@if [ -r $(LEGAL_WARNINGS) ]; then \
		sort -o $(LEGAL_WARNINGS) $(LEGAL_WARNINGS); \
	fi
	@$(call legal-warning,the Buildroot source code has not been saved)
	@if [ -r $(LEGAL_WARNINGS) ]; then \
		cat support/legal-info/README.warnings-header \
			$(LEGAL_WARNINGS) >>$(LEGAL_REPORT); \
		cat $(LEGAL_WARNINGS); fi
	@rm -f $(LEGAL_WARNINGS)
	@(cd $(LEGAL_INFO_DIR); \
		find * -type f -exec sha256sum {} + | LC_ALL=C sort -k2 \
			>.legal-info.sha256; \
		mv .legal-info.sha256 legal-info.sha256)
	@echo "Legal info produced in $(LEGAL_INFO_DIR)"

.PHONY: show-targets
show-targets:
	@echo $(sort $(PACKAGES)) $(sort $(TARGETS_ROOTFS))

.PHONY: show-build-order
show-build-order: $(patsubst %,%-show-build-order,$(PACKAGES))

.PHONY: show-versions
show-versions:
	@echo -e $(foreach p,$(sort $(COMPLETE_TARGETS)),"\n$(p) $($(call UPPERCASE,$(p))_VERSION)")

.PHONY: show-archives
show-archives:
	@echo -e $(foreach p,$(sort $(COMPLETE_TARGETS)),"\n$(p): $($(call UPPERCASE,$(p))_VERSION) $($(call UPPERCASE,$(p))_SITE_METHOD) $($(call UPPERCASE,$(p))_SITE) $(notdir $($(call UPPERCASE,$(p))_SOURCE)) $(notdir $($(call UPPERCASE,$(p))_PATCH)) $(notdir $($(call UPPERCASE,$(p))_EXTRA_DOWNLOADS))")

.PHONY: show-deps
show-deps:
	@echo -e $(foreach p,$(sort $(COMPLETE_TARGETS)),"\n$(p): $(call GET_DEPS,$(p))")

.PHONy: show-deps-complete
show-deps-complete:
	@echo -e $(foreach p,$(sort $(COMPLETE_TARGETS)),"\n$(p): $(call GET_COMPLETE_DEPS,$(p))")

.PHONY: show-reverse-deps
show-reverse-deps:
	$(call CALC_REVERSE_DEPS)
	$(foreach p,$(sort $(COMPLETE_TARGETS)),@echo "$(p): $(strip $(sort $($(call UPPERCASE,$(p))_DEPENDEES)))"$(sep))

.PHONY: show-reverse-deps-complete
show-reverse-deps-complete:
	$(call CALC_REVERSE_DEPS)
	$(foreach p,$(sort $(COMPLETE_TARGETS)),@echo "$(p): $(strip $(call PKG_CLOSURE,DEPENDEES,$(sort $($(call UPPERCASE,$(p))_DEPENDEES)),))"$(sep))

.PHONY: graph-build
graph-build: $(O)/build/build-time.log
	@install -d $(GRAPHS_DIR)
	$(foreach o,name build duration,./support/scripts/graph-build-time \
					--type=histogram --order=$(o) --input=$(<) \
					--output=$(GRAPHS_DIR)/build.hist-$(o).$(BR_GRAPH_OUT) \
					$(if $(BR2_GRAPH_ALT),--alternate-colors)$(sep))
	$(foreach t,packages steps,./support/scripts/graph-build-time \
				   --type=pie-$(t) --input=$(<) \
				   --output=$(GRAPHS_DIR)/build.pie-$(t).$(BR_GRAPH_OUT) \
				   $(if $(BR2_GRAPH_ALT),--alternate-colors)$(sep))

.PHONY: graph-depends-requirements
graph-depends-requirements:
	@dot -? >/dev/null 2>&1 || \
		{ echo "ERROR: The 'dot' program from Graphviz is needed for graph-depends" >&2; exit 1; }

.PHONY: graph-depends
graph-depends: graph-depends-requirements
	@$(INSTALL) -d $(GRAPHS_DIR)
	@cd "$(CONFIG_DIR)"; \
	$(TOPDIR)/support/scripts/graph-depends $(BR2_GRAPH_DEPS_OPTS) \
		--direct -o $(GRAPHS_DIR)/$(@).dot
	dot $(BR2_GRAPH_DOT_OPTS) -T$(BR_GRAPH_OUT) \
		-o $(GRAPHS_DIR)/$(@).$(BR_GRAPH_OUT) \
		$(GRAPHS_DIR)/$(@).dot

.PHONY: graph-size
graph-size:
	$(Q)mkdir -p $(GRAPHS_DIR)
	$(Q)$(TOPDIR)/support/scripts/size-stats --builddir $(BASE_DIR) \
		--graph $(GRAPHS_DIR)/graph-size.$(BR_GRAPH_OUT) \
		--file-size-csv $(GRAPHS_DIR)/file-size-stats.csv \
		--package-size-csv $(GRAPHS_DIR)/package-size-stats.csv

.PHONY: check-dependencies
check-dependencies:
	@cd "$(CONFIG_DIR)"; \
	$(TOPDIR)/support/scripts/graph-depends -C

else # ifeq ($(BR2_HAVE_DOT_CONFIG),y)

# Some subdirectories are also package names. To avoid that "make linux"
# on an unconfigured tree produces "Nothing to be done", add an explicit
# rule for it.
# Also for 'all' we error out and ask the user to configure first.
.PHONY: linux toolchain
linux toolchain all: outputmakefile
	$(error Please configure Buildroot first (e.g. "make menuconfig"))
	@exit 1

endif # ifeq ($(BR2_HAVE_DOT_CONFIG),y)

# configuration
# ---------------------------------------------------------------------------

HOSTCFLAGS = $(CFLAGS_FOR_BUILD)
export HOSTCFLAGS

.PHONY: prepare-kconfig
prepare-kconfig: outputmakefile $(BUILD_DIR)/.br2-external.in

$(BUILD_DIR)/buildroot-config/%onf:
	mkdir -p $(@D)/lxdialog
	PKG_CONFIG_PATH="$(HOST_PKG_CONFIG_PATH)" $(MAKE) CC="$(HOSTCC_NOCCACHE)" HOSTCC="$(HOSTCC_NOCCACHE)" \
	    obj=$(@D) -C $(CONFIG) -f Makefile.br $(@F)

DEFCONFIG = $(call qstrip,$(BR2_DEFCONFIG))

# We don't want to fully expand BR2_DEFCONFIG here, so Kconfig will
# recognize that if it's still at its default $(CONFIG_DIR)/defconfig
COMMON_CONFIG_ENV = \
	BR2_DEFCONFIG='$(call qstrip,$(value BR2_DEFCONFIG))' \
	KCONFIG_AUTOCONFIG=$(BUILD_DIR)/buildroot-config/auto.conf \
	KCONFIG_AUTOHEADER=$(BUILD_DIR)/buildroot-config/autoconf.h \
	KCONFIG_TRISTATE=$(BUILD_DIR)/buildroot-config/tristate.config \
	BR2_CONFIG=$(BR2_CONFIG) \
	HOST_GCC_VERSION="$(HOSTCC_VERSION)" \
	BUILD_DIR=$(BUILD_DIR) \
	SKIP_LEGACY=

xconfig: $(BUILD_DIR)/buildroot-config/qconf prepare-kconfig
	@$(COMMON_CONFIG_ENV) $< $(CONFIG_CONFIG_IN)

gconfig: $(BUILD_DIR)/buildroot-config/gconf prepare-kconfig
	@$(COMMON_CONFIG_ENV) srctree=$(TOPDIR) $< $(CONFIG_CONFIG_IN)

menuconfig: $(BUILD_DIR)/buildroot-config/mconf prepare-kconfig
	@$(COMMON_CONFIG_ENV) $< $(CONFIG_CONFIG_IN)

nconfig: $(BUILD_DIR)/buildroot-config/nconf prepare-kconfig
	@$(COMMON_CONFIG_ENV) $< $(CONFIG_CONFIG_IN)

config: $(BUILD_DIR)/buildroot-config/conf prepare-kconfig
	@$(COMMON_CONFIG_ENV) $< $(CONFIG_CONFIG_IN)

# For the config targets that automatically select options, we pass
# SKIP_LEGACY=y to disable the legacy options. However, in that case
# no values are set for the legacy options so a subsequent oldconfig
# will query them. Therefore, run an additional olddefconfig.

randconfig allyesconfig alldefconfig allnoconfig: $(BUILD_DIR)/buildroot-config/conf prepare-kconfig
	@$(COMMON_CONFIG_ENV) SKIP_LEGACY=y $< --$@ $(CONFIG_CONFIG_IN)
	@$(COMMON_CONFIG_ENV) $< --olddefconfig $(CONFIG_CONFIG_IN) >/dev/null

randpackageconfig allyespackageconfig allnopackageconfig: $(BUILD_DIR)/buildroot-config/conf prepare-kconfig
	@grep -v BR2_PACKAGE_ $(BR2_CONFIG) > $(CONFIG_DIR)/.config.nopkg
	@$(COMMON_CONFIG_ENV) SKIP_LEGACY=y \
		KCONFIG_ALLCONFIG=$(CONFIG_DIR)/.config.nopkg \
		$< --$(subst package,,$@) $(CONFIG_CONFIG_IN)
	@rm -f $(CONFIG_DIR)/.config.nopkg
	@$(COMMON_CONFIG_ENV) $< --olddefconfig $(CONFIG_CONFIG_IN) >/dev/null

oldconfig silentoldconfig olddefconfig: $(BUILD_DIR)/buildroot-config/conf prepare-kconfig
	@$(COMMON_CONFIG_ENV) $< --$@ $(CONFIG_CONFIG_IN)

defconfig: $(BUILD_DIR)/buildroot-config/conf prepare-kconfig
	@$(COMMON_CONFIG_ENV) $< --defconfig$(if $(DEFCONFIG),=$(DEFCONFIG)) $(CONFIG_CONFIG_IN)

define percent_defconfig
# Override the BR2_DEFCONFIG from COMMON_CONFIG_ENV with the new defconfig
%_defconfig: $(BUILD_DIR)/buildroot-config/conf $(1)/configs/%_defconfig prepare-kconfig
	@$$(COMMON_CONFIG_ENV) BR2_DEFCONFIG=$(1)/configs/$$@ \
		$$< --defconfig=$(1)/configs/$$@ $$(CONFIG_CONFIG_IN)
endef
$(eval $(foreach d,$(call reverse,$(TOPDIR) $(BR2_EXTERNAL_DIRS)),$(call percent_defconfig,$(d))$(sep)))

savedefconfig: $(BUILD_DIR)/buildroot-config/conf prepare-kconfig
	@$(COMMON_CONFIG_ENV) $< \
		--savedefconfig=$(if $(DEFCONFIG),$(DEFCONFIG),$(CONFIG_DIR)/defconfig) \
		$(CONFIG_CONFIG_IN)
	@$(SED) '/BR2_DEFCONFIG=/d' $(if $(DEFCONFIG),$(DEFCONFIG),$(CONFIG_DIR)/defconfig)

.PHONY: defconfig savedefconfig

################################################################################
#
# Cleanup and misc junk
#
################################################################################

# outputmakefile generates a Makefile in the output directory, if using a
# separate output directory. This allows convenient use of make in the
# output directory.
.PHONY: outputmakefile
outputmakefile:
ifeq ($(NEED_WRAPPER),y)
	$(Q)$(TOPDIR)/support/scripts/mkmakefile $(TOPDIR) $(O)
endif

# Even though the target is a real file, we mark it as PHONY as we
# want it to be re-generated each time make is invoked, in case the
# value of BR2_EXTERNAL is changed.
.PHONY: $(BUILD_DIR)/.br2-external.in
$(BUILD_DIR)/.br2-external.in:
	mkdir -p $(@D)
	$(Q)support/scripts/br2-external -k -o "$(@)" $(BR2_EXTERNAL)

# printvars prints all the variables currently defined in our
# Makefiles. Alternatively, if a non-empty VARS variable is passed,
# only the variables matching the make pattern passed in VARS are
# displayed.
.PHONY: printvars
printvars:
	@:
	$(foreach V, \
		$(sort $(if $(VARS),$(filter $(VARS),$(.VARIABLES)),$(.VARIABLES))), \
		$(if $(filter-out environment% default automatic, \
				$(origin $V)), \
		$(if $(QUOTED_VARS),\
			$(info $V='$(subst ','\'',$(if $(RAW_VARS),$(value $V),$($V)))'), \
			$(info $V=$(if $(RAW_VARS),$(value $V),$($V))))))
# ' Syntax colouring...

.PHONY: clean
clean:
	rm -rf $(TARGET_DIR) $(BINARIES_DIR) $(HOST_DIR) \
		$(STAMP_DIR) $(SANDBOX_DIR) $(DEBUG_DIR) \
		$(BUILD_DIR) $(BASE_DIR)/staging \
		$(LEGAL_INFO_DIR) $(GRAPHS_DIR)

.PHONY: distclean
distclean: clean
ifeq ($(O),$(CURDIR)/output)
	rm -rf $(O)
endif
	rm -rf $(TOPDIR)/dl $(BR2_CONFIG) $(CONFIG_DIR)/.config.old $(CONFIG_DIR)/..config.tmp \
		$(CONFIG_DIR)/.auto.deps $(BR2_EXTERNAL_FILE)

.PHONY: help
help:
	@echo 'Cleaning:'
	@echo '  clean                  - delete all files created by build'
	@echo '  distclean              - delete all non-source files (including .config)'
	@echo
	@echo 'Build:'
	@echo '  all                    - make world'
	@echo '  toolchain              - build toolchain'
	@echo '  sdk                    - build relocatable SDK'
	@echo
	@echo 'Configuration:'
	@echo '  menuconfig             - interactive curses-based configurator'
	@echo '  nconfig                - interactive ncurses-based configurator'
	@echo '  xconfig                - interactive Qt-based configurator'
	@echo '  gconfig                - interactive GTK-based configurator'
	@echo '  oldconfig              - resolve any unresolved symbols in .config'
	@echo '  silentoldconfig        - Same as oldconfig, but quietly, additionally update deps'
	@echo '  olddefconfig           - Same as silentoldconfig but sets new symbols to their default value'
	@echo '  randconfig             - New config with random answer to all options'
	@echo '  defconfig              - New config with default answer to all options;'
	@echo '                             BR2_DEFCONFIG, if set on the command line, is used as input'
	@echo '  savedefconfig          - Save current config to BR2_DEFCONFIG (minimal config)'
	@echo '  allyesconfig           - New config where all options are accepted with yes'
	@echo '  allnoconfig            - New config where all options are answered with no'
	@echo '  alldefconfig           - New config where all options are set to default'
	@echo '  randpackageconfig      - New config with random answer to package options'
	@echo '  allyespackageconfig    - New config where pkg options are accepted with yes'
	@echo '  allnopackageconfig     - New config where package options are answered with no'
	@echo
	@echo 'Package-specific:'
	@echo '  <pkg>                  - Build and install <pkg> and all its dependencies'
	@echo '  <pkg>-source           - Only download the source files for <pkg>'
	@echo '  <pkg>-extract          - Extract <pkg> sources'
	@echo '  <pkg>-patch            - Apply patches to <pkg>'
	@echo '  <pkg>-depends          - Build <pkg>'\''s dependencies'
	@echo '  <pkg>-configure        - Build <pkg> up to the configure step'
	@echo '  <pkg>-build            - Build <pkg> up to the build step'
	@echo '  <pkg>-show-depends     - List packages on which <pkg> depends'
	@echo '  <pkg>-show-rdepends    - List packages which have <pkg> as a dependency'
	@echo '  <pkg>-graph-depends    - Generate a graph of <pkg>'\''s dependencies'
	@echo '  <pkg>-graph-rdepends   - Generate a graph of <pkg>'\''s reverse dependencies'
	@echo '  <pkg>-dirclean         - Remove <pkg> build directory'
	@echo '  <pkg>-reconfigure      - Restart the build from the configure step'
	@echo '  <pkg>-rebuild          - Restart the build from the build step'
	$(foreach p,$(HELP_PACKAGES), \
		@echo $(sep) \
		@echo '$($(p)_NAME):' $(sep) \
		$($(p)_HELP_CMDS)$(sep))
	@echo
	@echo 'Documentation:'
	@echo '  manual                 - build manual in all formats'
	@echo '  manual-html            - build manual in HTML'
	@echo '  manual-split-html      - build manual in split HTML'
	@echo '  manual-pdf             - build manual in PDF'
	@echo '  manual-text            - build manual in text'
	@echo '  manual-epub            - build manual in ePub'
	@echo '  graph-build            - generate graphs of the build times'
	@echo '  graph-depends          - generate graph of the dependency tree'
	@echo '  graph-size             - generate stats of the filesystem size'
	@echo '  list-defconfigs        - list all defconfigs (pre-configured minimal systems)'
	@echo
	@echo 'Miscellaneous:'
	@echo '  source                 - download all sources needed for offline-build'
	@echo '  external-deps          - list external packages used'
	@echo '  legal-info             - generate info about license compliance'
	@echo '  printvars              - dump all the internal variables'
	@echo
	@echo '  make V=0|1             - 0 => quiet build (default), 1 => verbose build'
	@echo '  make O=dir             - Locate all output files in "dir", including .config'
	@echo
	@echo 'For further details, see README, generate the Buildroot manual, or consult'
	@echo 'it on-line at http://buildroot.org/docs.html'
	@echo

# List the defconfig files
# $(1): base directory
# $(2): br2-external name, empty for bundled
define list-defconfigs
	@first=true; \
	for defconfig in $(1)/configs/*_defconfig; do \
		[ -f "$${defconfig}" ] || continue; \
		if $${first}; then \
			if [ "$(2)" ]; then \
				printf 'External configs in "$(call qstrip,$(2))":\n'; \
			else \
				printf "Built-in configs:\n"; \
			fi; \
			first=false; \
		fi; \
		defconfig="$${defconfig##*/}"; \
		printf "  %-35s - Build for %s\n" "$${defconfig}" "$${defconfig%_defconfig}"; \
	done; \
	$${first} || printf "\n"
endef

# We iterate over BR2_EXTERNAL_NAMES rather than BR2_EXTERNAL_DIRS,
# because we want to display the name of the br2-external tree.
.PHONY: list-defconfigs
list-defconfigs:
	$(call list-defconfigs,$(TOPDIR))
	$(foreach name,$(BR2_EXTERNAL_NAMES),\
		$(call list-defconfigs,$(BR2_EXTERNAL_$(name)_PATH),\
			$(BR2_EXTERNAL_$(name)_DESC))$(sep))

release: OUT = buildroot-$(BR2_VERSION)

# Create release tarballs. We need to fiddle a bit to add the generated
# documentation to the git output
release:
	git archive --format=tar --prefix=$(OUT)/ HEAD > $(OUT).tar
	$(MAKE) O=$(OUT) manual-html manual-text manual-pdf
	$(MAKE) O=$(OUT) manual-clean
	tar rf $(OUT).tar $(OUT)
	gzip -9 -c < $(OUT).tar > $(OUT).tar.gz
	bzip2 -9 -c < $(OUT).tar > $(OUT).tar.bz2
	rm -rf $(OUT) $(OUT).tar

print-version:
	@echo $(BR2_VERSION_FULL)

.PHONY: .gitlab-ci.yml
.gitlab-ci.yml: .gitlab-ci.yml.in
	cp $< $@
	(cd configs; LC_ALL=C ls -1 *_defconfig) | sed 's/$$/: *defconfig/' >> $@
	./support/testing/run-tests -l 2>&1 | sed -r -e '/^test_run \((.*)\).*/!d; s//\1: *runtime_test/' | LC_ALL=C sort >> $@

include docs/manual/manual.mk
-include $(foreach dir,$(BR2_EXTERNAL_DIRS),$(sort $(wildcard $(dir)/docs/*/*.mk)))

.PHONY: $(noconfig_targets)

endif #umask / $(CURDIR) / $(O)
