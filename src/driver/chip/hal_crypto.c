/**
 * @file  hal_crypto.c
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

#include <stdbool.h>
#include "sys/io.h"
#include "driver/chip/hal_crypto.h"
#include "hal_base.h"
#include "sys/endian.h"
#include "pm/pm.h"
#include "sys/xr_debug.h"


/*************************************** Debug *****************************************/
#define CRYPTO_ALE  (0)
#define CRYPTO_ERR  (1)
#define CRYPTO_INF  (2)
#define CRYPTO_DBG  (3)
#define CRYPTO_PRINT_LEVEL (CRYPTO_INF)
#define CRYPTO_PRINT(print_level, fmt, arg...)  \
	do {                                        \
		if (print_level <= CRYPTO_PRINT_LEVEL)  \
			printf("[Crypto] "fmt, ##arg);      \
	} while (0);

#define CE_CRC_HASH_DMA_ENA 0    /* use dma to do crc/hash */
#define HAL_PRNG_RAND_NUM   5    /* number of random value for one generation */
#define HAL_PRNG_SEED_NUM   6

#define __CE_STATIC_INLINE__ static inline

#ifdef SWAP32
#undef SWAP32
#endif
#define SWAP32(d) bswap32(d)

static void  __attribute__((unused)) CE_Reg_All(const int line)
{
#if (CRYPTO_DBG <= CRYPTO_PRINT_LEVEL)
	printf("******************CE:Line %d**************************\n", line);
	for (int i = 0; i < 45; i += 1) {
		if (i % 4 == 0)
			printf("\n0x%8x:", (uint32_t)(&(CE->CTL) + i));
		printf("  0x%8x", (uint32_t)(*(&(CE->CTL) + i)));
	}
	printf("\n");
#endif
}

/*
 * @brief CE
 */
__CE_STATIC_INLINE__
void CE_Enable(CE_T *ce)
{
	HAL_SET_BIT(ce->CTL, CE_CTL_ENABLE_MASK);
}

__CE_STATIC_INLINE__
void CE_Disable(CE_T *ce)
{
	HAL_CLR_BIT(ce->CTL, CE_CTL_ENABLE_MASK);
}

__CE_STATIC_INLINE__
uint8_t CE_GetInputAvailCnt(CE_T *ce)
{
	return HAL_GET_BIT_VAL(ce->FCSR, CE_FCSR_RXFIFO_EMP_CNT_SHIFT, CE_FCSR_RXFIFO_EMP_CNT_VMASK);
}

__CE_STATIC_INLINE__
uint8_t CE_GetOutputAvailCnt(CE_T *ce)
{
	return HAL_GET_BIT_VAL(ce->FCSR, CE_FCSR_TXFIFO_AVA_CNT_SHIFT, CE_FCSR_TXFIFO_AVA_CNT_VMASK);
}

__CE_STATIC_INLINE__
void CE_SetInputThreshold(CE_T *ce, uint8_t threshold)
{
	/*if ((threshold & ~CE_FCSR_RXFIFO_INT_TRIG_LEVEL_MASK) == 0) {
		CRYPTO_PRINT(CRYPTO_ERR, "input threshold error\n");
	} */
	HAL_MODIFY_REG(ce->FCSR, CE_FCSR_RXFIFO_INT_TRIG_LEVEL_MASK, threshold << CE_FCSR_RXFIFO_INT_TRIG_LEVEL_SHIFT);
}

__CE_STATIC_INLINE__
void CE_SetOutputThreshold(CE_T *ce, uint8_t threshold)
{
	/*if ((threshold & ~CE_FCSR_TXFIFO_INT_TRIG_LEVEL_MASK) == 0) {
		CRYPTO_PRINT(CRYPTO_ERR, "onput threshold error\n");
	} */
	HAL_MODIFY_REG(ce->FCSR, CE_FCSR_TXFIFO_INT_TRIG_LEVEL_MASK, threshold << CE_FCSR_TXFIFO_INT_TRIG_LEVEL_SHIFT);
}

typedef enum {
	CE_INT_TPYE_TXFIFO_AVA = CE_ICSR_TXFIFO_AVA_INT_EN_SHIFT,
	CE_INT_TPYE_RXFIFO_EMP = CE_ICSR_RXFIFO_EMP_INT_EN_SHIFT,
	CE_INT_TPYE_HASH_CRC_END = CE_ICSR_HASH_CRC_END_INT_EN_SHIFT
} CE_Int_Type;

__CE_STATIC_INLINE__
void CE_EnableInt(CE_T *ce, CE_Int_Type type)
{
	HAL_SET_BIT(ce->ICSR, 1 << type);
}

__CE_STATIC_INLINE__
void CE_DisableInt(CE_T *ce, CE_Int_Type type)
{
	HAL_CLR_BIT(ce->ICSR, 1 << type);
}

__CE_STATIC_INLINE__
bool CE_Status(CE_T *ce, CE_Int_Type type)
{
	return (bool)HAL_GET_BIT_VAL(ce->ICSR, CE_ICSR_TXFIFO_AVA_SHIFT + type, 1);
}

__CE_STATIC_INLINE__
void CE_EnableDMA(CE_T *ce)
{
	HAL_SET_BIT(ce->ICSR, CE_ICSR_DRQ_ENABLE_MASK);
}

__CE_STATIC_INLINE__
void CE_DisableDMA(CE_T *ce)
{
	HAL_CLR_BIT(ce->ICSR, CE_ICSR_DRQ_ENABLE_MASK);
}

__CE_STATIC_INLINE__
void CE_SetLength(CE_T *ce, uint32_t len)
{
	ce->CTS_LEN = len;
}

/*
 * @brief CRC
 */
__CE_STATIC_INLINE__
HAL_Status CE_CRC_Init(CE_T *ce,
                       bool init_val,
                       bool xor_out,
                       bool ref_in,
                       bool ref_out,
                       CE_CTL_CRC_Width width,
                       uint32_t poly)
{
	bool last_pckt = 0;

	HAL_CLR_BIT(ce->CTL, CE_CTL_END_BIT_MASK);
	HAL_MODIFY_REG(ce->CTL,
	               CE_CTL_CRC_INIT_MASK
	               | CE_CTL_CRC_REF_IN_MASK
	               | CE_CTL_CRC_REF_OUT_MASK
	               | CE_CTL_CRC_XOR_OUT_MASK
	               | CE_CTL_CRC_WIDTH_MASK
	               | CE_CTL_CRC_CONT_MASK
	               | CE_CTL_OP_DIR_MASK
	               | CE_CTL_METHOD_MASK,
	               (init_val << CE_CTL_CRC_INIT_SHIFT)
	               | (ref_in << CE_CTL_CRC_REF_IN_SHIFT)
	               | (ref_out << CE_CTL_CRC_REF_OUT_SHIFT)
	               | (xor_out << CE_CTL_CRC_XOR_OUT_SHIFT)
	               | (width)
	               | (last_pckt << CE_CTL_CRC_CONT_SHIFT) /* not the last packet */
	               | CE_CRYPT_OP_ENCRYPTION
	               | CE_CTL_METHOD_CRC);
	ce->CRC_POLY = poly;
	return HAL_OK;
}

__CE_STATIC_INLINE__
void CE_CRC_Fill(CE_T *ce, uint32_t data)
{
	while (CE_GetInputAvailCnt(ce) == 0)
		;
	ce->RXFIFO = data;
}

__CE_STATIC_INLINE__
void CE_CRC_Finish(CE_T *ce)
{
	HAL_SET_BIT(ce->CTL, CE_CTL_END_BIT_MASK);
}

__CE_STATIC_INLINE__
void CE_CRC_Calc(CE_T *ce, uint32_t *crc)    /* _____________16bit mode is it generate a 16bits crc? */
{
	*crc = ce->CRC_RESULT;
}

__CE_STATIC_INLINE__
void CE_CRC_Deinit(CE_T *ce)
{
	/*CE_Disable(ce);*/
}

__CE_STATIC_INLINE__
void CE_CRC_SetResult(CE_T *ce, uint32_t crc)
{
	ce->CRC_RESULT = crc;
}

/*
 * @brief AES/DES/3DES
 */
__CE_STATIC_INLINE__
void CE_Crypto_Init(CE_T *ce, CE_CTL_Method algo, CE_CTL_Crypto_Mode mode, CE_Crypto_Op op)
{
	if (algo > CE_CTL_METHOD_3DES) {
		CRYPTO_PRINT(CRYPTO_ERR, "init error\n");
	}
	HAL_MODIFY_REG(ce->CTL,
	               CE_CTL_METHOD_MASK | CE_CTL_OP_MODE_MASK | CE_CTL_OP_DIR_MASK,
	               algo | mode | op); /* ce enable */
}

__CE_STATIC_INLINE__
void CE_AES_SetKey(CE_T *ce, CE_CTL_KeySource src, CE_CTL_AES_KeySize size, uint32_t *key)
{
	uint8_t key_cnt;
	uint8_t i = 0;
	HAL_MODIFY_REG(ce->CTL, CE_CTL_KEY_SEL_MASK | CE_CTL_AES_KEY_SIZE_MASK, src | size);
	if (size == CE_CTL_AES_KEYSIZE_128BITS)
		key_cnt = 4;
	else if (size == CE_CTL_AES_KEYSIZE_192BITS)
		key_cnt = 6;
	else if (size == CE_CTL_AES_KEYSIZE_256BITS)
		key_cnt = 8;
	else {
		CRYPTO_PRINT(CRYPTO_ERR, "param is out of range\n");
		return;
	}

	while (i < key_cnt) {
		ce->KEY[i] = *(key++);
		i++;
	}
}

__CE_STATIC_INLINE__
void CE_DES_SetKey(CE_T *ce, CE_CTL_KeySource src, uint32_t key[2]) /* 56bit key? 64bit key? = 2 * 32bit reg? */
{
	HAL_MODIFY_REG(ce->CTL, CE_CTL_KEY_SEL_MASK, src);
	ce->KEY[0] = key[0];
	ce->KEY[1] = key[1];
}

__CE_STATIC_INLINE__
void CE_3DES_SetKey(CE_T *ce, CE_CTL_KeySource src, uint32_t key[6]) /* 3 * 64bit key? = 6 * 32bit reg? */
{
	uint8_t i = 0;
	HAL_MODIFY_REG(ce->CTL, CE_CTL_KEY_SEL_MASK, src);

	while (i < CE_3DES_KEY_COUNT) {
		ce->KEY[i] = *(key++);
		i++;
	}
}

#ifdef CE_CTR_MODE
__CE_STATIC_INLINE__
HAL_Status CE_Crypto_SetCounter(CE_T *ce, uint32_t *cnt, CE_CTL_CtrWidth size)
{
	HAL_MODIFY_REG(ce->CTL, CE_CTL_CTR_WIDTH_MASK, size);
	switch (size) {
	case CE_CTL_CTRWITDH_128BITS:
		ce->CNT[3] = *(cnt + 3);
		ce->CNT[2] = *(cnt + 2);
	case CE_CTL_CTRWITDH_64BITS:
		ce->CNT[1] = *(cnt + 1);
	case CE_CTL_CTRWITDH_32BITS:
		ce->CNT[0] = *(cnt + 0);
		break;
	case CE_CTL_CTRWITDH_16BITS:
		ce->CNT[0] = *(cnt + 0) & 0xFFFF;
		break;
	default:
		return HAL_INVALID;
	}
	return HAL_OK;
}
#endif

__CE_STATIC_INLINE__
void CE_Crypto_SetLength(CE_T *ce, uint32_t len)
{
	CE_SetLength(ce, len);
}

__CE_STATIC_INLINE__
void CE_Crypto_SetIV(CE_T *ce, const uint32_t iv[4], uint8_t size)
{
	uint8_t i = size / 4;
	while (i--)
		ce->IV[i] = iv[i];
}

__CE_STATIC_INLINE__
void CE_Crypto_Input(CE_T *ce, uint32_t data)
{
	while (CE_GetInputAvailCnt(ce) == 0)
		;
	ce->RXFIFO = data;
}

__CE_STATIC_INLINE__
void CE_Crypto_Output(CE_T *ce, uint32_t *data)
{
	while (CE_GetOutputAvailCnt(ce) == 0)
		;
	*data = ce->TXFIFO;
}

__CE_STATIC_INLINE__
void CE_Crypto_Deinit(CE_T *ce)
{
	CE_Disable(ce);
}

__CE_STATIC_INLINE__
volatile uint32_t *CE_Crypto_GetInputAddr(CE_T *ce)
{
	return &ce->RXFIFO;
}

__CE_STATIC_INLINE__
volatile const uint32_t *CE_Crypto_GetOutputAddr(CE_T *ce)
{
	return &ce->TXFIFO;
}

__CE_STATIC_INLINE__
void CE_Crypto_GetIV(CE_T *ce, uint32_t iv[4], uint8_t size)
{
	uint8_t i = size / 4;
	while (i--)
		iv[i] = ce->IV[i];
}

#ifdef CE_CTR_MODE
__CE_STATIC_INLINE__
HAL_Status CE_Crypto_GetCounter(CE_T *ce, uint32_t cnt[4], CE_CTL_CtrWidth size)
{
	switch (size) {
	case CE_CTL_CTRWITDH_128BITS:
		cnt[2] = ce->CNT[2];
		cnt[3] = ce->CNT[3];
	case CE_CTL_CTRWITDH_64BITS:
		cnt[1] = ce->CNT[1];
	case CE_CTL_CTRWITDH_32BITS:
	case CE_CTL_CTRWITDH_16BITS:
		cnt[0] = ce->CNT[0];
		break;
	default:
		return HAL_INVALID;
	}
	return HAL_OK;
}
#endif

/*
 * @brief MD5/sha1/sha256
 */
void CE_Hash_Init(CE_T *ce, CE_CTL_Method algo)
{
	if ((algo != CE_CTL_METHOD_SHA1) && (algo != CE_CTL_METHOD_MD5) && (algo != CE_CTL_METHOD_SHA256)) {
		CRYPTO_PRINT(CRYPTO_ERR, "hash init error\n");
	}

	HAL_CLR_BIT(ce->CTL, CE_CTL_END_BIT_MASK);
	HAL_MODIFY_REG(ce->CTL,
	               CE_CTL_METHOD_MASK | CE_CTL_ENABLE_MASK | CE_CTL_OP_DIR_MASK,
	               algo | (1 << CE_CTL_ENABLE_SHIFT) | CE_CRYPT_OP_ENCRYPTION);
}

void CE_Hash_SetIV(CE_T *ce, CE_CTL_IvMode_SHA_MD5 iv_src, const uint32_t *iv, uint32_t iv_size)
{
	HAL_MODIFY_REG(ce->CTL, CE_CTL_IV_MODE_MASK, iv_src);

	if (iv_src == CE_CTL_IVMODE_SHA_MD5_FIPS180) {
		return;
	}

	ce->IV[0] = SWAP32(iv[0]);
	ce->IV[1] = SWAP32(iv[1]);
	ce->IV[2] = SWAP32(iv[2]);
	ce->IV[3] = SWAP32(iv[3]);
	if (iv_size == 5)
		ce->CNT[0] = SWAP32(iv[4]);
	else if (iv_size == 8) {
		ce->CNT[0] = SWAP32(iv[4]);
		ce->CNT[1] = SWAP32(iv[5]);
		ce->CNT[2] = SWAP32(iv[6]);
		ce->CNT[3] = SWAP32(iv[7]);
	}

}

__CE_STATIC_INLINE__
void CE_Hash_Finish(CE_T *ce)
{
	HAL_SET_BIT(ce->CTL, CE_CTL_END_BIT_MASK);
}

__CE_STATIC_INLINE__
void CE_Hash_Fill(CE_T *ce, uint32_t data)
{
	while (CE_GetInputAvailCnt(ce) == 0)
		;
	ce->RXFIFO = data;
}

void CE_Hash_Calc(CE_T *ce, CE_CTL_Method algo, uint32_t *digest)    /*  md5[5] sha1[5] sha256[8] */
{
	if (algo == CE_CTL_METHOD_SHA256) {
		*(digest + 7) = ce->MD7;
		*(digest + 6) = ce->MD6;
		*(digest + 5) = ce->MD5;
		*(digest + 4) = ce->MD4;
	} else if (algo == CE_CTL_METHOD_SHA1)
		*(digest + 4) = ce->MD4;
	*(digest + 3) = ce->MD3;
	*(digest + 2) = ce->MD2;
	*(digest + 1) = ce->MD1;
	*(digest + 0) = ce->MD0;
}

__CE_STATIC_INLINE__
void CE_Hash_Deinit(CE_T *ce)
{
	CE_Disable(ce);
}

/*
 * @brief PRNG
 */
__CE_STATIC_INLINE__
void CE_PRNG_Init(CE_T *ce, bool continue_mode)
{
	HAL_MODIFY_REG(ce->CTL,
	               CE_CTL_PRNG_MODE_MASK | CE_CTL_METHOD_MASK | CE_CTL_ENABLE_MASK,
	               (continue_mode << CE_CTL_PRNG_MODE_SHIFT) | CE_CTL_METHOD_PRNG | (1 << CE_CTL_ENABLE_SHIFT));

}

__CE_STATIC_INLINE__
void CE_PRNG_Seed(CE_T *ce, uint32_t seed[HAL_PRNG_SEED_NUM])
{
	int i;
	for (i = 0; i < HAL_PRNG_SEED_NUM; ++i) {
		ce->KEY[i] = seed[i];
	}
}

__CE_STATIC_INLINE__
void CE_PRNG_Generate(CE_T *ce, uint32_t random[HAL_PRNG_RAND_NUM])
{
	HAL_SET_BIT(ce->CTL, CE_CTL_PRNG_START_MASK);

	while (HAL_GET_BIT(CE->CTL, CE_CTL_PRNG_START_MASK) != 0)
		;

	random[0] = ce->MD0;
	random[1] = ce->MD1;
	random[2] = ce->MD2;
	random[3] = ce->MD3;
	random[4] = ce->MD4;
}

__CE_STATIC_INLINE__
void CE_PRNG_Deinit(CE_T *ce)
{
	CE_Disable(ce);
}


/************************ AES DES 3DES private **************************************/
static HAL_Mutex ce_lock;
static HAL_Semaphore ce_block;

__CE_STATIC_INLINE__
void HAL_CE_EnableCCMU(void)
{
	HAL_CCM_BusEnablePeriphClock(CCM_BUS_PERIPH_BIT_CE);
	HAL_CCM_CE_EnableMClock();
}

__CE_STATIC_INLINE__
void HAL_CE_DisableCCMU(void)
{
	HAL_CCM_CE_DisableMClock();
	HAL_CCM_BusDisablePeriphClock(CCM_BUS_PERIPH_BIT_CE);
}

/*static void HAL_CE_InputCmpl(void *arg)
{
	CE_DEBUG("input by dma had been finished.\n");
}

static void HAL_CE_OutputCmpl(void *arg)
{
	HAL_SemaphoreRelease(&ce_block);
	CE_DEBUG("output by dma had been finished.\n");
}*/

__nonxip_text
static void HAL_CE_DMACmpl(void *arg)
{
	if (arg != NULL)
		HAL_SemaphoreRelease(&ce_block);
#ifndef __CONFIG_SECTION_ATTRIBUTE_NONXIP
	CRYPTO_PRINT(CRYPTO_DBG, "Transfer by dma had been finished.\n");
#endif
}

static HAL_Status HAL_Crypto_InitDMA(DMA_Channel *input, DMA_Channel *output)
{
	HAL_Status ret = HAL_OK;
	DMA_ChannelInitParam Output_param;
	DMA_ChannelInitParam Input_param;
	HAL_Memset(&Output_param, 0, sizeof(Output_param));
	HAL_Memset(&Input_param, 0, sizeof(Input_param));

	*output = HAL_DMA_Request();
	if ((*output) == DMA_CHANNEL_INVALID) {
		CRYPTO_PRINT(CRYPTO_ERR, "DMA request failed \n");
		ret = HAL_INVALID;
		goto out;
	}
	*input = HAL_DMA_Request();
	if ((*input) == DMA_CHANNEL_INVALID) {
		CRYPTO_PRINT(CRYPTO_ERR, "DMA request failed \n");
		HAL_DMA_Release(*output);
		ret = HAL_INVALID;
		goto out;
	}

	Output_param.cfg = HAL_DMA_MakeChannelInitCfg(DMA_WORK_MODE_SINGLE,
	                   DMA_WAIT_CYCLE_16,
	                   DMA_BYTE_CNT_MODE_REMAIN,
	                   DMA_DATA_WIDTH_8BIT,
	                   DMA_BURST_LEN_4,
	                   DMA_ADDR_MODE_INC,
	                   DMA_PERIPH_SRAM,
	                   DMA_DATA_WIDTH_32BIT,
	                   DMA_BURST_LEN_1,
	                   DMA_ADDR_MODE_FIXED,
	                   DMA_PERIPH_CE);
	Output_param.irqType = DMA_IRQ_TYPE_END;
	Output_param.endCallback = HAL_CE_DMACmpl;
	Output_param.endArg = &ce_block;

	Input_param.cfg = HAL_DMA_MakeChannelInitCfg(DMA_WORK_MODE_SINGLE,
	                  DMA_WAIT_CYCLE_16,
	                  DMA_BYTE_CNT_MODE_REMAIN,
	                  DMA_DATA_WIDTH_32BIT,
	                  DMA_BURST_LEN_1,
	                  DMA_ADDR_MODE_FIXED,
	                  DMA_PERIPH_CE,
	                  DMA_DATA_WIDTH_8BIT,
	                  DMA_BURST_LEN_4,
	                  DMA_ADDR_MODE_INC,
	                  DMA_PERIPH_SRAM);
	Input_param.irqType = DMA_IRQ_TYPE_END;
	Input_param.endCallback = HAL_CE_DMACmpl;
	Input_param.endArg = NULL;

	HAL_DMA_Init(*input, &Input_param);
	HAL_DMA_Init(*output, &Output_param);

out:
	return ret;
}

static inline void HAL_Crypto_DeinitDMA(DMA_Channel input, DMA_Channel output)
{
	HAL_DMA_DeInit(input);
	HAL_DMA_DeInit(output);
	HAL_DMA_Release(input);
	HAL_DMA_Release(output);
}

static HAL_Status HAL_Crypto_Convey(uint8_t *input, uint8_t *output, uint32_t size, uint8_t block_size)
{
	HAL_Status ret = HAL_OK;
	uint32_t align_len = size & (~(block_size - 1));
	uint32_t remain_len = size & (block_size - 1);
	uint8_t padding[16] = {0};
	/*CRYPTO_PRINT(CRYPTO_ERR, "align_len = %d, remain_len = %d, len = %d\n",
	               align_len, remain_len, remain_len ? (align_len + block_size) : align_len); */

	/* data preprocess */
	if (remain_len != 0)
		HAL_Memcpy(padding, &input[align_len], remain_len);
	CE_Crypto_SetLength(CE, remain_len ? (align_len + block_size) : align_len);

	/* if the size<300, CPU mode more fast than DMA mode */
	if (size > 300) {
		/* use DMA mode */
		DMA_Channel Output_channel = DMA_CHANNEL_INVALID;
		DMA_Channel Input_channel = DMA_CHANNEL_INVALID;

		/* data transfer by DMA */
		ret = HAL_Crypto_InitDMA(&Input_channel, &Output_channel);
		if (ret != HAL_OK) {
			CRYPTO_PRINT(CRYPTO_ERR, "DMA Request failed\n");
			goto out;
		}

		CE_SetInputThreshold(CE, 0);
		CE_SetOutputThreshold(CE, 0);
		CE_EnableDMA(CE);
		CE_Enable(CE);

		if (align_len != 0) {
			HAL_DMA_Start(Input_channel, (uint32_t)input, (uint32_t)CE_Crypto_GetInputAddr(CE), align_len);
			HAL_DMA_Start(Output_channel, (uint32_t)CE_Crypto_GetOutputAddr(CE), (uint32_t)output, align_len);
			ret = HAL_SemaphoreWait(&ce_block, CE_WAIT_TIME);
			if (ret != HAL_OK) {
				CRYPTO_PRINT(CRYPTO_ERR, "DMA 1st transfer failed\n");
				HAL_DMA_Stop(Input_channel);
				HAL_DMA_Stop(Output_channel);
				goto failed;
			}
			HAL_DMA_Stop(Input_channel);
			HAL_DMA_Stop(Output_channel);
		}

		if (remain_len != 0) {
			HAL_DMA_Start(Input_channel, (uint32_t)padding, (uint32_t)CE_Crypto_GetInputAddr(CE), block_size);
			CE_Reg_All(__LINE__);

			HAL_DMA_Start(Output_channel, (uint32_t)CE_Crypto_GetOutputAddr(CE), (uint32_t)&output[align_len], block_size);
			ret = HAL_SemaphoreWait(&ce_block, CE_WAIT_TIME);
			if (ret != HAL_OK) {
				CRYPTO_PRINT(CRYPTO_ERR, "DMA 2nd transfer failed\n");
				CE_Reg_All(__LINE__);
				HAL_DMA_Stop(Input_channel);
				HAL_DMA_Stop(Output_channel);
				goto failed;
			}
			HAL_DMA_Stop(Input_channel);
			HAL_DMA_Stop(Output_channel);
		}

failed:
		CE_Disable(CE);
		CE_DisableDMA(CE);
		HAL_Crypto_DeinitDMA(Input_channel, Output_channel);
out:
		return ret;
	} else {
		/* use CPU mode */
		CE_Enable(CE);
		if (align_len != 0) {
			uint32_t i = 0;
			uint32_t j = 0;
			for (i = 0; i < align_len; i += block_size) {
				for (j = 0; j < block_size; j += 4) {
					while (!HAL_GET_BIT(CE->FCSR, CE_FCSR_RXFIFO_STATUS_MASK))
						;
					*CE_Crypto_GetInputAddr(CE) = *((uint32_t *)&input[i + j]);
				}
				for (j = 0; j < block_size; j += 4) {
					while (!HAL_GET_BIT(CE->FCSR, CE_FCSR_TXFIFO_STATUS_MASK))
						;
					*((uint32_t *)&output[i + j]) = *CE_Crypto_GetOutputAddr(CE);
				}
			}
		}
		if (remain_len != 0) {
			uint32_t j = 0;
			for (j = 0; j < sizeof(padding); j += 4) {
				while (!HAL_GET_BIT(CE->FCSR, CE_FCSR_RXFIFO_STATUS_MASK))
					;
				*CE_Crypto_GetInputAddr(CE) = *((uint32_t *)(padding + j));
			}
			for (j = 0; j < sizeof(padding); j += 4) {
				while (!HAL_GET_BIT(CE->FCSR, CE_FCSR_TXFIFO_STATUS_MASK))
					;
				*((uint32_t *)&output[align_len + j]) = *CE_Crypto_GetOutputAddr(CE);
			}
		}
		CE_Disable(CE);
		return ret;
	}
}

int ce_running;
#ifdef CONFIG_PM
static int hal_ce_suspending;

static int ce_suspend(struct soc_device *dev, enum suspend_state_t state)
{
	if (ce_running)
		return -1;
	hal_ce_suspending = 1;

	return 0;
}

static int ce_resume(struct soc_device *dev, enum suspend_state_t state)
{
	switch (state) {
	case PM_MODE_SLEEP:
		break;
	case PM_MODE_STANDBY:
	case PM_MODE_HIBERNATION:
		HAL_CE_Init();
		break;
	default:
		break;
	}

	hal_ce_suspending = 0;

	return 0;
}

static const struct soc_device_driver ce_drv = {
	.name = "ce",
	.suspend = ce_suspend,
	.resume = ce_resume,
};

static struct soc_device ce_dev = {
	.name = "ce",
	.driver = &ce_drv,
};

#define CE_DEV (&ce_dev)
#endif


/************************ public **************************************/

HAL_Status HAL_CE_Init()
{
	HAL_Status ret = HAL_OK;
	uint32_t clk;
	uint32_t div;

	HAL_CE_DisableCCMU();
	HAL_CCM_BusForcePeriphReset(CCM_BUS_PERIPH_BIT_CE);
	HAL_CCM_BusReleasePeriphReset(CCM_BUS_PERIPH_BIT_CE);
	clk = HAL_GetDevClock();
	div = (clk - 1) / (80 * 1000 * 1000) + 1;
	CRYPTO_PRINT(CRYPTO_DBG, "CCMU src clk is %u, div is %u\n", clk, div);

	if (div > (8 * 16))
		return HAL_ERROR;
	else if (div > (4 * 16))
		HAL_CCM_CE_SetMClock(CCM_AHB_PERIPH_CLK_SRC_DEVCLK, CCM_PERIPH_CLK_DIV_N_8, (CCM_PeriphClkDivM)((div >> 3) - 1));
	else if (div > (2 * 16))
		HAL_CCM_CE_SetMClock(CCM_AHB_PERIPH_CLK_SRC_DEVCLK, CCM_PERIPH_CLK_DIV_N_4, (CCM_PeriphClkDivM)((div >> 2) - 1));
	else if (div > (1 & 16))
		HAL_CCM_CE_SetMClock(CCM_AHB_PERIPH_CLK_SRC_DEVCLK, CCM_PERIPH_CLK_DIV_N_2, (CCM_PeriphClkDivM)((div >> 1) - 1));
	else
		HAL_CCM_CE_SetMClock(CCM_AHB_PERIPH_CLK_SRC_DEVCLK, CCM_PERIPH_CLK_DIV_N_1, (CCM_PeriphClkDivM)((div >> 0) - 1));

#ifdef CONFIG_PM
	if (!hal_ce_suspending) {
#endif

		ret = HAL_MutexInit(&ce_lock);
		if (ret != HAL_OK)
			goto out;
		ret = HAL_SemaphoreInit(&ce_block, 0, 1);
		if (ret != HAL_OK) {
			HAL_MutexDeinit(&ce_lock);
			goto out;
		}

		ce_running = 0;

#ifdef CONFIG_PM
		pm_register_ops(CE_DEV);
	}
#endif

out:
	return ret;
}

HAL_Status HAL_CE_Deinit()
{
	HAL_Status ret = HAL_OK;

#ifdef CONFIG_PM
	if (!hal_ce_suspending)
		pm_unregister_ops(CE_DEV);
#endif

	ret = HAL_MutexDeinit(&ce_lock);
	if (ret != HAL_OK)
		goto out;
	ret = HAL_SemaphoreDeinit(&ce_block);
	if (ret != HAL_OK)
		goto out;
out:
	return ret;
}

HAL_Status HAL_AES_Encrypt(CE_AES_Config *aes, uint8_t *plain, uint8_t *cipher, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();

	/* CE config */
	CE_Crypto_Init(CE, CE_CTL_METHOD_AES, aes->mode, CE_CRYPT_OP_ENCRYPTION);
	CE_AES_SetKey(CE, aes->src, aes->keysize, (uint32_t *)aes->key);
	if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_SetIV(CE, (uint32_t *)aes->iv, 16);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_SetCounter(CE, (uint32_t *)aes->cnt, aes->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_SetIV(CE, (uint32_t *)aes->iv, 16);
#endif
	CE_Reg_All(__LINE__);

	/* plain data convey to CE and cipher data convey from CE */
	HAL_Crypto_Convey(plain, cipher, size, AES_BLOCK_SIZE);
	CE_Reg_All(__LINE__);

	/* CE state storage */
	if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_GetIV(CE, (uint32_t *)aes->iv, 16);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTR) {
		CE_Crypto_GetCounter(CE, (uint32_t *)aes->cnt, aes->width);
		aes->cnt[15] += (size + 15) / 16;
	}
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_GetIV(CE, (uint32_t *)aes->iv, 16);
#endif

	CE_Crypto_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);
out:
	ce_running = 0;
	return ret;
}

