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
#ifndef HTTP2_API_H
#define HTTP2_API_H

#include <string.h>
#include <nghttp2/nghttp2.h>
#include "nghttp2/http2_decl.h"
#include "sys/list.h"

#define DATA_RECV_RST_STREAM      1    /* Flag indicating receive stream is reset */
#define DATA_RECV_FRAME_COMPLETE  2    /* Flag indicating frame is completely received */

typedef enum {
	HTTP2_ERR_OK      = 0,
	HTTP2_ERR_MEM     = 1,
	HTTP2_ERR_USE     = 2,
	HTTP2_ERR_PORT    = 3,
	HTTP2_ERR_SOCKET  = 4,
	HTTP2_ERR_TLS     = 5,
	HTTP2_ERR_NGHTTP  = 6,
	HTTP2_ERR_OS      = 7
} http2_err_t;

typedef enum {
	HTTP2_TRANSPORT_IPV4 = 0,    /* Use IPv4 transport */
#ifdef HTTP2_SP_IPV6
	HTTP2_TRANSPORT_IPV6 = 1     /* Use IPv6 transport */
#endif
} http2_transport;

struct http2_conn {
	HTTP2_SOCKET    socket_fd;              /* The actual socket file descriptor */
	nghttp2_session *http2_session;         /* Pointer to the nghttp2 session */
	char            *hostname;              /* The remote hostname */

	int             timeout_msec;           /* Maximum waiting time of connection process,
	                                           The unit is milliseconds */
	char            *certificate;
	char            *private_key;
	char            *chain_certificate;
	char            *ca_certificate;
#ifdef HTTP2_MBEDTLS
	int             certificate_size;
	int             private_key_size;
	int             chain_certificate_size;
	int             ca_certificate_size;
#endif
	http2_bool      disable_ssl_verify;     /* Flag of ssl/tls verify enable or not */
	http2_bool      enable_tls;             /* Flag of tls enable or not */

#ifdef HTTP2_MBEDTLS
	mbedtls_net_context      *fd_ctx;
	mbedtls_ssl_context      *ssl;
	mbedtls_ssl_config       *ssl_conf;
	mbedtls_x509_crt         *ssl_cert;     /* The own certificate chain */
	mbedtls_x509_crt         *ssl_ca_cert;  /* The ca certificate */
	mbedtls_pk_context       *ssl_pkey;
	mbedtls_entropy_context  *ssl_entropy;
	mbedtls_ctr_drbg_context *ssl_ctr_drbg;
#ifdef HTTP2_MBEDTLS_SSL_ALPN
	const char               **alpn_protos; /* A list of protocols that should be negotiated.
	                                           For the most common cases the following is ok:
	                                           const char **alpn_protos = { "h2", NULL }; */
#endif
#else  /* HTTP2_MBEDTLS */
	   /* TODO: openssl or something else */
#endif /* HTTP2_MBEDTLS */

	int (*input)(struct http2_conn *conn, void *data, size_t datalen);          /* Pointer to the actual network read function */
	int (*output)(struct http2_conn *conn, const void *data, size_t datalen);   /* Pointer to the actual network write function */

	int              refs;       /* Reference counting of itself */
	http2_mutex_t    ref_mutex;  /* Mutex of itself, useless for now. */
};

typedef enum {
	HTTP2_NODE_S_NOREADY     = 0,           /* Node is created */
	HTTP2_NODE_S_READY       = 1,           /* Node is added to manager list */
	HTTP2_NODE_S_RUNNING     = 2,           /* Node thread is running */
	HTTP2_NODE_S_ABORTED     = 3            /* Node thread abnormal exit */
} http2_node_state_t;

struct http2_node {
	struct list_head         head;          /* List head of node */
	uint16_t                 id;            /* Node ID */
	struct http2_conn        *conn;         /* Pointer to the http2 connection */
	http2_thread_t           thread;        /* Execute thread */
	uint32_t                 stack_size;    /* Stack size of execute thread */
	http2_node_state_t       state;         /* Indicating thread state */
	uint8_t                  stop_flag;     /* Flag is used to stop thread */
};

