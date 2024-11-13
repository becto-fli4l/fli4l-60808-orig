MMENCODE_VERSION = 1.0
MMENCODE_SOURCE = 
MMENCODE_SITE = $(FLI4L_SRCDIR)/mmencode
MMENCODE_SITE_METHOD = local

# arguments to configure script
MMENCODE_AUTORECONF = YES
MMENCODE_AUTORECONF_OPTS = -ivf

$(eval $(autotools-package))
