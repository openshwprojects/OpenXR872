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

#ifndef _NET_WLAN_WLAN_DEFS_H_
#define _NET_WLAN_WLAN_DEFS_H_

#include <stdint.h>
#include "compiler.h"
#include "net/wlan/wlan_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WPA_BIT
#define WPA_BIT(x)                    (1 << (x))
#endif

/**
 * @brief WPA cipher definition
 */
#define WPA_CIPHER_NONE                WPA_BIT(0)
#define WPA_CIPHER_WEP40               WPA_BIT(1)
#define WPA_CIPHER_WEP104              WPA_BIT(2)
#define WPA_CIPHER_TKIP                WPA_BIT(3)
#define WPA_CIPHER_CCMP                WPA_BIT(4)
#define WPA_CIPHER_AES_128_CMAC        WPA_BIT(5)
#define WPA_CIPHER_GCMP                WPA_BIT(6)
#define WPA_CIPHER_SMS4                WPA_BIT(7)
#define WPA_CIPHER_GCMP_256            WPA_BIT(8)
#define WPA_CIPHER_CCMP_256            WPA_BIT(9)
#define WPA_CIPHER_BIP_GMAC_128        WPA_BIT(11)
#define WPA_CIPHER_BIP_GMAC_256        WPA_BIT(12)
#define WPA_CIPHER_BIP_CMAC_256        WPA_BIT(13)
#define WPA_CIPHER_GTK_NOT_USED        WPA_BIT(14)
#define WPA_CIPHER_MASK                (WPA_BIT(15) - 1)

/**
 * @brief WPA key management definition
 */
#define WPA_KEY_MGMT_IEEE8021X              WPA_BIT(0)
#define WPA_KEY_MGMT_PSK                    WPA_BIT(1)
#define WPA_KEY_MGMT_NONE                   WPA_BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA       WPA_BIT(3)
#define WPA_KEY_MGMT_WPA_NONE               WPA_BIT(4)
#define WPA_KEY_MGMT_FT_IEEE8021X           WPA_BIT(5)
#define WPA_KEY_MGMT_FT_PSK                 WPA_BIT(6)
#define WPA_KEY_MGMT_IEEE8021X_SHA256       WPA_BIT(7)
#define WPA_KEY_MGMT_PSK_SHA256             WPA_BIT(8)
#define WPA_KEY_MGMT_WPS                    WPA_BIT(9)
#define WPA_KEY_MGMT_SAE                    WPA_BIT(10)
#define WPA_KEY_MGMT_FT_SAE                 WPA_BIT(11)
#define WPA_KEY_MGMT_WAPI_PSK               WPA_BIT(12)
#define WPA_KEY_MGMT_WAPI_CERT              WPA_BIT(13)
#define WPA_KEY_MGMT_CCKM                   WPA_BIT(14)
#define WPA_KEY_MGMT_OSEN                   WPA_BIT(15)
#define WPA_KEY_MGMT_IEEE8021X_SUITE_B      WPA_BIT(16)
#define WPA_KEY_MGMT_IEEE8021X_SUITE_B_192  WPA_BIT(17)
#define WPA_KEY_MGMT_FILS_SHA256            WPA_BIT(18)
#define WPA_KEY_MGMT_FILS_SHA384            WPA_BIT(19)
#define WPA_KEY_MGMT_FT_FILS_SHA256         WPA_BIT(20)
#define WPA_KEY_MGMT_FT_FILS_SHA384         WPA_BIT(21)
#define WPA_KEY_MGMT_OWE                    WPA_BIT(22)
#define WPA_KEY_MGMT_DPP                    WPA_BIT(23)
#define WPA_KEY_MGMT_FT_IEEE8021X_SHA384    WPA_BIT(24)

#define WPA_KEY_MGMT_FT (WPA_KEY_MGMT_FT_PSK | \
                         WPA_KEY_MGMT_FT_IEEE8021X | \
                         WPA_KEY_MGMT_FT_IEEE8021X_SHA384 | \
                         WPA_KEY_MGMT_FT_SAE | \
                         WPA_KEY_MGMT_FT_FILS_SHA256 | \
                         WPA_KEY_MGMT_FT_FILS_SHA384)


#define WPA_KEY_MGMT_MASK                   (WPA_BIT(25) - 1)

/**
 * @brief WPA protocol definition
 */
#define WPA_PROTO_WPA               WPA_BIT(0)
#define WPA_PROTO_RSN               WPA_BIT(1)
#define WPA_PROTO_WAPI              WPA_BIT(2)
#define WPA_PROTO_OSEN              WPA_BIT(3)
#define WPA_PROTO_MASK              (WPA_BIT(4) - 1)

/**
 * @brief WPA authentication algorithm definition
 */
