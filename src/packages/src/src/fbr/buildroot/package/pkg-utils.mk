################################################################################
#
# This file contains various utility functions used by the package
# infrastructure, or by the packages themselves.
#
################################################################################

#
# Manipulation of .config files based on the Kconfig
# infrastructure. Used by the BusyBox package, the Linux kernel
# package, and more.
#

define KCONFIG_ENABLE_OPT # (option, file)
	$(SED) "/\\<$(1)\\>/d" $(2)
	echo '$(1)=y' >> $(2)
endef

define KCONFIG_SET_OPT # (option, value, file)
	$(SED) "/\\<$(1)\\>/d" $(3)
	echo '$(1)=$(2)' >> $(3)
endef

define KCONFIG_DISABLE_OPT # (option, file)
	$(SED) "/\\<$(1)\\>/d" $(2)
	echo '# $(1) is not set' >> $(2)
endef

# Helper functions to determine the name of a package and its
# directory from its makefile directory, using the $(MAKEFILE_LIST)
# variable provided by make. This is used by the *-package macros to
# automagically find where the package is located.
pkgdir = $(dir $(lastword $(MAKEFILE_LIST)))
pkgname = $(if $(1),$(1),$(lastword $(subst /, ,$(pkgdir))))

# Define extractors for different archive suffixes
INFLATE.bz2  = $(BZCAT)
INFLATE.gz   = $(ZCAT)
INFLATE.lz   = $(LZCAT)
INFLATE.lzma = $(XZCAT)
INFLATE.tbz  = $(BZCAT)
INFLATE.tbz2 = $(BZCAT)
INFLATE.tgz  = $(ZCAT)
INFLATE.xz   = $(XZCAT)
INFLATE.tar  = cat
INFLATE.rpm  = support/scripts/unpackrpm
INFLATE.zip  = unzip
RPM_FORMATS  = .gz .bz2

