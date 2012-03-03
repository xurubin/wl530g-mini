#ifndef CONFIG_H
#define CONFIG_H

typedef u8 macaddr[ETH_ALEN];

struct hostapd_radius_server {
	struct in_addr addr;
	int port;
	u8 *shared_secret;
	size_t shared_secret_len;
};

struct hostapd_config {
	char iface[IFNAMSIZ + 1];
	char *manuf_file; /* manufacturer specific file name */

	enum {
		HOSTAPD_LEVEL_DEBUG_VERBOSE = 0,
		HOSTAPD_LEVEL_DEBUG = 1,
		HOSTAPD_LEVEL_INFO = 2,
		HOSTAPD_LEVEL_NOTICE = 3,
		HOSTAPD_LEVEL_WARNING = 4
	} logger_syslog_level, logger_stdout_level;

#define HOSTAPD_MODULE_IEEE80211 BIT(0)
#define HOSTAPD_MODULE_IEEE8021X BIT(1)
#define HOSTAPD_MODULE_RADIUS BIT(2)
	unsigned int logger_syslog; /* module bitfield */
	unsigned int logger_stdout; /* module bitfield */

	enum { HOSTAPD_DEBUG_NO = 0, HOSTAPD_DEBUG_MINIMAL = 1,
	       HOSTAPD_DEBUG_VERBOSE = 2,
	       HOSTAPD_DEBUG_MSGDUMPS = 3 } debug; /* debug verbosity level */
	char *dump_log_name; /* file name for state dump (SIGUSR1) */
	int daemonize; /* fork into background */

	int wireless_enable;
	int auto_link;
	int ssid_patch;
	int speed_booster;
	int ap_mode;
	int g_protect;
	int iw_mode;
#define HOSTAPD_RATE_LEN 32
	u32 oper_rate;
	u32 basic_rate;
	int fixed_tx_data_rate;
	int fixed_tx_b_rate;
	int fixed_tx_g_rate;
	int beacon_interval;
	int dtim_period;
	int rts_threshold;
	int fragment_threshold;
	int short_retry_limit;
	int long_retry_limit;
	int channel;
	int current_tx_power_level;
	int short_preamble;
	int wpa_mode;
#define HOSTAPD_WPA_ENCRY_TKIP           2
#define HOSTAPD_WPA_ENCRY_CCMP           4
	int wpa_encry;
	char *wpa_passphrase;
#ifndef AP_WPA2
	u32 wpa_group_rekey_time;
#else
	u32 group_rekey_time;
	int wpa2_mode;
#define HOSTAPD_WPA2_ENCRY_TKIP_OR_CCMP  2
#define HOSTAPD_WPA2_ENCRY_CCMP_ONLY     4
	int wpa2_encry;
	char *wpa2_passphrase;
#endif
	int wlan_tx_gpio;
	int wlan_rx_gpio;
	int antenna;
	int watchdog_timer;

	int ieee802_1x; /* use IEEE 802.1X */
	int minimal_eap; /* test version of EAP authentication that only
			  * requires EAP Response-Identity and authorizes any
			  * station */
	struct in_addr own_ip_addr;
	/* RADIUS Authentication and Accounting servers in priority order */
	struct hostapd_radius_server *auth_servers, *auth_server;
	int num_auth_servers;
	struct hostapd_radius_server *acct_servers, *acct_server;
	int num_acct_servers;

	int radius_retry_primary_interval;
	int radius_acct_interim_interval;
#define HOSTAPD_SSID_LEN 32
	char ssid[HOSTAPD_SSID_LEN + 1];
	size_t ssid_len;
	int hide_ssid;
	char *eap_req_id_text; /* optional displayable message sent with
				* EAP Request-Identity */
	int eapol_key_index_workaround;

	int default_wep_key;
	char *default_wep_key1;
	char *default_wep_key2;
	char *default_wep_key3;
	char *default_wep_key4;
	int wep_key_len;
	int wep_rekeying_period;
	int privacy_invoked;
	int exclude_unencrypted;

	int ieee802_11f; /* use IEEE 802.11f (IAPP) */
	char iapp_iface[IFNAMSIZ + 1]; /* interface used with IAPP broadcast
					* frames */

	u8 assoc_ap_addr[ETH_ALEN];
	int assoc_ap; /* whether assoc_ap_addr is set */

	enum {
		ACCEPT_UNLESS_DENIED = 0,
		DENY_UNLESS_ACCEPTED = 1,
		ACL_NOT_APPLY = 2,
		USE_EXTERNAL_RADIUS_AUTH = 3
	} macaddr_acl;
	macaddr *accept_mac;
	int num_accept_mac;
	macaddr *deny_mac;
	int num_deny_mac;

#define HOSTAPD_AUTH_OPEN BIT(0)
#define HOSTAPD_AUTH_SHARED_KEY BIT(1)
	int auth_algs; /* bitfield of allowed IEEE 802.11 authentication
			* algorithms */
    int op_mode;  /* operation mode, 0: ap, 1: p2p */   
    int sta_bridge;  /* 1: to bridge between wireless STAs, 0: not bridge between wireless STAs */            
};


struct hostapd_config * hostapd_config_read(const char *fname);
void hostapd_config_free(struct hostapd_config *conf);
int hostapd_maclist_found(macaddr *list, int num_entries, u8 *addr);


#endif /* CONFIG_H */
