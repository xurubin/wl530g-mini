/*
 * Host AP (software wireless LAN access point) user space daemon for
 * Host AP kernel driver / IEEE 802.11 Management
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
#include <net/if.h>
#include <arpa/inet.h>
#include <time.h>

#include "eloop.h"
#include "hostapd.h"
#include "ieee802_11.h"
#include "radius.h"
#include "radius_client.h"
#include "ieee802_11_auth.h"
#include "sta_info.h"
#include "driver.h"
#include "eapol_sm.h"
#include "rc4.h"
#include "ieee802_1x.h"

#include "mvWLAN_hw.h"

struct index2rate_table {
	u32 index;
	u8  rate;
};


struct index2rate_table Index2RateTable[] = {
	{ 0x00000001,   2 },
	{ 0x00000002,   4 },
	{ 0x00000004,  11 },
	{ 0x00000008,  22 },
	{ 0x00000010,  44 },
	{ 0x00000020,  12 },
	{ 0x00000040,  18 },
	{ 0x00000080,  24 },
	{ 0x00000100,  36 },
	{ 0x00000200,  48 },
	{ 0x00000400,  72 },
	{ 0x00000800,  96 },
	{ 0x00001000, 108 },
	{ 0x00000000,   0 }
};


/* PRIVATE FUNCTION DECLARATION
 */

static u32     IEEE802_11_GetHighestRateIndex(struct ieee802_11_elems *elems_p, Boolean *g_rate_exist);
static Boolean IEEE802_11_IsBasicRateSupported(hostapd *hapd_p, struct ieee802_11_elems *elems_p);
static Boolean IEEE802_11_IsCurShortSlotTime(hostapd *hapd);
static void    IEEE802_11_send_arp(hostapd *hapd, struct sta_info *sta);

#ifdef BOOSTER_MODE
static u32     IEEE802_11_AssocStaNum(hostapd *hapd_p);
static void    IEEE802_11_SendMarvellOUIPck(hostapd *hapd_p, u8 *sta_mac_addr);
static void    IEEE802_11_SendMarvellOUIPck2(hostapd *hapd_p, u8 *sta_mac_addr);
#endif


static u8 index2rate(u32 index)
{
	int i;
	u8 rate = 0;

	for (i = 0; i < HOSTAPD_RATE_LEN; i++) {
		if (! Index2RateTable[i].index)
			break;
		if (Index2RateTable[i].index == index) {
			rate = Index2RateTable[i].rate;
			break;
		}
	}

	return rate;
}


static u32 rate2index(u8 rate)
{
	int i;
	u32 index = 0;

	for (i = 0; i < HOSTAPD_RATE_LEN; i++) {
		if (! Index2RateTable[i].rate)
			break;
		if (Index2RateTable[i].rate == (rate & IEEE802_11_BASIC_RATE_MASK)) {
			index = Index2RateTable[i].index;
			break;
		}
	}

	return index;
}


static void set_sta_rate_index(u8 rate, u32 *tx_supp_rates)
{
	if ((rate & 0x7f) == 2)
		*tx_supp_rates |= WLAN_RATE_1M;
	if ((rate & 0x7f) == 4)
		*tx_supp_rates |= WLAN_RATE_2M;
	if ((rate & 0x7f) == 11)
		*tx_supp_rates |= WLAN_RATE_5M5;
	if ((rate & 0x7f) == 22)
		*tx_supp_rates |= WLAN_RATE_11M;
	if ((rate & 0x7f) == 44)
		*tx_supp_rates |= WLAN_RATE_22M;
	if ((rate & 0x7f) == 12)
		*tx_supp_rates |= WLAN_RATE_6M;
	if ((rate & 0x7f) == 18)
		*tx_supp_rates |= WLAN_RATE_9M;
	if ((rate & 0x7f) == 24)
		*tx_supp_rates |= WLAN_RATE_12M;
	if ((rate & 0x7f) == 36)
		*tx_supp_rates |= WLAN_RATE_18M;
	if ((rate & 0x7f) == 48)
		*tx_supp_rates |= WLAN_RATE_24M;
	if ((rate & 0x7f) == 72)
		*tx_supp_rates |= WLAN_RATE_36M;
	if ((rate & 0x7f) == 96)
		*tx_supp_rates |= WLAN_RATE_48M;
	if ((rate & 0x7f) == 108)
		*tx_supp_rates |= WLAN_RATE_54M;
}


static u8 * hostapd_eid_supp_rates(hostapd *hapd, u8 *eid)
{
	u8 *pos = eid, *eid_len;
	u8 rate;
	int i, count;
	u32 mask;

	*pos++ = WLAN_EID_SUPP_RATES;
	/* bypass length field and will be filled in later */
	eid_len = pos++;

	for (i = 0, count = 0, mask = 1; i < WLAN_RATE_COUNT; i++, mask <<= 1) {

		if (i == 8) {
			*eid_len = count;
			count = 0;
			*pos++ = WLAN_EID_EXT_SUPP_RATES;
			/* bypass length field and will be filled in
			 * later */
			eid_len = pos++;
		}

		if ((hapd->conf->oper_rate & mask) == mask) {

			count++;
			rate = index2rate(mask);
			if ((hapd->conf->basic_rate & mask) == mask) {
				rate |= 0x80;
			}
			*pos++ = rate;
		}
	}

	*eid_len = count;

	return pos;
}


static u16 hostapd_own_capab_info(hostapd *hapd)
{
	int capab = WLAN_CAPABILITY_ESS;

	if (hapd->conf->short_preamble)
		capab |= WLAN_CAPABILITY_SHORT_PREAMBLE;

	if (IEEE802_11_IsCurShortSlotTime(hapd))
		capab |= WLAN_CAPABILITY_SHORT_SLOT_TIME;

#ifndef AP_WPA2
	if (hapd->conf->wpa_mode)
#else
	if ((hapd->conf->wpa_mode) || (hapd->conf->wpa2_mode))
#endif
		capab |= WLAN_CAPABILITY_PRIVACY;

	return capab;
}


ParseRes ieee802_11_parse_elems(hostapd *hapd, u8 *start, size_t len,
				struct ieee802_11_elems *elems)
{
	size_t left = len;
	u8 *pos = start;
	int unknown = 0;

	memset(elems, 0, sizeof(*elems));

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				      "IEEE 802.11 element parse failed "
				      "(id=%d elen=%d left=%d)\n",
				      id, elen, left);
			return ParseFailed;
		}

		switch (id) {
		case WLAN_EID_SSID:
			elems->ssid = pos;
			elems->ssid_len = elen;
			break;
		case WLAN_EID_SUPP_RATES:
			elems->supp_rates = pos;
			elems->supp_rates_len = elen;
			break;
		case WLAN_EID_FH_PARAMS:
			elems->fh_params = pos;
			elems->fh_params_len = elen;
			break;
		case WLAN_EID_DS_PARAMS:
			elems->ds_params = pos;
			elems->ds_params_len = elen;
			break;
		case WLAN_EID_CF_PARAMS:
			elems->cf_params = pos;
			elems->cf_params_len = elen;
			break;
		case WLAN_EID_TIM:
			elems->tim = pos;
			elems->tim_len = elen;
			break;
		case WLAN_EID_IBSS_PARAMS:
			elems->ibss_params = pos;
			elems->ibss_params_len = elen;
			break;
		case WLAN_EID_CHALLENGE:
			elems->challenge = pos;
			elems->challenge_len = elen;
			break;
		case WLAN_EID_ERP_INFO:
			elems->erp_info = pos;
			elems->erp_info_len = elen;
			break;
#ifdef AP_WPA2
		case WLAN_EID_WPA2:
			elems->wpa2_ie = pos;
			elems->wpa2_ie_len = elen;
			break;
#endif
		case WLAN_EID_EXT_SUPP_RATES:
			elems->ext_supp_rates = pos;
			elems->ext_supp_rates_len = elen;
			break;
		case WLAN_EID_GENERIC:
			if (elen > 4 && memcmp(pos, hapd->thisStaRsnIE.OuiType, 4) == 0) {
				elems->wpa_ie = pos;
				elems->wpa_ie_len = elen;
			} else {
				HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE,
					      "IEEE 802.11 element parse "
					      "ignored unknown generic element"
					      " (id=%d elen=%d OUI:type="
					      "%02x-%02x-%02x:%d)\n",
					      id, elen,
					      elen >= 1 ? pos[0] : 0,
					      elen >= 2 ? pos[1] : 0,
					      elen >= 3 ? pos[2] : 0,
					      elen >= 4 ? pos[3] : 0);
				unknown++;
			}
			break;
		default:
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO,
				      "IEEE 802.11 element parse ignored "
				      "unknown element (id=%d elen=%d)\n",
				      id, elen);
			unknown++;
			break;
		}

		left -= elen;
		pos += elen;
	}

	if (left)
		return ParseFailed;

	return unknown ? ParseUnknown : ParseOK;
}


