/**
 * @file  hal_spi.h
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

#ifndef _DRIVER_CHIP_HAL_SPI_H_
#define _DRIVER_CHIP_HAL_SPI_H_

#include <stdbool.h>
#include "driver/chip/hal_def.h"
#include "driver/chip/hal_dma.h"
#include "driver/chip/hal_gpio.h"
#include "sys/xr_debug.h"

#ifdef __cplusplus
extern "C" {
#endif
/********************** private ************************************/

/**
  * @brief Serial Peripheral Interface
  */
typedef struct {
	__I  uint32_t VER;              /*!< SPI Version number Register,                 Address offset: 0x00   */
	__IO uint32_t CTRL;             /*!< SPI Global Control Register,                 Address offset: 0x04   */
	__IO uint32_t TCTRL;            /*!< SPI Transfer Control Register,               Address offset: 0x08   */
	     uint32_t RESERVED1[1];     /*!< Reserved,                                    Address offset: 0x0C   */
	__IO uint32_t IER;              /*!< SPI Interrupt Control Register,              Address offset: 0x10   */
	__IO uint32_t STA;              /*!< SPI Interrupt Status Register,               Address offset: 0x14   */
	__IO uint32_t FCTL;             /*!< SPI FIFO Control Register,                   Address offset: 0x18   */
	__I  uint32_t FST;              /*!< SPI FIFO Status Register,                    Address offset: 0x1C   */
	__IO uint32_t WAIT;             /*!< SPI Wait Clock Counter Register,             Address offset: 0x20   */
	__IO uint32_t CCTR;             /*!< SPI Clock Rate Control Register,             Address offset: 0x24   */
	     uint32_t RESERVED2[2];     /*!< Reserved,                                    Address offset: 0x28-0x2C */
	__IO uint32_t BC;               /*!< SPI Master mode Burst Counter Register,      Address offset: 0x30   */
	__IO uint32_t TC;               /*!< SPI Master mode Transmit Counter Register,   Address offset: 0x34   */
	__IO uint32_t BCC;              /*!< SPI Burst Control Register,                  Address offset: 0x38   */
	     uint32_t RESERVED3[19];    /*!< Reserved,                                    Address offset: 0x3C-0x84 */
	__IO uint32_t NDMA_MODE_CTRL;   /*!< SPI Nomal DMA Mode Control Regist            Address offset: 0x88     */
	     uint32_t RESERVED4[93];    /*!< Reserved,                                    Address offset: 0x8C-0x1FC */
	__IO uint32_t TXD;              /*!< SPI TX Date Register,                        Address offset: 0x200  */
	     uint32_t RESERVED5[63];    /*!< Reserved,                                    Address offset: 0x204-0x2FC */
	__I  uint32_t RXD;              /*!< SPI RX Date Register,                        Address offset: 0x300  */
} SPI_T;

/*
 * @brief SPI Global Control Register
 */
#define SPI_CTRL_RST_SHIFT                  (31)
#define SPI_CTRL_RST_MASK                   (0x1U << SPI_CTRL_RST_SHIFT)

#define SPI_CTRL_TP_EN_SHIFT                (7)
#define SPI_CTRL_TP_EN_MASK                 (0x1U << SPI_CTRL_TP_EN_SHIFT)

#define SPI_CTRL_MODE_SHIFT                 (1)
#define SPI_CTRL_MODE_MASK                  (0x1U << SPI_CTRL_MODE_SHIFT)
typedef enum {
	SPI_CTRL_MODE_SLAVE = 0 << SPI_CTRL_MODE_SHIFT,
	SPI_CTRL_MODE_MASTER = 1 << SPI_CTRL_MODE_SHIFT
} SPI_CTRL_Mode;

#define SPI_CTRL_EN_SHIFT                   (0)
#define SPI_CTRL_EN_MASK                    (0x1U << SPI_CTRL_EN_SHIFT)
typedef enum {
	SPI_CTRL_EN_DISABLE = 0 << SPI_CTRL_EN_SHIFT,
	SPI_CTRL_EN_ENABLE = 1 << SPI_CTRL_EN_SHIFT
} SPI_CTRL_En;


