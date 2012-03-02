#ifndef UTIL_H
#define UTIL_H

#include "../hostapd/common.h"
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
#include "../driver/modules/hostap_wlan.h"

void hostap_show_nicid(u8 *data, int len);
void hostap_show_priid(u8 *data, int len);
void hostap_show_staid(u8 *data, int len);
int hostapd_ioctl(const char *dev, struct prism2_hostapd_param *param,
		  int len, int show_err);
int hostapd_get_rid(const char *dev, struct prism2_hostapd_param *param,
		    u16 rid, int show_err);
int hostapd_set_rid(const char *dev, u16 rid, u8 *data, size_t len,
		    int show_err);
int hostap_ioctl_readmif(const char *dev, int cr);

#endif /* UTIL_H */
