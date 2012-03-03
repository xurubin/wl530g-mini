#ifndef MVWLAN_MGMT_H
#define MVWLAN_MGMT_H

#include "wltypes.h"
#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"
#include "keyMgmtCommon.h"
#include "keyMgmt.h"

/* AP data structures for STAs */

/* maximum number of frames to buffer per STA */
#define STA_MAX_TX_BUFFER           32

/* Flags used in skb->cb[6] to control how the packet is handled in TX path.
 * skb->cb[0..5] must contain magic value 'hostap' to indicate that cb[6] is
 * used. */
#define AP_SKB_CB_MAGIC             "hostap"
#define AP_SKB_CB_MAGIC_LEN         6
#define AP_SKB_CB_BUFFERED_FRAME    BIT(0)


/* STA flags */
#define WLAN_STA_AUTH               BIT(0)
#define WLAN_STA_ASSOC              BIT(1)
#define WLAN_STA_PS                 BIT(2)
 /* TIM bit is on for PS stations */
#define WLAN_STA_TIM                BIT(3)
 /* permanent; do not remove entry on expiration */
#define WLAN_STA_PERM               BIT(4)
 /* If 802.1X is used, this flag is controlling whether STA is authorized to
  * send and receive non-IEEE 802.1X frames */
#define WLAN_STA_AUTHORIZED         BIT(5)
 /* pending activity poll not ACKed */
#define WLAN_STA_PENDING_POLL       BIT(6)

/* Definitions of Supported Rates in bit */
#define WLAN_RATE_1M                BIT(0)
#define WLAN_RATE_2M                BIT(1)
#define WLAN_RATE_5M5               BIT(2)
#define WLAN_RATE_11M               BIT(3)
#define WLAN_RATE_22M               BIT(4)
#define WLAN_RATE_6M                BIT(5)
#define WLAN_RATE_9M                BIT(6)
#define WLAN_RATE_12M               BIT(7)
#define WLAN_RATE_18M               BIT(8)
#define WLAN_RATE_24M               BIT(9)
#define WLAN_RATE_36M               BIT(10)
#define WLAN_RATE_48M               BIT(11)
#define WLAN_RATE_54M               BIT(12)
#define WLAN_RATE_72M               BIT(13)
#define WLAN_RATE_COUNT             14

/* Definitions of channels in bit */
#define WLAN_CHANNEL_1              BIT(0)
#define WLAN_CHANNEL_2              BIT(1)
#define WLAN_CHANNEL_3              BIT(2)
#define WLAN_CHANNEL_4              BIT(3)
#define WLAN_CHANNEL_5              BIT(4)
#define WLAN_CHANNEL_6              BIT(5)
#define WLAN_CHANNEL_7              BIT(6)
#define WLAN_CHANNEL_8              BIT(7)
#define WLAN_CHANNEL_9              BIT(8)
#define WLAN_CHANNEL_10             BIT(9)
#define WLAN_CHANNEL_11             BIT(10)
#define WLAN_CHANNEL_12             BIT(11)
#define WLAN_CHANNEL_13             BIT(12)
#define WLAN_CHANNEL_14             BIT(13)

/* Maximum size of Supported Rates info element. IEEE 802.11 has a limit of 8,
 * but some pre-standard IEEE 802.11g products use longer elements. */
#define WLAN_SUPP_RATES_MAX         32

/* Try to increase TX rate after # successfully sent consecutive packets */
#define WLAN_RATE_UPDATE_COUNT      50

/* Decrease TX rate after # consecutive dropped packets */
#define WLAN_RATE_DECREASE_THRESHOLD    2

struct sta_info {
	struct list_head list;
	struct sta_info *hnext; /* next entry in hash table list */
	atomic_t users; /* number of users (do not remove if > 0) */
	struct proc_dir_entry *proc;

	u8 addr[ETH_ALEN];
	u16 aid; /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
	u32 flags;
	u16 capability;
	u16 listen_interval; /* or beacon_int for APs */
	u8 supported_rates[WLAN_SUPP_RATES_MAX];

	unsigned long last_auth;
	unsigned long last_assoc;
	unsigned long last_rx;
	unsigned long last_tx;
	unsigned long rx_packets, tx_packets;
	unsigned long rx_bytes, tx_bytes;
	struct sk_buff_head tx_buf;
	/* FIX: timeout buffers with an expiry time somehow derived from
	 * listen_interval */

	u8 last_rx_silence;
	u8 last_rx_signal;
	u16 last_rx_rate; /* last RX rate (in 0.1 Mbps) */
	u8 last_rx_updated; /* IWSPY's struct iw_quality::updated */

	u32 tx_supp_rates; /* bit field of supported TX rates */
	u16 tx_rate; /* current TX rate (in 0.1 Mbps) */
	u8 tx_rate_idx; /* current TX rate (WLAN_RATE_*) */
	u8 tx_max_rate; /* max TX rate (WLAN_RATE_*) */
	u32 tx_count[WLAN_RATE_COUNT]; /* number of frames sent (per rate) */
	u32 rx_count[WLAN_RATE_COUNT]; /* number of frames received (per rate) */
	u32 tx_since_last_failure;
	u32 tx_consecutive_exc;

