#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <linux/if_ether.h>

#include "ieee80211.h"

const char *mac2str(unsigned char *addr); /* wlansniff.c */


static int show_timestamp(unsigned char *pos, int len)
{
	int i;

	printf("  Timestamp: ");
	if (len < 8) {
		printf(" underflow!\n");
		return 0;
	}

	for (i = 7; i > 0; i--)
		printf("%02x", pos[i]);
	printf("\n");

	return 8;
}

static int show_beacon_interval(unsigned char *pos, int len)
{
	u16 *ival;

	printf("  Beacon interval: ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ival = (u16 *) pos;
	printf("%u\n", le_to_host16(*ival));

	return 2;
}

static int show_listen_interval(unsigned char *pos, int len)
{
	u16 *ival;

	printf("  Listen interval: ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ival = (u16 *) pos;
	printf("%u\n", le_to_host16(*ival));

	return 2;
}

static int show_status_code(unsigned char *pos, int len)
{
	u16 *ival;

	printf("  Status code: ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ival = (u16 *) pos;
	printf("%u=", le_to_host16(*ival));
	switch (le_to_host16(*ival)) {
	case WLAN_STATUS_SUCCESS:
		printf("Successful");
		break;
	case WLAN_STATUS_UNSPECIFIED_FAILURE:
		printf("Unspecified failure");
		break;
	case WLAN_STATUS_CAPS_UNSUPPORTED:
		printf("Cannot support all requested capabilities");
		break;
	case WLAN_STATUS_REASSOC_NO_ASSOC:
		printf("Reassociation denied due to inablity to confirm that "
		       "association exists");
		break;
	case WLAN_STATUS_ASSOC_DENIED_UNSPEC:
		printf("Association denied due to reason outside the scope of "
		       "802.11 standard");
		break;
	case WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG:
		printf("Responding station does not support the specified "
		       "authentication algorithm");
		break;
	case WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION:
		printf("Received an Authentication frame with authentication "
		       "transaction seq# out of expected sequence");
		break;
	case WLAN_STATUS_CHALLENGE_FAIL:
		printf("Authentication rejected because of challenge failure");
		break;
	case WLAN_STATUS_AUTH_TIMEOUT:
		printf("Authentication rejected due to timeout waiting for "
		       "next frame in sequence");
		break;
	case WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA:
		printf("Association denied because AP is unable to handle "
		       "additional associated stations");
		break;
	case WLAN_STATUS_ASSOC_DENIED_RATES:
		printf("Association denied due to requesting station not "
		       "supporting all of the data rates in the "
		       "BSSBasicRateSet parameter");
		break;
	case WLAN_STATUS_ASSOC_DENIED_NOSHORT:
		printf("Association denied due to requesting station not "
		       "supporting the Short Preample option[802.11b]");
		break;
	case WLAN_STATUS_ASSOC_DENIED_NOPBCC:
		printf("Association denied due to requesting station not "
		       "supporting the PBCC Modulation option[802.11b]");
		break;
	case WLAN_STATUS_ASSOC_DENIED_NOAGILITY:
		printf("Association denied due to requesting station not "
		       "supporting the Channel Agility option[802.11b]");
		break;
	default:
		printf("Reserved");
		break;
	}

	printf("\n");
	return 2;
}

static int show_reason_code(unsigned char *pos, int len)
{
	u16 *ival;

	printf("  Status code: ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ival = (u16 *) pos;
	printf("%u=", le_to_host16(*ival));
	switch (le_to_host16(*ival)) {
	case WLAN_REASON_UNSPECIFIED:
		printf("Unspecified reason");
		break;
	case WLAN_REASON_PREV_AUTH_NOT_VALID:
		printf("Previous authentication no longer valid");
		break;
	case WLAN_REASON_DEAUTH_LEAVING:
		printf("Deauthenticated because sending station is leaving "
		       "(or has left) IBSS or ESS");
		break;
	case WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY:
		printf("Disassociated due to inactivity");
		break;
	case WLAN_REASON_DISASSOC_AP_BUSY:
		printf("Disassociated because AP is unable to handle all "
		       "currently associated stations");
		break;
	case WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA:
		printf("Class 2 frame received from nonauthenticated station");
		break;
	case WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA:
		printf("Class 3 frame received from nonassociated station");
		break;
	case WLAN_REASON_DISASSOC_STA_HAS_LEFT:
		printf("Disassociated because sending station is leaving "
		       "(or has left) BSS");
		break;
	case WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH:
		printf("Station requesting (re)association is not "
		       "authenticated with responding station");
		break;
	default:
		printf("Reserved");
		break;
	}

	printf("\n");
	return 2;
}

static int show_aid(unsigned char *pos, int len)
{
	u16 *ival;

	printf("  Association ID (AID): ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ival = (u16 *) pos;
	printf("%u\n", le_to_host16(*ival));
	return 2;
}

static int show_auth_alg(unsigned char *pos, int len)
{
	u16 *ival;

	printf("  Authentication Algorithm Number: ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ival = (u16 *) pos;
	printf("%u=", le_to_host16(*ival));
	switch (le_to_host16(*ival)) {
	case 0:
		printf("Open System");
		break;
	case 1:
		printf("Shared Key");
		break;
	default:
		printf("Reserved");
		break;
	}
	printf("\n");
	return 2;
}

static int show_auth_trans(unsigned char *pos, int len)
{
	u16 *ival;

	printf("  Authentication Transaction Number: ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ival = (u16 *) pos;
	printf("%u\n", le_to_host16(*ival));
	return 2;
}

static int show_current_ap(unsigned char *pos, int len)
{
	printf("  Current AP address): ");
	if (len < 6) {
		printf(" underflow!\n");
		return 0;
	}

	printf("%s\n", mac2str(pos));
	return 6;
}

static int show_capability_info(unsigned char *pos, int len)
{
	u16 *ptr, cap;

	printf("  Capability information: ");
	if (len < 2) {
		printf(" underflow!\n");
		return 0;
	}

	ptr = (u16 *) pos;
	cap = le_to_host16(*ptr);
	printf("0x%04x", cap);
	if (cap & BIT(0))
		printf(" ESS");
	if (cap & BIT(1))
		printf(" IBSS");
	if (cap & BIT(2))
		printf(" CF_Pollable");
	if (cap & BIT(3))
		printf(" CF_Poll_Request");
	if (cap & BIT(4))
		printf(" Privacy");
	if (cap & BIT(5))
		printf(" Short_Preample[802.11b]");
	if (cap & BIT(6))
		printf(" PBCC_Modulation[802.11b]");
	if (cap & BIT(7))
		printf(" Channel_Agility[802.11b]");
	if (cap & (BIT(8) | BIT(9) | BIT(10) | BIT(11) | BIT(12) | BIT(13) |
		   BIT(14)))
		printf(" Reserved!?");
	printf("\n");

	return 2;
}

static const char *eid2str(unsigned char eid)
{
	switch (eid) {
	case WLAN_EID_SSID:
		return "Service Set Identify (SSID)";
	case WLAN_EID_SUPP_RATES:
		return "Supported rates";
	case WLAN_EID_FH_PARAMS:
		return "FH Parameter Set";
	case WLAN_EID_DS_PARAMS:
		return "DS Parameter Set";
	case WLAN_EID_CF_PARAMS:
		return "CF Parameter Set";
	case WLAN_EID_TIM:
		return "TIM";
	case WLAN_EID_IBSS_PARAMS:
		return "IBSS Parameter Set";
	case WLAN_EID_CHALLENGE:
		return "Challenge text";
	}

	if (eid >= 17 && eid <= 31)
		return "reserved EID for challenge text ext.";

	return "reserved EID";
}

static void show_elem_supp_rates(unsigned char *data, int len)
{
	int i;

	printf("(Mbit/s) BSSBasicRateSet:");
	for (i = 0; i < len; i++) {
		if ((data[i] & BIT(7)) == 0)
			continue;
		printf(" %i", (data[i] & ~BIT(7)) / 2);
		if (data[i] & 1)
			printf(".5");
	}

	printf(" Others:");
	for (i = 0; i < len; i++) {
		if ((data[i] & BIT(7)) == BIT(7))
			continue;
		printf(" %i", (data[i] & ~BIT(7)) / 2);
		if (data[i] & 1)
			printf(".5");
	}
}

static void show_elem_fh_parms(unsigned char *data, int len)
{
	printf("Dwell_Time=%u  Hop_Set=%u  Hop_Patterns=%u  Hop_Index=%u",
	       data[0] + 256 * data[1], data[2], data[3], data[4]);
}

static void show_elem_cf_parms(unsigned char *data, int len)
{
	printf("CFP_Count=%u  CFP_Period=%u  CFP_MaxDuration=%u  "
	       "CFP_DurRemaining=%u", data[0], data[1],
	       data[2] + 256 * data[3], data[4] + 256 * data[5]);
}

static void show_elem_tim(unsigned char *data, int len)
{
	int i;

	printf("DTIM_Count=%u  DTIM_Period=%u  Bitmap_Control=0x%02x  "
	       "Partial Virtual Bitmap: ", data[0], data[1],
	       data[2]);

	for (i = 3; i < len; i++)
		printf("%02x", data[i]);
}

static int show_element(unsigned char eid, unsigned char *pos, int len)
{
	int i;
	unsigned int infolen;
	unsigned char *start;

	if (eid != pos[0])
		return 0; /* this element not present */

	printf("  ElemID #%u (%s):", eid, eid2str(eid));
	infolen = pos[1];
	printf("\n    len=%u  ", infolen);
	if (len < infolen + 2) {
		printf("underflow!\n");
		return 0;
	}

	start = pos + 2;
	for (i = 0; i < infolen; i++)
		printf("%02x", start[i]);
	printf(" = ");

	switch (eid) {
	case WLAN_EID_SUPP_RATES:
		show_elem_supp_rates(start, infolen);
		break;
	case WLAN_EID_FH_PARAMS:
		show_elem_fh_parms(start, infolen);
		break;
	case WLAN_EID_DS_PARAMS:
		printf("Current_Channel=%u", *start);
		break;
	case WLAN_EID_CF_PARAMS:
		show_elem_cf_parms(start, infolen);
		break;
	case WLAN_EID_TIM:
		show_elem_tim(start, infolen);
		break;
	case WLAN_EID_IBSS_PARAMS:
		printf("ATIM_Window=%u", start[0] + 256 * start[1]);
		break;
	case WLAN_EID_SSID:
	case WLAN_EID_CHALLENGE:
	default:
		for (i = 0; i < infolen; i++) {
			if (isprint(start[i]))
				printf("%c", start[i]);
			else
				printf("_");
		}
		break;
	}

	printf("\n");

	return infolen + 2;
}

static int show_frame_assoc_req(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Association request frame body:\n");
	used += show_capability_info(pos, len);
	used += show_listen_interval(pos + used, len - used);
	used += show_element(WLAN_EID_SSID, pos + used, len - used);
	used += show_element(WLAN_EID_SUPP_RATES, pos + used, len - used);

	return used;
}

static int show_frame_assoc_resp(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Association response frame body:\n");
	used += show_capability_info(pos, len);
	used += show_status_code(pos + used, len - used);
	used += show_aid(pos + used, len - used);
	used += show_element(WLAN_EID_SUPP_RATES, pos + used, len - used);

	return used;
}

static int show_frame_reassoc_req(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Reassociation request frame body:\n");
	used += show_capability_info(pos, len);
	used += show_listen_interval(pos + used, len - used);
	used += show_current_ap(pos + used, len - used);
	used += show_element(WLAN_EID_SSID, pos + used, len - used);
	used += show_element(WLAN_EID_SUPP_RATES, pos + used, len - used);

	return used;
}

static int show_frame_reassoc_resp(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Reassociation response frame body:\n");
	used += show_capability_info(pos, len);
	used += show_status_code(pos + used, len - used);
	used += show_aid(pos + used, len - used);
	used += show_element(WLAN_EID_SUPP_RATES, pos + used, len - used);

	return used;
}

static int show_frame_probe_req(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Probe request frame body:\n");
	used += show_element(WLAN_EID_SSID, pos + used, len - used);
	used += show_element(WLAN_EID_SUPP_RATES, pos + used, len - used);

	return used;
}

static int show_frame_probe_resp(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Probe response frame body:\n");
	used += show_timestamp(pos, len);
	used += show_beacon_interval(pos + used, len - used);
	used += show_capability_info(pos + used, len - used);
	used += show_element(WLAN_EID_SSID, pos + used, len - used);
	used += show_element(WLAN_EID_SUPP_RATES, pos + used, len - used);
	used += show_element(WLAN_EID_FH_PARAMS, pos + used, len - used);
	used += show_element(WLAN_EID_DS_PARAMS, pos + used, len - used);
	used += show_element(WLAN_EID_CF_PARAMS, pos + used, len - used);
	used += show_element(WLAN_EID_IBSS_PARAMS, pos + used, len - used);

	return used;
}

static int show_frame_beacon(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Beacon frame body:\n");
	used += show_timestamp(pos, len);
	used += show_beacon_interval(pos + used, len - used);
	used += show_capability_info(pos + used, len - used);
	used += show_element(WLAN_EID_SSID, pos + used, len - used);
	used += show_element(WLAN_EID_SUPP_RATES, pos + used, len - used);
	used += show_element(WLAN_EID_FH_PARAMS, pos + used, len - used);
	used += show_element(WLAN_EID_DS_PARAMS, pos + used, len - used);
	used += show_element(WLAN_EID_CF_PARAMS, pos + used, len - used);
	used += show_element(WLAN_EID_IBSS_PARAMS, pos + used, len - used);
	used += show_element(WLAN_EID_TIM, pos + used, len - used);

	return used;
}

static int show_frame_atim(unsigned char *pos, int len)
{
	printf("  IBSS Announcement Traffic Indication Message (ATIM) "
	       "frame\n");
	/* frame body is null */

	return 0;
}

static int show_frame_disassoc(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Disassociation frame body:\n");
	used += show_reason_code(pos, len);

	return used;
}

static int show_frame_auth(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Authentication frame body:\n");
	{
		int i;
		printf("  ");
		for (i = 0; i < (len > 32 ? 32 : len); i++)
			printf("%02x ", pos[i]);
		printf("\n");
	}
	used += show_auth_alg(pos, len);
	used += show_auth_trans(pos + used, len - used);
	used += show_status_code(pos + used, len - used);
	used += show_element(WLAN_EID_CHALLENGE, pos + used, len - used);
#if 0
	printf("  Auth. Alg.    Trans.  Status code  Challenge text\n");
	printf("  Open System   1       Reserved     Not present\n");
	printf("  Open System   2       Status       Not present\n");
	printf("  Shared Key    1       Reserved     Not present\n");
	printf("  Shared Key    2       Status       Present\n");
	printf("  Shared Key    3       Reserved     Present\n");
	printf("  Shared Key    4       Status       Not present\n");
#endif

	return used;
}

static int show_frame_deauth(unsigned char *pos, int len)
{
	int used = 0;
	printf("  Deauthentication frame body:\n");
	used += show_reason_code(pos, len);

	return used;
}

int show_frame_management(unsigned int subtype, unsigned char *pos, int len)
{
	switch (subtype) {
	case WLAN_FC_STYPE_ASSOC_REQ:
		return show_frame_assoc_req(pos, len);
		break;
	case WLAN_FC_STYPE_ASSOC_RESP:
		return show_frame_assoc_resp(pos, len);
		break;
	case WLAN_FC_STYPE_REASSOC_REQ:
		return show_frame_reassoc_req(pos, len);
		break;
	case WLAN_FC_STYPE_REASSOC_RESP:
		return show_frame_reassoc_resp(pos, len);
		break;
	case WLAN_FC_STYPE_PROBE_REQ:
		return show_frame_probe_req(pos, len);
		break;
	case WLAN_FC_STYPE_PROBE_RESP:
		return show_frame_probe_resp(pos, len);
		break;
	case WLAN_FC_STYPE_BEACON:
		return show_frame_beacon(pos, len);
		break;
	case WLAN_FC_STYPE_ATIM:
		return show_frame_atim(pos, len);
		break;
	case WLAN_FC_STYPE_DISASSOC:
		return show_frame_disassoc(pos, len);
		break;
	case WLAN_FC_STYPE_AUTH:
		return show_frame_auth(pos, len);
		break;
	case WLAN_FC_STYPE_DEAUTH:
		return show_frame_deauth(pos, len);
		break;
	}

	return 0;
}
