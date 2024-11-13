# the package version
TMPL_AUTOCONF_VERSION = 1.0
# the name of the source archive (without a path!)
TMPL_AUTOCONF_SOURCE  = tmpl_autoconf-$(TMPL_AUTOCONF_VERSION).tar.bz2
# the download URL (the name of source archive will be appended automatically!)
TMPL_AUTOCONF_SITE    = http://www.example.com/tmpl_autoconf/downloads

# the package dependencies
TMPL_AUTOCONF_DEPENDENCIES = host-pkg-config openssl

# arguments to configure script
TMPL_AUTOCONF_CONF_OPT = --without-x
# environment variables to set for configure script
TMPL_AUTOCONF_CONF_ENV = ac_cv_va_copy=yes

$(eval $(autotools-package))
