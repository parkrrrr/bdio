include $(TOPDIR)/rules.mk

PKG_NAME:=brltty
PKG_VERSION:=6.0
PKG_INSTALL:=1
PKG_RELEASE:=1
PKG_HASH:=6045d64d5f02002eba51d8fc61d674c8
PKG_BUILD_DIR:=$(BUILD_DIR)/brltty-$(PKG_VERSION)
PKG_SOURCE:=brltty-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://mielke.cc/brltty/archive
PKG_CAT:=bzcat
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/openwrt-install

include $(INCLUDE_DIR)/package.mk

define Package/brltty
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Terminal
  TITLE:=Braille TTY
  URL:=http://mielke.cc/brltty/
endef

define Package/brltty/description
  BRLTTY is a daemon which provides access to the Linux/Unix console
  (when in text mode) for a blind person using a refreshable Braille 
  display.
endef

define Package/brltty/install
    $(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/brltty $(1)/usr/bin/brltty
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbrlapi.a $(1)/usr/lib/libbrlapi.a
	$(INSTALL_DIR) $(1)/usr/include/brltty
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/brltty/*.h $(1)/usr/include/brltty
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/brltty $(STAGING_DIR)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbrlapi.a $(STAGING_DIR)/usr/lib
	mkdir -p $(STAGING_DIR)/usr/include/brltty
	$(CP) $(PKG_INSTALL_DIR)/usr/include/brltty/*.h $(STAGING_DIR)/usr/include/brltty
endef

define Build/Install
	$(call Build/Install/Default,)
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/brltty $(STAGING_DIR)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbrlapi.a $(STAGING_DIR)/usr/lib
	mkdir -p $(STAGING_DIR)/usr/include/brltty
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(STAGING_DIR)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/brltty/*.h $(STAGING_DIR)/usr/include/brltty
endef

CONFIGURE_ARGS += \
	--disable-polkit \
	--disable-java-bindings \
	--disable-lisp-bindings \
	--disable-ocaml-bindings \
	--disable-python-bindings \
	--disable-tcl-bindings \
	--disable-x \
	--disable-contracted-braille \
	--disable-liblouis \
	--disable-gpm \
	--disable-expat \
	--disable-speech-support \
	--enable-standalone-programs \
	--disable-learn-mode \
	--disable-iconv \
	--disable-icu \
	--disable-beeper-support \
	--disable-pcm-support \
	--disable-midi-support \
	--disable-fm-support \
	--enable-standalone-programs \
	--with-braille-driver=all \
	--with-install-root=$(PKG_INSTALL_DIR)
	
define Build/Configure
	# This is ugly, but it gets the job done:
	# Configure needs a host-side version of tbl2hex, but
	# its cross-compilation detection fails so it botches the
	# build. Prebuild it with the options that work for a Linux
	# build host. (This might need to change if you build elsewhere.)
	cd $(PKG_BUILD_DIR)/Programs ; \
	gcc -DHAVE_WCHAR_H -DHAVE_SIGNAL_H -DHAVE_SYSLOG_H \
	-DHAVE_LINUX_INPUT_H -DHAVE_LINUX_UINPUT_H \
	-DHAVE_GETTIMEOFDAY -DHAVE_SETTIMEOFDAY \
	-DHAVE_SELECT -DHAVE_SYS_SELECT_H \
	-DHAVE_DECL_LOCALTIME_R -DHAVE_STIME \
	-DHAVE_CLOCK_GETTIME -DHAVE_TIME -DHAVE_CLOCK_SETTIME \
	-DHAVE_POSIX_THREADS \
	-DTEXT_TABLES_SUBDIRECTORY=\"Text\" \
	-DATTRIBUTES_TABLES_SUBDIRECTORY=\"Attributes\" \
	-DCONTRACTION_TABLES_SUBDIRECTORY=\"Contraction\" \
	-DTEXT_TABLE_EXTENSION=\".ttb\" \
	-DTEXT_SUBTABLE_EXTENSION=\".tti\" \
	-DATTRIBUTES_TABLE_EXTENSION=\".atb\" \
	-DATTRIBUTES_SUBTABLE_EXTENSION=\".ati\" \
	-DCONTRACTION_TABLE_EXTENSION=\".ctb\" \
	-DCONTRACTION_SUBTABLE_EXTENSION=\".cti\" \
	-DPACKAGE_TARNAME=\"brltty\" -DDEVICE_DIRECTORY=\"/dev\" \
	-DPACKAGE_NAME=\"BRLTTY\" -DPACKAGE_VERSION=\"6.0\" \
	-I/. -I.. -I../Headers -I/include -I/usr/include \
	-o tbl2hex-host \
	tbl2hex.c ttb_compile.c ttb_native.c atb_compile.c \
	ctb_compile.c cldr.c program.c pgmpath_none.c pid.c \
	log.c options.c params_none.c file.c dataarea.c \
	charset.c datafile.c parse.c system_linux.c timing.c \
	charset_none.c variables.c queue.c async*.c lock.c \
	thread.c log_history.c unicode.c addresses.c \
	hostcmd_unix.c device.c -lpthread hostcmd.c dynld_none.c 
	
	$(call Build/Configure/Default,)
endef

define Build/Compile
	# PKG_JOBS=brltty-all
	# MAKE_PATH=Programs		
	$(call Build/Compile/Default,)
endef

$(eval $(call BuildPackage,brltty)) 