HAL_Status HAL_AES_Decrypt(CE_AES_Config *aes, uint8_t *cipher, uint8_t *plain, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();

	/* CE config */
	CE_Crypto_Init(CE, CE_CTL_METHOD_AES, aes->mode, CE_CRYPT_OP_DECRYPTION);
	CE_AES_SetKey(CE, aes->src, aes->keysize, (uint32_t *)aes->key);
	if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_SetIV(CE, (uint32_t *)aes->iv, 16);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_SetCounter(CE, (uint32_t *)aes->cnt, aes->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_SetIV(CE, (uint32_t *)aes->iv, 16);
#endif
	CE_Reg_All(__LINE__);

	/* plain data convey to CE and cipher data convey from CE */
	HAL_Crypto_Convey(cipher, plain, size, AES_BLOCK_SIZE);
	CE_Reg_All(__LINE__);

	/* CE state storage */
	if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_GetIV(CE, (uint32_t *)aes->iv, 16);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTR) {
		CE_Crypto_GetCounter(CE, (uint32_t *)aes->cnt, aes->width);
		aes->cnt[15] += (size + 15) / 16;
	}
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)aes->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_GetIV(CE, (uint32_t *)aes->iv, 16);
#endif

	CE_Crypto_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);
out:
	ce_running = 0;
	return ret;
}