static void ieee802_11_print_ssid(int level, hostapd *hapd, u8 *ssid, u8 len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (ssid[i] >= 32 && ssid[i] < 127)
			HOSTAPD_DEBUG(level, "%c", ssid[i]);
		else
			HOSTAPD_DEBUG(level, "<%02x>", ssid[i]);
	}
}


void ieee802_11_send_deauth(hostapd *hapd, u8 *addr, u8 reason)
{
	struct ieee80211_mgmt mgmt;

	memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_DEAUTH);
	memcpy(mgmt.da, addr, ETH_ALEN);
	memcpy(mgmt.sa, hapd->own_addr, ETH_ALEN);
	memcpy(mgmt.bssid, hapd->own_addr, ETH_ALEN);
	mgmt.u.deauth.reason_code = host_to_le16(reason);
	if (send(hapd->sock, &mgmt, IEEE80211_HDRLEN + sizeof(mgmt.u.deauth),
		 0) < 0)
		perror("ieee802_11_send_deauth: send");
}


void ieee802_11_send_disassoc(hostapd *hapd, u8 *addr, u8 reason)
{
	struct ieee80211_mgmt mgmt;

	memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_DISASSOC);
	memcpy(mgmt.da, addr, ETH_ALEN);
	memcpy(mgmt.sa, hapd->own_addr, ETH_ALEN);
	memcpy(mgmt.bssid, hapd->own_addr, ETH_ALEN);
	mgmt.u.disassoc.reason_code = host_to_le16(reason);
	if (send(hapd->sock, &mgmt, IEEE80211_HDRLEN +
		 sizeof(mgmt.u.disassoc), 0) < 0)
		perror("ieee802_11_send_disassoc: send");
}


static void ieee802_11_sta_authenticate(void *eloop_ctx, void *timeout_ctx)
{
	hostapd *hapd = eloop_ctx;
	struct ieee80211_mgmt mgmt;

	if (hapd->assoc_ap_state == WAIT_BEACON)
		hapd->assoc_ap_state = AUTHENTICATE;
	if (hapd->assoc_ap_state != AUTHENTICATE)
		return;

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Authenticate with AP " MACSTR " SSID=",
	       MAC2STR(hapd->conf->assoc_ap_addr));
	ieee802_11_print_ssid(HOSTAPD_DEBUG_MINIMAL, hapd, hapd->assoc_ap_ssid, hapd->assoc_ap_ssid_len);
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, " (as station)\n");

	memset(&mgmt, 0, sizeof(mgmt));
	mgmt.frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_AUTH);
	/* Request TX callback */
	mgmt.frame_control |= host_to_le16(BIT(1));
	memcpy(mgmt.da, hapd->conf->assoc_ap_addr, ETH_ALEN);
	memcpy(mgmt.sa, hapd->own_addr, ETH_ALEN);
	memcpy(mgmt.bssid, hapd->conf->assoc_ap_addr, ETH_ALEN);
	mgmt.u.auth.auth_alg = host_to_le16(WLAN_AUTH_OPEN);
	mgmt.u.auth.auth_transaction = host_to_le16(1);
	mgmt.u.auth.status_code = host_to_le16(0);
	if (send(hapd->sock, &mgmt, IEEE80211_HDRLEN + sizeof(mgmt.u.auth),
		 0) < 0)
		perror("ieee802_11_sta_authenticate: send");

	/* Try to authenticate again, if this attempt fails or times out. */
	eloop_register_timeout(5, 0, ieee802_11_sta_authenticate, hapd, NULL);
}


static void ieee802_11_sta_associate(void *eloop_ctx, void *timeout_ctx)
{
	hostapd *hapd = eloop_ctx;
	u8 buf[256];
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) buf;
	u8 *p;

	if (hapd->assoc_ap_state == AUTHENTICATE)
		hapd->assoc_ap_state = ASSOCIATE;
	if (hapd->assoc_ap_state != ASSOCIATE)
		return;

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Associate with AP " MACSTR " SSID=",
	       MAC2STR(hapd->conf->assoc_ap_addr));
	ieee802_11_print_ssid(HOSTAPD_DEBUG_MINIMAL, hapd, hapd->assoc_ap_ssid, hapd->assoc_ap_ssid_len);
	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, " (as station)\n");

	memset(mgmt, 0, sizeof(*mgmt));
	mgmt->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					  WLAN_FC_STYPE_ASSOC_REQ);
	/* Request TX callback */
	mgmt->frame_control |= host_to_le16(BIT(1));
	memcpy(mgmt->da, hapd->conf->assoc_ap_addr, ETH_ALEN);
	memcpy(mgmt->sa, hapd->own_addr, ETH_ALEN);
	memcpy(mgmt->bssid, hapd->conf->assoc_ap_addr, ETH_ALEN);
	mgmt->u.assoc_req.capab_info = host_to_le16(0);
	mgmt->u.assoc_req.listen_interval = host_to_le16(1);
	p = &mgmt->u.assoc_req.variable[0];

	*p++ = WLAN_EID_SSID;
	*p++ = hapd->assoc_ap_ssid_len;
	memcpy(p, hapd->assoc_ap_ssid, hapd->assoc_ap_ssid_len);
	p += hapd->assoc_ap_ssid_len;

	p = hostapd_eid_supp_rates(hapd, p);

	if (send(hapd->sock, mgmt, p - (u8 *) mgmt, 0) < 0)
		perror("ieee802_11_sta_associate: send");

	/* Try to authenticate again, if this attempt fails or times out. */
	eloop_register_timeout(5, 0, ieee802_11_sta_associate, hapd, NULL);
}


static struct sta_info * auth_get_sta(hostapd *hapd, u8 *addr)
{
	struct sta_info *sta;

	sta = ap_get_sta(hapd, addr);
	if (sta)
		return sta;

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "  New STA\n");
	if (hapd->num_sta >= MAX_STA_COUNT) {
		/* FIX: might try to remove some old STAs first? */
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "  no more room for new STAs (%d/%d)\n",
		       hapd->num_sta, MAX_STA_COUNT);
		return NULL;
	}

	sta = (struct sta_info *) malloc(sizeof(struct sta_info));
	if (sta == NULL) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "  malloc failed\n");
		return NULL;
	}
	memset(sta, 0, sizeof(struct sta_info));
	sta->radius_identifier = -1;
	sta->acct_session_id_lo = hapd->radius->acct_session_id_lo++;

	/* initialize STA info data */
	eloop_register_timeout(AP_MAX_INACTIVITY, 0, ap_handle_timer,
			       hapd, sta);
	memcpy(sta->addr, addr, ETH_ALEN);
	sta->next = hapd->sta_list;
	hapd->sta_list = sta;
	hapd->num_sta++;
	ap_sta_hash_add(hapd, sta);
	return sta;
}


