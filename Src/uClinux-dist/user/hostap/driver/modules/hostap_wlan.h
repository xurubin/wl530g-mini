#ifndef HOSTAP_WLAN_H
#define HOSTAP_WLAN_H

#include "hostap_config.h"
#ifdef __KERNEL__
#include "hostap_compat.h"
#endif
#include "hostap_crypt.h"

#define MAX_PARM_DEVICES 8
#define PARM_MIN_MAX "1-" __MODULE_STRING(MAX_PARM_DEVICES)
#define DEF_INTS -1, -1, -1, -1, -1, -1, -1
#define GET_INT_PARM(var,idx) var[var[idx] < 0 ? 0 : idx]


#define BIT(x) (1 << (x))

/* Specific skb->protocol value that indicates that the packet already contains
 * txdesc header.
 * FIX: This might need own value that would be allocated especially for Prism2
 * txdesc; ETH_P_CONTROL is commented as "Card specific control frames".
 * However, these skb's should have only minimal path in the kernel side since
 * prism2_send_mgmt() sends these with dev_queue_xmit() to prism2_tx(). */
#define ETH_P_HOSTAP ETH_P_CONTROL

#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E /* Port Access Entity (IEEE 802.1X) */
#endif /* ETH_P_PAE */

#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801
#endif
#ifndef ARPHRD_IEEE80211_PRISM
#define ARPHRD_IEEE80211_PRISM 802
#endif

/* ARPHRD_IEEE80211_PRISM uses a bloated version of Prism2 RX frame header
 * (from linux-wlan-ng) */
struct linux_wlan_ng_val {
	u32 did;
	u16 status, len;
	u32 data;
} __attribute__ ((packed));

struct linux_wlan_ng_prism_hdr {
	u32 msgcode, msglen;
	char devname[16];
	struct linux_wlan_ng_val hosttime, mactime, channel, rssi, sq, signal,
		noise, rate, istx, frmlen;
} __attribute__ ((packed));

struct linux_wlan_ng_cap_hdr {
	u32 version;
	u32 length;
	u64 mactime;
	u64 hosttime;
	u32 phytype;
	u32 channel;
	u32 datarate;
	u32 antenna;
	u32 priority;
	u32 ssi_type;
	s32 ssi_signal;
	s32 ssi_noise;
	u32 preamble;
	u32 encoding;
} __attribute__ ((packed));

#define LWNG_CAP_DID_BASE   (4 | (1 << 6)) /* section 4, group 1 */
#define LWNG_CAPHDR_VERSION 0x80211001

struct hostap_ieee80211_hdr {
	u16 frame_control;
	u16 duration_id;
	u8 addr1[6];
	u8 addr2[6];
	u8 addr3[6];
	u16 seq_ctrl;
	u8 addr4[6];
} __attribute__ ((packed));


struct hfa384x_rx_frame {
	/* HFA384X RX frame descriptor */
	u16 status; /* HFA384X_RX_STATUS_ flags */
	u32 time; /* timestamp, 1 microsecond resolution */
	u8 silence; /* 27 .. 154; seems to be 0 */
	u8 signal; /* 27 .. 154 */
	u8 rate; /* 10, 20, 55, or 110 */
	u8 rxflow;
	u32 reserved;

	/* 802.11 */
	u16 frame_control;
	u16 duration_id;
	u8 addr1[6];
	u8 addr2[6];
	u8 addr3[6];
	u16 seq_ctrl;
	u8 addr4[6];
	u16 data_len;

	/* 802.3 */
	u8 dst_addr[6];
	u8 src_addr[6];
	u16 len;

	/* followed by frame data; max 2304 bytes */
} __attribute__ ((packed));


struct hfa384x_tx_frame {
	/* HFA384X TX frame descriptor */
	u16 status; /* HFA384X_TX_STATUS_ flags */
	u16 reserved1;
	u16 reserved2;
	u32 sw_support;
	u8 retry_count; /* not yet implemented */
	u8 tx_rate; /* Host AP only; 0 = firmware, or 10, 20, 55, 110 */
	u16 tx_control; /* HFA384X_TX_CTRL_ flags */

	/* 802.11 */
	u16 frame_control; /* parts not used */
	u16 duration_id;
	u8 addr1[6];
	u8 addr2[6]; /* filled by firmware */
	u8 addr3[6];
	u16 seq_ctrl; /* filled by firmware */
	u8 addr4[6];
	u16 data_len;

	/* 802.3 */
	u8 dst_addr[6];
	u8 src_addr[6];
	u16 len;

	/* followed by frame data; max 2304 bytes */
} __attribute__ ((packed));


struct hfa384x_rid_hdr
{
	u16 len;
	u16 rid;
} __attribute__ ((packed));


struct hfa384x_comp_ident
{
	u16 id;
	u16 variant;
	u16 major;
	u16 minor;
} __attribute__ ((packed));

#define HFA384X_COMP_ID_PRI 0x15
#define HFA384X_COMP_ID_STA 0x1f
#define HFA384X_COMP_ID_FW_AP 0x14b

struct hfa384x_sup_range
{
	u16 role;
	u16 id;
	u16 variant;
	u16 bottom;
	u16 top;
} __attribute__ ((packed));

struct hfa384x_build_id
{
	u16 pri_seq;
	u16 sec_seq;
} __attribute__ ((packed));

/* FD01 - Download Buffer */
struct hfa384x_rid_download_buffer
{
	u16 page;
	u16 offset;
	u16 length;
} __attribute__ ((packed));

/* BSS connection quality (RID FD43 range, RID FD51 dBm-normalized) */
struct hfa384x_comms_quality {
	u16 comm_qual; /* 0 .. 92 */
	u16 signal_level; /* 27 .. 154 */
	u16 noise_level; /* 27 .. 154 */
} __attribute__ ((packed));

/* Macro for converting signal levels (range 27 .. 154) to wireless ext
 * dBm value with some accuracy */
#define HFA384X_LEVEL_TO_dBm(v) 0x100 + (v) * 100 / 255 - 100

/* Macro for converting signal/silence levels (RSSI) from RX descriptor to
 * dBm */
#define HFA384X_RSSI_LEVEL_TO_dBm(v) ((v) - 100)

struct hfa384x_scan_request {
	u16 channel_list;
	u16 txrate; /* HFA384X_RATES_* */
} __attribute__ ((packed));

struct hfa384x_hostscan_request {
	u16 channel_list;
	u16 txrate;
	u16 target_ssid_len;
	u8 target_ssid[32];
} __attribute__ ((packed));

struct hfa384x_join_request {
	u8 bssid[6];
	u16 channel;
} __attribute__ ((packed));

struct hfa384x_info_frame {
	u16 len;
	u16 type;
} __attribute__ ((packed));

struct hfa384x_comm_tallies {
	u16 tx_unicast_frames;
	u16 tx_multicast_frames;
	u16 tx_fragments;
	u16 tx_unicast_octets;
	u16 tx_multicast_octets;
	u16 tx_deferred_transmissions;
	u16 tx_single_retry_frames;
	u16 tx_multiple_retry_frames;
	u16 tx_retry_limit_exceeded;
	u16 tx_discards;
	u16 rx_unicast_frames;
	u16 rx_multicast_frames;
	u16 rx_fragments;
	u16 rx_unicast_octets;
	u16 rx_multicast_octets;
	u16 rx_fcs_errors;
	u16 rx_discards_no_buffer;
	u16 tx_discards_wrong_sa;
	u16 rx_discards_wep_undecryptable;
	u16 rx_message_in_msg_fragments;
	u16 rx_message_in_bad_msg_fragments;
} __attribute__ ((packed));

struct hfa384x_scan_result_hdr {
	u16 reserved;
	u16 scan_reason;
#define HFA384X_SCAN_IN_PROGRESS 0 /* no results available yet */
#define HFA384X_SCAN_HOST_INITIATED 1
#define HFA384X_SCAN_FIRMWARE_INITIATED 2
#define HFA384X_SCAN_INQUIRY_FROM_HOST 3
} __attribute__ ((packed));

