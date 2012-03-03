#ifndef MVWLAN_WLAN_H
#define MVWLAN_WLAN_H

#include "mvWLAN_config.h"
#ifdef __KERNEL__
#include "mvWLAN_compat.h"
#include "mvWLAN_wext.h"
#endif
#include "mvWLAN_crypt.h"

#define MAX_PARM_DEVICES        1	/* No multi devices support due to
					 * the HAL layer */
#define PARM_MIN_MAX            "1-" __MODULE_STRING(MAX_PARM_DEVICES)
#define DEF_INTS
#define GET_INT_PARM(var,idx)   var[var[idx] < 0 ? 0 : idx]
#define DEF_STRS
#define GET_STR_PARM(var,idx)   var[var[idx] == NULL ? 0 : idx]


#define BIT(x)          (1 << (x))

/* Specific skb->protocol value that indicates that the packet already contains
 * txdesc header.
 * FIX: This might be needed but in the current implementation
 * mvWLAN_send_mgmt() sends directly to hardware tx function.
 * ETH_P_CONTROL is commented as "Card specific control frames".
 */
#define ETH_P_MVWLAN    ETH_P_CONTROL

#ifndef ETH_P_PAE
#define ETH_P_PAE       0x888E	/* Port Access Entity (IEEE 802.1X) */
#endif /* ETH_P_PAE */


/* See IEEE 802.1H for LLC/SNAP encapsulation/decapsulation */
/* Ethernet-II snap header (RFC1042 for most EtherTypes)
 * { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 } */
#define rfc1042_header              "\xAA\xAA\x03\x00\x00\x00"
/* Bridge-Tunnel header (for EtherTypes ETH_P_AARP and ETH_P_IPX)
 * { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8 } */
#define bridge_tunnel_header        "\xAA\xAA\x03\x00\x00\xF8"


#define MAC2STR(a)      (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR          "%02x:%02x:%02x:%02x:%02x:%02x"

#define MVWLAN_WEP_KEYS         4
#define MVWLAN_WEP_KEY_LEN      13


#define MVWLAN_MAX_FRAME_SIZE   2304
#define MVWLAN_MIN_MTU          256
/* FIX: */
#define MVWLAN_MAX_MTU  (MVWLAN_MAX_FRAME_SIZE - (6 /* LLC */ + 8 /* WEP */))


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


/* MVWLAN_IOCTL_MACCMD ioctl() subcommands: */
enum {
	AP_MAC_CMD_POLICY_OPEN = 0,
	AP_MAC_CMD_POLICY_ALLOW = 1,
	AP_MAC_CMD_POLICY_DENY = 2,
	AP_MAC_CMD_FLUSH = 3,
	AP_MAC_CMD_KICKALL = 4
};

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
	u32 oper_rate_set;       	/* bit string */
	u32 basic_rate_set;       	/* bit string */
	u8 dtim_period;          	/* 1 to 255 */
	u16 beacon_interval;     	/* 1 to 65535 */
	u16 rts_threshold;       	/* 0 to 2347 */
	u16 frag_threshold;      	/* 256 to 2346 */
	u8 short_retry_limit;   	/* 1 to 255 */
	u8 long_retry_limit;   		/* 1 to 255 */
	u8 privacy_invoked;     	/* 0=disable, 1=enable */
	u8 exclude_unencrypted;  	/* 0=acept, 1=exclude */
	u8 auth_mode;            	/* 1=open, 2=shared key,
					 * 3=open or shared key */
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
#define WPA_ENCRY_TKIP 2
#define WPA_ENCRY_CCMP 4
	u8 wpa_encry;			/* WPA data encryption */
	u8 wpa_passphrase[64];		/* WPA pass phrase */
#ifndef AP_WPA2
	u32 wpa_group_rekey_time;       /* WPA group key rekey time (in second) */
#else
	u32 group_rekey_time;           /* group key rekey time (in second) */
	u8 wpa2_mode;            	/* WPA2 mode */
#define WPA2_ENCRY_TKIP_OR_CCMP 2
#define WPA2_ENCRY_TKIP         2
#define WPA2_ENCRY_CCMP_ONLY    4
#define WPA2_ENCRY_CCMP         4
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

#define MVWLAN_CRYPT_FLAG_SET_TX_KEY        BIT(0)
#define MVWLAN_CRYPT_FLAG_PERMANENT         BIT(1)

