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
#include "sys/dma_heap.h"
#include "driver/chip/sdmmc/hal_sdhost.h"
#include "driver/chip/sdmmc/sdmmc.h"
#ifdef CONFIG_USE_SDIO
#include "driver/chip/sdmmc/sdio.h"
#endif
#include "driver/chip/private/hal_debug.h"
#include "driver/chip/hal_dcache.h"

#include "../hal_base.h"

#include "_sd_define.h"

#include "_sdhost.h"
#include "_core.h"
#ifdef CONFIG_USE_SDIO
#include "_sdio.h"
#endif
#ifdef CONFIG_USE_SD
#include "_sd.h"
#endif
#ifdef CONFIG_USE_MMC
#include "_mmc.h"
#endif

#ifdef __CONFIG_ROM

extern struct mmc_card *card_info[SDC_NUM];
extern int32_t _mmc_block_read(struct mmc_card *card, uint8_t *buf, uint64_t sblk, uint32_t nblk);
extern int32_t _mmc_block_write(struct mmc_card *card, const uint8_t *buf, uint64_t sblk, uint32_t nblk);
extern int32_t _mmc_card_deinit(struct mmc_card *card);
extern void mmc_power_off(struct mmc_host *host);

int32_t mmc_block_read(struct mmc_card *card, uint8_t *buf, uint64_t sblk, uint32_t nblk)
{
	int32_t err;
	uint8_t *dma_buf = buf;
#if ((defined __CONFIG_PSRAM_ALL_CACHEABLE) && (defined __CONFIG_PSRAM))
	uint8_t bufIsCacheable = HAL_Dcache_IsCacheable((uint32_t)buf, (uint32_t)(nblk*512));

	if (bufIsCacheable) {
		dma_buf = dma_malloc(nblk*512, DMAHEAP_PSRAM);
		if (dma_buf == NULL) {
			HAL_ERR("dma_malloc failed\n");
			return -1;
		}
	}
#endif

	HAL_SDC_Claim_Host(card->host);
	err = _mmc_block_read(card, dma_buf, sblk, nblk);
	HAL_SDC_Release_Host(card->host);
#if ((defined __CONFIG_PSRAM_ALL_CACHEABLE) && (defined __CONFIG_PSRAM))
	if (bufIsCacheable) {
		HAL_Memcpy(buf, dma_buf, nblk*512);
		dma_free(dma_buf, DMAHEAP_PSRAM);
	}
#endif
	return err;
}

int32_t mmc_block_write(struct mmc_card *card, const uint8_t *buf, uint64_t sblk, uint32_t nblk)
{
	int32_t err;
	uint8_t *dma_buf = (uint8_t *)buf;
#if ((defined __CONFIG_PSRAM_ALL_CACHEABLE) && (defined __CONFIG_PSRAM))
	uint8_t bufIsCacheable = HAL_Dcache_IsCacheable((uint32_t)buf, (uint32_t)(nblk*512));

	if (bufIsCacheable) {
		dma_buf = dma_malloc(nblk*512, DMAHEAP_PSRAM);
		if (dma_buf == NULL) {
			HAL_ERR("dma_malloc failed\n");
			return -1;
		}
		HAL_Memcpy(dma_buf, buf, nblk*512);
	}
#endif
	HAL_SDC_Claim_Host(card->host);
	err = _mmc_block_write(card, dma_buf, sblk, nblk);
	HAL_SDC_Release_Host(card->host);
#if ((defined __CONFIG_PSRAM_ALL_CACHEABLE) && (defined __CONFIG_PSRAM))
	if (bufIsCacheable) {
		dma_free(dma_buf, DMAHEAP_PSRAM);
	}
#endif
	return err;
}

/**
 * @brief deinit SD card.
 * @param card:
 *        @arg card->card handler.
 * @retval  0 if success or other if failed.
 */
int32_t mmc_card_deinit(struct mmc_card *card)
{
	struct mmc_host *host;

	if (!card || !card->host) {
		SD_LOGE("%s err\n", __func__);
		return -1;
	}

	host = card->host;

	mmc_claim_host(host);
#ifdef CONFIG_USE_SDIO
	if (mmc_card_sdio(card) || mmc_card_sd_combo(card))
		mmc_deattach_sdio(card, host);
#endif
#ifdef CONFIG_USE_SD
	if (mmc_card_sd(card))
		mmc_deattach_sd(card, host);
#endif

	mmc_power_off(host);
	mmc_release_host(host);
	HAL_SDC_Close(host->sdc_id);

	return 0;
}

