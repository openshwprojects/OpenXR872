/*
 * Copyright (C) 2021 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
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
#ifndef HTTP2_DECL_H
#define HTTP2_DECL_H

/* xradio nghttp2 declare */
#define XR_NGHTTP2_DECL

/* use mbedtls */
#define HTTP2_MBEDTLS

/* show debug information */
#define HTTP2_DEBUG

/**
 * setting debug threshold
 *  0  NONE
 *  1  ERR
 *  2  ERR, WAR
 *  3  ERR, WAR, INFO
 */
#define HTTP2_DEBUG_LEVEL_THRESHOLD  2

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "kernel/os/os.h"

#if (__CONFIG_NGHTTP2_HEAP_MODE == 1)
#include "driver/chip/psram/psram.h"
#endif

#ifdef HTTP2_MBEDTLS
#include <mbedtls/debug.h>
#include <mbedtls/net.h>
#include <mbedtls/ssl.h>
#include <mbedtls/sha1.h>
#include <mbedtls/base64.h>
#include <mbedtls/x509_crt.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#ifndef EVP_MAX_MD_SIZE
#define EVP_MAX_MD_SIZE 20
#endif

#if defined(MBEDTLS_SSL_RENEGOTIATION)
#define HTTP2_MBEDTLS_SSL_RENEGOTIATION
#endif

#if defined(MBEDTLS_SSL_ALPN)
#define HTTP2_MBEDTLS_SSL_ALPN
#endif
#endif /* HTTP2_MBEDTLS */

#if LWIP_IPV6
#define HTTP2_SP_IPV6
#endif

#define HTTP2_SOCKET                 int
#define HTTP2_INVALID_SOCKET         -1
#define HTTP2_SOCKET_TIMEO           (10 * 1000)    /* 10s */

#define XR_NGHTTP2_NV_NEW(NAME, VALUE) \
	{(uint8_t *)(NAME), (uint8_t *)(VALUE), strlen(NAME), strlen(VALUE), NGHTTP2_NV_FLAG_NONE}

#define HTTP2_OBJ_NEW(type, count)   (type *)http2_calloc(count, sizeof(type))
#define HTTP2_OBJ_FREE(ptr)          http2_free((void *)ptr)

#define HTTP2_MSLEEP(msec)           OS_MSleep(msec)

typedef void   *http2_ptr;
typedef uint8_t http2_bool;

#define HTTP2_TRUE                   1
#define HTTP2_FALSE                  0

#ifdef XR_NGHTTP2_DECL
http2_ptr http2_malloc(size_t size);
http2_ptr http2_calloc(size_t count, size_t size);
http2_ptr http2_realloc(http2_ptr ref, size_t size);
void      http2_free(http2_ptr ref);
#else
#define   http2_malloc(size) malloc(size)
#define   http2_calloc(count, size) calloc(size)
#define   http2_realloc(ref, size) realloc(ref, size)
#define   http2_free(ref) free(ref)
#endif

typedef OS_Thread_t http2_thread_t;

#define http2_thread_create(thread, name, entry, arg, priority, statck_size) \
	OS_ThreadCreate(thread, name, entry, arg, priority, statck_size)
#define http2_thread_delete(thread)  OS_ThreadDelete(thread)
#define http2_thread_isvalid(thread) OS_ThreadIsValid(thread)

typedef OS_Mutex_t http2_mutex_t;

#define http2_mutex_create(m)   OS_RecursiveMutexCreate(m)
#define http2_mutex_destroy(m)  OS_RecursiveMutexDelete(m)
#define http2_mutex_lock(m)     OS_RecursiveMutexLock(m, OS_WAIT_FOREVER)
#define http2_mutex_trylock(m)  OS_RecursiveMutexLock(m, 0)
#define http2_mutex_unlock(m)   OS_RecursiveMutexUnlock(m)

#endif /* HTTP2_DECL_H */
