################################################################################
#
# qt5webkit
#
################################################################################

# no 5.9.2 package available, fall back to 5.9.1 version
ifeq ($(BR2_PACKAGE_QT5_VERSION_LATEST),y)
QT5WEBKIT_VERSION = 5.9.1
QT5WEBKIT_SITE = https://download.qt.io/official_releases/qt/5.9/5.9.1/submodules
else
QT5WEBKIT_VERSION = $(QT5_VERSION)
QT5WEBKIT_SITE = https://download.qt.io/community_releases/5.6/$(QT5_VERSION)
endif

QT5WEBKIT_SOURCE = qtwebkit-opensource-src-$(QT5WEBKIT_VERSION).tar.xz
QT5WEBKIT_DEPENDENCIES = \
	host-bison host-flex host-gperf host-python host-ruby \
	qt5base sqlite
QT5WEBKIT_INSTALL_STAGING = YES

QT5WEBKIT_LICENSE_FILES = Source/WebCore/LICENSE-LGPL-2 Source/WebCore/LICENSE-LGPL-2.1

QT5WEBKIT_LICENSE = LGPL-2.1+, BSD-3-Clause, BSD-2-Clause
# Source files contain references to LGPL_EXCEPTION.txt but it is not included
# in the archive.
QT5WEBKIT_LICENSE_FILES += LICENSE.LGPLv21

ifeq ($(BR2_PACKAGE_QT5BASE_XCB),y)
QT5WEBKIT_DEPENDENCIES += xlib_libXext xlib_libXrender
endif

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE),y)
QT5WEBKIT_DEPENDENCIES += qt5declarative
endif

# QtWebkit's build system uses python, but only supports python2. We work
# around this by forcing python2 early in the PATH, via a python->python2
# symlink.
QT5WEBKIT_ENV = PATH=$(@D)/host-bin:$(BR_PATH)
define QT5WEBKIT_PYTHON2_SYMLINK
	mkdir -p $(@D)/host-bin
	ln -sf $(HOST_DIR)/bin/python2 $(@D)/host-bin/python
endef
QT5WEBKIT_PRE_CONFIGURE_HOOKS += QT5WEBKIT_PYTHON2_SYMLINK

# The mesa's EGL/eglplatform.h header includes X11 headers unless the flag
# MESA_EGL_NO_X11_HEADERS is defined. Tell to not include X11 headers if
# the libxcb is not selected.
ifeq ($(BR2_PACKAGE_MESA3D_OPENGL_EGL)x$(BR2_PACKAGE_LIBXCB),yx)
QT5WEBKIT_QMAKEFLAGS += QMAKE_CXXFLAGS+=-DMESA_EGL_NO_X11_HEADERS
endif

define QT5WEBKIT_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) $(QT5WEBKIT_ENV) $(HOST_DIR)/bin/qmake $(QT5WEBKIT_QMAKEFLAGS))
endef

define QT5WEBKIT_BUILD_CMDS
	+$(TARGET_MAKE_ENV) $(QT5WEBKIT_ENV) $(MAKE) -C $(@D)
endef

define QT5WEBKIT_INSTALL_STAGING_CMDS
	+$(TARGET_MAKE_ENV) $(QT5WEBKIT_ENV) $(MAKE) -C $(@D) install
	$(QT5_LA_PRL_FILES_FIXUP)
endef

ifeq ($(BR2_PACKAGE_QT5DECLARATIVE_QUICK),y)
define QT5WEBKIT_INSTALL_TARGET_QMLS
	cp -dpfr $(STAGING_DIR)/usr/qml/QtWebKit $(TARGET_DIR)/usr/qml/
endef
endif

define QT5WEBKIT_INSTALL_TARGET_CMDS
	cp -dpf $(STAGING_DIR)/usr/lib/libQt5WebKit*.so.* $(TARGET_DIR)/usr/lib
	cp -dpf $(@D)/bin/* $(TARGET_DIR)/usr/bin/
	$(QT5WEBKIT_INSTALL_TARGET_QMLS)
endef

$(eval $(generic-package))