#define MVWLAN_CRYPT_ERR_UNKNOWN_ALG        2
#define MVWLAN_CRYPT_ERR_UNKNOWN_ADDR       3
#define MVWLAN_CRYPT_ERR_CRYPT_INIT_FAILED  4
#define MVWLAN_CRYPT_ERR_KEY_SET_FAILED     5
#define MVWLAN_CRYPT_ERR_TX_KEY_SET_FAILED  6
#define MVWLAN_CRYPT_ERR_HW_CONF_FAILED     7


#ifdef __KERNEL__

#define SLOT_TIME_MODE_SHORT        0
#define SLOT_TIME_MODE_LONG         1

#define MVWLAN_MAX_SSID_LEN         32

/* this is assumed to be equal to MVWLAN_MAX_SSID_LEN */
#define MVWLAN_MAX_NAME_LEN         32

#define MVWLAN_DUMP_RX_HDR                  BIT(0)
#define MVWLAN_DUMP_TX_HDR                  BIT(1)

#define MVWLAN_CF_POLLABLE                  BIT(0)
#define MVWLAN_CF_POLL_REQUEST              BIT(1)


typedef struct mvwlan_wds_info mvwlan_wds_info_t;

struct mvwlan_wds_info {
	/* must start with dev, since it is used also as a pointer to whole
	 * mvwlan_wds_info structure */
	struct net_device dev;
	u8 remote_addr[ETH_ALEN];
	struct net_device_stats stats;
	mvwlan_wds_info_t *next;
};


#define MVWLAN_MAX_TX_QUEUE_SIZE    38
#define MVWLAN_MAX_TX_FRAME_SIZE    (3072 - 80 - 2)
#define MVWLAN_MAX_TX_FRAGMENTS     10

#define MVWLAN_TX_QUEUE_BIT_USED    0x0001

struct mvwlan_tx_queue {
	struct list_head list;
	u32 flags;
	WLAN_TX_INFO tx_desc;
	IEEEtypes_Frame_t *tx_frame;
	u32 cookie;
	u32 cb_index;
	u16 frame_body_length;
	/* Real 802.11 Frame starts here. Be careful the underlying hardware
	 * always uses 4 addresses even when the frame type is of management.
	 */
	u8 frame_data[MVWLAN_MAX_TX_FRAME_SIZE];
};


struct mvwlan_tx_callback_info {
	u16 idx;
	void (*func)(struct sk_buff *, int ok, void *);
	void *data;
	struct mvwlan_tx_callback_info *next;
};


/* IEEE 802.11 requires that STA supports concurrent reception of at least
 * three fragmented frames. This define can be increased to support more
 * concurrent frames, but it should be noted that each entry can consume about
 * 2 kB of RAM and increasing cache size will slow down frame reassembly. */
#define MVWLAN_FRAG_CACHE_LEN       4

struct mvwlan_frag_entry {
	unsigned long first_frag_time;
	unsigned int seq;
	unsigned int last_frag;
	struct sk_buff *skb;
	u8 src_addr[ETH_ALEN];
	u8 dst_addr[ETH_ALEN];
};


struct mvwlan_crypt_data {
	struct list_head list; /* delayed deletion list */
	struct mvwlan_crypto_ops *ops;
	void *priv;
	atomic_t refcnt;
};


struct mvwlan_mib_phyHRDSSSTable {
	u8 ShortPreambleOptionImplemented;
	u8 PBCCOptionImplemented;
	u8 ChannelAgilityPresent;
	u8 ChannelAgilityEnabled;
	u8 HRCCAModeSupported;
};

struct mvwlan_mib_phyERPTable {
	u8 ERPBCCOptionImplemented;
	u8 ERPBCCOptionEnabled;
	u8 DSSSOFDMOptionImplemented;
	u8 DSSSOFDMOptionEnabled;
	u8 ShortSlotTimeOptionImplemented;
	u8 ShortSlotTimeOptionEnabled;
};

/* options for hw_shutdown */
#define MVWLAN_HW_NO_DISABLE        BIT(0)

typedef struct local_info local_info_t;

struct mvwlan_helper_functions {
	/* the following functions are from mvWLAN_hw.c, but they may have some
	 * hardware model specific code */
	int (*dev_open)(struct net_device *dev);
	int (*dev_close)(struct net_device *dev);
	int (*get_cmd)(struct net_device *dev, u32 cmd, u32 param, void *buf,
		       int bufLen);
	int (*set_cmd)(struct net_device *dev, u32 cmd, u32 param, void *buf,
		       int len);
	void (*hw_reset)(struct net_device *dev);
	void (*hw_init)(struct net_device *dev);
	int (*hw_enable)(struct net_device *dev, int initial);
	void (*hw_shutdown)(struct net_device *dev, int no_disable);
	int (*tx)(struct sk_buff *skb, struct net_device *dev);
	void (*schedule_reset)(local_info_t *local);
	int (*tx_80211)(struct sk_buff *skb, struct net_device *dev);
};


