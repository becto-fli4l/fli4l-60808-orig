SS5_VERSION = 3.8.9-8
SS5_SITE = http://downloads.sourceforge.net/project/ss5/ss5/$(SS5_VERSION)
SS5_SOURCE = ss5-$(SS5_VERSION).tar.gz
SS5_CONF_OPTS = --with-configfile=/etc/ss5.conf --with-passwordfile=/etc/ss5.passwd --without-mysql
SS5_CONF_ENVS = ac_cv_lib_ldap_main=no ac_cv_lib_pam_main=no ac_cv_lib_pam_misc_main=no

SS5_DEPENDENCIES = openssl

SS5_LICENSE = GPLv2+, OpenSSL
SS5_LICENSE_FILES = License \
                    License.OpenSSL

SS5_INSTALL_TARGET_OPTS = dst_dir=$(TARGET_DIR) install

$(eval $(autotools-package))