static u16 auth_shared_key(hostapd *hapd, struct sta_info *sta,
			   u16 auth_transaction, u8 *challenge, int iswep)
{
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_DEBUG,
		       "authentication (shared key, transaction %d)",
		       auth_transaction);

	if (auth_transaction == 1) {
		if (!sta->challenge) {
			/* Generate a pseudo-random challenge */
			u8 key[8];
			time_t now;
			int r;
			sta->challenge = malloc(WLAN_AUTH_CHALLENGE_LEN);
			if (!sta->challenge)
				return WLAN_STATUS_UNSPECIFIED_FAILURE;
			memset(sta->challenge, 0, WLAN_AUTH_CHALLENGE_LEN);

			now = time(NULL);
			r = random();
			memcpy(key, &now, 4);
			memcpy(key + 4, &r, 4);
			rc4(sta->challenge, WLAN_AUTH_CHALLENGE_LEN,
			    key, sizeof(key));
		}
		return 0;
	}

	if (auth_transaction != 3)
		return WLAN_STATUS_UNSPECIFIED_FAILURE;

	/* Transaction 3 */
	if (!iswep || !sta->challenge || !challenge ||
	    memcmp(sta->challenge, challenge, WLAN_AUTH_CHALLENGE_LEN)) {
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_INFO,
			       "shared key authentication - invalid "
			       "challenge-response");
		return WLAN_STATUS_CHALLENGE_FAIL;
	}

	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_DEBUG,
		       "authentication OK (shared key)");
#ifdef IEEE80211_REQUIRE_AUTH_ACK
	/* Station will be marked authenticated if it ACKs the
	 * authentication reply. */
#else
	sta->flags |= WLAN_STA_AUTH;
#endif
	free(sta->challenge);
	sta->challenge = NULL;

	return 0;
}


static void send_auth_reply(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			    u16 auth_alg, u16 auth_transaction, u16 resp,
			    u8 *challenge)
{
	u8 buf[IEEE80211_HDRLEN + sizeof(mgmt->u.auth) + 2 +
	       WLAN_AUTH_CHALLENGE_LEN];
	struct ieee80211_mgmt *reply;
	size_t rlen;

	memset(buf, 0, sizeof(buf));
	reply = (struct ieee80211_mgmt *) buf;
	reply->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
					   WLAN_FC_STYPE_AUTH);
	/* Request TX callback */
	reply->frame_control |= host_to_le16(BIT(1));
	memcpy(reply->da, mgmt->sa, ETH_ALEN);
	memcpy(reply->sa, hapd->own_addr, ETH_ALEN);
	memcpy(reply->bssid, mgmt->bssid, ETH_ALEN);

	reply->u.auth.auth_alg = host_to_le16(auth_alg);
	reply->u.auth.auth_transaction = host_to_le16(auth_transaction);
	reply->u.auth.status_code = host_to_le16(resp);
	rlen = IEEE80211_HDRLEN + sizeof(reply->u.auth);
	if (auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 2 &&
	    challenge) {
		u8 *p = reply->u.auth.variable;
		*p++ = WLAN_EID_CHALLENGE;
		*p++ = WLAN_AUTH_CHALLENGE_LEN;
		memcpy(p, challenge, WLAN_AUTH_CHALLENGE_LEN);
		rlen += 2 + WLAN_AUTH_CHALLENGE_LEN;
	} else
		challenge = NULL;

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "authentication reply: STA=" MACSTR " auth_alg=%d "
		      "auth_transaction=%d resp=%d%s\n",
		      MAC2STR(mgmt->sa), auth_alg, auth_transaction,
		      resp, challenge ? " challenge" : "");
	if (send(hapd->sock, reply, rlen, 0) < 0)
		perror("send_auth_reply: send");
}


static void handle_auth(hostapd *hapd, struct ieee80211_mgmt *mgmt, size_t len)
{
	u16 auth_alg, auth_transaction, status_code;
	u16 resp = WLAN_STATUS_SUCCESS;
	struct sta_info *sta = NULL;
	int res;
	u16 fc;
	u8 *challenge = NULL;
	u32 session_timeout;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.auth)) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "handle_auth - too short payload (len=%d)\n", len);
		return;
	}

	auth_alg = le_to_host16(mgmt->u.auth.auth_alg);
	auth_transaction = le_to_host16(mgmt->u.auth.auth_transaction);
	status_code = le_to_host16(mgmt->u.auth.status_code);
	fc = le_to_host16(mgmt->frame_control);

	if (len >= IEEE80211_HDRLEN + sizeof(mgmt->u.auth) +
	    2 + WLAN_AUTH_CHALLENGE_LEN &&
	    mgmt->u.auth.variable[0] == WLAN_EID_CHALLENGE &&
	    mgmt->u.auth.variable[1] == WLAN_AUTH_CHALLENGE_LEN)
		challenge = &mgmt->u.auth.variable[2];

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "authentication: STA=" MACSTR " auth_alg=%d "
		      "auth_transaction=%d status_code=%d wep=%d%s\n",
		      MAC2STR(mgmt->sa), auth_alg, auth_transaction,
		      status_code, !!(fc & WLAN_FC_ISWEP),
		      challenge ? " challenge" : "");

	if (hapd->assoc_ap_state == AUTHENTICATE && auth_transaction == 2 &&
	    memcmp(mgmt->sa, hapd->conf->assoc_ap_addr, ETH_ALEN) == 0 &&
	    memcmp(mgmt->bssid, hapd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		if (status_code != 0) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Authentication (as station) with AP "
			       MACSTR " failed (status_code=%d)\n",
			       MAC2STR(hapd->conf->assoc_ap_addr),
			       status_code);
			return;
		}
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Authenticated (as station) with AP " MACSTR "\n",
		       MAC2STR(hapd->conf->assoc_ap_addr));
		ieee802_11_sta_associate(hapd, NULL);
		return;
	}

	if (!(((hapd->conf->auth_algs & HOSTAPD_AUTH_OPEN) &&
	       auth_alg == WLAN_AUTH_OPEN) ||
	      ((hapd->conf->auth_algs & HOSTAPD_AUTH_SHARED_KEY) &&
	       auth_alg == WLAN_AUTH_SHARED_KEY))) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Unsupported authentication algorithm (%d)\n",
		       auth_alg);
		resp = WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG;
		goto fail;
	}

	if (!(auth_transaction == 1 ||
	      (auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 3))) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Unknown authentication transaction number (%d)\n",
		       auth_transaction);
		resp = WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION;
		goto fail;
	}

	res = hostapd_allowed_address(hapd, mgmt->sa, (u8 *) mgmt, len,
				      &session_timeout);
	if (res == HOSTAPD_ACL_REJECT) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Station " MACSTR " not allowed to authenticate.\n",
		       MAC2STR(mgmt->sa));
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}
	if (res == HOSTAPD_ACL_PENDING) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Authentication frame "
			      "from " MACSTR " waiting for an external "
			      "authentication\n", MAC2STR(mgmt->sa));
		/* Authentication code will re-send the authentication frame
		 * after it has received (and cached) information from the
		 * external source. */
		return;
	}

	sta = auth_get_sta(hapd, mgmt->sa);
	if (!sta) {
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}

	if (res == HOSTAPD_ACL_ACCEPT_TIMEOUT)
		ap_sta_session_timeout(hapd, sta, session_timeout);
	else
		ap_sta_no_session_timeout(hapd, sta);

	switch (auth_alg) {
	case WLAN_AUTH_OPEN:
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_DEBUG,
			       "authentication OK (open system)");