#define WPA_AUTH_ALG_OPEN           WPA_BIT(0)
#define WPA_AUTH_ALG_SHARED         WPA_BIT(1)
#define WPA_AUTH_ALG_LEAP           WPA_BIT(2)
#define WPA_AUTH_ALG_FT             WPA_BIT(3)
#define WPA_AUTH_ALG_SAE            WPA_BIT(4)
#define WPA_AUTH_ALG_FILS           WPA_BIT(5)
#define WPA_AUTH_ALG_FILS_SK_PFS    WPA_BIT(6)
#define WPA_AUTH_ALG_MASK           (WPA_BIT(7) - 1)
#define WPA_AUTH_ALG_AUTO           0 /* automatic selection */

/**
 * @brief enum mfp_options - Management frame protection (IEEE 802.11w) options
 */
enum mfp_options {
	NO_MGMT_FRAME_PROTECTION = 0,
	MGMT_FRAME_PROTECTION_OPTIONAL = 1,
	MGMT_FRAME_PROTECTION_REQUIRED = 2,
};
#define MGMT_FRAME_PROTECTION_DEFAULT 3

/**
 * @brief WPA scan result flags definition
 */
#define WPA_FLAGS_WPA               WPA_BIT(0)
#define WPA_FLAGS_RSN               WPA_BIT(1)
#define WPA_FLAGS_WEP               WPA_BIT(2)
#define WPA_FLAGS_WPS_PBC           WPA_BIT(3)
#define WPA_FLAGS_WPS_AUTH          WPA_BIT(4)
#define WPA_FLAGS_WPS_PIN           WPA_BIT(5)
#define WPA_FLAGS_WPS               WPA_BIT(6)
#define WPA_FLAGS_IBSS              WPA_BIT(7)
#define WPA_FLAGS_ESS               WPA_BIT(8)
#define WPA_FLAGS_SAE               WPA_BIT(9)

/**
 * @brief WLAN SSID and passphrase definition
 */
#define WLAN_SSID_MAX_LEN       32
#define WLAN_PASSPHRASE_MIN_LEN 8
#define WLAN_PASSPHRASE_MAX_LEN 63
#define WLAN_PSK_HEX_LEN        32
#define WLAN_PSK_HEX_STR_LEN    64 /* two characters per octet of PSK */
#define WLAN_MAC_ADDR_LEN       6

/**
 * @brief WLAN WEP key length definition
 */
#define WLAN_WEP40_KEY_LEN      5  /* 5-byte  (40-bit)  */
#define WLAN_WEP104_KEY_LEN     13 /* 13-byte (104-bit) */
#define WLAN_WEP128_KEY_LEN     16 /* 16-byte (128-bit), unsupported */
#define WLAN_WEP_KEY_MAX_LEN    WLAN_WEP128_KEY_LEN

/**
 * @brief IEEE 802.11 frame type definition
 */
#define IEEE80211_FC_TYPE_SHIFT         2
#define IEEE80211_FC_TYPE_MASK          (0x3 << IEEE80211_FC_TYPE_SHIFT)
#define IEEE80211_FC_TYPE_MGMT          (0x0 << IEEE80211_FC_TYPE_SHIFT)
#define IEEE80211_FC_TYPE_CTRL          (0x1 << IEEE80211_FC_TYPE_SHIFT)
#define IEEE80211_FC_TYPE_DATA          (0x2 << IEEE80211_FC_TYPE_SHIFT)

/**
 * @brief IEEE 802.11 management frame subtype definition
 */
#define IEEE80211_FC_STYPE_SHIFT        4
#define IEEE80211_FC_STYPE_MASK         (0xf << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ASSOC_REQ    (0x0 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ASSOC_RESP   (0x1 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_REASSOC_REQ  (0x2 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_REASSOC_RESP (0x3 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_PROBE_REQ    (0x4 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_PROBE_RESP   (0x5 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_BEACON       (0x8 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ATIM         (0x9 << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_DISASSOC     (0xa << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_AUTH         (0xb << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_DEAUTH       (0xc << IEEE80211_FC_STYPE_SHIFT)
#define IEEE80211_FC_STYPE_ACTION       (0xd << IEEE80211_FC_STYPE_SHIFT)

#define IEEE80211_FC0_VERSION_MASK      0x03
#define IEEE80211_FC0_VERSION_SHIFT     0
#define IEEE80211_FC0_VERSION_0         0x00
#define IEEE80211_FC0_TYPE_MASK         0x0c
#define IEEE80211_FC0_TYPE_SHIFT        2
#define IEEE80211_FC0_TYPE_MGT          0x00
#define IEEE80211_FC0_TYPE_CTL          0x04
#define IEEE80211_FC0_TYPE_DATA         0x08

#define IEEE80211_FC0_SUBTYPE_MASK      0xf0
#define IEEE80211_FC0_SUBTYPE_SHIFT     4

#define IEEE80211_FC1_DIR_MASK          0x03
#define IEEE80211_FC1_DIR_NODS          0x00    /* STA->STA */
#define IEEE80211_FC1_DIR_TODS          0x01    /* STA->AP  */
#define IEEE80211_FC1_DIR_FROMDS        0x02    /* AP ->STA */
#define IEEE80211_FC1_DIR_DSTODS        0x03    /* AP ->AP  */