/*
 * @brief SPI Transfer Control Register
 */
#define SPI_TCTRL_XCH_SHIFT                 (31)
#define SPI_TCTRL_XCH_MASK                  (0x1U << SPI_TCTRL_XCH_SHIFT)
typedef enum {
	SPI_TCTRL_XCH_IDLE = 0 << SPI_TCTRL_XCH_SHIFT,
	SPI_TCTRL_XCH_START = 1 << SPI_TCTRL_XCH_SHIFT
} SPI_TCTRL_Xch;

#define SPI_TCTRL_SDDM_SHIFT                (14)
#define SPI_TCTRL_SDDM_MASK                 (0x0U << SPI_TCTRL_SDDM_SHIFT)
typedef enum {
	SPI_TCTRL_SDDM_SEND_NODELAY = 0 << SPI_TCTRL_SDDM_SHIFT,
	SPI_TCTRL_SDDM_SEND_DELAY = 1 << SPI_TCTRL_SDDM_SHIFT
} SPI_TCTRL_Sddm;

#define SPI_TCTRL_SDM_SHIFT                 (13)
#define SPI_TCTRL_SDM_MASK                  (0x1U << SPI_TCTRL_SDM_SHIFT)
typedef enum {
	SPI_TCTRL_SDM_SAMPLE_NODELAY = 1 << SPI_TCTRL_SDM_SHIFT,
	SPI_TCTRL_SDM_SAMPLE_DELAY = 0 << SPI_TCTRL_SDM_SHIFT
} SPI_TCTRL_Sdm;

#define SPI_TCTRL_FBS_SHIFT                 (12)
#define SPI_TCTRL_FBS_MASK                  (0x1U << SPI_TCTRL_FBS_SHIFT)
typedef enum {
	SPI_TCTRL_FBS_MSB = 0 << SPI_TCTRL_FBS_SHIFT,
	SPI_TCTRL_FBS_LSB = 1 << SPI_TCTRL_FBS_SHIFT
} SPI_TCTRL_Fbs;

#define SPI_TCTRL_SDC_SHIFT                 (11)
#define SPI_TCTRL_SDC_MASK                  (0x1U << SPI_TCTRL_SDC_SHIFT)

#define SPI_TCTRL_RPSM_SHIFT                (10)
#define SPI_TCTRL_RPSM_MASK                 (0x1U << SPI_TCTRL_RPSM_SHIFT)

#define SPI_TCTRL_DDB_SHIFT                 (9)
#define SPI_TCTRL_DDB_MASK                  (0x1U << SPI_TCTRL_DDB_SHIFT)

#define SPI_TCTRL_DHB_SHIFT                 (8)
#define SPI_TCTRL_DHB_MASK                  (0x1U << SPI_TCTRL_DHB_SHIFT)
typedef enum {
	SPI_TCTRL_DHB_FULL_DUPLEX = 0 << SPI_TCTRL_DHB_SHIFT,
	SPI_TCTRL_DHB_HALF_DUPLEX = 1 << SPI_TCTRL_DHB_SHIFT
} SPI_TCTRL_DHB_Duplex;

#define SPI_TCTRL_SS_LEVEL_SHIFT            (7)
#define SPI_TCTRL_SS_LEVEL_MASK             (0x1U << SPI_TCTRL_SS_LEVEL_SHIFT)

#define SPI_TCTRL_SS_OWNER_SHIFT            (6)
#define SPI_TCTRL_SS_OWNER_MASK             (0x1U << SPI_TCTRL_SS_OWNER_SHIFT)
typedef enum {
	SPI_TCTRL_SS_OWNER_CONTROLLER = 0 << SPI_TCTRL_SS_OWNER_SHIFT,
	SPI_TCTRL_SS_OWNER_SOFTWARE = 1 << SPI_TCTRL_SS_OWNER_SHIFT
} SPI_TCTRL_SS_OWNER;

