/*
 * Host AP (software wireless LAN access point) user space daemon for
 * Host AP kernel driver / Configuration file
 * Copyright (c) 2003, Jouni Malinen <jkmaline@cc.hut.fi>
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
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hostapd.h"
#include "common.h"


static struct hostapd_config *hostapd_config_defaults(void)
{
	struct hostapd_config *conf;

	conf = malloc(sizeof(*conf));
	if (conf == NULL) {
		printf("Failed to allocate memory for configuration data.\n");
		return NULL;
	}
	memset(conf, 0, sizeof(*conf));

	conf->ssid_len = 4;
	memcpy(conf->ssid, "AP32", 4);
	conf->wep_rekeying_period = 300;

	conf->logger_syslog_level = HOSTAPD_LEVEL_INFO;
	conf->logger_stdout_level = HOSTAPD_LEVEL_INFO;
	conf->logger_syslog = (unsigned int) -1;
	conf->logger_stdout = (unsigned int) -1;

	conf->wireless_enable = 1;
	conf->auto_link = 1;
	conf->ssid_patch = 1;
	conf->speed_booster = 0;
	conf->ap_mode = 0;
	conf->g_protect = 1;
	conf->iw_mode = 3;
	conf->oper_rate = 0x0000000F;
	conf->basic_rate = 0x0000000F;
	conf->fixed_tx_data_rate = 0;
	conf->beacon_interval = 1000;
	conf->dtim_period = 1;
	conf->rts_threshold = 2347;
	conf->fragment_threshold = 2346;
	conf->short_retry_limit = 7;
	conf->long_retry_limit = 4;
	conf->channel = 1;
	conf->current_tx_power_level = 1;
	conf->short_preamble = 0;
	conf->hide_ssid = 0;

	conf->privacy_invoked = 0;
	conf->exclude_unencrypted = 0;
	conf->wep_key_len = 5;
	conf->default_wep_key = 1;

	conf->auth_algs = HOSTAPD_AUTH_OPEN | HOSTAPD_AUTH_SHARED_KEY;

	conf->wpa_mode = 0;
	conf->wpa_encry = HOSTAPD_WPA_ENCRY_TKIP;  /* TKIP as default */
#ifndef AP_WPA2
	conf->wpa_group_rekey_time = 86400;
#else
	conf->group_rekey_time = 86400;
	conf->wpa2_mode = 0;
	conf->wpa2_encry = HOSTAPD_WPA2_ENCRY_CCMP_ONLY;  /* CCMP Only as defsult */
#endif

	conf->wlan_tx_gpio = 0;
	conf->wlan_rx_gpio = 1;
	conf->antenna = 2;
	conf->watchdog_timer = 1;
    conf->op_mode = 0;
    conf->sta_bridge = 1;
	return conf;
}


static int mac_comp(const void *a, const void *b)
{
	return memcmp(a, b, sizeof(macaddr));
}


static int hostapd_config_read_maclist(const char *fname, macaddr **acl,
				       int *num)
{
	FILE *f;
	char buf[128], *pos;
	int line = 0;
	u8 addr[ETH_ALEN];
	macaddr *newacl;

	if (!fname)
		return 0;

	f = fopen(fname, "r");
	if (!f) {
		printf("MAC list file '%s' not found.\n", fname);
		return -1;
	}

	while (fgets(buf, sizeof(buf), f)) {
		line++;

		if (buf[0] == '#')
			continue;
		pos = buf;
		while (*pos != '\0') {
			if ((*pos == '\n') || (*pos == '\r')) {
				*pos = '\0';
				break;
			}
			pos++;
		}
		if (buf[0] == '\0')
			continue;

		if (hwaddr_aton(buf, addr)) {
			printf("Invalid MAC address '%s' at line %d in '%s'\n",
			       buf, line, fname);
			fclose(f);
			return -1;
		}

		newacl = (macaddr *) realloc(*acl, (*num + 1) * ETH_ALEN);
		if (newacl == NULL) {
			printf("MAC list reallocation failed\n");
			fclose(f);
			return -1;
		}

		*acl = newacl;
		memcpy((*acl)[*num], addr, ETH_ALEN);
		(*num)++;
	}

	fclose(f);

	qsort(*acl, *num, sizeof(macaddr), mac_comp);

	return 0;
}


