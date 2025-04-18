#
# project local config options, override the global config options
#

# ----------------------------------------------------------------------------
# override global config options
# ----------------------------------------------------------------------------
# enable/disable wlan station mode, default to y
export __CONFIG_WLAN_STA := y

# enable/disable wps for wlan station mode, default to n
export __CONFIG_WLAN_STA_WPS := n

# enable/disable wlan hostap mode, default to y
export __CONFIG_WLAN_AP := y

# must choose flashc(with xip)/spi(no xip) one as flash driver
# default use flashc as flash driver
export __CONFIG_DEFAULT_FLASH_FLASHC := y
export __CONFIG_DEFAULT_FLASH_SPI := n

ifeq ($(__CONFIG_DEFAULT_FLASH_FLASHC), y)
# enable/disable XIP, default to y
export __CONFIG_XIP := y
endif

# enable/disable OTA, default to n
export __CONFIG_OTA := y

# enable/disable low power feature of wlan station mode, default to n
export __CONFIG_WLAN_STA_LP := n

export __CONFIG_WLAN_STA_SOFTAP_COEXIST := n
