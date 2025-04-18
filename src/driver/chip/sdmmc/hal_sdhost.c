/**
  * @file  hal_sdhost.c
  * @author  XRADIO IOT WLAN Team
  */

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

#include "sys/io.h"

#include "driver/chip/private/hal_debug.h"
#include "driver/chip/hal_prcm.h"
#include "driver/chip/hal_ccm.h"
#include "driver/chip/hal_nvic.h"
#include "driver/chip/hal_gpio.h"
#include "driver/hal_board.h"

#include "driver/hal_dev.h"
#include "driver/chip/hal_clock.h"

#include "driver/chip/sdmmc/hal_sdhost.h"
#ifdef CONFIG_USE_SDIO
#include "driver/chip/sdmmc/sdio.h"
#endif
#include "driver/chip/sdmmc/sdmmc.h"

#include "_sd_define.h"
#include "_sdhost.h"
#include "_core.h"

#include "pm/pm.h"

#define SDC_REQUEST_IRQ(n, hdl)         HAL_NVIC_SetIRQHandler(n, hdl)
#define SDC_SetPriority(n, l)           HAL_NVIC_SetPriority(n, l)
#define SDC_ENABLE_IRQ(n)               HAL_NVIC_EnableIRQ(n)
#define SDC_CONFIG_IRQ(n, hdl, l)       HAL_NVIC_ConfigExtIRQ(n, hdl, l)
#define SDC_DISABLE_IRQ(n)              HAL_NVIC_DisableIRQ(n)
#define SDC_CLEAR_IRQPINGD(n)           HAL_NVIC_ClearPendingIRQ(n)
#define SDC_IRQHandler                  NVIC_IRQHandler

//#define NUSE_STANDARD_INTERFACE  1
#ifdef NUSE_STANDARD_INTERFACE
#define SDC_CCM_BASE                    (0x40040400)
#define SDC_CCM_SDC0_SCLK_CTRL          (SDC_CCM_BASE + 0x028)
#else
#define SDC0_CCM_BusForceReset()        HAL_CCM_BusForcePeriphReset(CCM_BUS_PERIPH_BIT_SDC0)
#define SDC0_CCM_BusReleaseRest()       HAL_CCM_BusReleasePeriphReset(CCM_BUS_PERIPH_BIT_SDC0)
#define SDC0_CCM_BusEnableClock()       HAL_CCM_BusEnablePeriphClock(CCM_BUS_PERIPH_BIT_SDC0)
#define SDC0_CCM_BusDisableClock()      HAL_CCM_BusDisablePeriphClock(CCM_BUS_PERIPH_BIT_SDC0)
#define SDC0_CCM_EnableMClock()         HAL_CCM_SDC0_EnableMClock()
#define SDC0_DisableMClock              HAL_CCM_SDC0_DisableMClock
#define SDC0_SetMClock                  HAL_CCM_SDC0_SetMClock

#if defined(__CONFIG_CHIP_XR875_ON_XR871)
#define SDC1_SUPPORT                    1
#define SDC1_CCM_BusForceReset()        HAL_CCMN_BusForcePeriphReset(CCMN_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_BusReleaseRest()       HAL_CCMN_BusReleasePeriphReset(CCMN_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_BusEnableClock()       HAL_CCMN_BusEnablePeriphClock(CCMN_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_BusDisableClock()      HAL_CCMN_BusDisablePeriphClock(CCMN_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_EnableMClock()         HAL_CCMN_SDC1_EnableMClock()
#define SDC1_DisableMClock              HAL_CCMN_SDC1_DisableMClock
#define SDC1_SetMClock                  HAL_CCMN_SDC1_SetMClock
#elif defined(__CONFIG_CHIP_XR875)
#define SDC1_SUPPORT                    1
#define SDC1_CCM_BusForceReset()        HAL_CCM_BusForcePeriphReset(CCM_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_BusReleaseRest()       HAL_CCM_BusReleasePeriphReset(CCM_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_BusEnableClock()       HAL_CCM_BusEnablePeriphClock(CCM_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_BusDisableClock()      HAL_CCM_BusDisablePeriphClock(CCM_BUS_PERIPH_BIT_SDC1)
#define SDC1_CCM_EnableMClock()         HAL_CCM_SDC1_EnableMClock()
#define SDC1_DisableMClock              HAL_CCM_SDC1_DisableMClock
#define SDC1_SetMClock                  HAL_CCM_SDC1_SetMClock
#endif
#endif /* NUSE_STANDARD_INTERFACE */