/**
 * @brief malloc for card_info.
 * @param card_id:
 *        @arg card ID.
 * @retval  0 if success or other if failed.
 */
int32_t mmc_card_create(uint8_t card_id, SDCard_InitTypeDef *param)
{
	int ret = 0;
	struct mmc_card *card = card_info[card_id];

	if (card != NULL) {
		if (card->id == card_id) {
			SD_LOGW("%s id:%d already!!\n", __func__, card_id);
			return 0;
		} else {
			SD_LOGE("%s id:%d unvalid card id!!\n", __func__, card_id);
			return -1;
		}
	}

	card = HAL_Malloc(sizeof(struct mmc_card));
	if (card == NULL) {
		SD_LOGE("%s malloc fail\n", __func__);
		ret = -1;
	} else {
		SDC_Memset(card, 0, sizeof(struct mmc_card));
		SDC_MutexCreate(&card->mutex);
		SDC_MutexLock(&card->mutex, OS_WAIT_FOREVER);
		card->id = card_id;
		card->ref = 1;
		card->debug_mask = param->debug_mask;
		if (param->type < MMC_TYPE_MAX)
			card->type = param->type;
		card_info[card_id] = card;
		SDC_MutexUnlock(&card->mutex);
		SD_LOGN("%s card:%p id:%d\n", __func__, card, card->id);
	}

	return ret;
}

/**
 * @brief free for card_info.
 * @param card_id:
 *        @arg card ID.
 * @param flg:
 *        @arg 0:normal delete, 1:unnormal delete, internal use.
 * @retval  0 if success or other if failed.
 */
int32_t mmc_card_delete(uint8_t card_id, uint32_t flg)
{
	int ret = -1;
	struct mmc_card *card = card_info[card_id];

	if (card == NULL || card->id != card_id) {
		SD_LOGW("%s card not exit! card:%p id:%d del_id:%d\n",
		        __func__, card, card->id, card_id);
		return -1;
	}

	SDC_MutexLock(&card->mutex, OS_WAIT_FOREVER);
	if (!flg)
		card->ref--;
	if (card->ref != 0) {
		SDC_MutexUnlock(&card->mutex);
		SD_LOGW("%s fail, ref:%d\n", __func__, card->ref);
		goto out;
	}
	card_info[card_id] = NULL;
	SDC_MutexUnlock(&card->mutex);
	SDC_MutexDelete(&card->mutex);
	SD_LOGN("%s card:%p id:%d\n", __func__, card, card->id);
	HAL_Free(card);
	ret = 0;

out:

	return ret;
}

/**
 * @brief get pointer of mmc_card.
 * @param card_id:
 *        @arg card ID.
 * @retval  pointer of mmc_card if success or NULL if failed.
 */
struct mmc_card *mmc_card_open(uint8_t card_id)
{
	struct mmc_card *card = card_info[card_id];

	if (card == NULL || card->id != card_id) {
		SD_LOGW("%s card not exit! id:%d\n",  __func__, card_id);
		return NULL;
	}

	SDC_MutexLock(&card->mutex, OS_WAIT_FOREVER);
	card->ref++;
	SDC_MutexUnlock(&card->mutex);
	SD_LOGD("%s card:%p id:%d\n", __func__, card, card->id);

	return card;
}

/**
 * @brief close mmc_card.
 * @param card_id:
 *        @arg card ID.
 * @retval  0 if success or other if failed.
 */
int32_t mmc_card_close(uint8_t card_id)
{
	struct mmc_card *card = card_info[card_id];

	if (card == NULL || card->id != card_id || card->ref <= 0) {
		SD_LOGW("%s fail! id:%d\n",  __func__, card_id);
		return -1;
	}

	SDC_MutexLock(&card->mutex, OS_WAIT_FOREVER);
	card->ref--;
	SDC_MutexUnlock(&card->mutex);
	if (!card->ref) {
		mmc_card_delete(card_id, 1);
	}
	SD_LOGD("%s card:%p id:%d\n", __func__, card, card->id);

	return 0;
}

#endif

uint32_t mmc_get_capacity(struct mmc_card *card)
{
	if (!card) {
		HAL_ERR("card not exist\n");
		return 0;
	}
	return card->csd.capacity;
}

