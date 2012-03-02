/*
 * Wireless LAN (IEEE 802.11) link-layer frame sniffer
 * Copyright (c) 2001-2002, SSH Communications Security Corp
 * Jouni Malinen <jkm@ssh.com>
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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#include <linux/if_arp.h>

#include "ieee80211.h"

/* management.c */
int show_frame_management(unsigned int subtype, unsigned char *pos, int len);


enum { BEACONS_NO, BEACONS_ONE_LINE, BEACONS_FULL };
int show_beacons = BEACONS_NO;
int show_only_authen = 0;


const char *wlan_ftype_str(int type)
{
	switch (type) {
	case WLAN_FC_TYPE_DATA:
		return "data";
	case WLAN_FC_TYPE_MGMT:
		return "mgmt";
	case WLAN_FC_TYPE_CTRL:
		return "ctrl";
	default:
		return "unknown";
	}
}

const char *wlan_control_str(int type)
{
	switch (type) {
	case WLAN_FC_STYPE_PSPOLL:
		return "power_save_poll";
	case WLAN_FC_STYPE_RTS:
		return "request_to_send";
	case WLAN_FC_STYPE_CFEND:
		return "contention_free_end";
	case WLAN_FC_STYPE_CFENDACK:
		return "cf_end+cf_ack";
	case WLAN_FC_STYPE_CTS:
		return "clear_to_send";
	case WLAN_FC_STYPE_ACK:
		return "acknowledgement";
	default:
		return "unknown";
	}
}

const char *wlan_management_str(int type)
{
	switch (type) {
	case WLAN_FC_STYPE_ASSOC_REQ:
		return "association_request";
	case WLAN_FC_STYPE_ASSOC_RESP:
		return "association_response";
	case WLAN_FC_STYPE_REASSOC_REQ:
		return "reassociation_request";
	case WLAN_FC_STYPE_REASSOC_RESP:
		return "reassociation_response";
	case WLAN_FC_STYPE_PROBE_REQ:
		return "probe_request";
	case WLAN_FC_STYPE_PROBE_RESP:
		return "probe_response";
	case WLAN_FC_STYPE_BEACON:
		return "beacon";
	case WLAN_FC_STYPE_ATIM:
		return "announcement_traffic_indication_message";
	case WLAN_FC_STYPE_DISASSOC:
		return "disassociation";
	case WLAN_FC_STYPE_AUTH:
		return "authentication";
	case WLAN_FC_STYPE_DEAUTH:
		return "deauthentication";
	default:
		return "unknown";
	}
}

const char *wlan_data_str(int type)
{
	switch (type) {
	case WLAN_FC_STYPE_DATA:
		return "data";
	case WLAN_FC_STYPE_DATA_CFACK:
		return "data+cf-ack";
	case WLAN_FC_STYPE_DATA_CFPOLL:
		return "data+cf-poll";
	case WLAN_FC_STYPE_DATA_CFACKPOLL:
		return "data+cf-ack+cf-poll";
	case WLAN_FC_STYPE_NULLFUNC:
		return "null_function";
	case WLAN_FC_STYPE_CFACK:
		return "cf-ack";
	case WLAN_FC_STYPE_CFPOLL:
		return "cf-poll";
	case WLAN_FC_STYPE_CFACKPOLL:
		return "cf-ack+cf-poll";
	default:
		return "unknown";
	}
}

const char *mac2str(unsigned char *addr)
{
	static char buf[20];
	snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
		 addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	return buf;
}


void show_duration(unsigned int duration)
{
	if ((duration & BIT(15)) == 0)
		printf("dur=%u", duration);
	else if (duration == BIT(15))
		printf("fixed_value_during_CFP");
	else if ((duration & (BIT(15) | BIT(14))) == (BIT(15) | BIT(14))) {
		unsigned int aid = duration & ~BIT(15) & ~BIT(14);
		if (aid == 0 || aid > 2007)
			printf("reserved_AID(%u)", aid);
		else
			printf("AID_in_PS-Poll=%u", aid);
	} else
		printf("dur=reserved?");
}

void show_sequence(unsigned int seq)
{
	printf("  frag#=%u  seq#=%u",
	       WLAN_GET_SEQ_FRAG(seq), WLAN_GET_SEQ_SEQ(seq));
}