#ifdef IEEE80211_REQUIRE_AUTH_ACK
		/* Station will be marked authenticated if it ACKs the
		 * authentication reply. */
#else
		sta->flags |= WLAN_STA_AUTH;
#endif
		break;
	case WLAN_AUTH_SHARED_KEY:
		resp = auth_shared_key(hapd, sta, auth_transaction, challenge,
				       fc & WLAN_FC_ISWEP);
		break;
	}

 fail:
	send_auth_reply(hapd, mgmt, auth_alg, auth_transaction + 1, resp,
			sta ? sta->challenge : NULL);
}


static void handle_assoc(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			 size_t len, int reassoc)
{
	u16 capab_info, listen_interval;
	u16 resp = WLAN_STATUS_SUCCESS;
	u8 *pos;
	int send_deauth = 0, send_len, left, i;
	struct sta_info *sta;
	struct ieee802_11_elems elems;
	u32 highest_rate_index;
	Boolean g_rate_exist;

	if (len < IEEE80211_HDRLEN + (reassoc ? sizeof(mgmt->u.reassoc_req) :
				      sizeof(mgmt->u.assoc_req))) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "handle_assoc(reassoc=%d) - too short payload (len=%d)"
		       "\n", reassoc, len);
		return;
	}

	if (reassoc) {
		capab_info = le_to_host16(mgmt->u.reassoc_req.capab_info);
		listen_interval = le_to_host16(
			mgmt->u.reassoc_req.listen_interval);
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "reassociation request: STA=" MACSTR
			      " capab_info=0x%02x "
			      "listen_interval=%d current_ap=" MACSTR "\n",
			      MAC2STR(mgmt->sa), capab_info, listen_interval,
			      MAC2STR(mgmt->u.reassoc_req.current_ap));
		left = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.reassoc_req));
		pos = mgmt->u.reassoc_req.variable;
	} else {
		capab_info = le_to_host16(mgmt->u.assoc_req.capab_info);
		listen_interval = le_to_host16(
			mgmt->u.assoc_req.listen_interval);
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "association request: STA=" MACSTR
			      " capab_info=0x%02x listen_interval=%d\n",
			      MAC2STR(mgmt->sa), capab_info, listen_interval);
		left = len - (IEEE80211_HDRLEN + sizeof(mgmt->u.assoc_req));
		pos = mgmt->u.assoc_req.variable;
	}

	sta = ap_get_sta(hapd, mgmt->sa);
	if (sta == NULL || (sta->flags & WLAN_STA_AUTH) == 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "STA " MACSTR " trying to associate before "
		       "authentication\n", MAC2STR(mgmt->sa));
		if (sta) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "  sta: addr=" MACSTR " aid=%d flags=0x%04x\n",
			       MAC2STR(sta->addr), sta->aid, sta->flags);
		}
		send_deauth = 1;
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}

    /* if unit runs in p2p mode, reject all asso/reass request */
    if (hapd->conf->op_mode == 1)
    {
        HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "UNIT runs in P2P mode, no BSS services proviede\n");
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
    }
            
	sta->capability = capab_info;

	/* followed by SSID and Supported rates */
	if (ieee802_11_parse_elems(hapd, pos, left, &elems) == ParseFailed ||
	    !elems.ssid) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "STA " MACSTR " sent invalid association request\n",
		       MAC2STR(sta->addr));
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}

	if (elems.ssid_len != hapd->conf->ssid_len ||
	    memcmp(elems.ssid, hapd->conf->ssid, elems.ssid_len) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Station " MACSTR " tried to associate with "
		       "unknown SSID '", MAC2STR(sta->addr));
		ieee802_11_print_ssid(HOSTAPD_DEBUG_MINIMAL, hapd, elems.ssid, elems.ssid_len);
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "'\n");
		return;
	}

	/* check basic rate set
	 */
#if 0
	/* take off this checking due to g card will not set basic rate bit in support or extend support
	 * rate set
	 */
	if (IEEE802_11_IsBasicRateSupported(hapd, &elems) == FALSE)
	{
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "STA " MACSTR " deos not have collect basic rate set\n",
		       MAC2STR(sta->addr));
		resp = WLAN_STATUS_ASSOC_DENIED_RATES;
		goto fail;
	}
#endif

	/* check opearation mode
	 */
        highest_rate_index = IEEE802_11_GetHighestRateIndex(&elems, &g_rate_exist);

	if ((hapd->conf->ap_mode == MVWLAN_AP_MODE_G_ONLY) && (g_rate_exist == FALSE))
	{
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "STA " MACSTR " is B client, AP is G only mode\n",
		       MAC2STR(sta->addr));
		resp = WLAN_STATUS_ASSOC_DENIED_RATES;
		goto fail;
	}

	/* save information for this station
	 */
	if (elems.supp_rates) {
		if (elems.supp_rates_len > sizeof(sta->supported_rates)) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "STA " MACSTR ": Invalid supported rates "
			       "element length %d\n", MAC2STR(sta->addr),
			       elems.supp_rates_len);
			return;
		}

		memset(sta->supported_rates, 0, sizeof(sta->supported_rates));
		memcpy(sta->supported_rates, elems.supp_rates,
		       elems.supp_rates_len);

		sta->tx_supp_rates = 0;
		for (i = 0; i < elems.supp_rates_len; i++) {
			set_sta_rate_index(sta->supported_rates[i],
					   &sta->tx_supp_rates);
		}

		if (elems.ext_supp_rates) {
			memcpy(sta->supported_rates+elems.supp_rates_len,
			       elems.ext_supp_rates, elems.ext_supp_rates_len);

			for (i = 0; i < elems.ext_supp_rates_len; i++) {
				set_sta_rate_index(
				  sta->supported_rates[elems.supp_rates_len+i],
				  &sta->tx_supp_rates);
			}
		}
	} else
		sta->tx_supp_rates = 0xff;

#ifndef AP_WPA2
	if (hapd->conf->wpa_mode && elems.wpa_ie == NULL) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			"STA " MACSTR ": No WPA IE in association request\n",
			MAC2STR(sta->addr));
		resp = WLAN_STATUS_INVALID_IE;
		goto fail;
	}

	if (hapd->conf->wpa_mode) {

		if (hapd->MIC_Error.disableStaAsso) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				"STA " MACSTR ": MIC Countermeasures take effect\n",
				MAC2STR(sta->addr));
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			goto fail;
		}

		memset(&sta->key_mgmt_info, 0, sizeof(keyMgmtInfo_t));
		memcpy(sta->key_mgmt_info.RsnIEBuf, elems.wpa_ie - 2, elems.wpa_ie_len + 2);
	}