#define SPI_TCTRL_SS_SEL_SHIFT              (4)
#define SPI_TCTRL_SS_SEL_MASK               (0x3U << SPI_TCTRL_SS_SEL_SHIFT)
typedef enum {
	SPI_TCTRL_SS_SEL_SS0 = 0 << SPI_TCTRL_SS_SEL_SHIFT,
	SPI_TCTRL_SS_SEL_SS1 = 1 << SPI_TCTRL_SS_SEL_SHIFT,
	SPI_TCTRL_SS_SEL_SS2 = 2 << SPI_TCTRL_SS_SEL_SHIFT,
	SPI_TCTRL_SS_SEL_SS3 = 3 << SPI_TCTRL_SS_SEL_SHIFT
} SPI_TCTRL_SS_Sel;

#define SPI_TCTRL_SS_CTL_SHIFT              (3)
#define SPI_TCTRL_SS_CTL_MASK               (0x1U << SPI_TCTRL_SS_CTL_SHIFT)

#define SPI_TCTRL_SPOL_SHIFT                (2)
#define SPI_TCTRL_SPOL_MASK                 (0x1U << SPI_TCTRL_SPOL_SHIFT)

#define SPI_TCTRL_CPOL_SHIFT                (1)
#define SPI_TCTRL_CPOL_MASK                 (0x1U << SPI_TCTRL_CPOL_SHIFT)
typedef enum {
	SPI_TCTRL_CPOL_HIGH = 0 << SPI_TCTRL_CPOL_SHIFT,
	SPI_TCTRL_CPOL_LOW = 1 << SPI_TCTRL_CPOL_SHIFT
} SPI_TCTRL_Cpol;

#define SPI_TCTRL_CPHA_SHIFT                (0)
#define SPI_TCTRL_CPHA_MASK                 (0x1U << SPI_TCTRL_CPHA_SHIFT)
typedef enum {
	SPI_TCTRL_CPHA_PHASE0  = 0 << SPI_TCTRL_CPHA_SHIFT,
	SPI_TCTRL_CPHA_PHASE1  = 1 << SPI_TCTRL_CPHA_SHIFT
} SPI_TCTRL_Cpha;

typedef enum {
	SPI_SCLK_Mode0 = 0 << SPI_TCTRL_CPHA_SHIFT,
	SPI_SCLK_Mode1 = 1 << SPI_TCTRL_CPHA_SHIFT,
	SPI_SCLK_Mode2 = 2 << SPI_TCTRL_CPHA_SHIFT,
	SPI_SCLK_Mode3 = 3 << SPI_TCTRL_CPHA_SHIFT
} SPI_SCLK_Mode;

/*
 * @brief SPI Interrupt Control Register
 */
#define SPI_IER_SS_INT_EN_SHIFT             (13)
#define SPI_IER_SS_INT_EN_MASK              (0x1U << SPI_IER_SS_INT_EN_SHIFT)

#define SPI_IER_TC_INT_EN_SHIFT             (12)
#define SPI_IER_TC_INT_EN_MASK              (0x1U << SPI_IER_TC_INT_EN_SHIFT)

#define SPI_IER_TF_UDR_INT_EN_SHIFT         (11)
#define SPI_IER_TF_UDR_INT_EN_MASK          (0x1U << SPI_IER_TF_UDR_INT_EN_SHIFT)

#define SPI_IER_TF_OVF_INT_EN_SHIFT         (10)
#define SPI_IER_TF_OVF_INT_EN_MASK          (0x1U << SPI_IER_TF_OVF_INT_EN_SHIFT)

#define SPI_IER_RF_UDR_INT_EN_SHIFT         (9)
#define SPI_IER_RF_UDR_INT_EN_MASK          (0x1U << SPI_IER_RF_UDR_INT_EN_SHIFT)

#define SPI_IER_RF_OVF_INT_EN_SHIFT         (8)
#define SPI_IER_RF_OVF_INT_EN_MASK          (0x1U << SPI_IER_RF_OVF_INT_EN_SHIFT)

#define SPI_IER_TF_FUL_INT_EN_SHIFT         (6)
#define SPI_IER_TF_FUL_INT_EN_MASK          (0x1U << SPI_IER_TF_FUL_INT_EN_SHIFT)

