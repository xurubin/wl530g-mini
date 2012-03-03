/*
 * Host AP (software wireless LAN access point) user space daemon for
 * Host AP kernel driver
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
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "eloop.h"
#include "hostapd.h"
#include "ieee802_1x.h"
#include "ieee802_11.h"
#include "accounting.h"
#include "eapol_sm.h"
#include "iapp.h"
#include "ap.h"
#include "ieee802_11_auth.h"
#include "sta_info.h"
#include "driver.h"
#include "radius_client.h"

#ifdef TURBO_SETUP
#include "auto_link.h"
#endif


struct hapd_interfaces {
	int count;
	hostapd **hapd;
};

unsigned char rfc1042_header[6] = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };

struct hapd_interfaces interfaces;

static void hostapd_reinit(hostapd *hapd);


void hostapd_logger(hostapd *hapd, u8 *addr, unsigned int module, int level,
		    char *fmt, ...)
{
	char *format, *module_str;
	int maxlen;
	va_list ap;
	int conf_syslog_level, conf_stdout_level;
	unsigned int conf_syslog, conf_stdout;

	maxlen = strlen(fmt) + 100;
	format = malloc(maxlen);
	if (!format)
		return;

	va_start(ap, fmt);

	if (hapd && hapd->conf) {
		conf_syslog_level = hapd->conf->logger_syslog_level;
		conf_stdout_level = hapd->conf->logger_stdout_level;
		conf_syslog = hapd->conf->logger_syslog;
		conf_stdout = hapd->conf->logger_stdout;
	} else {
		conf_syslog_level = conf_stdout_level = 0;
		conf_syslog = conf_stdout = (unsigned int) -1;
	}

	switch (module) {
	case HOSTAPD_MODULE_IEEE80211:
		module_str = "IEEE 802.11";
		break;
	case HOSTAPD_MODULE_IEEE8021X:
		module_str = "IEEE 802.1X";
		break;
	case HOSTAPD_MODULE_RADIUS:
		module_str = "RADIUS";
		break;
	default:
		module_str = NULL;
		break;
	}

	if (hapd && hapd->conf && addr)
		snprintf(format, maxlen, "%s: STA " MACSTR "%s%s: %s",
			 hapd->conf->iface, MAC2STR(addr),
			 module_str ? " " : "", module_str, fmt);
	else if (hapd && hapd->conf)
		snprintf(format, maxlen, "%s:%s%s %s",
			 hapd->conf->iface, module_str ? " " : "",
			 module_str, fmt);
	else if (addr)
		snprintf(format, maxlen, "STA " MACSTR "%s%s: %s",
			 MAC2STR(addr), module_str ? " " : "",
			 module_str, fmt);
	else
		snprintf(format, maxlen, "%s%s%s",
			 module_str, module_str ? ": " : "", fmt);

	if ((conf_stdout & module) && level >= conf_stdout_level) {
		vprintf(format, ap);
		printf("\n");
	}

	if ((conf_syslog & module) && level >= conf_syslog_level) {
		int priority;
		switch (level) {
		case HOSTAPD_LEVEL_DEBUG_VERBOSE:
		case HOSTAPD_LEVEL_DEBUG:
			priority = LOG_DEBUG;
			break;
		case HOSTAPD_LEVEL_INFO:
			priority = LOG_INFO;
			break;
		case HOSTAPD_LEVEL_NOTICE:
			priority = LOG_NOTICE;
			break;
		case HOSTAPD_LEVEL_WARNING:
			priority = LOG_WARNING;
			break;
		default:
			priority = LOG_INFO;
			break;
		}
		vsyslog(priority, format, ap);
	}

	free(format);

	va_end(ap);
}


static void hostapd_deauth_all_stas(hostapd *hapd)
{
	u8 addr[ETH_ALEN];

	memset(addr, 0xff, ETH_ALEN);
	ieee802_11_send_deauth(hapd, addr, WLAN_REASON_PREV_AUTH_NOT_VALID);
}


/* This function will be called whenever a station associates with the AP */
void hostapd_new_assoc_sta(hostapd *hapd, struct sta_info *sta)
{
	/* IEEE 802.11f (IAPP) */
	if (hapd->conf->ieee802_11f)
		iapp_new_station(hapd, sta);

	/* Start accounting here, if IEEE 802.1X is not used. IEEE 802.1X code
	 * will start accounting after the station has been authorized. */
	if (!hapd->conf->ieee802_1x)
		accounting_sta_start(hapd, sta);

	/* Start IEEE 802.1x authentication process for new stations */
	ieee802_1x_new_station(hapd, sta);
}


