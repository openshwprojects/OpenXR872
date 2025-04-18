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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys/dma_heap.h"
#include "sys/sram_heap.h"
#ifdef __CONFIG_PSRAM_MALLOC_TRACE
#include "debug/heap_trace.h"
#endif

#if ((defined __CONFIG_PSRAM) && (__CONFIG_DMAHEAP_PSRAM_SIZE != 0))

#include "sys/param.h"
#include "sys/sys_heap.h"
#include "driver/chip/psram/psram.h"
#include "kernel/os/os_thread.h"
#include "driver/chip/hal_dcache.h"

static sys_heap_t g_dma_psram_heap;
static int dma_psram_heap_init_flag = 0;

static __always_inline int is_rangeof_dmaheap(void *ptr)
{
	return RANGEOF((uint32_t)ptr, (uint32_t)DMAHEAP_PSRAM_BASE, DMAHEAP_PSRAM_BASE + DMAHEAP_PSRAM_LENGTH);
}

static int32_t dma_psram_heap_init(void)
{
	HAL_Dcache_Enable_WriteThrough(DMAHEAP_PSRAM_BASE, rounddown2(DMAHEAP_PSRAM_END, 16));
	sys_heap_default_init(&g_dma_psram_heap, (uint8_t *)DMAHEAP_PSRAM_BASE, (size_t)DMAHEAP_PSRAM_LENGTH);
	sys_heap_init(&g_dma_psram_heap);
	return 0;
}

void *dma_malloc(size_t size, uint32_t flag)
{
	void *ptr;

	if (flag == DMAHEAP_PSRAM) {
		if (dma_psram_heap_init_flag == 0) {
			malloc_mutex_lock();
			if (dma_psram_heap_init_flag == 0) {
				dma_psram_heap_init();
				dma_psram_heap_init_flag = 1;
			}
			malloc_mutex_unlock();
		}

#ifdef __CONFIG_PSRAM_MALLOC_TRACE
		size += HEAP_MAGIC_LEN;
#endif

		ptr = sys_heap_malloc(&g_dma_psram_heap, size);

#ifdef __CONFIG_PSRAM_MALLOC_TRACE
		heap_trace_malloc(ptr, size - HEAP_MAGIC_LEN);
#endif

	} else {
		ptr = sram_malloc(size);
	}

	return ptr;
}

void dma_free(void *ptr, uint32_t flag)
{
	if (ptr == NULL) {
		return;
	}

	if (flag == DMAHEAP_PSRAM) {
#ifdef __CONFIG_PSRAM_MALLOC_TRACE
		heap_trace_free(ptr);
#endif
		sys_heap_free(&g_dma_psram_heap, ptr);
#ifdef __CONFIG_HEAP_FREE_CHECK
		HEAP_ASSERT(is_rangeof_dmaheap(ptr));
#endif
	} else {
		sram_free(ptr);
	}
}

void *dma_realloc(void *ptr, size_t size, uint32_t flag)
{
	void *new_ptr;

	if (flag == DMAHEAP_PSRAM) {
		if (dma_psram_heap_init_flag == 0) {
			malloc_mutex_lock();
			if (dma_psram_heap_init_flag == 0) {
				dma_psram_heap_init();
				dma_psram_heap_init_flag = 1;
			}
			malloc_mutex_unlock();
		}

#ifdef __CONFIG_PSRAM_MALLOC_TRACE
		malloc_mutex_lock();
		if (size != 0) {
			size += HEAP_MAGIC_LEN;
		}
#endif
		new_ptr = sys_heap_realloc(&g_dma_psram_heap, ptr, size);
#ifdef __CONFIG_PSRAM_MALLOC_TRACE
		if (size == 0) {
			heap_trace_realloc(ptr, new_ptr, size);
		} else {
			heap_trace_realloc(ptr, new_ptr, size - HEAP_MAGIC_LEN);
		}
		malloc_mutex_unlock();
#endif
	} else {
		new_ptr = sram_realloc(ptr, size);
	}

	return new_ptr;
}

void *dma_calloc(size_t nmemb, size_t size, uint32_t flag)
{
	void *ptr;

	ptr = dma_malloc(nmemb * size, flag);
	if (ptr != NULL) {
		memset(ptr, 0, nmemb * size);
	}
	return ptr;
}

size_t dma_psram_total_heap_size(void)
{
	return DMAHEAP_PSRAM_LENGTH;
}

size_t dma_psram_free_heap_size(void)
{
	if (dma_psram_heap_init_flag == 0) {
		malloc_mutex_lock();
		if (dma_psram_heap_init_flag == 0) {
			dma_psram_heap_init();
			dma_psram_heap_init_flag = 1;
		}
		malloc_mutex_unlock();
	}

	return sys_heap_xPortGetFreeHeapSize(&g_dma_psram_heap);
}

size_t dma_psram_minimum_ever_free_heap_size(void)
{
	if (dma_psram_heap_init_flag == 0) {
		malloc_mutex_lock();
		if (dma_psram_heap_init_flag == 0) {
			dma_psram_heap_init();
			dma_psram_heap_init_flag = 1;
		}
		malloc_mutex_unlock();
	}

	return sys_heap_xPortGetMinimumEverFreeHeapSize(&g_dma_psram_heap);
}

#else

void *dma_malloc(size_t size, uint32_t flag)
{
	return sram_malloc(size);
}

void dma_free(void *ptr, uint32_t flag)
{
	sram_free(ptr);
}

void *dma_realloc(void *ptr, size_t size, uint32_t flag)
{
	return sram_realloc(ptr, size);
}

void *dma_calloc(size_t nmemb, size_t size, uint32_t flag)
{
	return sram_calloc(nmemb, size);
}

#endif /* ((defined __CONFIG_PSRAM) && (__CONFIG_DMAHEAP_PSRAM_SIZE != 0)) */
