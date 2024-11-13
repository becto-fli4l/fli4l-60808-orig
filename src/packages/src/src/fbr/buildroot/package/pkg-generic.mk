################################################################################
# Generic package infrastructure
#
# This file implements an infrastructure that eases development of
# package .mk files. It should be used for packages that do not rely
# on a well-known build system for which Buildroot has a dedicated
# infrastructure (so far, Buildroot has special support for
# autotools-based and CMake-based packages).
#
# See the Buildroot documentation for details on the usage of this
# infrastructure
#
# In terms of implementation, this generic infrastructure requires the
# .mk file to specify:
#
#   1. Metadata information about the package: name, version,
#      download URL, etc.
#
#   2. Description of the commands to be executed to configure, build
#      and install the package
################################################################################

################################################################################
# Helper functions to catch start/end of each step
################################################################################

# Those two functions are called by each step below.
# They are responsible for calling all hooks defined in
# $(GLOBAL_INSTRUMENTATION_HOOKS) and pass each of them
# four arguments:
#   $1: either 'start' or 'end'
#   $2: the name of the step
#   $3: the name of the package
#   $4: the uppercase name of the package

# Start step
# $1: step name
define step_start
	$(foreach hook,$(GLOBAL_INSTRUMENTATION_HOOKS),$(call $(hook),start,$(1),$($(PKG)_NAME),$(PKG))$(sep))
endef

# End step
# $1: step name
define step_end
	$(foreach hook,$(GLOBAL_INSTRUMENTATION_HOOKS),$(call $(hook),end,$(1),$($(PKG)_NAME),$(PKG))$(sep))
endef

#######################################
# Actual steps hooks

# Time steps
define step_time
	printf "%s:%-5.5s:%-20.20s: %s\n"           \
	       "$$(date +%s)" "$(1)" "$(2)" "$(3)"  \
	       >>"$(BUILD_DIR)/build-time.log"
endef
GLOBAL_INSTRUMENTATION_HOOKS += step_time

# Hooks to collect statistics about installed files

# The suffix is typically empty for the target variant, for legacy backward
# compatibility.
# $(1): package name
# $(2): base directory to search in
# $(3): suffix of file  (optional)
define step_pkg_size_inner
	@touch $(BUILD_DIR)/packages-file-list$(3).txt
	$(SED) '/^$(1),/d' $(BUILD_DIR)/packages-file-list$(3).txt
	cd $(2); \
	find . \( -type f -o -type l \) \
		-newer $($(PKG)_DIR)/.stamp_built \
		-exec printf '$(1),%s\n' {} + \
		>> $(BUILD_DIR)/packages-file-list$(3).txt
endef

define step_pkg_size
	$(if $(filter install-target,$(2)),\
		$(if $(filter end,$(1)),$(call step_pkg_size_inner,$(3),$(TARGET_DIR))))
	$(if $(filter install-staging,$(2)),\
		$(if $(filter end,$(1)),$(call step_pkg_size_inner,$(3),$(STAGING_DIR),-staging)))
	$(if $(filter install-host,$(2)),\
		$(if $(filter end,$(1)),$(call step_pkg_size_inner,$(3),$(HOST_DIR),-host)))
endef
#GLOBAL_INSTRUMENTATION_HOOKS += step_pkg_size

# Relies on step_pkg_size, so must be after
define check_bin_arch
	$(if $(filter end-check-target-sandbox,$(1)-$(2)),\
		TARGET_DIR="$($(4)_SANDBOX_DIR)/target" support/scripts/check-bin-arch -p $(3) \
			-l $($(4)_SANDBOX_DIR)/target.installed \
			$(foreach i,$($(PKG)_BIN_ARCH_EXCLUDE),-i "$(i)") \
			-r $(TARGET_READELF) \
			-a $(BR2_READELF_ARCH_NAME))
endef

GLOBAL_INSTRUMENTATION_HOOKS += check_bin_arch

# This hook checks that host packages that need libraries that we build
# have a proper DT_RPATH or DT_RUNPATH tag
define check_host_rpath
	$(if $(filter install-host,$(2)),\
		$(if $(filter end,$(1)),support/scripts/check-host-rpath $(3) $(HOST_DIR)))
endef
GLOBAL_INSTRUMENTATION_HOOKS += check_host_rpath

define step_check_build_dir_one
	if [ -d $(2) ]; then \
		printf "%s: installs files in %s\n" $(1) $(2) >&2; \
		exit 1; \
	fi
endef

define step_check_build_dir
	$(if $(filter install-staging,$(2)),\
		$(if $(filter end,$(1)),$(call step_check_build_dir_one,$(3),$(STAGING_DIR)/$(O))))
	$(if $(filter install-target,$(2)),\
		$(if $(filter end,$(1)),$(call step_check_build_dir_one,$(3),$(TARGET_DIR)/$(O))))
endef
GLOBAL_INSTRUMENTATION_HOOKS += step_check_build_dir

# User-supplied script
ifneq ($(BR2_INSTRUMENTATION_SCRIPTS),)
define step_user
	@$(foreach user_hook, $(BR2_INSTRUMENTATION_SCRIPTS), \
		$(EXTRA_ENV) $(user_hook) "$(1)" "$(2)" "$(3)"$(sep))
endef
GLOBAL_INSTRUMENTATION_HOOKS += step_user
endif

################################################################################
# virt-provides-single -- check that provider-pkg is the declared provider for
# the virtual package virt-pkg
#
# argument 1 is the lower-case name of the virtual package
# argument 2 is the upper-case name of the virtual package
# argument 3 is the lower-case name of the provider
#
# example:
#   $(call virt-provides-single,libegl,LIBEGL,rpi-userland)
################################################################################
define virt-provides-single
ifneq ($$(call qstrip,$$(BR2_PACKAGE_PROVIDES_$(2))),$(3))
$$(error Configuration error: both "$(3)" and $$(BR2_PACKAGE_PROVIDES_$(2))\
are selected as providers for virtual package "$(1)". Only one provider can\
be selected at a time. Please fix your configuration)
endif
endef

define pkg-graph-depends
	@$$(INSTALL) -d $$(GRAPHS_DIR)
	@cd "$$(CONFIG_DIR)"; \
	$$(TOPDIR)/support/scripts/graph-depends $$(BR2_GRAPH_DEPS_OPTS) \
		-p $(1) $(2) -o $$(GRAPHS_DIR)/$$(@).dot
	dot $$(BR2_GRAPH_DOT_OPTS) -T$$(BR_GRAPH_OUT) \
		-o $$(GRAPHS_DIR)/$$(@).$$(BR_GRAPH_OUT) \
		$$(GRAPHS_DIR)/$$(@).dot
endef

################################################################################
# inner-generic-package -- generates the make targets needed to build a
# generic package
#
#  argument 1 is the lowercase package name
#  argument 2 is the uppercase package name, including a HOST_ prefix
#             for host packages
#  argument 3 is the uppercase package name, without the HOST_ prefix
#             for host packages
#  argument 4 is the type (target or host)
#
# Note about variable and function references: inside all blocks that are
# evaluated with $(eval), which includes all 'inner-xxx-package' blocks,
# specific rules apply with respect to variable and function references.
# - Numbered variables (parameters to the block) can be referenced with a single
#   dollar sign: $(1), $(2), $(3), etc.
# - pkgdir and pkgname should be referenced with a single dollar sign too. These
#   functions rely on 'the most recently parsed makefile' which is supposed to
#   be the package .mk file. If we defer the evaluation of these functions using
#   double dollar signs, then they may be evaluated too late, when other
#   makefiles have already been parsed. One specific case is when $$(pkgdir) is
#   assigned to a variable using deferred evaluation with '=' and this variable
#   is used in a target rule outside the eval'ed inner block. In this case, the
#   pkgdir will be that of the last makefile parsed by buildroot, which is not
#   the expected value. This mechanism is for example used for the TARGET_PATCH
#   rule.
# - All other variables should be referenced with a double dollar sign:
#   $$(TARGET_DIR), $$($(2)_VERSION), etc. Also all make functions should be
#   referenced with a double dollar sign: $$(subst), $$(call), $$(filter-out),
#   etc. This rule ensures that these variables and functions are only expanded
#   during the $(eval) step, and not earlier. Otherwise, unintuitive and
#   undesired behavior occurs with respect to these variables and functions.
#
################################################################################