#define SPI_IER_TX_EMP_INT_EN_SHIFT         (5)
#define SPI_IER_TX_EMP_INT_EN_MASK          (0x1U << SPI_IER_TX_EMP_INT_EN_SHIFT)

#define SPI_IER_TX_ERQ_INT_EN_SHIFT         (4)
#define SPI_IER_TX_ERQ_INT_EN_MASK          (0x1U << SPI_IER_TX_ERQ_INT_EN_SHIFT)

#define SPI_IER_RF_FUL_INT_EN_SHIFT         (2)
#define SPI_IER_RF_FUL_INT_EN_MASK          (0x1U << SPI_IER_RF_FUL_INT_EN_SHIFT)

#define SPI_IER_RX_EMP_INT_EN_SHIFT         (1)
#define SPI_IER_RX_EMP_INT_EN_MASK          (0x1U << SPI_IER_RX_EMP_INT_EN_SHIFT)

#define SPI_IER_RF_RDY_INT_EN_SHIFT         (0)
#define SPI_IER_RF_RDY_INT_EN_MASK          (0x1U << SPI_IER_RF_RDY_INT_EN_SHIFT)

/*
 * @brief SPI Interrupt Status Register
 */
#define SPI_STA_SSI_SHIFT                   (13)
#define SPI_STA_SSI_MASK                    (0x1U << SPI_STA_SSI_SHIFT)

#define SPI_STA_TC_SHIFT                    (12)
#define SPI_STA_TC_MASK                     (0x1U << SPI_STA_TC_SHIFT)

#define SPI_STA_TF_UDF_SHIFT                (11)
#define SPI_STA_TF_UDF_MASK                 (0x1U << SPI_STA_TF_UDF_SHIFT)

#define SPI_STA_TF_OVF_SHIFT                (10)
#define SPI_STA_TF_OVF_MASK                 (0x1U << SPI_STA_TF_OVF_SHIFT)

#define SPI_STA_RX_UDF_SHIFT                (9)
#define SPI_STA_RX_UDF_MASK                 (0x1U << SPI_STA_RX_UDF_SHIFT)

#define SPI_STA_RX_OVF_SHIFT                (8)
#define SPI_STA_RX_OVF_MASK                 (0x1U << SPI_STA_RX_OVF_SHIFT)

#define SPI_STA_TX_FULL_SHIFT               (6)
#define SPI_STA_TX_FULL_MASK                (0x1U << SPI_STA_TX_FULL_SHIFT)

#define SPI_STA_TX_EMP_SHIFT                (5)
#define SPI_STA_TX_EMP_MASK                 (0x1U << SPI_STA_TX_EMP_SHIFT)

#define SPI_STA_TX_READY_SHIFT              (4)
#define SPI_STA_TX_READY_MASK               (0x1U << SPI_STA_TX_READY_SHIFT)

#define SPI_STA_RX_FULL_SHIFT               (2)
#define SPI_STA_RX_FULL_MASK                (0x1U << SPI_STA_RX_FULL_SHIFT)

#define SPI_STA_RX_EMP_SHIFT                (1)
#define SPI_STA_RX_EMP_MASK                 (0x1U << SPI_STA_RX_EMP_SHIFT)

#define SPI_STA_RX_RDY_SHIFT                (0)
#define SPI_STA_RX_RDY_MASK                 (0x1U << SPI_STA_RX_RDY_SHIFT)

/*
 * @brief SPI FIFO Control Register
 */
#define SPI_FCTL_TF_RST_SHIFT               (31)
#define SPI_FCTL_TF_RST_MASK                (0x1U << SPI_FCTL_TF_RST_SHIFT)

#define SPI_FCTL_TF_TEST_EN_SHIFT           (30)
#define SPI_FCTL_TF_TEST_EN_MASK            (0x1U << SPI_FCTL_TF_TEST_EN_SHIFT)

#define SPI_FCTL_TF_DRQ_EN_SHIFT            (24)
#define SPI_FCTL_TF_DRQ_EN_MASK             (0x1U << SPI_FCTL_TF_DRQ_EN_SHIFT)
#define SPI_FCTL_TF_DRQ_EN_BIT              HAL_BIT(24)

