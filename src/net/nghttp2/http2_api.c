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
#include "url-parser/url_parser.h"
#include "nghttp2/http2_debug.h"
#include "nghttp2/http2_api.h"
#include "kernel/os/os_errno.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct http2_node_manager g_node_manager;
static http2_bool node_manager_init_flag = HTTP2_FALSE;
static http2_mutex_t dns_mutex;

static struct http2_node *http2_find_node_from_manager(struct http2_node_manager *manager,
                                                       struct http2_conn *conn);
static void http2_node_remove_from_manager(struct http2_node_manager *manager,
                                           struct http2_node *node);
static http2_err_t http2_node_stop(struct http2_node *node);
static void http2_node_free(struct http2_node *node);

static ssize_t http2_callback_try_send(struct http2_conn *conn,
                            const uint8_t *data, size_t length)
{
	int ret = conn->output(conn, data, length);
	if (ret <= 0) {
		if (conn->enable_tls) {
#ifdef HTTP2_MBEDTLS
			if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
				ret = NGHTTP2_ERR_WOULDBLOCK;
			else
				ret = NGHTTP2_ERR_CALLBACK_FAILURE;
#else
			/* TODO: openssl or something else */
#endif
		} else {
			if (errno == EWOULDBLOCK)
				ret = NGHTTP2_ERR_WOULDBLOCK;
			else
				ret = NGHTTP2_ERR_CALLBACK_FAILURE;
		}
	}
	return ret;
}

static ssize_t http2_callback_send(nghttp2_session *session, const uint8_t *data,
                                   size_t length, int flags, void *user_data)
{
	int rv = 0;
	struct http2_conn *conn = user_data;

	int copy_offset = 0;
	int pending_data = length;

	/* Send data in 1000 byte chunks */
	while (copy_offset != length) {
		int chunk_len = pending_data > 1000 ? 1000 : pending_data;
		int subrv = http2_callback_try_send(conn, data + copy_offset, chunk_len);
		if (subrv <= 0) {
			if (copy_offset == 0) {
				/* If no data is transferred, send the error code */
				rv = subrv;
			}
			break;
		}
		copy_offset += subrv;
		pending_data -= subrv;
		rv += subrv;
	}
	return rv;
}

static ssize_t http2_callback_recv(nghttp2_session *session, uint8_t *buf,
                                   size_t length, int flags, void *user_data)
{
	struct http2_conn *conn = user_data;
	int ret = conn->input(conn, (char *)buf, (int)length);
	if (conn->enable_tls) {
#ifdef HTTP2_MBEDTLS
		if (ret < 0) {
			if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
				ret = NGHTTP2_ERR_WOULDBLOCK;
			} else {
				HTTP2_DBG(HTTP2_LEVEL_ERR, "tls conn->input fail, ret = %d\n", ret);
				ret = NGHTTP2_ERR_CALLBACK_FAILURE;
			}
		} else if (ret == 0) {
			HTTP2_DBG(HTTP2_LEVEL_INFO, "tls conn->input recv EOF, ret = %d\n", ret);
			ret = NGHTTP2_ERR_EOF;
		}
#else
		/* TODO: openssl or something else */
#endif
	} else {
		if (ret < 0) {
			if (errno == EWOULDBLOCK) {
				ret = NGHTTP2_ERR_WOULDBLOCK;
			} else {
				HTTP2_DBG(HTTP2_LEVEL_ERR, "tls conn->input fail, ret = %d\n", ret);
				ret = NGHTTP2_ERR_CALLBACK_FAILURE;
			}
		} else if (ret == 0) {
			HTTP2_DBG(HTTP2_LEVEL_INFO, "tls conn->input recv EOF, ret = %d\n", ret);
			ret = NGHTTP2_ERR_EOF;
		}

	}
	return ret;
}

static int http2_callback_on_frame_send(nghttp2_session *session,
                      const nghttp2_frame *frame, void *user_data)
{
	switch (frame->hd.type) {
	case NGHTTP2_HEADERS:
		if (nghttp2_session_get_stream_user_data(session, frame->hd.stream_id)) {
			HTTP2_DBG(HTTP2_LEVEL_INFO, "Frame send to Server\n");
			HTTP2_DBG(HTTP2_LEVEL_INFO, "Headers nvlen = %d\n", frame->headers.nvlen);
		}
		break;
	default:
		break;
	}
	return 0;
}

static int http2_callback_on_frame_recv(nghttp2_session *session,
                     const nghttp2_frame *frame, void *user_data)
{
	if (frame->hd.type != NGHTTP2_DATA) {
		return 0;
	}
	/* Subsequent processing only for data frame */
	http2_data_recv_cb data_recv_cb = nghttp2_session_get_stream_user_data(session, frame->hd.stream_id);
	if (data_recv_cb) {
		HTTP2_DBG(HTTP2_LEVEL_INFO, "Frame recv form Server, stream_id = %d\n", frame->hd.stream_id);
		struct http2_conn *conn = user_data;
		(*data_recv_cb)(conn, NULL, 0, DATA_RECV_FRAME_COMPLETE);
	}
	return 0;
}

static int http2_callback_on_stream_close(nghttp2_session *session, int32_t stream_id,
                                          uint32_t error_code, void *user_data)
{
	http2_data_recv_cb data_recv_cb = nghttp2_session_get_stream_user_data(session, stream_id);
	if (data_recv_cb) {
		HTTP2_DBG(HTTP2_LEVEL_INFO, "Stream close, stream_id = %d\n", stream_id);
		struct http2_conn *conn = user_data;
		(*data_recv_cb)(conn, NULL, 0, DATA_RECV_RST_STREAM);
	}
	return 0;
}