define inner-generic-package

# Ensure the package is only declared once, i.e. do not accept that a
# package be re-defined by a br2-external tree
ifneq ($(call strip,$(filter $(1),$(PACKAGES_ALL))),)
$$(error Package '$(1)' defined a second time in '$(pkgdir)'; \
	previous definition was in '$$($(2)_PKGDIR)')
endif
PACKAGES_ALL += $(1)

# Define default values for various package-related variables, if not
# already defined. For some variables (version, source, site and
# subdir), if they are undefined, we try to see if a variable without
# the HOST_ prefix is defined. If so, we use such a variable, so that
# this information has only to be specified once, for both the
# target and host packages of a given .mk file.

$(2)_TYPE                       =  $(4)
$(2)_NAME			=  $(1)
$(2)_RAWNAME			=  $$(patsubst host-%,%,$(1))
$(2)_PKGDIR			=  $(pkgdir)

# Keep the package version that may contain forward slashes in the _DL_VERSION
# variable, then replace all forward slashes ('/') by underscores ('_') to
# sanitize the package version that is used in paths, directory and file names.
# Forward slashes may appear in the package's version when pointing to a
# version control system branch or tag, for example remotes/origin/1_10_stable.
# Similar for spaces and colons (:) that may appear in date-based revisions for
# CVS.
ifndef $(2)_VERSION
 ifdef $(3)_DL_VERSION
  $(2)_DL_VERSION := $$($(3)_DL_VERSION)
 else ifdef $(3)_VERSION
  $(2)_DL_VERSION := $$($(3)_VERSION)
 endif
else
 $(2)_DL_VERSION := $$(strip $$($(2)_VERSION))
endif
$(2)_VERSION := $$(call sanitize,$$($(2)_DL_VERSION))

ifdef $(3)_OVERRIDE_SRCDIR
  $(2)_OVERRIDE_SRCDIR ?= $$($(3)_OVERRIDE_SRCDIR)
endif

$(2)_BASENAME	= $$(if $$($(2)_VERSION),$(1)-$$($(2)_VERSION),$(1))
$(2)_BASENAME_RAW = $$(if $$($(2)_VERSION),$$($(2)_RAWNAME)-$$($(2)_VERSION),$$($(2)_RAWNAME))
$(2)_DL_DIR	=  $$(DL_DIR)
$(2)_DIR	=  $$(BUILD_DIR)/$$($(2)_BASENAME)
$(2)_SANDBOX_DIR =  $$(SANDBOX_DIR)/$$($(2)_BASENAME)

ifndef $(2)_SUBDIR
 ifdef $(3)_SUBDIR
  $(2)_SUBDIR = $$($(3)_SUBDIR)
 else
  $(2)_SUBDIR ?=
 endif
endif

ifndef $(2)_STRIP_COMPONENTS
 ifdef $(3)_STRIP_COMPONENTS
  $(2)_STRIP_COMPONENTS = $$($(3)_STRIP_COMPONENTS)
 else
  $(2)_STRIP_COMPONENTS ?= 1
 endif
endif

$(2)_SRCDIR		       = $$($(2)_DIR)/$$($(2)_SUBDIR)
$(2)_BUILDDIR		       ?= $$($(2)_SRCDIR)

ifneq ($$($(2)_OVERRIDE_SRCDIR),)
$(2)_VERSION = custom
endif

ifndef $(2)_SOURCE
 ifdef $(3)_SOURCE
  $(2)_SOURCE = $$($(3)_SOURCE)
 else ifdef $(2)_VERSION
  $(2)_SOURCE			?= $$($(2)_BASENAME_RAW).tar.gz
 endif
endif

# If FOO_ACTUAL_SOURCE_TARBALL is explicitly defined, it means FOO_SOURCE is
# indeed a binary (e.g. external toolchain) and FOO_ACTUAL_SOURCE_TARBALL/_SITE
# point to the actual sources tarball. Use the actual sources for legal-info.
# For most packages the FOO_SITE/FOO_SOURCE pair points to real source code,
# so these are the defaults for FOO_ACTUAL_*.
$(2)_ACTUAL_SOURCE_TARBALL ?= $$($(2)_SOURCE)
$(2)_ACTUAL_SOURCE_SITE    ?= $$(call qstrip,$$($(2)_SITE))

ifndef $(2)_PATCH
 ifdef $(3)_PATCH
  $(2)_PATCH = $$($(3)_PATCH)
 endif
endif