static int
hostapd_config_read_radius_addr(struct hostapd_radius_server **server,
				int *num_server, const char *val, int def_port,
				struct hostapd_radius_server **curr_serv)
{
	struct hostapd_radius_server *nserv;
	int ret;

	nserv = realloc(*server, (*num_server + 1) * sizeof(*nserv));
	if (nserv == NULL)
		return -1;

	*server = nserv;
	nserv = &nserv[*num_server];
	(*num_server)++;
	(*curr_serv) = nserv;

	memset(nserv, 0, sizeof(*nserv));
	nserv->port = def_port;
	ret = !inet_aton(val, &nserv->addr);

	return ret;
}


static int hostapd_config_check(struct hostapd_config *conf)
{
	if (conf->ieee802_1x && !conf->minimal_eap && !conf->auth_servers) {
		printf("Invalid IEEE 802.1X configuration.\n");
		return -1;
	}

	if (conf->wpa_mode && !conf->wpa_passphrase) {
		printf("Invalid WPA configuration.\n");
		return -1;
	}

#ifdef AP_WPA2
	if (conf->wpa2_mode && !conf->wpa2_passphrase) {
		printf("Invalid WPA2 configuration.\n");
		return -1;
	}
#endif

	return 0;
}


struct rate2index_table {
	u32 index;
	int rate;
};


struct rate2index_table Rate2IndexTable[] =
{
	{ 0x00000001,  1000 },
	{ 0x00000002,  2000 },
	{ 0x00000004,  5500 },
	{ 0x00000008, 11000 },
	{ 0x00000010, 22000 },
	{ 0x00000020,  6000 },
	{ 0x00000040,  9000 },
	{ 0x00000080, 12000 },
	{ 0x00000100, 18000 },
	{ 0x00000200, 24000 },
	{ 0x00000400, 36000 },
	{ 0x00000800, 48000 },
	{ 0x00001000, 54000 },
	{ 0x00000000,     0 }
};


static u32 rate2index(int rate)
{
	int i;
	u32 index = 0;

	for (i = 0; i < HOSTAPD_RATE_LEN; i++) {
		if (! Rate2IndexTable[i].index)
			break;
		if (Rate2IndexTable[i].rate == rate) {
			index = Rate2IndexTable[i].index;
			break;
		}
	}

	return index;
}


static int rate2idx(int rate)
{
	int i;

	for (i = 0; i < HOSTAPD_RATE_LEN; i++) {

		if (! Rate2IndexTable[i].index)
			break;

		if (Rate2IndexTable[i].rate == rate) {
			return i;
		}
	}

	return -1;
}


struct hostapd_config * hostapd_config_read(const char *fname)
{
	struct hostapd_config *conf;
	FILE *f;
	char buf[256], *pos, *pos1;
	int idx, rate;
	enum { WEP_KEY_ASCII, WEP_KEY_HEX, WEP_KEY_UNKNOWN_TYPE };
	int line = 0, errors = 0, wep_key_type = 0;
	char *accept_mac_file = NULL, *deny_mac_file = NULL;

	f = fopen(fname, "r");
	if (f == NULL) {
		printf("Could not open configuration file '%s' for reading.\n",
		       fname);
		return NULL;
	}

	conf = hostapd_config_defaults();
	if (conf == NULL) {
		fclose(f);
		return NULL;
	}

