#ifndef HOSTAP_COMMON_H
#define HOSTAP_COMMON_H

#define BIT(x) (1 << (x))

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"


#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */
#endif /* ETH_P_PAE */



/* IEEE 802.11 defines */

#define WLAN_FC_PVER (BIT(1) | BIT(0))
#define WLAN_FC_TODS BIT(8)
#define WLAN_FC_FROMDS BIT(9)
#define WLAN_FC_MOREFRAG BIT(10)
#define WLAN_FC_RETRY BIT(11)
#define WLAN_FC_PWRMGT BIT(12)
#define WLAN_FC_MOREDATA BIT(13)
#define WLAN_FC_ISWEP BIT(14)
#define WLAN_FC_ORDER BIT(15)

#define WLAN_FC_GET_TYPE(fc) (((fc) & (BIT(3) | BIT(2))) >> 2)
#define WLAN_FC_GET_STYPE(fc) \
	(((fc) & (BIT(7) | BIT(6) | BIT(5) | BIT(4))) >> 4)

#define WLAN_GET_SEQ_FRAG(seq) ((seq) & (BIT(3) | BIT(2) | BIT(1) | BIT(0)))
#define WLAN_GET_SEQ_SEQ(seq) \
	(((seq) & (~(BIT(3) | BIT(2) | BIT(1) | BIT(0)))) >> 4)

#define WLAN_FC_TYPE_MGMT 0
#define WLAN_FC_TYPE_CTRL 1
#define WLAN_FC_TYPE_DATA 2

/* management */
#define WLAN_FC_STYPE_ASSOC_REQ 0
#define WLAN_FC_STYPE_ASSOC_RESP 1
#define WLAN_FC_STYPE_REASSOC_REQ 2
#define WLAN_FC_STYPE_REASSOC_RESP 3
#define WLAN_FC_STYPE_PROBE_REQ 4
#define WLAN_FC_STYPE_PROBE_RESP 5
#define WLAN_FC_STYPE_BEACON 8
#define WLAN_FC_STYPE_ATIM 9
#define WLAN_FC_STYPE_DISASSOC 10
#define WLAN_FC_STYPE_AUTH 11
#define WLAN_FC_STYPE_DEAUTH 12

/* control */
#define WLAN_FC_STYPE_PSPOLL 10
#define WLAN_FC_STYPE_RTS 11
#define WLAN_FC_STYPE_CTS 12
#define WLAN_FC_STYPE_ACK 13
#define WLAN_FC_STYPE_CFEND 14
#define WLAN_FC_STYPE_CFENDACK 15

/* data */
#define WLAN_FC_STYPE_DATA 0
#define WLAN_FC_STYPE_DATA_CFACK 1
#define WLAN_FC_STYPE_DATA_CFPOLL 2
#define WLAN_FC_STYPE_DATA_CFACKPOLL 3
#define WLAN_FC_STYPE_NULLFUNC 4
#define WLAN_FC_STYPE_CFACK 5
#define WLAN_FC_STYPE_CFPOLL 6
#define WLAN_FC_STYPE_CFACKPOLL 7

/* Authentication algorithms */
#define WLAN_AUTH_OPEN 0
#define WLAN_AUTH_SHARED_KEY 1

#define WLAN_AUTH_CHALLENGE_LEN 128

#define WLAN_CAPABILITY_ESS BIT(0)
#define WLAN_CAPABILITY_IBSS BIT(1)
#define WLAN_CAPABILITY_CF_POLLABLE BIT(2)
#define WLAN_CAPABILITY_CF_POLL_REQUEST BIT(3)
#define WLAN_CAPABILITY_PRIVACY BIT(4)
#define WLAN_CAPABILITY_SHORT_PREAMBLE BIT(5)
#define WLAN_CAPABILITY_PBCC BIT(6)
#define WLAN_CAPABILITY_CHANNEL_AGILITY BIT(7)
#define WLAN_CAPABILITY_SHORT_SLOT_TIME BIT(10)
#define WLAN_CAPABILITY_DSSS_OFDM BIT(13)