#define HFA384X_SCAN_MAX_RESULTS 32

struct hfa384x_scan_result {
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

struct hfa384x_hostscan_result {
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

struct comm_tallies_sums {
	unsigned int tx_unicast_frames;
	unsigned int tx_multicast_frames;
	unsigned int tx_fragments;
	unsigned int tx_unicast_octets;
	unsigned int tx_multicast_octets;
	unsigned int tx_deferred_transmissions;
	unsigned int tx_single_retry_frames;
	unsigned int tx_multiple_retry_frames;
	unsigned int tx_retry_limit_exceeded;
	unsigned int tx_discards;
	unsigned int rx_unicast_frames;
	unsigned int rx_multicast_frames;
	unsigned int rx_fragments;
	unsigned int rx_unicast_octets;
	unsigned int rx_multicast_octets;
	unsigned int rx_fcs_errors;
	unsigned int rx_discards_no_buffer;
	unsigned int tx_discards_wrong_sa;
	unsigned int rx_discards_wep_undecryptable;
	unsigned int rx_message_in_msg_fragments;
	unsigned int rx_message_in_bad_msg_fragments;
};


struct hfa384x_regs {
	u16 cmd;
	u16 evstat;
	u16 offset0;
	u16 offset1;
	u16 swsupport0;
};


#if defined(PRISM2_PCCARD) || defined(PRISM2_PLX)
/* I/O ports for HFA384X Controller access */
#define HFA384X_CMD_OFF 0x00
#define HFA384X_PARAM0_OFF 0x02
#define HFA384X_PARAM1_OFF 0x04
#define HFA384X_PARAM2_OFF 0x06
#define HFA384X_STATUS_OFF 0x08
#define HFA384X_RESP0_OFF 0x0A
#define HFA384X_RESP1_OFF 0x0C
#define HFA384X_RESP2_OFF 0x0E
#define HFA384X_INFOFID_OFF 0x10
#define HFA384X_CONTROL_OFF 0x14
#define HFA384X_SELECT0_OFF 0x18
#define HFA384X_SELECT1_OFF 0x1A
#define HFA384X_OFFSET0_OFF 0x1C
#define HFA384X_OFFSET1_OFF 0x1E
#define HFA384X_RXFID_OFF 0x20
#define HFA384X_ALLOCFID_OFF 0x22
#define HFA384X_TXCOMPLFID_OFF 0x24
#define HFA384X_SWSUPPORT0_OFF 0x28
#define HFA384X_SWSUPPORT1_OFF 0x2A
#define HFA384X_SWSUPPORT2_OFF 0x2C
#define HFA384X_EVSTAT_OFF 0x30
#define HFA384X_INTEN_OFF 0x32
#define HFA384X_EVACK_OFF 0x34
#define HFA384X_DATA0_OFF 0x36
#define HFA384X_DATA1_OFF 0x38
#define HFA384X_AUXPAGE_OFF 0x3A
#define HFA384X_AUXOFFSET_OFF 0x3C
#define HFA384X_AUXDATA_OFF 0x3E
#endif /* PRISM2_PCCARD || PRISM2_PLX */

#ifdef PRISM2_PCI
/* Memory addresses for ISL3874 controller access */
#define HFA384X_CMD_OFF 0x00
#define HFA384X_PARAM0_OFF 0x04
#define HFA384X_PARAM1_OFF 0x08
#define HFA384X_PARAM2_OFF 0x0C
#define HFA384X_STATUS_OFF 0x10
#define HFA384X_RESP0_OFF 0x14
#define HFA384X_RESP1_OFF 0x18
#define HFA384X_RESP2_OFF 0x1C
#define HFA384X_INFOFID_OFF 0x20
#define HFA384X_CONTROL_OFF 0x28
#define HFA384X_SELECT0_OFF 0x30
#define HFA384X_SELECT1_OFF 0x34
#define HFA384X_OFFSET0_OFF 0x38
#define HFA384X_OFFSET1_OFF 0x3C
#define HFA384X_RXFID_OFF 0x40
#define HFA384X_ALLOCFID_OFF 0x44
#define HFA384X_TXCOMPLFID_OFF 0x48
#define HFA384X_PCICOR_OFF 0x4C
#define HFA384X_SWSUPPORT0_OFF 0x50
#define HFA384X_SWSUPPORT1_OFF 0x54
#define HFA384X_SWSUPPORT2_OFF 0x58
#define HFA384X_PCIHCR_OFF 0x5C
#define HFA384X_EVSTAT_OFF 0x60
#define HFA384X_INTEN_OFF 0x64
#define HFA384X_EVACK_OFF 0x68
#define HFA384X_DATA0_OFF 0x6C
#define HFA384X_DATA1_OFF 0x70
#define HFA384X_AUXPAGE_OFF 0x74
#define HFA384X_AUXOFFSET_OFF 0x78
#define HFA384X_AUXDATA_OFF 0x7C
#define HFA384X_PCI_M0_ADDRH_OFF 0x80
#define HFA384X_PCI_M0_ADDRL_OFF 0x84
#define HFA384X_PCI_M0_LEN_OFF 0x88
#define HFA384X_PCI_M0_CTL_OFF 0x8C
#define HFA384X_PCI_STATUS_OFF 0x98
#define HFA384X_PCI_M1_ADDRH_OFF 0xA0
#define HFA384X_PCI_M1_ADDRL_OFF 0xA4
#define HFA384X_PCI_M1_LEN_OFF 0xA8
#define HFA384X_PCI_M1_CTL_OFF 0xAC

/* PCI bus master control bits (these are undocumented; based on guessing and
 * experimenting..) */
#define HFA384X_PCI_CTL_FROM_BAP (BIT(5) | BIT(1) | BIT(0))
#define HFA384X_PCI_CTL_TO_BAP (BIT(5) | BIT(0))

#endif /* PRISM2_PCI */


/* Command codes for CMD reg. */
#define HFA384X_CMDCODE_INIT 0x00
#define HFA384X_CMDCODE_ENABLE 0x01
#define HFA384X_CMDCODE_DISABLE 0x02
#define HFA384X_CMDCODE_ALLOC 0x0A
#define HFA384X_CMDCODE_TRANSMIT 0x0B
#define HFA384X_CMDCODE_INQUIRE 0x11
#define HFA384X_CMDCODE_ACCESS 0x21
#define HFA384X_CMDCODE_ACCESS_WRITE (0x21 | BIT(8))
#define HFA384X_CMDCODE_DOWNLOAD 0x22
#define HFA384X_CMDCODE_READMIF 0x30
#define HFA384X_CMDCODE_WRITEMIF 0x31
#define HFA384X_CMDCODE_TEST 0x38

#define HFA384X_CMDCODE_MASK 0x3F

/* Test mode operations */
#define HFA384X_TEST_CHANGE_CHANNEL 0x08
#define HFA384X_TEST_MONITOR 0x0B
#define HFA384X_TEST_STOP 0x0F
#define HFA384X_TEST_CFG_BITS 0x15
#define HFA384X_TEST_CFG_BIT_ALC BIT(3)

#define HFA384X_CMD_BUSY BIT(15)

#define HFA384X_CMD_TX_RECLAIM BIT(8)

#define HFA384X_OFFSET_ERR BIT(14)
#define HFA384X_OFFSET_BUSY BIT(15)


/* ProgMode for download command */
#define HFA384X_PROGMODE_DISABLE 0
#define HFA384X_PROGMODE_ENABLE_VOLATILE 1
#define HFA384X_PROGMODE_ENABLE_NON_VOLATILE 2
#define HFA384X_PROGMODE_PROGRAM_NON_VOLATILE 3

#define HFA384X_AUX_MAGIC0 0xfe01
#define HFA384X_AUX_MAGIC1 0xdc23
#define HFA384X_AUX_MAGIC2 0xba45

#define HFA384X_AUX_PORT_DISABLED 0
#define HFA384X_AUX_PORT_DISABLE BIT(14)
#define HFA384X_AUX_PORT_ENABLE BIT(15)
#define HFA384X_AUX_PORT_ENABLED (BIT(14) | BIT(15))
#define HFA384X_AUX_PORT_MASK (BIT(14) | BIT(15))

#define PRISM2_PDA_SIZE 1024


/* Events; EvStat, Interrupt mask (IntEn), and acknowledge bits (EvAck) */
#define HFA384X_EV_TICK BIT(15)
#define HFA384X_EV_WTERR BIT(14)
#define HFA384X_EV_INFDROP BIT(13)
#ifdef PRISM2_PCI
#define HFA384X_EV_PCI_M1 BIT(9)
#define HFA384X_EV_PCI_M0 BIT(8)
#endif /* PRISM2_PCI */
#define HFA384X_EV_INFO BIT(7)
#define HFA384X_EV_DTIM BIT(5)
#define HFA384X_EV_CMD BIT(4)
#define HFA384X_EV_ALLOC BIT(3)
#define HFA384X_EV_TXEXC BIT(2)
#define HFA384X_EV_TX BIT(1)
#define HFA384X_EV_RX BIT(0)


/* HFA384X Configuration RIDs */
#define HFA384X_RID_CNFPORTTYPE 0xFC00
#define HFA384X_RID_CNFOWNMACADDR 0xFC01
#define HFA384X_RID_CNFDESIREDSSID 0xFC02
#define HFA384X_RID_CNFOWNCHANNEL 0xFC03
#define HFA384X_RID_CNFOWNSSID 0xFC04
#define HFA384X_RID_CNFOWNATIMWINDOW 0xFC05
#define HFA384X_RID_CNFSYSTEMSCALE 0xFC06
#define HFA384X_RID_CNFMAXDATALEN 0xFC07
#define HFA384X_RID_CNFWDSADDRESS 0xFC08
#define HFA384X_RID_CNFPMENABLED 0xFC09
#define HFA384X_RID_CNFPMEPS 0xFC0A
#define HFA384X_RID_CNFMULTICASTRECEIVE 0xFC0B
#define HFA384X_RID_CNFMAXSLEEPDURATION 0xFC0C
#define HFA384X_RID_CNFPMHOLDOVERDURATION 0xFC0D
#define HFA384X_RID_CNFOWNNAME 0xFC0E
#define HFA384X_RID_CNFOWNDTIMPERIOD 0xFC10
#define HFA384X_RID_CNFWDSADDRESS1 0xFC11 /* AP f/w only */
#define HFA384X_RID_CNFWDSADDRESS2 0xFC12 /* AP f/w only */
#define HFA384X_RID_CNFWDSADDRESS3 0xFC13 /* AP f/w only */
#define HFA384X_RID_CNFWDSADDRESS4 0xFC14 /* AP f/w only */
#define HFA384X_RID_CNFWDSADDRESS5 0xFC15 /* AP f/w only */
#define HFA384X_RID_CNFWDSADDRESS6 0xFC16 /* AP f/w only */
#define HFA384X_RID_CNFMULTICASTPMBUFFERING 0xFC17 /* AP f/w only */
#define HFA384X_RID_UNKNOWN1 0xFC20
#define HFA384X_RID_UNKNOWN2 0xFC21
#define HFA384X_RID_CNFWEPDEFAULTKEYID 0xFC23
#define HFA384X_RID_CNFDEFAULTKEY0 0xFC24
#define HFA384X_RID_CNFDEFAULTKEY1 0xFC25
#define HFA384X_RID_CNFDEFAULTKEY2 0xFC26
#define HFA384X_RID_CNFDEFAULTKEY3 0xFC27
#define HFA384X_RID_CNFWEPFLAGS 0xFC28
#define HFA384X_RID_CNFWEPKEYMAPPINGTABLE 0xFC29
#define HFA384X_RID_CNFAUTHENTICATION 0xFC2A
#define HFA384X_RID_CNFMAXASSOCSTA 0xFC2B /* AP f/w only */
#define HFA384X_RID_CNFTXCONTROL 0xFC2C
#define HFA384X_RID_CNFROAMINGMODE 0xFC2D
#define HFA384X_RID_CNFHOSTAUTHENTICATION 0xFC2E /* AP f/w only */
#define HFA384X_RID_CNFRCVCRCERROR 0xFC30
#define HFA384X_RID_CNFMMLIFE 0xFC31
#define HFA384X_RID_CNFALTRETRYCOUNT 0xFC32
#define HFA384X_RID_CNFBEACONINT 0xFC33
#define HFA384X_RID_CNFAPPCFINFO 0xFC34 /* AP f/w only */
#define HFA384X_RID_CNFSTAPCFINFO 0xFC35
#define HFA384X_RID_CNFPRIORITYQUSAGE 0xFC37
#define HFA384X_RID_CNFTIMCTRL 0xFC40
#define HFA384X_RID_UNKNOWN3 0xFC41 /* added in STA f/w 0.7.x */
#define HFA384X_RID_CNFTHIRTY2TALLY 0xFC42 /* added in STA f/w 0.8.0 */
#define HFA384X_RID_CNFENHSECURITY 0xFC43 /* AP f/w or STA f/w >= 1.6.3 */
#define HFA384X_RID_CNFDBMADJUST 0xFC46 /* added in STA f/w 1.3.1 */
#define HFA384X_RID_GENERICELEMENT 0xFC48 /* added in STA f/w 1.7.0;
					   * write only */
#define HFA384X_RID_GROUPADDRESSES 0xFC80
#define HFA384X_RID_CREATEIBSS 0xFC81
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD 0xFC82
#define HFA384X_RID_RTSTHRESHOLD 0xFC83
#define HFA384X_RID_TXRATECONTROL 0xFC84
#define HFA384X_RID_PROMISCUOUSMODE 0xFC85
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD0 0xFC90 /* AP f/w only */
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD1 0xFC91 /* AP f/w only */
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD2 0xFC92 /* AP f/w only */
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD3 0xFC93 /* AP f/w only */
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD4 0xFC94 /* AP f/w only */
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD5 0xFC95 /* AP f/w only */
#define HFA384X_RID_FRAGMENTATIONTHRESHOLD6 0xFC96 /* AP f/w only */
#define HFA384X_RID_RTSTHRESHOLD0 0xFC97 /* AP f/w only */
#define HFA384X_RID_RTSTHRESHOLD1 0xFC98 /* AP f/w only */
#define HFA384X_RID_RTSTHRESHOLD2 0xFC99 /* AP f/w only */
#define HFA384X_RID_RTSTHRESHOLD3 0xFC9A /* AP f/w only */
#define HFA384X_RID_RTSTHRESHOLD4 0xFC9B /* AP f/w only */
#define HFA384X_RID_RTSTHRESHOLD5 0xFC9C /* AP f/w only */
#define HFA384X_RID_RTSTHRESHOLD6 0xFC9D /* AP f/w only */
#define HFA384X_RID_TXRATECONTROL0 0xFC9E /* AP f/w only */
#define HFA384X_RID_TXRATECONTROL1 0xFC9F /* AP f/w only */
#define HFA384X_RID_TXRATECONTROL2 0xFCA0 /* AP f/w only */
#define HFA384X_RID_TXRATECONTROL3 0xFCA1 /* AP f/w only */
#define HFA384X_RID_TXRATECONTROL4 0xFCA2 /* AP f/w only */
#define HFA384X_RID_TXRATECONTROL5 0xFCA3 /* AP f/w only */
#define HFA384X_RID_TXRATECONTROL6 0xFCA4 /* AP f/w only */
#define HFA384X_RID_CNFSHORTPREAMBLE 0xFCB0
#define HFA384X_RID_CNFEXCLUDELONGPREAMBLE 0xFCB1
#define HFA384X_RID_CNFAUTHENTICATIONRSPTO 0xFCB2
#define HFA384X_RID_CNFBASICRATES 0xFCB3
#define HFA384X_RID_CNFSUPPORTEDRATES 0xFCB4
#define HFA384X_RID_CNFFALLBACKCTRL 0xFCB5 /* added in STA f/w 1.3.1 */
#define HFA384X_RID_WEPKEYDISABLE 0xFCB6 /* added in STA f/w 1.3.1 */
#define HFA384X_RID_WEPKEYMAPINDEX 0xFCB7 /* ? */
#define HFA384X_RID_BROADCASTKEYID 0xFCB8 /* ? */
#define HFA384X_RID_ENTSECFLAGEYID 0xFCB9 /* ? */
#define HFA384X_RID_CNFPASSIVESCANCTRL 0xFCBA /* added in STA f/w 1.5.0 */
#define HFA384X_RID_SSNHANDLINGMODE 0xFCBB /* added in STA f/w 1.7.0 */
#define HFA384X_RID_MDCCONTROL 0xFCBC /* added in STA f/w 1.7.0 */
#define HFA384X_RID_MDCCOUNTRY 0xFCBD /* added in STA f/w 1.7.0 */
#define HFA384X_RID_TXPOWERMAX 0xFCBE /* added in STA f/w 1.7.0 */
#define HFA384X_RID_CNFLFOENABLED 0xFCBF /* added in STA f/w 1.6.3 */
#define HFA384X_RID_CAPINFO 0xFCC0 /* added in STA f/w 1.7.0 */
#define HFA384X_RID_LISTENINTERVAL 0xFCC1 /* added in STA f/w 1.7.0 */
#define HFA384X_RID_SW_ANT_DIV 0xFCC2 /* added in STA f/w 1.7.0; Prism3 */
#define HFA384X_RID_TICKTIME 0xFCE0
#define HFA384X_RID_SCANREQUEST 0xFCE1
#define HFA384X_RID_JOINREQUEST 0xFCE2
#define HFA384X_RID_AUTHENTICATESTATION 0xFCE3 /* AP f/w only */
#define HFA384X_RID_CHANNELINFOREQUEST 0xFCE4 /* AP f/w only */
#define HFA384X_RID_HOSTSCAN 0xFCE5 /* added in STA f/w 1.3.1 */

/* HFA384X Information RIDs */
#define HFA384X_RID_MAXLOADTIME 0xFD00
#define HFA384X_RID_DOWNLOADBUFFER 0xFD01
#define HFA384X_RID_PRIID 0xFD02
#define HFA384X_RID_PRISUPRANGE 0xFD03
#define HFA384X_RID_CFIACTRANGES 0xFD04
#define HFA384X_RID_NICSERNUM 0xFD0A
#define HFA384X_RID_NICID 0xFD0B
#define HFA384X_RID_MFISUPRANGE 0xFD0C
#define HFA384X_RID_CFISUPRANGE 0xFD0D
#define HFA384X_RID_CHANNELLIST 0xFD10
#define HFA384X_RID_REGULATORYDOMAINS 0xFD11
#define HFA384X_RID_TEMPTYPE 0xFD12
#define HFA384X_RID_CIS 0xFD13
#define HFA384X_RID_STAID 0xFD20
#define HFA384X_RID_STASUPRANGE 0xFD21
#define HFA384X_RID_MFIACTRANGES 0xFD22
#define HFA384X_RID_CFIACTRANGES2 0xFD23
#define HFA384X_RID_PRODUCTNAME 0xFD24 /* added in STA f/w 1.3.1;
					* only Prism2.5(?) */
#define HFA384X_RID_PORTSTATUS 0xFD40
#define HFA384X_RID_CURRENTSSID 0xFD41
#define HFA384X_RID_CURRENTBSSID 0xFD42
#define HFA384X_RID_COMMSQUALITY 0xFD43
#define HFA384X_RID_CURRENTTXRATE 0xFD44
#define HFA384X_RID_CURRENTBEACONINTERVAL 0xFD45
#define HFA384X_RID_CURRENTSCALETHRESHOLDS 0xFD46
#define HFA384X_RID_PROTOCOLRSPTIME 0xFD47
#define HFA384X_RID_SHORTRETRYLIMIT 0xFD48
#define HFA384X_RID_LONGRETRYLIMIT 0xFD49
#define HFA384X_RID_MAXTRANSMITLIFETIME 0xFD4A
#define HFA384X_RID_MAXRECEIVELIFETIME 0xFD4B
#define HFA384X_RID_CFPOLLABLE 0xFD4C
#define HFA384X_RID_AUTHENTICATIONALGORITHMS 0xFD4D
#define HFA384X_RID_PRIVACYOPTIONIMPLEMENTED 0xFD4F
#define HFA384X_RID_DBMCOMMSQUALITY 0xFD51 /* added in STA f/w 1.3.1 */
#define HFA384X_RID_CURRENTTXRATE1 0xFD80 /* AP f/w only */
#define HFA384X_RID_CURRENTTXRATE2 0xFD81 /* AP f/w only */
#define HFA384X_RID_CURRENTTXRATE3 0xFD82 /* AP f/w only */
#define HFA384X_RID_CURRENTTXRATE4 0xFD83 /* AP f/w only */
#define HFA384X_RID_CURRENTTXRATE5 0xFD84 /* AP f/w only */
#define HFA384X_RID_CURRENTTXRATE6 0xFD85 /* AP f/w only */
#define HFA384X_RID_OWNMACADDR 0xFD86 /* AP f/w only */
#define HFA384X_RID_SCANRESULTSTABLE 0xFD88 /* added in STA f/w 0.8.3 */
#define HFA384X_RID_HOSTSCANRESULTS 0xFD89 /* added in STA f/w 1.3.1 */
#define HFA384X_RID_AUTHENTICATIONUSED 0xFD8A /* added in STA f/w 1.3.4 */
#define HFA384X_RID_CNFFAASWITCHCTRL 0xFD8B /* added in STA f/w 1.6.3 */
#define HFA384X_RID_PHYTYPE 0xFDC0
#define HFA384X_RID_CURRENTCHANNEL 0xFDC1
#define HFA384X_RID_CURRENTPOWERSTATE 0xFDC2
#define HFA384X_RID_CCAMODE 0xFDC3
#define HFA384X_RID_SUPPORTEDDATARATES 0xFDC6
#define HFA384X_RID_LFO_VOLT_REG_TEST_RES 0xFDC7 /* added in STA f/w 1.7.1 */
#define HFA384X_RID_BUILDSEQ 0xFFFE
#define HFA384X_RID_FWID 0xFFFF

/* HFA384X Information frames */
#define HFA384X_INFO_HANDOVERADDR 0xF000 /* AP f/w ? */
#define HFA384X_INFO_HANDOVERDEAUTHADDR 0xF001 /* AP f/w 1.3.7 */
#define HFA384X_INFO_COMMTALLIES 0xF100
#define HFA384X_INFO_SCANRESULTS 0xF101
#define HFA384X_INFO_CHANNELINFORESULTS 0xF102 /* AP f/w only */
#define HFA384X_INFO_HOSTSCANRESULTS 0xF103
#define HFA384X_INFO_LINKSTATUS 0xF200
#define HFA384X_INFO_ASSOCSTATUS 0xF201 /* ? */
#define HFA384X_INFO_AUTHREQ 0xF202 /* ? */
#define HFA384X_INFO_PSUSERCNT 0xF203 /* ? */
#define HFA384X_INFO_KEYIDCHANGED 0xF204 /* ? */

enum { HFA384X_LINKSTATUS_CONNECTED = 1,
       HFA384X_LINKSTATUS_DISCONNECTED = 2,
       HFA384X_LINKSTATUS_AP_CHANGE = 3,
       HFA384X_LINKSTATUS_AP_OUT_OF_RANGE = 4,
       HFA384X_LINKSTATUS_AP_IN_RANGE = 5,
       HFA384X_LINKSTATUS_ASSOC_FAILED = 6 };

enum { HFA384X_PORTTYPE_BSS = 1, HFA384X_PORTTYPE_WDS = 2,
       HFA384X_PORTTYPE_PSEUDO_IBSS = 3, HFA384X_PORTTYPE_IBSS = 0,
       HFA384X_PORTTYPE_HOSTAP = 6 };

#define HFA384X_RATES_1MBPS BIT(0)
#define HFA384X_RATES_2MBPS BIT(1)
#define HFA384X_RATES_5MBPS BIT(2)
#define HFA384X_RATES_11MBPS BIT(3)

#define HFA384X_ROAMING_FIRMWARE 1
#define HFA384X_ROAMING_HOST 2
#define HFA384X_ROAMING_DISABLED 3

#define HFA384X_WEPFLAGS_PRIVACYINVOKED BIT(0)
#define HFA384X_WEPFLAGS_EXCLUDEUNENCRYPTED BIT(1)
#define HFA384X_WEPFLAGS_HOSTENCRYPT BIT(4)
#define HFA384X_WEPFLAGS_HOSTDECRYPT BIT(7)

#define HFA384X_RX_STATUS_MSGTYPE (BIT(15) | BIT(14) | BIT(13))
#define HFA384X_RX_STATUS_PCF BIT(12)
#define HFA384X_RX_STATUS_MACPORT (BIT(10) | BIT(9) | BIT(8))
#define HFA384X_RX_STATUS_UNDECR BIT(1)
#define HFA384X_RX_STATUS_FCSERR BIT(0)

#define HFA384X_RX_STATUS_GET_MSGTYPE(s) \
(((s) & HFA384X_RX_STATUS_MSGTYPE) >> 13)
#define HFA384X_RX_STATUS_GET_MACPORT(s) \
(((s) & HFA384X_RX_STATUS_MACPORT) >> 8)

enum { HFA384X_RX_MSGTYPE_NORMAL = 0, HFA384X_RX_MSGTYPE_RFC1042 = 1,
       HFA384X_RX_MSGTYPE_BRIDGETUNNEL = 2, HFA384X_RX_MSGTYPE_MGMT = 4 };


#define HFA384X_TX_CTRL_ALT_RTRY BIT(5)
#define HFA384X_TX_CTRL_802_11 BIT(3)
#define HFA384X_TX_CTRL_802_3 0
#define HFA384X_TX_CTRL_TX_EX BIT(2)
#define HFA384X_TX_CTRL_TX_OK BIT(1)

#define HFA384X_TX_STATUS_RETRYERR BIT(0)
#define HFA384X_TX_STATUS_AGEDERR BIT(1)
#define HFA384X_TX_STATUS_DISCON BIT(2)
#define HFA384X_TX_STATUS_FORMERR BIT(3)

/* HFA3861/3863 (BBP) Control Registers */
#define HFA386X_CR_TX_CONFIGURE 0x12
#define HFA386X_CR_RX_CONFIGURE 0x14
#define HFA386X_CR_A_D_TEST_MODES2 0x1A
#define HFA386X_CR_MANUAL_TX_POWER 0x3E

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


/* Information Element IDs */
#define WLAN_EID_SSID 0
#define WLAN_EID_SUPP_RATES 1
#define WLAN_EID_FH_PARAMS 2
#define WLAN_EID_DS_PARAMS 3
#define WLAN_EID_CF_PARAMS 4
#define WLAN_EID_TIM 5
#define WLAN_EID_IBSS_PARAMS 6
#define WLAN_EID_CHALLENGE 16


#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"


/* netdevice private ioctls (used, e.g., with iwpriv from user space) */

#if WIRELESS_EXT >= 12

/* New wireless extensions API - SET/GET convention (even ioctl numbers are
 * root only)
 */
#define PRISM2_IOCTL_PRISM2_PARAM (SIOCIWFIRSTPRIV + 0)
#define PRISM2_IOCTL_GET_PRISM2_PARAM (SIOCIWFIRSTPRIV + 1)
#define PRISM2_IOCTL_WRITEMIF (SIOCIWFIRSTPRIV + 2)
#define PRISM2_IOCTL_READMIF (SIOCIWFIRSTPRIV + 3)
#define PRISM2_IOCTL_MONITOR (SIOCIWFIRSTPRIV + 4)
#define PRISM2_IOCTL_RESET (SIOCIWFIRSTPRIV + 6)
#define PRISM2_IOCTL_INQUIRE (SIOCIWFIRSTPRIV + 8)
#define PRISM2_IOCTL_WDS_ADD (SIOCIWFIRSTPRIV + 10)
#define PRISM2_IOCTL_WDS_DEL (SIOCIWFIRSTPRIV + 12)
#define PRISM2_IOCTL_SET_RID_WORD (SIOCIWFIRSTPRIV + 14)
#define PRISM2_IOCTL_MACCMD (SIOCIWFIRSTPRIV + 16)
#define PRISM2_IOCTL_ADDMAC (SIOCIWFIRSTPRIV + 18)
#define PRISM2_IOCTL_DELMAC (SIOCIWFIRSTPRIV + 20)
#define PRISM2_IOCTL_KICKMAC (SIOCIWFIRSTPRIV + 22)

/* following are not in SIOCGIWPRIV list; check permission in the driver code
 */
#define PRISM2_IOCTL_DOWNLOAD (SIOCDEVPRIVATE + 13)
#define PRISM2_IOCTL_HOSTAPD (SIOCDEVPRIVATE + 14)

#else /* WIRELESS_EXT >= 12 */

/* Old wireless extensions API - check permission in the driver code */
#define PRISM2_IOCTL_MONITOR (SIOCDEVPRIVATE)
#define PRISM2_IOCTL_PRISM2_PARAM (SIOCDEVPRIVATE + 1)
#define PRISM2_IOCTL_READMIF (SIOCDEVPRIVATE + 2)
#define PRISM2_IOCTL_WRITEMIF (SIOCDEVPRIVATE + 3)
#define PRISM2_IOCTL_RESET (SIOCDEVPRIVATE + 4)
#define PRISM2_IOCTL_INQUIRE (SIOCDEVPRIVATE + 5)
#define PRISM2_IOCTL_WDS_ADD (SIOCDEVPRIVATE + 6)
#define PRISM2_IOCTL_WDS_DEL (SIOCDEVPRIVATE + 7)
#define PRISM2_IOCTL_SET_RID_WORD (SIOCDEVPRIVATE + 8)
#define PRISM2_IOCTL_MACCMD (SIOCDEVPRIVATE + 9)
#define PRISM2_IOCTL_ADDMAC (SIOCDEVPRIVATE + 10)
#define PRISM2_IOCTL_DELMAC (SIOCDEVPRIVATE + 11)
#define PRISM2_IOCTL_KICKMAC (SIOCDEVPRIVATE + 12)
#define PRISM2_IOCTL_DOWNLOAD (SIOCDEVPRIVATE + 13)
#define PRISM2_IOCTL_HOSTAPD (SIOCDEVPRIVATE + 14)

#endif /* WIRELESS_EXT >= 12 */


/* PRISM2_IOCTL_PRISM2_PARAM ioctl() subtypes: */
enum {
	PRISM2_PARAM_PTYPE = 1,
	PRISM2_PARAM_TXRATECTRL = 2,
	PRISM2_PARAM_BEACON_INT = 3,
	PRISM2_PARAM_PSEUDO_IBSS = 4,
	PRISM2_PARAM_ALC = 5,
	PRISM2_PARAM_TXPOWER = 6,
	PRISM2_PARAM_DUMP = 7,
	PRISM2_PARAM_OTHER_AP_POLICY = 8,
	PRISM2_PARAM_AP_MAX_INACTIVITY = 9,
	PRISM2_PARAM_AP_BRIDGE_PACKETS = 10,
	PRISM2_PARAM_DTIM_PERIOD = 11,
	PRISM2_PARAM_AP_NULLFUNC_ACK = 12,
	PRISM2_PARAM_MAX_WDS = 13,
	PRISM2_PARAM_AP_AUTOM_AP_WDS = 14,
	PRISM2_PARAM_AP_AUTH_ALGS = 15,
	PRISM2_PARAM_MONITOR_ALLOW_FCSERR = 16,
	PRISM2_PARAM_HOST_ENCRYPT = 17,
	PRISM2_PARAM_HOST_DECRYPT = 18,
	PRISM2_PARAM_BUS_MASTER_THRESHOLD_RX = 19,
	PRISM2_PARAM_BUS_MASTER_THRESHOLD_TX = 20,
	PRISM2_PARAM_HOST_ROAMING = 21,
	PRISM2_PARAM_BCRX_STA_KEY = 22,
	PRISM2_PARAM_IEEE_802_1X = 23,
	PRISM2_PARAM_ANTSEL_TX = 24,
	PRISM2_PARAM_ANTSEL_RX = 25,
	PRISM2_PARAM_MONITOR_TYPE = 26,
	PRISM2_PARAM_WDS_TYPE = 27,
	PRISM2_PARAM_HOSTSCAN = 28,
	PRISM2_PARAM_AP_SCAN = 29,
	PRISM2_PARAM_ENH_SEC = 30,
	PRISM2_PARAM_IO_DEBUG = 31,
	PRISM2_PARAM_BASIC_RATES = 32,
	PRISM2_PARAM_OPER_RATES = 33,
	PRISM2_PARAM_HOSTAPD = 34,
};

enum { HOSTAP_ANTSEL_DO_NOT_TOUCH = 0, HOSTAP_ANTSEL_DIVERSITY = 1,
       HOSTAP_ANTSEL_LOW = 2, HOSTAP_ANTSEL_HIGH = 3 };


/* PRISM2_IOCTL_MACCMD ioctl() subcommands: */
enum { AP_MAC_CMD_POLICY_OPEN = 0, AP_MAC_CMD_POLICY_ALLOW = 1,
       AP_MAC_CMD_POLICY_DENY = 2, AP_MAC_CMD_FLUSH = 3,
       AP_MAC_CMD_KICKALL = 4 };


/* PRISM2_IOCTL_DOWNLOAD ioctl() dl_cmd: */
enum {
	PRISM2_DOWNLOAD_VOLATILE = 1 /* RAM */,
	/* Note! Old versions of prism2_srec have a fatal error in CRC-16
	 * calculation, which will corrupt all non-volatile downloads.
	 * PRISM2_DOWNLOAD_NON_VOLATILE used to be 2, but it is now 3 to
	 * prevent use of old versions of prism2_srec for non-volatile
	 * download. */
	PRISM2_DOWNLOAD_NON_VOLATILE = 3 /* FLASH */
};

struct prism2_download_param {
	u32 dl_cmd;
	u32 start_addr;
	u32 num_areas;
	struct prism2_download_area {
		u32 addr; /* wlan card address */
		u32 len;
		caddr_t ptr; /* pointer to data in user space */
	} data[0];
};

#define PRISM2_MAX_DOWNLOAD_AREA_LEN 131072
#define PRISM2_MAX_DOWNLOAD_LEN 262144


/* PRISM2_IOCTL_HOSTAPD ioctl() cmd: */
enum {
	PRISM2_HOSTAPD_FLUSH = 1,
	PRISM2_HOSTAPD_ADD_STA = 2,
	PRISM2_HOSTAPD_REMOVE_STA = 3,
	PRISM2_HOSTAPD_GET_INFO_STA = 4,
	PRISM2_HOSTAPD_RESET_TXEXC_STA = 5,
	PRISM2_SET_ENCRYPTION = 6,
	PRISM2_GET_ENCRYPTION = 7,
	PRISM2_HOSTAPD_SET_FLAGS_STA = 8,
	PRISM2_HOSTAPD_GET_RID = 9,
	PRISM2_HOSTAPD_SET_RID = 10,
	PRISM2_HOSTAPD_SET_ASSOC_AP_ADDR = 11,
};

#define PRISM2_HOSTAPD_MAX_BUF_SIZE 1024
#define PRISM2_HOSTAPD_RID_HDR_LEN \
((int) (&((struct prism2_hostapd_param *) 0)->u.rid.data))

struct prism2_hostapd_param {
	u32 cmd;
	u8 sta_addr[ETH_ALEN];
	union {
		struct {
			u16 aid;
			u16 capability;
			u8 tx_supp_rates;
		} add_sta;
		struct {
			u32 inactive_sec;
			u32 txexc;
		} get_info_sta;
		struct {
			u8 alg[HOSTAP_CRYPT_ALG_NAME_LEN];
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
			u16 rid;
			u16 len;
			u8 data[0];
		} rid;
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


#ifdef __KERNEL__

#define PRISM2_TXFID_COUNT 8
#define PRISM2_DATA_MAXLEN 2304
#define PRISM2_TXFID_LEN (PRISM2_DATA_MAXLEN + sizeof(struct hfa384x_tx_frame))
#define PRISM2_TXFID_EMPTY 0xffff
#define PRISM2_TXFID_RESERVED 0xfffe
#define PRISM2_DUMMY_FID 0xffff
#define MAX_SSID_LEN 32
#define MAX_NAME_LEN 32 /* this is assumed to be equal to MAX_SSID_LEN */

#define PRISM2_DUMP_RX_HDR BIT(0)
#define PRISM2_DUMP_TX_HDR BIT(1)
#define PRISM2_DUMP_TXEXC_HDR BIT(2)

typedef struct prism2_wds_info prism2_wds_info_t;

struct prism2_wds_info {
	/* must start with dev, since it is used also as a pointer to whole
	 * prism2_wds_info structure */
	struct net_device dev;
	u8 remote_addr[6];
	struct net_device_stats stats;
	prism2_wds_info_t *next;
};


struct hostap_tx_callback_info {
	u16 idx;
	void (*func)(struct sk_buff *, int ok, void *);
	void *data;
	struct hostap_tx_callback_info *next;
};


/* IEEE 802.11 requires that STA supports concurrent reception of at least
 * three fragmented frames. This define can be increased to support more
 * concurrent frames, but it should be noted that each entry can consume about
 * 2 kB of RAM and increasing cache size will slow down frame reassembly. */
#define PRISM2_FRAG_CACHE_LEN 4

struct prism2_frag_entry {
	unsigned long first_frag_time;
	unsigned int seq;
	unsigned int last_frag;
	struct sk_buff *skb;
	u8 src_addr[ETH_ALEN];
	u8 dst_addr[ETH_ALEN];
};


struct prism2_crypt_data {
	struct list_head list; /* delayed deletion list */
	struct hostap_crypto_ops *ops;
	void *priv;
	atomic_t refcnt;
};

struct hostap_cmd_queue {
	struct list_head list;
	wait_queue_head_t compl;
	volatile enum { CMD_SLEEP, CMD_CALLBACK, CMD_COMPLETED } type;
	void (*callback)(struct net_device *dev, void *context, u16 resp0,
			 u16 res);
	void *context;
	u16 cmd, param0, param1;
	u16 resp0, res;
	volatile int issued, issuing;