static int http2_callback_on_data_chunk_recv(nghttp2_session *session, uint8_t flags,
                int32_t stream_id, const uint8_t *data, size_t len, void *user_data)
{
	http2_data_recv_cb data_recv_cb = nghttp2_session_get_stream_user_data(session, stream_id);
	if (data_recv_cb) {
		struct http2_conn *conn = user_data;
		(*data_recv_cb)(conn, (char *)data, len, 0);
		/* TODO: What to do with the return value: look for pause/abort */
	}
	return 0;
}

static int http2_callback_on_header(nghttp2_session *session, const nghttp2_frame *frame,
                               const uint8_t *name, size_t namelen, const uint8_t *value,
                               size_t valuelen, uint8_t flags, void *user_data)
{
	return 0;
}

http2_err_t http2_execute(struct http2_conn *conn)
{
	int ret = 0;

	ret = nghttp2_session_send(conn->http2_session);
	if (ret != 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2_session_send fail, ret = %d\n", ret);
		return HTTP2_ERR_NGHTTP;
	}

	ret = nghttp2_session_recv(conn->http2_session);
	if (ret != 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2_session_recv fail, ret = %d\n", ret);
		return HTTP2_ERR_NGHTTP;
	}

	return HTTP2_ERR_OK;
}

http2_err_t http2_get(struct http2_conn *conn, const char *path, http2_data_recv_cb recv_cb)
{
	const nghttp2_nv nvarr[] = {
		XR_NGHTTP2_NV_NEW(":method", "GET"),
		XR_NGHTTP2_NV_NEW(":scheme", conn->enable_tls ? "https" : "http"),
		XR_NGHTTP2_NV_NEW(":authority", conn->hostname),
		XR_NGHTTP2_NV_NEW(":path", path)
	};
	size_t nvarr_len = sizeof(nvarr) / sizeof(nvarr[0]);
	HTTP2_DBG(HTTP2_LEVEL_INFO, "http2 %s <%s://%s%s>\n",
	        nvarr[0].value, nvarr[1].value, nvarr[2].value, nvarr[3].value);

	if (nghttp2_submit_request(conn->http2_session, NULL, nvarr, nvarr_len, NULL, recv_cb) < 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2_submit_request fail\n");
		return HTTP2_ERR_NGHTTP;
	}
	return HTTP2_ERR_OK;
}

static ssize_t http2_put_data_provider_cb(nghttp2_session *session, int32_t stream_id,
    uint8_t *buf, size_t length, uint32_t *data_flags, nghttp2_data_source *source, void *user_data)
{
	struct http2_conn *conn = user_data;
	http2_put_data_cb put_cb = source->ptr;
	return (*put_cb)(conn, (char *)buf, length, data_flags);
}

static ssize_t http2_post_data_provider_cb(nghttp2_session *session, int32_t stream_id,
    uint8_t *buf, size_t length, uint32_t *data_flags, nghttp2_data_source *source, void *user_data)
{
	struct http2_conn *conn = user_data;
	http2_post_data_cb post_cb = source->ptr;
	return (*post_cb)(conn, (char *)buf, length, data_flags);
}

http2_err_t http2_put(struct http2_conn *conn, const char *path,
                http2_put_data_cb put_cb, http2_data_recv_cb recv_cb)
{
	const nghttp2_nv nvarr[] = {
		XR_NGHTTP2_NV_NEW(":method", "PUT"),
		XR_NGHTTP2_NV_NEW(":scheme", conn->enable_tls ? "https" : "http"),
		XR_NGHTTP2_NV_NEW(":authority", conn->hostname),
		XR_NGHTTP2_NV_NEW(":path", path),
	};
	size_t nvarr_len = sizeof(nvarr) / sizeof(nghttp2_nv);
	HTTP2_DBG(HTTP2_LEVEL_INFO, "http2 %s <%s://%s%s>\n",
	        nvarr[0].value, nvarr[1].value, nvarr[2].value, nvarr[3].value);

	nghttp2_data_provider data_provider;
	data_provider.read_callback = http2_put_data_provider_cb;
	data_provider.source.ptr = put_cb;

	if (nghttp2_submit_request(conn->http2_session, NULL, nvarr, nvarr_len, &data_provider, recv_cb) < 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2_submit_request fail\n");
		return HTTP2_ERR_NGHTTP;
	}
	return HTTP2_ERR_OK;
}

http2_err_t http2_post(struct http2_conn *conn, const char *path,
                 http2_post_data_cb post_cb, http2_data_recv_cb recv_cb)
{
	const nghttp2_nv nvarr[] = {
		XR_NGHTTP2_NV_NEW(":method", "POST"),
		XR_NGHTTP2_NV_NEW(":scheme", conn->enable_tls ? "https" : "http"),
		XR_NGHTTP2_NV_NEW(":authority", conn->hostname),
		XR_NGHTTP2_NV_NEW(":path", path),
	};
	size_t nvarr_len = sizeof(nvarr) / sizeof(nghttp2_nv);
	HTTP2_DBG(HTTP2_LEVEL_INFO, "http2 %s <://%s%s%s>\n",
	        nvarr[0].value, nvarr[1].value, nvarr[2].value, nvarr[3].value);

	nghttp2_data_provider data_provider;
	data_provider.read_callback = http2_post_data_provider_cb;
	data_provider.source.ptr = post_cb;

	if (nghttp2_submit_request(conn->http2_session, NULL, nvarr, nvarr_len, &data_provider, recv_cb) < 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2_submit_request fail\n");
		return HTTP2_ERR_NGHTTP;
	}
	return HTTP2_ERR_OK;
}