#define IEEE80211_ADDR_LEN 6

/* Status codes (IEEE Std 802.11-2016, 9.4.1.9, Table 9-46) */
#define WLAN_STATUS_SUCCESS                      0
#define WLAN_STATUS_UNSPECIFIED_FAILURE          1
#define WLAN_STATUS_TDLS_WAKEUP_ALTERNATE        2
#define WLAN_STATUS_TDLS_WAKEUP_REJECT           3
#define WLAN_STATUS_SECURITY_DISABLED            5
#define WLAN_STATUS_UNACCEPTABLE_LIFETIME        6
#define WLAN_STATUS_NOT_IN_SAME_BSS              7
#define WLAN_STATUS_CAPS_UNSUPPORTED             10
#define WLAN_STATUS_REASSOC_NO_ASSOC             11
#define WLAN_STATUS_ASSOC_DENIED_UNSPEC          12
#define WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG       13
#define WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION     14
#define WLAN_STATUS_CHALLENGE_FAIL               15
#define WLAN_STATUS_AUTH_TIMEOUT                 16
#define WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA  17
#define WLAN_STATUS_ASSOC_DENIED_RATES           18
#define WLAN_STATUS_ASSOC_DENIED_NOSHORT         19
#define WLAN_STATUS_SPEC_MGMT_REQUIRED           22
#define WLAN_STATUS_PWR_CAPABILITY_NOT_VALID     23
#define WLAN_STATUS_SUPPORTED_CHANNEL_NOT_VALID        24
#define WLAN_STATUS_ASSOC_DENIED_NO_SHORT_SLOT_TIME    25
#define WLAN_STATUS_ASSOC_DENIED_NO_HT                 27
#define WLAN_STATUS_R0KH_UNREACHABLE                   28
#define WLAN_STATUS_ASSOC_DENIED_NO_PCO                29
#define WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY         30
#define WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION 31
#define WLAN_STATUS_UNSPECIFIED_QOS_FAILURE            32
#define WLAN_STATUS_DENIED_INSUFFICIENT_BANDWIDTH      33
#define WLAN_STATUS_DENIED_POOR_CHANNEL_CONDITIONS     34
#define WLAN_STATUS_DENIED_QOS_NOT_SUPPORTED           35
#define WLAN_STATUS_REQUEST_DECLINED                   37
#define WLAN_STATUS_INVALID_PARAMETERS                 38
#define WLAN_STATUS_REJECTED_WITH_SUGGESTED_CHANGES    39
#define WLAN_STATUS_INVALID_IE                         40
#define WLAN_STATUS_GROUP_CIPHER_NOT_VALID             41
#define WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID          42
#define WLAN_STATUS_AKMP_NOT_VALID                     43
#define WLAN_STATUS_UNSUPPORTED_RSN_IE_VERSION         44
#define WLAN_STATUS_INVALID_RSN_IE_CAPAB               45
#define WLAN_STATUS_CIPHER_REJECTED_PER_POLICY         46
#define WLAN_STATUS_TS_NOT_CREATED                     47
#define WLAN_STATUS_DIRECT_LINK_NOT_ALLOWED            48
#define WLAN_STATUS_DEST_STA_NOT_PRESENT               49
#define WLAN_STATUS_DEST_STA_NOT_QOS_STA               50
#define WLAN_STATUS_ASSOC_DENIED_LISTEN_INT_TOO_LARGE  51
#define WLAN_STATUS_INVALID_FT_ACTION_FRAME_COUNT      52
#define WLAN_STATUS_INVALID_PMKID                      53
#define WLAN_STATUS_INVALID_MDIE                       54
#define WLAN_STATUS_INVALID_FTIE                       55
#define WLAN_STATUS_REQUESTED_TCLAS_NOT_SUPPORTED      56
#define WLAN_STATUS_INSUFFICIENT_TCLAS_PROCESSING_RESOURCES  57
#define WLAN_STATUS_TRY_ANOTHER_BSS                          58
#define WLAN_STATUS_GAS_ADV_PROTO_NOT_SUPPORTED              59
#define WLAN_STATUS_NO_OUTSTANDING_GAS_REQ                   60
#define WLAN_STATUS_GAS_RESP_NOT_RECEIVED                    61
#define WLAN_STATUS_STA_TIMED_OUT_WAITING_FOR_GAS_RESP       62
#define WLAN_STATUS_GAS_RESP_LARGER_THAN_LIMIT               63
#define WLAN_STATUS_REQ_REFUSED_HOME                         64
#define WLAN_STATUS_ADV_SRV_UNREACHABLE                      65
#define WLAN_STATUS_REQ_REFUSED_SSPN                         67
#define WLAN_STATUS_REQ_REFUSED_UNAUTH_ACCESS                68
#define WLAN_STATUS_INVALID_RSNIE                            72
#define WLAN_STATUS_U_APSD_COEX_NOT_SUPPORTED                73
#define WLAN_STATUS_U_APSD_COEX_MODE_NOT_SUPPORTED           74
#define WLAN_STATUS_BAD_INTERVAL_WITH_U_APSD_COEX            75
#define WLAN_STATUS_ANTI_CLOGGING_TOKEN_REQ                  76
#define WLAN_STATUS_FINITE_CYCLIC_GROUP_NOT_SUPPORTED        77
#define WLAN_STATUS_CANNOT_FIND_ALT_TBTT                     78
#define WLAN_STATUS_TRANSMISSION_FAILURE                     79
#define WLAN_STATUS_REQ_TCLAS_NOT_SUPPORTED                  80
#define WLAN_STATUS_TCLAS_RESOURCES_EXCHAUSTED               81
#define WLAN_STATUS_REJECTED_WITH_SUGGESTED_BSS_TRANSITION   82
#define WLAN_STATUS_REJECT_WITH_SCHEDULE                     83
#define WLAN_STATUS_REJECT_NO_WAKEUP_SPECIFIED               84
#define WLAN_STATUS_SUCCESS_POWER_SAVE_MODE                  85
#define WLAN_STATUS_PENDING_ADMITTING_FST_SESSION            86
#define WLAN_STATUS_PERFORMING_FST_NOW                       87
#define WLAN_STATUS_PENDING_GAP_IN_BA_WINDOW                 88
#define WLAN_STATUS_REJECT_U_PID_SETTING                     89
#define WLAN_STATUS_REFUSED_EXTERNAL_REASON                  92
#define WLAN_STATUS_REFUSED_AP_OUT_OF_MEMORY                 93
#define WLAN_STATUS_REJECTED_EMERGENCY_SERVICE_NOT_SUPPORTED 94
#define WLAN_STATUS_QUERY_RESP_OUTSTANDING                   95
   #define WLAN_STATUS_REJECT_DSE_BAND                       96
   #define WLAN_STATUS_TCLAS_PROCESSING_TERMINATED           97