HAL_Status HAL_DES_Encrypt(CE_DES_Config *des, uint8_t *plain, uint8_t *cipher, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();

	/* CE config */
	CE_Crypto_Init(CE, CE_CTL_METHOD_DES, des->mode, CE_CRYPT_OP_ENCRYPTION);
	CE_DES_SetKey(CE, des->src, (uint32_t *)des->key);
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_SetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#endif

	/* plain data convey to CE and cipher data convey from CE */
	HAL_Crypto_Convey(plain, cipher, size, DES_BLOCK_SIZE);

	/* CE state storage */
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_GetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#endif

	CE_Crypto_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);
out:
	ce_running = 0;
	return ret;
}

HAL_Status HAL_DES_Decrypt(CE_DES_Config *des, uint8_t *cipher, uint8_t *plain, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();

	/* CE config */
	CE_Crypto_Init(CE, CE_CTL_METHOD_DES, des->mode, CE_CRYPT_OP_DECRYPTION);
	CE_DES_SetKey(CE, des->src, (uint32_t *)des->key);
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_SetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#endif

	/* plain data convey to CE and cipher data convey from CE */
	HAL_Crypto_Convey(cipher, plain, size, DES_BLOCK_SIZE);

	/* CE state storage */
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_GetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#endif

	CE_Crypto_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);