	atomic_t usecnt;
	int del_req;
};

/* prism2_rx_80211 'type' argument */
enum {
	PRISM2_RX_MONITOR, PRISM2_RX_MGMT, PRISM2_RX_NON_ASSOC,
	PRISM2_RX_NULLFUNC_ACK
};

/* options for hw_shutdown */
#define HOSTAP_HW_NO_DISABLE BIT(0)
#define HOSTAP_HW_ENABLE_CMDCOMPL BIT(1)

typedef struct local_info local_info_t;

struct prism2_helper_functions {
	/* these functions are defined in hardware model specific files
	 * (hostap_{cs,plx,pci}.c */
	int (*card_present)(local_info_t *local);
	void (*cor_sreset)(local_info_t *local);
	int (*dev_open)(local_info_t *local);
	int (*dev_close)(local_info_t *local);

	/* the following functions are from hostap_hw.c, but they may have some
	 * hardware model specific code */

	/* FIX: low-level commands like cmd might disappear at some point to
	 * make it easier to change them if needed (e.g., cmd would be replaced
	 * with write_mif/read_mif/testcmd/inquire); at least get_rid and
	 * set_rid might move to hostap_{cs,plx,pci}.c */
	int (*cmd)(struct net_device *dev, u16 cmd, u16 param0, u16 *param1,
		   u16 *resp0);
	void (*read_regs)(struct net_device *dev, struct hfa384x_regs *regs);
	int (*get_rid)(struct net_device *dev, u16 rid, void *buf, int len,
		       int exact_len);
	int (*set_rid)(struct net_device *dev, u16 rid, void *buf, int len);
	int (*hw_enable)(struct net_device *dev, int initial);
	int (*hw_config)(struct net_device *dev, int initial);
	void (*hw_reset)(struct net_device *dev);
	void (*hw_shutdown)(struct net_device *dev, int no_disable);
	int (*reset_port)(struct net_device *dev);
	int (*tx)(struct sk_buff *skb, struct net_device *dev);
	void (*schedule_reset)(local_info_t *local);
#ifdef PRISM2_DOWNLOAD_SUPPORT
	int (*download)(local_info_t *local,
			struct prism2_download_param *param);
#endif /* PRISM2_DOWNLOAD_SUPPORT */
	int (*rx_80211)(struct net_device *dev,	struct sk_buff *skb,
			int type, char *extra, int extra_len);
	int (*tx_80211)(struct sk_buff *skb, struct net_device *dev);
};

struct local_info {
	struct module *hw_module;
	int card_idx;
	int dev_enabled;
	struct net_device *dev;
	spinlock_t cmdlock, baplock, wdslock, lock;
	struct semaphore rid_bap_sem;
	u16 infofid; /* MAC buffer id for info frame */
	/* txfid, intransmitfid, next_txtid, and next_alloc are protected by
	 * txfidlock */
	spinlock_t txfidlock;
	int txfid_len; /* length of allocated TX buffers */
	u16 txfid[PRISM2_TXFID_COUNT]; /* buffer IDs for TX frames */
	/* buffer IDs for intransmit frames or PRISM2_TXFID_EMPTY if
	 * corresponding txfid is free for next TX frame */
	u16 intransmitfid[PRISM2_TXFID_COUNT];
	int next_txfid; /* index to the next txfid to be checked for
			 * availability */
	int next_alloc; /* index to the next intransmitfid to be checked for
			 * allocation events */

