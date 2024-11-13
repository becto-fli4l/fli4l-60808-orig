DSLTOOL_VERSION = ec7e5da80180ce35494f6020e85f9fb935879c85
DSLTOOL_SITE = https://gitlab.com/dsltool/dsltool.git
DSLTOOL_SITE_METHOD = git
DSLTOOL_DEPENDENCIES = host-pkgconf cairo pango collectd libtelnet libxml2 libpcap

# arguments to configure script
DSLTOOL_CONF_OPTS += -DCMAKE_INSTALL_PREFIX:PATH="/usr"
DSLTOOL_CONF_OPTS += -DWITH_GUI=OFF
DSLTOOL_CONF_OPTS += -DWITH_LOCALE=OFF
DSLTOOL_CONF_OPTS += -DWITH_LONGOPT=OFF
DSLTOOL_CONF_OPTS += -DENABLE_CAPTURE=ON

$(eval $(cmake-package))
