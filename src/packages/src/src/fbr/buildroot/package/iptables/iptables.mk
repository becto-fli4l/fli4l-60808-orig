################################################################################
#
# iptables
#
################################################################################

IPTABLES_VERSION = 1.6.2
IPTABLES_SOURCE = iptables-$(IPTABLES_VERSION).tar.bz2
IPTABLES_SITE = http://ftp.netfilter.org/pub/iptables
IPTABLES_INSTALL_STAGING = YES
IPTABLES_DEPENDENCIES = host-pkgconf \
	$(if $(BR2_PACKAGE_LIBNETFILTER_CONNTRACK),libnetfilter_conntrack)
IPTABLES_LICENSE = GPL-2.0
IPTABLES_LICENSE_FILES = COPYING
IPTABLES_IMQ_SITE  = http://www.linuximq.net/patches
IPTABLES_IMQ_PATCH = iptables-1.4.13-IMQ-test1.diff
IPTABLES_CONF_OPTS = --libexecdir=/lib --with-kernel=$(STAGING_DIR)/usr \
	--with-pkgconfigdir=/usr/lib/pkgconfig \
	--libdir=/lib --sbindir=/sbin
# we don't want to have a bunch of libxt libraries but prefer to have them
# linked into the executable
IPTABLES_CONF_OPTS += --enable-static
# does not work with --enable-static
IPTABLES_CONF_OPTS += --disable-connlabel

# For connlabel match
ifeq ($(BR2_PACKAGE_LIBNETFILTER_CONNTRACK),y)
IPTABLES_DEPENDENCIES += libnetfilter_conntrack
endif

# For nfnl_osf
ifeq ($(BR2_PACKAGE_LIBNFNETLINK),y)
IPTABLES_DEPENDENCIES += libnfnetlink
endif

# For iptables-compat tools
ifeq ($(BR2_PACKAGE_IPTABLES_NFTABLES),y)
IPTABLES_CONF_OPTS += --enable-nftables
IPTABLES_DEPENDENCIES += host-bison host-flex libmnl libnftnl
else
IPTABLES_CONF_OPTS += --disable-nftables
endif

# bpf compiler support and nfsynproxy tool
ifeq ($(BR2_PACKAGE_IPTABLES_BPF_NFSYNPROXY),y)
# libpcap is tricky for static-only builds and needs help
ifeq ($(BR2_STATIC_LIBS),y)
IPTABLES_LIBS_FOR_STATIC_LINK += `$(STAGING_DIR)/usr/bin/pcap-config --static --additional-libs`
IPTABLES_CONF_OPTS += LIBS="$(IPTABLES_LIBS_FOR_STATIC_LINK)"
endif
IPTABLES_CONF_OPTS += --enable-bpf-compiler --enable-nfsynproxy
IPTABLES_DEPENDENCIES += libpcap
else
IPTABLES_CONF_OPTS += --disable-bpf-compiler --disable-nfsynproxy
endif

define IPTABLES_DOWNLOAD_IMQ_PATCH
	$(call DOWNLOAD,$(IPTABLES_IMQ_SITE)/$(IPTABLES_IMQ_PATCH))
endef

IPTABLES_POST_DOWNLOAD_HOOKS += IPTABLES_DOWNLOAD_IMQ_PATCH

define IPTABLES_APPLY_IMQ_PATCH
	support/scripts/apply-patches.sh $(@D) $(DL_DIR) $(IPTABLES_IMQ_PATCH)
endef

IPTABLES_POST_PATCH_HOOKS += IPTABLES_APPLY_IMQ_PATCH

IPTABLES_POST_INSTALL_TARGET_HOOKS += IPTABLES_TARGET_SYMLINK_CREATE

IPTABLES_POST_INSTALL_TARGET_HOOKS += IPTABLES_TARGET_IPV6_SYMLINK_CREATE

$(eval $(autotools-package))