	/* bitfield for atomic bitops */
#define HOSTAP_BITS_TRANSMIT 0
#define HOSTAP_BITS_BAP_TASKLET 1
#define HOSTAP_BITS_BAP_TASKLET2 2
	long bits;

	struct ap_data *ap;

	char essid[MAX_SSID_LEN + 1];
	char name[MAX_NAME_LEN + 1];
	int name_set;
	u16 channel_mask;
	struct comm_tallies_sums comm_tallies;
	struct net_device_stats stats;
	struct proc_dir_entry *proc;
	int iw_mode; /* operating mode (IW_MODE_*) */
	int pseudo_adhoc; /* 0: IW_MODE_ADHOC is real 802.11 compliant IBSS
			   * 1: IW_MODE_ADHOC is "pseudo IBSS" */
	char bssid[ETH_ALEN];
	int channel;
	int beacon_int;
	int dtim_period;
	int disable_on_close;
	int mtu;
	int frame_dump; /* dump RX/TX frame headers, PRISM2_DUMP_ flags */
	int fw_tx_rate_control;
	u16 tx_rate_control;
	u16 basic_rates;
	int hw_resetting;
	int hw_ready;
	int hw_reset_tries; /* how many times reset has been tried */
	int hw_downloading;
	int shutdown;

	enum {
		PRISM2_TXPOWER_AUTO = 0, PRISM2_TXPOWER_OFF,
		PRISM2_TXPOWER_FIXED, PRISM2_TXPOWER_UNKNOWN
	} txpower_type;
	int txpower; /* if txpower_type == PRISM2_TXPOWER_FIXED */