#ifndef SDC1_SUPPORT
#define SDC1_SUPPORT 0
#endif

#define MEMS_VA2PA(x) (x)
#define SDXC_REG_NTSR                   (0x5C)
#define SDXC_REG_DELAY_CTRL0            (0x140)
#define SDXC_REG_DELAY_CTRL             (0x144)

#define mci_readl(host, reg) \
	readl((uint32_t)(host)->reg_base + reg)
#define mci_writel(value, host, reg) \
	writel((value), (uint32_t)(host)->reg_base + reg)

uint32_t _mci_dbg_flg = 0;
extern struct mmc_host *_mci_host[SDC_NUM];
extern struct soc_device sdc_dev[SDC_NUM];
#define SDC_DEV(id) (&sdc_dev[id])

extern void __mci_dat3_det(void *arg);
extern void __mci_cd_irq(void *arg);
extern void __mci_cd_timer(void *arg);
extern void SDC0_IRQHandler(void);
extern  void __mci_clk_prepare_enable(struct mmc_host *host);
extern void __mci_hold_io(struct mmc_host *host);
extern void __mci_restore_io(struct mmc_host *host);

static __inline void __mci_sel_access_mode(struct mmc_host *host, uint32_t access_mode)
{
	mci_writel((mci_readl(host, SDXC_REG_GCTRL) & (~SDXC_ACCESS_BY_AHB)) | access_mode,
	           host, SDXC_REG_GCTRL);
}

int32_t __mci_wait_access_done(struct mmc_host *host)
{
	int32_t own_set = 0;
	int32_t timeout = 0x0f0000;

	while (!(mci_readl(host, SDXC_REG_GCTRL) & SDXC_MemAccessDone) && timeout--)
		;
	if (!(mci_readl(host, SDXC_REG_GCTRL) & SDXC_MemAccessDone)) {
		SDC_LOGW("wait memory access done timeout !!\n");
	}

	return own_set;
}

#ifdef CONFIG_DETECT_CARD
static void __mci_enable_cd_pin_irq(struct mmc_host *host)
{
	GPIO_IrqParam Irq_param;

	Irq_param.event = GPIO_IRQ_EVT_BOTH_EDGE;
	Irq_param.callback = &__mci_cd_irq;
	Irq_param.arg = host;
	HAL_GPIO_EnableIRQ(host->cd_port, host->cd_pin, &Irq_param);
}
#endif

/**
 * @brief Initializes the SDC peripheral.
 * @param sdc_id:
 *        @arg sdc_id->SDC ID.
 * @param param:
 *        @arg param->[in] The configuration information.
 * @retval  SDC handler.
 */
struct mmc_host *_HAL_SDC_Init(struct mmc_host *host)
{
#ifdef __CONFIG_ARCH_APP_CORE
	const HAL_SDCGPIOCfg *sd_gpio_cfg = NULL;
#endif

	if (host->State != SDC_STATE_RESET) {
		SDC_LOGW("%s reinit sdc!\n", __func__);
		return NULL;
	}

	if (host->sdc_id == 0) {
		host->reg_base = (volatile void *)SMC0_BASE;
#if SDC1_SUPPORT
	} else if (host->sdc_id == 1) {
		host->reg_base = (volatile void *)SMC1_BASE;
#endif
	} else {
		SDC_LOGW("%s unsupport sdc id:%d!\n", __func__, host->sdc_id);
		return NULL;
	}

#ifdef __CONFIG_PLATFORM_FPGA
	host->caps = MMC_CAP_MMC_HIGHSPEED  | MMC_CAP_WAIT_WHILE_BUSY |
		     MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 | MMC_CAP_UHS_SDR50;
	if (HAL_GetDevClock() > 25000000) {
		host->caps |= MMC_CAP_SD_HIGHSPEED;
	}
#else
	host->caps = MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED | MMC_CAP_WAIT_WHILE_BUSY |
	             MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 | MMC_CAP_UHS_SDR50;
#endif
#ifdef CONFIG_SD_PM
	host->pm_caps = MMC_PM_KEEP_POWER | MMC_PM_WAKE_SDIO_IRQ;
#endif

#ifdef __CONFIG_ARCH_APP_CORE
	HAL_BoardIoctl(HAL_BIR_GET_CFG, HAL_MKDEV(HAL_DEV_MAJOR_SDC, host->sdc_id),
	               (uint32_t)&sd_gpio_cfg);
	if (!sd_gpio_cfg)
		host->caps |= MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA;
	else if (sd_gpio_cfg->data_bits == 8)
		host->caps |= MMC_CAP_8_BIT_DATA | MMC_CAP_4_BIT_DATA;
	else if (sd_gpio_cfg->data_bits == 4)
		host->caps |= MMC_CAP_4_BIT_DATA;
#else
	host->caps |= MMC_CAP_4_BIT_DATA;
#endif