static int http2_socket_read(struct http2_conn *conn, void *data, size_t datalen)
{
	return recv(conn->socket_fd, data, datalen, 0);
}

static int http2_socket_write(struct http2_conn *conn, const void *data, size_t datalen)
{
	return send(conn->socket_fd, data, datalen, 0);
}

static int http2_tls_read(struct http2_conn *conn, void *data, size_t datalen)
{
#ifdef HTTP2_MBEDTLS
	ssize_t ret = mbedtls_ssl_read(conn->ssl, (unsigned char *)data, datalen);
	if (ret < 0) {
		if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
			return 0;
		}
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls_ssl_read() error, ret = %d\n", ret);
		}
	}
	return ret;
#else
	/* TODO: openssl or something else */
#endif
}

static int http2_tls_write(struct http2_conn *conn, const void *data, size_t datalen)
{
#ifdef HTTP2_MBEDTLS
	size_t written = 0;
	size_t write_len = datalen;
	while (written < datalen) {
		if (write_len > MBEDTLS_SSL_OUT_CONTENT_LEN) {
			write_len = MBEDTLS_SSL_OUT_CONTENT_LEN;
		}
		if (datalen > MBEDTLS_SSL_OUT_CONTENT_LEN) {
			HTTP2_DBG(HTTP2_LEVEL_WAR,
			          "Fragmenting data of excessive size :%d, offset: %d, size %d",
			          datalen, written, write_len);
		}
		ssize_t ret = mbedtls_ssl_write(conn->ssl, (unsigned char *)data + written, write_len);
		if (ret <= 0) {
			if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
				ret != MBEDTLS_ERR_SSL_WANT_WRITE &&
				ret != 0) {
				HTTP2_DBG(HTTP2_LEVEL_ERR, "write error :%d:", ret);
				return ret;
			} else {
				HTTP2_DBG(HTTP2_LEVEL_INFO,
				          "mbedtls_ssl_write() returned %d, already written %d, exitting...",
				          ret, written);
				return written;
			}
		}
		written += ret;
		write_len = datalen - written;
	}
	return written;
#else
	/* TODO: openssl or something else */
#endif
}

static void http2_set_io(struct http2_conn *conn, http2_bool enable_tls)
{
	if (enable_tls) {
#ifdef HTTP2_MBEDTLS
		conn->input = http2_tls_read;
		conn->output = http2_tls_write;
#else
	/* TODO: openssl or something else */
#endif
	} else {
		conn->input = http2_socket_read;
		conn->output = http2_socket_write;
	}
}

static int __get_port(const char *url, struct http_parser_url *u)
{
	if (u->field_data[UF_PORT].len) {
		return strtol(&url[u->field_data[UF_PORT].off], NULL, 10);
	} else {
		if (strncasecmp(&url[u->field_data[UF_SCHEMA].off], "http", u->field_data[UF_SCHEMA].len) == 0) {
			return 80;
		} else if (strncasecmp(&url[u->field_data[UF_SCHEMA].off], "https", u->field_data[UF_SCHEMA].len) == 0) {
			return 443;
		}
	}
	return 0;
}

static http2_err_t __resolve_host_name(const char *host, size_t hostlen,
                                       struct addrinfo **address_info)
{
	int ret = 0;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	char *use_host = strndup(host, hostlen);
	if (!use_host) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "malloc use_host fail\n");
		return HTTP2_ERR_MEM;
	}
	HTTP2_DBG(HTTP2_LEVEL_INFO, "resolve hostname [%s]\n", use_host);

	ret = getaddrinfo(use_host, NULL, &hints, address_info);
	if (ret != 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "getaddrinfo fail, ret = %d\n", ret);
		free(use_host);
		return HTTP2_ERR_USE;
	}
	free(use_host);

	return HTTP2_ERR_OK;
}

