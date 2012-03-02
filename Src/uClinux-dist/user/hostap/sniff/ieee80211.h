#ifndef IEEE80211_H
#define IEEE80211_H

#include "../hostapd/common.h"

#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le_to_host16(n) (n)
#define host_to_le32(n) (n)
#else
#include <byteswap.h>
#define le_to_host16(n) bswap_16(n)
#define host_to_le32(n) bswap_32(n)
#endif

#include "../driver/modules/hostap_wlan.h"


struct p80211_hdr_data {
	u16 frame_control __attribute__ ((packed));
	u16 duration_id __attribute__ ((packed));
	u8 addr1[6] __attribute__ ((packed));
	u8 addr2[6] __attribute__ ((packed));
	u8 addr3[6] __attribute__ ((packed));
	u16 seq_ctrl __attribute__ ((packed));
	u8 addr4[6] __attribute__ ((packed));
};

struct p80211_hdr_pspoll {
	u16 frame_control __attribute__ ((packed));
	u16 duration_id __attribute__ ((packed));
	u8 bssid[6] __attribute__ ((packed));
	u8 ta[6] __attribute__ ((packed));
};

struct p80211_hdr_rts {
	u16 frame_control __attribute__ ((packed));
	u16 duration_id __attribute__ ((packed));
	u8 ra[6] __attribute__ ((packed));
	u8 ta[6] __attribute__ ((packed));
};

struct p80211_hdr_cts {
	u16 frame_control __attribute__ ((packed));
	u16 duration_id __attribute__ ((packed));
	u8 ra[6] __attribute__ ((packed));
};

struct p80211_hdr_cfend {
	u16 frame_control __attribute__ ((packed));
	u16 duration_id __attribute__ ((packed));
	u8 ra[6] __attribute__ ((packed));
	u8 bssid[6] __attribute__ ((packed));
};

#endif
