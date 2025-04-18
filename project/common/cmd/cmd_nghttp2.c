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

#if PRJCONF_NET_EN
#ifdef __CONFIG_NGHTTP2

#include "cmd_util.h"
#include "net/nghttp2/http2_api.h"

/* use for two-phase verification */
static const char nghttp2_client_cert[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIC/DCCAeQCFFk4HVa42niSzTZbbPwh49UZW3rHMA0GCSqGSIb3DQEBBQUAMDIx\r\n"
"MDAuBgNVBAoMJ1RMUyBQcm9qZWN0IERvZGd5IENlcnRpZmljYXRlIEF1dGhvcml0\r\n"
"eTAeFw0xOTA3MTUwNjMzMzhaFw0zMzAzMjMwNjMzMzhaMEMxJzAlBgNVBAoMHlRM\r\n"
"UyBQcm9qZWN0IERldmljZSBDZXJ0aWZpY2F0ZTEYMBYGA1UEAwwPMTkyLjE2OC4x\r\n"
"MTEuMTAxMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxY05JdapYr6B\r\n"
"ohID4gFxh5ANvqGtTqTdfmeZSQIPzA3b6kTHF+HbMw0FGaKCd2Minah4IlWHq5mE\r\n"
"9jpggj3UxqEcdmCDzBV2bbj+ZA8XdtB7oqeb+6Bt+Gqxi83RDEQpDI/nwwWrGahu\r\n"
"6YXF+PXlJQC3YasW/JrQDJY9VMf1nyGf3FN5TJQ4HJ6eHWsXhwNUCSwE6e4ol+V6\r\n"
"GO2coFukOmQDQ2KgK7MYMKF9YTbusvaSIx1AbmJCfszoudUolbnlVIYIh98g2z7X\r\n"
"p1ki0Vtr0UnrK3qBYVlPXWq+d4hbtgL9rNoR4mSrp9k4QvdChwkZ4szblwkMZ7Xf\r\n"
"wB2qBYO2LwIDAQABMA0GCSqGSIb3DQEBBQUAA4IBAQAl0ui4I0cCtZ9q1el7ACS4\r\n"
"y9ph8aD0WsosQlHl6sjIQOMIU0dH9xSyELaST7UzGauOXx4sWnxWb6NhtB1huC0l\r\n"
"eRZgHqnHMqioxDTb2LPB1rA/B4ntzVkOBzoi44xuL3LFhQap8UMm+1oYJi+WSGsF\r\n"
"3qwIkju1VmL6b85P6xKBoC8N9jLKGVVUFBZ0gCUT+vCsb8fcoiQWqAGXkrz1RWEp\r\n"
"9CEFH1AQYHiRlZOb0lIPizI5Zo29GsPWbj0yai4/vI+q7uYs67Q3Yjd6V8tSu8AC\r\n"
"npskXlMqLl76F1SkD4SEBj57eB40Llm7WSfXPx182naZdHzsKpFzDkjod6MlCWCS\r\n"
"-----END CERTIFICATE-----\r\n";

/* use for two-phase verification */
static const char nghttp2_client_key[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEpAIBAAKCAQEAxY05JdapYr6BohID4gFxh5ANvqGtTqTdfmeZSQIPzA3b6kTH\r\n"
"F+HbMw0FGaKCd2Minah4IlWHq5mE9jpggj3UxqEcdmCDzBV2bbj+ZA8XdtB7oqeb\r\n"
"+6Bt+Gqxi83RDEQpDI/nwwWrGahu6YXF+PXlJQC3YasW/JrQDJY9VMf1nyGf3FN5\r\n"
"TJQ4HJ6eHWsXhwNUCSwE6e4ol+V6GO2coFukOmQDQ2KgK7MYMKF9YTbusvaSIx1A\r\n"
"bmJCfszoudUolbnlVIYIh98g2z7Xp1ki0Vtr0UnrK3qBYVlPXWq+d4hbtgL9rNoR\r\n"
"4mSrp9k4QvdChwkZ4szblwkMZ7XfwB2qBYO2LwIDAQABAoIBAQCtbkjwdh7oqHTo\r\n"
"EsbD4B6KM6ZNaGTcuRIWyd6hYKT7sGMTrOPYjJjCnbiPg8LkCu012dP12H6t4K4A\r\n"
"+MkHLj8hTgnNxveN8H2y4Ai9UR55WZhg+KKQ6owA3bIXGU5gZWpgM+n0pYJLmTod\r\n"
"2yotYbqUnKdhoDEi/MqPckpPpuh0lSVXOGBf88Cl1A3WGqhPn/fvMAGAMEuayCU9\r\n"
"NxsK9K05OBox9+CuJX9Gfhx0vcC4cB0HQfLzpAOe5j6W/FhYb39sZCalNSx/9sKt\r\n"
"uxTGiiQrmPJ9mGtY6x0ksPvmZGIKmh6YdNIZ+MQlE5iDf1mYr4SuzuLBpuU6rDEI\r\n"
"1zKV6Ih5AoGBAPL5cNV9QZNSGg/bS5hBzKnyOdubSg0nkFXPA2lqfz2uwQ9c0DYl\r\n"
"5TF2JExGy7YpUWiJCuXjmcNO6ck5QTo8K3M17zikcthOxEtM0cZDHnSWcWLz+EiO\r\n"
"XiK9VMyJZqb2ZivEHPyzjxffE6rvPVN/vYMaKt5Mdw6Jzh8cGPzZjIqFAoGBANAk\r\n"
"Z6rXzrEwbNv9YGiXFj9gnbLTcfVLDlropL9Hf/dxWP8PQxBeyvZ3XfvFxpWI/MU5\r\n"
"hdj3c+AvkB//5AUcHU0K5vE8Sv+M2e7bsFxyMYuVybXlpvqwt9zOpzw5x7xlreRE\r\n"
"/xVpKbUO3YKCjrqeOsJtVOeEhYwK4hAcygD/y44jAoGAAKYX3goSlcEfXrF4NzTd\r\n"
"xgpmiyaUAQr9AK2n1a06H8EKtO7Lg4mAXixxll6OBrN/iybqh4ifDX11dFsZyH0G\r\n"
"pK0dMWqG//rd3VGcMcpWF3ubW+dI33C54Z/dzRoE0ydPSIiihy43kJnA5LD02fc6\r\n"
"W1JDkQplOv21NjIOAwbVsD0CgYBeG0efz8kNBecCI/I197HAX++NDdrlW9UWtz7d\r\n"
"mPc7qkzhrUXWHfXIL7oXfplFvNUEWviwW1lR0E9qmGjBArAgyEAYa/lAx681NrDr\r\n"
"a1oJUWUMz9OKXuISfIDSUxGClbpkjemDBbQsv5bZTiw4JhhNFd+geaNj6PvC6zFN\r\n"
"+FlRXwKBgQDBWxw6FPL/q+AP+B+RNde9pueVHQFx0hZM57K7jjddoVOTo1f98TM/\r\n"
"51b2QXR65Rl08c4tkvDqtf3jT3LeRsvgazfXNyDTp0C2BQ+ocXhyvQ2Har8z+ujs\r\n"
"0h9L+vHw1lUzfFrda7igD2XZDvJEgOlTQ4FQRcTxRSu7A5QSi8WQOg==\r\n"
"-----END RSA PRIVATE KEY-----\r\n";

#define CMD_NGHTTP2_MSG_EXIT        (0)
#define CMD_NGHTTP2_MSG_OPEN        (1)
#define CMD_NGHTTP2_MSG_REQ         (2)
#define CMD_NGHTTP2_MSG_CLOSE       (3)

#define CMD_NGHTTP2_REQ_GET         (1)
#define CMD_NGHTTP2_REQ_PUT         (2)
#define CMD_NGHTTP2_REQ_POST        (3)
#define CMD_NGHTTP2_REQ_INVALID     (0)

struct cmd_nghttp2_open_data {
	uint32_t tls;
	uint32_t verify;
	uint32_t cer;
	int      timeout_msec;
	char     server_host[32];
	char     server_port[8];
};

struct cmd_nghttp2_send_request {
	uint8_t  type;
	char     path[100];
	char     putpost_data[100];
};

struct cmd_nghttp2_msg {
	uint32_t    type;
	void       *data;
};

#define CMD_NGHTTP2_QUEUE_WAIT_TIME        (5000)
#define CMD_NGHTTP2_THREAD_STACK_SIZE      (8 * 1024)
#define CMD_NGHTTP2_EXEC_THREAD_STACK_SIZE (2 * 1024)

static OS_Thread_t g_nghttp2_client_thread;
static OS_Queue_t g_client_queue;

static struct http2_conn *g_conn = NULL;
static char putpost_data_buf[100];
static char putpost_data_len = 0;

static int get_response_cb(struct http2_conn *conn, const char *data, size_t len, int flags)
{
	if (len)
		printf("[get-response] %.*s\n", len, data);
	if (flags == DATA_RECV_FRAME_COMPLETE)
		printf("[get-response] Frame fully received\n");
	if (flags == DATA_RECV_RST_STREAM)
		printf("[get-response] Stream Closed\n");
	return 0;
}

static int put_response_cb(struct http2_conn *conn, const char *data, size_t len, int flags)
{
	if (len)
		printf("[put-response] %.*s\n", len, data);
	if (flags == DATA_RECV_FRAME_COMPLETE)
		printf("[put-response] Frame fully received\n");
	if (flags == DATA_RECV_RST_STREAM)
		printf("[put-response] Stream Closed\n");
	return 0;
}

static int post_response_cb(struct http2_conn *conn, const char *data, size_t len, int flags)
{
	if (len)
		printf("[post-response] %.*s\n", len, data);
	if (flags == DATA_RECV_FRAME_COMPLETE)
		printf("[post-response] Frame fully received\n");
	if (flags == DATA_RECV_RST_STREAM)
		printf("[post-response] Stream Closed\n");
	return 0;
}

static int send_putpost_data(struct http2_conn *conn, char *data, size_t len, uint32_t *data_flags)
{
	int copylen = putpost_data_len;
	if (copylen < len) {
		printf("[putpost-data] Sending %d bytes\n", copylen);
		memcpy(data, putpost_data_buf, copylen);
	} else {
		copylen = 0;
	}

	(*data_flags) |= NGHTTP2_DATA_FLAG_EOF;
	return copylen;
}

static void urlcat(char *url, const char *host, const char *port)
{
	int pos = 0;
	/* hostname */
	cmd_memcpy(url, host, strlen(host));
	pos += strlen(host);
	/* : */
	url[pos] = ':';
	pos += 1;
	/* port */
	cmd_memcpy(url + pos, port, strlen(port) + 1);
}

static void cmd_nghttp2_client_open_task(struct cmd_nghttp2_open_data *data)
{
	char url[40] = {0};
	urlcat(url, data->server_host, data->server_port);
	CMD_DBG("urlcat : [%s]\n", url);

	if (g_conn != NULL) {
		CMD_ERR("already open. To open again, need to close first\n");
		goto open_task_exit;
	}

	if (HTTP2_ERR_OK != http2_conn_new(&g_conn)) {
		CMD_ERR("http2_conn_new fail\r\n");
		goto open_task_exit;
	}

	http2_set_timeout(g_conn, data->timeout_msec);

	if (data->tls == 0) {
		CMD_ERR("do not support h2c now!\n");
		goto open_task_exit;
	} else {
		if (data->cer != 0) {
			if (HTTP2_ERR_OK != http2_set_ssl_certs(g_conn,
			               nghttp2_client_cert, sizeof(nghttp2_client_cert),
			               nghttp2_client_key, sizeof(nghttp2_client_key),
			               NULL, 0, NULL, 0)) {
				CMD_ERR("http2_set_ssl_certs fail\n");
				goto open_task_exit;
			}
		}

		if (data->verify == 0)
			http2_set_ssl_verify(g_conn, HTTP2_FALSE);
		else
			http2_set_ssl_verify(g_conn, HTTP2_TRUE);

		CMD_DBG("http2 start to connect server...\n");
		if (HTTP2_ERR_OK != http2_connect(g_conn, url, HTTP2_TRUE)) {
			CMD_ERR("http2_connect fail\n");
			goto open_task_exit;
		}
		CMD_DBG("http2 connected successfully\n");
	}

	if (HTTP2_ERR_OK != http2_execute_task_start(g_conn, CMD_NGHTTP2_EXEC_THREAD_STACK_SIZE)) {
		CMD_ERR("http2_execute_task_start fail\n");
	}
	CMD_DBG("nghttp2 execute task is started.\n");

open_task_exit:
	cmd_free(data);
	data = NULL;
	return;
}

static void cmd_nghttp2_client_request_task(struct cmd_nghttp2_send_request *req)
{
	if (g_conn == NULL) {
		CMD_ERR("without open\n");
		goto request_task_exit;
	}

	switch (req->type) {
	case CMD_NGHTTP2_REQ_GET:
		if (HTTP2_ERR_OK != http2_get(g_conn, req->path, get_response_cb)) {
			CMD_ERR("http2 req-get fail\n");
		}
		break;
	case CMD_NGHTTP2_REQ_PUT:
		putpost_data_len = cmd_strlen(req->putpost_data);
		cmd_memcpy(putpost_data_buf, req->putpost_data, putpost_data_len);
		if (HTTP2_ERR_OK != http2_put(g_conn, req->path, send_putpost_data, put_response_cb)) {
			CMD_ERR("http2 req-put fail\n");
		}
		break;
	case CMD_NGHTTP2_REQ_POST:
		putpost_data_len = cmd_strlen(req->putpost_data);
		cmd_memcpy(putpost_data_buf, req->putpost_data, putpost_data_len);
		if (HTTP2_ERR_OK != http2_post(g_conn, req->path, send_putpost_data, post_response_cb)) {
			CMD_ERR("http2 req-post fail\n");
		}
		break;
	default:
		CMD_ERR("req type is invaild\n");
		break;
	}

request_task_exit:
	cmd_free(req);
	req = NULL;
	return;
}

static void cmd_nghttp2_client_close_task(void *data)
{
	http2_execute_task_stop(g_conn);
	http2_conn_free(g_conn);
	g_conn = NULL;
	printf("http2 connection is closed\n");

	return;
}

static void cmd_nghttp2_client_task(void *arg)
{
	uint8_t task_exit = 0;
	struct cmd_nghttp2_msg *msg;

	while (task_exit == 0) {

		if (OS_MsgQueueReceive(&g_client_queue, (void **)&msg, OS_WAIT_FOREVER) != OS_OK) {
			CMD_ERR("msg queue receive failed\n");
			break;
		}
		//CMD_DBG("recv msg type %u\n", msg->type);

		switch (msg->type) {
		case CMD_NGHTTP2_MSG_EXIT:
			task_exit = 1;
			break;
		case CMD_NGHTTP2_MSG_OPEN:
			cmd_nghttp2_client_open_task(msg->data);
			break;
		case CMD_NGHTTP2_MSG_REQ:
			cmd_nghttp2_client_request_task(msg->data);
			break;
		case CMD_NGHTTP2_MSG_CLOSE:
			cmd_nghttp2_client_close_task(msg->data);
			break;
		default:
			CMD_WRN("unknown msg\n");
			break;
		}

		cmd_free(msg);
		msg = NULL;
	}

	CMD_DBG("%s() end\n", __func__);
	OS_ThreadDelete(&g_nghttp2_client_thread);
}

static enum cmd_status cmd_nghttp2_client_init_exec(char *cmd)
{
	OS_ThreadSetInvalid(&g_nghttp2_client_thread);

	if (OS_MsgQueueCreate(&g_client_queue, 1) != OS_OK) {
		CMD_ERR("msg queue create failed\n");
		return CMD_STATUS_FAIL;
	}

	if (OS_ThreadCreate(&g_nghttp2_client_thread,
	                    "cmd_nghttp2_cli",
	                    cmd_nghttp2_client_task,
	                    NULL,
	                    OS_THREAD_PRIO_CONSOLE,
	                    CMD_NGHTTP2_THREAD_STACK_SIZE) != OS_OK) {
		CMD_ERR("thread create failed\n");
		return CMD_STATUS_FAIL;
	}

	return CMD_STATUS_OK;
}

static enum cmd_status cmd_nghttp2_client_open_exec(char *cmd)
{
	int ret;
	int cnt;
	struct cmd_nghttp2_open_data *data = NULL;
	struct cmd_nghttp2_msg *msg = NULL;

	msg = cmd_malloc(sizeof(struct cmd_nghttp2_msg));
	data = cmd_malloc(sizeof(struct cmd_nghttp2_open_data));
	if ((msg == NULL) || (data == NULL)) {
		CMD_ERR("malloc failed\n");
		ret = CMD_STATUS_FAIL;
		goto open_exec_fail;
	}

	cnt = cmd_sscanf(cmd, "h=%s p=%s t=%u v=%u c=%u o=%u",
	                 data->server_host, data->server_port, &data->tls,
	                 &data->verify, &data->cer, &data->timeout_msec);
	if (cnt != 6) {
		CMD_ERR("cmd_sscanf return: cnt = %d\n", cnt);
		ret = CMD_STATUS_INVALID_ARG;
		goto open_exec_fail;
	}

	if (g_conn != NULL) {
		CMD_ERR("already open. To open again, need to close first\n");
		ret = CMD_STATUS_FAIL;
		goto open_exec_fail;
	}

	msg->type = CMD_NGHTTP2_MSG_OPEN;
	msg->data = data;

	if (OS_MsgQueueSend(&g_client_queue, msg, CMD_NGHTTP2_QUEUE_WAIT_TIME) != OS_OK) {
		CMD_ERR("msg queue send failed\n");
		ret = CMD_STATUS_FAIL;
		goto open_exec_fail;
	}

	return CMD_STATUS_OK;

open_exec_fail:
	if (msg)
		cmd_free(msg);
	if (data)
		cmd_free(data);

	return ret;
}

static enum cmd_status cmd_nghttp2_client_req_exec(char *cmd)
{
	int cnt;
	int ret;
	uint8_t req_type[16];
	struct cmd_nghttp2_send_request *req;
	struct cmd_nghttp2_msg *msg;

	msg = cmd_malloc(sizeof(struct cmd_nghttp2_msg));
	req = cmd_malloc(sizeof(struct cmd_nghttp2_send_request));
	if ((msg == NULL) || (req == NULL)) {
		CMD_ERR("malloc failed\n");
		ret = CMD_STATUS_FAIL;
		goto req_exec_fail;
	}

	cnt = cmd_sscanf(cmd, "m=%s p=%s d=%s", req_type, req->path, req->putpost_data);
	if (cnt != 3) {
		CMD_ERR("cmd_sscanf return: cnt = %d\n", cnt);
		ret = CMD_STATUS_INVALID_ARG;
		goto req_exec_fail;
	}

	if (g_conn == NULL) {
		CMD_ERR("without open\n");
		ret = CMD_STATUS_FAIL;
		goto req_exec_fail;
	}

	if (req_type[0] == 'g' && req_type[1] == 'e' && req_type[2] == 't') {
		req->type = CMD_NGHTTP2_REQ_GET;
	} else if (req_type[0] == 'p' && req_type[1] == 'u' && req_type[2] == 't') {
		req->type = CMD_NGHTTP2_REQ_PUT;
	} else if (req_type[0] == 'p' && req_type[1] == 'o' && req_type[2] == 's' && req_type[3] == 't') {
		req->type = CMD_NGHTTP2_REQ_POST;
	} else {
		req->type = CMD_NGHTTP2_REQ_INVALID;
	}

	msg->type = CMD_NGHTTP2_MSG_REQ;
	msg->data = req;

	if (OS_MsgQueueSend(&g_client_queue, msg, CMD_NGHTTP2_QUEUE_WAIT_TIME) != OS_OK) {
		CMD_ERR("msg queue send failed\n");
		ret = CMD_STATUS_ACKED;
		goto req_exec_fail;
	}

	return CMD_STATUS_OK;

req_exec_fail:
	if (msg)
		cmd_free(msg);
	if (req)
		cmd_free(req);

	return ret;
}

static enum cmd_status cmd_nghttp2_client_close_exec(char *cmd)
{
	struct cmd_nghttp2_msg *msg;

	if (g_conn == NULL) {
		CMD_ERR("without open\n");
		return CMD_STATUS_FAIL;
	}

	msg = cmd_malloc(sizeof(struct cmd_nghttp2_msg));
	if (msg == NULL) {
		CMD_ERR("msg queue malloc failed\n");
		return CMD_STATUS_FAIL;
	}

	msg->type = CMD_NGHTTP2_MSG_CLOSE;
	msg->data = NULL;

	if (OS_MsgQueueSend(&g_client_queue, msg, CMD_NGHTTP2_QUEUE_WAIT_TIME) != OS_OK) {
		CMD_ERR("msg queue send failed\n");
		if (msg)
			cmd_free(msg);
		return CMD_STATUS_FAIL;
	}

	return CMD_STATUS_OK;
}

static enum cmd_status cmd_nghttp2_client_deinit_exec(char *cmd)
{
	struct cmd_nghttp2_msg *msg;

	if (g_conn != NULL) {
		CMD_ERR("without close\n");
		return CMD_STATUS_FAIL;
	}

	msg = cmd_malloc(sizeof(struct cmd_nghttp2_msg));
	if (msg == NULL) {
		CMD_ERR("msg queue malloc failed\n");
		return CMD_STATUS_FAIL;
	}

	msg->type = CMD_NGHTTP2_MSG_EXIT;
	msg->data = NULL;

	if (OS_MsgQueueSend(&g_client_queue, msg, CMD_NGHTTP2_QUEUE_WAIT_TIME) != OS_OK) {
		CMD_ERR("msg queue send failed\n");
		if (msg)
			cmd_free(msg);
		return CMD_STATUS_FAIL;
	}

	while (OS_ThreadIsValid(&g_nghttp2_client_thread))
		OS_MSleep(1);

	if (OS_MsgQueueDelete(&g_client_queue) != OS_OK) {
		CMD_ERR("msg queue delete failed\n");
		return CMD_STATUS_FAIL;
	}

	return CMD_STATUS_OK;
}

#if CMD_DESCRIBE
#define h2_cli_init_help_info "init"
#define h2_cli_open_help_info \
"open h=<server_host> p=<server_port> t=<tls> v=<verify> c=<cert> o=<timeout>, "\
"open the client and connect the server, "\
"such as [open h=https://nghttp2.golang.org p=443 t=1 v=0 c=1 o=15000]"
#define h2_cli_req_help_info \
"req m=<method>, p=<path>, d=<data>, "\
"send http request to server, "\
"such as [req m=get p=/clockstream d=null], [req m=put p=/ECHO d=hello]"
#define h2_cli_close_help_info "disconnet from server"
#define h2_cli_deinit_help_info "deinit the client"
#endif

static enum cmd_status cmd_nghttp2_client_help_exec(char *cmd);

static const struct cmd_data g_nghttp2_client_cmds[] = {
	{ "init",   cmd_nghttp2_client_init_exec,   CMD_DESC(h2_cli_init_help_info) },
	{ "open",   cmd_nghttp2_client_open_exec,   CMD_DESC(h2_cli_open_help_info) },
	{ "req",    cmd_nghttp2_client_req_exec,    CMD_DESC(h2_cli_req_help_info) },
	{ "close",  cmd_nghttp2_client_close_exec,  CMD_DESC(h2_cli_close_help_info) },
	{ "deinit", cmd_nghttp2_client_deinit_exec, CMD_DESC(h2_cli_deinit_help_info) },
	{ "help",   cmd_nghttp2_client_help_exec,   CMD_DESC(CMD_HELP_DESC) },
};

static enum cmd_status cmd_nghttp2_client_help_exec(char *cmd)
{
	return cmd_help_exec(g_nghttp2_client_cmds, cmd_nitems(g_nghttp2_client_cmds), 8);
}

static enum cmd_status cmd_nghttp2_client_exec(char *cmd)
{
	return cmd_exec(cmd, g_nghttp2_client_cmds, cmd_nitems(g_nghttp2_client_cmds));
}

static enum cmd_status cmd_nghttp2_help_exec(char *cmd);

static const struct cmd_data g_nghttp2_cmds[] = {
	{ "cli",    cmd_nghttp2_client_exec, CMD_DESC("nghttp2 client command") },
	{ "help",   cmd_nghttp2_help_exec,   CMD_DESC(CMD_HELP_DESC) },
};

static enum cmd_status cmd_nghttp2_help_exec(char *cmd)
{
	return cmd_help_exec(g_nghttp2_cmds, cmd_nitems(g_nghttp2_cmds), 8);
}

enum cmd_status cmd_nghttp2_exec(char *cmd)
{
	return cmd_exec(cmd, g_nghttp2_cmds, cmd_nitems(g_nghttp2_cmds));
}

#endif /* __CONFIG_NGHTTP2 */
#endif /* PRJCONF_NET_EN */