static http2_err_t http2_connect_socket(struct http2_conn *conn,
                const char *url, http2_transport transport)
{
	struct http_parser_url u;
	http_parser_url_init(&u);
	http_parser_parse_url(url, strlen(url), 0, &u);
	const char *hostname = &url[u.field_data[UF_HOST].off];
	int hostname_len = u.field_data[UF_HOST].len;
	conn->hostname = strndup(hostname, hostname_len);

	/* why need the muxtex? because getaddrinfo limited by MEMP_NUM_NETDB */
	http2_mutex_lock(&dns_mutex);
	struct addrinfo *addrinfo;
	if (HTTP2_ERR_OK != __resolve_host_name(hostname, hostname_len, &addrinfo)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "resolve host name fail\n");
		return HTTP2_ERR_USE;
	}

	int port = __get_port(url, &u);
	if (port == 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "invaild port\n");
		return HTTP2_ERR_PORT;
	}
	HTTP2_DBG(HTTP2_LEVEL_INFO, "get remote port [%d]\n", port);

	HTTP2_SOCKET socket_fd;
	void *sock_in = NULL;

	switch (addrinfo->ai_family) {
	case AF_INET:
		socket_fd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
		struct sockaddr_in *tmp = (struct sockaddr_in *)addrinfo->ai_addr;
		tmp->sin_port = htons(port);
		sock_in = (void *)tmp;
		break;

#ifdef HTTP2_SP_IPV6
	case AF_INET6:
		socket_fd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
		struct sockaddr_in6 *tmp6 = (struct sockaddr_in6 *)addrinfo->ai_addr;
		tmp6->sin6_port = htons(port);
		tmp6->sin6_family = AF_INET6;
		sock_in = (void *)tmp6;
		break;
#endif
	default:
		HTTP2_DBG(HTTP2_LEVEL_ERR, "invaild ai_family [%d]\n", addrinfo->ai_family);
		freeaddrinfo(addrinfo);
		http2_mutex_unlock(&dns_mutex);
		return HTTP2_ERR_USE;
	}

	if (socket_fd == HTTP2_INVALID_SOCKET) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "fail to create socket\n");
		freeaddrinfo(addrinfo);
		http2_mutex_unlock(&dns_mutex);
		return HTTP2_ERR_SOCKET;
	}

	struct timeval tv;
	tv.tv_sec = HTTP2_SOCKET_TIMEO / 1000;
	tv.tv_usec = (HTTP2_SOCKET_TIMEO % 1000) * 1000;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "fcntl set noblock fail\n");
		goto connect_sock_fail;
	}

	if (connect(socket_fd, (struct sockaddr *)sock_in, addrinfo->ai_addrlen) < 0) {
		if (errno != EINPROGRESS && errno != EWOULDBLOCK && errno != ENOTCONN) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "socket connect fail\n");
			goto connect_sock_fail;
		}
	}

	conn->socket_fd = socket_fd;
	freeaddrinfo(addrinfo);
	http2_mutex_unlock(&dns_mutex);
	return HTTP2_ERR_OK;

connect_sock_fail:
	shutdown(socket_fd, SHUT_RDWR);
	close(socket_fd);
	freeaddrinfo(addrinfo);
	http2_mutex_unlock(&dns_mutex);
	return HTTP2_ERR_SOCKET;
}

static http2_err_t http2_close_socket(struct http2_conn *conn)
{
	if (conn == NULL)
		return HTTP2_ERR_USE;

	if (conn->socket_fd != HTTP2_INVALID_SOCKET) {
		shutdown(conn->socket_fd, SHUT_RDWR);
		close(conn->socket_fd);
	}
	conn->socket_fd = HTTP2_INVALID_SOCKET;

	return HTTP2_ERR_OK;
}

static void mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
	printf("%s:%04d: %s", file, line, str);
}

static http2_err_t http2_connect_tls(struct http2_conn *conn,
             const char *url, http2_transport transport)
{
	if (HTTP2_ERR_OK != http2_connect_socket(conn, url, transport)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "tls socket connect fail\n");
		return HTTP2_ERR_SOCKET;
	}

#ifdef HTTP2_MBEDTLS
	conn->fd_ctx        = HTTP2_OBJ_NEW(mbedtls_net_context, 1);
	conn->ssl           = HTTP2_OBJ_NEW(mbedtls_ssl_context, 1);
	conn->ssl_conf      = HTTP2_OBJ_NEW(mbedtls_ssl_config, 1);
	conn->ssl_cert      = HTTP2_OBJ_NEW(mbedtls_x509_crt, 1);
	conn->ssl_ca_cert   = HTTP2_OBJ_NEW(mbedtls_x509_crt, 1);
	conn->ssl_pkey      = HTTP2_OBJ_NEW(mbedtls_pk_context, 1);
	conn->ssl_entropy   = HTTP2_OBJ_NEW(mbedtls_entropy_context, 1);
	conn->ssl_ctr_drbg  = HTTP2_OBJ_NEW(mbedtls_ctr_drbg_context, 1);

	if (!(conn->ssl && conn->ssl_conf && conn->ssl_cert &&
		  conn->ssl_pkey && conn->ssl_entropy && conn->ssl_ctr_drbg)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls new fail\n");
		goto fail_ssl_connection1;
	}

	mbedtls_ssl_init(conn->ssl);
	mbedtls_ssl_config_init(conn->ssl_conf);
	mbedtls_x509_crt_init(conn->ssl_cert);
	mbedtls_pk_init(conn->ssl_pkey);
	mbedtls_entropy_init(conn->ssl_entropy);
	mbedtls_ctr_drbg_init(conn->ssl_ctr_drbg);
	mbedtls_debug_set_threshold(3);
	mbedtls_ssl_conf_dbg(conn->ssl_conf, mbedtls_debug, stdout);

	if (conn->certificate) {
		if (0 != mbedtls_x509_crt_parse(conn->ssl_cert,
		                                (const unsigned char *)conn->certificate,
		                                conn->certificate_size)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls parse x509 certificate fail\n");
			goto fail_ssl_connection2;
		}
	}

	if (conn->chain_certificate) {
		if (0 != mbedtls_x509_crt_parse(conn->ssl_cert,
		                                (const unsigned char *)conn->chain_certificate,
		                                conn->chain_certificate_size)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls parse x509 chain_certificate fail\n");
			goto fail_ssl_connection2;
		}
	}

	if (conn->ca_certificate) {
		if (0 != mbedtls_x509_crt_parse(conn->ssl_ca_cert,
		                                (const unsigned char *)conn->ca_certificate,
		                                conn->ca_certificate_size)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls parse x509 ca_certificate fail\n");
			goto fail_ssl_connection2;
		}
	}

	if (conn->private_key) {
		if (0 != mbedtls_pk_parse_key(conn->ssl_pkey,
		                              (const unsigned char *)conn->private_key,
		                              conn->private_key_size, NULL, 0)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls parse private_key fail\n");
			goto fail_ssl_connection2;
		}
	}

	if (conn->hostname) {
		if (0 != mbedtls_ssl_set_hostname(conn->ssl, conn->hostname)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls_ssl_set_hostname fail\n");
			goto fail_ssl_connection2;
		}
	} else {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "conn->hostname is NULL\n");
		goto fail_ssl_connection2;
	}

	if (0 != mbedtls_ssl_config_defaults(conn->ssl_conf,
		                                 MBEDTLS_SSL_IS_CLIENT,
		                                 MBEDTLS_SSL_TRANSPORT_STREAM,
		                                 MBEDTLS_SSL_PRESET_DEFAULT)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls ssl config defaults fail\n");
		goto fail_ssl_connection2;
	}

