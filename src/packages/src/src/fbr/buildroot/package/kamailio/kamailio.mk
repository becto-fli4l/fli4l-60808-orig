KAMAILIO_VERSION = 4.2.3
KAMAILIO_SOURCE  = kamailio-$(KAMAILIO_VERSION)_src.tar.gz
KAMAILIO_SITE    = http://www.kamailio.org/pub/kamailio/$(KAMAILIO_VERSION)/src

KAMAILIO_LICENSE = GPLv2+
KAMAILIO_LICENSE_FILES = COPYING

KAMAILIO_DEPENDENCIES += expat libxml2 libcurl libconfuse sqlite pcre iptables host-bison

KAMAILIO_GROUP_INCLUDE   = "standard presence experimental"
KAMAILIO_INCLUDE_MODULES = "acc alias_db async auth auth_db avp avpops blst call_control carrierroute cfg_db counters cpl-c db2_ops db_flatstore db_sqlite db_text dialog dialog_ng dialplan dispatcher diversion domain drouting enum exec group htable imc ipops iprtpproxy lcr mangler matrix maxfwd msilo nat_traversal nathelper outbound p_usrloc path permissions pipelimit prefix_route presence presence_conference presence_dialoginfo presence_mwi presence_profile presence_reginfo presence_xml print pua pua_bla pua_dialoginfo pua_mi pua_reginfo pua_usrloc pua_xmpp ratelimit regex registrar rls rtimer rtpproxy  rtpengine sanity sca sdpops sipcapture siptrace siputils sl speeddial sqlops textops textopsx timer tm tmrec topoh uac uac_redirect uid_auth_db uid_domain uid_gflags uid_avp_db uri_db userblacklist usrloc utils rtpproxy-ng"

# missing modules: json 

define KAMAILIO_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) PREFIX=/usr \
		CC="$(TARGET_CC)" LD="$(TARGET_CC)" LD_RPATH="" \
		CC_OPT="$(TARGET_CFLAGS)" LD_EXTRA_OPTS="$(TARGET_LDFLAGS)" \
		LOCALBASE=$(STAGING_DIR)/usr group_include=$(KAMAILIO_GROUP_INCLUDE) \
		include_modules=$(KAMAILIO_INCLUDE_MODULES) CROSS_COMPILE=true \
		LEX=$(HOST_DIR)/bin/flex YACC=$(HOST_DIR)/bin/bison \
		LIBDIR=lib all
endef

define KAMAILIO_INSTALL_TARGET_CMDS
	+$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) PREFIX=/usr DESTDIR=$(TARGET_DIR) \
		CROSS_COMPILE=true \
		install
endef

$(eval $(generic-package))