# suitable-extractor(filename): returns extractor based on suffix
suitable-extractor = $(INFLATE$(suffix $(1)))
dl-tools = $(if $(filter .rpm,$(suffix $(1))),$(foreach format,$(RPM_FORMATS),$(firstword $(INFLATE$(format)))),$(firstword $(INFLATE$(suffix $(1)))))
EXTRACTTAR = $(1) $(2) | $(TAR) --strip-components=$(4) -C $(3) $(foreach x,$(5),--exclude='$(x)' ) $(TAR_OPTIONS) -
INFLATEANDEXTRACT.bz2  = $(call EXTRACTTAR,$(INFLATE.bz2),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.gz   = $(call EXTRACTTAR,$(INFLATE.gz),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.lz   = $(call EXTRACTTAR,$(INFLATE.lz),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.tbz  = $(call EXTRACTTAR,$(INFLATE.tbz),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.tbz2 = $(call EXTRACTTAR,$(INFLATE.tbz2),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.tgz  = $(call EXTRACTTAR,$(INFLATE.tgz),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.xz   = $(call EXTRACTTAR,$(INFLATE.xz),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.tar  = $(call EXTRACTTAR,$(INFLATE.tar),$(1),$(2),$(3),$(4))
INFLATEANDEXTRACT.rpm  = $(INFLATE.rpm) $(1) $(2)
INFLATEANDEXTRACT.zip  = $(INFLATE.zip) $(1) -d $(2)

# extractor-dependency(filename): returns extractor for 'filename' if the
# extractor is a dependency. If we build the extractor return nothing.
# $(firstword) is used here because the extractor can have arguments, like
# ZCAT="gzip -d -c", and to check for the dependency we only want 'gzip'.
extractor-dependency = $(firstword $(INFLATE$(filter-out \
	$(EXTRACTOR_DEPENDENCY_PRECHECKED_EXTENSIONS),$(suffix $(1)))))

# check-deprecated-variable -- throw an error on deprecated variables
# example:
#   $(eval $(call check-deprecated-variable,FOO_MAKE_OPT,FOO_MAKE_OPTS))
define check-deprecated-variable # (deprecated var, new var)
ifneq ($$(origin $(1)),undefined)
$$(error Package error: use $(2) instead of $(1). Please fix your .mk file)
endif
endef

#
# legal-info helper functions
#
LEGAL_INFO_SEPARATOR = "::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"

define legal-warning # text
	flock $(LEGAL_WARNINGS) echo "WARNING: $(1)" >>$(LEGAL_WARNINGS)
endef

define legal-warning-pkg # pkg, text
	flock $(LEGAL_WARNINGS) echo "WARNING: $(1): $(2)" >>$(LEGAL_WARNINGS)
endef

define legal-warning-nosource # pkg, {local|override}
	$(call legal-warning-pkg,$(1),sources not saved ($(2) packages not handled))
endef

define legal-manifest # pkg, version, license, license-files, source, url, {HOST|TARGET}
	flock $(LEGAL_MANIFEST_CSV_$(7)) echo '"$(1)","$(2)","$(3)","$(4)","$(5)","$(6)"' >>$(LEGAL_MANIFEST_CSV_$(7))
endef

define legal-license-file # pkgname, pkgname-pkgver, pkgdir, filename, file-fullpath, {HOST|TARGET}
	mkdir -p $(LICENSE_FILES_DIR_$(6))/$(2)/$(dir $(4)) && \
	{ \
		if [ -f $(3)/$($(PKG)_VERSION)/$(1).hash ]; then \
			support/download/check-hash $(3)/$($(PKG)_VERSION)/$(1).hash $(5) $(4); \
		else \
			support/download/check-hash $(3)/$(1).hash $(5) $(4); \
		fi; \
		case $${?} in (0|3) ;; (*) exit 1;; esac; \
	} && \
	cp $(5) $(LICENSE_FILES_DIR_$(6))/$(2)/$(4)
endef

#
# (de)installation helper functions
#

# $1 = package name
# $2 = sandbox type (target or staging)
# $3 = target directory
define MARK_ORPHANS
	error=; \
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	mkdir -p $$sandbox_dir; \
	while read file; do \
		file=$${file#-}; \
		target_dir=$$(dirname $$file); \
		if ! mkdir -p $$sandbox_dir/$$target_dir; then \
			$(call MESSAGE,Error: Cannot create directory $$sandbox_dir/$$target_dir); \
			error=1; \
		elif ! cp -pdfv $(3)/$$file $$sandbox_dir/$$target_dir/; then \
			$(call MESSAGE,Error: Cannot copy file $(3)/$$file to $$sandbox_dir/$$target_dir); \
			error=1; \
		else \
			echo "$($(1)_NAME) $$file" >> $$sandbox_dir.installed; \
		fi \
	done < <(diff -u <(find $(3)/ \! -type d | sed "s,^$(3)/,," | sort -u) <(cat $(SANDBOX_DIR)/*/$(2).installed 2>/dev/null | cut -d ' ' -f 2 | sort -u) | sed '1,2d' | grep "^-"); \
	[ -n "$$error" ] && exit 1 || true
endef

GET_BUILD_ID = eu-unstrip -n -e $(1) 2>/dev/null | head -n 1 | sed -n 's/^[^ ]\+ \([^ ]\+\)@.*/\1/p'
GET_DEBUG_FILE = $${$(1):0:2}/$${$(1):2}.debug

# $1 = package name
# $2 = sandbox type (target or staging)
# $3 = target directory
define INSTALL_RECURSIVE
	error=; \
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	while read pkg file; do \
		target_dir=$$(dirname $$file); \
		if ! mkdir -p $(3)/$$target_dir; then \
			$(call MESSAGE,Error: Cannot create directory $(3)/$$target_dir); \
			error=1; \
		elif echo $$pkg | grep -q "^script:"; then \
			script=$$(echo $$pkg | sed -n 's/^script://p'); \
			if ! flock $(3)/$$target_dir support/scripts/$$script merge $(3)/$$file $$sandbox_dir/$$file; \
			then \
				$(call MESSAGE,Error: Cannot merge file $(3)/$$target_dir with $$sandbox_dir/$$file); \
				error=1; \
			fi \
		elif ! cp -pdfv $$sandbox_dir/$$file $(3)/$$target_dir/; then \
			$(call MESSAGE,Error: Cannot copy file $$sandbox_dir/$$file to $(3)/$$target_dir); \
			error=1; \
		elif [ ! -L $(3)/$$file ]; then \
			chmod ugo+r $(3)/$$file; \
			if readelf -h $(3)/$$file >/dev/null 2>&1; then \
				id=$$($(call GET_BUILD_ID,$(3)/$$file)); \
				if [ -n "$$id" ]; then \
					debugsrc=$$sandbox_dir.debug/$$id; \
					debugtgt=$(DEBUG_DIR)/.build-id/$(call GET_DEBUG_FILE,id); \
					if [ -f "$$debugsrc" ]; then \
						mkdir -p $$(dirname $$debugtgt); \
						cp -pdfv $$debugsrc $$debugtgt; \
					fi \
				fi \
			fi \
		fi \
	done < $$sandbox_dir.installed; \
	[ -n "$$error" ] && exit 1 || true
endef

# $1 = package name
# $2 = sandbox type (target or staging)
# $3 = target directory
define UNINSTALL_RECURSIVE
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	[ -f $$sandbox_dir.installed ] && while read pkg file; do \
		rfile=$$(echo $$file | sed 's/\[/\\[/g;s/\./\\./g;s/\//\\\//g'); \
		cpkg=$$(sed -n "s/^\([^ ]\+\) $$rfile$$/\1/p" $(TOPDIR)/package/conflict.resolver 2>/dev/null); \
		if [ -n "$$cpkg" ] && echo $$cpkg | grep -q "^script:"; then \
			script=$$(echo $$cpkg | sed -n 's/^script://p'); \
			flock $(3)/$$(dirname $$file) support/scripts/$$script unmerge $(3)/$$file $$sandbox_dir/$$file; \
		else \
			rm -fv $(3)/$$file; \
		fi; \
		if readelf -h $$sandbox_dir/$$file >/dev/null 2>&1; then \
			id=$$($(call GET_BUILD_ID,$$sandbox_dir/$$file)); \
			if [ -n "$$id" ]; then \
				debugtgt=$(DEBUG_DIR)/.build-id/$(call GET_DEBUG_FILE,id); \
				if [ -f "$$debugtgt" ]; then \
					rm -fv $$debugtgt; \
				fi \
			fi \
		fi; \
		rmdir -p $(3)/$$(dirname $$file) 2>/dev/null; \
	done < $$sandbox_dir.installed; \
	true
endef

define GLOBAL_POST_INSTALL_HOOK_TIDY_UP_INTRO
	$(call MESSAGE,Deleting superfluous files from $(2) sandbox)
endef

GLOBAL_POST_INSTALL_STAGING_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_INTRO
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_INTRO
GLOBAL_POST_INSTALL_IMAGES_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_INTRO

define GLOBAL_POST_INSTALL_HOOK_TIDY_UP
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/include $($(1)_SANDBOX_DIR)/$(2)/usr/share/aclocal \
		$($(1)_SANDBOX_DIR)/$(2)/usr/lib/pkgconfig $($(1)_SANDBOX_DIR)/$(2)/usr/share/pkgconfig \
		$($(1)_SANDBOX_DIR)/$(2)/usr/lib/cmake $($(1)_SANDBOX_DIR)/$(2)/usr/share/cmake
	-find $($(1)_SANDBOX_DIR)/$(2)/usr/{lib,share}/ -name '*.cmake' -type f -exec rm -f \{\} +
	-find $($(1)_SANDBOX_DIR)/$(2)/lib $($(1)_SANDBOX_DIR)/$(2)/usr/lib $($(1)_SANDBOX_DIR)/$(2)/usr/libexec \( -name '*.a' -o -name '*.la' \) -type f -exec rm -f \{\} +
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/share/bash-completion
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/share/zsh
endef

ifneq ($(BR2_PACKAGE_BASH),y)
GLOBAL_POST_INSTALL_HOOK_TIDY_UP += $(sep) rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/share/bash-completion
endif
ifneq ($(BR2_PACKAGE_ZSH),y)
GLOBAL_POST_INSTALL_HOOK_TIDY_UP += $(sep) rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/share/zsh
endif

GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP

# This piece of junk does the following:
# First collect the whitelist in a file.
# Then go over all the locale dirs and for each subdir, check if it exists
# in the whitelist file. If it doesn't, kill it.
# Finally, specifically for X11, regenerate locale.dir from the whitelist.
ifeq ($(BR2_ENABLE_LOCALE_PURGE),y)
define GLOBAL_POST_INSTALL_HOOK_TIDY_UP_LOCALE
	for dir in $(wildcard $(addprefix $($(1)_SANDBOX_DIR)/$(2),/usr/share/locale /usr/share/X11/locale /usr/lib/locale)); \
	do \
		for langdir in $$dir/*; \
		do \
			if [ -e "$${langdir}" ]; \
			then \
				grep -qx "$${langdir##*/}" $(LOCALE_WHITELIST) || rm -rf $$langdir; \
			fi \
		done; \
	done
	if [ -d $($(1)_SANDBOX_DIR)/$(2)/usr/share/X11/locale ]; \
	then \
		for lang in $(LOCALE_NOPURGE); \
		do \
			if [ -f $($(1)_SANDBOX_DIR)/$(2)/usr/share/X11/locale/$$lang/XLC_LOCALE ]; \
			then \
				echo "$$lang/XLC_LOCALE: $$lang"; \
			fi \
		done > $($(1)_SANDBOX_DIR)/$(2)/usr/share/X11/locale/locale.dir; \
	fi
endef
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_LOCALE
endif

ifneq ($(BR2_PACKAGE_GDB),y)
define GLOBAL_POST_INSTALL_HOOK_TIDY_UP_GDB
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/share/gdb
endef
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_GDB
endif

ifneq ($(BR2_HAVE_DOCUMENTATION),y)
define GLOBAL_POST_INSTALL_HOOK_TIDY_UP_DOCS
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/man $($(1)_SANDBOX_DIR)/$(2)/usr/share/man
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/info $($(1)_SANDBOX_DIR)/$(2)/usr/share/info
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/doc $($(1)_SANDBOX_DIR)/$(2)/usr/share/doc
	rm -rf $($(1)_SANDBOX_DIR)/$(2)/usr/share/gtk-doc
	rmdir $($(1)_SANDBOX_DIR)/$(2)/usr/share 2>/dev/null || true
endef
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_DOCS
endif

ifeq ($(BR2_PACKAGE_PYTHON_PY_ONLY),y)
define GLOBAL_POST_INSTALL_HOOK_TIDY_UP_PY
	-find $($(1)_SANDBOX_DIR)/$(2)/usr/lib/ -name '*.pyc' -type f -exec rm -f \{\} +
endef
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_PY
endif

ifeq ($(BR2_PACKAGE_PYTHON_PYC_ONLY),y)
define GLOBAL_POST_INSTALL_HOOK_TIDY_UP_PYC
	-find $($(1)_SANDBOX_DIR)/$(2)/usr/lib/ -name '*.py' -type f -exec rm -f \{\} +
endef
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_TIDY_UP_PYC
endif

define GLOBAL_POST_INSTALL_HOOK_ADD_LIBRARY_LINKS
	$(call MESSAGE,Adding missing library links to $(2) sandbox); \
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	if [ -x "$(TARGET_CROSS)ldconfig" ]; then \
		$(TARGET_CROSS)ldconfig -N -r $$sandbox_dir \
			-f $(TARGET_DIR)/etc/ld.so.conf >/dev/null 2>&1; \
	else \
		/sbin/ldconfig -N -r $$sandbox_dir \
			-f $(TARGET_DIR)/etc/ld.so.conf >/dev/null 2>&1; \
	fi
endef

GLOBAL_POST_INSTALL_STAGING_HOOKS += GLOBAL_POST_INSTALL_HOOK_ADD_LIBRARY_LINKS
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_ADD_LIBRARY_LINKS

define GLOBAL_POST_INSTALL_HOOK_RECORD_FILES
	$(call MESSAGE,Recording files in $(2) sandbox)
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	find $$sandbox_dir/ \! -type d | sed "s,^$$sandbox_dir/,," | sed 's/.*/$($(1)_NAME) &/' > $$sandbox_dir.installed
endef

GLOBAL_POST_INSTALL_STAGING_HOOKS += GLOBAL_POST_INSTALL_HOOK_RECORD_FILES
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_RECORD_FILES
GLOBAL_POST_INSTALL_IMAGES_HOOKS += GLOBAL_POST_INSTALL_HOOK_RECORD_FILES

define GLOBAL_POST_INSTALL_HOOK_CHECK_CONFLICTS
	$(call MESSAGE,Checking for conflicts in $(2) sandbox)
	error=; \
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	while read pkg file; do \
		rfile=$$(echo $$file | sed 's/\[/\\[/g;s/\./\\./g;s/\//\\\//g'); \
		cpkg=$$(sed -n "s/^\([^ ]\+\) $$rfile$$/\1/p" $(TOPDIR)/package/conflict.resolver 2>/dev/null); \
		ipkg=$$(sed -n "s/^\([^ ]\+\) $$rfile$$/\1/p" $(SANDBOX_DIR)/*/$(2).installed 2>/dev/null | sed '/^$($(1)_NAME)$$/d'); \
		if [ -n "$$cpkg" ]; then \
			if echo "$$cpkg" | grep -q "^script:"; then \
				sed -i "s/^$($(1)_NAME) \($$rfile\)$$/$$cpkg \1/" $$sandbox_dir.installed; \
			elif [ "$$cpkg" != "$($(1)_NAME)" ]; then \
				sed -i "/^$($(1)_NAME) $$rfile$$/d" $$sandbox_dir.installed; \
			fi \
		elif [ -n "$$ipkg" ]; then \
			$(call MESSAGE,Error: $(2) file $$file already installed by package $$ipkg); \
			error=1; \
		fi \
	done < $$sandbox_dir.installed; \
	[ -n "$$error" ] && exit 1 || true
endef

GLOBAL_POST_INSTALL_STAGING_HOOKS += GLOBAL_POST_INSTALL_HOOK_CHECK_CONFLICTS
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_CHECK_CONFLICTS
GLOBAL_POST_INSTALL_IMAGES_HOOKS += GLOBAL_POST_INSTALL_HOOK_CHECK_CONFLICTS

define GLOBAL_POST_INSTALL_HOOK_CHECK_DYNAMIC_LINKING
	$(call MESSAGE,Checking dynamic linking in $(2) sandbox)
	error=; \
	deps="$(call GET_COMPLETE_DEPS,$($(1)_NAME))"; \
	echo "$($(1)_NAME) depends on: $$deps"; \
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	rpaths=" "; \
	while read pkg file; do \
		[ -f $$sandbox_dir/$$file -a ! -L $$sandbox_dir/$$file ] || continue; \
		rpath=$$(readelf -d $$sandbox_dir/$$file 2>/dev/null | grep RPATH \
				| sed 's/^.*\[\([^]]*\)\]$$/\1/'); \
		if echo "$$rpath" | grep -q $(BASE_DIR); then \
			$(call MESSAGE,Error: Wrong RPATH $$rpath used by $$file); \
			error=1; \
		else \
			rpath=$$(echo "$$rpath" | sed 's/^\///'); \
			if ! echo "$$rpaths" | grep -qF " $$rpath "; then \
				rpaths="$$rpaths$$rpath "; \
			fi \
		fi \
	done < $$sandbox_dir.installed; \
	while read pkg file; do \
		[ -f $$sandbox_dir/$$file -a ! -L $$sandbox_dir/$$file ] || continue; \
		for lib in $$(readelf -d $$sandbox_dir/$$file 2>/dev/null | grep NEEDED \
				| sed 's/^.*\[\([^]]*\)\]$$/\1/'); do \
			found=; \
			for p in $$rpaths lib usr/lib; do \
				rlibfile=$$(echo $$p/$$lib | sed 's/\[/\\[/g;s/\./\\./g;s/\//\\\//g'); \
				pkg=$$(sed -n "s/^\([^ ]\+\) $$rlibfile$$/\1/p" $(SANDBOX_DIR)/*/$(2).installed 2>/dev/null); \
				if [ -n "$$pkg" ]; then \
					found=1; \
					if ! echo " $($(1)_NAME) $$deps " | grep -q " $$pkg "; then \
						$(call MESSAGE,Error: Library $$lib referenced by $$file belongs to package $$pkg which is not a dependency of $($(1)_NAME)); \
						error=1; \
					fi; \
					break; \
				fi \
			done; \
			if [ -z "$$found" ]; then \
				$(call MESSAGE,Error: Library $$lib referenced by $$file not found); \
				error=1; \
			fi \
		done \
	done < $$sandbox_dir.installed; \
	[ -n "$$error" ] && exit 1 || true
endef

GLOBAL_POST_INSTALL_STAGING_HOOKS += GLOBAL_POST_INSTALL_HOOK_CHECK_DYNAMIC_LINKING
GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_CHECK_DYNAMIC_LINKING

# See http://sourceware.org/gdb/wiki/FAQ, "GDB does not see any threads
# besides the one in which crash occurred; or SIGTRAP kills my program when
# I set a breakpoint" why libpthread-*.so is handled specially
#
# Valgrind needs ld.so with enough information, so only strip
# debugging symbols.
define GLOBAL_POST_INSTALL_HOOK_STRIP
	$(call MESSAGE,Stripping of binaries in $(2) sandbox)
	sandbox_dir=$($(1)_SANDBOX_DIR)/$(2); \
	while read pkg file; do \
		[ -f $$sandbox_dir/$$file -a ! -L $$sandbox_dir/$$file ] || continue; \
		echo $$file | grep -q "\<lib[^/]*\.a$$" && continue; \
		[ -x $$sandbox_dir/$$file ] || continue; \
		readelf -h $$sandbox_dir/$$file >/dev/null 2>&1 || continue; \
		stripcmd="$(STRIPCMD)"; \
		case "$$file" in \
		lib/libpthread-*.so) \
			stripcmd="$$stripcmd $(STRIP_STRIP_DEBUG)";; \
		lib/ld-*.so*) \
			stripcmd="$$stripcmd $(STRIP_STRIP_DEBUG)";; \
		*) \
			stripcmd="$$stripcmd $(STRIP_STRIP_UNNEEDED)";; \
		esac; \
		id=$$($(call GET_BUILD_ID,$$sandbox_dir/$$file)); \
		echo "$$stripcmd $$sandbox_dir/$$file (with build-id=$$id)"; \
		if [ -n "$$id" ]; then \
			debugfile=$$sandbox_dir.debug/$$id; \
			if [ ! -f "$$debugfile" ]; then \
				mkdir -p $$(dirname $$debugfile); \
				$(TARGET_CROSS)objcopy --only-keep-debug $$sandbox_dir/$$file $$debugfile; \
			else \
				echo "debug info already exists, possibly hardlinked file"; \
			fi \
		fi; \
		chmod +w $$sandbox_dir/$$file; \
		$$stripcmd $$sandbox_dir/$$file; \
	done < $$sandbox_dir.installed || true
endef

GLOBAL_POST_INSTALL_TARGET_HOOKS += GLOBAL_POST_INSTALL_HOOK_STRIP