	int ap; /* whether this station is an AP */

	local_info_t *local;

	keyMgmtInfo_t key_mgmt_info;

	/* rate adaptation related data fields
	 */
	u8 tx_success;
	u8 tx_failures;
	u8 tx_retry;
	u8 osc_flag;
	u32 disable_osc_flag_counter;
	u8 rssi_store;
	u8 pkt_tx_since_fail;
	u8 tx_failures_stats;
	u8 rate_increase_increment;
	u8 current_rssi;
	u8 b_only_client;

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	union {
		struct {
			/* shared key authentication challenge */
                        char *challenge; 
		} sta;
		struct {
			int ssid_len;
			/* AP's ssid */
			unsigned char ssid[MVWLAN_MAX_SSID_LEN + 1];
			int channel;
			unsigned long last_beacon; /* last RX beacon time */
		} ap;
	} u;

	struct timer_list timer;
	enum { STA_NULLFUNC = 0, STA_DISASSOC, STA_DEAUTH } timeout_next;
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
};


#define MAX_STA_COUNT       1024

/* Maximum number of AIDs to use for STAs; must be 2007 or lower
 * (8802.11 limitation) */
#define MAX_AID_TABLE_SIZE  128

#define STA_HASH_SIZE       256
#define STA_HASH(sta)       (sta[5])


/* Default value for maximum station inactivity. After AP_MAX_INACTIVITY has
 * passed since last received frame from the station, a nullfunc data frame is
 * sent to the station. If this frame is not acknowledged and no other frames
 * have been received, the station will be disassociated after
 * AP_DISASSOC_DELAY. Similarily, a the station will be deauthenticated after
 * AP_DEAUTH_DELAY. AP_TIMEOUT_RESOLUTION is the resolution that is used with
 * max inactivity timer. */
#define AP_MAX_INACTIVITY   (5 * 60 * HZ)
#define AP_DISASSOC_DELAY   (HZ)
#define AP_DEAUTH_DELAY     (HZ)

/* ap_policy: whether to accept frames to/from other APs/IBSS */
typedef enum {
	AP_OTHER_AP_SKIP_ALL = 0,
	AP_OTHER_AP_SAME_SSID = 1,
	AP_OTHER_AP_ALL = 2,
	AP_OTHER_AP_EVEN_IBSS = 3
} ap_policy_enum;

#define MVWLAN_AUTH_OPEN        BIT(0)
#define MVWLAN_AUTH_SHARED_KEY  BIT(1)


#define MVWLAN_MAX_TIM_SIZE     (MAX_STA_COUNT / 8 + 1)

/* MAC address-based restrictions */
struct mac_entry {
	struct list_head list;
	u8 addr[ETH_ALEN];
};

struct mac_restrictions {
	enum { MAC_POLICY_OPEN = 0, MAC_POLICY_ALLOW, MAC_POLICY_DENY } policy;
	unsigned int entries;
	struct list_head mac_list;
	spinlock_t lock;
};


struct add_sta_proc_data {
	u8 addr[ETH_ALEN];
	struct add_sta_proc_data *next;
};


struct add_wds_data {
	u8 addr[ETH_ALEN];
	struct add_wds_data *next;
};


struct ap_data {
	int initialized; /* whether ap_data has been initialized */
	local_info_t *local;
	int bridge_packets; /* send packet to associated STAs directly to the
			     * wireless media instead of higher layers in the
			     * kernel */
	unsigned int bridged_unicast; /* number of unicast frames bridged on
				       * wireless media */
	unsigned int bridged_multicast; /* number of non-unicast frames
					 * bridged on wireless media */
	int nullfunc_ack; /* use workaround for nullfunc frame ACKs */

	spinlock_t sta_table_lock;
	int num_sta; /* number of entries in sta_list */
	int ps_sta; /* number of entries in PS mode in sta_list */
	int b_only_sta; /* number of entries in 802.11b mode in sta_list */
	struct list_head sta_list; /* STA info list head */
	struct sta_info *sta_hash[STA_HASH_SIZE];

	int b_sta_around_cnt; /* 802.11b STAs around our AP */
	int b_ap_cnt; /* 802.11b APs around our AP */
	int ap_erp_cnt;
	int barker_preamble_set; /* if there is any associated STA not supporting short preamble */
	int PreviousBAroundStnCount;

	struct proc_dir_entry *proc;

	ap_policy_enum ap_policy;
	unsigned int max_inactivity;
	int autom_ap_wds;

	struct mac_restrictions mac_restrictions; /* MAC-based auth */
	int last_tx_rate;

	u8 tim[MVWLAN_MAX_TIM_SIZE];
	MVWLAN_QUEUE set_tim_queue;
	struct list_head set_tim_list;
	spinlock_t set_tim_lock;

