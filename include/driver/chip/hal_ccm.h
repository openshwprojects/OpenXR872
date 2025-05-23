/**
 * @file  hal_ccm.h
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

#ifndef _DRIVER_CHIP_HAL_CCM_H_
#define _DRIVER_CHIP_HAL_CCM_H_

#include "driver/chip/hal_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CCM (Clock Control Module) register block structure
 */
typedef struct {
	__IO uint32_t CPU_BUS_CLKCFG;       /* offset: 0x00, CPU bus clock configure register */
	__IO uint32_t BUS_PERIPH_CLK_CTRL;  /* offset: 0x04, Bus device clock gating control Register */
	__IO uint32_t BUS_PERIPH_RST_CTRL;  /* offset: 0x08, Bus device reset control register */
#if (__CONFIG_CHIP_ARCH_VER == 1)
         uint32_t RESERVED0[5];
#elif (__CONFIG_CHIP_ARCH_VER == 2)
	     uint32_t RESERVED0[4];
	__IO uint32_t SDC1_MCLK_CTRL;       /* offset: 0x1C, SDC1 clock control register */
#endif
	__IO uint32_t SPI0_MCLK_CTRL;       /* offset: 0x20, SPI0 clock control register */
	__IO uint32_t SPI1_MCLK_CTRL;       /* offset: 0x24, SPI1 clock control register */
	__IO uint32_t SDC0_MCLK_CTRL;       /* offset: 0x28, SDC0 clock control register */
	__IO uint32_t CE_MCLK_CTRL;         /* offset: 0x2C, CE clock control register */
	__IO uint32_t CSI_JPE_DEV_CLK_CTRL; /* offset: 0x30, CSI JPEG module clock control register */
	__IO uint32_t DAUDIO_MCLK_CTRL;     /* offset: 0x34, Digital audio clock control register */
	__IO uint32_t IRRX_MCLK_CTRL;       /* offset: 0x38, IRRX clock control register */
	__IO uint32_t IRTX_MCLK_CTRL;       /* offset: 0x3C, IRTX clock control register */
	__IO uint32_t SYSTICK_REFCLK_CTRL;  /* offset: 0x40, System tick reference clock register */
	__IO uint32_t SYSTICK_CALIB_CTRL;   /* offset: 0x44, System tick clock calibration register */
	__IO uint32_t DMIC_MCLK_CTRL;       /* offset: 0x48, DMIC clock control register */
	__IO uint32_t GPADC_MCLK_CTRL;      /* offset: 0x4C, GPADC clock control register */
	__IO uint32_t CSI_MCLK_CTRL;        /* offset: 0x50, CSI output clock control register */
	__IO uint32_t FLASHC_MCLK_CTRL;     /* offset: 0x54, Flash controller clock control register */
#if (__CONFIG_CHIP_ARCH_VER == 2)
	__IO uint32_t AUDIO_CODEC_MCLK_CTRL;/* offset: 0x58, AUDIO module clock control register */
	__IO uint32_t APBS_CLKCFG;          /* offset: 0x5C, Special APB clock configure register (UART/PRCM/RTC APB bus clock) */
	__IO uint32_t PSRAMC_MCLK_CTRL;     /* offset: 0x60, PSRAM controller clock control register */
#endif
} CCM_T;

#define CCM ((CCM_T *)CCM_BASE)         /* address: 0x40040400 */

/*
 * Bit field definition of CCM->CPU_BUS_CLKCFG
 */
#define CCM_AHB2_CLK_DIV_SHIFT  8   /* R/W */
#define CCM_AHB2_CLK_DIV_VMASK  0x3U
#define CCM_AHB2_CLK_DIV_MASK   (CCM_AHB2_CLK_DIV_VMASK << CCM_AHB2_CLK_DIV_SHIFT)
typedef enum {
	CCM_AHB2_CLK_DIV_1          = (0x0U << CCM_AHB2_CLK_DIV_SHIFT),
	CCM_AHB2_CLK_DIV_2          = (0x1U << CCM_AHB2_CLK_DIV_SHIFT),
	CCM_AHB2_CLK_DIV_3          = (0x2U << CCM_AHB2_CLK_DIV_SHIFT),
	CCM_AHB2_CLK_DIV_4          = (0x3U << CCM_AHB2_CLK_DIV_SHIFT)
} CCM_AHB2ClkDiv;

