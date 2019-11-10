include $(TOPDIR)/rules.mk

PKG_NAME:=bdiohid
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

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
endef

define Build/Compile
	cd src; $(TARGET_CC) $(TARGET_CPPFLAGS) \
	  -Iinclude \
	  -Isqlitecpp/sqlite3 \
	  -Isqlitecpp/include \
	  -o $(PKG_BUILD_DIR)/bdiohid \
	  *.cpp \
	  sqlitecpp/src/*.cpp \
	  sqlitecpp/sqlite3/sqlite3.c \
	  $(TARGET_LDFLAGS) \
	  -lstdc++ \
	  -lbrlapi
endef

define Build/Install
	cp $(PKG_BUILD_DIR)/bdiohid $(STAGING_DIR)/bdiohid
endef

$(eval $(call BuildPackage,bdiohid)) 