#define WLAN_STATUS_TS_SCHEDULE_CONFLICT                     98
#define WLAN_STATUS_DENIED_WITH_SUGGESTED_BAND_AND_CHANNEL   99
#define WLAN_STATUS_MCCAOP_RESERVATION_CONFLICT              100
#define WLAN_STATUS_MAF_LIMIT_EXCEEDED                       101
#define WLAN_STATUS_MCCA_TRACK_LIMIT_EXCEEDED                102
#define WLAN_STATUS_DENIED_DUE_TO_SPECTRUM_MANAGEMENT        103
#define WLAN_STATUS_ASSOC_DENIED_NO_VHT                      104
#define WLAN_STATUS_ENABLEMENT_DENIED                        105
#define WLAN_STATUS_RESTRICTION_FROM_AUTHORIZED_GDB          106
#define WLAN_STATUS_AUTHORIZATION_DEENABLED                  107
#define WLAN_STATUS_FILS_AUTHENTICATION_FAILURE              112
#define WLAN_STATUS_UNKNOWN_AUTHENTICATION_SERVER            113
#define WLAN_STATUS_UNKNOWN_PASSWORD_IDENTIFIER              123

/* Reason codes (IEEE Std 802.11-2016, 9.4.1.7, Table 9-45) */
#define WLAN_REASON_UNSPECIFIED                              1
#define WLAN_REASON_PREV_AUTH_NOT_VALID                      2
#define WLAN_REASON_DEAUTH_LEAVING                           3
#define WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY               4
#define WLAN_REASON_DISASSOC_AP_BUSY                         5
#define WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA            6
#define WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA           7
#define WLAN_REASON_DISASSOC_STA_HAS_LEFT                    8
#define WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH               9
#define WLAN_REASON_PWR_CAPABILITY_NOT_VALID                 10
#define WLAN_REASON_SUPPORTED_CHANNEL_NOT_VALID              11
#define WLAN_REASON_BSS_TRANSITION_DISASSOC                  12
#define WLAN_REASON_INVALID_IE                               13
#define WLAN_REASON_MICHAEL_MIC_FAILURE                      14
#define WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT                   15
#define WLAN_REASON_GROUP_KEY_UPDATE_TIMEOUT                 16
#define WLAN_REASON_IE_IN_4WAY_DIFFERS                       17
#define WLAN_REASON_GROUP_CIPHER_NOT_VALID                   18
#define WLAN_REASON_PAIRWISE_CIPHER_NOT_VALID                19
#define WLAN_REASON_AKMP_NOT_VALID                           20
#define WLAN_REASON_UNSUPPORTED_RSN_IE_VERSION               21
#define WLAN_REASON_INVALID_RSN_IE_CAPAB                     22
#define WLAN_REASON_IEEE_802_1X_AUTH_FAILED                  23
#define WLAN_REASON_CIPHER_SUITE_REJECTED                    24
#define WLAN_REASON_TDLS_TEARDOWN_UNREACHABLE                25
#define WLAN_REASON_TDLS_TEARDOWN_UNSPECIFIED                26
#define WLAN_REASON_SSP_REQUESTED_DISASSOC                   27
#define WLAN_REASON_NO_SSP_ROAMING_AGREEMENT                 28
#define WLAN_REASON_BAD_CIPHER_OR_AKM                        29
#define WLAN_REASON_NOT_AUTHORIZED_THIS_LOCATION             30
#define WLAN_REASON_SERVICE_CHANGE_PRECLUDES_TS              31
#define WLAN_REASON_UNSPECIFIED_QOS_REASON                   32
#define WLAN_REASON_NOT_ENOUGH_BANDWIDTH                     33
#define WLAN_REASON_DISASSOC_LOW_ACK                         34
#define WLAN_REASON_EXCEEDED_TXOP                            35
#define WLAN_REASON_STA_LEAVING                              36
#define WLAN_REASON_END_TS_BA_DLS                            37
#define WLAN_REASON_UNKNOWN_TS_BA                            38
#define WLAN_REASON_TIMEOUT                                  39
#define WLAN_REASON_PEERKEY_MISMATCH                         45
#define WLAN_REASON_AUTHORIZED_ACCESS_LIMIT_REACHED          46
#define WLAN_REASON_EXTERNAL_SERVICE_REQUIREMENTS            47
#define WLAN_REASON_INVALID_FT_ACTION_FRAME_COUNT            48
#define WLAN_REASON_INVALID_PMKID                            49
#define WLAN_REASON_INVALID_MDE                              50
#define WLAN_REASON_INVALID_FTE                              51
#define WLAN_REASON_MESH_PEERING_CANCELLED                   52
#define WLAN_REASON_MESH_MAX_PEERS                           53
#define WLAN_REASON_MESH_CONFIG_POLICY_VIOLATION             54
#define WLAN_REASON_MESH_CLOSE_RCVD                          55
#define WLAN_REASON_MESH_MAX_RETRIES                         56
#define WLAN_REASON_MESH_CONFIRM_TIMEOUT                     57
#define WLAN_REASON_MESH_INVALID_GTK                         58
#define WLAN_REASON_MESH_INCONSISTENT_PARAMS                 59
#define WLAN_REASON_MESH_INVALID_SECURITY_CAP                60
#define WLAN_REASON_MESH_PATH_ERROR_NO_PROXY_INFO            61
#define WLAN_REASON_MESH_PATH_ERROR_NO_FORWARDING_INFO       62
#define WLAN_REASON_MESH_PATH_ERROR_DEST_UNREACHABLE         63
#define WLAN_REASON_MAC_ADDRESS_ALREADY_EXISTS_IN_MBSS       64
#define WLAN_REASON_MESH_CHANNEL_SWITCH_REGULATORY_REQ       65
#define WLAN_REASON_MESH_CHANNEL_SWITCH_UNSPECIFIED          66