#define CCM_APB_CLK_SRC_SHIFT   4   /* R/W */
#define CCM_APB_CLK_SRC_VMASK   0x3U
#define CCM_APB_CLK_SRC_MASK    (CCM_APB_CLK_SRC_VMASK << CCM_APB_CLK_SRC_SHIFT)
typedef enum {
	CCM_APB_CLK_SRC_HFCLK       = (0x0U << CCM_APB_CLK_SRC_SHIFT),
	CCM_APB_CLK_SRC_LFCLK       = (0x1U << CCM_APB_CLK_SRC_SHIFT),
	CCM_APB_CLK_SRC_AHB2CLK     = (0x2U << CCM_APB_CLK_SRC_SHIFT)
} CCM_APBClkSrc;

#define CCM_APB_CLK_DIV_SHIFT   0   /* R/W */
#define CCM_APB_CLK_DIV_VMASK   0x3U
#define CCM_APB_CLK_DIV_MASK    (CCM_APB_CLK_DIV_VMASK << CCM_APB_CLK_DIV_SHIFT)
typedef enum {
	CCM_APB_CLK_DIV_1           = (0x0U << CCM_APB_CLK_DIV_SHIFT),
	CCM_APB_CLK_DIV_2           = (0x1U << CCM_APB_CLK_DIV_SHIFT),
	CCM_APB_CLK_DIV_4           = (0x2U << CCM_APB_CLK_DIV_SHIFT),
	CCM_APB_CLK_DIV_8           = (0x3U << CCM_APB_CLK_DIV_SHIFT)
} CCM_APBClkDiv;

/*
 * Bit field definition of
 *     - CCM->BUS_PERIPH_CLK_CTRL
 *     - CCM->BUS_PERIPH_RST_CTRL
 */
typedef enum {
#if (__CONFIG_CHIP_ARCH_VER == 2)
	CCM_BUS_PERIPH_BIT_TRNG         = HAL_BIT(30),  /* R/W */
	CCM_BUS_PERIPH_BIT_AUDIO_CODEC  = HAL_BIT(29),  /* R/W */
	CCM_BUS_PERIPH_BIT_UART2        = HAL_BIT(28),  /* R/W */
#endif
	CCM_BUS_PERIPH_BIT_GPIO         = HAL_BIT(27),  /* R/W, only valid for clock, not for reset */
	CCM_BUS_PERIPH_BIT_DMIC         = HAL_BIT(26),  /* R/W */
	CCM_BUS_PERIPH_BIT_GPADC        = HAL_BIT(25),  /* R/W */
	CCM_BUS_PERIPH_BIT_IRRX         = HAL_BIT(24),  /* R/W */
	CCM_BUS_PERIPH_BIT_IRTX         = HAL_BIT(23),  /* R/W */
	CCM_BUS_PERIPH_BIT_DAUDIO       = HAL_BIT(22),  /* R/W */
	CCM_BUS_PERIPH_BIT_PWM          = HAL_BIT(21),  /* R/W */
	CCM_BUS_PERIPH_BIT_I2C1         = HAL_BIT(19),  /* R/W */
	CCM_BUS_PERIPH_BIT_I2C0         = HAL_BIT(18),  /* R/W */
	CCM_BUS_PERIPH_BIT_UART1        = HAL_BIT(17),  /* R/W */
	CCM_BUS_PERIPH_BIT_UART0        = HAL_BIT(16),  /* R/W */
#if (__CONFIG_CHIP_ARCH_VER == 2)
	CCM_BUS_PERIPH_BIT_DCACHE       = HAL_BIT(13),  /* R/W, only valid for reset */
	CCM_BUS_PERIPH_BIT_ICACHE       = HAL_BIT(12),  /* R/W */
	CCM_BUS_PERIPH_BIT_PSRAM_CTRL   = HAL_BIT(11),  /* R/W */
	CCM_BUS_PERIPH_BIT_WLANC        = HAL_BIT(10),  /* R/W */
	CCM_BUS_PERIPH_BIT_SDC1         = HAL_BIT(9),   /* R/W */
#endif
	CCM_BUS_PERIPH_BIT_MSGBOX       = HAL_BIT(8),   /* R/W */
	CCM_BUS_PERIPH_BIT_SPINLOCK     = HAL_BIT(7),   /* R/W */
	CCM_BUS_PERIPH_BIT_DMA          = HAL_BIT(6),   /* R/W */
	CCM_BUS_PERIPH_BIT_CSI_JPEG     = HAL_BIT(5),   /* R/W */
	CCM_BUS_PERIPH_BIT_SDC0         = HAL_BIT(4),   /* R/W */
	CCM_BUS_PERIPH_BIT_FLASH_CTRL   = HAL_BIT(3),   /* R/W */
	CCM_BUS_PERIPH_BIT_CE           = HAL_BIT(2),   /* R/W */
	CCM_BUS_PERIPH_BIT_SPI1         = HAL_BIT(1),   /* R/W */
	CCM_BUS_PERIPH_BIT_SPI0         = HAL_BIT(0)    /* R/W */
} CCM_BusPeriphBit;