#ifdef HTTP2_MBEDTLS_SSL_RENEGOTIATION
	mbedtls_ssl_conf_renegotiation(conn->ssl_conf, MBEDTLS_SSL_RENEGOTIATION_ENABLED);
#endif
	if (conn->alpn_protos) {
#ifdef HTTP2_MBEDTLS_SSL_ALPN
		if (0 != mbedtls_ssl_conf_alpn_protocols(conn->ssl_conf, conn->alpn_protos)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls ssl conf alpn protocols fail\n");
			goto fail_ssl_connection2;
		}
#else
		HTTP2_DBG(HTTP2_LEVEL_WAR, "alpn_protos configured but not set HTTP2_MBEDTLS_SSL_ALPN\n");
#endif
	}

	if (conn->disable_ssl_verify) {
		HTTP2_DBG(HTTP2_LEVEL_INFO, "set ssl authmode VERIFY_NONE\n");
		mbedtls_ssl_conf_authmode(conn->ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
	} else {
		HTTP2_DBG(HTTP2_LEVEL_INFO, "set ssl authmode VERIFY_REQUIRED\n");
		mbedtls_ssl_conf_authmode(conn->ssl_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
	}

	if (0 != mbedtls_ctr_drbg_seed(conn->ssl_ctr_drbg, mbedtls_entropy_func, conn->ssl_entropy, NULL, 0)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls ctr drbg seed fail\n");
		goto fail_ssl_connection2;
	}

	mbedtls_ssl_conf_rng(conn->ssl_conf, mbedtls_ctr_drbg_random, conn->ssl_ctr_drbg);
	mbedtls_ssl_conf_ca_chain(conn->ssl_conf, conn->ssl_ca_cert, NULL);

	if (0 != mbedtls_ssl_conf_own_cert(conn->ssl_conf, conn->ssl_cert, conn->ssl_pkey)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls ssl conf own cert fail\n");
		goto fail_ssl_connection2;
	}

	if (0 != mbedtls_ssl_setup(conn->ssl, conn->ssl_conf)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls ssl setup fail\n");
		goto fail_ssl_connection2;
	}

	conn->fd_ctx->fd = conn->socket_fd;
	mbedtls_ssl_set_bio(conn->ssl, conn->fd_ctx, mbedtls_net_send, mbedtls_net_recv, NULL);

	int rc = -1;
	int iterator = 0;
	struct timeval tv;
	fd_set fdset;
	while (1) {
		FD_ZERO(&fdset);
		FD_SET(conn->socket_fd, &fdset);

		tv.tv_sec = 0;
		tv.tv_usec = 10000;

		rc = select(conn->socket_fd + 1, NULL, &fdset, NULL, &tv);
		if (rc > 0) {
			break;
		} else if (rc == 0) {
			if (iterator > 1000) {
				HTTP2_DBG(HTTP2_LEVEL_ERR,
				         "Max retry calls=%d to select reached, shutting down connection\n",
				         iterator);
				goto fail_ssl_connection2;
			} else {
				iterator++;
				continue;
			}
		} else {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "select failed, rc %d\n", rc);
			goto fail_ssl_connection2;
		}
	}

	int ssl_error;
	iterator = 0;
	while ((ssl_error = mbedtls_ssl_handshake(conn->ssl)) != 0) {
		uint32_t ret;
		if ((ret = mbedtls_ssl_get_verify_result(conn->ssl)) != 0) {
			#define MBEDTLS_VRFY_BUF_SIZE 256
			char *vrfy_buf = malloc(MBEDTLS_VRFY_BUF_SIZE);
			if (!vrfy_buf) {
				HTTP2_DBG(HTTP2_LEVEL_ERR, "Malloc vrfy buf failed\n");
			} else {
				mbedtls_x509_crt_verify_info(vrfy_buf, MBEDTLS_VRFY_BUF_SIZE, "! ", ret);
				HTTP2_DBG(HTTP2_LEVEL_ERR, " %s\n", vrfy_buf);
				free(vrfy_buf);
			}
		}
		if ((ssl_error != MBEDTLS_ERR_SSL_WANT_READ) && (ssl_error != MBEDTLS_ERR_SSL_WANT_WRITE)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls ssl handshake fail\n");
			goto fail_ssl_connection2;
		}

		// TODO: debug code

		iterator++;  /* try and limit max reconnect allowed */
		if (iterator > conn->timeout_msec / 10) {
			HTTP2_DBG(HTTP2_LEVEL_ERR,
			         "Max retry calls=%d to SSL_connect reached, shutting down connection\n",
			         iterator);
			goto fail_ssl_connection2;
		}
		HTTP2_MSLEEP(10);  /* wait a bit before retry */
	} /* end while */

	if (mbedtls_ssl_get_verify_result(conn->ssl) != 0) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "mbedtls ssl get verify result fail\n");
		goto fail_ssl_connection2;
	}

	conn->enable_tls = 1;