out:
	ce_running = 0;
	return ret;
}

HAL_Status HAL_3DES_Encrypt(CE_3DES_Config *des, uint8_t *plain, uint8_t *cipher, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();

	/* CE config */
	CE_Crypto_Init(CE, CE_CTL_METHOD_3DES, des->mode, CE_CRYPT_OP_ENCRYPTION);
	CE_3DES_SetKey(CE, des->src, (uint32_t *)des->key);
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_SetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#endif

	/* plain data convey to CE and cipher data convey from CE */
	HAL_Crypto_Convey(plain, cipher, size, DES3_BLOCK_SIZE);

	/* CE state storage */
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_GetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#endif

	CE_Crypto_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);

out:
	ce_running = 0;

	return ret;
}

HAL_Status HAL_3DES_Decrypt(CE_3DES_Config *des, uint8_t *cipher, uint8_t *plain, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();

	/* CE config */
	CE_Crypto_Init(CE, CE_CTL_METHOD_3DES, des->mode, CE_CRYPT_OP_DECRYPTION);
	CE_3DES_SetKey(CE, des->src, (uint32_t *)des->key);
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_SetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_SetIV(CE, (uint32_t *)des->iv, 8);
#endif

	/* plain data convey to CE and cipher data convey from CE */
	HAL_Crypto_Convey(cipher, plain, size, 8);

	/* CE state storage */
	if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CBC)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#ifdef CE_CTR_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTR)
		CE_Crypto_GetCounter(CE, (uint32_t *)des->cnt, des->width);