#else
	{
	u8 *save_ie;
	u8 save_ie_len;

	save_ie = NULL;

	if (hapd->conf->wpa_mode) {

		if (hapd->conf->wpa2_mode) {

			if (elems.wpa_ie == NULL && elems.wpa2_ie == NULL) {
				HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
					"STA " MACSTR ": No WPA or WPA2 IE in association request\n",
					MAC2STR(sta->addr));
				resp = WLAN_STATUS_INVALID_IE;
				goto fail;
			} else {
				if (elems.wpa2_ie != NULL) {
					save_ie = elems.wpa2_ie;
					save_ie_len = elems.wpa2_ie_len;
				} else if (elems.wpa_ie != NULL) {
					save_ie = elems.wpa_ie;
					save_ie_len = elems.wpa_ie_len;
				}
			}
		} else {

			if (elems.wpa_ie == NULL) {
				HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
					"STA " MACSTR ": No WPA IE in association request\n",
					MAC2STR(sta->addr));
				resp = WLAN_STATUS_INVALID_IE;
				goto fail;
			} else {
				save_ie = elems.wpa_ie;
				save_ie_len = elems.wpa_ie_len;
			}
		}
	} else {

		if (hapd->conf->wpa2_mode && elems.wpa2_ie == NULL) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				"STA " MACSTR ": No WPA2 IE in association request\n",
				MAC2STR(sta->addr));
			resp = WLAN_STATUS_INVALID_IE;
			goto fail;
		} else {
			save_ie = elems.wpa2_ie;
			save_ie_len = elems.wpa2_ie_len;
		}
	}

	if (hapd->conf->wpa_mode || hapd->conf->wpa2_mode) {

		if (hapd->MIC_Error.disableStaAsso) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				"STA " MACSTR ": MIC Countermeasures take effect\n",
				MAC2STR(sta->addr));
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			goto fail;
		}

		if (save_ie == NULL) {
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				"STA " MACSTR ": No WPA or WPA2 IE in association request\n",
				MAC2STR(sta->addr));
			resp = WLAN_STATUS_INVALID_IE;
			goto fail;
		} else {
			memset(&sta->key_mgmt_info, 0, sizeof(keyMgmtInfo_t));
			memcpy(sta->key_mgmt_info.RsnIEBuf, save_ie - 2, save_ie_len + 2);
		}
	}

	}
#endif

	/* get a unique AID */
	if (sta->aid > 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "  old AID %d\n", sta->aid);
	} else {
		for (sta->aid = 1; sta->aid <= MAX_AID_TABLE_SIZE; sta->aid++)
			if (hapd->sta_aid[sta->aid - 1] == NULL)
				break;
		if (sta->aid > MAX_AID_TABLE_SIZE) {
			sta->aid = 0;
			resp = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "  no room for more AIDs\n");
			goto fail;
		} else {
			hapd->sta_aid[sta->aid - 1] = sta;
			HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
				      "  new AID %d\n", sta->aid);
		}
	}

	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_DEBUG,
		       "association OK (aid %d)", sta->aid);
	/* Station will be marked associated, after it acknowledges AssocResp
	 */

	if (sta->last_assoc_req)
		free(sta->last_assoc_req);
	sta->last_assoc_req = (struct ieee80211_mgmt *) malloc(len);
	if (sta->last_assoc_req)
		memcpy(sta->last_assoc_req, mgmt, len);

    /* send the arp(b'cast) packet on the behalf of the new-associated station */
    IEEE802_11_send_arp(hapd, sta);
    
 fail:

	/* use the queued buffer for transmission because it is large enough
	 * and not needed anymore */
	mgmt->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_MGMT,
			     (send_deauth ? WLAN_FC_STYPE_DEAUTH :
			      (reassoc ? WLAN_FC_STYPE_REASSOC_RESP :
			       WLAN_FC_STYPE_ASSOC_RESP)));
	memcpy(mgmt->da, mgmt->sa, ETH_ALEN);
	memcpy(mgmt->sa, hapd->own_addr, ETH_ALEN);
	/* Addr3 = BSSID - already set */

	send_len = IEEE80211_HDRLEN;
	if (send_deauth) {
		send_len += sizeof(mgmt->u.deauth);
		mgmt->u.deauth.reason_code = host_to_le16(resp);
	} else {
		u8 *p;
		send_len += sizeof(mgmt->u.assoc_resp);
		mgmt->u.assoc_resp.capab_info =
			host_to_le16(hostapd_own_capab_info(hapd));
		mgmt->u.assoc_resp.status_code = host_to_le16(resp);
		mgmt->u.assoc_resp.aid = host_to_le16((sta ? sta->aid : 0)
						      | BIT(14) | BIT(15));
		/* Supported rates */
		p = hostapd_eid_supp_rates(hapd, mgmt->u.assoc_resp.variable);
		send_len += p - mgmt->u.assoc_resp.variable;

		/* Request TX callback */
		mgmt->frame_control |= host_to_le16(BIT(1));
	}

	if (send(hapd->sock, mgmt, send_len, 0) < 0)
		perror("handle_assoc: send");
}


static void handle_assoc_resp(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			      size_t len)
{
	u16 status_code, aid;
	struct mvwlan_hostapd_param param;

	if (hapd->assoc_ap_state != ASSOCIATE) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Unexpected association response received from " MACSTR
		       "\n", MAC2STR(mgmt->sa));
		return;
	}

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.assoc_resp)) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "handle_assoc_resp - too short payload (len=%d)\n",
		       len);
		return;
	}

	if (memcmp(mgmt->sa, hapd->conf->assoc_ap_addr, ETH_ALEN) != 0 ||
	    memcmp(mgmt->bssid, hapd->conf->assoc_ap_addr, ETH_ALEN) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Received association response from unexpected address "
		       "(SA=" MACSTR " BSSID=" MACSTR "\n",
		       MAC2STR(mgmt->sa), MAC2STR(mgmt->bssid));
		return;
	}

	status_code = le_to_host16(mgmt->u.assoc_resp.status_code);
	aid = le_to_host16(mgmt->u.assoc_resp.aid);
	aid &= ~(BIT(14) | BIT(15));

	if (status_code != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Association (as station) with AP " MACSTR " failed "
		       "(status_code=%d)\n",
		       MAC2STR(hapd->conf->assoc_ap_addr), status_code);
		/* Try to authenticate again */
		hapd->assoc_ap_state = AUTHENTICATE;
		eloop_register_timeout(5, 0, ieee802_11_sta_authenticate,
				       hapd, NULL);
	}

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Associated (as station) with AP " MACSTR " (aid=%d)\n",
	       MAC2STR(hapd->conf->assoc_ap_addr), aid);
	hapd->assoc_ap_aid = aid;
	hapd->assoc_ap_state = ASSOCIATED;

	memset(&param, 0, sizeof(param));
	param.cmd = MVWLAN_HOSTAPD_SET_ASSOC_AP_ADDR;
	memcpy(param.sta_addr, hapd->conf->assoc_ap_addr, ETH_ALEN);
	if (hostapd_ioctl(hapd, &param, sizeof(param))) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Could not set associated AP address to kernel "
		       "driver.\n");
	}
}


static void handle_disassoc(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			    size_t len)
{
	struct sta_info *sta;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.disassoc)) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "handle_disassoc - too short payload (len=%d)\n", len);
		return;
	}

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "disassocation: STA=" MACSTR " reason_code=%d\n",
		      MAC2STR(mgmt->sa),
		      le_to_host16(mgmt->u.disassoc.reason_code));

	if (hapd->assoc_ap_state != DO_NOT_ASSOC &&
	    memcmp(mgmt->sa, hapd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Assoc AP " MACSTR " sent disassociation "
		       "(reason_code=%d) - try to authenticate\n",
		       MAC2STR(hapd->conf->assoc_ap_addr),
		       le_to_host16(mgmt->u.disassoc.reason_code));
		hapd->assoc_ap_state = AUTHENTICATE;
		ieee802_11_sta_authenticate(hapd, NULL);
		eloop_register_timeout(0, 500000, ieee802_11_sta_authenticate,
				       hapd, NULL);
		return;
	}

	sta = ap_get_sta(hapd, mgmt->sa);
	if (sta == NULL) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Station " MACSTR " trying to disassociate, but it "
		       "is not associated.\n", MAC2STR(mgmt->sa));
		return;
	}

	sta->flags &= ~WLAN_STA_ASSOC;
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_INFO, "disassociated");
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
	ieee802_1x_set_port_enabled(hapd, sta, 0);
	ieee802_1x_free_station(sta);
	remove_sta(hapd, sta);
}