#else
/* TODO: openssl or something else */
#endif

	return HTTP2_ERR_OK;

#ifdef HTTP2_MBEDTLS
fail_ssl_connection2:
	mbedtls_ssl_free(conn->ssl);
	mbedtls_ssl_config_free(conn->ssl_conf);
	mbedtls_x509_crt_free(conn->ssl_cert);
	mbedtls_pk_free(conn->ssl_pkey);
	mbedtls_entropy_free(conn->ssl_entropy);
	mbedtls_ctr_drbg_free(conn->ssl_ctr_drbg);

fail_ssl_connection1:
	http2_close_socket(conn);

	return HTTP2_ERR_TLS;
#endif
}

static http2_err_t xr_call_nghttp2_connect(struct http2_conn *conn)
{
	nghttp2_session_callbacks *callbacks;
	nghttp2_session_callbacks_new(&callbacks);
	nghttp2_session_callbacks_set_send_callback(callbacks, http2_callback_send);
	nghttp2_session_callbacks_set_recv_callback(callbacks, http2_callback_recv);
	nghttp2_session_callbacks_set_on_frame_send_callback(callbacks, http2_callback_on_frame_send);
	nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, http2_callback_on_frame_recv);
	nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, http2_callback_on_stream_close);
	nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, http2_callback_on_data_chunk_recv);
	nghttp2_session_callbacks_set_on_header_callback(callbacks, http2_callback_on_header);

	if (0 != nghttp2_session_client_new(&conn->http2_session, callbacks, (void *)conn)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2_session_client_new fail\n");
		nghttp2_session_callbacks_del(callbacks);
		return HTTP2_ERR_NGHTTP;
	}
	nghttp2_session_callbacks_del(callbacks);

	if (0 != nghttp2_submit_settings(conn->http2_session, NGHTTP2_FLAG_NONE, NULL, 0)) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2_submit_settings fail\n");
		return HTTP2_ERR_NGHTTP;
	}
	return HTTP2_ERR_OK;
}

static http2_err_t xr_call_nghttp2_close(struct http2_conn *conn)
{
	if (conn->http2_session) {
		nghttp2_session_del(conn->http2_session);
		conn->http2_session = NULL;
	}

	return HTTP2_ERR_OK;
}

const char *alpn_protos[] = {"h2", NULL};
static http2_err_t http2_connect_entry(struct http2_conn *conn, const char *url,
                               http2_bool enable_tls, http2_transport transport)
{
	http2_err_t ret;

	conn->disable_ssl_verify = 1;
	conn->alpn_protos = alpn_protos;

	if (enable_tls)
		ret = http2_connect_tls(conn, url, transport);
	else
		ret = http2_connect_socket(conn, url, transport);

	if (ret != HTTP2_ERR_OK) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "socket/tls connect fail\n");
		return ret;
	}
	HTTP2_DBG(HTTP2_LEVEL_INFO, "socket/tls connected successfully\n");

	http2_set_io(conn, enable_tls);

	ret = xr_call_nghttp2_connect(conn);
	if (ret != HTTP2_ERR_OK) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "nghttp2 connect fail\n");
	}
	HTTP2_DBG(HTTP2_LEVEL_INFO, "nghttp2 connected successfully\n");

	return ret;
}

http2_err_t http2_connect(struct http2_conn *conn, const char *url, http2_bool enable_tls)
{
	if (conn == NULL) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "http2_conn is NULL\n");
		return HTTP2_ERR_USE;
	}

	return http2_connect_entry(conn, url, enable_tls, HTTP2_TRANSPORT_IPV4);
}

http2_err_t http2_conn_new(struct http2_conn **conn)
{
	struct http2_conn *new_conn = HTTP2_OBJ_NEW(struct http2_conn, 1);
	if (new_conn == NULL)
		return HTTP2_ERR_MEM;

	*conn = new_conn;
	memset(*conn, 0, sizeof(struct http2_conn));
	(*conn)->timeout_msec = 15000;
	(*conn)->refs = 1;
	return HTTP2_ERR_OK;
}

http2_err_t http2_conn_free(struct http2_conn *conn)
{
	if (conn == NULL)
		return HTTP2_ERR_OK;

	xr_call_nghttp2_close(conn);

	struct http2_node *node = NULL;
	node = http2_find_node_from_manager(&g_node_manager, conn);
	if (node != NULL)
		http2_node_free(node);

#ifdef HTTP2_MBEDTLS
	mbedtls_ssl_free(conn->ssl);
	mbedtls_ssl_config_free(conn->ssl_conf);
	mbedtls_x509_crt_free(conn->ssl_cert);
	mbedtls_pk_free(conn->ssl_pkey);
	mbedtls_entropy_free(conn->ssl_entropy);
	mbedtls_ctr_drbg_free(conn->ssl_ctr_drbg);
#else
	/* TODO: openssl or something else */
#endif
	http2_close_socket(conn);

	if (conn->http2_session) {
		nghttp2_session_del(conn->http2_session);
		conn->http2_session = NULL;
	}
	if (conn->hostname) {
		HTTP2_OBJ_FREE(conn->hostname);
		conn->hostname = NULL;
	}
	if (conn->certificate) {
		HTTP2_OBJ_FREE(conn->certificate);
		conn->certificate = NULL;
	}
	if (conn->private_key) {
		HTTP2_OBJ_FREE(conn->private_key);
		conn->private_key = NULL;
	}
	if (conn->chain_certificate) {
		HTTP2_OBJ_FREE(conn->chain_certificate);
		conn->chain_certificate = NULL;
	}
	if (conn->ca_certificate) {
		HTTP2_OBJ_FREE(conn->ca_certificate);
		conn->ca_certificate = NULL;
	}

	HTTP2_OBJ_FREE(conn);
	return HTTP2_ERR_OK;
}

