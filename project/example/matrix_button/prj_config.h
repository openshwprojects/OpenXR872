/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PRJ_CONFIG_H_
#define _PRJ_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * project base config
 */

/* main thread priority */
#define PRJCONF_MAIN_THREAD_PRIO        OS_THREAD_PRIO_APP

/* main thread stack size */
#define PRJCONF_MAIN_THREAD_STACK_SIZE  (2 * 1024)

/* sys ctrl enable/disable */
#define PRJCONF_SYS_CTRL_EN             1

/* sys ctrl thread priority */
#define PRJCONF_SYS_CTRL_PRIO           OS_THREAD_PRIO_SYS_CTRL

/* sys ctrl stack size */
#define PRJCONF_SYS_CTRL_STACK_SIZE     (2 * 1024)

/* sys ctrl queue length for receiving message */
#define PRJCONF_SYS_CTRL_QUEUE_LEN      6

/* image flash ID */
#define PRJCONF_IMG_FLASH               0

/* image start address, including bootloader */
#define PRJCONF_IMG_ADDR                0x00000000

/* save sysinfo to flash or not */
#define PRJCONF_SYSINFO_SAVE_TO_FLASH   1

#if PRJCONF_SYSINFO_SAVE_TO_FLASH

/* sysinfo flash ID */
#define PRJCONF_SYSINFO_FLASH           0

/* sysinfo start address */
#define PRJCONF_SYSINFO_ADDR            ((1024 - 4) * 1024)

/* sysinfo size */
#define PRJCONF_SYSINFO_SIZE            (4 * 1024)

/* enable/disable checking whether sysinfo is overlap with image */
#define PRJCONF_SYSINFO_CHECK_OVERLAP   1

#endif /* PRJCONF_SYSINFO_SAVE_TO_FLASH */

/* MAC address source */
#define PRJCONF_MAC_ADDR_SOURCE         SYSINFO_MAC_ADDR_CHIPID

/* watchdog enable/disable */
#define PRJCONF_WDG_EN                  1

/* watchdog timeout value */
#define PRJCONF_WDG_TIMEOUT             WDG_TIMEOUT_16SEC

/* watchdog feeding period (in ms), MUST less than PRJCONF_WDG_TIMEOUT */
#define PRJCONF_WDG_FEED_PERIOD         (10 * 1000)

/*
 * project hardware feature
 */

/* uart enable/disable */
#define PRJCONF_UART_EN                 1

/* h/w crypto engine enable/disable */
#define PRJCONF_CE_EN                   0

/* spi enable/disable */
#define PRJCONF_SPI_EN                  0

/* mmc enable/disable */
#define PRJCONF_MMC_EN                  0

/* mmc detect mode */
#define PRJCONF_MMC_DETECT_MODE         CARD_ALWAYS_PRESENT

/* Xradio internal codec sound card enable/disable */
#define PRJCONF_INTERNAL_SOUNDCARD_EN   0

/* AC107 sound card enable/disable */
#define PRJCONF_AC107_SOUNDCARD_EN      0

/*
 * project service feature
 */

/* console enable/disable */
#define PRJCONF_CONSOLE_EN              1

/* app pm mode enable/disable */
#define PRJCONF_PM_EN                   1

/* network and wlan enable/disable */
#define PRJCONF_NET_EN                  0

/* net pm mode enable/disable */
#define PRJCONF_NET_PM_EN               0

/* environment variable "TZ" for time zone setting */
#define PRJCONF_ENV_TZ                  "TZ=GMT-8"

/* swd enable/disable */
#define PRJCONF_SWD_EN                  0

#ifdef __cplusplus
}
#endif

#endif /* _PRJ_CONFIG_H_ */