struct local_info {
	void *fun_sta_existed;
	struct module *hw_module;
	int dev_enabled;
	struct net_device *dev;
	struct net_device_stats stats;

	/* For WDS link configuration protection */
	spinlock_t wdslock;
	/* For TX callback configuration protection */
	spinlock_t cblock;
	/* For TX queue protection */
	spinlock_t txqlock;
	/* For beacon and probe response buffer configuration */
	spinlock_t bcnlock;
	/* For hardware configuration */
	spinlock_t cmdlock;
	/* For any internal linked list operation */
	spinlock_t lock;

	/* bitfield for atomic bitops */
#define MVWLAN_BITS_TX_WEP          0
#define MVWLAN_BITS_TRANSMIT        1
	long bits;

	struct ap_data *ap;

	/* 802.11 system specific configuration */
	WL_SYS_CFG *sysConfig;

	/* 802.11b system specific configuration (not currently in HAL) */
	struct mvwlan_mib_phyHRDSSSTable mib_phyHRDSSSTable;

	/* 802.11g system specific configuration (not currently in HAL) */
	struct mvwlan_mib_phyERPTable mib_phyERPTable;

	u8 mib_DeviceName[16+1];
	u8 mib_ErpProtEnabled;
	u8 mib_ApMode;
	u8 mib_rxAntenna;
	u8 mib_txAntenna;
	u8 auth_mode;

	/* Extra configuration outside the definitions of
	 * the cuttent 802.11 Management Information Base */
	char essid_len;
	u8 wep_key_len;
	u16 channel_mask;
	int wireless_enable;
	int disable_on_close;
	int mtu;
	int frame_dump; /* dump RX/TX frame headers, MVWLAN_DUMP_ flags */
	u32 tx_rate_control;
	u32 basic_rate_set;
	u32 max_basic_rate_idx;
	int fixed_tx_data_rate;
	int fixed_tx_b_rate_idx;
	int fixed_tx_g_rate_idx;
	int wlan_tx_gpio;
	int wlan_rx_gpio;
	int watchdog_timer;
	int hw_resetting;
	int hw_ready;
	int hw_reset_tries; /* how many times reset has been tried */
	int shutdown;

	/* if hardware timeout/error is detected in interrupt context,
	 * reset_queue is used to schedule hardware reseting to be done in
	 * user context */
	MVWLAN_QUEUE reset_queue;

	/* For scheduling a change of the promiscuous mode */
	int is_promisc;
	MVWLAN_QUEUE set_multicast_list_queue;

	/* For WDS */
	mvwlan_wds_info_t *wds; /* list of established wds connections */
	int wds_max_connections;

	int wds_connections;
#define MVWLAN_WDS_BROADCAST_RA     BIT(0)
#define MVWLAN_WDS_AP_CLIENT        BIT(1)
	u32 wds_type;

	/* For Wireless Extension */
	int iw_mode; /* operating mode (IW_MODE_*) */
#ifdef WIRELESS_EXT
	struct iw_statistics wstats;
#if WIRELESS_EXT > 13
	unsigned long scan_timestamp; /* Time started to scan */
#endif /* WIRELESS_EXT > 13 */
#if WIRELESS_EXT > 15
	struct iw_spy_data spy_data; /* iwspy support */
#endif /* WIRELESS_EXT > 15 */
#endif /* WIRELESS_EXT */

	int hostapd; /* whether user space daemon, hostapd, is used for AP
		      * management */

	/* For Access Point device */
	char bssid[ETH_ALEN];
	struct net_device *apdev;
	struct net_device_stats apdevstats;

	/* For Station device */
	char assoc_ap_addr[ETH_ALEN];
	struct net_device *stadev;
	struct net_device_stats stadevstats;

	int host_roaming;
	unsigned long last_join_time; /* time of last JoinRequest */
	struct mvwlan_scan_result *last_scan_results;
	int last_scan_results_count;

	u8 preferred_ap[ETH_ALEN]; /* use this AP if possible */

	/* For monitor mode */
	int monitor_allow_fcserr;