	/* command queue for hfa384x_cmd(); protected with cmdlock */
	struct list_head cmd_queue;
	/* max_len for cmd_queue; in addition, cmd_callback can use two
	 * additional entries to prevent sleeping commands from stopping
	 * transmits */
#define HOSTAP_CMD_QUEUE_MAX_LEN 16
	int cmd_queue_len; /* number of entries in cmd_queue */

	/* if card timeout is detected in interrupt context, reset_queue is
	 * used to schedule card reseting to be done in user context */
	HOSTAP_QUEUE reset_queue;

	/* For scheduling a change of the promiscuous mode RID */
	int is_promisc;
	HOSTAP_QUEUE set_multicast_list_queue;

	prism2_wds_info_t *wds; /* list of established wds connections */
	int wds_max_connections;
	int wds_connections;
#define HOSTAP_WDS_BROADCAST_RA BIT(0)
#define HOSTAP_WDS_AP_CLIENT BIT(1)
#define HOSTAP_WDS_STANDARD_FRAME BIT(2)
	u32 wds_type;
	u16 tx_control; /* flags to be used in TX description */
	int manual_retry_count; /* -1 = use f/w default; otherwise retry count
				 * to be used with all frames */

#ifdef WIRELESS_EXT
	struct iw_statistics wstats;
#if WIRELESS_EXT > 13
	unsigned long scan_timestamp; /* Time started to scan */
#endif /* WIRELESS_EXT > 13 */
#if WIRELESS_EXT > 15
	struct iw_spy_data spy_data; /* iwspy support */
#endif /* WIRELESS_EXT > 15 */
#endif /* WIRELESS_EXT */
	enum {
		PRISM2_MONITOR_80211 = 0, PRISM2_MONITOR_PRISM = 1,
		PRISM2_MONITOR_CAPHDR = 2
	} monitor_type;
	int (*saved_eth_header_parse)(struct sk_buff *skb,
				      unsigned char *haddr);
	int monitor_allow_fcserr;

