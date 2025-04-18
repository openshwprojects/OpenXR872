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

#include "nghttp2/http2_decl.h"

#ifdef XR_NGHTTP2_DECL
http2_ptr http2_malloc(size_t size)
{
#if (__CONFIG_NGHTTP2_HEAP_MODE == 1)
	return psram_malloc(size);
#else
	return malloc(size);
#endif
}

http2_ptr http2_calloc(size_t count, size_t size)
{
#if (__CONFIG_NGHTTP2_HEAP_MODE == 1)
	return psram_calloc(count, size);
#else
	return calloc(count, size);
#endif
}

http2_ptr http2_realloc(http2_ptr ref, size_t size)
{
#if (__CONFIG_NGHTTP2_HEAP_MODE == 1)
	return psram_realloc(ref, size);
#else
	return realloc(ref, size);
#endif
}

void http2_free(http2_ptr ref)
{
#if (__CONFIG_NGHTTP2_HEAP_MODE == 1)
	psram_free(ref);
#else
	free(ref);
#endif
	return;
}
#endif /* XR_NGHTTP2_DECL */