	__mci_restore_io(host);

	/* register IRQ */
	if (host->sdc_id == 0) {
		SDC_REQUEST_IRQ(SDC0_IRQn, (SDC_IRQHandler)&SDC0_IRQHandler);
		SDC_SetPriority(SDC0_IRQn, NVIC_PERIPH_PRIO_DEFAULT);
#if SDC1_SUPPORT
	} else if (host->sdc_id == 1) {
		SDC_REQUEST_IRQ(SDC1_IRQn, (SDC_IRQHandler)&SDC1_IRQHandler);
		SDC_SetPriority(SDC1_IRQn, NVIC_PERIPH_PRIO_DEFAULT);
#endif
	}

#ifdef CONFIG_DETECT_CARD
	if (host->param.cd_mode == CARD_ALWAYS_PRESENT) {
		host->present = 1;
	} else if (host->param.cd_mode == CARD_DETECT_BY_GPIO_IRQ) {
		if (!sd_gpio_cfg->has_detect_gpio) {
			SDC_LOGE("%s,%d cd_mode:%d with no detect_gpio!\n",
			         __func__, __LINE__, host->param.cd_mode);
			return NULL;
		}

		host->cd_port = sd_gpio_cfg->detect_port;
		host->cd_pin = sd_gpio_cfg->detect_pin;
		host->cd_pin_present_val = sd_gpio_cfg->detect_pin_present_val;
		HAL_BoardIoctl(HAL_BIR_PINMUX_INIT, HAL_MKDEV(HAL_DEV_MAJOR_SDC,
		               host->sdc_id), SDCGPIO_DET);

		host->cd_delay = sd_gpio_cfg->detect_delay;

		SDC_InitTimer(&host->cd_timer, &__mci_cd_timer, host, 300);

		//HAL_GPIO_SetDebounce(&Irq_param, (2U << 4) | 1); /* set debounce clock */
		__mci_enable_cd_pin_irq(host);
		host->present = (host->cd_pin_present_val ==
		                 HAL_GPIO_ReadPin(host->cd_port, host->cd_pin)) ? 1 : 0;
	} else if (host->param.cd_mode == CARD_DETECT_BY_D3) {
		uint32_t rval;

		host->cd_delay = sd_gpio_cfg->detect_delay;

		__mci_clk_prepare_enable(host);
		mmc_mdelay(1);
		host->present = 1;
		rval = mci_readl(host, SDXC_REG_RINTR);
		SDC_LOGD("sdc +> REG_RINTR=0x%x\n", rval);
		if ((rval & SDXC_CardRemove)) {
			SDC_LOGD("sdc data[3] detect Card Remove\n");
			host->present = 0;
		}
		SDC_InitTimer(&host->cd_timer, &__mci_dat3_det, host, 300);
	}
#else
	host->present = 1;
#endif

	host->max_blk_count = 8192;
	host->max_blk_size = 4096;
	host->max_req_size = host->max_blk_size * host->max_blk_count;
	host->max_seg_size = host->max_req_size;
	host->max_segs = 128;
	host->ocr_avail = MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32
	                  | MMC_VDD_32_33 | MMC_VDD_33_34;

	SDC_LOGN("SDC Host Capability:0x%x Ocr avail:0x%x\n", host->caps, host->ocr_avail);

	/* init semaphore */
	SDC_SemCreate(&host->lock, 0);
	SDC_MutexCreate(&host->thread_lock);
#ifdef CONFIG_SDC_EXCLUSIVE_HOST
	SDC_SemCreate(&host->exclusive_lock, 1);
#endif

#ifdef CONFIG_DETECT_CARD
	if (host->param.cd_mode == CARD_DETECT_BY_D3 && host->present == 0) {
		SDC_LOGD("SDC power init.\n");
		HAL_SDC_PowerOn(host);
	} else if (host->present == 0) {
		/* if card is not present and the card detect mode is not CARD_DETECT_BY_D3,
		 * we shutdown io voltage to save power. */
		SDC_LOGD("SDC no card detected, shutdown io voltage.\n");
		__mci_hold_io(host);
		//__mci_set_vddio(host, SDC_VOLTAGE_OFF);
	}
#endif

#ifdef CONFIG_SD_PM
	SDC_DEV(host->sdc_id)->platform_data = host;
	pm_register_ops(SDC_DEV(host->sdc_id));
#endif