	MVWLAN_QUEUE add_sta_proc_queue;
	struct add_sta_proc_data *add_sta_proc_entries;

	MVWLAN_QUEUE add_wds_queue;
	struct add_wds_data *add_wds_entries;

	u16 tx_callback_idx;

	u32 macCurSlotTimeMode;
	u32 macChangeSlotTimeModeTo;

	u32 CBP_QID;
	u32 BRATE_QID;

	int gtk_installed;

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	/* pointers to STA info; based on allocated AID or NULL if AID free
	 * AID is in the range 1-2007, so sta_aid[0] corresponders to AID 1
	 * and so on
	 */
	struct sta_info *sta_aid[MAX_AID_TABLE_SIZE];

	u16 tx_callback_auth, tx_callback_assoc, tx_callback_poll;

	/* WEP operations for generating challenges to be used with shared key
	 * authentication */
	struct mvwlan_crypto_ops *crypt;
	void *crypt_priv;
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
    int sta_bridge; /* 0: disable pkt switch between wireless sta, 1: enable pkt swithc between wireless sta */
};


void mvWLAN_init_ap_data(local_info_t *local);
void mvWLAN_free_ap_data(struct ap_data *ap);

void mvWLAN_handle_sta_release(void *ptr);
struct sta_info * mvWLAN_ap_get_sta(struct ap_data *ap, u8 *sta);
struct sta_info * mvWLAN_ap_add_sta(struct ap_data *ap, u8 *addr);
int mvWLAN_is_sta_assoc(struct ap_data *ap, u8 *sta_addr);
void mvWLAN_update_sq(struct sta_info *sta,
		      struct mvwlan_80211_rx_status *rx_stats);
int mvWLAN_update_rx_stats(struct ap_data *ap,
			   struct mvwlan_ieee80211_hdr *hdr,
			   struct mvwlan_80211_rx_status *rx_stats);
void mvWLAN_update_rates(local_info_t *local);
int mvWLAN_ap_tx_rate_ok(int rateidx, struct sta_info *sta,
			 local_info_t *local);
void mvWLAN_set_tim(local_info_t *local, int aid, int set);
void mvWLAN_add_wds_links(local_info_t *local);
void mvWLAN_add_wds_link(local_info_t *local, u8 *addr);
int mvWLAN_wds_del(local_info_t *local, u8 *remote_addr, int rtnl_locked,
		   int do_not_remove);

void mvWLAN_deauth_all_stas(struct net_device *dev, struct ap_data *ap,
			    int resend);
void mvWLAN_ap_handle_pspoll(local_info_t *local,
			     struct mvwlan_ieee80211_hdr *hdr,
			     struct mvwlan_80211_rx_status *rx_stats);
void mvWLAN_ap_handle_probereq(local_info_t *local, struct sk_buff *skb,
			       struct mvwlan_80211_rx_status *rx_stats);
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
void mvWLAN_ap_handle_authen(local_info_t *local, struct sk_buff *skb,
			     struct mvwlan_80211_rx_status *rx_stats);
void mvWLAN_ap_handle_deauth(local_info_t *local, struct sk_buff *skb,
			     struct mvwlan_80211_rx_status *rx_stats);
void mvWLAN_ap_handle_assoc(local_info_t *local, struct sk_buff *skb,
			    struct mvwlan_80211_rx_status *rx_stats,
			    int reassoc);
void mvWLAN_ap_handle_disassoc(local_info_t *local, struct sk_buff *skb,
			       struct mvwlan_80211_rx_status *rx_stats);
void mvWLAN_ap_handle_dropped_data(local_info_t *local,
				   struct mvwlan_ieee80211_hdr *hdr);
void mvWLAN_ap_handle_data_nullfunc(local_info_t *local,
				    struct mvwlan_ieee80211_hdr *hdr);
void mvWLAN_ap_handle_beacon(local_info_t *local, struct sk_buff *skb,
			     struct mvwlan_80211_rx_status *rx_stats);
int mvWLAN_ap_control_add_mac(struct mac_restrictions *mac_restrictions,
			      u8 *mac);
int mvWLAN_ap_control_del_mac(struct mac_restrictions *mac_restrictions,
			      u8 *mac);
void mvWLAN_ap_control_flush_macs(struct mac_restrictions *mac_restrictions);
int mvWLAN_ap_control_kick_mac(struct ap_data *ap, struct net_device *dev,
			       u8 *mac);
#endif /* !MVWLAN_NO_KERNEL_IEEE80211_MGMT */
void mvWLAN_ap_control_kickall(struct ap_data *ap);

u32 mvWLAN_rate2mask(u8 rate);
int mvWLAN_hostapd(struct ap_data *ap, struct mvwlan_hostapd_param *param);
int mvWLAN_sta_existed(struct net_device *dev, u8 *sta_addr, u8 delSta);

#ifdef SELF_CTS
WLAN_TX_FRAME *sendCts(UINT32 duration, int Qnum, local_info_t *local);
#endif

#endif /* MVWLAN_MGMT_H */