void show_hdr_management(struct hfa384x_rx_frame *frm)
{
	struct p80211_hdr_data *hdr;

	hdr = (struct p80211_hdr_data *) &frm->frame_control;
	printf("  ");
	show_duration(le_to_host16(hdr->duration_id));
	show_sequence(le_to_host16(hdr->seq_ctrl));
	printf("\n  DA=%s", mac2str(hdr->addr1));
	printf(" SA=%s", mac2str(hdr->addr2));
	printf(" BSSID=%s\n", mac2str(hdr->addr3));
}


struct addr_titles {
	char *a1, *a2, *a3, *a4;
};

static struct addr_titles titles[] = {
	{"DA", "SA", "BSSID", NULL},
	{"DA", "BSSID", "SA", NULL},
	{"BSSID", "SA", "DA", NULL},
	{"RA", "TA", "DA", "SA"}};

void show_hdr_data(struct hfa384x_rx_frame *frm)
{
	struct p80211_hdr_data *hdr;
	unsigned int fc;
	int addr4, tpos;

	hdr = (struct p80211_hdr_data *) &frm->frame_control;
	fc = le_to_host16(hdr->frame_control);
	addr4 = (fc & WLAN_FC_TODS) && (fc & WLAN_FC_FROMDS);

	printf("  ");
	show_duration(le_to_host16(hdr->duration_id));
	show_sequence(le_to_host16(hdr->seq_ctrl));
	if (!(fc & WLAN_FC_TODS) && !(fc & WLAN_FC_FROMDS))
		tpos = 0;
	else if (!(fc & WLAN_FC_TODS) && (fc & WLAN_FC_FROMDS))
		tpos = 1;
	else if ((fc & WLAN_FC_TODS) && !(fc & WLAN_FC_FROMDS))
		tpos = 2;
	else
		tpos = 3;
	printf("\n  %s=%s", titles[tpos].a1, mac2str(hdr->addr1));
	printf("  %s=%s", titles[tpos].a2, mac2str(hdr->addr2));
	printf("  %s=%s", titles[tpos].a3, mac2str(hdr->addr3));
	if (titles[tpos].a4 != NULL)
		printf("  %s=%s", titles[tpos].a4, mac2str(hdr->addr4));
	printf("\n");
}

void show_hdr_rts(struct hfa384x_rx_frame *frm)
{
	struct p80211_hdr_rts *hdr;

	hdr = (struct p80211_hdr_rts *) &frm->frame_control;
	printf("  ");
	show_duration(le_to_host16(hdr->duration_id));
	printf("\n  RA=%s", mac2str(hdr->ra));
	printf(" TA=%s\n", mac2str(hdr->ta));
}

void show_hdr_pspoll(struct hfa384x_rx_frame *frm)
{
	struct p80211_hdr_pspoll *hdr;

	hdr = (struct p80211_hdr_pspoll *) &frm->frame_control;
	printf("  ");
	/* duration_id should always be AID */
	show_duration(le_to_host16(hdr->duration_id));
	printf("\n  BSSID=%s", mac2str(hdr->bssid));
	printf(" TA=%s\n", mac2str(hdr->ta));
}

void show_hdr_cfend(struct hfa384x_rx_frame *frm)
{
	struct p80211_hdr_cfend *hdr;

	hdr = (struct p80211_hdr_cfend *) &frm->frame_control;
	printf("  ");
	/* duration_id should always be AID */
	show_duration(le_to_host16(hdr->duration_id));
	printf("\n  RA=%s", mac2str(hdr->ra));
	printf(" BSSID=%s\n", mac2str(hdr->bssid));
}

void show_hdr_cts(struct hfa384x_rx_frame *frm)
{
	struct p80211_hdr_cts *hdr;

	hdr = (struct p80211_hdr_cts *) &frm->frame_control;
	printf("  ");
	show_duration(le_to_host16(hdr->duration_id));
	printf("\n  RA=%s\n", mac2str(hdr->ra));
}

