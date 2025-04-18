# ChangeLog for XRADIO Skylark SDK

## xradio_skylark_sdk-1.2.2 (20230203)

  * WLAN
    - Add an API to simplify the configuration of WLAN keep alive.
    - Optimize the API of getting WPA type.
    - Support automatic power control.
    - Add an API to config parameters of low power.
    - Optimize function of receiving beacon.
    - Add some APIs for WMM debug.
    - Optimize anti-interference performance.
    - Fix some bugs about wlan low power.

  * Cedarx
    - Support creating multiple players.
    - Fix a bug that cedarx can not play the AAC media smaller than 4k.
    - Fix memory corruption in AAC decoder.
    - Fix a bug about playing AAC media failed.

  * Driver
    - Add an API to config quality of JPEG.
    - Fix the SPI de-init bug to avoid reboot fail.
    - Update SDMMC driver to fix the detect failed issue.

  * Network
    - Add an API to get mac address by IP address.

  * System:
    - Support C++.

  * Misc
    - Tools(phoenixMC): Add mass production mode.
    - Tools(phoenixMC): Optimize the procedure to make synchronization faster.
    - Tools(phoenixMC): Fix the verification failed issue when settings_win.ini is missing.
    - Tools(phoenixMC): Optimize the debug log.
    - Add the C++ example.


## xradio_skylark_sdk-1.2.1 (20220426)

  * WLAN
    - Support more low power features.
    - Support connecting to a AP with the specific MAC address.
    - Add an API to config the threshold of BSS lost.
    - Add event notifies about station connection and disconnection in AP mode.
    - Fix an issue about fast connection in WPA3 mode.
    - Fix an issue about connecting to the google AP.

  * System:
    - Reset the NOR Flash pinmux to fix the reboot failed issue.
    - Config DMA heap to uncacheable to fix DMA memory issues when PSRAM is enabled.


## xradio_skylark_sdk-1.2.0 (20210625)

  * WLAN
    - Update wpa_supplicant to v2.9 to support WPA3
    - Add an API to config ps policy
    - Add commands for monitor mode
    - Fix a system blocking issue caused by wlan rx

  * Cedarx
    - Add config APIs to control output of log
    - Fix a bug about http seek fail
    - Fix a blocking issue when play using FIFO stream
    - Fix a bug about getting current audio position

  * Audio
    - Support DRC
    - Fix a POP sound issue

  * Driver
    - SD: fix a bug about reading/writing failed

  * Network
    - Nghttp2: add nghttp2 to support http2
    - Lwip: support lwip2.1.2
    - DHCPD: fix a connecting issue in AP mode
    - Iperf: fix exit error log

  * Debug
    - Support backtrace
    - Support watchpoint

  * System:
    - Optimize memory manager
    - Add an API to get voltage and temperature
    - Add an API to get startup state
    - Support configuration to enable Idle hook

  * Misc
    - Tools: Support production mode in PhoenixMC


## xradio_skylark_sdk-1.1.0 (20200605)

  * WLAN
    - Optimize rate control algorithm
    - Optimize TX power calibration algorithm
    - Support changing the default configuration of AP mode
    - Support flushing AP scan result
    - Fix a bug about command timeout
    - Fix some bugs about PHY reset
    - Smart config: fix a bug about decoding lead code failed

  * Cedarx
    - Fix a bug about playing HTTPS stream failed
    - Adjust the implementation of os glue layer
    - Support TS stream

  * Audio
    - Fix bugs about setting volume and route
    - Optimize the time of starting play
    - Support multi output sample rate for audio player
    - Support EQ
    - Support opus
    - Support speex

  * Driver
    - EFUSE: fix timing parameter's issue
    - GPIO: fix a bug about cannot use GPIO interrupt after watchdog reboot
    - I2C: fix a bug about accessing 16-bit memory address
    - DMA: fix bugs of transfer using PSRAM
    - PSRAM: improve performance
    - WDG: reset wlan cpu in HAL_WDG_Reboot()
    - Camera: refactor some functions and support suspend/resume
    - I2S: refactor
    - Audio codec: support AC101

  * Network
    - Ping: support more parameters setting

  * OS
    - support FreeRTOS 10.2.1

  * System:
    - Adjust LDO1 voltage according to eFUSE value
    - Support setting CPU to 349MHz

  * Misc
    - Keep external LDO mode when enter hibernation
    - Support compressed PSRAM bin
    - Console: support echo function and help command
    - OTA: support upgrade status feedback and push mode
    - Tools: fix some bugs


## xradio_skylark_sdk-1.0.2 (20191223)

  * Driver
    - FLASH: Optimize PM operation

  * Network
    - libwebsocket: Fix a bug about memory leakage

  * Misc
    - SDD Editor Ex: update to v2.5.1912a
    - Sound Config Apk: update to v1.1.2s


## xradio_skylark_sdk-1.0.1 (20191211)

  * WLAN
    - Optimize RF power calibration algorithm

  * Misc
    - Improve the flow of setting PRNG seed
    - Adjust the flow of setting SoC voltages
    - Improve the configuration flow of D-Cache
    - Add the QR example


## xradio_skylark_sdk-1.0.0 (20191129)

  * Driver
    - CACHE: fix several bugs of cache operations
    - SPI: support slave mode
    - CAMERA: fix a bug about CSI exception
    - Audio Codec: support delay after setting PA on

  * WLAN
    - Optimize EVM
    - Add an API to set rx queue size

  * Misc
    - Fix a bug of setting sram retention
    - Support decoding QR code


## xradio_skylark_sdk-1.0.0-rc3 (20191115)

  * WLAN
    - Improve the RF performance
    - Fix a bug about dpd calculation exception
    - Increase limitation for 802.11b tx power
    - Fix bugs of smart config to avoid memory writing out of bounds
    - Add two new APIs for sta mode and ap mode

  * Driver
    - CACHE: adjust some h/w operations according to spec

  * System
    - Support PSRAM heap trace function
    - adjust board config names


## xradio_skylark_sdk-1.0.0-rc2 (20191104)

  * Driver
    - PSRAM: Improve the performance and stability
    - IR: support 40M HOSC
    - CSI/JPEG: improve the working flow and fix several small issues
    - DMA: fix data inconsistent issue when using cache
    - FLASH: add support for P25Q64H

  * WLAN
    - Improve working efficiency and power
    - Fix a bug about memory leakage in some cases
    - Fix a bug about failed to connect the open AP after setting WEP keys
    - Add APIs to get the number of scan results

  * Cedarx
    - Fix a bug of blocking when stop playing in some cases
    - Fix some issues about playing AAC stream and HTTP stream
    - Improve the speed of parsing MP3 stream
    - Add an API to set buffer size for playing HTTP stream

  * System
    - Add heap usage mode (SRAM or PSRAM) configurations for some modules
    - Add some policies to improve the system's stability

  * Misc
    - FOTA: support image compression policy
    - Bootloader: fix a bug of decompressing bin files failed
    - Add some project examples


## xradio_skylark_sdk-1.0.0-rc1 (20190925)

  * First release candidate for XRADIO Skylark SDK