#define SPI_FCTL_TX_TRIG_LEVEL_SHIFT        (16)
#define SPI_FCTL_TX_TRIG_LEVEL_MASK         (0xFFU << SPI_FCTL_TX_TRIG_LEVEL_SHIFT)

#define SPI_FCTL_RF_RST_SHIFT               (15)
#define SPI_FCTL_RF_RST_MASK                (0x1U << SPI_FCTL_RF_RST_SHIFT)

#define SPI_FCTL_RF_TEST_SHIFT              (14)
#define SPI_FCTL_RF_TEST_MASK               (0x1U << SPI_FCTL_RF_TEST_SHIFT)

#define SPI_FCTL_RF_DRQ_EN_SHIFT            (8)
#define SPI_FCTL_RF_DRQ_EN_MASK             (0x1U << SPI_FCTL_RF_DRQ_EN_SHIFT)

#define SPI_FCTL_RX_TRIG_LEVEL_SHIFT        (0)
#define SPI_FCTL_RX_TRIG_LEVEL_MASK         (0xFFU << SPI_FCTL_RX_TRIG_LEVEL_SHIFT)

/*
 * @brief SPI FIFO Status Registe
 */
#define SPI_FST_TB_WR_SHIFT                 (31)
#define SPI_FST_TB_WR_MASK                  (0x1U << SPI_FST_TB_WR_SHIFT)

#define SPI_FST_TB_CNT_SHIFT                (28)
#define SPI_FST_TB_CNT_MASK                 (0x7U << SPI_FST_TB_CNT_SHIFT)

#define SPI_FST_TF_CNT_SHIFT                (16)
#define SPI_FST_TF_CNT_MASK                 (0xFFU << SPI_FST_TF_CNT_SHIFT)

#define SPI_FST_RB_WR_SHIFT                 (15)
#define SPI_FST_RB_WR_MASK                  (0x1U << SPI_FST_RB_WR_SHIFT)

#define SPI_FST_RB_CNT_SHIFT                (12)
#define SPI_FST_RB_CNT_MASK                 (0x7U << SPI_FST_RB_CNT_SHIFT)

#define SPI_FST_RF_CNT_SHIFT                (0)
#define SPI_FST_RF_CNT_MASK                 (0xFFU << SPI_FST_RF_CNT_SHIFT)

/*
 * @brief SPI Wait Clock Counter Register
 */
#define SPI_WAIT_SWC_SHIFT                  (16)
#define SPI_WAIT_SWC_MASK                   (0xFU << SPI_WAIT_SWC_SHIFT)

#define SPI_WAIT_WCC_SHIFT                  (0)
#define SPI_WAIT_WCC_MASK                   (0xFFFFU << SPI_WAIT_WCC_SHIFT)

/*
 * @brief SPI Clock Rate Control Register
 */
#define SPI_CCTR_DRS_SHIFT                  (12)
#define SPI_CCTR_DRS_MASK                   (0x1U << SPI_CCTR_DRS_SHIFT)
typedef enum {
	 SPI_CCTR_DRS_type_divRate1 = 0 << SPI_CCTR_DRS_SHIFT,
	 SPI_CCTR_DRS_type_divRate2 = 1 << SPI_CCTR_DRS_SHIFT
} SPI_CCTR_DRS_type;

#define SPI_CCTR_CDR1_SHIFT                 (8)
#define SPI_CCTR_CDR1_MASK                  (0xFU << SPI_CCTR_CDR1_SHIFT)

#define SPI_CCTR_CDR2_SHIFT                 (0)
#define SPI_CCTR_CDR2_MASK                  (0xFFU << SPI_CCTR_CDR2_SHIFT)

/*
 * @brief SPI Master mode Burst Control Register
 */
#define SPI_BC_MBC_SHIFT                    (0)
#define SPI_BC_MBC_MASK                     (0xFFFFFFU << SPI_BC_MBC_SHIFT)

/*
 * @brief SPI Master mode Transmit Counter Register
 */
