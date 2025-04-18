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
#ifndef HTTP2_DEBUG_H
#define HTTP2_DEBUG_H

#include <stdio.h>
#include "nghttp2/http2_decl.h"

#define HTTP2_LEVEL_INFO 1
#define HTTP2_LEVEL_WAR  2
#define HTTP2_LEVEL_ERR  3
#define HTTP2_LEVEL_NONE 4

#if HTTP2_DEBUG_LEVEL_THRESHOLD >= 3
#define HTTP2_DEBUG_LEVEL HTTP2_LEVEL_INFO
#elif HTTP2_DEBUG_LEVEL_THRESHOLD == 2
#define HTTP2_DEBUG_LEVEL HTTP2_LEVEL_WAR
#elif HTTP2_DEBUG_LEVEL_THRESHOLD == 1
#define HTTP2_DEBUG_LEVEL HTTP2_LEVEL_ERR
#else
#define HTTP2_DEBUG_LEVEL HTTP2_LEVEL_NONE
#endif /* HTTP2_DEBUG_LEVEL_THRESHOLD */

#define HTTP2_DEBUG_PROMPT(level) (level == HTTP2_LEVEL_ERR ? \
                                   "[net HTTP2] ERR: " : (level == HTTP2_LEVEL_WAR ? \
                                   "[net HTTP2] WAR: " : "[net HTTP2] INFO: "))

#if 1
#define HTTP2_PLATFORM_DIAG(format, ...) printf(format, ##__VA_ARGS__)
#endif

#ifdef HTTP2_DEBUG
#define HTTP2_DBG(level, message, ...) \
	do { \
		if (level >= HTTP2_DEBUG_LEVEL) { \
			HTTP2_PLATFORM_DIAG(HTTP2_DEBUG_PROMPT(level)); \
			HTTP2_PLATFORM_DIAG(message, ##__VA_ARGS__); \
		} \
	} while (0)
#else  /* HTTP2_DEBUG */
#define HTTP2_DBG(level, message, ...)
#endif /* HTTP2_DEBUG */
#endif /* HTTP2_DEBUG_H */