struct http2_node_manager {
	struct list_head         head;
	uint8_t                  node_num;
	http2_mutex_t            mutex;
};

typedef int (*http2_data_recv_cb)(struct http2_conn *conn, const char *data, size_t len, int flags);
typedef int (*http2_put_data_cb)(struct http2_conn *conn, char *data, size_t len, uint32_t *data_flags);
typedef int (*http2_post_data_cb)(struct http2_conn *conn, char *data, size_t len, uint32_t *data_flags);

/**
 * @brief Init http2 api
 *
 * @return    void
 */
void http2_api_init(void);

/**
 * @brief Create a new connection options object.
 *
 * A newly created connection object. you can use it like :
 *      {
 *      struct http2_conn *conn;
 *      http2_conn_new(&conn);
 *      }
 *
 * @param[out] conn -- Pointer to the conn pointer by Created.
 *
 * @return     HTTP2_ERR_OK -- if create successfully
 *         others       -- see @{http2_err_t}
 */
http2_err_t http2_conn_new(struct http2_conn **conn);

/**
 * @brief Allows to release a connection object by pointer.
 *
 * @param[in] conn -- Pointer to the conn that will be released.
 *
 * @return    HTTP2_ERR_OK -- if free successfully
 *        others       -- see @{http2_err_t}
 */
http2_err_t http2_conn_free(struct http2_conn *conn);

/**
 * @brief Establish an HTTP2 connection with the peer through the URL
 *
 * This function can establish an HTTP2 connection with the peer as client.
 * Support h2 and h2c (HTTP/2 without TLS).
 *
 * @param[in] conn -- Pointer to the conn that will be connected.
 *
 * @param[in] url -- Uniform Resource Locator, it's a string, such as "/PostTest".
 *
 * @param[in] enable_tls -- HTTP2_TRUE for h2
 *               -- HTTP2_FALSE for h2c.
 *
 * @return    HTTP2_ERR_OK -- if connect successfully
 *        others       -- see @{http2_err_t}
 */
http2_err_t http2_connect(struct http2_conn *conn, const char *url, http2_bool enable_tls);

/**
 * @brief Execute send/receive on an HTTP/2 connection.
 *
 * This function is used to execute the HTTP2 protocol process, sending and receiving.
 * When you use the http2_get()、http2_put() those APIs, you only write data to nghttp2.
 * You need to call this function to do the actual sending and receiving operations.
 *
 * @param[in] conn -- Pointer to the conn that will be executed.
 *
 * @return    HTTP2_ERR_OK -- if execute successfully
 *        others       -- see @{http2_err_t}
 */
http2_err_t http2_execute(struct http2_conn *conn);

/**
 * @brief Setup an HTTP2 GET request stream.
 *
 * This function will generate HTTP2 GET request streams, these data streams will be
 * temporarily stored in nghttp2, you need to call @{http2_execute()} to complete the
 * actual sending and receiving operations.
 * When receiving the response from the peer, call back through recv_cb.
 *
 * @param[in] conn -- Pointer to the conn that will do an HTTP2 GET.
 *
 * @param[in] path -- Pointer to the request path, it's a string, such as "/GetTest".
 *
 * @param[in] recv_cb -- Setting the response callback function.
 *
 * @return    HTTP2_ERR_OK -- if setup successfully
 *        others       -- see @{http2_err_t}
 */
http2_err_t http2_get(struct http2_conn *conn, const char *path, http2_data_recv_cb recv_cb);