#define SPI_TC_MWTC_SHIFT                   (0)
#define SPI_TC_MWTC_MASK                    (0xFFFFFFU << SPI_TC_MWTC_SHIFT)

/*
 * @brief SPI Burst Control Register
 */
#define SPI_BCC_DRM_SHIFT                   (28)
#define SPI_BCC_DRM_MASK                    (0x1U << SPI_BCC_DRM_SHIFT)

#define SPI_BCC_DBC_SHIFT                   (24)
#define SPI_BCC_DBC_MASK                    (0xFU << SPI_BCC_DBC_SHIFT)

#define SPI_BCC_STC_SHIFT                   (0)
#define SPI_BCC_STC_MASK                    (0xFFFFFFU << SPI_BCC_STC_SHIFT)

/*
 * @brief SPI Nomal DMA Mode Control Regist
 */
#define SPI_NDMA_MODE_CTRL_SHIFT            (0)
#define SPI_NDMA_MODE_CTRL_MASK             (0xFFU << SPI_NDMA_MODE_CTRL_SHIFT)

/*
 * @brief SPI TX Date Register
 */
#define SPI_TXD_SHIFT                       (0)
#define SPI_TXD_MASK                        (0xFFFFFFFFU << SPI_TXD_SHIFT)

/*
 * @brief SPI RX Date Register
 */
#define SPI_RXD_SHIFT                       (0)
#define SPI_RXD_MASK                        (0xFFFFFFFFU << SPI_RXD_SHIFT)


#define SPI_FIFO_SIZE                       (64)

/************************ public **************************************/
typedef enum {
	SPI0 = 0,   /*!< SPI0 controller */
	SPI1 = 1,   /*!< SPI1 controller */
	SPI_NUM = 2 /*!< only support 2 SPI controller for now */
} SPI_Port;

typedef SPI_CTRL_Mode SPI_Mode;

typedef SPI_TCTRL_Fbs SPI_FirstBit;

typedef SPI_TCTRL_DHB_Duplex SPI_Duplex;

typedef enum {
	SPI_CS_MODE_AUTO,   /*!< auto chip select mode */
	SPI_CS_MODE_MANUAL  /*!< manual chip select mode */
} SPI_CS_Mode;

typedef SPI_TCTRL_SS_Sel SPI_CS;

typedef SPI_SCLK_Mode SPI_SclkMode;

typedef enum {
	SPI_OPERATION_MODE_DMA,     /*!< dma mode moving data */
	SPI_OPERATION_MODE_POLL     /*!< cpu mode moving data */
} SPI_Operation_Mode;

typedef enum {
	SPI_IO_MODE_NORMAL,     /*!< MOSI to transmit data, MISO to receive data */
	SPI_IO_MODE_DUAL_RX,    /*!< MOSI and MISO to receive data */
	SPI_IO_MODE_DUAL_TX,    /*!< not support */
	SPI_IO_MODE_QUAD_RX,    /*!< not support */
	SPI_IO_MODE_QUAD_TX     /*!< not support */
} SPI_IO_Mode;

typedef enum {
	SPI_ATTRIBUTION_IO_MODE
} SPI_Attribution;

typedef struct {
	SPI_Mode                mode;       /*!< SPI master mode or slave mode, only master mode for now */
	SPI_Operation_Mode      opMode;     /*!< dma mode or poll(cpu) mode */
	SPI_FirstBit            firstBit;   /*!< msb or lsb on line */
	uint32_t                sclk;       /*!< device sclk frequency */
	SPI_SclkMode            sclkMode;   /*!< device sclk mode */
} SPI_Config;

#define SPI_CONFIG_DEFAULT \
	{ \
		.sclk = 6000000, \
		.sclkMode = SPI_SCLK_Mode0, \
		.firstBit = SPI_TCTRL_FBS_MSB, \
		.mode = SPI_CTRL_MODE_MASTER, \
		.opMode = SPI_OPERATION_MODE_POLL \
	}

typedef struct {
	SPI_Port port;      /*!< spi port */
	SPI_CS cs;          /*!< spi cs pin */
	SPI_Config config;  /*!< spi device config */
} SPI_Device;