#ifdef HTTP2_MBEDTLS
http2_err_t http2_set_ssl_certs(struct http2_conn *conn,
                                const char     *certificate,
                                int             certificate_size,
                                const char     *private_key,
                                int             private_key_size,
                                const char     *chain_certificate,
                                int             chain_certificate_size,
                                const char     *ca_certificate,
                                int             ca_certificate_size)
#else
/* TODO: openssl or something else */
http2_err_t http2_set_ssl_certs(struct http2_conn *conn)
#endif
{
	if (conn == NULL)
		return HTTP2_ERR_USE;

#ifdef HTTP2_MBEDTLS
	if (certificate)
		conn->certificate = HTTP2_OBJ_NEW(char, certificate_size);
	if (private_key)
		conn->private_key = HTTP2_OBJ_NEW(char, private_key_size);
	if (chain_certificate)
		conn->chain_certificate = HTTP2_OBJ_NEW(char, chain_certificate_size);
	if (ca_certificate)
		conn->ca_certificate = HTTP2_OBJ_NEW(char, ca_certificate_size);

	if ((certificate && (conn->certificate == NULL))
		|| (private_key && (conn->private_key == NULL))
		|| (chain_certificate && (conn->chain_certificate == NULL))
		|| (ca_certificate && (conn->ca_certificate == NULL))) {
		if (conn->certificate) {
			HTTP2_OBJ_FREE(conn->certificate);
			conn->certificate = NULL;
		}
		if (conn->private_key) {
			HTTP2_OBJ_FREE(conn->private_key);
			conn->private_key = NULL;
		}
		if (conn->chain_certificate) {
			HTTP2_OBJ_FREE(conn->chain_certificate);
			conn->chain_certificate = NULL;
		}
		if (conn->ca_certificate) {
			HTTP2_OBJ_FREE(conn->ca_certificate);
			conn->ca_certificate = NULL;
		}
		return HTTP2_ERR_MEM;
	}

	if (certificate) {
		memcpy(conn->certificate, certificate, certificate_size);
		conn->certificate_size = certificate_size;
	}
	if (private_key) {
		memcpy(conn->private_key, private_key, private_key_size);
		conn->private_key_size = private_key_size;
	}
	if (chain_certificate) {
		memcpy(conn->chain_certificate, chain_certificate, chain_certificate_size);
		conn->chain_certificate_size = chain_certificate_size;
	}
	if (ca_certificate) {
		memcpy(conn->ca_certificate, ca_certificate, ca_certificate_size);
		conn->ca_certificate_size = ca_certificate_size;
	}
#endif /* HTTP2_MBEDTLS */
	return HTTP2_ERR_OK;
}

void http2_set_timeout(struct http2_conn *conn, int msec)
{
	conn->timeout_msec = msec;
}

void http2_set_ssl_verify(struct http2_conn *conn, http2_bool enable)
{
	conn->disable_ssl_verify = !enable;
}


static void http2_node_manager_init(struct http2_node_manager *manager)
{
	if (manager->node_num != 0) {
		/* XXX: There may be need to prevent reinit */
		HTTP2_DBG(HTTP2_LEVEL_WAR, "node manager maybe reinit\n");
	}

	INIT_LIST_HEAD(&manager->head);
	manager->node_num = 0;
	http2_mutex_create(&manager->mutex);
	node_manager_init_flag = HTTP2_TRUE;
}

static http2_bool http2_node_manager_is_init(void)
{
	return node_manager_init_flag;
}

static struct http2_node *http2_node_new(struct http2_conn *conn, uint32_t stack_size)
{
	struct http2_node *node = NULL;
	node = (struct http2_node *)http2_malloc(sizeof(struct http2_node));
	if (node == NULL) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "node malloc fail\n");
		return NULL;
	}

	memset(node, 0, sizeof(struct http2_node));
	INIT_LIST_HEAD(&node->head);
	node->id = conn->socket_fd;
	node->conn = conn;
	node->stack_size = stack_size;
	node->state = HTTP2_NODE_S_NOREADY;
	node->stop_flag = 0;

	return node;
}

static void http2_node_free(struct http2_node *node)
{
	list_del(&node->head);

	if (http2_thread_isvalid(&node->thread)) {
		if (node->stop_flag == 0) {
			http2_node_stop(node);
		} else {
			/* XXX: Abnormal process, node->thread may have a dead loop */
			HTTP2_DBG(HTTP2_LEVEL_ERR, "maybe abnormal process happen\n");
		}
	}

	node->conn = NULL;  /* The release of conn is done by the application */
	http2_node_remove_from_manager(&g_node_manager, node);
	http2_free(node);
}