/* Status codes */
#define WLAN_STATUS_SUCCESS 0
#define WLAN_STATUS_UNSPECIFIED_FAILURE 1
#define WLAN_STATUS_CAPS_UNSUPPORTED 10
#define WLAN_STATUS_REASSOC_NO_ASSOC 11
#define WLAN_STATUS_ASSOC_DENIED_UNSPEC 12
#define WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG 13
#define WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION 14
#define WLAN_STATUS_CHALLENGE_FAIL 15
#define WLAN_STATUS_AUTH_TIMEOUT 16
#define WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA 17
#define WLAN_STATUS_ASSOC_DENIED_RATES 18
/* 802.11b */
#define WLAN_STATUS_ASSOC_DENIED_NOSHORT 19
#define WLAN_STATUS_ASSOC_DENIED_NOPBCC 20
#define WLAN_STATUS_ASSOC_DENIED_NOAGILITY 21
/* IEEE 802.11i */
#define WLAN_STATUS_INVALID_IE 40
#define WLAN_STATUS_GROUP_CIPHER_NOT_VALID 41
#define WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID 42
#define WLAN_STATUS_AKMP_NOT_VALID 43
#define WLAN_STATUS_UNSUPPORTED_RSN_IE_VERSION 44
#define WLAN_STATUS_INVALID_RSN_IE_CAPAB 45
#define WLAN_STATUS_CIPHER_REJECTED_PER_POLICY 46

/* Reason codes */
#define WLAN_REASON_UNSPECIFIED 1
#define WLAN_REASON_PREV_AUTH_NOT_VALID 2
#define WLAN_REASON_DEAUTH_LEAVING 3
#define WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY 4
#define WLAN_REASON_DISASSOC_AP_BUSY 5
#define WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA 6
#define WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA 7
#define WLAN_REASON_DISASSOC_STA_HAS_LEFT 8
#define WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH 9
/* IEEE 802.11i */
#define WLAN_REASON_INVALID_IE 13
#define WLAN_REASON_MICHAEL_MIC_FAILURE 14
#define WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT 15
#define WLAN_REASON_GROUP_KEY_UPDATE_TIMEOUT 16
#define WLAN_REASON_IE_IN_4WAY_DIFFERS 17
#define WLAN_REASON_GROUP_CIPHER_NOT_VALID 18
#define WLAN_REASON_PAIRWISE_CIPHER_NOT_VALID 19
#define WLAN_REASON_AKMP_NOT_VALID 20
#define WLAN_REASON_UNSUPPORTED_RSN_IE_VERSION 21
#define WLAN_REASON_INVALID_RSN_IE_CAPAB 22
#define WLAN_REASON_IEEE_802_1X_AUTH_FAILED 23
#define WLAN_REASON_CIPHER_SUITE_REJECTED 24


/* Information Element IDs */
#define WLAN_EID_SSID 0
#define WLAN_EID_SUPP_RATES 1
#define WLAN_EID_FH_PARAMS 2
#define WLAN_EID_DS_PARAMS 3
#define WLAN_EID_CF_PARAMS 4
#define WLAN_EID_TIM 5
#define WLAN_EID_IBSS_PARAMS 6
#define WLAN_EID_CHALLENGE 16
#define WLAN_EID_ERP_INFO 42
#ifdef AP_WPA2
#define WLAN_EID_WPA2 48
#endif
#define WLAN_EID_EXT_SUPP_RATES 50
#define WLAN_EID_GENERIC 221


/* netdevice private ioctls (used, e.g., with iwpriv from user space) */

#if WIRELESS_EXT >= 12

/* New wireless extensions API - SET/GET convention (even ioctl numbers are
 * root only)
 */
#define MVWLAN_IOCTL_SET_PARAM      (SIOCIWFIRSTPRIV + 0)
#define MVWLAN_IOCTL_GET_PARAM      (SIOCIWFIRSTPRIV + 1)
#define MVWLAN_IOCTL_WRITEMIF       (SIOCIWFIRSTPRIV + 2)
#define MVWLAN_IOCTL_READMIF        (SIOCIWFIRSTPRIV + 3)
#define MVWLAN_IOCTL_MONITOR        (SIOCIWFIRSTPRIV + 4)
#define MVWLAN_IOCTL_RESET          (SIOCIWFIRSTPRIV + 5)
#define MVWLAN_IOCTL_WDS_ADD        (SIOCIWFIRSTPRIV + 6)
#define MVWLAN_IOCTL_WDS_DEL        (SIOCIWFIRSTPRIV + 7)
#define MVWLAN_IOCTL_MACCMD         (SIOCIWFIRSTPRIV + 8)
#define MVWLAN_IOCTL_ADDMAC         (SIOCIWFIRSTPRIV + 9)
#define MVWLAN_IOCTL_DELMAC         (SIOCIWFIRSTPRIV + 10)
#define MVWLAN_IOCTL_KICKMAC        (SIOCIWFIRSTPRIV + 11)