	int hostapd; /* whether user space daemon, hostapd, is used for AP
		      * management */
	struct net_device *apdev;
	struct net_device_stats apdevstats;

	char assoc_ap_addr[ETH_ALEN];
	struct net_device *stadev;
	struct net_device_stats stadevstats;

	struct prism2_crypt_data *crypt;
	struct timer_list crypt_deinit_timer;
	struct list_head crypt_deinit_list;

#define WEP_KEYS 4
#define WEP_KEY_LEN 13
	int open_wep; /* allow unencrypted frames */
	int host_encrypt;
	int host_decrypt;
	int fw_encrypt_ok; /* whether firmware-based WEP encrypt is working
			    * in Host AP mode (STA f/w 1.4.9 or newer) */
	int bcrx_sta_key; /* use individual keys to override default keys even
			   * with RX of broad/multicast frames */

	struct prism2_frag_entry frag_cache[PRISM2_FRAG_CACHE_LEN];
	unsigned int frag_next_idx;

	int ieee_802_1x; /* is IEEE 802.1X used */

	int antsel_tx, antsel_rx;

	int enh_sec; /* cnfEnhSecurity options (broadcast SSID hide/ignore) */

	struct prism2_helper_functions *func;

	int bus_master_threshold_tx;
	int bus_master_threshold_rx;
	u8 *bus_m1_buf;