	host->State = SDC_STATE_READY;
	host->wait = SDC_WAIT_NONE;

#ifdef CONFIG_DETECT_CARD
	if (host->param.cd_mode != CARD_ALWAYS_PRESENT && host->present) {
		host->wait_voltage_stable = 1;
		if (host->cd_delay == 0) {
			SDC_ModTimer(&host->cd_timer, 10);
		} else {
			SDC_ModTimer(&host->cd_timer, host->cd_delay);
		}
	}
	SDC_LOGN("SDC cd_mode:%d present_val:%d\n", host->param.cd_mode,
	         host->cd_pin_present_val);
#endif
	SDC_LOGN("SDC id:%d dma_use:%d present:%d init ok.\n", host->sdc_id,
	         host->dma_use, host->present);

	return host;
}

static void HAL_SDC_Calibrate(struct mmc_host *host)
{
	uint32_t time = 0;

	HAL_ASSERT_PARAM(host);

	if (host->sdc_id == 0)
		host->caps &= ~(MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED);

	//mci_writel(mci_readl(host, SDXC_REG_NTSR) & (~(1 << 31)), host, SDXC_REG_NTSR);
	mci_writel(mci_readl(host, SDXC_REG_DELAY_CTRL) & (~(1 << 7)), host, SDXC_REG_DELAY_CTRL);  //disable sample delay
	mci_writel(mci_readl(host, SDXC_REG_DELAY_CTRL) | (1 << 15), host, SDXC_REG_DELAY_CTRL);     //enanle sample delay calibration

	while (!(mci_readl(host, SDXC_REG_DELAY_CTRL) & (1 << 14))) {
		time++;
		if (time == 1024)
			break;
	}
	mci_writel(mci_readl(host, SDXC_REG_DELAY_CTRL) | 0x4, host, SDXC_REG_DELAY_CTRL);
	mci_writel(mci_readl(host, SDXC_REG_DELAY_CTRL) | (1 << 7), host, SDXC_REG_DELAY_CTRL);  //enable sample delay

#ifdef __CONFIG_ROM
	host->clk = 400000;
#endif
}

#ifdef __CONFIG_ROM
SDC_Semaphore _sdc_lock[SDC_NUM];

int32_t HAL_SDC_Claim_Host(struct mmc_host *host)
{
	HAL_ASSERT_PARAM(host->sdc_id < SDC_NUM);

	return (OS_SemaphoreWait(&_sdc_lock[host->sdc_id], OS_WAIT_FOREVER) == OS_OK ? 0 : -1);
}

void HAL_SDC_Release_Host(struct mmc_host *host)
{
	HAL_ASSERT_PARAM(host->sdc_id < SDC_NUM);
	OS_SemaphoreRelease(&_sdc_lock[host->sdc_id]);
}

struct mmc_host *HAL_SDC_Init(struct mmc_host *host)
{
	HAL_ASSERT_PARAM(host->sdc_id < SDC_NUM);

	OS_SemaphoreCreate(&_sdc_lock[host->sdc_id], 1, 1);

	_HAL_SDC_Init(host);
	HAL_SDC_Calibrate(host);

	return host;
}

int32_t HAL_SDC_Deinit(uint32_t sdc_id)
{
	struct mmc_host *host = _mci_host[sdc_id];

	if (_HAL_SDC_Deinit(sdc_id)) {
		return -1;
	}
	_mci_host[sdc_id] = host;
	host->sdc_id = sdc_id;

	OS_SemaphoreDelete(&_sdc_lock[sdc_id]);

	return 0;
}

extern int32_t __HAL_SDC_Update_Clk(struct mmc_host *host, uint32_t clk);

int32_t HAL_SDC_Update_Clk(struct mmc_host *host, uint32_t clk)
{
	int32_t ret = __HAL_SDC_Update_Clk(host, clk);

	mci_writel(mci_readl(host, SDXC_REG_DELAY_CTRL0) | (1 << 17), host, SDXC_REG_DELAY_CTRL0);

	return ret;
}
#endif
