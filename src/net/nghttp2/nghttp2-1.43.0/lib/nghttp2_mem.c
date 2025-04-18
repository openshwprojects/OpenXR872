/*
 * nghttp2 - HTTP/2 C Library
 *
 * Copyright (c) 2014 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "nghttp2_mem.h"
#include "nghttp2/http2_decl.h"

static void *default_malloc(size_t size, void *mem_user_data) {
  (void)mem_user_data;

#ifdef XR_NGHTTP2_DECL
  return http2_malloc(size);
#else
  return malloc(size);
#endif
}

static void default_free(void *ptr, void *mem_user_data) {
  (void)mem_user_data;

#ifdef XR_NGHTTP2_DECL
  http2_free(ptr);
#else
  free(ptr);
#endif
}

static void *default_calloc(size_t nmemb, size_t size, void *mem_user_data) {
  (void)mem_user_data;

#ifdef XR_NGHTTP2_DECL
  return http2_calloc(nmemb, size);
#else
  return calloc(nmemb, size);
#endif
}

static void *default_realloc(void *ptr, size_t size, void *mem_user_data) {
  (void)mem_user_data;

#ifdef XR_NGHTTP2_DECL
  return http2_realloc(ptr, size);
#else
  return realloc(ptr, size);
#endif
}

static nghttp2_mem mem_default = {NULL, default_malloc, default_free,
                                  default_calloc, default_realloc};

nghttp2_mem *nghttp2_mem_default(void) { return &mem_default; }

void *nghttp2_mem_malloc(nghttp2_mem *mem, size_t size) {
  return mem->malloc(size, mem->mem_user_data);
}

void nghttp2_mem_free(nghttp2_mem *mem, void *ptr) {
  mem->free(ptr, mem->mem_user_data);
}

void nghttp2_mem_free2(nghttp2_free free_func, void *ptr, void *mem_user_data) {
  free_func(ptr, mem_user_data);
}

void *nghttp2_mem_calloc(nghttp2_mem *mem, size_t nmemb, size_t size) {
  return mem->calloc(nmemb, size, mem->mem_user_data);
}

void *nghttp2_mem_realloc(nghttp2_mem *mem, void *ptr, size_t size) {
  return mem->realloc(ptr, size, mem->mem_user_data);
}