static void handle_deauth(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			  size_t len)
{
	struct sta_info *sta;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.deauth)) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "handle_deauth - too short payload (len=%d)\n", len);
		return;
	}

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
		      "deauthentication: STA=" MACSTR " reason_code=%d\n",
		      MAC2STR(mgmt->sa),
		      le_to_host16(mgmt->u.deauth.reason_code));

	if (hapd->assoc_ap_state != DO_NOT_ASSOC &&
	    memcmp(mgmt->sa, hapd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Assoc AP " MACSTR " sent deauthentication "
		       "(reason_code=%d) - try to authenticate\n",
		       MAC2STR(hapd->conf->assoc_ap_addr),
		       le_to_host16(mgmt->u.deauth.reason_code));
		hapd->assoc_ap_state = AUTHENTICATE;
		eloop_register_timeout(0, 500000, ieee802_11_sta_authenticate,
				       hapd, NULL);
		return;
	}

	sta = ap_get_sta(hapd, mgmt->sa);
	if (sta == NULL) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Station " MACSTR " trying to deauthenticate, but it "
		       "is not authenticated.\n", MAC2STR(mgmt->sa));
		return;
	}

	sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_DEBUG, "deauthenticated");
	sta->acct_terminate_cause = RADIUS_ACCT_TERMINATE_CAUSE_USER_REQUEST;
	ieee802_1x_set_port_enabled(hapd, sta, 0);
	ieee802_1x_free_station(sta);
	remove_sta(hapd, sta);
}


static void handle_beacon(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			  size_t len)
{
	struct ieee802_11_elems elems;

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.beacon)) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "handle_beacon - too short payload (len=%d)\n", len);
		return;
	}

	(void) ieee802_11_parse_elems(hapd, mgmt->u.beacon.variable,
				      len - (IEEE80211_HDRLEN +
					     sizeof(mgmt->u.beacon)), &elems);

	if (hapd->assoc_ap_state == WAIT_BEACON &&
	    memcmp(mgmt->sa, hapd->conf->assoc_ap_addr, ETH_ALEN) == 0) {
		if (elems.ssid && elems.ssid_len <= 32) {
			memcpy(hapd->assoc_ap_ssid, elems.ssid,
			       elems.ssid_len);
			hapd->assoc_ap_ssid[elems.ssid_len] = '\0';
			hapd->assoc_ap_ssid_len = elems.ssid_len;
		}
		ieee802_11_sta_authenticate(hapd, NULL);
	}

	if (!HOSTAPD_DEBUG_COND(HOSTAPD_DEBUG_VERBOSE))
		return;

	printf("Beacon from " MACSTR, MAC2STR(mgmt->sa));
	if (elems.ssid) {
		printf(" SSID='");
		ieee802_11_print_ssid(HOSTAPD_DEBUG_VERBOSE, hapd, elems.ssid, elems.ssid_len);
		printf("'");
	}
	if (elems.ds_params && elems.ds_params_len == 1)
		printf(" CHAN=%d", elems.ds_params[0]);
	printf("\n");
}


void ieee802_11_mgmt(hostapd *hapd, char *buf, size_t len, u16 stype)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) buf;

	if (stype == WLAN_FC_STYPE_BEACON) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_VERBOSE, "mgmt::beacon\n");
		handle_beacon(hapd, mgmt, len);
		return;
	}

	if (memcmp(mgmt->bssid, hapd->own_addr, ETH_ALEN) != 0 &&
	    (hapd->assoc_ap_state == DO_NOT_ASSOC ||
	     memcmp(mgmt->bssid, hapd->conf->assoc_ap_addr, ETH_ALEN) != 0)) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "MGMT: BSSID=" MACSTR " not our address\n",
		       MAC2STR(mgmt->bssid));
		return;
	}


	if (stype == WLAN_FC_STYPE_PROBE_REQ) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::probe_req\n");
		return;
	}

	if (memcmp(mgmt->da, hapd->own_addr, ETH_ALEN) != 0) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "MGMT: DA=" MACSTR " not our address\n",
		       MAC2STR(mgmt->da));
		return;
	}

	switch (stype) {
	case WLAN_FC_STYPE_AUTH:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::auth\n");
		handle_auth(hapd, mgmt, len);
		break;
	case WLAN_FC_STYPE_ASSOC_REQ:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::assoc_req\n");
		handle_assoc(hapd, mgmt, len, 0);
		break;
	case WLAN_FC_STYPE_ASSOC_RESP:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::assoc_resp\n");
		handle_assoc_resp(hapd, mgmt, len);
		break;
	case WLAN_FC_STYPE_REASSOC_REQ:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::reassoc_req\n");
		handle_assoc(hapd, mgmt, len, 1);
		break;
	case WLAN_FC_STYPE_DISASSOC:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::disassoc\n");
		handle_disassoc(hapd, mgmt, len);
		break;
	case WLAN_FC_STYPE_DEAUTH:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::deauth\n");
		handle_deauth(hapd, mgmt, len);
		break;
	default:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "unknown mgmt frame subtype %d\n", stype);
		break;
	}
}


static void handle_auth_cb(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			   size_t len, int ok)
{
	u16 auth_alg, auth_transaction, status_code;
	struct sta_info *sta;

	if (!ok) {
		hostapd_logger(hapd, mgmt->da, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_NOTICE,
			       "did not acknowledge authentication response");
		return;
	}

	if (len < IEEE80211_HDRLEN + sizeof(mgmt->u.auth)) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "handle_auth_cb - too short payload (len=%d)\n", len);
		return;
	}

	auth_alg = le_to_host16(mgmt->u.auth.auth_alg);
	auth_transaction = le_to_host16(mgmt->u.auth.auth_transaction);
	status_code = le_to_host16(mgmt->u.auth.status_code);

	sta = ap_get_sta(hapd, mgmt->da);
	if (!sta) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "handle_auth_cb: STA " MACSTR " not found\n",
		       MAC2STR(mgmt->da));
		return;
	}

	if (status_code == WLAN_STATUS_SUCCESS &&
	    ((auth_alg == WLAN_AUTH_OPEN && auth_transaction == 2) ||
	     (auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 4))) {
		hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_INFO, "authenticated");
		sta->flags |= WLAN_STA_AUTH;
	}
}


static void handle_assoc_cb(hostapd *hapd, struct ieee80211_mgmt *mgmt,
			    size_t len, int reassoc, int ok)
{
	u16 status;
	struct mvwlan_hostapd_param param;
	struct sta_info *sta;
	int new_assoc = 1;

	if (!ok) {
		hostapd_logger(hapd, mgmt->da, HOSTAPD_MODULE_IEEE80211,
			       HOSTAPD_LEVEL_DEBUG,
			       "did not acknowledge association response");
		return;
	}

	if (len < IEEE80211_HDRLEN + (reassoc ? sizeof(mgmt->u.reassoc_req) :
				      sizeof(mgmt->u.assoc_req))) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "handle_assoc_cb(reassoc=%d) - too short payload "
		       "(len=%d)\n", reassoc, len);
		return;
	}

	if (reassoc)
		status = le_to_host16(mgmt->u.reassoc_resp.status_code);
	else
		status = le_to_host16(mgmt->u.assoc_resp.status_code);

	sta = ap_get_sta(hapd, mgmt->da);
	if (!sta) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "handle_assoc_cb: STA " MACSTR " not found\n",
		       MAC2STR(mgmt->da));
		return;
	}

	if (status != WLAN_STATUS_SUCCESS) {
		sta->flags &= ~WLAN_STA_ASSOC;
		goto fail;
	}

	hostapd_logger(hapd, sta->addr, HOSTAPD_MODULE_IEEE80211,
		       HOSTAPD_LEVEL_INFO, "associated (aid %d)", sta->aid);
	if (sta->flags & WLAN_STA_ASSOC)
		new_assoc = 0;
	sta->flags |= WLAN_STA_ASSOC;

	memset(&param, 0, sizeof(param));
	param.cmd = MVWLAN_HOSTAPD_ADD_STA;
	memcpy(param.sta_addr, sta->addr, ETH_ALEN);
	param.u.add_sta.aid = sta->aid;
	param.u.add_sta.capability = sta->capability;
	param.u.add_sta.tx_supp_rates = sta->tx_supp_rates;
	if (hostapd_ioctl(hapd, &param, sizeof(param))) {
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "Could not add station to kernel driver.\n");
	}

	if (new_assoc) {

		hostapd_new_assoc_sta(hapd, sta);
	} else {

		if (sta->eapol_sm) {
			eapol_sm_initialize(sta->eapol_sm);
			sta->eapol_sm->portEnabled = TRUE;
		}
	}