/*
 * Bit field definition of CLK enable
 *     - SPI0/1, SDC0/1, CE, DAUDIO, IRRX, IRTX, SYSTICK, DMIC, GPADC, CSI,
 *       FLASHC, AUDIO_CODEC, PSRAMC
 */
#define CCM_PERIPH_CLK_EN_BIT       HAL_BIT(31) /* R/W */

/*
 * Bit field definition of CLK source
 *     - AHB peripheral: SPI0/1, SDC0/1, CE, CSI, FLASHC, PSRAMC
 *     - APB peripheral: IRRX, IRTX, SYSTICK, GPADC
 */
#define CCM_PERIPH_CLK_SRC_SHIFT    24  /* R/W */
#define CCM_PERIPH_CLK_SRC_MASK     (0x3U << CCM_PERIPH_CLK_SRC_SHIFT)

typedef enum {
	CCM_AHB_PERIPH_CLK_SRC_HFCLK    = (0x0U << CCM_PERIPH_CLK_SRC_SHIFT),
	CCM_AHB_PERIPH_CLK_SRC_DEVCLK   = (0x1U << CCM_PERIPH_CLK_SRC_SHIFT),
} CCM_AHBPeriphClkSrc;

typedef enum {
	CCM_APB_PERIPH_CLK_SRC_HFCLK    = (0x0U << CCM_PERIPH_CLK_SRC_SHIFT),
	CCM_APB_PERIPH_CLK_SRC_LFCLK    = (0x1U << CCM_PERIPH_CLK_SRC_SHIFT)
} CCM_APBPeriphClkSrc;

/*
 * Bit field definition of CLK divider N, M
 *     - SPI0/1, SDC0/1, CE, IRRX, IRTX, SYSTICK, GPADC, CSI, FLASHC, PSRAMC
 */
#define CCM_PERIPH_CLK_DIV_N_SHIFT  16  /* R/W */
#define CCM_PERIPH_CLK_DIV_N_MASK   (0x3U << CCM_PERIPH_CLK_DIV_N_SHIFT)
typedef enum {
	CCM_PERIPH_CLK_DIV_N_1          = (0U << CCM_PERIPH_CLK_DIV_N_SHIFT),
	CCM_PERIPH_CLK_DIV_N_2          = (1U << CCM_PERIPH_CLK_DIV_N_SHIFT),
	CCM_PERIPH_CLK_DIV_N_4          = (2U << CCM_PERIPH_CLK_DIV_N_SHIFT),
	CCM_PERIPH_CLK_DIV_N_8          = (3U << CCM_PERIPH_CLK_DIV_N_SHIFT)
} CCM_PeriphClkDivN;