#endif
#ifdef CE_CTS_MODE
	else if ((CE_CTL_Crypto_Mode)des->mode == CE_CTL_CRYPT_MODE_CTS)
		CE_Crypto_GetIV(CE, (uint32_t *)des->iv, 8);
#endif

	CE_Crypto_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);
out:
	ce_running = 0;
	return ret;
}


/************************ CRC MD5 SHA1 SHA256 private **************************************/
typedef struct {
	CE_CTL_CRC_Width width;
	uint32_t poly;
	uint8_t init;
	uint8_t refin;
	uint8_t refout;
	uint8_t xorout;
} CE_CRC_Config;

static const CE_CRC_Config crc_cfg[] = {
	{CE_CTL_CRC_WIDTH_16BITS, 0x8005,       0, 1, 1, 0},        /* CE_CRC16_IBM,    */
	{CE_CTL_CRC_WIDTH_16BITS, 0x8005,       0, 1, 1, 1},        /* CE_CRC16_MAXIM,  */
	{CE_CTL_CRC_WIDTH_16BITS, 0x8005,       1, 1, 1, 1},        /* CE_CRC16_USB,    */
	{CE_CTL_CRC_WIDTH_16BITS, 0x8005,       1, 1, 1, 0},        /* CE_CRC16_MODBUS, */
	{CE_CTL_CRC_WIDTH_16BITS, 0x1021,       0, 1, 1, 0},        /* CE_CRC16_CCITT,  */
	{CE_CTL_CRC_WIDTH_16BITS, 0x1021,       1, 0, 0, 0},        /* CE_CRC16_CCITT_1 */
	{CE_CTL_CRC_WIDTH_16BITS, 0x1021,       1, 1, 1, 1},        /* CE_CRC16_X25,    */
	{CE_CTL_CRC_WIDTH_16BITS, 0x1021,       0, 0, 0, 0},        /* CE_CRC16_XMODEM, */
	{CE_CTL_CRC_WIDTH_16BITS, 0x3d65,       0, 1, 1, 1},        /* CE_CRC16_DNP,    */
	{CE_CTL_CRC_WIDTH_32BITS, 0x04c11db7,   1, 1, 1, 1},        /* CE_CRC32,        */
	{CE_CTL_CRC_WIDTH_32BITS, 0x04c11db7,   1, 0, 0, 0},        /* CE_CRC32_MPEG2,  */
};