/* following are not in SIOCGIWPRIV list; check permission in the driver code
 */
#define MVWLAN_IOCTL_HOSTAPD        (SIOCDEVPRIVATE + 12)

#else /* WIRELESS_EXT >= 12 */

/* Old wireless extensions API - check permission in the driver code */
#define MVWLAN_IOCTL_MONITOR        (SIOCDEVPRIVATE)
#define MVWLAN_IOCTL_SET_PARAM      (SIOCDEVPRIVATE + 1)
#define MVWLAN_IOCTL_READMIF        (SIOCDEVPRIVATE + 2)
#define MVWLAN_IOCTL_WRITEMIF       (SIOCDEVPRIVATE + 3)
#define MVWLAN_IOCTL_RESET          (SIOCDEVPRIVATE + 4)
#define MVWLAN_IOCTL_WDS_ADD        (SIOCDEVPRIVATE + 5)
#define MVWLAN_IOCTL_WDS_DEL        (SIOCDEVPRIVATE + 6)
#define MVWLAN_IOCTL_MACCMD         (SIOCDEVPRIVATE + 7)
#define MVWLAN_IOCTL_ADDMAC         (SIOCDEVPRIVATE + 8)
#define MVWLAN_IOCTL_DELMAC         (SIOCDEVPRIVATE + 9)
#define MVWLAN_IOCTL_KICKMAC        (SIOCDEVPRIVATE + 10)
#define MVWLAN_IOCTL_HOSTAPD        (SIOCDEVPRIVATE + 11)

#endif /* WIRELESS_EXT >= 12 */


/* MVWLAN_IOCTL_SET_PARAM ioctl() subtypes: */
enum {
	MVWLAN_PARAM_APMODE = 1,
	MVWLAN_PARAM_BEACON_INT,
	MVWLAN_PARAM_DTIM_PERIOD,
	MVWLAN_PARAM_TXPOWER,
	MVWLAN_PARAM_DUMP,
	MVWLAN_PARAM_OTHER_AP_POLICY,
	MVWLAN_PARAM_AP_MAX_INACTIVITY,
	MVWLAN_PARAM_AP_BRIDGE_PACKETS,
	MVWLAN_PARAM_AP_NULLFUNC_ACK,
	MVWLAN_PARAM_MAX_WDS,
	MVWLAN_PARAM_AP_AUTOM_AP_WDS,
	MVWLAN_PARAM_AUTH_MODE,
	MVWLAN_PARAM_HOST_ENCRYPT,
	MVWLAN_PARAM_HOST_DECRYPT,
	MVWLAN_PARAM_HOST_ROAMING,
	MVWLAN_PARAM_IEEE_802_1X,
	MVWLAN_PARAM_WDS_TYPE,
	MVWLAN_PARAM_HOSTSCAN,
	MVWLAN_PARAM_AP_SCAN,
	MVWLAN_PARAM_ANTSEL_TX,
	MVWLAN_PARAM_ANTSEL_RX,
	MVWLAN_PARAM_ENH_SEC,
	MVWLAN_PARAM_BASIC_RATES,
	MVWLAN_PARAM_OPER_RATES,
	MVWLAN_PARAM_HOSTAPD
};

enum {
	MVWLAN_RX_ANTENNA,
	MVWLAN_TX_ANTENNA,
};

enum {
	MVWLAN_ANTSEL_DO_NOT_TOUCH = 0, 
	MVWLAN_ANTSEL_DIVERSITY = 1,
	MVWLAN_ANTSEL_LOW = 2, 
	MVWLAN_ANTSEL_HIGH = 3
};

enum {
	MVWLAN_AP_MODE_B_ONLY,
	MVWLAN_AP_MODE_G_ONLY,
	MVWLAN_AP_MODE_MIXED
};


enum {
	MVWLAN_AUTH_MODE_OPEN_SYSTEM,
	MVWLAN_AUTH_MODE_SHARED_KEY,
	MVWLAN_AUTH_MODE_BOTH
};