$(2)_ALL_DOWNLOADS = \
	$$(foreach p,$$($(2)_SOURCE) $$($(2)_PATCH) $$($(2)_EXTRA_DOWNLOADS),\
		$$(if $$(findstring ://,$$(p)),$$(p),\
			$$($(2)_SITE)/$$(p)))

ifndef $(2)_SITE
 ifdef $(3)_SITE
  $(2)_SITE = $$($(3)_SITE)
 endif
endif

ifndef $(2)_SITE_METHOD
 ifdef $(3)_SITE_METHOD
  $(2)_SITE_METHOD = $$($(3)_SITE_METHOD)
 else
	# Try automatic detection using the scheme part of the URI
	$(2)_SITE_METHOD = $$(call geturischeme,$$($(2)_SITE))
 endif
endif

# Do not accept to download git submodule if not using the git method
ifneq ($$($(2)_GIT_SUBMODULES),)
 ifneq ($$($(2)_SITE_METHOD),git)
  $$(error $(2) declares having git sub-modules, but does not use the \
	   'git' method (uses '$$($(2)_SITE_METHOD)' instead))
 endif
endif

ifeq ($$($(2)_SITE_METHOD),local)
ifeq ($$($(2)_OVERRIDE_SRCDIR),)
$(2)_OVERRIDE_SRCDIR = $$($(2)_SITE)
endif
ifeq ($$($(2)_OVERRIDE_SRCDIR),)
$$(error $(1) has local site method, but `$(2)_SITE` is not defined)
endif
endif

ifndef $(2)_LICENSE
 ifdef $(3)_LICENSE
  $(2)_LICENSE = $$($(3)_LICENSE)
 endif
endif

$(2)_LICENSE			?= unknown

ifndef $(2)_LICENSE_FILES
 ifdef $(3)_LICENSE_FILES
  $(2)_LICENSE_FILES = $$($(3)_LICENSE_FILES)
 endif
endif

ifndef $(2)_REDISTRIBUTE
 ifdef $(3)_REDISTRIBUTE
  $(2)_REDISTRIBUTE = $$($(3)_REDISTRIBUTE)
 endif
endif

$(2)_REDISTRIBUTE		?= YES

$(2)_REDIST_SOURCES_DIR = $$(REDIST_SOURCES_DIR_$$(call UPPERCASE,$(4)))/$$($(2)_BASENAME_RAW)

# When a target package is a toolchain dependency set this variable to
# 'NO' so the 'toolchain' dependency is not added to prevent a circular
# dependency.
# Similarly for the skeleton.
$(2)_ADD_TOOLCHAIN_DEPENDENCY	?= YES
$(2)_ADD_SKELETON_DEPENDENCY	?= YES


ifeq ($(4),target)
ifeq ($$($(2)_ADD_SKELETON_DEPENDENCY),YES)
$(2)_DEPENDENCIES += skeleton
endif
ifeq ($$($(2)_ADD_TOOLCHAIN_DEPENDENCY),YES)
$(2)_DEPENDENCIES += toolchain
endif
endif

# Eliminate duplicates in dependencies
$(2)_FINAL_DEPENDENCIES = $$(sort $$($(2)_DEPENDENCIES))
$(2)_FINAL_PATCH_DEPENDENCIES = $$(sort $$($(2)_PATCH_DEPENDENCIES))
$(2)_FINAL_ALL_DEPENDENCIES = $$(sort $$($(2)_FINAL_DEPENDENCIES) $$($(2)_FINAL_PATCH_DEPENDENCIES))

$(2)_INSTALL_STAGING		?= NO
$(2)_INSTALL_IMAGES		?= NO
$(2)_INSTALL_TARGET		?= YES

# define sub-target stamps
$(2)_TARGET_INSTALL =		$$($(2)_DIR)/.stamp_installed
$(2)_TARGET_INSTALL_TARGET =	$$($(2)_DIR)/.stamp_target_installed
$(2)_TARGET_CHECK_TARGET_SANDBOX = $$($(2)_DIR)/.stamp_target_sandbox_checked
$(2)_TARGET_INSTALL_TARGET_SANDBOX = $$($(2)_DIR)/.stamp_target_sandbox_installed
$(2)_TARGET_CREATE_TARGET_SANDBOX = $$($(2)_DIR)/.stamp_target_sandbox_created
$(2)_TARGET_INSTALL_STAGING =	$$($(2)_DIR)/.stamp_staging_installed
$(2)_TARGET_CHECK_STAGING_SANDBOX = $$($(2)_DIR)/.stamp_staging_sandbox_checked
$(2)_TARGET_INSTALL_STAGING_SANDBOX = $$($(2)_DIR)/.stamp_staging_sandbox_installed
$(2)_TARGET_CREATE_STAGING_SANDBOX = $$($(2)_DIR)/.stamp_staging_sandbox_created
$(2)_TARGET_INSTALL_IMAGES =	$$($(2)_DIR)/.stamp_images_installed
$(2)_TARGET_CHECK_IMAGES_SANDBOX = $$($(2)_DIR)/.stamp_images_sandbox_checked
$(2)_TARGET_INSTALL_IMAGES_SANDBOX = $$($(2)_DIR)/.stamp_images_sandbox_installed
$(2)_TARGET_CREATE_IMAGES_SANDBOX = $$($(2)_DIR)/.stamp_images_sandbox_created
$(2)_TARGET_INSTALL_HOST =      $$($(2)_DIR)/.stamp_host_installed
$(2)_TARGET_INSTALL_DEPS =	$$($(2)_DIR)/.stamp_deps_installed
$(2)_TARGET_BUILD =		$$($(2)_DIR)/.stamp_built
$(2)_TARGET_CONFIGURE =		$$($(2)_DIR)/.stamp_configured
$(2)_TARGET_RSYNC =	        $$($(2)_DIR)/.stamp_rsynced
$(2)_TARGET_PATCH =		$$($(2)_DIR)/.stamp_patched
$(2)_TARGET_EXTRACT =		$$($(2)_DIR)/.stamp_extracted
$(2)_TARGET_SOURCE =		$$($(2)_DIR)/.stamp_downloaded
$(2)_TARGET_ACTUAL_SOURCE =	$$($(2)_DIR)/.stamp_actual_downloaded
$(2)_TARGET_UNINSTALL =		$$($(2)_DIR)/.stamp_uninstalled
$(2)_TARGET_CLEAN =		$$($(2)_DIR)/.stamp_cleaned
$(2)_TARGET_DIRCLEAN =		$$($(2)_DIR)/.stamp_dircleaned

# default extract command
$(2)_EXTRACT_CMDS ?= \
	$$(if $$($(2)_SOURCE),$$(call INFLATEANDEXTRACT$$(suffix $$($(2)_SOURCE)),$$(DL_DIR)/$$($(2)_SOURCE),$$($(2)_DIR),$$($(2)_STRIP_COMPONENTS)),$$($(2)_EXCLUDES))

# pre/post-steps hooks
$(2)_PRE_DOWNLOAD_HOOKS         ?=
$(2)_POST_DOWNLOAD_HOOKS        ?=
$(2)_PRE_EXTRACT_HOOKS          ?=
$(2)_POST_EXTRACT_HOOKS         ?=
$(2)_PRE_RSYNC_HOOKS            ?=
$(2)_POST_RSYNC_HOOKS           ?=
$(2)_PRE_PATCH_HOOKS            ?=
$(2)_POST_PATCH_HOOKS           ?=
$(2)_PRE_CONFIGURE_HOOKS        ?=
$(2)_POST_CONFIGURE_HOOKS       ?=
$(2)_PRE_BUILD_HOOKS            ?=
$(2)_POST_BUILD_HOOKS           ?=
$(2)_PRE_INSTALL_HOOKS          ?=
$(2)_POST_INSTALL_HOOKS         ?=
$(2)_PRE_INSTALL_STAGING_HOOKS  ?=
$(2)_POST_INSTALL_STAGING_HOOKS ?=
$(2)_PRE_INSTALL_TARGET_HOOKS   ?=
$(2)_POST_INSTALL_TARGET_HOOKS  ?=
$(2)_PRE_INSTALL_IMAGES_HOOKS   ?=
$(2)_POST_INSTALL_IMAGES_HOOKS  ?=
$(2)_PRE_LEGAL_INFO_HOOKS       ?=
$(2)_POST_LEGAL_INFO_HOOKS      ?=
$(2)_TARGET_FINALIZE_HOOKS      ?=
$(2)_ROOTFS_PRE_CMD_HOOKS       ?=
$(2)_ROOTFS_POST_CMD_HOOKS      ?=

# human-friendly targets and target sequencing
$(1):			$(1)-install

$(1)-install: $$($(2)_TARGET_INSTALL)

ifeq ($$($(2)_TYPE),host)

$(1)-install-host:      	$$($(2)_TARGET_INSTALL_HOST)
$$($(2)_TARGET_INSTALL):	$$($(2)_TARGET_INSTALL_HOST)

else

ifeq ($$($(2)_INSTALL_TARGET),YES)
$(1)-install-target:		$$($(2)_TARGET_INSTALL_TARGET)
$$($(2)_TARGET_INSTALL):	$$($(2)_TARGET_INSTALL_TARGET)
else
$(1)-install-target:
endif

ifeq ($$($(2)_INSTALL_STAGING),YES)
$(1)-install-staging:			$$($(2)_TARGET_INSTALL_STAGING)
$$($(2)_TARGET_INSTALL):		$$($(2)_TARGET_INSTALL_STAGING)
# Some packages use install-staging stuff for install-target
$$($(2)_TARGET_INSTALL_TARGET_SANDBOX):	$$($(2)_TARGET_INSTALL_STAGING)
else
$(1)-install-staging:
endif

ifeq ($$($(2)_INSTALL_IMAGES),YES)
$(1)-install-images:		$$($(2)_TARGET_INSTALL_IMAGES)
$$($(2)_TARGET_INSTALL):	$$($(2)_TARGET_INSTALL_IMAGES)
# Some packages use install-staging stuff for install-images
$$($(2)_TARGET_INSTALL_IMAGES_SANDBOX):	$$($(2)_TARGET_INSTALL_STAGING)
else
$(1)-install-images:
endif

endif

$(1)-build:		$$($(2)_TARGET_BUILD)

$(1)-configure:		$$($(2)_TARGET_CONFIGURE)

ifeq ($$($(2)_OVERRIDE_SRCDIR),)
# In the normal case (no package override), the sequence of steps is
#  source, by downloading
#  depends
#  extract
#  patch
#  configure

$(1)-patch:		$$($(2)_TARGET_PATCH)
$$($(2)_TARGET_PATCH):	$$($(2)_TARGET_EXTRACT)

$(1)-extract:		$$($(2)_TARGET_EXTRACT)

$(1)-source:		$$($(2)_TARGET_SOURCE)

$(1)-all-source:	$(1)-legal-source
$(1)-legal-info:	$(1)-legal-source
$(1)-legal-source:	$(1)-source

# Only download the actual source if it differs from the 'main' archive
ifneq ($$($(2)_ACTUAL_SOURCE_TARBALL),)
ifneq ($$($(2)_ACTUAL_SOURCE_TARBALL),$$($(2)_SOURCE))
$(1)-legal-source:	$$($(2)_TARGET_ACTUAL_SOURCE)
endif # actual sources != sources
endif # actual sources != ""

$(1)-external-deps:
	@for p in $$($(2)_SOURCE) $$($(2)_PATCH) $$($(2)_EXTRA_DOWNLOADS) ; do \
		echo `basename $$$$p` ; \
	done
else
# In the package override case, the sequence of steps
#  source, by rsyncing
#  depends
#  configure

# Use an order-only dependency so the "<pkg>-clean-for-rebuild" rule
# can remove the stamp file without triggering the configure step.
$$($(2)_TARGET_CONFIGURE): | $$($(2)_TARGET_RSYNC)

$(1)-patch:			$$($(2)_TARGET_PATCH)
$$($(2)_TARGET_PATCH):		$$($(2)_TARGET_RSYNC)
$(1)-extract:			$$($(2)_TARGET_EXTRACT)
$$($(2)_TARGET_EXTRACT):	$$($(2)_TARGET_RSYNC)

$(1)-rsync:		$$($(2)_TARGET_RSYNC)

$(1)-source:
$(1)-legal-source:

$(1)-external-deps:
	@echo "file://$$($(2)_OVERRIDE_SRCDIR)"
endif

$(1)-show-version:
			@echo $$($(2)_VERSION)

$(1)-show-depends:
			@echo $$($(2)_FINAL_ALL_DEPENDENCIES)

$(1)-show-rdepends:
			@echo $$($(2)_RDEPENDENCIES)

$(1)-show-build-order: $$(patsubst %,%-show-build-order,$$($(2)_FINAL_ALL_DEPENDENCIES))
	@:
	$$(info $(1))

$(1)-graph-depends: graph-depends-requirements
	$(call pkg-graph-depends,$(1),--direct)

$(1)-graph-rdepends: graph-depends-requirements
	$(call pkg-graph-depends,$(1),--reverse)

$(1)-all-source:	$(1)-source
$(1)-all-source:	$$(foreach p,$$($(2)_FINAL_ALL_DEPENDENCIES),$$(p)-all-source)

$(1)-all-external-deps:	$(1)-external-deps
$(1)-all-external-deps:	$$(foreach p,$$($(2)_FINAL_ALL_DEPENDENCIES),$$(p)-all-external-deps)

$(1)-all-legal-info:	$(1)-legal-info
$(1)-all-legal-info:	$$(foreach p,$$($(2)_FINAL_ALL_DEPENDENCIES),$$(p)-all-legal-info)

$(1)-uninstall:		$$($(2)_TARGET_UNINSTALL)

$(1)-clean:		$(1)-uninstall \
			$$($(2)_TARGET_CLEAN)

$(1)-dirclean:		$$($(2)_TARGET_DIRCLEAN)

$(1)-clean-for-reinstall:
ifneq ($$($(2)_OVERRIDE_SRCDIR),)
			rm -f $$($(2)_TARGET_RSYNC)
endif
			rm -f $$($(2)_TARGET_INSTALL_STAGING)
			rm -f $$($(2)_TARGET_INSTALL_TARGET)
			rm -f $$($(2)_TARGET_INSTALL_IMAGES)
			rm -f $$($(2)_TARGET_INSTALL_HOST)

$(1)-reinstall:		$(1)-clean-for-reinstall $(1)

$(1)-clean-for-rebuild: $(1)-clean-for-reinstall
			rm -f $$($(2)_TARGET_BUILD)

$(1)-rebuild:		$(1)-clean-for-rebuild $(1)

$(1)-clean-for-reconfigure: $(1)-clean-for-rebuild
			rm -f $$($(2)_TARGET_CONFIGURE)

$(1)-reconfigure:	$(1)-clean-for-reconfigure $(1)

################################################################################
# Targets -- produce a stamp file for each step of a package build
################################################################################

# Retrieve the archive
ifneq ($$($(2)_SOURCE),)
SOURCES += $$($(2)_SOURCE)
endif

$$($(2)_TARGET_SOURCE): | $(BUILD_DIR)/buildroot-config/auto.conf
	$$(foreach hook,$$($(2)_PRE_DOWNLOAD_HOOKS),$$(call $$(hook))$$(sep))
# Only show the download message if it isn't already downloaded
	$(Q)for p in $$($(2)_ALL_DOWNLOADS); do \
		if test ! -e $(DL_DIR)/`basename $$$$p` ; then \
			$$(call MESSAGE,"Downloading") ; \
			break ; \
		fi ; \
	done
	$$(foreach p,$$($(2)_ALL_DOWNLOADS),$$(call DOWNLOAD,$$(p))$$(sep))
	$$(foreach hook,$$($(2)_POST_DOWNLOAD_HOOKS),$$(call $$(hook))$$(sep))
	$(Q)mkdir -p $$(@D)
	$(Q)touch $$@

# Unpack the archive
$$($(2)_TARGET_EXTRACT): $$($(2)_TARGET_SOURCE)
	@$$(call step_start,extract)
	@$$(call MESSAGE,"Extracting")
	$$(foreach hook,$$($(2)_PRE_EXTRACT_HOOKS),$$(call $$(hook))$$(sep))
	$(Q)mkdir -p $$(@D)
	$$($(2)_EXTRACT_CMDS)
# some packages have messed up permissions inside
	$(Q)chmod -R +rw $$(@D)
	$$(foreach hook,$$($(2)_POST_EXTRACT_HOOKS),$$(call $$(hook))$$(sep))
	@$$(call step_end,extract)
	$(Q)touch $$@

# Rsync the source directory if the <pkg>_OVERRIDE_SRCDIR feature is
# used.
$$($(2)_TARGET_RSYNC): | $(BUILD_DIR)/buildroot-config/auto.conf
	@$$(call MESSAGE,"Syncing from source dir $$($(2)_OVERRIDE_SRCDIR)")
	$$(foreach hook,$$($(2)_PRE_RSYNC_HOOKS),$$(call $$(hook))$$(sep))
	@test -d $$($(2)_OVERRIDE_SRCDIR) || (echo "ERROR: $$($(2)_OVERRIDE_SRCDIR) does not exist" ; exit 1)
	rsync -auL $(RSYNC_VCS_EXCLUSIONS) $$($(2)_OVERRIDE_SRCDIR)/ $$(@D)
	$$(foreach hook,$$($(2)_POST_RSYNC_HOOKS),$$(call $$(hook))$$(sep))
	$(Q)touch $$@

# Patch
$$($(2)_TARGET_PATCH): PATCH_BASE_DIRS = $$($(2)_PKGDIR)
$$($(2)_TARGET_PATCH): PATCH_BASE_DIRS += $$(addsuffix /$$($(2)_RAWNAME),$(call qstrip,$(BR2_GLOBAL_PATCH_DIR)))
$$($(2)_TARGET_PATCH):
	@$$(call step_start,patch)
	@$$(call MESSAGE,"Patching")
	$$(if $$($(2)_SOURCE)$$($(2)_OVERRIDE_SRCDIR),$$(foreach hook,$$($(2)_PRE_PATCH_HOOKS),$$(call $$(hook))$$(sep)))
	$$(if $$($(2)_SOURCE)$$($(2)_OVERRIDE_SRCDIR),$$(foreach p,$$($(2)_PATCH),$$(APPLY_PATCHES) $$(@D) $(DL_DIR) $$(notdir $$(p))$$(sep)))
	$$(if $$($(2)_SOURCE)$$($(2)_OVERRIDE_SRCDIR),$(Q)( \
	for D in $$(PATCH_BASE_DIRS); do \
	  if test -d $$$${D}; then \
	    if test -d $$$${D}/$$($(2)_VERSION); then \
	      $$(APPLY_PATCHES) $$(@D) $$$${D}/$$($(2)_VERSION) \*.patch \*.patch.$$(ARCH) || exit 1; \
	    elif test -n "$$($(2)_MAJOR_VERSION)" && test -d $$$${D}/$$($(2)_MAJOR_VERSION); then \
	      $$(APPLY_PATCHES) $$(@D) $$$${D}/$$($(2)_MAJOR_VERSION) \*.patch \*.patch.$$(ARCH) || exit 1; \
	    else \
	      $$(APPLY_PATCHES) $$(@D) $$$${D} \*.patch \*.patch.$(ARCH) || exit 1; \
	    fi; \
	  fi; \
	done; \
	))
	$$(if $$($(2)_SOURCE)$$($(2)_OVERRIDE_SRCDIR),$$(foreach hook,$$($(2)_POST_PATCH_HOOKS),$$(call $$(hook))$$(sep)))
	@$$(call step_end,patch)
	$(Q)touch $$@

# Check that all directories specified in BR2_GLOBAL_PATCH_DIR exist.
$(foreach dir,$(call qstrip,$(BR2_GLOBAL_PATCH_DIR)),\
	$(if $(wildcard $(dir)),,\
		$(error BR2_GLOBAL_PATCH_DIR contains nonexistent directory $(dir))))

ifeq ($$($(2)_OVERRIDE_SRCDIR),)
$$($(2)_TARGET_INSTALL_DEPS): $$($(2)_TARGET_SOURCE)
else
$$($(2)_TARGET_INSTALL_DEPS): $$($(2)_TARGET_RSYNC)
endif
	$(Q)touch $$@

# Configure
$$($(2)_TARGET_CONFIGURE): $$($(2)_TARGET_INSTALL_DEPS) $$($(2)_TARGET_PATCH)
	@$$(call step_start,configure)
	@$$(call MESSAGE,"Configuring")
	$$(foreach hook,$$($(2)_PRE_CONFIGURE_HOOKS),$$(call $$(hook))$$(sep))
	$$($(2)_CONFIGURE_CMDS)
	$$(foreach hook,$$($(2)_POST_CONFIGURE_HOOKS),$$(call $$(hook))$$(sep))
# uclibc installs headers and start files in the configure stage :-/
	$(if $(filter uclibc,$(1)),$(Q)$$(call MARK_ORPHANS,$(2),staging,$$(STAGING_DIR))$$(sep)$(Q)$$(call MARK_ORPHANS,$(2),target,$$(TARGET_DIR)))
	@$$(call step_end,configure)
	$(Q)touch $$@

# Build
$$($(2)_TARGET_BUILD): STAGING_DIR := $$(if $$(ORIG_STAGING_DIR),$$(ORIG_STAGING_DIR),$$(STAGING_DIR))
$$($(2)_TARGET_BUILD): TARGET_DIR := $$(if $$(ORIG_TARGET_DIR),$$(ORIG_TARGET_DIR),$$(TARGET_DIR))
$$($(2)_TARGET_BUILD): BINARIES_DIR := $$(if $$(ORIG_BINARIES_DIR),$$(ORIG_BINARIES_DIR),$$(BINARIES_DIR))
$$($(2)_TARGET_BUILD): $$($(2)_TARGET_CONFIGURE)
	@$$(call step_start,build)
	@$$(call MESSAGE,"Building")
	$$(foreach hook,$$($(2)_PRE_BUILD_HOOKS),$$(call $$(hook))$$(sep))
	+$$($(2)_BUILD_CMDS)
	$$(foreach hook,$$($(2)_POST_BUILD_HOOKS),$$(call $$(hook))$$(sep))
	@$$(call step_end,build)
	$(Q)touch $$@

# Install to host dir
$$($(2)_TARGET_INSTALL_HOST): $$($(2)_TARGET_BUILD)
	@$$(call step_start,install-host)
	@$$(call MESSAGE,"Installing to host directory")
	$$(foreach hook,$$($(2)_PRE_INSTALL_HOOKS),$$(call $$(hook))$$(sep))
	+$$($(2)_INSTALL_CMDS)
	$$(foreach hook,$$($(2)_POST_INSTALL_HOOKS),$$(call $$(hook))$$(sep))
# host-gcc-final installs libraries into the target and staging areas :-/
	$(if $(filter host-gcc-final,$(1)),$(Q)$$(call MARK_ORPHANS,$(2),staging,$$(STAGING_DIR))$$(sep)$(Q)$$(call MARK_ORPHANS,$(2),target,$$(TARGET_DIR))$(Q)$$(call GLOBAL_POST_INSTALL_HOOK_STRIP,$(2),target)$(Q)$$(call INSTALL_RECURSIVE,$(2),target,$$(TARGET_DIR)))
	@$$(call step_end,install-host)
	$(Q)touch $$@

# Create staging sandbox dir
$$($(2)_TARGET_CREATE_STAGING_SANDBOX): $$($(2)_TARGET_BUILD)
	$(Q)mkdir -p $$(STAGING_DIR)/bin
	$(Q)mkdir -p $$(STAGING_DIR)/sbin
	$(Q)mkdir -p $$(STAGING_DIR)/lib
	$(Q)mkdir -p $$(STAGING_DIR)/usr/lib
	$(Q)mkdir -p $$(STAGING_DIR)/usr/include
	$(Q)mkdir -p $$(STAGING_DIR)/usr/bin
	$(Q)mkdir -p $$(STAGING_DIR)/usr/sbin
	$(Q)mkdir -p $$(STAGING_DIR)/etc/init.d
	$(Q)touch $$@

# Install to staging sandbox dir
#
# Some packages install libtool .la files alongside any installed
# libraries. These .la files sometimes refer to paths relative to the
# sysroot, which libtool will interpret as absolute paths to host
# libraries instead of the target libraries. Since this is not what we
# want, these paths are fixed by prefixing them with $(STAGING_DIR).
# As we configure with --prefix=/usr, this fix needs to be applied to
# any path that starts with /usr.
#
# To protect against the case that the output or staging directories or
# the pre-installed external toolchain themselves are under /usr, we first
# substitute away any occurrences of these directories with @BASE_DIR@,
# @STAGING_DIR@ and @TOOLCHAIN_EXTERNAL_INSTALL_DIR@ respectively.
#
# Note that STAGING_DIR can be outside BASE_DIR when the user sets
# BR2_HOST_DIR to a custom value. Note that TOOLCHAIN_EXTERNAL_INSTALL_DIR
# can be under @BASE_DIR@ when it's a downloaded toolchain, and can be
# empty when we use an internal toolchain.
#
$$($(2)_TARGET_INSTALL_STAGING_SANDBOX): $$($(2)_TARGET_CREATE_STAGING_SANDBOX)
	@$$(call step_start,install-staging-sandbox)
	@$$(call MESSAGE,"Installing to staging sandbox")
	$$(foreach hook,$$($(2)_PRE_INSTALL_STAGING_HOOKS),$$(call $$(hook))$$(sep))
	$$($(2)_INSTALL_STAGING_CMDS)
	$$(foreach hook,$$($(2)_POST_INSTALL_STAGING_HOOKS),$$(call $$(hook))$$(sep))
	$(Q)if test -n "$$($(2)_CONFIG_SCRIPTS)" ; then \
		$$(call MESSAGE,"Fixing package configuration files") ;\
			$(SED)  "s,$$(BASE_DIR),@BASE_DIR@,g" \
				-e "s,$$(STAGING_DIR),@STAGING_DIR@,g" \
				-e "s,^\(exec_\)\?prefix=.*,\1prefix=@STAGING_DIR@/usr,g" \
				-e "s,-I/usr/,-I@STAGING_DIR@/usr/,g" \
				-e "s,-L/usr/,-L@STAGING_DIR@/usr/,g" \
				-e "s,@STAGING_DIR@,$(STAGING_DIR),g" \
				-e "s,@BASE_DIR@,$$(BASE_DIR),g" \
				$$(addprefix $$($(2)_SANDBOX_DIR)/staging/usr/bin/,$$($(2)_CONFIG_SCRIPTS)) ;\
	fi
	@$$(call MESSAGE,"Fixing libtool files")
	$(Q)for i in $$$$(find $$(STAGING_DIR)/lib* $$(STAGING_DIR)/usr/lib* -name "*.la"); do \
		$$(SED) "s:\(['= ]\)/usr:\\1$(STAGING_DIR)/usr:g;s:\(['= ]\)/lib:\\1$(STAGING_DIR)/lib:g" $$$$i; \
		larefs=$$$$(sed -n "s:['= ]$$(@D)/\([^ ]\+/\)*\([^ /]\+\.la\): ###LAREF###\\2 :gp" $$$$i); \
		while echo "$$$$larefs" | grep -q "###LAREF###"; do \
			larefs=$$$${larefs#*###LAREF###}; \
			laref=$$$${larefs%% *}; \
			latgt=$$$$(find $$(STAGING_DIR)/lib* $$(STAGING_DIR)/usr/lib* -name "$$$$laref" -type f); \
			[ -f "$$$$latgt" ] || continue; \
			latgt=$(STAGING_DIR)$$$${latgt#$$(STAGING_DIR)}; \
			$$(SED) "s:\(['= ]\)$$(@D)/\([^ ]\+/\)*$$$$laref:\\1$$$$latgt:" $$$$i; \
		done \
	done
	@$$(call step_end,install-staging-sandbox)
	$(Q)touch $$@

# Check staging sandbox dir
$$($(2)_TARGET_CHECK_STAGING_SANDBOX): $$($(2)_TARGET_INSTALL_STAGING_SANDBOX)
	@$$(call step_start,check-staging-sandbox)
	$$(foreach hook,$(GLOBAL_POST_INSTALL_STAGING_HOOKS),$(Q)$$(call $$(hook),$(2),staging,$$(STAGING_DIR))$$(sep))
	@$$(call step_end,check-staging-sandbox)
	$(Q)touch $$@

# Install to staging dir
$$($(2)_TARGET_INSTALL_STAGING): $$($(2)_TARGET_CHECK_STAGING_SANDBOX)
	@$$(call step_start,install-staging)
	@$$(call MESSAGE,"Installing to staging directory")
	$(Q)$$(call INSTALL_RECURSIVE,$(2),staging,$$(STAGING_DIR))
	@$$(call step_end,install-staging)
	$(Q)touch $$@

# Create target sandbox dir
$$($(2)_TARGET_CREATE_TARGET_SANDBOX): $$($(2)_TARGET_BUILD)
	$(Q)mkdir -p $$(TARGET_DIR)/bin
	$(Q)mkdir -p $$(TARGET_DIR)/sbin
	$(Q)mkdir -p $$(TARGET_DIR)/lib
	$(Q)mkdir -p $$(TARGET_DIR)/usr/lib
	$(Q)mkdir -p $$(TARGET_DIR)/usr/include
	$(Q)mkdir -p $$(TARGET_DIR)/usr/bin
	$(Q)mkdir -p $$(TARGET_DIR)/usr/sbin
	$(Q)mkdir -p $$(TARGET_DIR)/etc/init.d
	$(Q)touch $$@

# Install to target sandbox dir
$$($(2)_TARGET_INSTALL_TARGET_SANDBOX): $$($(2)_TARGET_CREATE_TARGET_SANDBOX)
	@$$(call step_start,install-target-sandbox)
	@$$(call MESSAGE,"Installing to target sandbox")
	$$(foreach hook,$$($(2)_PRE_INSTALL_TARGET_HOOKS),$$(call $$(hook))$$(sep))
	+$$($(2)_INSTALL_TARGET_CMDS)
	$(if $(BR2_INIT_SYSTEMD),\
		$$($(2)_INSTALL_INIT_SYSTEMD))
	$(if $(BR2_INIT_SYSV)$(BR2_INIT_BUSYBOX),\
		$$($(2)_INSTALL_INIT_SYSV))
	$$(foreach hook,$$($(2)_POST_INSTALL_TARGET_HOOKS),$$(call $$(hook))$$(sep))
	$(Q)if test -n "$$($(2)_CONFIG_SCRIPTS)" ; then \
		$(RM) -f $$(addprefix $$(TARGET_DIR)/usr/bin/,$$($(2)_CONFIG_SCRIPTS)) ; \
	fi
	@$$(call step_end,install-target-sandbox)
	$(Q)touch $$@

# Check target sandbox dir
$$($(2)_TARGET_CHECK_TARGET_SANDBOX): $$($(2)_TARGET_INSTALL_TARGET_SANDBOX)
	@$$(call step_start,check-target-sandbox)
	$$(foreach hook,$(GLOBAL_POST_INSTALL_TARGET_HOOKS),$(Q)$$(call $$(hook),$(2),target,$$(TARGET_DIR))$$(sep))
	@$$(call step_end,check-target-sandbox)
	$(Q)touch $$@

# Install to target dir
$$($(2)_TARGET_INSTALL_TARGET): $$($(2)_TARGET_CHECK_TARGET_SANDBOX)
	@$$(call step_start,install-target)
	@$$(call MESSAGE,"Installing to target directory")
	$(Q)$$(call INSTALL_RECURSIVE,$(2),target,$$(TARGET_DIR))
	@$$(call step_end,install-target)
	$(Q)touch $$@

# Create images sandbox dir
$$($(2)_TARGET_CREATE_IMAGES_SANDBOX): $$($(2)_TARGET_BUILD)
	$(Q)mkdir -p $$(BINARIES_DIR)
	$(Q)touch $$@

# Install to images sandbox dir
$$($(2)_TARGET_INSTALL_IMAGES_SANDBOX): $$($(2)_TARGET_CREATE_IMAGES_SANDBOX)
	@$$(call step_start,install-images-sandbox)
	@$$(call MESSAGE,"Installing to images sandbox")
	$$(foreach hook,$$($(2)_PRE_INSTALL_IMAGES_HOOKS),$$(call $$(hook))$$(sep))
	+$$($(2)_INSTALL_IMAGES_CMDS)
	$$(foreach hook,$$($(2)_POST_INSTALL_IMAGES_HOOKS),$$(call $$(hook))$$(sep))
	@$$(call step_end,install-images-sandbox)
	$(Q)touch $$@

# Check images sandbox dir
$$($(2)_TARGET_CHECK_IMAGES_SANDBOX): $$($(2)_TARGET_INSTALL_IMAGES_SANDBOX)
	@$$(call step_start,check-images-sandbox)
	$$(foreach hook,$(GLOBAL_POST_INSTALL_IMAGES_HOOKS),$(Q)$$(call $$(hook),$(2),images,$$(BINARIES_DIR))$$(sep))
	@$$(call step_end,check-images-sandbox)
	$(Q)touch $$@

# Install to images dir
$$($(2)_TARGET_INSTALL_IMAGES): $$($(2)_TARGET_CHECK_IMAGES_SANDBOX)
	@$$(call step_start,install-images)
	@$$(call MESSAGE,"Installing to images directory")
	$(Q)$$(call INSTALL_RECURSIVE,$(2),images,$$(BINARIES_DIR))
	@$$(call step_end,install-images)
	$(Q)touch $$@

# Install
$$($(2)_TARGET_INSTALL):
	@$$(call step_start,complete-install)
	@$$(call MESSAGE,"Completing installation")
	$$(if $$(filter y,$$(FBR_TIDY)),$$(if $$(filter-out linux-headers linux,$(1)),\
find $$(@D)/ -mindepth 1 -maxdepth 1 \! -name '.stamp*' \! -name '.config' -exec rm -Rf \{\} +))
	$$(if $$(filter k y,$$(FBR_TIDY)),$$(if $$(filter $(1),linux-headers linux),\
$$(MAKE) -C $$(@D) clean))
	@$$(call step_end,complete-install)
	$(Q)touch $$@

# Clean package
$$($(2)_DIR)/.stamp_cleaned:
	@$$(call MESSAGE,"Cleaning up")
	rm -f $$($(2)_TARGET_BUILD)

# Uninstall package from target and staging
$$($(2)_DIR)/.stamp_uninstalled:
	@$$(call MESSAGE,"Uninstalling")
	$(Q)rm -f $$($(2)_TARGET_INSTALL)
	$(Q)rm -f $$($(2)_TARGET_INSTALL_STAGING)
	$(Q)$$(call UNINSTALL_RECURSIVE,$(2),staging,$$(STAGING_DIR))
	$(Q)rm -f $$($(2)_TARGET_INSTALL_TARGET)
	$(Q)$$(call UNINSTALL_RECURSIVE,$(2),target,$$(TARGET_DIR))
	$(Q)rm -f $$($(2)_TARGET_INSTALL_IMAGES)
	$(Q)$$(call UNINSTALL_RECURSIVE,$(2),images,$$(BINARIES_DIR))
	$(Q)$(if $(filter y,$(FBR_TIDY)),rm -f $$($(2)_TARGET_EXTRACT) $$($(2)_TARGET_RSYNC))

# Remove package sources
$$($(2)_DIR)/.stamp_dircleaned: | $$($(2)_DIR)/.stamp_uninstalled
	@$$(call MESSAGE,"Cleaning")
	rm -Rf $$(@D)
	rm -Rf $$($(2)_SANDBOX_DIR)

# define the PKG variable for all targets, containing the
# uppercase package variable prefix
$$($(2)_TARGET_INSTALL_TARGET_SANDBOX):	PKG:=$(2)
$$($(2)_TARGET_INSTALL_TARGET_SANDBOX):	ORIG_TARGET_DIR:=$(TARGET_DIR)
$$($(2)_TARGET_INSTALL_TARGET_SANDBOX):	TARGET_DIR:=$$($(2)_SANDBOX_DIR)/target
$$($(2)_TARGET_INSTALL_TARGET):		PKG:=$(2)
$$($(2)_TARGET_INSTALL_STAGING_SANDBOX): PKG:=$(2)
$$($(2)_TARGET_INSTALL_STAGING_SANDBOX): ORIG_STAGING_DIR:=$(STAGING_DIR)
$$($(2)_TARGET_INSTALL_STAGING_SANDBOX): STAGING_DIR:=$$($(2)_SANDBOX_DIR)/staging
$$($(2)_TARGET_INSTALL_STAGING):	PKG:=$(2)
$$($(2)_TARGET_INSTALL_IMAGES_SANDBOX):	PKG:=$(2)
$$($(2)_TARGET_INSTALL_IMAGES_SANDBOX):	ORIG_BINARIES_DIR:=$(BINARIES_DIR)
$$($(2)_TARGET_INSTALL_IMAGES_SANDBOX):	BINARIES_DIR:=$$($(2)_SANDBOX_DIR)/images
$$($(2)_TARGET_INSTALL_IMAGES):		PKG:=$(2)
$$($(2)_TARGET_INSTALL_HOST):		PKG:=$(2)
$$($(2)_TARGET_INSTALL):		PKG:=$(2)
$$($(2)_TARGET_CHECK_TARGET_SANDBOX): PKG:=$(2)
$$($(2)_TARGET_CHECK_STAGING_SANDBOX): PKG:=$(2)
$$($(2)_TARGET_CHECK_IMAGES_SANDBOX): PKG:=$(2)
$$($(2)_TARGET_BUILD):			PKG:=$(2)
$$($(2)_TARGET_CONFIGURE):		PKG:=$(2)
$$($(2)_TARGET_RSYNC):                  SRCDIR:=$$($(2)_OVERRIDE_SRCDIR)
$$($(2)_TARGET_RSYNC):                  PKG:=$(2)
$$($(2)_TARGET_PATCH):			PKG:=$(2)
$$($(2)_TARGET_PATCH):			RAWNAME:=$$(patsubst host-%,%,$(1))
$$($(2)_TARGET_PATCH):			PKGDIR:=$(pkgdir)
$$($(2)_TARGET_EXTRACT):		PKG:=$(2)
$$($(2)_TARGET_SOURCE):			PKG:=$(2)
$$($(2)_TARGET_SOURCE):			PKGDIR:=$(pkgdir)
$$($(2)_TARGET_ACTUAL_SOURCE):		PKG:=$(2)
$$($(2)_TARGET_ACTUAL_SOURCE):		PKGDIR:=$(pkgdir)
$$($(2)_TARGET_UNINSTALL):		PKG:=$(2)
$$($(2)_TARGET_CLEAN):			PKG:=$(2)
$$($(2)_TARGET_DIRCLEAN):		PKG:=$(2)

# Compute the name of the Kconfig option that correspond to the
# package being enabled. We handle three cases: the special Linux
# kernel case, the bootloaders case, and the normal packages case.
ifeq ($(1),linux)
$(2)_KCONFIG_VAR = BR2_LINUX_KERNEL
else ifneq ($$(filter boot/% $$(foreach dir,$$(BR2_EXTERNAL_DIRS),$$(dir)/boot/%),$(pkgdir)),)
$(2)_KCONFIG_VAR = BR2_TARGET_$(2)
else ifneq ($$(filter toolchain/% $$(foreach dir,$$(BR2_EXTERNAL_DIRS),$$(dir)/toolchain/%),$(pkgdir)),)
$(2)_KCONFIG_VAR = BR2_$(2)
else
$(2)_KCONFIG_VAR = BR2_PACKAGE_$(2)
endif

# legal-info: declare dependencies and set values used later for the manifest
ifneq ($$($(2)_LICENSE_FILES),)
$(2)_MANIFEST_LICENSE_FILES = $$($(2)_LICENSE_FILES)
endif

# We need to extract and patch a package to be able to retrieve its
# license files (if any) and the list of patches applied to it (if
# any).
$(1)-legal-info: $$($(2)_TARGET_PATCH)

# We only save the sources of packages we want to redistribute, that are
# non-overriden (local or true override).
ifeq ($$($(2)_REDISTRIBUTE),YES)
ifeq ($$($(2)_OVERRIDE_SRCDIR),)
# Packages that have a tarball need it downloaded beforehand
$(1)-legal-info: $$($(2)_TARGET_SOURCE)
endif
endif

# legal-info: produce legally relevant info.
$(1)-legal-info: PKG=$(2)
$(1)-legal-info: legal-info-prepare
	@$$(call MESSAGE,"Collecting legal info")
# Packages without a source are assumed to be part of Buildroot, skip them.
	$$(foreach hook,$$($(2)_PRE_LEGAL_INFO_HOOKS),$$(call $$(hook))$$(sep))
ifneq ($$(call qstrip,$$($(2)_SOURCE)),)

# Save license files if defined
# We save the license files for any kind of package: normal, local,
# overridden, or non-redistributable alike.
# The reason to save license files even for no-redistribute packages
# is that the license still applies to the files distributed as part
# of the rootfs, even if the sources are not themselves redistributed.
ifeq ($$(call qstrip,$$($(2)_LICENSE_FILES)),)
	$(Q)$$(call legal-warning-pkg,$$($(2)_BASENAME_RAW),cannot save license ($(2)_LICENSE_FILES not defined))
else
	$(Q)$$(foreach F,$$($(2)_LICENSE_FILES),$$(call legal-license-file,$$($(2)_RAWNAME),$$($(2)_BASENAME_RAW),$$($(2)_PKGDIR),$$(F),$$($(2)_DIR)/$$(F),$$(call UPPERCASE,$(4)))$$(sep))
endif # license files

ifeq ($$($(2)_SITE_METHOD),local)
# Packages without a tarball: don't save and warn
	@$$(call legal-warning-nosource,$$($(2)_RAWNAME),local)

else ifneq ($$($(2)_OVERRIDE_SRCDIR),)
	@$$(call legal-warning-nosource,$$($(2)_RAWNAME),override)

else
# Other packages

ifeq ($$($(2)_REDISTRIBUTE),YES)
# Save the source tarball and any extra downloads, but not
# patches, as they are handled specially afterwards.
	$$(foreach e,$$($(2)_ACTUAL_SOURCE_TARBALL) $$(notdir $$($(2)_EXTRA_DOWNLOADS)),\
		$$(Q)support/scripts/hardlink-or-copy \
			$$(DL_DIR)/$$(e) \
			$$($(2)_REDIST_SOURCES_DIR)$$(sep))
# Save patches and generate the series file
	$$(Q)while read f; do \
		support/scripts/hardlink-or-copy \
			$$$${f} \
			$$($(2)_REDIST_SOURCES_DIR) || exit 1; \
		printf "%s\n" "$$$${f##*/}" >>$$($(2)_REDIST_SOURCES_DIR)/series || exit 1; \
	done <$$($(2)_DIR)/.applied_patches_list
endif # redistribute

endif # other packages
	@$$(call legal-manifest,$$($(2)_RAWNAME),$$($(2)_VERSION),$$($(2)_LICENSE),$$($(2)_MANIFEST_LICENSE_FILES),$$($(2)_ACTUAL_SOURCE_TARBALL),$$($(2)_ACTUAL_SOURCE_SITE),$$(call UPPERCASE,$(4)))
endif # ifneq ($$(call qstrip,$$($(2)_SOURCE)),)
	$$(foreach hook,$$($(2)_POST_LEGAL_INFO_HOOKS),$$(call $$(hook))$$(sep))

# add package to the general list of targets if requested by the buildroot
# configuration
TARGETS += $(1)
ifeq ($$($$($(2)_KCONFIG_VAR)),y)

# Ensure the calling package is the declared provider for all the virtual
# packages it claims to be an implementation of.
ifneq ($$($(2)_PROVIDES),)
$$(foreach pkg,$$($(2)_PROVIDES),\
	$$(eval $$(call virt-provides-single,$$(pkg),$$(call UPPERCASE,$$(pkg)),$(1))$$(sep)))
endif

# Register package as a reverse-dependencies of all its dependencies
$$(eval $$(foreach p,$$($(2)_FINAL_ALL_DEPENDENCIES),\
	$$(call UPPERCASE,$$(p))_RDEPENDENCIES += $(1)$$(sep)))

# Ensure unified variable name conventions between all packages Some
# of the variables are used by more than one infrastructure; so,
# rather than duplicating the checks in each infrastructure, we check
# all variables here in pkg-generic, even though pkg-generic should
# have no knowledge of infra-specific variables.
$(eval $(call check-deprecated-variable,$(2)_MAKE_OPT,$(2)_MAKE_OPTS))
$(eval $(call check-deprecated-variable,$(2)_INSTALL_OPT,$(2)_INSTALL_OPTS))
$(eval $(call check-deprecated-variable,$(2)_INSTALL_TARGET_OPT,$(2)_INSTALL_TARGET_OPTS))
$(eval $(call check-deprecated-variable,$(2)_INSTALL_STAGING_OPT,$(2)_INSTALL_STAGING_OPTS))
$(eval $(call check-deprecated-variable,$(2)_INSTALL_HOST_OPT,$(2)_INSTALL_HOST_OPTS))
$(eval $(call check-deprecated-variable,$(2)_AUTORECONF_OPT,$(2)_AUTORECONF_OPTS))
$(eval $(call check-deprecated-variable,$(2)_CONF_OPT,$(2)_CONF_OPTS))
$(eval $(call check-deprecated-variable,$(2)_BUILD_OPT,$(2)_BUILD_OPTS))
$(eval $(call check-deprecated-variable,$(2)_GETTEXTIZE_OPT,$(2)_GETTEXTIZE_OPTS))
$(eval $(call check-deprecated-variable,$(2)_KCONFIG_OPT,$(2)_KCONFIG_OPTS))

PACKAGES += $(1)

ifneq ($$($(2)_PERMISSIONS),)
PACKAGES_PERMISSIONS_TABLE += $$($(2)_PERMISSIONS)$$(sep)
endif
ifneq ($$($(2)_DEVICES),)
PACKAGES_DEVICES_TABLE += $$($(2)_DEVICES)$$(sep)
endif
ifneq ($$($(2)_USERS),)
PACKAGES_USERS += $$($(2)_USERS)$$(sep)
endif
TARGET_FINALIZE_HOOKS += $$($(2)_TARGET_FINALIZE_HOOKS)
ROOTFS_PRE_CMD_HOOKS += $$($(2)_ROOTFS_PRE_CMD_HOOKS)
ROOTFS_POST_CMD_HOOKS += $$($(2)_ROOTFS_POST_CMD_HOOKS)

ifeq ($$($(2)_SITE_METHOD),svn)
DL_TOOLS_DEPENDENCIES += svn
else ifeq ($$($(2)_SITE_METHOD),git)
DL_TOOLS_DEPENDENCIES += git
else ifeq ($$($(2)_SITE_METHOD),bzr)
DL_TOOLS_DEPENDENCIES += bzr
else ifeq ($$($(2)_SITE_METHOD),scp)
DL_TOOLS_DEPENDENCIES += scp ssh
else ifeq ($$($(2)_SITE_METHOD),hg)
DL_TOOLS_DEPENDENCIES += hg
else ifeq ($$($(2)_SITE_METHOD),cvs)
DL_TOOLS_DEPENDENCIES += cvs
endif # SITE_METHOD

DL_TOOLS_DEPENDENCIES += $$(call extractor-dependency,$$($(2)_SOURCE))

# Ensure all virtual targets are PHONY. Listed alphabetically.
.PHONY:	$(1) \
	$(1)-all-external-deps \
	$(1)-all-legal-info \
	$(1)-all-source \
	$(1)-build \
	$(1)-clean-for-rebuild \
	$(1)-clean-for-reconfigure \
	$(1)-clean-for-reinstall \
	$(1)-configure \
	$(1)-depends \
	$(1)-dirclean \
	$(1)-external-deps \
	$(1)-extract \
	$(1)-graph-depends \
	$(1)-install \
	$(1)-install-host \
	$(1)-install-images \
	$(1)-install-staging \
	$(1)-install-target \
	$(1)-legal-info \
	$(1)-legal-source \
	$(1)-patch \
	$(1)-rebuild \
	$(1)-reconfigure \
	$(1)-reinstall \
	$(1)-rsync \
	$(1)-show-depends \
	$(1)-show-version \
	$(1)-source

ifneq ($$($(2)_SOURCE),)
ifeq ($$($(2)_SITE),)
$$(error $(2)_SITE cannot be empty when $(2)_SOURCE is not)
endif
endif

ifeq ($$(patsubst %/,ERROR,$$($(2)_SITE)),ERROR)
$$(error $(2)_SITE ($$($(2)_SITE)) cannot have a trailing slash)
endif

ifneq ($$($(2)_HELP_CMDS),)
HELP_PACKAGES += $(2)
endif

endif # $(2)_KCONFIG_VAR
endef # inner-generic-package

################################################################################
# generic-package -- the target generator macro for generic packages
################################################################################

# In the case of target packages, keep the package name "pkg"
generic-package = $(call inner-generic-package,$(call pkgname,$(1)),$(call UPPERCASE,$(call pkgname,$(1))),$(call UPPERCASE,$(call pkgname,$(1))),target)
# In the case of host packages, turn the package name "pkg" into "host-pkg"
host-generic-package = $(call inner-generic-package,host-$(call pkgname,$(1)),$(call UPPERCASE,host-$(call pkgname,$(1))),$(call UPPERCASE,$(call pkgname,$(1))),host)

# :mode=makefile:
