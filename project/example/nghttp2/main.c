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

#include <stdio.h>
#include "kernel/os/os.h"
#include "common/framework/platform_init.h"
#include "net/wlan/wlan.h"
#include "common/framework/net_ctrl.h"
#include "nghttp2/http2_api.h"
#include "time.h"

#define MULTI_CONNECT_TEST 0

#define HTTP2_DEMO_THREAD_STACK_SIZE (8 * 1024) /* ssl need more stack */
#define HTTP2_EXEC_THREAD_STACK_SIZE (2 * 1024)
static OS_Thread_t http2_demo_thread;
#if MULTI_CONNECT_TEST
static OS_Thread_t http2_demo_thread_1;
#endif

/* The HTTP/2 server to connect to */
#define HTTP2_SERVER_URI          "https://http2.golang.org"
/* A GET request that keeps streaming current time every second */
#define HTTP2_STREAMING_GET_PATH  "/clockstream"
/* A PUT request that echoes whatever we had sent to it */
#define HTTP2_PUT_PATH            "/ECHO"

static int get_response_cb(struct http2_conn *conn, const char *data, size_t len, int flags)
{
	if (len) {
		printf("[get-response] %.*s\n", len, data);
	}
	if (flags == DATA_RECV_FRAME_COMPLETE) {
		printf("[get-response] Frame fully received\n");
	}
	if (flags == DATA_RECV_RST_STREAM) {
		printf("[get-response] Stream Closed\n");
	}
	return 0;
}

static int put_response_cb(struct http2_conn *conn, const char *data, size_t len, int flags)
{
	if (len) {
		printf("[put-response] %.*s\n", len, data);
	}
	if (flags == DATA_RECV_FRAME_COMPLETE) {
		printf("[put-response] Frame fully received\n");
	}
	if (flags == DATA_RECV_RST_STREAM) {
		printf("[put-response] Stream Closed\n");
	}
	return 0;
}

static char put_data_buf[100];
static char put_data_len = 0;

static int send_put_data(struct http2_conn *conn, char *data, size_t len, uint32_t *data_flags)
{
	int copylen = put_data_len;
	if (copylen < len) {
		printf("[put-data] Sending %d bytes\n", copylen);
		memcpy(data, put_data_buf, copylen);
	} else {
		copylen = 0;
	}

	(*data_flags) |= NGHTTP2_DATA_FLAG_EOF;
	return copylen;
}

static void http2_demo_loop(void)
{
	const char PUT_STRING[] = "Hello World!!!";
	struct http2_conn *conn = NULL;
	if (HTTP2_ERR_OK != http2_conn_new(&conn)) {
		printf("http2 conn new fail\n");
		return;
	}

	if (HTTP2_ERR_OK != http2_set_ssl_certs(conn, NULL, 0, NULL, 0, NULL, 0, NULL, 0)) {
		printf("http2 set certificates fail\n");
		return;
	}
	http2_set_timeout(conn, 15000);

	printf("http2 start connect to server...\n");
	if (HTTP2_ERR_OK != http2_connect(conn, HTTP2_SERVER_URI, HTTP2_TRUE)) {
		printf("http2 connect fail\n");
		return;
	}
	printf("http2 connected successfully\n");
	if (HTTP2_ERR_OK != http2_execute_task_start(conn, HTTP2_EXEC_THREAD_STACK_SIZE)) {
		printf("http2_execute_task_start fail\n");
	}

	printf("\nDo HTTP2 PUT %s\n", HTTP2_PUT_PATH);
	put_data_len = strlen(PUT_STRING) + 1;
	memcpy(put_data_buf, PUT_STRING, put_data_len);
	if (HTTP2_ERR_OK != http2_put(conn, HTTP2_PUT_PATH, send_put_data, put_response_cb)) {
		printf("http2 put fail\n");
	}
	OS_Sleep(5);

	printf("\nDo HTTP2 GET %s\n", HTTP2_STREAMING_GET_PATH);
	if (HTTP2_ERR_OK != http2_get(conn, HTTP2_STREAMING_GET_PATH, get_response_cb)) {
		printf("http2 get fail\n");
	}
	OS_Sleep(10);

	printf("\nStop http2 task\n");
	http2_execute_task_stop(conn);
	http2_conn_free(conn);
}

static void http2_demo_fun(void *arg)
{
	http2_demo_loop();

	OS_ThreadDelete(&http2_demo_thread);
}

#if MULTI_CONNECT_TEST
static void http2_demo_fun_1(void *arg)
{
	http2_demo_loop();

	OS_ThreadDelete(&http2_demo_thread_1);
}
#endif

static void net_cb(uint32_t event, uint32_t data, void *arg)
{
	uint16_t type = EVENT_SUBTYPE(event);

	switch (type) {
	case NET_CTRL_MSG_NETWORK_UP:
		if (!OS_ThreadIsValid(&http2_demo_thread)) {
			OS_ThreadCreate(&http2_demo_thread,
			                "http2_demo_thread",
			                http2_demo_fun,
			                (void *)NULL,
			                OS_THREAD_PRIO_APP,
			                HTTP2_DEMO_THREAD_STACK_SIZE);
		}
#if MULTI_CONNECT_TEST
		if (!OS_ThreadIsValid(&http2_demo_thread_1)) {
			OS_ThreadCreate(&http2_demo_thread_1,
			                "http2_demo_thread_1",
			                http2_demo_fun_1,
			                (void *)NULL,
			                OS_THREAD_PRIO_APP,
			                HTTP2_DEMO_THREAD_STACK_SIZE);
		}
#endif
		break;

	case NET_CTRL_MSG_NETWORK_DOWN:
		break;

	default:
		break;
	}

}

int main(void)
{
	observer_base *net_ob;

	platform_init();

	printf("http2 demo start\n\n");

	printf("use these commands to connect ap:\n\n");
	printf("1. config ssid and password : net sta config ssid password\n");
	printf("2. enable sta to connect ap : net sta enable\n\n");

	/* create an observer to monitor the net work state */
	net_ob = sys_callback_observer_create(CTRL_MSG_TYPE_NETWORK,
	                                      NET_CTRL_MSG_ALL,
	                                      net_cb,
	                                      NULL);
	if (net_ob == NULL)
		return -1;

	if (sys_ctrl_attach(net_ob) != 0)
		return -1;

	return 0;
}