/**
 * @brief Setup an HTTP2 PUT request stream.
 *
 * This function will generate HTTP2 PUT request streams, these data streams will be
 * temporarily stored in nghttp2, you need to call @{http2_execute()} to complete the
 * actual sending and receiving operations.
 * When receiving the response from the peer, call back through recv_cb.
 *
 * @param[in] conn -- Pointer to the conn that will do an HTTP2 PUT.
 *
 * @param[in] path -- Pointer to the request path, it's a string, such as "/PutTest".
 *
 * @param[in] put_cb -- Setting the HTTP2 PUT data function. This function is used to pass
 *              data to nghttp2, it will be called by nghttp2 internally.
 *
 * @param[in] recv_cb -- Setting the response callback function.
 *
 * @return    HTTP2_ERR_OK -- if setup successfully
 *        others       -- see @{http2_err_t}
 */
http2_err_t http2_put(struct http2_conn *conn, const char *path,
              http2_put_data_cb put_cb, http2_data_recv_cb recv_cb);

/**
 * @brief Setup an HTTP2 POST request stream.
 *
 * This function will generate HTTP2 POST request streams, these data streams will be
 * temporarily stored in nghttp2, you need to call @{http2_execute()} to complete the
 * actual sending and receiving operations.
 * When receiving the response from the peer, call back through recv_cb.
 *
 * @param[in] conn -- Pointer to the conn that will do an HTTP2 POST.
 *
 * @param[in] path -- Pointer to the request path, it's a string, such as "/PostTest".
 *
 * @param[in] post_cb -- Setting the HTTP2 POST data function. This function is used to pass
 *              data to nghttp2, it will be called by nghttp2 internally.
 *
 * @param[in] recv_cb -- Setting the response callback function.
 *
 * @return    HTTP2_ERR_OK -- if setup successfully
 *        others       -- see @{http2_err_t}
 */
http2_err_t http2_post(struct http2_conn *conn, const char *path,
               http2_post_data_cb post_cb, http2_data_recv_cb recv_cb);

/**
 * @brief Start http2 execute task.
 *
 * @param[in] conn -- The connection where these settings will be applied.
 *
 * @param[in] stack_size -- The stack size of task.
 *
 * @return    HTTP2_ERR_OK -- if setting successfully
 *            others       -- see @{http2_err_t}
 */
http2_err_t http2_execute_task_start(struct http2_conn *conn, uint32_t stack_size);

/**
 * @brief Stop http2 execute task.
 *
 * @param[in] conn -- The connection will be stopped.
 *
 * @return    HTTP2_ERR_OK -- if setting successfully
 *            others       -- see @{http2_err_t}
 */
http2_err_t http2_execute_task_stop(struct http2_conn *conn);

/**
 * @brief Set connection timeout.
 *
 * @param[in] conn -- The connection where these settings will be applied.
 *
 * @param[in] msec -- Maximum waiting time of connection process, The unit is milliseconds.
 *
 * @return    void
 */
void http2_set_timeout(struct http2_conn *conn, int msec);

/**
 * @brief Set SSL verification mode.
 *
 * @param[in] conn -- The connection where these settings will be applied.
 *
 * @param[in] enable -- Indicates whether verification is enabled.
 *
 * @return    void
 */
void http2_set_ssl_verify(struct http2_conn *conn, http2_bool enable);

/**
 * @brief Allows to certificate, private key and optional chain
 * certificate and ca for on a particular options that can be used for
 * a client and a listener connection.
 *
 * @param[in] conn -- The connection where these settings will be applied.
 *
 * @param[in] certificate -- The certificate to use on the connection.
 *
 * @param[in] private_key -- client_certificate private key.
 *
 * @param[in] chain_certificate -- chain certificate to use.
 *
 * @param[in] ca_certificate -- CA certificate to use during the process.
 *
 * @return    HTTP2_ERR_OK -- if setting successfully
 *        others       -- see @{http2_err_t}
 */
http2_err_t http2_set_ssl_certs(struct http2_conn *conn,
                                const char     *certificate,
                                int             certificate_size,
                                const char     *private_key,
                                int             private_key_size,
                                const char     *chain_certificate,
                                int             chain_certificate_size,
                                const char     *ca_certificate,
                                int             ca_certificate_size);
#endif /* HTTP2_API_H */