#if CE_CRC_HASH_DMA_ENA

static HAL_Status HAL_CRC_Hash_InitDMA(DMA_Channel *input)
{
	HAL_Status ret = HAL_OK;
	DMA_ChannelInitParam Input_param;
	HAL_Memset(&Input_param, 0, sizeof(Input_param));

	*input = HAL_DMA_Request();
	if ((*input) == DMA_CHANNEL_INVALID) {
		ret = HAL_INVALID;
		CRYPTO_PRINT(CRYPTO_ERR, "DMA request failed \n");
		goto out;
	}

	Input_param.cfg = HAL_DMA_MakeChannelInitCfg(DMA_WORK_MODE_SINGLE,
	                                             DMA_WAIT_CYCLE_16,
	                                             DMA_BYTE_CNT_MODE_REMAIN,
	                                             DMA_DATA_WIDTH_32BIT,
	                                             DMA_BURST_LEN_1,
	                                             DMA_ADDR_MODE_FIXED,
	                                             DMA_PERIPH_CE,
	                                             DMA_DATA_WIDTH_8BIT,
	                                             DMA_BURST_LEN_4,
	                                             DMA_ADDR_MODE_INC,
	                                             DMA_PERIPH_SRAM);
	Input_param.irqType = DMA_IRQ_TYPE_END;
	Input_param.endCallback = HAL_CE_DMACmpl;
	Input_param.endArg = &ce_block;

	HAL_DMA_Init(*input, &Input_param);
	CE_EnableDMA(CE);

out:
	return ret;
}

static void HAL_CRC_Hash_DenitDMA(DMA_Channel input)
{
	CE_DisableDMA(CE);
	HAL_DMA_DeInit(input);
	HAL_DMA_Release(input);
}

#endif /* CE_CRC_HASH_DMA_ENA */