#define CCM_PERIPH_CLK_DIV_M_SHIFT  0   /* R/W */
#define CCM_PERIPH_CLK_DIV_M_MASK   (0xFU << CCM_PERIPH_CLK_DIV_M_SHIFT)
typedef enum {
	CCM_PERIPH_CLK_DIV_M_1          = (0U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_2          = (1U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_3          = (2U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_4          = (3U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_5          = (4U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_6          = (5U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_7          = (6U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_8          = (7U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_9          = (8U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_10         = (9U  << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_11         = (10U << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_12         = (11U << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_13         = (12U << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_14         = (13U << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_15         = (14U << CCM_PERIPH_CLK_DIV_M_SHIFT),
	CCM_PERIPH_CLK_DIV_M_16         = (15U << CCM_PERIPH_CLK_DIV_M_SHIFT)
} CCM_PeriphClkDivM;

#define CCM_PERIPH_CLK_PARAM_MASK   (CCM_PERIPH_CLK_SRC_MASK |   \
                                     CCM_PERIPH_CLK_DIV_N_MASK | \
                                     CCM_PERIPH_CLK_DIV_M_MASK)

/* CCM->SDC1_MCLK_CTRL */

/* CCM->SPI0_MCLK_CTRL */

/* CCM->SPI1_MCLK_CTRL */

/* CCM->SDC0_MCLK_CTRL */

/* CCM->CE_MCLK_CTRL */

/* CCM->CSI_OCLK_CTRL */

/*
 * Bit field definition of CCM->DAUDIO_MCLK_CTRL
 */
#define CCM_DAUDIO_MCLK_SRC_SHIFT   0   /* R/W */
#define CCM_DAUDIO_MCLK_SRC_MASK    (0x3U << CCM_DAUDIO_MCLK_SRC_SHIFT)
typedef enum {
	CCM_DAUDIO_MCLK_SRC_8X          = (0x0U << CCM_DAUDIO_MCLK_SRC_SHIFT),
	CCM_DAUDIO_MCLK_SRC_4X          = (0x1U << CCM_DAUDIO_MCLK_SRC_SHIFT),
	CCM_DAUDIO_MCLK_SRC_2X          = (0x2U << CCM_DAUDIO_MCLK_SRC_SHIFT),
	CCM_DAUDIO_MCLK_SRC_1X          = (0x3U << CCM_DAUDIO_MCLK_SRC_SHIFT)
} CCM_DAudioClkSrc;

/* CCM->IRRX_MCLK_CTRL */

/* CCM->IRTX_MCLK_CTRL */

/* CCM->SYSTICK_REFCLK_CTRL */

/*
 * Bit field definition of CCM->SYSTICK_CALIB_CTRL
 */
#define CCM_SYSTICK_NOREF_BIT       HAL_BIT(25) /* R/W */
#define CCM_SYSTICK_SKEW_BIT        HAL_BIT(24) /* R/W */
#define CCM_SYSTICK_TENMS_SHIFT     0           /* R/W */
#define CCM_SYSTICK_TENMS_MASK      (0xFFFFFFU << CCM_SYSTICK_TENMS_SHIFT)

/* CCM->DMIC_MCLK_CTRL */

/* CCM->GPADC_MCLK_CTRL */

/* CCM->CSI_MCLK_CTRL */

/* CCM->FLASHC_MCLK_CTRL */

#if (__CONFIG_CHIP_ARCH_VER == 2)
/* CCM->AUDIO_CODEC_MCLK_CTRL */

/*
 * Bit field definition of CCM->APBS_CLKCFG
 */
#define CCM_APBS_CLK_SRC_SHIFT      24  /* R/W */
#define CCM_APBS_CLK_SRC_VMASK      0x3U
#define CCM_APBS_CLK_SRC_MASK       (CCM_APBS_CLK_SRC_VMASK << CCM_APBS_CLK_SRC_SHIFT)
typedef enum {
	CCM_APBS_CLK_SRC_HFCLK          = (0x0U << CCM_APBS_CLK_SRC_SHIFT),
	CCM_APBS_CLK_SRC_DEVCLK         = (0x1U << CCM_APBS_CLK_SRC_SHIFT),
	CCM_APBS_CLK_SRC_LFCLK          = (0x2U << CCM_APBS_CLK_SRC_SHIFT)
} CCM_APBSClkSrc;

#define CCM_APBS_CLK_DIV_N_SHIFT    16  /* R/W */
#define CCM_APBS_CLK_DIV_N_VMASK    0x3U
#define CCM_APBS_CLK_DIV_N_MASK     (CCM_APBS_CLK_DIV_N_VMASK << CCM_APBS_CLK_DIV_N_SHIFT)
typedef enum {
	CCM_APBS_CLK_DIV_N_1            = (0x0U << CCM_APBS_CLK_DIV_N_SHIFT),
	CCM_APBS_CLK_DIV_N_2            = (0x1U << CCM_APBS_CLK_DIV_N_SHIFT),
	CCM_APBS_CLK_DIV_N_4            = (0x2U << CCM_APBS_CLK_DIV_N_SHIFT),
	CCM_APBS_CLK_DIV_N_8            = (0x3U << CCM_APBS_CLK_DIV_N_SHIFT)
} CCM_APBSClkDivN;

#define CCM_APBS_CLK_DIV_M_SHIFT    0   /* R/W */
#define CCM_APBS_CLK_DIV_M_VMASK    0xFU
#define CCM_APBS_CLK_DIV_M_MASK     (CCM_APBS_CLK_DIV_M_VMASK << CCM_APBS_CLK_DIV_M_SHIFT)
typedef enum {
	CCM_APBS_CLK_DIV_M_1            = (0U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_2            = (1U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_3            = (2U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_4            = (3U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_5            = (4U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_6            = (5U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_7            = (6U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_8            = (7U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_9            = (8U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_10           = (9U  << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_11           = (10U << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_12           = (11U << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_13           = (12U << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_14           = (13U << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_15           = (14U << CCM_APBS_CLK_DIV_M_SHIFT),
	CCM_APBS_CLK_DIV_M_16           = (15U << CCM_APBS_CLK_DIV_M_SHIFT)
} CCM_APBSClkDivM;

#define CCM_APBS_CLK_FACTOR(n, m)    ((n) | (m))

#define CCM_APBS_CLK_DEV_FACTOR_MASK    (CCM_APBS_CLK_DIV_N_MASK | CCM_APBS_CLK_DIV_M_MASK)
typedef enum {
	CCM_APBS_CLK_DEV_FACTOR_48M  = CCM_APBS_CLK_FACTOR(CCM_APBS_CLK_DIV_N_1, CCM_APBS_CLK_DIV_M_4),/*dev clock = 192Mhz*/
	CCM_APBS_CLK_DEV_FACTOR_24M  = CCM_APBS_CLK_FACTOR(CCM_APBS_CLK_DIV_N_1, CCM_APBS_CLK_DIV_M_8),
	CCM_APBS_CLK_DEV_FACTOR_15M  = CCM_APBS_CLK_FACTOR(CCM_APBS_CLK_DIV_N_1, CCM_APBS_CLK_DIV_M_13), /* 14.769MHz */
} CCM_APBSClkDEVFactor;

#define CCM_APBS_CLK_HF_FACTOR_MASK    (CCM_APBS_CLK_DIV_N_MASK | CCM_APBS_CLK_DIV_M_MASK)
typedef enum {
	CCM_APBS_CLK_HF_FACTOR_24M  = CCM_APBS_CLK_FACTOR(CCM_APBS_CLK_DIV_N_1, CCM_APBS_CLK_DIV_M_1),
} CCM_APBSClkHFFactor;

/* CCM->PSRAMC_MCLK_CTRL */

#endif /* __CONFIG_CHIP_ARCH_VER */

/******************************************************************************/

/**
 * @brief Configure AHB2 and APB clock
 * @param[in] AHB2Div AHB2 clock divider
 * @param[in] APBSrc APB clock source
 * @param[in] APBDiv APB clock divider
 * @return None
 */
void HAL_CCM_BusSetClock(CCM_AHB2ClkDiv AHB2Div, CCM_APBClkSrc APBSrc, CCM_APBClkDiv APBDiv);

/**
 * @brief Get AHB1 clock
 * @return AHB1 clock in Hz
 */
uint32_t HAL_CCM_BusGetAHB1Clock(void);

/**
 * @brief Get AHB2 clock
 * @return AHB2 clock in Hz
 */
uint32_t HAL_CCM_BusGetAHB2Clock(void);

/**
 * @brief Get APB clock
 * @return APB clock in Hz
 */
uint32_t HAL_CCM_BusGetAPBClock(void);

/**
 * @brief Enable peripheral clock
 * @param[in] periphMask Bitmask of peripherals, refer to CCM_BusPeriphBit
 * @return None
 */
void HAL_CCM_BusEnablePeriphClock(uint32_t periphMask);

/**
 * @brief Disable peripheral clock
 * @param[in] periphMask Bitmask of peripherals, refer to CCM_BusPeriphBit
 * @return None
 */
void HAL_CCM_BusDisablePeriphClock(uint32_t periphMask);

/**
 * @brief Disable all peripherals clock
 * @return None
 */
void HAL_CCM_BusDisableAllPeriphClock(void);

/**
 * @brief Force peripheral reset
 * @param[in] periphMask Bitmask of peripherals, refer to CCM_BusPeriphBit
 * @return None
 */
void HAL_CCM_BusForcePeriphReset(uint32_t periphMask);

/**
 * @brief Release peripheral reset
 * @param[in] periphMask Bitmask of peripherals, refer to CCM_BusPeriphBit
 * @return None
 */
void HAL_CCM_BusReleasePeriphReset(uint32_t periphMask);

/**
 * @brief Force all peripherals reset
 * @return None
 */
void HAL_CCM_BusForceAllPeriphReset(void);

/**
 * @brief Configure SPI0 module clock
 * @param[in] src SPI0 module clock source
 * @param[in] divN SPI0 module clock divider N
 * @param[in] divM SPI0 module clock divider M
 * @return None
 */
void HAL_CCM_SPI0_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable SPI0 module clock
 * @return None
 */
void HAL_CCM_SPI0_EnableMClock(void);

/**
 * @brief Disable SPI0 module clock
 * @return None
 */
void HAL_CCM_SPI0_DisableMClock(void);

/**
 * @brief Configure SPI1 module clock
 * @param[in] src SPI1 module clock source
 * @param[in] divN SPI1 module clock divider N
 * @param[in] divM SPI1 module clock divider M
 * @return None
 */
void HAL_CCM_SPI1_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable SPI1 module clock
 * @return None
 */
void HAL_CCM_SPI1_EnableMClock(void);

/**
 * @brief Disable SPI1 module clock
 * @return None
 */
void HAL_CCM_SPI1_DisableMClock(void);

/**
 * @brief Configure SD0 controller module clock
 * @param[in] src SD0 controller module clock source
 * @param[in] divN SD0 controller module clock divider N
 * @param[in] divM SD0 controller module clock divider M
 * @return None
 */
void HAL_CCM_SDC0_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable SD0 controller module clock
 * @return None
 */
void HAL_CCM_SDC0_EnableMClock(void);

/**
 * @brief Disable SD0 controller module clock
 * @return None
 */
void HAL_CCM_SDC0_DisableMClock(void);

/**
 * @brief Configure SD1 controller module clock
 * @param[in] src SD1 controller module clock source
 * @param[in] divN SD1 controller module clock divider N
 * @param[in] divM SD1 controller module clock divider M
 * @return None
 */
void HAL_CCM_SDC1_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable SD1 controller module clock
 * @return None
 */
void HAL_CCM_SDC1_EnableMClock(void);

/**
 * @brief Disable SD1 controller module clock
 * @return None
 */
void HAL_CCM_SDC1_DisableMClock(void);

/**
 * @brief Configure Crypto Engine module clock
 * @param[in] src Crypto Engine module clock source
 * @param[in] divN Crypto Engine module clock divider N
 * @param[in] divM Crypto Engine module clock divider M
 * @return None
 */
void HAL_CCM_CE_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable Crypto Engine module clock
 * @return None
 */
void HAL_CCM_CE_EnableMClock(void);

/**
 * @brief Disable Crypto Engine module clock
 * @return None
 */
void HAL_CCM_CE_DisableMClock(void);

/**
 * @brief Configure CSI JPEG module clock
 * @param[in] src CSI JPEG module clock source
 * @param[in] divN JPEG module output clock divider N
 * @param[in] divM JPEG module output clock divider M
 * @return None
 */
void HAL_CCM_CSI_JPEG_SetDevClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable CSI JPEG module clock
 * @return None
 */
void HAL_CCM_CSI_JPEG_EnableDevClock(void);

/**
 * @brief Disable CSI JPEG module clock
 * @return None
 */
void HAL_CCM_CSI_JPEG_DisableDevClock(void);

/**
 * @brief Configure Digital audio module clock
 * @param[in] src Digital audio module source
 * @return None
 */
void HAL_CCM_DAUDIO_SetMClock(CCM_DAudioClkSrc src);

/**
 * @brief Enable Digital audio module clock
 * @return None
 */
void HAL_CCM_DAUDIO_EnableMClock(void);

/**
 * @brief Disable Digital audio module clock
 * @return None
 */
void HAL_CCM_DAUDIO_DisableMClock(void);

/**
 * @brief Configure IR RX module clock
 * @param[in] src IR RX module clock source
 * @param[in] divN IR RX module clock divider N
 * @param[in] divM IR RX module clock divider M
 * @return None
 */
void HAL_CCM_IRRX_SetMClock(CCM_APBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable IR RX module clock
 * @return None
 */
void HAL_CCM_IRRX_EnableMClock(void);

/**
 * @brief Disable IR RX module clock
 * @return None
 */
void HAL_CCM_IRRX_DisableMClock(void);

/**
 * @brief Configure IR TX module clock
 * @param[in] src IR TX module clock source
 * @param[in] divN IR TX module clock divider N
 * @param[in] divM IR TX module clock divider M
 * @return None
 */
void HAL_CCM_IRTX_SetMClock(CCM_APBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable IR TX module clock
 * @return None
 */
void HAL_CCM_IRTX_EnableMClock(void);

/**
 * @brief Disable IR TX module clock
 * @return None
 */
void HAL_CCM_IRTX_DisableMClock(void);

/**
 * @brief Configure system tick reference clock
 * @param[in] src system tick reference clock source
 * @param[in] divN system tick reference clock divider N
 * @param[in] divM system tick reference clock divider M
 * @return None
 */
void HAL_CCM_SYSTICK_SetRefClock(CCM_APBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable system tick reference clock
 * @return None
 */
void HAL_CCM_SYSTICK_EnableRefClock(void);

/**
 * @brief Disable system tick reference clock
 * @return None
 */
void HAL_CCM_SYSTICK_DisableRefClock(void);

/**
 * @brief Enable system tick reference
 * @return None
 */
void HAL_CCM_SYSTICK_EnableRef(void);

/**
 * @brief Disable system tick reference
 * @return None
 */
void HAL_CCM_SYSTICK_DisableRef(void);

/**
 * @brief Enable system tick skew, which means calibration value is not exactly 10 ms
 * @return None
 */
void HAL_CCM_SYSTICK_EnableSkew(void);

/**
 * @brief Disable system tick skew, which means calibration value is accurate
 * @return None
 */
void HAL_CCM_SYSTICK_DisableSkew(void);

/**
 * @brief Set calibration value for 10 ms
 * @return None
 */
void HAL_CCM_SYSTICK_SetTENMS(uint32_t cnt);

/**
 * @brief Enable DMIC module clock
 * @return None
 */
void HAL_CCM_DMIC_EnableMClock(void);

/**
 * @brief Disable DMIC module clock
 * @return None
 */
void HAL_CCM_DMIC_DisableMClock(void);

/**
 * @brief Configure GPADC module clock
 * @param[in] src GPADC module clock source
 * @param[in] divN GPADC module clock divider N
 * @param[in] divM GPADC module clock divider M
 * @return None
 */
void HAL_CCM_GPADC_SetMClock(CCM_APBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable GPADC module clock
 * @return None
 */
void HAL_CCM_GPADC_EnableMClock(void);

/**
 * @brief Disable GPADC module clock
 * @return None
 */
void HAL_CCM_GPADC_DisableMClock(void);

/**
 * @brief Configure CSI output clock
 * @param[in] src CSI output clock source
 * @param[in] divN CSI output clock divider N
 * @param[in] divM CSI output clock divider M
 * @return None
 */
void HAL_CCM_CSI_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable CSI output clock
 * @return None
 */
void HAL_CCM_CSI_EnableMClock(void);

/**
 * @brief Disable CSI output clock
 * @return None
 */
void HAL_CCM_CSI_DisableMClock(void);

/**
 * @brief Configure Flash controller module clock
 * @param[in] src Flash controller module clock source
 * @param[in] divN Flash controller module clock divider N
 * @param[in] divM Flash controller module clock divider M
 * @return None
 */
void HAL_CCM_FLASHC_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable Flash controller module clock
 * @return None
 */
void HAL_CCM_FLASHC_EnableMClock(void);

/**
 * @brief Disable Flash controller module clock
 * @return None
 */
void HAL_CCM_FLASHC_DisableMClock(void);

/**
 * @brief Initializes CCM module
 * @return None
 */
void HAL_CCM_Init(void);

#if (__CONFIG_CHIP_ARCH_VER == 2)
/**
 * @brief Enable Audio Codec module clock
 * @return None
 */
void HAL_CCM_AudioCodec_EnableMClock(void);

/**
 * @brief Disable Audio Codec module clock
 * @return None
 */
void HAL_CCM_AudioCodec_DisableMClock(void);

/**
 * @brief Configure APB-S clock
 * @param[in] src APB-S clock source
 * @param[in] (divN | divM) APB-S clock divider
 * @return None
 */
void HAL_CCM_BusSetAPBSClock(CCM_APBSClkSrc src, uint32_t ccm_apbs_clk_div_factor);

uint32_t HAL_CCM_BusGetAPBSClock(void);

/**
 * @brief Configure PSRAM controller module clock
 * @param[in] src PSRAM controller module clock source
 * @param[in] divN PSRAM controller module clock divider N
 * @param[in] divM PSRAM controller module clock divider M
 * @return None
 */
void HAL_CCM_PSRAMC_SetMClock(CCM_AHBPeriphClkSrc src, CCM_PeriphClkDivN divN, CCM_PeriphClkDivM divM);

/**
 * @brief Enable Flash controller module clock
 * @return None
 */
void HAL_CCM_PSRAMC_EnableMClock(void);

/**
 * @brief Disable Flash controller module clock
 * @return None
 */
void HAL_CCM_PSRAMC_DisableMClock(void);
#endif /* __CONFIG_CHIP_ARCH_VER */

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_CHIP_HAL_CCM_H_ */