	/* For crypt engine */
	struct mvwlan_crypt_data *crypt;
#ifdef AP_WPA2
	struct mvwlan_crypt_data *tkip_crypt;
	struct mvwlan_crypt_data *ccmp_crypt;
	struct mvwlan_crypt_data *multicast_crypt;
	int ccmp_encry_delay_wait;
#endif
	struct timer_list crypt_deinit_timer;
	struct list_head crypt_deinit_list;

	int host_encrypt;
	int host_decrypt;

	/* For fragmentation */
	struct mvwlan_frag_entry frag_cache[MVWLAN_FRAG_CACHE_LEN];
	unsigned int frag_next_idx;

	int ieee_802_1x; /* is IEEE 802.1X used */

#define MVWLAN_ENHSEC_BCAST_SSID            BIT(0)
	int enh_sec; /* enhanced security options */

	struct mvwlan_helper_functions *func;

	/* Registered TX callbacks */
	struct mvwlan_tx_callback_info *tx_callback;

	/* TX queues */
	struct list_head tx_queue_list; /* TX free queue list head */

	/* Tasklets for handling hardware IRQ related operations outside hw
	 * IRQ handler */
	MVWLAN_TASKLET tx_done_tasklet;
	MVWLAN_TASKLET tx_wep_done_tasklet;

	MVWLAN_TASKLET rx_mgmt_ctrl_tasklet;
	MVWLAN_TASKLET rx_data_tasklet;

#ifdef MVWLAN_CALLBACK
	void *callback_data; /* Can be used in callbacks; e.g., allocate
			      * on enable event and free on disable event.
			      * Host AP driver code does not touch this. */
#endif /* MVWLAN_CALLBACK */

	wait_queue_head_t hostscan_wq;

	/* Passive scan in Host AP mode */
	struct timer_list passive_scan_timer;
	int passive_scan_interval; /* in seconds, 0 = disabled */
	int passive_scan_channel;
	enum { PASSIVE_SCAN_WAIT, PASSIVE_SCAN_LISTEN } passive_scan_state;


	struct timer_list tick_timer;
	unsigned long last_tick_timer;
	unsigned int sw_tick_stuck;

	struct timer_list routine_timer;
	unsigned long last_tx_packets;
	unsigned long last_rx_packets;

    struct mvwlan_cfg_param *cfg_data; /* only valid in device initialize period */    
        
	/* Make sure that the hardware version specific fields are in the end
	 * of the struct (these could also be moved to void *priv or something
	 * like that). */

	/* NOTE! Do not add common entries here after hardware version
	 * specific blocks. */
};


#ifndef MVWLAN_NO_DEBUG

#define DEBUG_FID       BIT(0)
#define DEBUG_PS        BIT(1)
#define DEBUG_FLOW      BIT(2)
#define DEBUG_AP        BIT(3)
#define DEBUG_HW        BIT(4)
#define DEBUG_EXTRA     BIT(5)
#define DEBUG_EXTRA2    BIT(6)
#define DEBUG_PS2       BIT(7)
#define DEBUG_MASK      (DEBUG_PS | DEBUG_FLOW | DEBUG_AP | DEBUG_HW | \
			DEBUG_EXTRA)
#define PDEBUG(n, args...) \
do { if ((n) & DEBUG_MASK) printk(KERN_DEBUG args); } while (0)
#define PDEBUG2(n, args...) \
do { if ((n) & DEBUG_MASK) printk(args); } while (0)

#else /* MVWLAN_NO_DEBUG */

#define PDEBUG(n, args...)
#define PDEBUG2(n, args...)

#endif /* MVWLAN_NO_DEBUG */


#ifdef MVWLAN_CALLBACK
enum {
	/* Called when device is enabled */
	MVWLAN_CALLBACK_ENABLE,

	/* Called when device is disabled */
	MVWLAN_CALLBACK_DISABLE,

	/* Called when RX/TX starts/ends */
	MVWLAN_CALLBACK_RX_START, MVWLAN_CALLBACK_RX_END,
	MVWLAN_CALLBACK_TX_START, MVWLAN_CALLBACK_TX_END
};
void mvWLAN_callback(local_info_t *local, int event);
#else /* MVWLAN_CALLBACK */
#define mvWLAN_callback(d, e) do { } while (0)
#endif /* MVWLAN_CALLBACK */

#ifdef TURBO_SETUP
#define AUTOLINK_RATE      12
#endif

#endif /* __KERNEL__ */

#endif /* MVWLAN_WLAN_H */