	u8 *pda;
	int fw_ap;
#define PRISM2_FW_VER(major, minor, variant) \
(((major) << 16) | ((minor) << 8) | variant)
	u32 sta_fw_ver;

	/* Tasklets for handling hardware IRQ related operations outside hw IRQ
	 * handler */
	HOSTAP_TASKLET bap_tasklet;

	HOSTAP_TASKLET info_tasklet;
	struct sk_buff_head info_list; /* info frames as skb's for
					* info_tasklet */

	HOSTAP_TASKLET tx_callback_tasklet;
	struct sk_buff_head tx_callback_list; /* queued TX ACK/no-ACK data
					       * frames for tx_callback_tasklet
					       */
	struct hostap_tx_callback_info *tx_callback; /* registered TX callbacks
						      */

	HOSTAP_TASKLET rx_tasklet;
	struct sk_buff_head rx_list;

	HOSTAP_TASKLET sta_tx_exc_tasklet;
	struct sk_buff_head sta_tx_exc_list;

	int host_roaming;
	unsigned long last_join_time; /* time of last JoinRequest */
	struct hfa384x_scan_result *last_scan_results;
	int last_scan_results_count;
	HOSTAP_QUEUE info_queue;
	long pending_info; /* bit field of pending info_queue items */
#define PRISM2_INFO_PENDING_LINKSTATUS 0
#define PRISM2_INFO_PENDING_SCANRESULTS 1
	int prev_link_status; /* previous received LinkStatus info */
	u8 preferred_ap[6]; /* use this AP if possible */

#ifdef PRISM2_CALLBACK
	void *callback_data; /* Can be used in callbacks; e.g., allocate
			      * on enable event and free on disable event.
			      * Host AP driver code does not touch this. */
#endif /* PRISM2_CALLBACK */