static HAL_Status HAL_CRC_Hash_Convey(CE_Fifo_Align *align, uint8_t *data, uint32_t size)
{

	HAL_Status ret = HAL_OK;
	uint32_t align_len;
	uint32_t buf_left = 4 - align->word_size;

	CRYPTO_PRINT(CRYPTO_DBG, "input size = %u, buf_left = %u\n", size, buf_left);

	if (size < buf_left) {
		HAL_Memcpy(align->word + align->word_size, data, size);
		align->word_size += size;
		goto out;
	}
	HAL_Memcpy(&align->word[align->word_size], data, buf_left);
#if CE_CRC_HASH_DMA_ENA
	/* use DMA mode */
	DMA_Channel input = DMA_CHANNEL_INVALID;
	ret = HAL_CRC_Hash_InitDMA(&input);
	if (ret != HAL_OK) {
		CRYPTO_PRINT(CRYPTO_ERR, "DMA Request failed\n");
		goto out;
	}

	CE_SetInputThreshold(CE, 0);

	HAL_DMA_Start(input, (uint32_t)align->word, (uint32_t)CE_Crypto_GetInputAddr(CE), 4);
	ret = HAL_SemaphoreWait(&ce_block, CE_WAIT_TIME);
	if (ret != HAL_OK) {
		CRYPTO_PRINT(CRYPTO_ERR, "DMA transfer 1st block failed\n");
		HAL_CRC_Hash_DenitDMA(input);
		goto out;
	}
	HAL_DMA_Stop(input);

	align->word_size = (size - buf_left) & 0x3; // len % 4
	align_len = size - buf_left - align->word_size;
	//CE_DEBUG("align_len = %d, new buf_left should be = %d\n", align_len, align->word_size);

	if (align_len != 0) {
		HAL_DMA_Start(input, (uint32_t)(data + buf_left), (uint32_t)CE_Crypto_GetInputAddr(CE), align_len);
		ret = HAL_SemaphoreWait(&ce_block, CE_WAIT_TIME);
		if (ret != HAL_OK) {
			CRYPTO_PRINT(CRYPTO_ERR, "DMA transfer 2rd block failed\n");
			HAL_CRC_Hash_DenitDMA(input);
			goto out;
		}
		HAL_DMA_Stop(input);
	}
	HAL_CRC_Hash_DenitDMA(input);
#else /* CE_CRC_HASH_DMA_ENA */
	/* use CPU mode */
	while (!HAL_GET_BIT(CE->FCSR, CE_FCSR_RXFIFO_STATUS_MASK))
		;
	uint32_t *p = (uint32_t *)align->word;
	*CE_Crypto_GetInputAddr(CE) = *p;

	align->word_size = (size - buf_left) & 0x3; // len % 4
	align_len = size - buf_left - align->word_size;

	uint32_t i;
	if (align_len != 0) {
		for (i = 0; i < align_len; i += 4) {
			while (!HAL_GET_BIT(CE->FCSR, CE_FCSR_RXFIFO_STATUS_MASK))
				;
			*CE_Crypto_GetInputAddr(CE) = *((uint32_t *)&data[buf_left + i]);
		}
	}
#endif /* CE_CRC_HASH_DMA_ENA */
	HAL_Memset(align->word, 0, sizeof(align->word));
	HAL_Memcpy(align->word, data + size - align->word_size, align->word_size);

out:
	return ret;
}

HAL_Status HAL_MD5_Append(CE_MD5_Handler *hdl, uint8_t *data, uint32_t size);

static void HAL_Hash_Finish(CE_MD5_Handler *hdl, uint64_t bit_size)
{
	uint32_t pad_size;
	uint32_t remain_size = hdl->total_size & 0x3f; /* len % 64 */
	/* uint32_t total_size = hdl->total_size; */
	uint8_t pad[128] = {0};

	/* CE_DEBUG("total size before padding = %d, remain size = %d.\n", (uint32_t)hdl->total_size, (uint32_t)remain_size); */

	if (remain_size < 56)
		pad_size = 64 - remain_size - 8 - 1;
	else
		pad_size = 128 - remain_size - 8 - 1;
	pad[0] = 0x80;
	HAL_Memcpy(&pad[1 + pad_size], &bit_size, 8);

	/* CE_DEBUG("pad size = %d, buf size = %d, len copy to %d.\n",
	            (uint32_t)pad_size, (uint32_t)hdl->word_size, (uint32_t)(1 + pad_size)); */

	HAL_MD5_Append(hdl, pad, pad_size + 1 + 8);

	/* CE_ASSERT(((total_size + pad_size + 1 + 8) & 0x3f) == 0); */
	/* CE_DEBUG("append size = %d, final total size = 0x%x.\n",
		         (uint32_t)(pad_size + 1 + 8), (uint32_t)(total_size + pad_size + 1 + 8)); */

	CE_Hash_Finish(CE);
}
/************************ public **************************************/

/**
 * @brief Initialize CRC module.
 * @param hdl: It's a handler stored private info. created by user.
 * @param type: CRC algorithm.
 * @param total_size: the total size of data needed to calculate crc.
 * @retval HAL_Status:  The status of driver
 */
HAL_Status HAL_CRC_Init1(CE_CRC_Handler *hdl, CE_CRC_Types type, uint32_t total_size)
{
	HAL_Status ret = HAL_OK;
	const CE_CRC_Config *config = &crc_cfg[type];

	if (total_size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;
	HAL_CE_EnableCCMU();

	hdl->type = type;
	hdl->crc = 0;
	hdl->word_size = 0;
	HAL_Memset(hdl->word, 0, sizeof(hdl->word));

	CE_CRC_Init(CE, config->init,
	            config->xorout,
	            config->refin,
	            config->refout,
	            config->width,
	            config->poly);
	CE_CRC_SetResult(CE, 0);
	CE_SetLength(CE, total_size);
	CE_Enable(CE);
	CE_Reg_All(__LINE__);

out:
	return ret;
}

HAL_Status HAL_CRC_Init(CE_CRC_Handler *hdl, CE_CRC_Types type, uint32_t total_size)
{
	uint32_t crc;

	/* temperarily bug fixed */
	HAL_CRC_Init1(hdl, CE_CRC32, 1);
	HAL_CRC_Append(hdl, (uint8_t *)hdl, 1);
	HAL_CRC_Finish(hdl, &crc);

	HAL_Memset(hdl, 0, sizeof(*hdl));
	return HAL_CRC_Init1(hdl, type, total_size);
}

HAL_Status HAL_CRC_Append(CE_CRC_Handler *hdl, uint8_t *data, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ret = HAL_CRC_Hash_Convey((CE_Fifo_Align *)(&hdl->word[0]), data, size);
	if (ret == HAL_OK)
		hdl->total_size += size;

out:
	return ret;
}

HAL_Status HAL_CRC_Finish(CE_CRC_Handler *hdl, uint32_t *crc)
{
	HAL_Status ret = HAL_OK;
	uint8_t pad[4] = {0};

	HAL_CRC_Append(hdl, pad, (4 - hdl->word_size) & 0x03);

	CE_CRC_Finish(CE);
	while (CE_Status(CE, CE_INT_TPYE_HASH_CRC_END) == 0)
		;        /* use irq would be better */
	CE_CRC_Calc(CE, crc);
	CE_Reg_All(__LINE__);
	CE_Disable(CE);
	CE_CRC_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);

	ce_running = 0;
	return ret;
}