struct ieee80211_frame {
	uint8_t i_fc[2];
	uint8_t i_dur[2];
	uint8_t i_addr1[IEEE80211_ADDR_LEN];
	uint8_t i_addr2[IEEE80211_ADDR_LEN];
	uint8_t i_addr3[IEEE80211_ADDR_LEN];
	uint8_t i_seq[2];
	/* possibly followed by addr4[IEEE80211_ADDR_LEN]; */
} __packed;

/**
 * @brief Wlan event definition
 */
typedef enum wlan_event {
	WLAN_EVENT_CONNECTED,
	WLAN_EVENT_DISCONNECTED,
	WLAN_EVENT_SCAN_SUCCESS,
	WLAN_EVENT_SCAN_FAILED,
	WLAN_EVENT_4WAY_HANDSHAKE_FAILED,
	WLAN_EVENT_SSID_NOT_FOUND,
	WLAN_EVENT_AUTH_TIMEOUT,
	WLAN_EVENT_ASSOC_TIMEOUT,
	WLAN_EVENT_CONNECT_FAILED,
	WLAN_EVENT_CONNECTION_LOSS,
	WLAN_EVENT_ASSOC_FAILED,
	WLAN_EVENT_SAE_COMMIT_FAILED,
	WLAN_EVENT_SAE_CONFIRM_FAILED,
	WLAN_EVENT_P2P_WAKE_UP,
	WLAN_EVENT_KEEPALIVE_CONNECT_LOST,
#ifdef STA_SOFTAP_COEXIST
	WLAN_EVENT_AP_START,
	WLAN_EVENT_AP_STOP,
	WLAN_EVENT_SELECT_BSS,
#endif

	WLAN_EVENT_DEV_HANG,

	WLAN_EVENT_AP_STA_CONNECTED,
	WLAN_EVENT_AP_STA_DISCONNECTED,
} wlan_event_t;

/**
 * @brief Wlan power state definition
 */
typedef enum wlan_pm_state {
	WLAN_PM_MODE_ON         = 0,
	WLAN_PM_MODE_SUSPENDED  = 1,
	WLAN_PM_MODE_RESUMED    = 2,
	WLAN_PM_MODE_OFF        = 3,
	WLAN_PM_MODE_MAX        = 4,
} wlan_pm_state_t;