#ifdef BOOSTER_MODE

	if (hapd->conf->speed_booster) {

		u8 broadcast_addr[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

		if (IEEE802_11_AssocStaNum(hapd) == 1)
			IEEE802_11_SendMarvellOUIPck(hapd, broadcast_addr);
		else
			IEEE802_11_SendMarvellOUIPck2(hapd, broadcast_addr);

	}

#endif

 fail:
	/* Copy of the association request is not needed anymore */
	if (sta->last_assoc_req) {
		free(sta->last_assoc_req);
		sta->last_assoc_req = NULL;
	}
}


void ieee802_11_mgmt_cb(hostapd *hapd, char *buf, size_t len, u16 stype,
			int ok)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *) buf;

	switch (stype) {
	case WLAN_FC_STYPE_AUTH:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL, "mgmt::auth cb\n");
		handle_auth_cb(hapd, mgmt, len, ok);
		break;
	case WLAN_FC_STYPE_ASSOC_RESP:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "mgmt::assoc_resp cb\n");
		handle_assoc_cb(hapd, mgmt, len, 0, ok);
		break;
	case WLAN_FC_STYPE_REASSOC_RESP:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_MINIMAL,
			      "mgmt::reassoc_resp cb\n");
		handle_assoc_cb(hapd, mgmt, len, 1, ok);
		break;
	default:
		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "unknown mgmt cb frame subtype %d\n", stype);
		break;
	}
}


/* PRIVATE FUNCTION DEFINITION
 */

static u32 IEEE802_11_GetHighestRateIndex(struct ieee802_11_elems *elems_p, Boolean *g_rate_exist)
{
   int rate_i, index, highest_rate_index;
   u8  highest_rate, rate;

   highest_rate = 0;
   *g_rate_exist = FALSE;

   if (elems_p->supp_rates)
   {
      for (rate_i = 0; rate_i < elems_p->supp_rates_len; rate_i++)
      {
         rate = elems_p->supp_rates[rate_i];

         index = rate2index(rate);

         if (index == 0) continue;

         if (index > WLAN_RATE_22M)
            *g_rate_exist = TRUE;

         if ((rate & IEEE802_11_BASIC_RATE_MASK) > highest_rate)
         {
            highest_rate = (rate & IEEE802_11_BASIC_RATE_MASK);
            highest_rate_index = index;
         }
      }
   }

   if (elems_p->ext_supp_rates)
   {
      for (rate_i = 0; rate_i < elems_p->ext_supp_rates_len; rate_i++)
      {
         rate = elems_p->ext_supp_rates[rate_i];

         index = rate2index(rate);

         if (index == 0) continue;

         if (index > WLAN_RATE_22M)
            *g_rate_exist = TRUE;

         if ((rate & IEEE802_11_BASIC_RATE_MASK) > highest_rate)
         {
            highest_rate = (rate & IEEE802_11_BASIC_RATE_MASK);
            highest_rate_index = index;
         }
      }
   }

   return (highest_rate_index);
}


static Boolean IEEE802_11_IsBasicRateSupported(hostapd *hapd_p, struct ieee802_11_elems *elems_p)
{
   int rate_i, rate_j;
   u32 basic_index, basic_rate_set;
   u8  basic_rate, supp_rate;

   /* get our AP basic rate set
    */
   basic_rate_set = hapd_p->conf->basic_rate;

   for (rate_i = 0; rate_i < HOSTAPD_RATE_LEN; rate_i++)
   {
      basic_index = (basic_rate_set & (1 << rate_i));

      if (basic_index == 0) continue;

      basic_rate = index2rate(basic_index);

      /* check if this basic rate is supported in support rates
       */
      if (elems_p->supp_rates)
      {
         for (rate_j = 0; rate_j < elems_p->supp_rates_len; rate_j++)
         {
            supp_rate = elems_p->supp_rates[rate_j];

            /* check if this is basic rate
             */
            if (supp_rate & IEEE802_11_BASIC_RATE_FLAG)
            {
               if (basic_rate == (supp_rate & IEEE802_11_BASIC_RATE_MASK))
                  break;
            }
         }

         if (rate_j < elems_p->supp_rates_len)
            continue;
      }

      /* check if this basic rate is supported in extended support rates
       */
      if (elems_p->ext_supp_rates)
      {
         for (rate_j = 0; rate_j < elems_p->ext_supp_rates_len; rate_j++)
         {
            supp_rate = elems_p->ext_supp_rates[rate_j];

            /* check if this is basic rate
             */
            if (supp_rate & IEEE802_11_BASIC_RATE_FLAG)
            {
               if (basic_rate == (supp_rate & IEEE802_11_BASIC_RATE_MASK))
                  break;
            }
         }

         if (rate_j < elems_p->ext_supp_rates_len)
            continue;
         else
            return FALSE;
      }
   }

   return TRUE;
}


static Boolean IEEE802_11_IsCurShortSlotTime(hostapd *hapd)
{
	struct mvwlan_hostapd_param *param_p;
	char *buf_p;
	int blen, dlen;
	u32 mac_current_slot_time;

	dlen = sizeof(u32);
	blen = sizeof(*param_p) + dlen;

	buf_p = malloc(blen);
	if (buf_p == (char *) NULL)   {

		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "IEEE802_11: Can not get current slot time, no memory!\n");
		return FALSE; /* something wrong, think we are long slot time */
	}
	memset(buf_p, 0, blen);

	param_p = (struct mvwlan_hostapd_param *) buf_p;
	param_p->cmd = MVWLAN_HOSTAPD_GET_CMD;
	param_p->u.cmd.cmd = APCMD_GET_CURRENT_SLOT_TIME;
	param_p->u.cmd.len = dlen;

	if (hostapd_ioctl(hapd, param_p, blen)) {

		HOSTAPD_DEBUG(HOSTAPD_DEBUG_NO, "IEEE802_11: Can not get current slot time, I/O error!\n");
		return FALSE; /* something wrong, think we are long slot time */
	}

	memcpy(&mac_current_slot_time, (u8 *) (param_p->u.cmd.data), dlen);

	free(buf_p);

	if (mac_current_slot_time == 0)
		return TRUE;
	else
		return FALSE;
}


#ifdef BOOSTER_MODE

#define MARVELL_OUIPKT_BODY_LEN      100
#define MARVELL_OUIPKT_LEN           (IEEE80211_HDRLEN + MARVELL_OUIPKT_BODY_LEN)


