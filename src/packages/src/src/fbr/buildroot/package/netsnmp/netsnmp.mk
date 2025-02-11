################################################################################
#
# netsnmp
#
################################################################################

NETSNMP_VERSION = 5.8
NETSNMP_SITE = https://downloads.sourceforge.net/project/net-snmp/net-snmp/$(NETSNMP_VERSION)
NETSNMP_SOURCE = net-snmp-$(NETSNMP_VERSION).tar.gz
NETSNMP_LICENSE = Various BSD-like
NETSNMP_LICENSE_FILES = COPYING
NETSNMP_INSTALL_STAGING = YES
NETSNMP_CONF_ENV = ac_cv_NETSNMP_CAN_USE_SYSCTL=no CFLAGS="$(TARGET_CFLAGS) -D_BSD_SOURCE -D_XOPEN_SOURCE=500"
NETSNMP_CONF_OPTS = \
	--with-persistent-directory=/var/lib/snmp \
	--with-defaults \
	--enable-mini-agent \
	--without-rpm \
	--with-logfile=none \
	--without-kmem-usage \
	--enable-as-needed \
	--without-perl-modules \
	--disable-embedded-perl \
	--disable-perl-cc-checks \
	--disable-scripts \
	--with-default-snmp-version="1" \
	--enable-silent-libtool \
	--enable-mfd-rewrites \
	--with-sys-contact="root@localhost" \
	--with-sys-location="Unknown" \
	--with-mib-modules="$(call qstrip,$(BR2_PACKAGE_NETSNMP_WITH_MIB_MODULES))" \
	--with-out-mib-modules="$(call qstrip,$(BR2_PACKAGE_NETSNMP_WITHOUT_MIB_MODULES))" \
	--with-out-transports="Unix" \
	--disable-manuals
NETSNMP_INSTALL_STAGING_OPTS = DESTDIR=$(STAGING_DIR) LIB_LDCONFIG_CMD=true install
NETSNMP_INSTALL_TARGET_OPTS = DESTDIR=$(TARGET_DIR) LIB_LDCONFIG_CMD=true install
NETSNMP_MAKE = $(MAKE1)
NETSNMP_CONFIG_SCRIPTS = net-snmp-config
NETSNMP_AUTORECONF = YES
NETSNMP_DEPENDENCIES = pciutils libnl

ifeq ($(BR2_ENDIAN),"BIG")
NETSNMP_CONF_OPTS += --with-endianness=big
else
NETSNMP_CONF_OPTS += --with-endianness=little
endif

ifeq ($(BR2_PACKAGE_LIBNL),y)
NETSNMP_DEPENDENCIES += host-pkgconf libnl
NETSNMP_CONF_OPTS += --with-nl
else
NETSNMP_CONF_OPTS += --without-nl
endif

# PCRE
ifeq ($(BR2_PACKAGE_PCRE),y)
NETSNMP_DEPENDENCIES += pcre
NETSNMP_CONF_OPTS += --with-pcre
else
NETSNMP_CONF_OPTS += --without-pcre
endif

# libpcap
ifeq ($(BR2_PACKAGE_LIBPCAP),y)
NETSNMP_DEPENDENCIES += libpcap
endif

# OpenSSL
ifeq ($(BR2_PACKAGE_OPENSSL),y)
NETSNMP_DEPENDENCIES += openssl
NETSNMP_CONF_OPTS += \
	--with-openssl=$(STAGING_DIR)/usr/include/openssl \
	--with-security-modules="tsm,usm" \
	--with-transports="DTLSUDP,TLSTCP"
ifeq ($(BR2_STATIC_LIBS),y)
# openssl uses zlib, so we need to explicitly link with it when static
NETSNMP_CONF_ENV += LIBS=-lz
endif
else ifeq ($(BR2_PACKAGE_NETSNMP_OPENSSL_INTERNAL),y)
NETSNMP_CONF_OPTS += --with-openssl=internal
else
NETSNMP_CONF_OPTS += --without-openssl
endif

# There's no option to forcibly enable or disable it
ifeq ($(BR2_PACKAGE_PCIUTILS),y)
NETSNMP_DEPENDENCIES += pciutils
endif

# For ucd-snmp/lmsensorsMib
ifeq ($(BR2_PACKAGE_LM_SENSORS),y)
NETSNMP_DEPENDENCIES += lm-sensors
endif

ifneq ($(BR2_PACKAGE_NETSNMP_ENABLE_MIBS),y)
NETSNMP_CONF_OPTS += --disable-mib-loading
NETSNMP_CONF_OPTS += --disable-mibs
endif

ifneq ($(BR2_PACKAGE_NETSNMP_ENABLE_DEBUGGING),y)
NETSNMP_CONF_OPTS += --disable-debugging
endif

ifeq ($(BR2_PACKAGE_NETSNMP_SERVER),y)
NETSNMP_CONF_OPTS += --enable-agent
else
NETSNMP_CONF_OPTS += --disable-agent
endif

ifeq ($(BR2_PACKAGE_NETSNMP_CLIENTS),y)
NETSNMP_CONF_OPTS += --enable-applications
else
NETSNMP_CONF_OPTS += --disable-applications
endif

ifeq ($(BR2_PACKAGE_NETSNMP_SERVER),y)
define NETSNMP_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/netsnmp/S59snmpd \
		$(TARGET_DIR)/etc/init.d/S59snmpd
endef
endif

define NETSNMP_STAGING_NETSNMP_CONFIG_FIXUP
	$(SED)	"s,^includedir=.*,includedir=\'$(STAGING_DIR)/usr/include\',g" \
		-e "s,^libdir=.*,libdir=\'$(STAGING_DIR)/usr/lib\',g" \
		$(STAGING_DIR)/usr/bin/net-snmp-config
endef

#NETSNMP_POST_INSTALL_STAGING_HOOKS += NETSNMP_STAGING_NETSNMP_CONFIG_FIXUP

$(eval $(autotools-package))