static void http2_node_add_to_manager(struct http2_node_manager *manager,
                                      struct http2_node *node)
{
	http2_mutex_lock(&manager->mutex);

	list_add_tail(&node->head, &manager->head);
	node->state = HTTP2_NODE_S_READY;
	manager->node_num += 1;

	http2_mutex_unlock(&manager->mutex);
}

static void http2_node_remove_from_manager(struct http2_node_manager *manager,
                                           struct http2_node *node)
{
	http2_mutex_lock(&manager->mutex);

	list_del(&node->head);
	node->state = HTTP2_NODE_S_NOREADY;
	manager->node_num -= 1;

	http2_mutex_unlock(&manager->mutex);
}

static struct http2_node *http2_find_node_from_manager(struct http2_node_manager *manager,
                                                       struct http2_conn *conn)
{
	struct http2_node *pos_node;

	http2_mutex_lock(&manager->mutex);

	list_for_each_entry(pos_node, &manager->head, head) {
		if (pos_node->conn == conn) {
		        http2_mutex_unlock(&manager->mutex);
		        return pos_node;
		}
	}

	http2_mutex_unlock(&manager->mutex);
	return NULL; /* no found */
}

static void http2_execute_task(void *arg)
{
	uint8_t exit_reason;
	struct http2_node *node = arg;

	node->state = HTTP2_NODE_S_RUNNING;
	HTTP2_DBG(HTTP2_LEVEL_INFO, "http2_node_thread_%d starts running\n", node->id);

	while (1) {
		if (node->stop_flag == 1) {
			exit_reason = 1;
			break;
		}

		if (HTTP2_ERR_OK != http2_execute(node->conn)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "http2 send/receive fail\n");
			exit_reason = 2;
			break;
		}

		HTTP2_MSLEEP(2);
	}

	switch (exit_reason) {
	case 1:
		node->state = HTTP2_NODE_S_READY;
		break;
	case 2:
		node->state = HTTP2_NODE_S_ABORTED;
		break;
	default:
		HTTP2_DBG(HTTP2_LEVEL_ERR, "unknow exit reason\n");
		break;
	}

	HTTP2_DBG(HTTP2_LEVEL_INFO, "http2 execute thread exit, reason: %d\n", exit_reason);
	http2_thread_delete(&node->thread);
}

static http2_err_t http2_node_run(struct http2_node *node)
{
	if (node == NULL) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "can't run, node == NULL\n");
		return HTTP2_ERR_USE;
	}

	char node_thread_name[30];
	sprintf(node_thread_name, "http2_node_thread_%d", node->id);

	if (!http2_thread_isvalid(&node->thread)) {
		if (OS_OK != http2_thread_create(&node->thread,
		                                 node_thread_name,
		                                 http2_execute_task,
		                                 (void *)node,
		                                 OS_THREAD_PRIO_LWIP,
		                                 node->stack_size)) {
			HTTP2_DBG(HTTP2_LEVEL_ERR, "http2 thread create fail\n");
			return HTTP2_ERR_OS;
		}
		HTTP2_DBG(HTTP2_LEVEL_INFO, "http2_node_thread_%d is created\n", node->id);
		return HTTP2_ERR_OK;
	} else {
		HTTP2_DBG(HTTP2_LEVEL_WAR, "do not create thread again, http2 thread is running\n");
		return HTTP2_ERR_OK;
	}
}

static http2_err_t http2_node_stop(struct http2_node *node)
{
	node->stop_flag = 1;
	while (http2_thread_isvalid(&node->thread))
		HTTP2_MSLEEP(2);

	HTTP2_DBG(HTTP2_LEVEL_INFO, "http2_node_thread_%d stopped\n", node->id);
	return HTTP2_ERR_OK;
}

http2_err_t http2_execute_task_start(struct http2_conn *conn, uint32_t stack_size)
{
	int ret = HTTP2_ERR_OK;

	if (conn == NULL) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "fail to start task, conn is NULL\n");
		ret = HTTP2_ERR_USE;
		goto task_start_end;
	}

	if (HTTP2_FALSE == http2_node_manager_is_init()) {
		HTTP2_DBG(HTTP2_LEVEL_INFO, "node manager was not initialized\n");
		ret = HTTP2_ERR_USE;
		goto task_start_end;
	}

	struct http2_node *node = NULL;
	node = http2_find_node_from_manager(&g_node_manager, conn);
	if (node == NULL) {
		node = http2_node_new(conn, stack_size);
		if (node == NULL) {
			ret = HTTP2_ERR_MEM;
			goto task_start_end;
		}
		http2_node_add_to_manager(&g_node_manager, node);
	}

	ret = http2_node_run(node);
	if (ret != HTTP2_ERR_OK) {
		HTTP2_DBG(HTTP2_LEVEL_ERR, "http2_node_run fail\n");
		goto task_start_end;
	}

task_start_end:
	return ret;
}

http2_err_t http2_execute_task_stop(struct http2_conn *conn)
{
	struct http2_node *node = NULL;
	node = http2_find_node_from_manager(&g_node_manager, conn);
	if (node == NULL) {
		HTTP2_DBG(HTTP2_LEVEL_WAR, "don't execute stop, no found node\n");
		return HTTP2_ERR_USE;
	}

	http2_node_stop(node);
	return HTTP2_ERR_OK;
}

void http2_api_init(void)
{
	http2_mutex_create(&dns_mutex);
	http2_node_manager_init(&g_node_manager);
}