static void handle_term(int sig, void *eloop_ctx, void *signal_ctx)
{
	printf("Signal %d received - terminating\n", sig);
	eloop_terminate();
}


static void handle_reload(int sig, void *eloop_ctx, void *signal_ctx)
{
	struct hapd_interfaces *hapds = (struct hapd_interfaces *) eloop_ctx;
	int i;

	printf("Signal %d received - reloading configuration\n", sig);

	for (i = 0; i < hapds->count; i++) {
		hostapd *hapd = hapds->hapd[i];

		hostapd_reinit(hapd);
	}
}


#ifdef HOSTAPD_DUMP_STATE
static void hostapd_dump_state(hostapd *hapd)
{
	FILE *f;
	time_t now;
	struct sta_info *sta;
	int i;

	if (!hapd->conf->dump_log_name) {
		printf("Dump file not defined - ignoring dump request\n");
		return;
	}

	printf("Dumping hostapd state to '%s'\n", hapd->conf->dump_log_name);
	f = fopen(hapd->conf->dump_log_name, "w");
	if (f == NULL) {
		printf("Could not open dump file '%s' for writing.\n",
		       hapd->conf->dump_log_name);
		return;
	}

	time(&now);
	fprintf(f, "hostapd state dump - %s", ctime(&now));

	for (sta = hapd->sta_list; sta != NULL; sta = sta->next) {
		fprintf(f, "\nSTA=" MACSTR "\n", MAC2STR(sta->addr));

		fprintf(f,
			"  AID=%d flags=0x%x %s%s%s%s%s%s\n"
			"  capability=0x%x listen_interval=%d\n",
			sta->aid,
			sta->flags,
			(sta->flags & WLAN_STA_AUTH ? "[AUTH]" : ""),
			(sta->flags & WLAN_STA_ASSOC ? "[ASSOC]" : ""),
			(sta->flags & WLAN_STA_PS ? "[PS]" : ""),
			(sta->flags & WLAN_STA_TIM ? "[TIM]" : ""),
			(sta->flags & WLAN_STA_PERM ? "[PERM]" : ""),
			(sta->flags & WLAN_STA_AUTHORIZED ? "[AUTHORIZED]" :
			 ""),
			sta->capability,
			sta->listen_interval);

		fprintf(f, "  supported_rates=");
		for (i = 0; i < sizeof(sta->supported_rates); i++)
			if (sta->supported_rates[i] != 0)
				fprintf(f, "%02x ", sta->supported_rates[i]);
		fprintf(f, "%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
			(sta->tx_supp_rates & WLAN_RATE_1M ? "[1M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_2M ? "[2M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_5M5 ? "[5.5M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_11M ? "[11M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_22M ? "[22M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_6M ? "[6M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_9M ? "[9M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_12M ? "[12M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_18M ? "[18M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_24M ? "[24M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_36M ? "[36M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_48M ? "[48M]" : ""),
			(sta->tx_supp_rates & WLAN_RATE_54M ? "[54M]" : ""));

		fprintf(f,
			"  timeout_next=%s\n",
			(sta->timeout_next == STA_NULLFUNC ? "NULLFUNC POLL" :
			 (sta->timeout_next == STA_DISASSOC ? "DISASSOC" :
			  "DEAUTH")));

		ieee802_1x_dump_state(f, "  ", sta);
	}

	fclose(f);
}
#endif /* HOSTAPD_DUMP_STATE */


static void handle_dump_state(int sig, void *eloop_ctx, void *signal_ctx)
{
#ifdef HOSTAPD_DUMP_STATE
	struct hapd_interfaces *hapds = (struct hapd_interfaces *) eloop_ctx;
	int i;

	for (i = 0; i < hapds->count; i++) {
		hostapd *hapd = hapds->hapd[i];
		hostapd_dump_state(hapd);
	}
#endif /* HOSTAPD_DUMP_STATE */
}


static void hostapd_set_broadcast_wep(hostapd *hapd)
{
	if (hapd->conf->wep_key_len < 1)
		return;

	hapd->default_wep_key = malloc(hapd->conf->wep_key_len);
	if (hapd->default_wep_key == NULL ||
	    hostapd_get_rand(hapd->default_wep_key,
			     hapd->conf->wep_key_len)) {
		printf("Could not generate random WEP key.\n");
		free(hapd->default_wep_key);
		exit(1);
	}
}


/* The rekeying function: generate a new broadcast WEP key, rotate
 * the key index, and direct Key Transmit State Machines of all of the
 * authenticators to send a new key to the authenticated stations.
 */
static void hostapd_rotate_wep(void *eloop_ctx, void *timeout_ctx)
{
	struct sta_info *s;
	hostapd *hapd = eloop_ctx;

	if (hapd->default_wep_key)
		free(hapd->default_wep_key);

	if (hapd->default_wep_key_idx >= 3)
		hapd->default_wep_key_idx =
			hapd->conf->wep_key_len > 0 ? 1 : 0;
	else
		hapd->default_wep_key_idx++;

	hostapd_set_broadcast_wep(hapd);

	for (s = hapd->sta_list; s != NULL; s = s->next)
		if (s->eapol_sm)
			s->eapol_sm->keyAvailable = TRUE;

	if (HOSTAPD_DEBUG_COND(HOSTAPD_DEBUG_MINIMAL)) {
		hostapd_hexdump("New WEP key generated",
				hapd->default_wep_key,
				hapd->conf->wep_key_len);
	}

	/* TODO: Could setup key for RX here, but change default TX keyid only
	 * after new broadcast key has been sent to all stations. */
	if (hostapd_set_encryption(hapd, "WEP", NULL,
    				   hapd->default_wep_key_idx,
			 	   hapd->default_wep_key,
			 	   hapd->conf->wep_key_len)) {
		printf("Could not set WEP encryption.\n");
	}

	if (hapd->conf->wep_rekeying_period > 0)
		eloop_register_timeout(hapd->conf->wep_rekeying_period, 0,
				       hostapd_rotate_wep, hapd, NULL);
}


static void hostapd_cleanup(hostapd *hapd, int reinit)
{
	FILE *f;
	u32 value = 0;

	if (hapd->sock >= 0)
		close(hapd->sock);
	if (hapd->ioctl_sock >= 0)
		close(hapd->ioctl_sock);
	free(hapd->default_wep_key);
	if (hapd->conf->ieee802_11f)
		iapp_deinit(hapd);
	accounting_deinit(hapd);
	ieee802_1x_deinit(hapd);
	hostapd_acl_deinit(hapd);
	radius_client_deinit(hapd);

	hostapd_config_free(hapd->conf);
	hapd->conf = NULL;

	/* Shut down the wireless driver */
	f = fopen("/proc/net/mvwlan/apdio", "w");
	if (f) {
		fwrite(&value, 1, sizeof(u32), f);
		fclose(f);
	}

	if (! reinit)
		free(hapd->config_fname);
}


int hostapd_flush_old_stations(hostapd *hapd)
{
	int ret = 0;

	printf("Flushing old station entries\n");
	if (hostapd_flush(hapd)) {
		printf("Could not connect to kernel driver.\n");
		ret = -1;
	}
	printf("Deauthenticate all stations\n");
	hostapd_deauth_all_stas(hapd);

	return ret;
}


static int hostapd_setup_encryption(hostapd *hapd)
{
	if (HOSTAPD_DEBUG_COND(HOSTAPD_DEBUG_MINIMAL))
		hostapd_hexdump("Default WEP key", hapd->default_wep_key,
				hapd->conf->wep_key_len);

	hostapd_set_encryption(hapd, "none", NULL, 0, NULL, 0);

	if (hostapd_set_encryption(hapd, "WEP", NULL,
				   hapd->default_wep_key_idx,
				   hapd->default_wep_key,
				   hapd->conf->wep_key_len)) {
		printf("Could not set WEP encryption.\n");
		return -1;
	}

	/* Setup rekeying timer. */
	if (hapd->conf->wep_rekeying_period > 0 &&
	    (hapd->default_wep_key || hapd->conf->wep_key_len > 0)
	    && eloop_register_timeout(hapd->conf->wep_rekeying_period, 0,
				      hostapd_rotate_wep, hapd, NULL)) {
		printf("Could not set rekeying timer.\n");
		return -1;
	}

	return 0;
}


static int hostapd_setup_interface(hostapd *hapd)
{

	hapd->ioctl_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hapd->ioctl_sock < 0) {
		perror("socket[PF_INET,SOCK_DGRAM]");
		return -1;
	}

	if (hostap_ioctl_mvWLANparam(hapd, MVWLAN_PARAM_HOSTAPD, 1)) {
		printf("Could not enable hostapd mode for interface %s\n",
		       hapd->conf->iface);
		return -1;
	}

	if (hostapd_init_sockets(hapd))
		return -1;

	printf("Using interface %sap with hwaddr " MACSTR " and ssid '%s'\n",
	       hapd->conf->iface, MAC2STR(hapd->own_addr), hapd->conf->ssid);

	/* Set SSID for the kernel driver (to be used in beacon and probe
	 * response frames) */
/*	if (hostap_ioctl_setiwessid(hapd, hapd->conf->ssid,
				    hapd->conf->ssid_len)) {
		printf("Could not set SSID for kernel driver\n");
		return -1;
	} */

#ifdef TURBO_SETUP
	AUTO_LINK_LowerPower(hapd);
#endif

	if (radius_client_init(hapd)) {
		printf("RADIUS client initialization failed.\n");
		return -1;
	}

	if (hostapd_acl_init(hapd)) {
		printf("ACL initialization failed.\n");
		return -1;
	}

	if (ieee802_1x_init(hapd)) {
		printf("IEEE 802.1X initialization failed.\n");
		return -1;
	}

	if (key_mgmt_init(hapd)) {
		printf("Key Management initialization failed.\n");
		return -1;
	}

	if (accounting_init(hapd)) {
		printf("Accounting initialization failed.\n");
		return -1;
	}

	if (hapd->conf->ieee802_11f && iapp_init(hapd)) {
		printf("IEEE 802.11f (IAPP) initialization failed.\n");
		return -1;
	}

	if (hostapd_flush_old_stations(hapd))
		return -1;

	return 0;
}


static void usage(void)
{
	fprintf(stderr,
		"Host AP user space daemon for management functionality of "
		"Host AP kernel driver\n"
		"Copyright (c) 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>\n"
		"\n"
		"usage: hostapd [-hdB] <configuration file>\n"
		"\n"
		"options:\n"
		"   -h   show this usage\n"
		"   -d   show more debug messages (-dd for even more)\n"
		"   -B   run daemon in the background\n");

	exit(1);
}


static hostapd * hostapd_init(const char *config_file, hostapd *apd,
			      int reinit)
{
	u8 *param_buf;
	hostapd *hapd;
	u32 *hostapd;
	struct mvwlan_mfg_param *mfg;
	struct mvwlan_cfg_param *cfg;
	FILE *f;
	size_t rc;

	if (! reinit) {
		hapd = malloc(sizeof(*hapd));
		if (hapd == NULL) {
			printf("Could not allocate memory for hostapd data\n");
			exit(1);
		}
		memset(hapd, 0, sizeof(*hapd));
	} else {
		hapd = apd;
	}
	hapd->sock = hapd->ioctl_sock = -1;

	if (! reinit) {
		hapd->config_fname = strdup(config_file);
		if (hapd->config_fname == NULL) {
			printf("Could not allocate memory for config_fname\n");
			exit(1);
		}

		hapd->conf = hostapd_config_read(hapd->config_fname);
		if (hapd->conf == NULL) {
			free(hapd->config_fname);
			exit(1);
		}
	}
	if (hapd->conf->wep_key_len > 0) {
		/* use user specified key as the default broadcast key */
		hapd->default_wep_key_idx = hapd->conf->default_wep_key;
	}

	if (hapd->conf->assoc_ap)
		hapd->assoc_ap_state = WAIT_BEACON;

	f = fopen(hapd->conf->manuf_file, "rb");
	if (f == NULL) {
		printf("Could not open manufacture file '%s' for reading.\n",
		       hapd->conf->manuf_file);
		free(hapd->config_fname);
		exit(1);
	}

	param_buf = (u8 *) malloc(sizeof(u32) +
				  sizeof(struct mvwlan_mfg_param) +
				  sizeof(struct mvwlan_cfg_param));
	if (param_buf == NULL) {
		printf("Could not allocate memory for configuration data\n");
		fclose(f);
		free(hapd->config_fname);
		exit(1);
	}
	memset(param_buf, 0, sizeof(u32) + sizeof(struct mvwlan_mfg_param) +
	       sizeof(struct mvwlan_cfg_param));

	hostapd = (u32 *) param_buf;
	mfg = (struct mvwlan_mfg_param *)(param_buf + sizeof(u32));
	cfg = (struct mvwlan_cfg_param *)(param_buf + sizeof(u32) +
					  sizeof(struct mvwlan_mfg_param));
	/* Initialize the first parameter */
	*hostapd = 1;

	/* Initialize the second parameter */
	rc = fread(mfg, 1, sizeof(struct mvwlan_mfg_param), f);
	fclose(f);

#ifdef TURBO_SETUP
	if (hapd->conf->auto_link != 0) {

		if (hapd->conf->auto_link == 1) {
			hapd->conf->auth_algs = 3;
			hapd->conf->privacy_invoked = 0;
			hapd->conf->exclude_unencrypted = 0;
			hapd->conf->wpa_mode = 0;
#ifdef AP_WPA2
			hapd->conf->wpa2_mode = 0;
#endif
		}
	}

	AUTO_LINK_UpdateSSIDRoot(hapd->conf->ssid);
	AUTO_LINK_PatchSSID(hapd, hapd->conf->ssid);
#endif

	/* Initialize the third parameter */
	memcpy(cfg->ssid, hapd->conf->ssid, SZ_SSID);
	cfg->oper_rate_set = hapd->conf->oper_rate;
	cfg->basic_rate_set = hapd->conf->basic_rate;
	cfg->dtim_period = hapd->conf->dtim_period;
	cfg->beacon_interval = hapd->conf->beacon_interval;
	cfg->rts_threshold = hapd->conf->rts_threshold;
	cfg->fragment_threshold = hapd->conf->fragment_threshold;
	cfg->short_retry_limit = hapd->conf->short_retry_limit;
	cfg->long_retry_limit = hapd->conf->long_retry_limit;
	cfg->privacy_invoked = hapd->conf->privacy_invoked;
	cfg->exclude_unencrypted = hapd->conf->exclude_unencrypted;
	cfg->auth_mode = hapd->conf->auth_algs;
	cfg->hide_ssid = hapd->conf->hide_ssid;
	cfg->wep_key_size = hapd->conf->wep_key_len;
	cfg->wep_default_key_index = hapd->conf->default_wep_key;
	memcpy(cfg->wep_default_key1, hapd->conf->default_wep_key1,
	       cfg->wep_key_size);
	memcpy(cfg->wep_default_key2, hapd->conf->default_wep_key2,
	       cfg->wep_key_size);
	memcpy(cfg->wep_default_key3, hapd->conf->default_wep_key3,
	       cfg->wep_key_size);
	memcpy(cfg->wep_default_key4, hapd->conf->default_wep_key4,
	       cfg->wep_key_size);

	cfg->channel = hapd->conf->channel;
	cfg->current_tx_power_level = hapd->conf->current_tx_power_level;
#if 0
	cfg->tx_power_level1 = hapd->conf->;
	cfg->tx_power_level2 = hapd->conf->;
	cfg->tx_power_level3 = hapd->conf->;
	cfg->tx_power_level4 = hapd->conf->;
	cfg->tx_power_level5 = hapd->conf->;
	cfg->tx_power_level6 = hapd->conf->;
	cfg->tx_power_level7 = hapd->conf->;
	cfg->tx_power_level8 = hapd->conf->;
	cfg->rx_antenna = hapd->conf->;
	cfg->tx_antenna = hapd->conf->;
#endif
	cfg->short_preamble = hapd->conf->short_preamble;
	cfg->fixed_tx_data_rate = hapd->conf->fixed_tx_data_rate;
	cfg->fixed_tx_b_rate_idx = hapd->conf->fixed_tx_b_rate;
	cfg->fixed_tx_g_rate_idx = hapd->conf->fixed_tx_g_rate;

	memcpy(cfg->mac_addr, mfg->mfg_mac_addr, SZ_PHY_ADDR);
	memcpy(hapd->own_addr, mfg->mfg_mac_addr, SZ_PHY_ADDR);
	memcpy(cfg->dev_name, hapd->conf->iface, SZ_DEVICE_NAME);
	cfg->wireless_enable = hapd->conf->wireless_enable;
	cfg->auto_link = hapd->conf->auto_link;
	cfg->ssid_patch = hapd->conf->ssid_patch;
	cfg->speed_booster = hapd->conf->speed_booster;
	cfg->ap_mode = hapd->conf->ap_mode;
	cfg->g_protect = hapd->conf->g_protect;
	cfg->iw_mode = hapd->conf->iw_mode;
	cfg->wpa_mode = hapd->conf->wpa_mode;
	cfg->wpa_encry = hapd->conf->wpa_encry;
	strcpy(cfg->wpa_passphrase, hapd->conf->wpa_passphrase);
#ifndef AP_WPA2
	cfg->wpa_group_rekey_time = hapd->conf->wpa_group_rekey_time;
#else
	cfg->group_rekey_time = hapd->conf->group_rekey_time;
	cfg->wpa2_mode = hapd->conf->wpa2_mode;
	cfg->wpa2_encry = hapd->conf->wpa2_encry;
	strcpy(cfg->wpa2_passphrase, hapd->conf->wpa2_passphrase);
#endif
	cfg->wlan_tx_gpio = hapd->conf->wlan_tx_gpio;
	cfg->wlan_rx_gpio = hapd->conf->wlan_rx_gpio;
	cfg->antenna = hapd->conf->antenna;
	cfg->watchdog_timer = hapd->conf->watchdog_timer;
    cfg->sta_bridge = hapd->conf->sta_bridge; 

	f = fopen("/proc/net/mvwlan/apdio", "w");
	if (f == NULL) {
		printf("Could not write into IO file '%s'.\n",
		       "/proc/net/mvwlan/apdio");
		if (! reinit) {
			free(hapd->config_fname);
			exit(1);
		}
	}
	fwrite(param_buf, 1, (sizeof(u32) + sizeof(struct mvwlan_mfg_param) +
	       sizeof(struct mvwlan_cfg_param)), f);
	fclose(f);

	free(param_buf);

	return hapd;
}


static void hostapd_reinit(hostapd *hapd)
{
	struct hostapd_config *newconf;
	int i;

	newconf = hostapd_config_read(hapd->config_fname);
	if (newconf == NULL) {
		printf("Failed to read new configuration file - "
		       "continuing with old.\n");
		return;
	}

	for (i = 0; i < interfaces.count; i++) {
		if (!interfaces.hapd[i])
			continue;

		hostapd_free_stas(interfaces.hapd[i]);
		hostapd_flush_old_stations(interfaces.hapd[i]);

		(void) hostapd_set_iface_flags(interfaces.hapd[i], 0);
		(void) hostap_ioctl_mvWLANparam(interfaces.hapd[i],
						MVWLAN_PARAM_HOSTAPD, 0);

		hostapd_cleanup(interfaces.hapd[i], 1);
	}

	eloop_deinit();

	hapd->conf = newconf;

	for (i = 0; i < interfaces.count; i++) {
		hostapd_init(hapd->config_fname, interfaces.hapd[i], 1);
		hostapd_set_broadcast_wep(interfaces.hapd[i]);
		(void) hostapd_setup_interface(interfaces.hapd[i]);
	}
}


int main (int argc, char *argv[])
{
	int ret = 1, i;
	int c, debug = 0, daemonize = 0;

	for (;;) {
		c = getopt(argc, argv, "Bdh");
		if (c < 0)
			break;
		switch (c) {
		case 'h':
			usage();
			break;
		case 'd':
			debug++;
			break;
		case 'B':
			daemonize++;
			break;

		default:
			usage();
			break;
		}
	}

	if (optind == argc)
		usage();

	interfaces.count = optind;

	interfaces.hapd = malloc(interfaces.count * sizeof(hostapd *));
	if (interfaces.hapd == NULL) {
		printf("malloc failed\n");
		exit(1);
	}

	eloop_init(&interfaces);
	eloop_register_signal(SIGHUP, handle_reload, NULL);
	eloop_register_signal(SIGINT, handle_term, NULL);
	eloop_register_signal(SIGTERM, handle_term, NULL);
	eloop_register_signal(SIGUSR1, handle_dump_state, NULL);

	for (i = 0; i < interfaces.count; i++) {
		printf("Configuration file: %s\n", argv[optind + i]);
		interfaces.hapd[i] = hostapd_init(argv[optind + i], NULL, 0);
		if (!interfaces.hapd[i])
			goto out;
		hostapd_set_broadcast_wep(interfaces.hapd[i]);
		if (hostapd_setup_interface(interfaces.hapd[i]))
			goto out;
	}

#if ! defined(__uClinux__)
	if (daemonize && daemon(0, 0)) {
		perror("daemon");
		goto out;
	}
#endif

	openlog("hostapd", 0, LOG_DAEMON);

	eloop_run();

	for (i = 0; i < interfaces.count; i++) {
		hostapd_free_stas(interfaces.hapd[i]);
		hostapd_flush_old_stations(interfaces.hapd[i]);
	}

	ret = 0;

 out:
	for (i = 0; i < interfaces.count; i++) {
		if (!interfaces.hapd[i])
			continue;
		(void) hostapd_set_iface_flags(interfaces.hapd[i], 0);
		(void) hostap_ioctl_mvWLANparam(interfaces.hapd[i],
						MVWLAN_PARAM_HOSTAPD, 0);

		hostapd_cleanup(interfaces.hapd[i], 0);
		free(interfaces.hapd[i]);
	}
	free(interfaces.hapd);

	eloop_destroy();

	closelog();

	return ret;
}
