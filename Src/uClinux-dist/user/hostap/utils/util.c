/*
 * Common functions for Host AP utils
 * Copyright (c) 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>

#include "wireless_copy.h"
#include "util.h"

struct hostap_nicid_rec {
	u16 id;
	char *txt;
};

static struct hostap_nicid_rec hostap_nicids[] =
{
	{ 0x8000, "EVB2 (HFA3841EVAL1) with PRISM I (3860B) Radio" },
	{ 0x8001, "HWB3763 Rev B" },
	{ 0x8002, "HWB3163-01,02,03,04 Rev A" },
	{ 0x8003, "HWB3163 Rev B, Samsung PC Card Rev. B" },
	{ 0x8004, "EVB3 (HFA3843EVAL1, Rev B1)" },
	{ 0x8006, "Nortel Sputnik I" },
	{ 0x8007, "HWB1153 PRISM I Ref" },
	{ 0x8008, "HWB3163, Prism II reference with SSF Flash" },
	{ 0x800A, "3842 Evaluation Board" },
	{ 0x800B, "PRISM II (2.5) PCMCIA (AMD parallel flash)" },
	{ 0x800C, "PRISM II (2.5) PCMCIA (SST parallel flash)" },
	{ 0x800D, "PRISM II (2.5) PCMCIA (AT45DB011 compatible large serial "
	  "flash)" },
	{ 0x800E, "PRISM II (2.5) PCMCIA (AT24C08 compatible small serial "
	  "flash)" },
	{ 0x8012, "PRISM II (2.5) Mini-PCI (AMD parallel flash)" },
	{ 0x8013, "PRISM II (2.5) Mini-PCI (SST parallel flash)" },
	{ 0x8014, "PRISM II (2.5) Mini-PCI (AT45DB011 compatible large serial "
	  "flash)" },
	{ 0x8015, "PRISM II (2.5) Mini-PCI (AT24C08 compatible small serial "
	  "flash)" },
	{ 0x8016, "PCI-bridge (AMD parallel flash)" },
	{ 0x8017, "PCI-bridge (SST parallel flash)" },
	{ 0x8018, "PCI-bridge (AT45DB011 compatible large serial flash)" },
	{ 0x8019, "PCI-bridge (AT24C08 compatible small serial flash)" },
	{ 0x801A, "PRISM III PCMCIA (AMD parallel flash)" },
	{ 0x801B, "PRISM III PCMCIA (SST parallel flash)" },
	{ 0x801C, "PRISM III PCMCIA (AT45DB011 compatible large serial flash)"
	},
	{ 0x801D, "PRISM III PCMCIA (AT24C08 compatible small serial flash)" },
	{ 0x8021, "PRISM III Mini-PCI (AMD parallel flash)" },
	{ 0x8022, "PRISM III Mini-PCI (SST parallel flash)" },
	{ 0x8023, "PRISM III Mini-PCI (AT45DB011 compatible large serial "
	  "flash)" },
	{ 0x8024, "PRISM III Mini-PCI (AT24C08 compatible small serial flash)"
	},
};

void hostap_show_nicid(u8 *data, int len)
{
	struct hfa384x_comp_ident *comp;
	int i;
	u16 id;
	char *txt = "unknown";

	if (len != sizeof(*comp)) {
		printf("Invalid NICID length %d\n", len);
		return;
	}

	comp = (struct hfa384x_comp_ident *) data;

	id = le_to_host16(comp->id);
	for (i = 0; i < sizeof(hostap_nicids) / sizeof(hostap_nicids[0]); i++)
	{
		if (hostap_nicids[i].id == id) {
			txt = hostap_nicids[i].txt;
			break;
		}
	}

	printf("NICID: id=0x%04x v%d.%d.%d (%s)", id,
	       le_to_host16(comp->major),
	       le_to_host16(comp->minor),
	       le_to_host16(comp->variant), txt);
	printf("\n");
}


void hostap_show_priid(u8 *data, int len)
{
	struct hfa384x_comp_ident *comp;

	if (len != sizeof(*comp)) {
		printf("Invalid PRIID length %d\n", len);
		return;
	}

	comp = (struct hfa384x_comp_ident *) data;
	printf("PRIID: id=0x%04x v%d.%d.%d\n",
	       le_to_host16(comp->id),
	       le_to_host16(comp->major),
	       le_to_host16(comp->minor),
	       le_to_host16(comp->variant));
	if (le_to_host16(comp->id) != HFA384X_COMP_ID_PRI)
		printf("   Unknown primary firmware component id!\n");
}


void hostap_show_staid(u8 *data, int len)
{
	struct hfa384x_comp_ident *comp;
	u16 id, major, minor, variant;

	if (len != sizeof(*comp)) {
		printf("Invalid STAID length %d\n", len);
		return;
	}

	comp = (struct hfa384x_comp_ident *) data;

	id = le_to_host16(comp->id);
	major = le_to_host16(comp->major);
	minor = le_to_host16(comp->minor);
	variant = le_to_host16(comp->variant);

	printf("STAID: id=0x%04x v%d.%d.%d", id, major, minor, variant);

	switch (id) {
	case HFA384X_COMP_ID_STA:
		printf(" (station firmware)\n");
		break;
	case HFA384X_COMP_ID_FW_AP:
		printf(" (tertiary firmware)\n");
		break;
	default:
		printf(" (unknown component id!)\n");
		break;
	}
}


int hostapd_ioctl(const char *dev, struct prism2_hostapd_param *param,
		  int len, int show_err)
{
	int s;
	struct iwreq iwr;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, dev, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) param;
	iwr.u.data.length = len;

	if (ioctl(s, PRISM2_IOCTL_HOSTAPD, &iwr) < 0) {
		int ret;
		close(s);
		ret = errno;
		if (show_err) 
			perror("ioctl");
		return ret;
	}
	close(s);

	return 0;
}


int hostapd_get_rid(const char *dev, struct prism2_hostapd_param *param,
		    u16 rid, int show_err)
{
	int res;
	memset(param, 0, PRISM2_HOSTAPD_MAX_BUF_SIZE);
	param->cmd = PRISM2_HOSTAPD_GET_RID;
	param->u.rid.rid = rid;
	param->u.rid.len = PRISM2_HOSTAPD_MAX_BUF_SIZE -
		PRISM2_HOSTAPD_RID_HDR_LEN;
	res = hostapd_ioctl(dev, param, PRISM2_HOSTAPD_MAX_BUF_SIZE, show_err);

	if (res >= 0 && param->u.rid.len >
	    PRISM2_HOSTAPD_MAX_BUF_SIZE - PRISM2_HOSTAPD_RID_HDR_LEN)
		return -1;

	return res;
}


int hostapd_set_rid(const char *dev, u16 rid, u8 *data, size_t len,
		    int show_err)
{
	struct prism2_hostapd_param *param;
	int res;
	size_t blen = PRISM2_HOSTAPD_RID_HDR_LEN + len;
	if (blen < sizeof(*param))
		blen = sizeof(*param);

	param = (struct prism2_hostapd_param *) malloc(blen);
	if (param == NULL)
		return -1;

	memset(param, 0, blen);
	param->cmd = PRISM2_HOSTAPD_SET_RID;
	param->u.rid.rid = rid;
	param->u.rid.len = len;
	memcpy(param->u.rid.data, data, len);
	res = hostapd_ioctl(dev, param, blen, show_err);

	free(param);

	return res;
}


int hostap_ioctl_readmif(const char *dev, int cr)
{
	int s;
	struct iwreq iwr;
	u8 val;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		return -1;
	}

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, dev, IFNAMSIZ);
	iwr.u.name[0] = cr * 2;

	if (ioctl(s, PRISM2_IOCTL_READMIF, &iwr) < 0) {
		perror("ioctl[PRISM2_IOCTL_READMIF]");
		close(s);
		return -1;
	}
	close(s);

	val = iwr.u.name[0];
	return (int) val;
}
