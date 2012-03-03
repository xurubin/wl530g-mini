#ifndef MVWLAN_80211_H
#define MVWLAN_80211_H

/* IEEE 802.11 defines */

#define WLAN_FC_PVER                (BIT(1) | BIT(0))
#define WLAN_FC_TODS                BIT(8)
#define WLAN_FC_FROMDS              BIT(9)
#define WLAN_FC_MOREFRAG            BIT(10)
#define WLAN_FC_RETRY               BIT(11)
#define WLAN_FC_PWRMGT              BIT(12)
#define WLAN_FC_MOREDATA            BIT(13)
#define WLAN_FC_ISWEP               BIT(14)
#define WLAN_FC_ORDER               BIT(15)

#define WLAN_FC_GET_TYPE(fc)        (((fc) & (BIT(3) | BIT(2))) >> 2)
#define WLAN_FC_GET_STYPE(fc)       (((fc) & (BIT(7) | BIT(6) | BIT(5) | \
                                              BIT(4))) >> 4)

#define WLAN_GET_SEQ_FRAG(seq)      ((seq) & (BIT(3) | BIT(2) | BIT(1) | \
                                              BIT(0)))
#define WLAN_GET_SEQ_SEQ(seq)       (((seq) & (~(BIT(3) | BIT(2) | BIT(1) | \
                                                 BIT(0)))) >> 4)

#define WLAN_FC_TYPE_MGMT           0
#define WLAN_FC_TYPE_CTRL           1
#define WLAN_FC_TYPE_DATA           2

/* management */
#define WLAN_FC_STYPE_ASSOC_REQ     0
#define WLAN_FC_STYPE_ASSOC_RESP    1
#define WLAN_FC_STYPE_REASSOC_REQ   2
#define WLAN_FC_STYPE_REASSOC_RESP  3
#define WLAN_FC_STYPE_PROBE_REQ     4
#define WLAN_FC_STYPE_PROBE_RESP    5
#define WLAN_FC_STYPE_BEACON        8
#define WLAN_FC_STYPE_ATIM          9
#define WLAN_FC_STYPE_DISASSOC      10
#define WLAN_FC_STYPE_AUTH          11
#define WLAN_FC_STYPE_DEAUTH        12

/* control */
#define WLAN_FC_STYPE_PSPOLL        10
#define WLAN_FC_STYPE_RTS           11
#define WLAN_FC_STYPE_CTS           12
#define WLAN_FC_STYPE_ACK           13
#define WLAN_FC_STYPE_CFEND         14
#define WLAN_FC_STYPE_CFENDACK      15

/* data */
#define WLAN_FC_STYPE_DATA              0
#define WLAN_FC_STYPE_DATA_CFACK        1
#define WLAN_FC_STYPE_DATA_CFPOLL       2
#define WLAN_FC_STYPE_DATA_CFACKPOLL    3
#define WLAN_FC_STYPE_NULLFUNC          4
#define WLAN_FC_STYPE_CFACK             5
#define WLAN_FC_STYPE_CFPOLL            6
#define WLAN_FC_STYPE_CFACKPOLL         7

/* Authentication algorithms */
#define WLAN_AUTH_OPEN              0
#define WLAN_AUTH_SHARED_KEY        1

#define WLAN_AUTH_CHALLENGE_LEN     128

#define WLAN_CAPABILITY_ESS             BIT(0)
#define WLAN_CAPABILITY_IBSS            BIT(1)
#define WLAN_CAPABILITY_CF_POLLABLE     BIT(2)
#define WLAN_CAPABILITY_CF_POLL_REQUEST BIT(3)
#define WLAN_CAPABILITY_PRIVACY         BIT(4)
#define WLAN_CAPABILITY_SHORT_PREAMBLE  BIT(5)
#define WLAN_CAPABILITY_PBCC            BIT(6)
#define WLAN_CAPABILITY_CHANNEL_AGILITY BIT(7)
#define WLAN_CAPABILITY_SHORT_SLOT_TIME BIT(10)
#define WLAN_CAPABILITY_DSSS_OFDM       BIT(13)

/* Status codes */
#define WLAN_STATUS_SUCCESS                     0
#define WLAN_STATUS_UNSPECIFIED_FAILURE         1
#define WLAN_STATUS_CAPS_UNSUPPORTED            10
#define WLAN_STATUS_REASSOC_NO_ASSOC            11
#define WLAN_STATUS_ASSOC_DENIED_UNSPEC         12
#define WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG      13
#define WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION    14
#define WLAN_STATUS_CHALLENGE_FAIL              15
#define WLAN_STATUS_AUTH_TIMEOUT                16
#define WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA 17
#define WLAN_STATUS_ASSOC_DENIED_RATES          18
/* 802.11b */
#define WLAN_STATUS_ASSOC_DENIED_NOSHORT        19
#define WLAN_STATUS_ASSOC_DENIED_NOPBCC         20
#define WLAN_STATUS_ASSOC_DENIED_NOAGILITY      21

/* Reason codes */
#define WLAN_REASON_UNSPECIFIED                     1
#define WLAN_REASON_PREV_AUTH_NOT_VALID             2
#define WLAN_REASON_DEAUTH_LEAVING                  3
#define WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY      4
#define WLAN_REASON_DISASSOC_AP_BUSY                5
#define WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA   6
#define WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA  7
#define WLAN_REASON_DISASSOC_STA_HAS_LEFT           8
#define WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH      9


/* Information Element IDs */
#define WLAN_EID_SSID           0
#define WLAN_EID_SUPP_RATES     1
#define WLAN_EID_FH_PARAMS      2
#define WLAN_EID_DS_PARAMS      3
#define WLAN_EID_CF_PARAMS      4
#define WLAN_EID_TIM            5
#define WLAN_EID_IBSS_PARAMS    6
#define WLAN_EID_REQUEST_INFO   10
#define WLAN_EID_CHALLENGE      16
#define WLAN_EID_EXT_SUPP_RATES 50


struct mvwlan_ieee80211_hdr {
	u16 frame_control;
	u16 duration_id;
	u8 addr1[6];
	u8 addr2[6];
	u8 addr3[6];
	u16 seq_ctrl;
	u8 addr4[6];
} __attribute__ ((packed));


#define IEEE80211_MGMT_HDR_LEN  24
#define IEEE80211_DATA_HDR3_LEN 24
#define IEEE80211_DATA_HDR4_LEN 30


struct mvwlan_80211_rx_status {
	u32 mac_time;
	u8 signal;
	u8 noise;
	u16 rate; /* in 100 kbps */
};


#endif /* MVWLAN_80211_H */