	while (fgets(buf, sizeof(buf), f)) {
		line++;

		if (buf[0] == '#')
			continue;
		pos = buf;
		while (*pos != '\0') {
			if ((*pos == '\n') || (*pos == '\r')){
				*pos = '\0';
				break;
			}
			pos++;
		}
		if (buf[0] == '\0')
			continue;

		pos = strchr(buf, '=');
		if (pos == NULL) {
			printf("Line %d: invalid line '%s'\n", line, buf);
			errors++;
			continue;
		}
		*pos = '\0';
		pos++;

		if (strcmp(buf, "interface") == 0) {
			snprintf(conf->iface, sizeof(conf->iface), "%s", pos);
		} else if (strcmp(buf, "debug") == 0) {
			conf->debug = atoi(pos);
		} else if (strcmp(buf, "logger_syslog_level") == 0) {
			conf->logger_syslog_level = atoi(pos);
		} else if (strcmp(buf, "logger_stdout_level") == 0) {
			conf->logger_stdout_level = atoi(pos);
		} else if (strcmp(buf, "logger_syslog") == 0) {
			conf->logger_syslog = atoi(pos);
		} else if (strcmp(buf, "logger_stdout") == 0) {
			conf->logger_stdout = atoi(pos);
		} else if (strcmp(buf, "manuf_file") == 0) {
			conf->manuf_file = strdup(pos);
		} else if (strcmp(buf, "dump_file") == 0) {
			conf->dump_log_name = strdup(pos);
		} else if (strcmp(buf, "daemonize") == 0) {
			conf->daemonize = atoi(pos);
		} else if (strcmp(buf, "wireless_enable") == 0) {
			conf->wireless_enable = atoi(pos);
		} else if (strcmp(buf, "auto_link") == 0) {
			conf->auto_link = atoi(pos);
		} else if (strcmp(buf, "ssid_patch") == 0) {
			conf->ssid_patch = atoi(pos);
		} else if (strcmp(buf, "speed_booster") == 0) {
			conf->speed_booster = atoi(pos);
		} else if (strcmp(buf, "ap_mode") == 0) {
			conf->ap_mode = atoi(pos);
		} else if (strcmp(buf, "g_protect") == 0) {
			conf->g_protect = atoi(pos);
		} else if (strcmp(buf, "iw_mode") == 0) {
			conf->iw_mode = atoi(pos);
		} else if (strcmp(buf, "oper_rate") == 0) {
			idx = 0;
			pos1 = pos;
			while (*pos != '\0') {
				if (idx >= HOSTAPD_RATE_LEN)
					break;
				pos = strchr(pos1, ',');
				if (pos == NULL) {
					conf->oper_rate |=
						rate2index(atoi(pos1));
					break;
				}
				*pos++ = '\0';
				conf->oper_rate |= rate2index(atoi(pos1));
				pos1 = pos;
			}
		} else if (strcmp(buf, "basic_rate") == 0) {
			idx = 0;
			pos1 = pos;
			while (*pos != '\0') {
				if (idx >= HOSTAPD_RATE_LEN)
					break;
				pos = strchr(pos1, ',');
				if (pos == NULL) {
					conf->basic_rate |=
						rate2index(atoi(pos1));
					break;
				}
				*pos++ = '\0';
				conf->basic_rate |= rate2index(atoi(pos1));
				pos1 = pos;
			}
		} else if (strcmp(buf, "fixed_tx_data_rate") == 0) {
			conf->fixed_tx_data_rate = atoi(pos);
		} else if (strcmp(buf, "fixed_tx_b_rate") == 0) {
			conf->fixed_tx_b_rate = rate2idx(atoi(pos));
			if (conf->fixed_tx_b_rate < 0) {
				printf("Line %d: invalid rate '%d'\n", line,
					conf->fixed_tx_b_rate);
				errors++;
			}
		} else if (strcmp(buf, "fixed_tx_g_rate") == 0) {
			conf->fixed_tx_g_rate = rate2idx(atoi(pos));
			if (conf->fixed_tx_g_rate < 0) {
				printf("Line %d: invalid rate '%d'\n", line,
					conf->fixed_tx_g_rate);
				errors++;
			}
		} else if (strcmp(buf, "beacon_interval") == 0) {
			conf->beacon_interval = atoi(pos);
		} else if (strcmp(buf, "dtim_period") == 0) {
			conf->dtim_period = atoi(pos);
		} else if (strcmp(buf, "rts_threshold") == 0) {
			conf->rts_threshold = atoi(pos);
		} else if (strcmp(buf, "fragment_threshold") == 0) {
			conf->fragment_threshold = atoi(pos);
		} else if (strcmp(buf, "short_retry_limit") == 0) {
			conf->short_retry_limit = atoi(pos);
		} else if (strcmp(buf, "long_retry_limit") == 0) {
			conf->long_retry_limit = atoi(pos);
		} else if (strcmp(buf, "channel") == 0) {
			conf->channel = atoi(pos);
		} else if (strcmp(buf, "current_tx_power_level") == 0) {
			conf->current_tx_power_level = atoi(pos);
		} else if (strcmp(buf, "short_preamble") == 0) {
			conf->short_preamble = atoi(pos);
		} else if (strcmp(buf, "hide_ssid") == 0) {
			conf->hide_ssid = atoi(pos);
		} else if (strcmp(buf, "ssid") == 0) {
			conf->ssid_len = strlen(pos);
			if (conf->ssid_len > HOSTAPD_SSID_LEN ||
			    conf->ssid_len < 1) {
				printf("Line %d: invalid SSID '%s'\n", line,
				       pos);
				errors++;
			}
			memcpy(conf->ssid, pos, conf->ssid_len);
			conf->ssid[conf->ssid_len] = '\0';
		} else if (strcmp(buf, "macaddr_acl") == 0) {
			conf->macaddr_acl = atoi(pos);
			if (conf->macaddr_acl != ACCEPT_UNLESS_DENIED &&
			    conf->macaddr_acl != DENY_UNLESS_ACCEPTED &&
			    conf->macaddr_acl != ACL_NOT_APPLY &&
			    conf->macaddr_acl != USE_EXTERNAL_RADIUS_AUTH) {
				printf("Line %d: unknown macaddr_acl %d\n",
				       line, conf->macaddr_acl);
			}
		} else if (strcmp(buf, "accept_mac_file") == 0) {
			accept_mac_file = strdup(pos);
			if (!accept_mac_file) {
				printf("Line %d: allocation failed\n", line);
				errors++;
			}
		} else if (strcmp(buf, "deny_mac_file") == 0) {
			deny_mac_file = strdup(pos);
			if (!deny_mac_file) {
				printf("Line %d: allocation failed\n", line);
				errors++;
			}
		} else if (strcmp(buf, "assoc_ap_addr") == 0) {
			if (hwaddr_aton(pos, conf->assoc_ap_addr)) {
				printf("Line %d: invalid MAC address '%s'\n",
				       line, pos);
				errors++;
			}
			conf->assoc_ap = 1;
		} else if (strcmp(buf, "wpa_mode") == 0) {
			conf->wpa_mode = atoi(pos);
		} else if (strcmp(buf, "wpa_encry") == 0) {
			conf->wpa_encry = atoi(pos);
			if ((conf->wpa_encry != HOSTAPD_WPA_ENCRY_TKIP) &&
			    (conf->wpa_encry != HOSTAPD_WPA_ENCRY_CCMP)) {
			    	printf("Line %d: invalid WPA encryption method %d\n",
				       line, conf->wpa_encry);
			}
		} else if (strcmp(buf, "wpa_passphrase") == 0) {
			int len = strlen(pos);
			if (len < 8 || len > 63) {
				printf("Line %d: invalid WPA passphrase length"
				       "%d (expected 8..63)\n", line, len);
				errors++;
			} else
				conf->wpa_passphrase = strdup(pos);
#ifndef AP_WPA2
		} else if (strcmp(buf, "wpa_group_rekey_time") == 0) {
			conf->wpa_group_rekey_time = atoi(pos);
#else
		} else if (strcmp(buf, "group_rekey_time") == 0) {
			conf->group_rekey_time = atoi(pos);
		} else if (strcmp(buf, "wpa2_mode") == 0) {
			conf->wpa2_mode = atoi(pos);
		} else if (strcmp(buf, "wpa2_encry") == 0) {
			conf->wpa2_encry = atoi(pos);
#if 0
			if ((conf->wpa2_encry != HOSTAPD_WPA2_ENCRY_TKIP_OR_CCMP) &&
			    (conf->wpa2_encry != HOSTAPD_WPA2_ENCRY_CCMP_ONLY)) {
#else
			if (conf->wpa2_encry != HOSTAPD_WPA2_ENCRY_CCMP_ONLY) {
#endif
			    	printf("Line %d: invalid WPA2 encryption method %d\n",
				       line, conf->wpa2_encry);
			}
		} else if (strcmp(buf, "wpa2_passphrase") == 0) {
			int len = strlen(pos);
			if (len < 8 || len > 63) {
				printf("Line %d: invalid WPA2 passphrase length"
				       "%d (expected 8..63)\n", line, len);
				errors++;
			} else
				conf->wpa2_passphrase = strdup(pos);
#endif
		} else if (strcmp(buf, "wlan_tx_gpio") == 0) {
			conf->wlan_tx_gpio = atoi(pos);
		} else if (strcmp(buf, "wlan_rx_gpio") == 0) {
			conf->wlan_rx_gpio = atoi(pos);
		} else if (strcmp(buf, "antenna") == 0) {
			conf->antenna = atoi(pos);
		} else if (strcmp(buf, "watchdog_timer") == 0) {
			conf->watchdog_timer = atoi(pos);
		} else if (strcmp(buf, "ieee8021x") == 0) {
			conf->ieee802_1x = atoi(pos);
		} else if (strcmp(buf, "minimal_eap") == 0) {
			conf->minimal_eap = atoi(pos);
		} else if (strcmp(buf, "eap_message") == 0) {
			conf->eap_req_id_text = strdup(pos);
		} else if (strcmp(buf, "wep_key_len") == 0) {
			conf->wep_key_len = atoi(pos);
			if (conf->wep_key_len != 5 &&
				conf->wep_key_len != 13) {
				printf("Line %d: invalid WEP key len %d "
				       "(= %d bits)\n", line,
				       conf->wep_key_len,
				       conf->wep_key_len * 8);
				errors++;
			}
		} else if (strcmp(buf, "wep_default_key") == 0) {
			conf->default_wep_key = atoi(pos);
			if (conf->default_wep_key < 1 ||
			    conf->default_wep_key > 4) {
				printf("Line %d: invalid default WEP key %d\n",
				       line, conf->default_wep_key);
				errors++;
			}
		} else if (strcmp(buf, "wep_key_type") == 0) {
			if (strcmp(pos, "ascii") == 0) {
				wep_key_type = WEP_KEY_ASCII;
			} else if (strcmp(pos, "hex") == 0) {
				wep_key_type = WEP_KEY_HEX;
			} else {
				wep_key_type = WEP_KEY_UNKNOWN_TYPE;
				printf("Line %d: invalid WEP key type %s\n",
				       line, pos);
				errors++;
			}
		} else if (strcmp(buf, "wep_default_key1") == 0) {
			conf->default_wep_key1 = strdup(pos);
		} else if (strcmp(buf, "wep_default_key2") == 0) {
			conf->default_wep_key2 = strdup(pos);
		} else if (strcmp(buf, "wep_default_key3") == 0) {
			conf->default_wep_key3 = strdup(pos);
		} else if (strcmp(buf, "wep_default_key4") == 0) {
			conf->default_wep_key4 = strdup(pos);
		} else if (strcmp(buf, "wep_rekey_period") == 0) {
			conf->wep_rekeying_period = atoi(pos);
			if (conf->wep_rekeying_period < 0) {
				printf("Line %d: invalid period %d\n",
				       line, conf->wep_rekeying_period);
				errors++;
			}
		} else if (strcmp(buf, "privacy_invoked") == 0) {
			conf->privacy_invoked = atoi(pos);
		} else if (strcmp(buf, "exclude_unencrypted") == 0) {
			conf->exclude_unencrypted = atoi(pos);
		} else if (strcmp(buf, "eapol_key_index_workaround") == 0) {
			conf->eapol_key_index_workaround = atoi(pos);
		} else if (strcmp(buf, "iapp_interface") == 0) {
			conf->ieee802_11f = 1;
			snprintf(conf->iapp_iface, sizeof(conf->iapp_iface),
				 "%s", pos);
		} else if (strcmp(buf, "own_ip_addr") == 0) {
			if (!inet_aton(pos, &conf->own_ip_addr)) {
				printf("Line %d: invalid IP address '%s'\n",
				       line, pos);
				errors++;
			}
		} else if (strcmp(buf, "auth_server_addr") == 0) {
			if (hostapd_config_read_radius_addr(
				    &conf->auth_servers,
				    &conf->num_auth_servers, pos, 1812,
				    &conf->auth_server)) {
				printf("Line %d: invalid IP address '%s'\n",
				       line, pos);
				errors++;
			}
		} else if (conf->auth_server &&
			   strcmp(buf, "auth_server_port") == 0) {
			conf->auth_server->port = atoi(pos);
		} else if (conf->auth_server &&
			   strcmp(buf, "auth_server_shared_secret") == 0) {
			int len = strlen(pos);
			if (len == 0) {
				/* RFC 2865, Ch. 3 */
				printf("Line %d: empty shared secret is not "
				       "allowed.\n", line);
				errors++;
			}
			conf->auth_server->shared_secret = strdup(pos);
			conf->auth_server->shared_secret_len = len;
		} else if (strcmp(buf, "acct_server_addr") == 0) {
			if (hostapd_config_read_radius_addr(
				    &conf->acct_servers,
				    &conf->num_acct_servers, pos, 1813,
				    &conf->acct_server)) {
				printf("Line %d: invalid IP address '%s'\n",
				       line, pos);
				errors++;
			}
		} else if (conf->acct_server &&
			   strcmp(buf, "acct_server_port") == 0) {
			conf->acct_server->port = atoi(pos);
		} else if (conf->acct_server &&
			   strcmp(buf, "acct_server_shared_secret") == 0) {
			int len = strlen(pos);
			if (len == 0) {
				/* RFC 2865, Ch. 3 */
				printf("Line %d: empty shared secret is not "
				       "allowed.\n", line);
				errors++;
			}
			conf->acct_server->shared_secret = strdup(pos);
			conf->acct_server->shared_secret_len = len;
		} else if (strcmp(buf, "radius_retry_primary_interval") == 0) {
			conf->radius_retry_primary_interval = atoi(pos);
		} else if (strcmp(buf, "radius_acct_interim_interval") == 0) {
			conf->radius_acct_interim_interval = atoi(pos);
		} else if (strcmp(buf, "auth_algs") == 0) {
			conf->auth_algs = atoi(pos);
        } else if (strcmp(buf, "op_mode") == 0) {
			conf->op_mode = atoi(pos);
         } else if (strcmp(buf, "sta_bridge") == 0) {
			conf->sta_bridge = atoi(pos);            
		} else {
			printf("Line %d: unknown configuration item '%s'\n",
			       line, buf);
			errors++;
		}
	}

	if ((conf->basic_rate & conf->oper_rate) != conf->basic_rate) {
			printf("Configured basic rate set mismatches configured"
			       " operational rate set\n");
			errors++;
	}

	if (((conf->fixed_tx_b_rate & conf->oper_rate) != conf->fixed_tx_b_rate) ||
		((conf->fixed_tx_g_rate & conf->oper_rate) != conf->fixed_tx_g_rate)) {
			printf("Configured fixed transmit rate mismatches any "
			       "configured operational rate set\n");
			errors++;
	}

	/* Hexdecimal WEP key conversion */
	if (wep_key_type == WEP_KEY_HEX) {
		for (idx = 0; idx < 4; idx++) {
			char *wep_key;

			switch (idx) {
			case 0:
				wep_key = conf->default_wep_key1;
				break;
			case 1:
				wep_key = conf->default_wep_key2;
				break;
			case 2:
				wep_key = conf->default_wep_key3;
				break;
			case 3:
				wep_key = conf->default_wep_key4;
				break;
			}
			if (wep_key && wep_key[0]) {
				if ((conf->wep_key_len == 5) ||
				    (conf->wep_key_len == 13)) {
					int hex_len = (2 * conf->wep_key_len);
					int i1 = 0, i2 = 0, num1, num2;

					if (strlen(wep_key) != hex_len) {
						printf("WEP key %d length "
						       "mismatches\n", idx + 1);
						errors++;
						break;
					}

					while (hex_len) {
						num1 = hex2num(wep_key[i1++]);
						num2 = hex2num(wep_key[i1++]);
						if (num1 < 0 || num2 < 0) {
							printf("WEP key %d "
							       "has invalid "
							       "characters\n",
							       idx + 1);
							errors++;
							break;
						}
						num1 <<= 4;
						buf[i2++] = num1 | num2;
						hex_len -= 2;
					}

					if (! hex_len) {
						memcpy(wep_key, buf,
						       conf->wep_key_len);
						wep_key[conf->wep_key_len] = 0;
					}
				}
			}
		}
	}

	fclose(f);

	if (hostapd_config_read_maclist(accept_mac_file, &conf->accept_mac,
					&conf->num_accept_mac))
		errors++;
	free(accept_mac_file);
	if (hostapd_config_read_maclist(deny_mac_file, &conf->deny_mac,
					&conf->num_deny_mac))
		errors++;
	free(deny_mac_file);

	conf->auth_server = conf->auth_servers;
	conf->acct_server = conf->acct_servers;

	if (hostapd_config_check(conf))
		errors++;

	if (errors) {
		printf("%d errors found in configuration file '%s'\n",
		       errors, fname);
#if 0
		hostapd_config_free(conf);
		conf = NULL;
#endif
	}

	return conf;
}


static void hostapd_config_free_radius(struct hostapd_radius_server *servers,
				       int num_servers)
{
	int i;

	for (i = 0; i < num_servers; i++) {
		free(servers[i].shared_secret);
	}
	free(servers);
}


void hostapd_config_free(struct hostapd_config *conf)
{
	if (conf == NULL)
		return;

	free(conf->manuf_file);
	free(conf->dump_log_name);
	free(conf->eap_req_id_text);
	free(conf->accept_mac);
	free(conf->deny_mac);
	free(conf->wpa_passphrase);
	hostapd_config_free_radius(conf->auth_servers, conf->num_auth_servers);
	hostapd_config_free_radius(conf->acct_servers, conf->num_acct_servers);
	free(conf);
}


/* Perform a binary search for given MAC address from a pre-sorted list.
 * Returns 1 if address is in the list or 0 if not. */
int hostapd_maclist_found(macaddr *list, int num_entries, u8 *addr)
{
	int start, end, middle, res;

	start = 0;
	end = num_entries - 1;

	while (start <= end) {
		middle = (start + end) / 2;
		res = memcmp(list[middle], addr, ETH_ALEN);
		if (res == 0)
			return 1;
		if (res < 0)
			start = middle + 1;
		else
			end = middle - 1;
	}

	return 0;
}
