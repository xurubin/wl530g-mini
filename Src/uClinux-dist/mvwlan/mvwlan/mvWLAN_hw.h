#ifndef MVWLAN_HW_H
#define MVWLAN_HW_H

#ifdef __KERNEL__

#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801
#endif


#define MVWLAN_HW_HEADER_LEN            32

struct mvwlan_scan_request {
	u16 channel_list;
	u16 txrate;
} __attribute__ ((packed));

struct mvwlan_hostscan_request {
	u16 channel_list;
	u16 txrate;
	u16 target_ssid_len;
	u8 target_ssid[32];
} __attribute__ ((packed));

struct mvwlan_join_request {
	u8 bssid[6];
	u16 channel;
} __attribute__ ((packed));

#define MVWLAN_SCAN_IN_PROGRESS         0 /* no results available yet */
#define MVWLAN_SCAN_HOST_INITIATED      1
#define MVWLAN_SCAN_INQUIRY_FROM_HOST   2

struct mvwlan_scan_result_hdr {
	u16 reserved;
	u16 scan_reason;
} __attribute__ ((packed));

#define MVWLAN_SCAN_MAX_RESULTS         32

struct mvwlan_scan_result {
	u16 chid;
	u16 anl;
	u16 sl;
	u8 bssid[6];
	u16 beacon_interval;
	u16 capability;
	u16 ssid_len;
	u8 ssid[32];
	u8 sup_rates[10];
	u16 rate;
} __attribute__ ((packed));

struct mvwlan_hostscan_result {
	u16 chid;
	u16 anl;
	u16 sl;
	u8 bssid[6];
	u16 beacon_interval;
	u16 capability;
	u16 ssid_len;
	u8 ssid[32];
	u8 sup_rates[10];
	u16 rate;
	u16 atim;
} __attribute__ ((packed));


#endif


enum {
/* Station configuration commands */
/* Not implemented for the set ccommands since they are handled by
 * restarting BSS */
	APCMD_GET_STATION_ID,
	APCMD_SET_STATION_ID,
	APCMD_GET_MEDIUM_OCCUPANCY_LIMIT,
/*	APCMD_SET_MEDIUM_OCCUPANCY_LIMIT, */
	APCMD_GET_CFP_PERIOD,
	APCMD_SET_CFP_PERIOD,
	APCMD_GET_CFP_MAX_DURATION,
	APCMD_SET_CFP_MAX_DURATION,
	APCMD_GET_POWER_MGMT_MODE,
	APCMD_SET_POWER_MGMT_MODE,
	APCMD_GET_SSID,
	APCMD_SET_SSID,
	APCMD_GET_BSS_TYPE,
	APCMD_SET_BSS_TYPE,
	APCMD_GET_OPER_RATE_SET,
	APCMD_SET_OPER_RATE_SET,
	APCMD_GET_BEACON_PERIOD,
	APCMD_SET_BEACON_PERIOD,
	APCMD_GET_DTIM_PERIOD,
	APCMD_SET_DTIM_PERIOD,
	APCMD_GET_ENCRYPTION,
	APCMD_SET_ENCRYPTION,
	APCMD_GET_WEP_KEY_LEN,
	APCMD_SET_WEP_KEY_LEN,
	APCMD_GET_WEP_KEY,
	APCMD_SET_WEP_KEY,
	APCMD_GET_WEP_DEFAULT_KEY,
	APCMD_SET_WEP_DEFAULT_KEY,
/* MAC operation commands */
/* No corresponding hardware configuration for the following controls
	APCMD_GET_RTS_THRESHOLD,
	APCMD_SET_RTS_THRESHOLD, */
	APCMD_GET_SHORT_RETRY_LIMIT,
	APCMD_SET_SHORT_RETRY_LIMIT,
	APCMD_GET_LONG_RETRY_LIMIT,
	APCMD_SET_LONG_RETRY_LIMIT,
/* No corresponding hardware configuration for the following controls
 	APCMD_GET_FRAG_THRESHOLD,
	APCMD_SET_FRAG_THRESHOLD, */
	APCMD_GET_MAX_TX_MSDU_LIFETIME,
	APCMD_SET_MAX_TX_MSDU_LIFETIME,
	APCMD_GET_MAX_RX_LIFETIME,
	APCMD_SET_MAX_RX_LIFETIME,
/* PHY related commands */
	APCMD_GET_TX_POWER,
	APCMD_SET_TX_POWER,
	APCMD_GET_RF_CHANNEL,
/* Not implemented for the set ccommand since it is handled by
 * restarting BSS */
	APCMD_SET_RF_CHANNEL,
	APCMD_GET_RX_ANTENNA,
	APCMD_SET_RX_ANTENNA,
	APCMD_GET_TX_ANTENNA,
	APCMD_SET_TX_ANTENNA,
/* Misc commands */
	APCMD_GET_AP_MODE,
	APCMD_SET_AP_MODE,
	APCMD_GET_RX_MODE,
	APCMD_SET_RX_MODE,
	APCMD_GET_CAP_INFO,
	APCMD_SET_CAP_INFO,
	APCMD_GET_ERP_INFO,
	APCMD_SET_ERP_INFO,
/*	APCMD_GET_SSID_HIDDEN, */
	APCMD_SET_SSID_HIDDEN,
/*	APCMD_GET_BASIC_RATE, */
	APCMD_SET_BASIC_RATE,
	APCMD_GET_TIM,
	APCMD_SET_TIM,
/* Registers related commands */
	APCMD_GET_MMF_REG,
	APCMD_SET_MMF_REG,
	APCMD_GET_BBP_REG,
	APCMD_SET_BBP_REG,
	APCMD_GET_WLAN_MAC_REG,
	APCMD_SET_WLAN_MAC_REG,
	APCMD_SET_MFG_MODE,
	APCMD_SET_TX_MULTIFRAMES,
	APCMD_SET_TX_DUFY_CYCLE,
	APCMD_GET_GROUP_KEY,
	APCMD_SET_GROUP_KEY,
	APCMD_SET_PRE_SHARE_KEY,
#ifdef AP_WPA2
	APCMD_SET_WPA2_PRE_SHARE_KEY,
#endif
	APCMD_GET_CURRENT_SLOT_TIME
#ifdef PEREGRINE
    ,
    APCMD_SET_POWER_LEVEL
#endif
};