/**
 * @brief Wlan ssid definition
 */
typedef struct wlan_ssid {
	uint8_t ssid[WLAN_SSID_MAX_LEN];
	uint8_t ssid_len;
} wlan_ssid_t;

/**
 * @brief Wlan bssid definition
 */
typedef struct wlan_bssid {
	uint8_t bssid[WLAN_MAC_ADDR_LEN];
	uint8_t bssid_set;
} wlan_bssid_t;

/**
 * @brief Wlan station configuration field definition
 */
typedef enum wlan_sta_field {
	WLAN_STA_FIELD_SSID = 0,
	WLAN_STA_FIELD_PSK,
	WLAN_STA_FIELD_WEP_KEY0,
	WLAN_STA_FIELD_WEP_KEY1,
	WLAN_STA_FIELD_WEP_KEY2,
	WLAN_STA_FIELD_WEP_KEY3,
	WLAN_STA_FIELD_WEP_KEY_INDEX,
	WLAN_STA_FIELD_KEY_MGMT,
	WLAN_STA_FIELD_PAIRWISE_CIPHER,
	WLAN_STA_FIELD_GROUP_CIPHER,
	WLAN_STA_FIELD_GROUP_MGMT_CIPHER,
	WLAN_STA_FIELD_PROTO,
	WLAN_STA_FIELD_AUTH_ALG,
	WLAN_STA_FIELD_IEEE80211W,
	WLAN_STA_FIELD_WPA_PTK_REKEY,
	WLAN_STA_FIELD_SCAN_SSID,
	WLAN_STA_FIELD_BSSID,

	WLAN_STA_FIELD_NUM,
} wlan_sta_field_t;

/**
 * @brief Wlan station configuration definition
 */
typedef struct wlan_sta_config {
	wlan_sta_field_t field;

	union {
		/**
		 * Network name
		 */
		wlan_ssid_t ssid;

		/**
		 * bssid - BSSID
		 *
		 * If set, this network block is used only when associating with the AP
		 * using the configured BSSID
		 */
		wlan_bssid_t bssid;

		/**
		 * WPA preshared key in one of the optional formats:
		 *   - an ASCII string of passphrase, length is [8, 63]
		 *   - a hex string of PSK (two characters per octet of PSK), length is 64
		 */
		uint8_t psk[65];

		/**
		 * WEP key in one of the optional formats:
		 *   - an ASCII string with double quotation, length is {5, 13}
		 *   - a hex string (two characters per octet), length is {10, 26}
		 */
		uint8_t wep_key[WLAN_WEP_KEY_MAX_LEN * 2 + 1];

		/**
		 * Default key index for TX frames using WEP
		 */
		int wep_tx_keyidx;

		/**
		 * Bitfield of allowed key management protocols
		 *
		 * WPA_KEY_MGMT_*
		 */
		int key_mgmt;

		/**
		 * Bitfield of allowed pairwise ciphers, WPA_CIPHER_*
		 */
		int pairwise_cipher;

		/**
		 * Bitfield of allowed group ciphers, WPA_CIPHER_*
		 */
		int group_cipher;

		/**
		 * Bitfield of allowed group management ciphers
		 *
		 * This is a bitfield of WPA_CIPHER_AES_128_CMAC and WPA_CIPHER_BIP_*
		 * values. If 0, no constraint is used for the cipher, i.e., whatever
		 * the AP uses is accepted.
		 */
		int group_mgmt_cipher;

		/**
		 * Bitfield of allowed protocols
		 *
		 * WPA_PROTO_*
		 */
		int proto;

		/**
		 * Bitfield of allowed authentication algorithms
		 *
		 * WPA_AUTH_ALG_*
		 */
		int auth_alg;

		/**
		 * Maximum lifetime for PTK in seconds
		 *
		 * This value can be used to enforce rekeying of PTK to
		 * mitigate some attacks against TKIP deficiencies.
		 */
		int wpa_ptk_rekey;

		/**
		 * Scan this SSID with Probe Requests
		 *
		 * scan_ssid can be used to scan for APs using hidden SSIDs.
		 */
		int scan_ssid;

		/**
		 * ieee80211w - Whether management frame protection is enabled
		 *
		 * This value is used to configure policy for management frame
		 * protection (IEEE 802.11w). 0 = disabled, 1 = optional, 2 = required.
		 */
		enum mfp_options ieee80211w;
	} u;
} wlan_sta_config_t;

/**
 * @brief Wlan station connection state definition
 */
typedef enum wlan_sta_states {
	WLAN_STA_STATE_DISCONNECTED = 0,
	WLAN_STA_STATE_CONNECTED = 1,
} wlan_sta_states_t;

/**
 * @brief WLAN AP status flags definition
 */
#define WLAN_STA_AP_STATUS_USE_WPA3   WPA_BIT(0)

/**
 * @brief Wlan AP information definition
 */