typedef struct {
	uint32_t mclk;      /*!< SPI main working frequency */
	bool cs_level;      /*!< the cs voltage level of chip running */
} SPI_Global_Config;

#define SPI_MAX_WAIT_MS (2000)
/* #define SPI_SOURCE_CLK (24 * 1000 * 1000) */

typedef enum {
	SPI_INT_CS_DESELECT       = SPI_IER_SS_INT_EN_MASK,
	SPI_INT_TRANSFER_COMPLETE = SPI_IER_TC_INT_EN_MASK,
	SPI_INT_TXFIFO_UNDER_RUN  = SPI_IER_TF_UDR_INT_EN_MASK,
	SPI_INT_TXFIFO_OVERFLOW   = SPI_IER_TF_OVF_INT_EN_MASK,
	SPI_INT_RXFIFO_UNDER_RUN  = SPI_IER_RF_UDR_INT_EN_MASK,
	SPI_INT_RXFIFO_OVERFLOW   = SPI_IER_RF_OVF_INT_EN_MASK,
	SPI_INT_TXFIFO_FULL       = SPI_IER_TF_FUL_INT_EN_MASK,
	SPI_INT_TXFIFO_EMPTY      = SPI_IER_TX_EMP_INT_EN_MASK,
	SPI_INT_TXFIFO_READY      = SPI_IER_TX_ERQ_INT_EN_MASK,
	SPI_INT_RXFIFO_FULL       = SPI_IER_RF_FUL_INT_EN_MASK,
	SPI_INT_RXFIFO_EMPTY      = SPI_IER_RX_EMP_INT_EN_MASK,
	SPI_INT_RXFIFO_READY      = SPI_IER_RF_RDY_INT_EN_MASK
} SPI_Int_Type;

typedef void (*SPI_IRQCallback)(uint32_t irq, void *arg);

typedef struct {
	uint32_t         irqMask;
	SPI_IRQCallback  callback;
	void            *arg;
} SPI_IrqParam;

/*
 * @brief
 *        reset(not init) <--> ready(inited/closed) <--> busy(opened) <--> tx/rx/tx_rx(transmitting)
 *
 *        each state --> error
 */

/**
 * @brief Initialize SPI driver.
 * @note Each SPI port can master sevral devices, HAL_SPI_Init initialize the
 *       SPI port for its own devices. Mclk configed here must be the max freq
 *       of all devices under this SPI port.
 * @param port: spi port
 * @param gconfig:
 *        @arg gconfig->mclk: SPI controller frequency. It must be the the max
 *             freq of all devices under this SPI port.
 *        @arg gconfig->cs_level: chip selected level of SPI devices enabled,
 *             cs level should be same amound all devices.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_Init(SPI_Port port, const SPI_Global_Config *gconfig);

/**
 * @brief Deinitialize SPI driver.
 * @param port: spi port
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_Deinit(SPI_Port port);

/**
 * @brief Enable SPI device or not by chip select pin
 * @note Example: device is select by low voltage level, HAL_SPI_CS(spi_port, 1)
 *       output cs pin to low voltage level.
 * @param port: spi port.
 * @param select: select/enable device or not.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_CS(SPI_Port port, bool select);

/**
 * @brief Config SPI attribution, such as dual rx mode.
 * @note This is a ioctl.
 *       attr : arg
 *       SPI_ATTRIBUTION_IO_MODE : SPI_IO_MODE_NORMAL
 *                               : SPI_IO_MODE_DUAL_RX
 *       other is not support for now.
 * @param port: spi port
 * @param attr:
 *        @arg SPI_ATTRIBUTION_IO_MODE: config SPI to a IO mode, for example:
 *             flash has a dual output fast read mode may use this. And the
 *             arg can be SPI_IO_MODE_NORMAL or SPI_IO_MODE_DUAL_RX, others
 *             are not support for now.
 * @param arg: an arguement according attribution.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_Config(SPI_Port port, SPI_Attribution attr, uint32_t arg);

/* APIs for SPI master mode */

