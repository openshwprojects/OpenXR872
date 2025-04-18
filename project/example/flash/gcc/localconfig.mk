#
# project local config options, override the global config options
#

# ----------------------------------------------------------------------------
# override global config options
# ----------------------------------------------------------------------------
# enable/disable wlan, default to y
export __CONFIG_WLAN := n

# must choose flashc(with xip)/spi(no xip) one as flash driver
# default use flashc as flash driver
export __CONFIG_DEFAULT_FLASH_FLASHC := y
export __CONFIG_DEFAULT_FLASH_SPI := n

ifeq ($(__CONFIG_DEFAULT_FLASH_FLASHC), y)
# enable/disable XIP, default to y
export __CONFIG_XIP := n
endif