static u32 IEEE802_11_AssocStaNum(hostapd *hapd_p)
{
	struct sta_info *sta_p;
	u32 assoc_sta_num;

	assoc_sta_num = 0;

	for (sta_p = hapd_p->sta_list; sta_p != NULL; sta_p = sta_p->next) {

		if (sta_p->flags & WLAN_STA_ASSOC)
			assoc_sta_num++;
	}

	return (assoc_sta_num);
}


static void IEEE802_11_SendMarvellOUIPck(hostapd *hapd_p, u8 *sta_mac_addr)
{
	u8 buf[MARVELL_OUIPKT_LEN];
	struct ieee80211_hdr *marvell_ouipkt_hdr_p;
	u8 *marvell_ouipkt_body_p;

	memset(buf, 0, sizeof(buf));
	marvell_ouipkt_hdr_p = (struct ieee80211_hdr *) buf;
	marvell_ouipkt_body_p = (buf + IEEE80211_HDRLEN);

	marvell_ouipkt_hdr_p->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
	marvell_ouipkt_hdr_p->frame_control |= host_to_le16(WLAN_FC_FROMDS);
	memcpy(marvell_ouipkt_hdr_p->addr1, sta_mac_addr, ETH_ALEN);
	memcpy(marvell_ouipkt_hdr_p->addr2, hapd_p->own_addr, ETH_ALEN);
	memcpy(marvell_ouipkt_hdr_p->addr3, hapd_p->own_addr, ETH_ALEN);

	marvell_ouipkt_body_p[0] = 0xAA;  /* SNAP Header 3 bytes */
	marvell_ouipkt_body_p[1] = 0xAA;
	marvell_ouipkt_body_p[2] = 0x03;
	marvell_ouipkt_body_p[3] = 0x00;  /* Marvell OUI 3 bytes */
	marvell_ouipkt_body_p[4] = 0x50;
	marvell_ouipkt_body_p[5] = 0x43;
	marvell_ouipkt_body_p[6] = 0x00;  /* Product type 2 bytes */
	marvell_ouipkt_body_p[7] = 0x00;
	marvell_ouipkt_body_p[8] = 0x02;  /* Length */
	marvell_ouipkt_body_p[9] = 0x01;  /* type - AP */
	marvell_ouipkt_body_p[10] = 1;   

	if (send(hapd_p->sock, buf, MARVELL_OUIPKT_LEN, 0) < 0) {
		perror("IEEE802_11_SendMarvellOUIPck: send");
		printf("IEEE802_11_SendMarvellOUIPck - packet len: %d - failed\n", MARVELL_OUIPKT_LEN);
	}
}


static void IEEE802_11_SendMarvellOUIPck2(hostapd *hapd_p, u8 *sta_mac_addr)
{
	u8 buf[MARVELL_OUIPKT_LEN];
	struct ieee80211_hdr *marvell_ouipkt_hdr_p;
	u8 *marvell_ouipkt_body_p;

	memset(buf, 0, sizeof(buf));
	marvell_ouipkt_hdr_p = (struct ieee80211_hdr *) buf;
	marvell_ouipkt_body_p = (buf + IEEE80211_HDRLEN);

	marvell_ouipkt_hdr_p->frame_control =
		IEEE80211_FC(WLAN_FC_TYPE_DATA, WLAN_FC_STYPE_DATA);
	marvell_ouipkt_hdr_p->frame_control |= host_to_le16(WLAN_FC_FROMDS);
	memcpy(marvell_ouipkt_hdr_p->addr1, sta_mac_addr, ETH_ALEN);
	memcpy(marvell_ouipkt_hdr_p->addr2, hapd_p->own_addr, ETH_ALEN);
	memcpy(marvell_ouipkt_hdr_p->addr3, hapd_p->own_addr, ETH_ALEN);

	marvell_ouipkt_body_p[0] = 0xAA;  /* SNAP Header 3 bytes */
	marvell_ouipkt_body_p[1] = 0xAA;
	marvell_ouipkt_body_p[2] = 0x03;
	marvell_ouipkt_body_p[3] = 0x00;  /* Marvell OUI 3 bytes */
	marvell_ouipkt_body_p[4] = 0x50;
	marvell_ouipkt_body_p[5] = 0x43;
	marvell_ouipkt_body_p[6] = 0x00;  /* Product type 2 bytes */
	marvell_ouipkt_body_p[7] = 0x00;
	marvell_ouipkt_body_p[8] = 0x02;  /* Length */
	marvell_ouipkt_body_p[9] = 0x01;  /* type - AP */
	marvell_ouipkt_body_p[10] = 2;

	if (send(hapd_p->sock, buf, MARVELL_OUIPKT_LEN, 0) < 0) {
		perror("IEEE802_11_SendMarvellOUIPck: send");
		printf("IEEE802_11_SendMarvellOUIPck - packet len: %d - failed\n", MARVELL_OUIPKT_LEN);
	}
}

#endif

#include <netdb.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <stdio.h> 
#include <errno.h> 
#include <sys/ioctl.h> 
#include <net/if.h> 
#include <signal.h> 
#include <netinet/ip.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h> 
#include <netinet/ip_icmp.h> 
#include <linux/if_ether.h> 
#include <net/if_arp.h>
#include <netpacket/packet.h>
#define ETH_HW_ADDR_LEN 6  
#define IP_ADDR_LEN 4  
#define ARP_FRAME_TYPE 0x0806  
#define ETHER_HW_TYPE 1  
#define IP_PROTO_TYPE 0x0800  
#define OP_ARP_REQUEST 2  
#define OP_ARP_QUEST 1 
#define DEFAULT_DEVICE "br0" 

struct arp_packet  
{  
    u_char targ_hw_addr[ETH_HW_ADDR_LEN];  
    u_char src_hw_addr[ETH_HW_ADDR_LEN];  
    u_short frame_type;  
    u_short hw_type;  
    u_short prot_type;  
    u_char hw_addr_size;  
    u_char prot_addr_size;  
    u_short op;  
    u_char sndr_hw_addr[ETH_HW_ADDR_LEN];  
    u_char sndr_ip_addr[IP_ADDR_LEN];  
    u_char rcpt_hw_addr[ETH_HW_ADDR_LEN];  
    u_char rcpt_ip_addr[IP_ADDR_LEN];      
};  

static void IEEE802_11_send_arp(hostapd *hapd, struct sta_info *sta)
{              
    struct arp_packet pkt;  
    struct ifreq ifr;
	struct sockaddr_ll addr;    
    u_char bcast[ETH_HW_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    
    int sock;
    
    sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));  
    
    if (sock < 0)  
    {  
        perror("socket");  
        return;  
    }                 
    
    /* set up the arp packet */    
    memset(&pkt, 0, sizeof(pkt));
    memcpy(pkt.targ_hw_addr, bcast, ETH_HW_ADDR_LEN);
    memcpy(pkt.src_hw_addr, sta->addr, ETH_HW_ADDR_LEN);
    pkt.frame_type = htons(ARP_FRAME_TYPE);  
    pkt.hw_type = htons(ETHER_HW_TYPE);  
    pkt.prot_type = htons(IP_PROTO_TYPE);  
    pkt.hw_addr_size = ETH_HW_ADDR_LEN;  
    pkt.prot_addr_size = IP_ADDR_LEN;  
    pkt.op = htons(OP_ARP_QUEST); 
    memcpy(pkt.sndr_hw_addr, sta->addr, ETH_HW_ADDR_LEN);                   
    
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", DEFAULT_DEVICE);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) != 0) {
		perror("ioctl(SIOCGIFINDEX)");
		goto err;
    }

    memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifr.ifr_ifindex;

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		goto err;
	}
    
    if (send(sock,&pkt,sizeof(pkt),0) < 0)  
    {  
        perror("send");  
        goto err;
    }
                 
err:
    close(sock);
    return;
} 