include $(TOPDIR)/rules.mk

PKG_NAME:=bdiohid
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
MAKE_PATH:=src

include $(INCLUDE_DIR)/package.mk

define Package/bdiohid
  TITLE:=BDIO HID Converter
  DEPENDS:=+brltty +libstdcpp
endef

define Package/bdiohid/description
  The BDIO HID converter uses brltty to interface with legacy Braille
  displays, while exposing an HUTRR-78 HID Braille interface on the 
  USB gadget interface.
endef

define Package/bdiohid/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bdiohid $(1)/usr/bin/bdiohid
	$(INSTALL_DIR) $(1)/etc/bdio
	$(INSTALL_DATA) ./files/etc/bdio/bdio.db $(1)/etc/bdio/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/bdiohid $(1)/etc/init.d/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
               CC="$(TARGET_CC)" \
			  CXX="$(TARGET_CXX)" \
           CFLAGS="$(TARGET_CFLAGS)" \
	     CXXFLAGS="$(TARGET_CXXFLAGS)" \
	     CPPFLAGS="$(TARGET_CPPFLAGS)" \
          LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Build/Install
	cp $(PKG_BUILD_DIR)/bdiohid $(STAGING_DIR)/bdiohid
	$(INSTALL_DIR) $(STAGING_DIR)/etc/bdio/
	$(INSTALL_DATA) ./files/etc/bdio/bdio.db $(STAGING_DIR)/etc/bdio/
	$(INSTALL_DIR) $(STAGING_DIR)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/bdiohid $(STAGING_DIR)/etc/init.d/
endef

$(eval $(call BuildPackage,bdiohid)) 