enum {
	MVWLAN_ENCRYPT_TYPE_NONE,
	MVWLAN_ENCRYPT_TYPE_WEP,
	MVWLAN_ENCRYPT_TYPE_WPA,
	MVWLAN_ENCRYPT_TYPE_OTHER
};

enum { HOSTAP_ANTSEL_DO_NOT_TOUCH = 0, HOSTAP_ANTSEL_DIVERSITY = 1,
       HOSTAP_ANTSEL_LOW = 2, HOSTAP_ANTSEL_HIGH = 3 };


/* MVWLAN_IOCTL_MACCMD ioctl() subcommands: */
enum { AP_MAC_CMD_POLICY_OPEN = 0, AP_MAC_CMD_POLICY_ALLOW = 1,
       AP_MAC_CMD_POLICY_DENY = 2, AP_MAC_CMD_FLUSH = 3,
       AP_MAC_CMD_KICKALL = 4 };


/* MVWLAN_IOCTL_HOSTAPD ioctl() cmd: */
enum {
	MVWLAN_HOSTAPD_FLUSH = 1,
	MVWLAN_HOSTAPD_ADD_STA = 2,
	MVWLAN_HOSTAPD_REMOVE_STA = 3,
	MVWLAN_HOSTAPD_GET_INFO_STA = 4,
	/* REMOVED: MVWLAN_HOSTAPD_RESET_TXEXC_STA = 5, */
	MVWLAN_SET_ENCRYPTION = 6,
	MVWLAN_GET_ENCRYPTION = 7,
	MVWLAN_HOSTAPD_SET_FLAGS_STA = 8,
	MVWLAN_HOSTAPD_GET_CMD = 9,
	MVWLAN_HOSTAPD_SET_CMD = 10,
	MVWLAN_HOSTAPD_SET_ASSOC_AP_ADDR = 11,
	MVWLAN_HOSTAPD_SET_PTK_STA = 12
};


/* mfg data struct*/
#define SZ_PHY_ADDR         6   /* Number of bytes in ethernet MAC address */
#define SZ_BOARD_NAME       8
#define SZ_BOOT_VERSION     12
#define SZ_PRODUCT_ID       58
#define SZ_INTERNAL_PA_CFG  14
#define SZ_EXTERNAL_PA_CFG  1
#define SZ_CCA_CFG          8
#define SZ_LED              4
#define SZ_DEVICE_NAME      16
#define SZ_SSID             32

/* Manufature data structure */
struct mvwlan_mfg_param {
	u8 board_descr[SZ_BOARD_NAME];  	/* 8 byte ASCII to descript 
						 * the type and version of 
						 * the board */
	u8 revision;
	u8 pa_options;
	u8 ext_pa[SZ_EXTERNAL_PA_CFG];
	u8 antenna_cfg;
	u16 int_pa[SZ_INTERNAL_PA_CFG];
	u8 cca[SZ_CCA_CFG];
	u16 domain;                     	/* Wireless domain */
	u16 customer_options;
	u8 led[SZ_LED];
	u16 xosc;
	u8 reserved_1[2];
	u16 magic;
	u16 check_sum;
	u8 mfg_mac_addr[SZ_PHY_ADDR];   	/* Mfg mac address */
	u8 reserved_2[4];
	u8 pid[SZ_PRODUCT_ID];          	/* Production ID */
	u8 boot_version[SZ_BOOT_VERSION];
};

/* Configuration data structure */
struct mvwlan_cfg_param {
	u8 ssid[SZ_SSID];             	/* 32 byte string */
	u32 oper_rate_set;      	/* bit string */
	u32 basic_rate_set;      	/* bit string */
	u8 dtim_period;          	/* 1 to 255 */
	u16 beacon_interval;     	/* 1 to 65535 */
	u16 rts_threshold;       	/* 0 to 2347 */
	u16 fragment_threshold;      	/* 256 to 2346 */
	u8 short_retry_limit;   	/* 1 to 255 */
	u8 long_retry_limit;   		/* 1 to 255 */
	u8 privacy_invoked;     	/* 0=disable, 1=enable */
	u8 exclude_unencrypted;  	/* 0=acept, 1=exclude */
	u8 auth_mode;            	/* 0=open, 1=shared key,
					 * 2=open or shared key */
	u8 hide_ssid;            	/* SSID blanking, true or false */
	u8 wep_key_size;         	/* 0=not_set, 1=40_bit, 2=104_bit */
	u8 wep_default_key_index;
	u8 wep_default_key1[13];
	u8 wep_default_key2[13];
	u8 wep_default_key3[13];
	u8 wep_default_key4[13];

