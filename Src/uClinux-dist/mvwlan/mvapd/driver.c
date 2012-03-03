/*
 * Host AP (software wireless LAN access point) user space daemon for
 * Host AP kernel driver / Kernel driver communication
 * Copyright (c) 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#ifdef USE_KERNEL_HEADERS
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#include <linux/if_arp.h>
#include <linux/wireless.h>
#else /* USE_KERNEL_HEADERS */
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include "wireless_copy.h"
#endif /* USE_KERNEL_HEADERS */

#include "hostapd.h"
#include "driver.h"
#include "ieee802_1x.h"
#include "sta_info.h"


int hostapd_set_iface_flags(hostapd *hapd, int dev_up)
{
	struct ifreq ifr;
	int change = 0;

	if (hapd->ioctl_sock < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", hapd->conf->iface);

	if (ioctl(hapd->ioctl_sock, SIOCGIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		return -1;
	}

	/* Try to open the wireless LAN interface first if it has not been 
	 * done. */
	if (dev_up) {
		if ((ifr.ifr_flags & IFF_UP) != IFF_UP) {
			ifr.ifr_flags |= IFF_UP;
			change = 1;
		}
	} else {
		if ((ifr.ifr_flags & IFF_UP) == IFF_UP) {
			ifr.ifr_flags &= ~IFF_UP;
			change = 1;
		}
	}

	if (change) {
		if (ioctl(hapd->ioctl_sock, SIOCSIFFLAGS, &ifr) != 0) {
			perror("ioctl[SIOCSIFFLAGS]");
			return -1;
		}
	}

	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ, "%sap", hapd->conf->iface);

	if (ioctl(hapd->ioctl_sock, SIOCGIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCGIFFLAGS]");
		return -1;
	}

	if (dev_up)
		ifr.ifr_flags |= IFF_UP;
	else
		ifr.ifr_flags &= ~IFF_UP;

	if (ioctl(hapd->ioctl_sock, SIOCSIFFLAGS, &ifr) != 0) {
		perror("ioctl[SIOCSIFFLAGS]");
		return -1;
	}

	if (dev_up) {
		memset(&ifr, 0, sizeof(ifr));
		snprintf(ifr.ifr_name, IFNAMSIZ, "%sap", hapd->conf->iface);
		ifr.ifr_mtu = HOSTAPD_MTU;
		if (ioctl(hapd->ioctl_sock, SIOCSIFMTU, &ifr) != 0) {
			perror("ioctl[SIOCSIFMTU]");
			printf("Setting MTU failed - trying to survive with "
			       "current value\n");
		}
	}

	return 0;
}


int hostapd_ioctl(hostapd *hapd, struct mvwlan_hostapd_param *param, int len)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, hapd->conf->iface, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) param;
	iwr.u.data.length = len;

	if (ioctl(hapd->ioctl_sock, MVWLAN_IOCTL_HOSTAPD, &iwr) < 0) {
		perror("ioctl[MVWLAN_IOCTL_HOSTAPD]");
		return -1;
	}

	return 0;
}


int hostapd_set_encryption(hostapd *hapd, const char *alg, u8 *addr,
			   int idx, u8 *key, size_t key_len)
{
	struct mvwlan_hostapd_param *param;
	u8 *buf;
	size_t blen;
	int ret = 0;

	blen = sizeof(*param) + key_len;
	buf = malloc(blen);
	if (buf == NULL)
		return -1;
	memset(buf, 0, blen);

	param = (struct mvwlan_hostapd_param *) buf;
	param->cmd = MVWLAN_SET_ENCRYPTION;
	if (addr == NULL)
		memset(param->sta_addr, 0xff, ETH_ALEN);
	else
		memcpy(param->sta_addr, addr, ETH_ALEN);
	strncpy(param->u.crypt.alg, alg, MVWLAN_CRYPT_ALG_NAME_LEN);
	param->u.crypt.flags = HOSTAP_CRYPT_FLAG_SET_TX_KEY;
	param->u.crypt.idx = idx;
	param->u.crypt.key_len = key_len;
	memcpy((u8 *) (param + 1), key, key_len);

	if (hostapd_ioctl(hapd, param, blen)) {
		printf("Failed to set encryption.\n");
		ret = -1;
	}
	free(buf);

	return ret;
}


int hostap_ioctl_mvWLANparam(hostapd *hapd, int param, int value)
{
	struct iwreq iwr;
	int *i;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, hapd->conf->iface, IFNAMSIZ);
	i = (int *) iwr.u.name;
	*i++ = param;
	*i++ = value;

	if (ioctl(hapd->ioctl_sock, MVWLAN_IOCTL_SET_PARAM, &iwr) < 0) {
		perror("ioctl[MVWLAN_IOCTL_SET_PARAM]");
		return -1;
	}

	return 0;
}


int hostap_ioctl_setiwessid(hostapd *hapd, char *buf, int len)
{
	struct iwreq iwr;

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, hapd->conf->iface, IFNAMSIZ);
	iwr.u.essid.flags = 1; /* SSID active */
	iwr.u.essid.pointer = (caddr_t) buf;
	iwr.u.essid.length = len + 1;

	if (ioctl(hapd->ioctl_sock, SIOCSIWESSID, &iwr) < 0) {
		perror("ioctl[SIOCSIWESSID]");
		printf("len=%d\n", len);
		return -1;
	}

	return 0;
}


int hostapd_flush(hostapd *hapd)
{
	struct mvwlan_hostapd_param param;

	memset(&param, 0, sizeof(param));
	param.cmd = MVWLAN_HOSTAPD_FLUSH;
	return hostapd_ioctl(hapd, &param, sizeof(param));
}


int hostapd_read_sta_driver_data(hostapd *hapd,
				 struct hostap_sta_driver_data *data,
				 u8 *addr)
{
	char buf[1024], line[128], *pos;
	FILE *f;
	unsigned long val;

	memset(data, 0, sizeof(*data));
	snprintf(buf, sizeof(buf), "/proc/net/mvwlan/%s/" MACSTR,
		 hapd->conf->iface, MAC2STR(addr));

	f = fopen(buf, "r");
	if (!f)
		return -1;
	/* Need to read proc file with in one piece, so use large enough
	 * buffer. */
	setbuffer(f, buf, sizeof(buf));

	while (fgets(line, sizeof(line), f)) {
		pos = strchr(line, '=');
		if (!pos)
			continue;
		*pos++ = '\0';
		val = atoi(pos);
		if (strcmp(line, "rx_packets") == 0)
			data->rx_packets = val;
		else if (strcmp(line, "tx_packets") == 0)
			data->tx_packets = val;
		else if (strcmp(line, "rx_bytes") == 0)
			data->rx_bytes = val;
		else if (strcmp(line, "tx_bytes") == 0)
			data->tx_bytes = val;
	}

	fclose(f);

	return 0;
}


void remove_sta(hostapd *hapd, struct sta_info *sta)
{
	struct mvwlan_hostapd_param param;

	ieee802_1x_set_port_enabled(hapd, sta, 0);
	ap_sta_no_session_timeout(hapd, sta);

	memset(&param, 0, sizeof(param));
	param.cmd = MVWLAN_HOSTAPD_REMOVE_STA;
	memcpy(param.sta_addr, sta->addr, ETH_ALEN);
	if (hostapd_ioctl(hapd, &param, sizeof(param))) {
		printf("Could not remove station from kernel driver.\n");
	}
}