void handle_wlansniffrm(unsigned char *buf, int len, int prism_header)
{
	struct hfa384x_rx_frame frmbuf, *frm;
	int left, used, hdrlen;
	unsigned char *pos;
	unsigned int fc;

	left = len;
	pos = buf;

	if (prism_header) {
		struct linux_wlan_ng_prism_hdr *hdr;
			if (len < sizeof(*hdr)) {
			printf("Short wlansniffrm (len=%i)!\n", len);
			return;
		}

		memset(&frmbuf, 0, sizeof(frmbuf));
		hdr = (struct linux_wlan_ng_prism_hdr *) buf;
		pos += sizeof(struct linux_wlan_ng_prism_hdr);
		frm = (struct hfa384x_rx_frame *) (pos - 14);
		memcpy(&frmbuf.frame_control, &frm->frame_control, 24);
		frmbuf.time = host_to_le32(hdr->mactime.data);
		frmbuf.signal = hdr->signal.data;
		frmbuf.silence = hdr->noise.data;
		frmbuf.rate = hdr->rate.data * 5;
		frm = &frmbuf;
		left -= sizeof(struct linux_wlan_ng_prism_hdr);
	} else {
		frm = (struct hfa384x_rx_frame *) (buf - 14);
	}

	fc = le_to_host16(frm->frame_control);
	hdrlen = 24;
	switch (WLAN_FC_GET_TYPE(fc)) {
	case WLAN_FC_TYPE_DATA:
		if ((fc & WLAN_FC_FROMDS) && (fc & WLAN_FC_TODS))
			hdrlen = 30; /* Addr4 */
		break;
	case WLAN_FC_TYPE_CTRL:
		switch (WLAN_FC_GET_STYPE(fc)) {
		case WLAN_FC_STYPE_CTS:
		case WLAN_FC_STYPE_ACK:
			hdrlen = 10;
			break;
		default:
			hdrlen = 16;
			break;
			}
		break;
	}

	if (left < hdrlen) {
		printf("Too short packed (left=%d < hdrlen=%d)\n",
		       left, hdrlen);
		return;
	}

	left -= hdrlen;
	pos += hdrlen;

	if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_MGMT &&
	    WLAN_FC_GET_STYPE(fc) == WLAN_FC_STYPE_BEACON) {
		if (show_beacons == BEACONS_NO)
			return; /* skip beacons */
		else if (show_beacons == BEACONS_ONE_LINE) {
			printf("Beacon: SA=%s ", mac2str(frm->addr2));
			printf("BSSID=%s silence=%u signal=%u rate=%u\n", 
			       mac2str(frm->addr3), frm->silence,
			       frm->signal, frm->rate);
			return;
		}
	}

	if (show_only_authen &&
	    (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT ||
	     WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_AUTH))
		return;

	if (prism_header) {
		/* channel,rssi,sq always zero */
		printf("Sniffed frame: status %04X ", frm->status);
		if (frm->status & BIT(0))
			printf("[FCSErr]");
		if (frm->status & BIT(1))
			printf("[UnDecr]");
		if (frm->status & BIT(12))
			printf("[PCF]");
		printf(" mactime=%u\n  silence=%u signal=%u "
		       "rate=%u " /* "(%u.%u Mbps) " */ "rxflow=%u\n",
		       frm->time, frm->silence, frm->signal,
		       frm->rate, /* frm->rate / 10, frm->rate % 10, */
		       frm->rxflow);
	} else {
		printf("Sniffed frame (only 802.11 header)\n");
	}

	printf("  FC=0x%04x  %s", fc,
	       wlan_ftype_str(WLAN_FC_GET_TYPE(fc)));

	if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_MGMT)
		printf("::%s", wlan_management_str(WLAN_FC_GET_STYPE(fc)));
	else if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_CTRL)
		printf("::%s", wlan_control_str(WLAN_FC_GET_STYPE(fc)));
	else if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_DATA)
		printf("::%s", wlan_data_str(WLAN_FC_GET_STYPE(fc)));

	printf("  pver=%i ", fc & WLAN_FC_PVER);
	if (fc & WLAN_FC_TODS)
		printf(" tods");
	if (fc & WLAN_FC_FROMDS)
		printf(" fromds");
	if (fc & WLAN_FC_MOREFRAG)
		printf(" morefrag");
	if (fc & WLAN_FC_RETRY)
		printf(" retry");
	if (fc & WLAN_FC_PWRMGT)
		printf(" pwrmgt");
	if (fc & WLAN_FC_MOREDATA)
		printf(" moredata");
	if (fc & WLAN_FC_ISWEP)
		printf(" iswep");
	if (fc & WLAN_FC_ORDER)
		printf(" order");
	printf("\n");

	switch (WLAN_FC_GET_TYPE(fc)) {
	case WLAN_FC_TYPE_DATA:
		show_hdr_data(frm);
		break;
	case WLAN_FC_TYPE_MGMT:
		show_hdr_management(frm);
		break;
	case WLAN_FC_TYPE_CTRL:
		switch (WLAN_FC_GET_STYPE(fc)) {
		case WLAN_FC_STYPE_PSPOLL:
			show_hdr_pspoll(frm);
			break;
		case WLAN_FC_STYPE_RTS:
			show_hdr_rts(frm);
			break;
		case WLAN_FC_STYPE_CFEND:
		case WLAN_FC_STYPE_CFENDACK:
			show_hdr_cfend(frm);
			break;
		case WLAN_FC_STYPE_CTS:
		case WLAN_FC_STYPE_ACK:
			show_hdr_cts(frm);
			break;
		}
		break;
	}

	used = 0;
	switch (WLAN_FC_GET_TYPE(fc)) {
	case WLAN_FC_TYPE_MGMT:
		used = show_frame_management(WLAN_FC_GET_STYPE(fc), pos, left);
		break;
	}
	pos += used;
	left -= used;

	/* remove FCS (always set to 0xffffffff by driver) */
	if (left >= 4 && pos[left-1] == 0xff && pos[left-2] == 0xff &&
	    pos[left-3] == 0xff && pos[left-4] == 0xff)
		left -= 4;

	if (left > 0) {
		printf("  data:");
		while (left > 0) {
			printf(" %02x", *pos++);
			left--;
		}
	}
	printf("\n\n");
}