	u8 channel;              	/* 1 to 14 */
	u8 current_tx_power_level;	/* 1 to 8 */
	u16 tx_power_level1;         	/* 0 to 10000 */
	u16 tx_power_level2;         	/* 0 to 10000 */

	u16 tx_power_level3;         	/* 0 to 10000 */
	u16 tx_power_level4;         	/* 0 to 10000 */
	u16 tx_power_level5;         	/* 0 to 10000 */
	u16 tx_power_level6;       	/* 0 to 10000 */
	u16 tx_power_level7;       	/* 0 to 10000 */
	u16 tx_power_level8;       	/* 0 to 10000 */
	u8 rx_antenna;
	u8 tx_antenna;
	u8 short_preamble;       	/* Use short preample, true or false
					 * This has no effect when AP is
					 * running in 802.11g only mode */
	u8 fixed_tx_data_rate;    	/* fixed tx data rate */
	u8 fixed_tx_b_rate_idx;   /* fixed tx rate index for B client */
	u8 fixed_tx_g_rate_idx;   /* fixed rx rate index for G client */

	u8 mac_addr[SZ_PHY_ADDR];   	/* MAC address */
	u8 dev_name[SZ_DEVICE_NAME];	/* device name */
	u8 wireless_enable;
	u8 auto_link;
	u8 ssid_patch;
	u8 speed_booster;
	u8 ap_mode;               	/* 0=B only, 1=G only, 2=Mixed */
	u8 iw_mode;              	/* iw mode */
	u8 g_protect;			/* G (ERP) Protect Mode */
	u8 wpa_mode;            	/* WPA mode */
	u8 wpa_encry;			/* WPA data encryption */
	u8 wpa_passphrase[64];		/* WPA pass phrase */
#ifndef AP_WPA2
	u32 wpa_group_rekey_time;       /* WPA group key rekey time (in second) */
#else
	u32 group_rekey_time;           /* group key rekey time (in second) */
	u8 wpa2_mode;            	/* WPA2 mode */
	u8 wpa2_encry;			/* WPA2 data encryption */
	u8 wpa2_passphrase[64];		/* WPA2 pass phrase */
#endif
	u8 wlan_tx_gpio;		/* GPIO for WLAN LED of TX */
	u8 wlan_rx_gpio;		/* GPIO for WLAN LED of RX */
	u8 antenna;			/* antenna selection 1: antenna A, 2: antenna B*/
	u8 watchdog_timer;
    u8 sta_bridge;      /* 1: to bridge between wireless STAs, 0: not bridge between wireless STAs */
};

#define MVWLAN_HOSTAPD_MAX_BUF_SIZE     1024
#define MVWLAN_CRYPT_ALG_NAME_LEN       16

struct mvwlan_hostapd_param {
	u32 cmd;
	u8 sta_addr[ETH_ALEN];
	u8 reserved[2];
	union {
		struct {
			u16 aid;
			u16 capability;
			u32 tx_supp_rates;
		} add_sta;
		struct {
			u32 inactive_sec;
		} get_info_sta;
		struct {
			u8 alg[MVWLAN_CRYPT_ALG_NAME_LEN];
			u32 flags;
			u32 err;
			u8 idx;
			u16 key_len;
			u8 key[0];
		} crypt;
		struct {
			u32 flags_and;
			u32 flags_or;
		} set_flags_sta;
		struct {
			u32 cmd;
			u32 param;
			u32 len;
			u8 data[0];
		} cmd;
	} u;
};


#define HOSTAP_CRYPT_FLAG_SET_TX_KEY BIT(0)
#define HOSTAP_CRYPT_FLAG_PERMANENT BIT(1)

#define HOSTAP_CRYPT_ERR_UNKNOWN_ALG 2
#define HOSTAP_CRYPT_ERR_UNKNOWN_ADDR 3
#define HOSTAP_CRYPT_ERR_CRYPT_INIT_FAILED 4
#define HOSTAP_CRYPT_ERR_KEY_SET_FAILED 5
#define HOSTAP_CRYPT_ERR_TX_KEY_SET_FAILED 6
#define HOSTAP_CRYPT_ERR_CARD_CONF_FAILED 7


#endif /* HOSTAP_COMMON_H */