HAL_Status HAL_MD5_Init(CE_MD5_Handler *hdl, CE_Hash_IVsrc src, const uint32_t iv[4])
{
	HAL_Status ret = HAL_OK;

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();
	CE_Hash_Init(CE, CE_CTL_METHOD_MD5);
	CE_Hash_SetIV(CE, src, iv, CE_MD5_IV_SIZE);

	hdl->total_size = 0;
	hdl->word_size = 0;
	HAL_Memset(hdl->word, 0, sizeof(hdl->word));
	CE_Reg_All(__LINE__);

out:
	return ret;
}

HAL_Status HAL_MD5_Append(CE_MD5_Handler *hdl, uint8_t *data, uint32_t size)
{
	HAL_Status ret = HAL_OK;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ret = HAL_CRC_Hash_Convey((CE_Fifo_Align *)(&hdl->word[0]), data, size);
	if (ret == HAL_OK)
		hdl->total_size += size;

out:
	return ret;
}

HAL_Status HAL_MD5_Finish(CE_MD5_Handler *hdl, uint32_t digest[4])
{
	HAL_Status ret = HAL_OK;
	uint64_t bit_size = htole64(hdl->total_size << 3);

	HAL_Hash_Finish(hdl, bit_size);
	while (CE_Status(CE, CE_INT_TPYE_HASH_CRC_END) == 0)
		;        /* use irq would be better */
	CE_Reg_All(__LINE__);

	CE_Hash_Calc(CE, CE_CTL_METHOD_MD5, digest);

	CE_Hash_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);

	ce_running = 0;
	return ret;
}

HAL_Status HAL_SHA1_Init(CE_SHA1_Handler *hdl, CE_Hash_IVsrc src, const uint32_t iv[5])
{
	HAL_Status ret = HAL_OK;

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();
	CE_Hash_Init(CE, CE_CTL_METHOD_SHA1);
	CE_Hash_SetIV(CE, src, iv, CE_SHA1_IV_SIZE);
	CE_Reg_All(__LINE__);

	hdl->total_size = 0;
	hdl->word_size = 0;
	HAL_Memset(hdl->word, 0, sizeof(hdl->word));

out:
	return ret;
}

HAL_Status HAL_SHA1_Append(CE_SHA1_Handler *hdl, uint8_t *data, uint32_t size)
{
	HAL_Status ret;
	ret = HAL_MD5_Append(hdl, data, size);
	return ret;
}

HAL_Status HAL_SHA1_Finish(CE_SHA1_Handler *hdl, uint32_t digest[5])
{
	HAL_Status ret = HAL_OK;
	uint64_t bit_size = htobe64(hdl->total_size << 3);

	HAL_Hash_Finish(hdl, bit_size);
	CE_Reg_All(__LINE__);
	while (CE_Status(CE, CE_INT_TPYE_HASH_CRC_END) == 0)
		;        /* use irq */
	CE_Reg_All(__LINE__);
	CE_Hash_Calc(CE, CE_CTL_METHOD_SHA1, digest);

	CE_Hash_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);

	ce_running = 0;
	return ret;
}

HAL_Status HAL_SHA256_Init(CE_SHA256_Handler *hdl, CE_Hash_IVsrc src, const uint32_t iv[8])
{
	HAL_Status ret = HAL_OK;

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;

	HAL_CE_EnableCCMU();
	CE_Hash_Init(CE, CE_CTL_METHOD_SHA256);
	CE_Hash_SetIV(CE, src, iv, CE_SHA256_IV_SIZE);
	CE_Reg_All(__LINE__);

	hdl->total_size = 0;
	hdl->word_size = 0;
	HAL_Memset(hdl->word, 0, sizeof(hdl->word));

out:
	return ret;
}

HAL_Status HAL_SHA256_Append(CE_SHA256_Handler *hdl, uint8_t *data, uint32_t size)
{
	HAL_Status ret;
	ret = HAL_MD5_Append(hdl, data, size);
	return ret;
}

HAL_Status HAL_SHA256_Finish(CE_SHA256_Handler *hdl, uint32_t digest[8])
{
	HAL_Status ret = HAL_OK;
	uint64_t bit_size = htobe64(hdl->total_size << 3);

	HAL_Hash_Finish(hdl, bit_size);
	while (CE_Status(CE, CE_INT_TPYE_HASH_CRC_END) == 0)
		;        /* use irq would be better */
	CE_Reg_All(__LINE__);

	CE_Hash_Calc(CE, CE_CTL_METHOD_SHA256, digest);

	CE_Hash_Deinit(CE);
	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);

	ce_running = 0;
	return ret;
}

static uint32_t prng_seed[HAL_PRNG_SEED_NUM];

HAL_Status HAL_PRNG_SetSeed(uint32_t seed[6])
{
	HAL_Memcpy(prng_seed, seed, sizeof(prng_seed));
	return HAL_OK;
}

HAL_Status HAL_PRNG_Generate(uint8_t *random, uint32_t size)
{
#define HAL_PRNG_MAX_RAND_LEN   (HAL_PRNG_RAND_NUM * sizeof(uint32_t))

	HAL_Status ret = HAL_OK;
	uint32_t *seed = prng_seed;
	uint8_t *p = random;
	uint32_t cpsize;

	if (size == 0) {
		ret = HAL_INVALID;
		goto out;
	}

	ce_running = 1;
	ret = HAL_MutexLock(&ce_lock, CE_WAIT_TIME);
	if (ret != HAL_OK)
		goto out;
	HAL_CE_EnableCCMU();

	CE_PRNG_Init(CE, 0);
	CE_PRNG_Seed(CE, seed);
	CE_Reg_All(__LINE__);

	while (1) {
		CE_PRNG_Generate(CE, seed);
		cpsize = (size <= HAL_PRNG_MAX_RAND_LEN) ? size : HAL_PRNG_MAX_RAND_LEN;
		HAL_Memcpy(p, (uint8_t *)seed, cpsize);
		if (size <= HAL_PRNG_MAX_RAND_LEN)
			break;
		p += cpsize;
		size -= cpsize;
	}
	seed[5] ^= seed[SysTick->VAL % 5];
	CE_Reg_All(__LINE__);

	CE_PRNG_Deinit(CE);

	HAL_CE_DisableCCMU();
	HAL_MutexUnlock(&ce_lock);

out:
	ce_running = 0;
	return ret;
#undef HAL_PRNG_MAX_RAND_LEN
}