#ifdef __KERNEL__

/* mvWLAN_rx.c */
int mvWLAN_rx_apd(struct net_device *dev, struct sk_buff *skb,
		  struct mvwlan_80211_rx_status *rx_stats);
void mvWLAN_dump_raw(void *data, int len);
void mvWLAN_dump_rx_80211(const char *name, struct sk_buff *skb,
			  struct mvwlan_80211_rx_status *rx_stats);
void mvWLAN_rx_80211_frame(struct net_device *dev, struct sk_buff *skb,
			   struct mvwlan_80211_rx_status *rx_stats);

/* mvWLAN_tx.c */
void mvWLAN_dump_tx_header(const char *name, MRVL_TX_DONE_INFO *txdone,
			   IEEEtypes_Frame_t *frame802dot11);
void mvWLAN_send_mgmt(struct net_device *dev,
		      int type, int subtype, char *body,
		      int body_len, int txevent, u8 *addr,
		      u16 tx_cb_idx);
int mvWLAN_tx(struct sk_buff *skb, struct net_device *dev);
int mvWLAN_tx_80211(struct sk_buff *skb, struct net_device *dev);
u16 mvWLAN_tx_callback_register(local_info_t *local,
				void (*func)(struct sk_buff *, int ok, void *),
				void *data);
int mvWLAN_tx_callback_unregister(local_info_t *local, u16 idx);
struct mvwlan_tx_queue * mvWLAN_tx_queue_alloc(local_info_t *local);
void mvWLAN_tx_queue_free(local_info_t *local, WLAN_TX_FRAME *wlanFrame,
			  int status);

/* mvWLAN_hw.c */
u8 * mvWLAN_GetProbeResponseBuffer(u8 *ssid, int ssid_len, int *frame_len);
int mvWLAN_transmit(struct net_device *dev, WLAN_TX_FRAME *frame);
void mvWLAN_EnableBcnFreeIntr(void);
void mvWLAN_DisableBcnFreeIntr(void);
void mvWLAN_inc_b_sta(local_info_t *local, int inc);
void mvWLAN_dec_b_sta(local_info_t *local, int dec);
void mvWLAN_passive_scan(unsigned long data);
void mvWLAN_tick_timer(unsigned long data);
void mvWLAN_routine_timer(unsigned long data);
int mvWLAN_setup_hw_resources(local_info_t *local);

#endif

#endif /* MVWLAN_HW_H */
