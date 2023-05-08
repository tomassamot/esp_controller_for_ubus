include $(TOPDIR)/rules.mk

PKG_NAME:=ubus_esp
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/ubus_esp
	CATEGORY:=Base system
	TITLE:=ubus_esp
	DEPENDS:=+libubus +libubox +libblobmsg-json +libserialport
endef

define Package/ubus_esp/description
	C Program for establishing communications between ubus and esp controllers
endef

define Package/ubus_esp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ubus_esp $(1)/usr/bin/ubus_esp
endef

$(eval $(call BuildPackage,ubus_esp))
