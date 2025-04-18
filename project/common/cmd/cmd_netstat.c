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

#if PRJCONF_NET_EN

#include "cmd_util.h"
#include "common/framework/net_ctrl.h"
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/api.h"
#include "lwip/ip_addr.h"

#define NUM_TCP_PCB_LISTS 4
extern struct tcp_pcb ** const tcp_pcb_lists[NUM_TCP_PCB_LISTS];
static const char * const tcp_state_str[] = {
	"CLOSED",
	"LISTEN",
	"SYN_SENT",
	"SYN_RCVD",
	"ESTABLISHED",
	"FIN_WAIT_1",
	"FIN_WAIT_2",
	"CLOSE_WAIT",
	"CLOSING",
	"LAST_ACK",
	"TIME_WAIT"
};

static void tcp_netstat(void)
{
	int i;
	struct tcp_pcb *cpcb;
	struct netconn *conn;
	u16_t send_q;
	u16_t recv_q;

	for (i = 0; i < NUM_TCP_PCB_LISTS; i++) {
		for (cpcb = *tcp_pcb_lists[i]; cpcb != NULL; cpcb = cpcb->next) {
			conn = (struct netconn *)cpcb->callback_arg;

			if (cpcb->state < SYN_SENT) {
				send_q = recv_q = 0;
			} else {
				send_q = TCP_SND_BUF - tcp_sndbuf(cpcb);
#if LWIP_SO_RCVBUF
				recv_q = conn->recv_avail;
#else
				recv_q = 0;
#endif
			}

#ifdef __CONFIG_LWIP_V1
			CMD_LOG(1,
			    "%s\t%d\t%d\t%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\t"
			    "%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\t%s\n",
			    "tcp",
			    recv_q,
			    send_q,
			    ip4_addr1_16(&cpcb->local_ip),
			    ip4_addr2_16(&cpcb->local_ip),
			    ip4_addr3_16(&cpcb->local_ip),
			    ip4_addr4_16(&cpcb->local_ip),
			    cpcb->local_port,
			    ip4_addr1_16(&cpcb->remote_ip),
			    ip4_addr2_16(&cpcb->remote_ip),
			    ip4_addr3_16(&cpcb->remote_ip),
			    ip4_addr4_16(&cpcb->remote_ip),
			    cpcb->remote_port,
			    tcp_state_str[cpcb->state]);
#else
			CMD_LOG(1,
			    "%s\t%d\t%d\t%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\t"
			    "%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\t%s\n",
			    "tcp",
			    recv_q,
			    send_q,
			    ip4_addr1_16(ip_2_ip4(&cpcb->local_ip)),
			    ip4_addr2_16(ip_2_ip4(&cpcb->local_ip)),
			    ip4_addr3_16(ip_2_ip4(&cpcb->local_ip)),
			    ip4_addr4_16(ip_2_ip4(&cpcb->local_ip)),
			    cpcb->local_port,
			    ip4_addr1_16(ip_2_ip4(&cpcb->remote_ip)),
			    ip4_addr2_16(ip_2_ip4(&cpcb->remote_ip)),
			    ip4_addr3_16(ip_2_ip4(&cpcb->remote_ip)),
			    ip4_addr4_16(ip_2_ip4(&cpcb->remote_ip)),
			    cpcb->remote_port,
			    tcp_state_str[cpcb->state]);
#endif
		}
	}
}

#define DHCP_CLIENT_PORT 68
#define DNS_SERVER_PORT  53
extern struct udp_pcb *udp_pcbs;

static void udp_netstat(void)
{
	struct udp_pcb *pcb;
	struct netconn *conn;
	s16_t recv_q;

	for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next) {
		conn = (struct netconn *)pcb->recv_arg;

		if (pcb->local_port == DHCP_CLIENT_PORT || pcb->local_port == DNS_SERVER_PORT)
			recv_q = 0;
		else
#if LWIP_SO_RCVBUF
			recv_q = conn->recv_avail;
#else
			recv_q = 0;
#endif

#ifdef __CONFIG_LWIP_V1
		CMD_LOG(1,
		    "%s\t%d\t%d\t%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\t"
		    "%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\n",
		    "udp",
		    recv_q,
		    0,
		    ip4_addr1_16(&pcb->local_ip),
		    ip4_addr2_16(&pcb->local_ip),
		    ip4_addr3_16(&pcb->local_ip),
		    ip4_addr4_16(&pcb->local_ip),
		    pcb->local_port,
		    ip4_addr1_16(&pcb->remote_ip),
		    ip4_addr2_16(&pcb->remote_ip),
		    ip4_addr3_16(&pcb->remote_ip),
		    ip4_addr4_16(&pcb->remote_ip),
		    pcb->remote_port);
#else
		CMD_LOG(1,
		    "%s\t%d\t%d\t%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\t"
		    "%-3"U16_F".%-3"U16_F".%-3"U16_F".%-3"U16_F":%d\n",
		    "udp",
		    recv_q,
		    0,
		    ip4_addr1_16(ip_2_ip4(&pcb->local_ip)),
		    ip4_addr2_16(ip_2_ip4(&pcb->local_ip)),
		    ip4_addr3_16(ip_2_ip4(&pcb->local_ip)),
		    ip4_addr4_16(ip_2_ip4(&pcb->local_ip)),
		    pcb->local_port,
		    ip4_addr1_16(ip_2_ip4(&pcb->remote_ip)),
		    ip4_addr2_16(ip_2_ip4(&pcb->remote_ip)),
		    ip4_addr3_16(ip_2_ip4(&pcb->remote_ip)),
		    ip4_addr4_16(ip_2_ip4(&pcb->remote_ip)),
		    pcb->remote_port);
#endif
	}
}

void netstat(void)
{
	CMD_LOG(1, "Proto\tRecv-Q\tSend-Q\tLocal Address\t\tForeign Address\t\tState\n");
	tcp_netstat();
	udp_netstat();
}

#if CMD_DESCRIBE
static const char *netstat_help_info =
"[*] -a : display all sockets.\n"
"[*] -h : Print this message and quit.\n";
#endif /* CMD_DESCRIBE */

static enum cmd_status cmd_netstat_help_exec(char *cmd)
{
#if CMD_DESCRIBE
	CMD_LOG(1, "%s\n", netstat_help_info);
#endif
	return CMD_STATUS_ACKED;
}

enum cmd_status cmd_netstat_exec(char *cmd)
{
	if (cmd_strcmp(cmd, "help") == 0 || cmd_strcmp(cmd, "-h") == 0) {
		cmd_netstat_help_exec(cmd);
		return CMD_STATUS_ACKED;
	}

	struct netif *nif = g_wlan_netif;
	if (nif == NULL || !NET_IS_IP4_VALID(nif)) {
		CMD_ERR("net is down, netstat failed\n");
		return CMD_STATUS_FAIL;
	}

	netstat();

	return CMD_STATUS_OK;
}

#endif /* PRJCONF_NET_EN */