typedef struct wlan_sta_ap {
	wlan_ssid_t    ssid;
	uint8_t        bssid[6];
	uint8_t        channel;
	uint8_t        status;   /* WLAN AP status flags */
	uint16_t       beacon_int;
	int            freq;
	int            rssi;     /* unit is 0.5db */
	int            level;    /* signal level, unit is dbm */
	int            wpa_flags;
	int            wpa_cipher;
	int            wpa_key_mgmt;
	int            rsn_cipher;   /* rsn: WPA2/WPA3 */
	int            rsn_key_mgmt; /* rsn: WPA2/WPA3 */
} wlan_sta_ap_t;

/**
 * @brief Wlan station scan parameters definition
 */
typedef struct wlan_sta_scan_param {
	uint8_t scan_only;    /* do scan only */
	uint8_t scan_passive; /* passive scan */
	uint8_t scan_ssid;    /* Scan SSID of configured network with Probe Requests */
} wlan_sta_scan_param_t;

/**
 * @brief Wlan station scan results definition
 */
typedef struct wlan_sta_scan_results {
	wlan_sta_ap_t *ap;
	int size;
	int num;
} wlan_sta_scan_results_t;

/**
 * @brief Wlan station bss infomation definition
 */
typedef struct wlan_sta_bss_info {
	uint8_t *bss;
	uint32_t size;
} wlan_sta_bss_info_t;

/**
 * @brief Parameter of generating WPA PSK based on passphrase and SSID
 */
typedef struct wlan_gen_psk_param {
	uint8_t ssid[WLAN_SSID_MAX_LEN];
	uint8_t ssid_len;
	char passphrase[WLAN_PASSPHRASE_MAX_LEN + 1];
	uint8_t psk[WLAN_PSK_HEX_LEN]; /* out */
} wlan_gen_psk_param_t;

/**
 * @brief Wlan station SSID, passphrase and WPA PSK infomation definition
 */
typedef struct wlan_ssid_psk {
	uint8_t ssid[WLAN_SSID_MAX_LEN];
	uint8_t ssid_len;
	char passphrase[WLAN_PASSPHRASE_MAX_LEN + 1];
	uint8_t psk_valid;
	uint8_t psk[WLAN_PSK_HEX_LEN];
} wlan_ssid_psk_t;

/**
 * @brief Wlan WPS pin definition
 */
typedef struct wlan_sta_wps_pin {
	uint8_t pin[9];
} wlan_sta_wps_pin_t;

/**
 * @brief Wlan AP configuration field definition
 */
typedef enum wlan_ap_field {
	WLAN_AP_FIELD_SSID = 0,
	WLAN_AP_FIELD_PSK,
	WLAN_AP_FIELD_KEY_MGMT,
	WLAN_AP_FIELD_WPA_PAIRWISE_CIPHER,
	WLAN_AP_FIELD_RSN_PAIRWISE_CIPHER,
	WLAN_AP_FIELD_PROTO,
	WLAN_AP_FIELD_AUTH_ALG,
	WLAN_AP_FIELD_GROUP_REKEY,
	WLAN_AP_FIELD_STRICT_REKEY,
	WLAN_AP_FIELD_GMK_REKEY,
	WLAN_AP_FIELD_PTK_REKEY,
	WLAN_AP_FIELD_HW_MODE,
	WLAN_AP_FIELD_IEEE80211N,
	WLAN_AP_FIELD_CHANNEL,
	WLAN_AP_FIELD_BEACON_INT,
	WLAN_AP_FIELD_DTIM,
	WLAN_AP_FIELD_MAX_NUM_STA,

	WLAN_AP_FIELD_NUM,
} wlan_ap_field_t;

/* for compatibility only */
#define WLAN_AP_FIELD_WPA_CIPHER    WLAN_AP_FIELD_WPA_PAIRWISE_CIPHER
#define WLAN_AP_FIELD_RSN_CIPHER    WLAN_AP_FIELD_RSN_PAIRWISE_CIPHER

/**
 * @brief Wlan AP hardware mode definition
 */
typedef enum wlan_ap_hw_mode {
	WLAN_AP_HW_MODE_IEEE80211B = 0,
	WLAN_AP_HW_MODE_IEEE80211G,
	WLAN_AP_HW_MODE_IEEE80211A,
	WLAN_AP_HW_MODE_IEEE80211AD,
	WLAN_AP_HW_MODE_IEEE80211ANY,

	WLAN_AP_HW_MODE_NUM,
} wlan_ap_hw_mode_t;

/**
 * @brief Wlan AP configuration definition
 */