	wait_queue_head_t hostscan_wq;

	/* Passive scan in Host AP mode */
	struct timer_list passive_scan_timer;
	int passive_scan_interval; /* in seconds, 0 = disabled */
	int passive_scan_channel;
	enum { PASSIVE_SCAN_WAIT, PASSIVE_SCAN_LISTEN } passive_scan_state;

	struct timer_list tick_timer;
	unsigned long last_tick_timer;
	unsigned int sw_tick_stuck;

#ifdef PRISM2_IO_DEBUG
#define PRISM2_IO_DEBUG_SIZE 10000
	u32 io_debug[PRISM2_IO_DEBUG_SIZE];
	int io_debug_head;
	int io_debug_enabled;
#endif /* PRISM2_IO_DEBUG */

	/* struct local_info is used also in hostap.o that does not define
	 * any PRISM2_{PCCARD,PLX,PCI}. Make sure that the hardware version
	 * specific fields are in the end of the struct (these could also be
	 * moved to void *priv or something like that). */
#ifdef PRISM2_PCCARD
	dev_node_t node;
	dev_link_t *link;
#endif /* PRISM2_PCCARD */

#ifdef PRISM2_PLX
	unsigned long attr_mem;
	unsigned int cor_offset;
#endif /* PRISM2_PLX */

#ifdef PRISM2_PCI
	unsigned long attr_mem;
	unsigned int cor_offset;
#ifdef PRISM2_BUS_MASTER
	/* bus master for BAP0 (TX) */
	int bus_m0_tx_idx;
	u8 *bus_m0_buf;

	/* bus master for BAP1 (RX) */
	struct sk_buff *rx_skb;
#endif /* PRISM2_BUS_MASTER */
#ifdef CONFIG_PM
	u32 pci_save_state[16];
#endif /* CONFIG_PM */
#endif /* PRISM2_PCI */

	/* NOTE! Do not add common entries here after hardware version
	 * specific blocks. */
};

/* if wireless ext is not supported */
#ifndef IW_MODE_ADHOC
#define IW_MODE_ADHOC 1
#endif
#ifndef IW_MODE_INFRA
#define IW_MODE_INFRA 2
#endif
#ifndef IW_MODE_MASTER
#define IW_MODE_MASTER 3
#endif
#ifndef IW_MODE_REPEAT
#define IW_MODE_REPEAT 4
#endif
#ifndef IW_MODE_SECOND
#define IW_MODE_SECOND 5
#endif
#ifndef IW_MODE_MONITOR
#define IW_MODE_MONITOR 6
#endif

#ifndef PRISM2_NO_DEBUG

#define DEBUG_FID BIT(0)
#define DEBUG_PS BIT(1)
#define DEBUG_FLOW BIT(2)
#define DEBUG_AP BIT(3)
#define DEBUG_HW BIT(4)
#define DEBUG_EXTRA BIT(5)
#define DEBUG_EXTRA2 BIT(6)
#define DEBUG_PS2 BIT(7)
#define DEBUG_MASK (DEBUG_PS | DEBUG_FLOW | DEBUG_AP | DEBUG_HW | \
	DEBUG_EXTRA)
#define PDEBUG(n, args...) \
do { if ((n) & DEBUG_MASK) printk(KERN_DEBUG args); } while (0)
#define PDEBUG2(n, args...) \
do { if ((n) & DEBUG_MASK) printk(args); } while (0)

#else /* PRISM2_NO_DEBUG */

#define PDEBUG(n, args...)
#define PDEBUG2(n, args...)

#endif /* PRISM2_NO_DEBUG */

enum { BAP0 = 0, BAP1 = 1 };

#define PRISM2_IO_DEBUG_CMD_INB 0
#define PRISM2_IO_DEBUG_CMD_INW 1
#define PRISM2_IO_DEBUG_CMD_INSW 2
#define PRISM2_IO_DEBUG_CMD_OUTB 3
#define PRISM2_IO_DEBUG_CMD_OUTW 4
#define PRISM2_IO_DEBUG_CMD_OUTSW 5
#define PRISM2_IO_DEBUG_CMD_ERROR 6
#define PRISM2_IO_DEBUG_CMD_INTERRUPT 7

#ifdef PRISM2_IO_DEBUG

#define PRISM2_IO_DEBUG_ENTRY(cmd, reg, value) \
(((cmd) << 24) | ((reg) << 16) | value)

static inline void prism2_io_debug_add(struct net_device *dev, int cmd,
				       int reg, int value)
{
	local_info_t *local = dev->priv;

	if (!local->io_debug_enabled)
		return;

	local->io_debug[local->io_debug_head] =	jiffies & 0xffffffff;
	if (++local->io_debug_head >= PRISM2_IO_DEBUG_SIZE)
		local->io_debug_head = 0;
	local->io_debug[local->io_debug_head] =
		PRISM2_IO_DEBUG_ENTRY(cmd, reg, value);
	if (++local->io_debug_head >= PRISM2_IO_DEBUG_SIZE)
		local->io_debug_head = 0;
}


static inline void prism2_io_debug_error(struct net_device *dev, int err)
{
	local_info_t *local = dev->priv;
	unsigned long flags;

	if (!local->io_debug_enabled)
		return;

	spin_lock_irqsave(&local->lock, flags);
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_ERROR, 0, err);
	if (local->io_debug_enabled == 1) {
		local->io_debug_enabled = 0;
		printk(KERN_DEBUG "%s: I/O debug stopped\n", dev->name);
	}
	spin_unlock_irqrestore(&local->lock, flags);
}

#else /* PRISM2_IO_DEBUG */

static inline void prism2_io_debug_add(struct net_device *dev, int cmd,
				       int reg, int value)
{
}

static inline void prism2_io_debug_error(struct net_device *dev, int err)
{
}

#endif /* PRISM2_IO_DEBUG */


#ifdef PRISM2_CALLBACK
enum {
	/* Called when card is enabled */
	PRISM2_CALLBACK_ENABLE,

	/* Called when card is disabled */
	PRISM2_CALLBACK_DISABLE,

	/* Called when RX/TX starts/ends */
	PRISM2_CALLBACK_RX_START, PRISM2_CALLBACK_RX_END,
	PRISM2_CALLBACK_TX_START, PRISM2_CALLBACK_TX_END
};
void prism2_callback(local_info_t *local, int event);
#else /* PRISM2_CALLBACK */
#define prism2_callback(d, e) do { } while (0)
#endif /* PRISM2_CALLBACK */

#endif /* __KERNEL__ */

#endif /* HOSTAP_WLAN_H */