void sniffer(const char *iface)
{
	int s, arptype;
	struct ifreq ifr;
	struct sockaddr_ll addr;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		exit(-1);
	}
        memset(&ifr, 0, sizeof(ifr));
        snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", iface);
        if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0) {
		perror("ioctl(SIOCGIFHWADDR)");
		close(s);
		exit(-1);
        }
	arptype = ifr.ifr_hwaddr.sa_family;

        if (ioctl(s, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)");
		close(s);
		exit(-1);
        }
	close(s);

	if (arptype != ARPHRD_IEEE80211 && arptype != ARPHRD_IEEE80211_PRISM) {
		fprintf(stderr, "Unsupported arptype 0x%04x\n", arptype);
		exit(-1);
	}

	s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (s < 0) {
		perror("socket[PF_PACKET,SOCK_RAW]");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;
	printf("Opening raw packet socket for ifindex %d\n", addr.sll_ifindex);

	if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		close(s);
		exit(-1);
	}

	for (;;) {
		unsigned char buf[8192];
		int len;

		len = recvfrom(s, buf, sizeof(buf), 0, NULL, NULL);
		if (len < 0) {
			perror("recvfrom");
			close(s);
			exit(-1);
		}

		handle_wlansniffrm(buf, len,
				   arptype == ARPHRD_IEEE80211_PRISM ? 1 : 0);
	}
}


void usage(void)
{
	printf("wlansniff [-h] [-b#] [auth] <wlan#>\n");
	printf("   -h = help\n");
	printf("   -b0 = do not show beacons\n");
	printf("   -b1 = show only one line of data for each beacon\n");
	printf("   -b2 = show full beacon data\n");
	printf("   -auth = show only authentication frames\n");
}


int main(int argc, char *argv[])
{
	int i;
	const char *iface = NULL;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0) {
			usage();
			exit(0);
		} else if (strcmp(argv[i], "-b0") == 0)
			show_beacons = BEACONS_NO;
		else if (strcmp(argv[i], "-b1") == 0)
			show_beacons = BEACONS_ONE_LINE;
		else if (strcmp(argv[i], "-b2") == 0)
			show_beacons = BEACONS_FULL;
		else if (strcmp(argv[i], "-auth") == 0)
			show_only_authen = 1;
		else if (i != argc - 1) {
			printf("Unknown option '%s'\n", argv[i]);
			usage();
			exit(-1);
		} else {
			iface = argv[i];
			break;
		}
	}

	if (iface == NULL) {
		usage();
		exit(-1);
	}

	sniffer(iface);
	return 0;
}