typedef struct wlan_ap_config {
	wlan_ap_field_t field;

	union {
		/**
		 * Network name
		 */
		wlan_ssid_t ssid;

		/**
		 * WPA preshared key in one of the optional formats:
		 *   - an ASCII string of passphrase, length is [8, 63]
		 *   - a hex string of PSK (two characters per octet of PSK), length is 64
		 */
		uint8_t psk[65];

		/**
		 * Bitfield of allowed key management protocols
		 *
		 * WPA_KEY_MGMT_*
		 */
		int key_mgmt;

		/**
		 * Bitfield of allowed WPA pairwise ciphers
		 *
		 * WPA_CIPHER_*
		 */
		int wpa_cipher;

		/**
		 * Bitfield of allowed RSN pairwise ciphers
		 *
		 * WPA_CIPHER_*
		 */
		int rsn_cipher;

		/**
		 * Bitfield of allowed protocols
		 *
		 * WPA_PROTO_*
		 */
		int proto;

		/**
		 * Bitfield of allowed authentication algorithms
		 *
		 * WPA_AUTH_ALG_*
		 */
		int auth_alg;

		/**
		 * Maximum lifetime for GTK in seconds
		 */
		int group_rekey;

		/**
		 * Rekey GTK when any STA that possesses the current GTK is
		 * leaving the BSS
		 */
		int strict_rekey;

		/**
		 * Maximum lifetime for GMK in seconds
		 */
		int gmk_rekey;

		/**
		 * Maximum lifetime for PTK in seconds
		 */
		int ptk_rekey;

		/**
		 * Hardware mode
		 */
		wlan_ap_hw_mode_t hw_mode;

		/**
		 * IEEE802.11n mode
		 */
		int ieee80211n;

		/**
		 * RF channel
		 */
		uint8_t channel;

		/**
		 * MIB defines range as 1..65535, but very small values
		 * cause problems with the current implementation.
		 * Since it is unlikely that this small numbers are
		 * useful in real life scenarios, do not allow beacon
		 * period to be set below 15 TU.
		 */
		uint16_t beacon_int;

		/**
		 * Delivery traffic indication message
		 */
		int dtim;

		/**
		 * Maximum number of STAs in station table
		 */
		int max_num_sta;
	} u;
} wlan_ap_config_t;

/**
 * @brief Wlan AP default configuration definition
 */
typedef struct wlan_ap_default_conf {
	/**
	 * Network name
	 */
	char *ssid;

	/**
	 * Network name length
	 */
	uint8_t ssid_len;

	/**
	 * First two octets: country code as described in ISO/IEC 3166-1.
	 * Third octet:
	 *   ' ' (ascii 32): all environments
	 *   'O': Outdoor environemnt only
	 *   'I': Indoor environment only
	 */
	char country[3];

	/**
	 * WPA preshared key in one of the optional formats:
	 *   - an ASCII string of passphrase, length is [8, 63]
	 *   - a hex string of PSK (two characters per octet of PSK), length is 64
	 */
	char *psk;

	/**
	 * Bitfield of allowed key management protocols
	 *
	 * WPA_KEY_MGMT_*
	 */
	int key_mgmt;

	/**
	 * Bitfield of allowed WPA pairwise ciphers
	 *
	 * WPA_CIPHER_*
	 */
	int wpa_pairwise_cipher;

	/**
	 * Bitfield of allowed RSN pairwise ciphers
	 *
	 * WPA_CIPHER_*
	 */
	int rsn_pairwise_cipher;

	/**
	 * Bitfield of allowed protocols
	 *
	 * WPA_PROTO_*
	 */
	int proto;

	/**
	 * Bitfield of allowed authentication algorithms
	 *
	 * WPA_AUTH_ALG_*
	 */
	int auth_alg;

	/**
	 * Maximum lifetime for GTK in seconds
	 */
	int group_rekey;

	/**
	 * Maximum lifetime for GMK in seconds
	 */
	int gmk_rekey;

	/**
	 * Maximum lifetime for PTK in seconds
	 */
	int ptk_rekey;

	/**
	 * Rekey GTK when any STA that possesses the current GTK is
	 * leaving the BSS
	 */
	uint8_t strict_rekey;

	/**
	 * Hardware mode
	 */
	wlan_ap_hw_mode_t hw_mode;

	/**
	 * IEEE802.11n mode
	 */
	uint8_t ieee80211n;
	/**
	 * RF channel
	 */
	uint8_t channel;

	/**
	 * MIB defines range as 1..65535, but very small values
	 * cause problems with the current implementation.
	 * Since it is unlikely that this small numbers are
	 * useful in real life scenarios, do not allow beacon
	 * period to be set below 15 TU.
	 */
	uint16_t beacon_int;

	/**
	 * Delivery traffic indication message
	 */
	uint8_t dtim;

	/**
	 * Maximum number of STAs in station table
	 */
	uint8_t max_num_sta;
} wlan_ap_default_conf_t;

/**
 * @brief Wlan station information definition
 */
typedef struct wlan_ap_sta {
	uint8_t addr[6];
} wlan_ap_sta_t;

/**
 * @brief Wlan connected stations information definition
 */
typedef struct wlan_ap_stas {
	wlan_ap_sta_t *sta;
	int size;
	int num;
} wlan_ap_stas_t;

#ifdef __cplusplus
}
#endif

#endif /* _NET_WLAN_WLAN_DEFS_H_ */
