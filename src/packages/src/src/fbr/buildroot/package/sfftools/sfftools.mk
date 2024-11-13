SFFTOOLS_VERSION = 29
SFFTOOLS_RELEASE = 3_1_4
SFFTOOLS_SITE_METHOD = svn
SFFTOOLS_SITE = svn://svn.code.sf.net/p/sfftools/code-0/sfftobmp3/tags/REL_$(SFFTOOLS_RELEASE)
SFFTOOLS_SOURCE = sfftools-REL_$(SFFTOOLS_RELEASE).tar.gz

SFFTOOLS_LICENSE_FILES = doc/copying

SFFTOOLS_AUTORECONF = YES
SFFTOOLS_AUTORECONF_OPTS = -ivf
SFFTOOLS_DEPENDENCIES = boost tiff jpeg

# do some cleanup before patching
# remove unnecessary files, add m4 directory 
# and convert to unix line endings
define SFFTOOLS_DO_BUILD_PREPARE
	rm -rf $(@D)/.settings/
	rm -rf $(@D)/win32/
	rm -rf $(@D)/config/
	rm -f $(@D)/.*project
	rm -f $(@D)/*.in $(@D)/src/*.in
	rm -f $(@D)/configure
	rm -f $(@D)/*.m4
	mkdir -p $(@D)/m4
	$(SED) 's/\r$$//g' $(@D)/*.am $(@D)/*.ac $(@D)/src/*
endef
SFFTOOLS_POST_EXTRACT_HOOKS += SFFTOOLS_DO_BUILD_PREPARE

$(eval $(autotools-package))