/**
 * @brief Receive data from SPI device.
 * @note  HAL_SPI_Receive use in spi master mode,
 *        HAL_SPI_Slave_StartReceive_DMA use in spi slave mode.
 * @param port: spi port
 * @param data: the buf to store received data, created by user.
 * @param size: the data size needed to receive.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_Receive(SPI_Port port, uint8_t *data, uint32_t size);

/**
 * @brief Transmit data to SPI device.
 * @note  HAL_SPI_Transmit use in spi master mode,
 *        HAL_SPI_Slave_StartTransmit_DMA use in spi slave mode.
 * @param port: spi port
 * @param data: the data transmitted to SPI device.
 * @param size: the data size needed to transmit.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_Transmit(SPI_Port port, uint8_t *data, uint32_t size);

/**
 * @brief Transmit and Receive data from SPI device in the same time.
 * @note Transmit data by MOSI pin, and the exactly same time, receive the
 *       data by MISO pin.
 * @param port: spi port
 * @param tx_data: the data transmitted to SPI device.
 * @param rx_data: the buf to store received data, created by user.
 * @param size: the data size needed to transmit and receive.
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_TransmitReceive(SPI_Port port, uint8_t *tx_data, uint8_t *rx_data, uint32_t size);

/**
 * @brief Open a SPI device before using this spi device.
 * @note Other SPI devices will block when open a spi device, please make sure
 *       this device will be close eventually.
 *       HAL_SPI_Open use in spi master mode,
 *       HAL_SPI_Slave_Open use in spi slave mode.
 * @param port: spi port
 * @param cs: spi cs pin. SPI_TCTRL_SS_SEL_SS0~3 are pin cs0~cs3.
 * @param config:
 *        @arg config->mode: spi in master mode or slave mode,but HAL_SPI_Open
 *                           api only suport mater mode.
 *        @arg config->opMode: use dma to move data or CPU to move data.
 *        @arg config->firstBit: data on line is MSB or LSB.
 *        @arg config->sclk: spi device working frequency.
 *        @arg config->sclkMode: SPI sclk mode.
 * @param msec: timeout in millisecond
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_Open(SPI_Port port, SPI_CS cs, SPI_Config *config, uint32_t msec);

/**
 * @brief Close a SPI device to release SPI port.
 * @note  HAL_SPI_Close use in spi master mode,
 *        HAL_SPI_Slave_Close use in spi slave mode.
 * @param port: spi port
 * @retval HAL_Status: The status of driver
 */
HAL_Status HAL_SPI_Close(SPI_Port port);

/* APIs for SPI slave mode */

HAL_Status HAL_SPI_Slave_Open(SPI_Port port, SPI_CS cs, SPI_Config *config, uint32_t msec);
HAL_Status HAL_SPI_Slave_Close(SPI_Port port);

HAL_Status HAL_SPI_Slave_InitTxDMA(SPI_Port port, const DMA_ChannelInitParam *param);
HAL_Status HAL_SPI_Slave_InitRxDMA(SPI_Port port, const DMA_ChannelInitParam *param);
HAL_Status HAL_SPI_Slave_DeInitTxDMA(SPI_Port port);
HAL_Status HAL_SPI_Slave_DeInitRxDMA(SPI_Port port);

HAL_Status HAL_SPI_Slave_StartTransmit_DMA(SPI_Port port, const uint8_t *buf, int32_t size);
int32_t HAL_SPI_Slave_StopTransmit_DMA(SPI_Port port);

HAL_Status HAL_SPI_Slave_StartReceive_DMA(SPI_Port port, uint8_t *buf, int32_t size);
int32_t HAL_SPI_Slave_StopReceive_DMA(SPI_Port port);

HAL_Status HAL_SPI_Slave_EnableIRQ(SPI_Port port, SPI_IrqParam *param);
HAL_Status HAL_SPI_Slave_DisableIRQ(SPI_Port port);

HAL_Status HAL_SPI_Slave_ClearTransferDone(SPI_Port port);
HAL_Status HAL_SPI_Slave_WaitTransferDone(SPI_Port port, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_CHIP_HAL_SPI_H_ */
