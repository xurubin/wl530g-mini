/* ioctl() (mostly Linux Wireless Extensions) routines for Host AP driver */

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/if_arp.h>
#include <linux/random.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,44))
#include <linux/tqueue.h>
#else
#include <linux/workqueue.h>
#endif
#include <linux/kmod.h>
#include <linux/rtnetlink.h>
#include <asm/uaccess.h>

#ifdef in_atomic
/* Get kernel_locked() for in_atomic() */
#include <linux/smp_lock.h>
#endif

#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_hw.h"
#include "mvWLAN_crypt.h"

#ifdef BOOSTER_MODE
#include "mvWLAN_booster.h"
#endif


#ifdef WIRELESS_EXT

static const long freq_list[] = { 2412, 2417, 2422, 2427, 2432, 2437, 2442,
				  2447, 2452, 2457, 2462, 2467, 2472, 2484 };
#define FREQ_COUNT (sizeof(freq_list) / sizeof(freq_list[0]))


static local_info_t *dev_local[MAX_PARM_DEVICES];
static int dev_index;

static int mtu = 1500;
MODULE_PARM(mtu, "i");
MODULE_PARM_DESC(mtu, "Maximum transfer unit");

static char *essid[MAX_PARM_DEVICES] = { "AP32uClinux", DEF_STRS };
MODULE_PARM(essid, PARM_MIN_MAX "s");
MODULE_PARM_DESC(essid, "AP's Default ESSID");

static int iw_mode[MAX_PARM_DEVICES] = { IW_MODE_MASTER, DEF_INTS };
MODULE_PARM(iw_mode, PARM_MIN_MAX "i");
MODULE_PARM_DESC(iw_mode, "Initial operation mode");

static int beacon_int[MAX_PARM_DEVICES] = { 100, DEF_INTS };
MODULE_PARM(beacon_int, PARM_MIN_MAX "i");
MODULE_PARM_DESC(beacon_int, "Beacon interval (1 = 1024 usec)");

static int dtim_period[MAX_PARM_DEVICES] = { 1, DEF_INTS };
MODULE_PARM(dtim_period, PARM_MIN_MAX "i");
MODULE_PARM_DESC(dtim_period, "DTIM period");

static int rts_threshold[MAX_PARM_DEVICES] = { 2347, DEF_INTS };
MODULE_PARM(rts_threshold, PARM_MIN_MAX "i");
MODULE_PARM_DESC(rts_threshold, "RTS Threshold");

static int short_retry_limit[MAX_PARM_DEVICES] = { 7, DEF_INTS };
MODULE_PARM(short_retry_limit, PARM_MIN_MAX "i");
MODULE_PARM_DESC(short_retry_limit, "Short Retry Limit");

static int long_retry_limit[MAX_PARM_DEVICES] = { 4, DEF_INTS };
MODULE_PARM(long_retry_limit, PARM_MIN_MAX "i");
MODULE_PARM_DESC(long_retry_limit, "Long Retry Limit");

static int frag_threshold[MAX_PARM_DEVICES] = { 2346, DEF_INTS };
MODULE_PARM(frag_threshold, PARM_MIN_MAX "i");
MODULE_PARM_DESC(frag_threshold, "Fragmentation Threshold");

static int max_tx_msdu_lifetime[MAX_PARM_DEVICES] = { 512, DEF_INTS };
MODULE_PARM(max_tx_msdu_lifetime, PARM_MIN_MAX "i");
MODULE_PARM_DESC(max_tx_msdu_lifetime, "MaxTransmitMSDULifetime");

static int max_rx_lifetime[MAX_PARM_DEVICES] = { 512, DEF_INTS };
MODULE_PARM(max_rx_lifetime, PARM_MIN_MAX "i");
MODULE_PARM_DESC(max_rx_lifetime, "MaxReceiveLifetime");

static int current_regdomain[MAX_PARM_DEVICES] = { 0x10, DEF_INTS };
MODULE_PARM(current_regdomain, PARM_MIN_MAX "i");
MODULE_PARM_DESC(current_regdomain, "Current regulatory domain");

static int antenna_mode[MAX_PARM_DEVICES] = { 1, DEF_INTS };
MODULE_PARM(antenna_mode, PARM_MIN_MAX "i");
MODULE_PARM_DESC(antenna_mode, "Current rx/tx antennaa");

#if 0
static int tx_power_level[MAX_PARM_DEVICES] = { 1, DEF_INTS };
MODULE_PARM(tx_power_level, PARM_MIN_MAX "i");
MODULE_PARM_DESC(tx_power_level, "Tx Power Level");

static int delayed_enable /* = 0 */;
MODULE_PARM(delayed_enable, "i");
MODULE_PARM_DESC(delayed_enable, "Delay MAC port enable until netdevice open");
#endif

static int disable_on_close /* = 0 */;
MODULE_PARM(disable_on_close, "i");
MODULE_PARM_DESC(disable_on_close, "Disable MAC port on netdevice close");


enum {
	DOMAIN_CODE_FCC = 0x10,
	DOMAIN_CODE_IC  = 0x20,
	DOMAIN_CODE_ETSI = 0x30,
	DOMAIN_CODE_SPAIN = 0x31,
	DOMAIN_CODE_FRANCE = 0x32,
	DOMAIN_CODE_MKK = 0x40,
	DOMAIN_CODE_MPHPT = 0x41
};

struct regulatory_domain {
	u8 domainCode;
	u8 minChannel;
	u8 maxChannel;
	u8 defaultChannel;
};

static struct regulatory_domain RegDomains[] =
{
	{ DOMAIN_CODE_FCC,     1, 11,  6 }, /* FCC (USA)*/
	{ DOMAIN_CODE_IC,      1, 11,  6 }, /* IC (Canada) */
	{ DOMAIN_CODE_ETSI,    1, 13, 11 }, /* ESTI (most of Europe) */
	{ DOMAIN_CODE_SPAIN,  10, 11, 11 }, /* Spain */
	{ DOMAIN_CODE_FRANCE, 10, 13, 11 }, /* France */
	{ DOMAIN_CODE_MKK,    14, 14, 14 }, /* MKK (Japan) */
	{ DOMAIN_CODE_MPHPT,   1, 13, 11 }, /* MPHPT (Japan) */
	{ 0,                   0,  0,  0 }  /* Others */
};

/* Get the information regarding to a specific regulatory domain */
static int getRegDomainInfo(u8 reg_domain, u8 *minChannel,
			    u8 *maxChannel, u8 *defaultChannel)
{
	int i;

	for (i = 0; RegDomains[i].domainCode; i++) {
		if (reg_domain == RegDomains[i].domainCode) {
			*minChannel = RegDomains[i].minChannel;
			*maxChannel = RegDomains[i].maxChannel;
			*defaultChannel = RegDomains[i].defaultChannel;
			return 0;
		}
	}

        return -1;
}


static void mvWLAN_restart_mlme(struct net_device *dev)
{
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	local_info_t *local = (local_info_t *) dev->priv;

	mvWLAN_netif_stop_queues(local);

	mvWLAN_ap_control_kickall(local->ap);
	mvWLAN_deauth_all_stas(dev, local->ap, 0);

	mvWLAN_netif_wake_queues(local);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
}


static void mvWLAN_set_multicast_list_queue(void *data)
{
	/* XXX, enable promiscuous mode here */

#ifndef NEW_MODULE_CODE
	MOD_DEC_USE_COUNT;
#endif
}


static local_info_t *init_local_data(struct mvwlan_cfg_param *cfg_data,
				     struct mvwlan_mfg_param *mfg_data,
				     int dev_idx)
{
	local_info_t *local;
	int len, i;
	u8 minChannel, maxChannel, defaultChannel, rate = 0;
	u32 opMode, mask;
	MIB_802DOT11 *mib_p;
	struct mvwlan_mfg_param *mfg_data_p;


	local = kmalloc(sizeof(local_info_t), GFP_KERNEL);
	if (local == NULL)
		return NULL;

	memset(local, 0, sizeof(local_info_t));
	local->fun_sta_existed = (void *) mvWLAN_sta_existed;
	local->hw_module = THIS_MODULE;
	local->ap = kmalloc(sizeof(struct ap_data), GFP_KERNEL);
	if (local->ap == NULL)
		goto fail;

	memset(local->ap, 0, sizeof(struct ap_data));

	local->dev = kmalloc(sizeof(struct net_device) + MVWLAN_NETDEV_EXTRA,
			     GFP_KERNEL);
	if (local->dev == NULL)
		goto fail;

	memset(local->dev, 0, sizeof(struct net_device) + MVWLAN_NETDEV_EXTRA);

	len = strlen(cfg_data->dev_name);
	memcpy(local->dev->name, cfg_data->dev_name, len + 1);

	local->dev->priv = local;

	local->disable_on_close = disable_on_close;
	local->mtu = mtu;

	spin_lock_init(&local->wdslock);
	spin_lock_init(&local->txqlock);
	spin_lock_init(&local->cblock);
	spin_lock_init(&local->bcnlock);
	spin_lock_init(&local->cmdlock);
	spin_lock_init(&local->lock);

	switch (cfg_data->iw_mode) {
	case IW_MODE_MASTER:
	case IW_MODE_REPEAT:
		opMode = WL_OP_MODE_AP;
		break;
	case IW_MODE_INFRA:
	case IW_MODE_MONITOR:
	case IW_MODE_ADHOC:
		opMode = WL_OP_MODE_STA;
		break;
	default:
		printk(KERN_WARNING "Unknown iw_mode %d; using "
		       "IW_MODE_MASTER\n", cfg_data->iw_mode);
		cfg_data->iw_mode = GET_INT_PARM(iw_mode, dev_idx);
		opMode = WL_OP_MODE_AP;
		break;
	}
	local->iw_mode = cfg_data->iw_mode;

	/* need to allocate memory for manufacture data
	 */
	mfg_data_p = kmalloc(sizeof(struct mvwlan_mfg_param), GFP_KERNEL);

	memcpy(mfg_data_p, mfg_data, sizeof(struct mvwlan_mfg_param));

	/* Ideally we should not touch any HAL related API here.
	 * However, we need to create the system configuration first
	 * before we can initialized any the MIB related variable. */
	local->sysConfig = msi_wl_CreateSysCfg(opMode,
					       (MFG_CAL_DATA *) mfg_data_p);

	/* Load the configuration to the corresponding MIB variables */
	mib_p = local->sysConfig->Mib802dot11;

	/* For SMT Station Config Table */
	memcpy(&mib_p->StationConfig.StationId[0], mfg_data->mfg_mac_addr,
	       SZ_PHY_ADDR);
	memcpy(local->bssid, mfg_data->mfg_mac_addr, SZ_PHY_ADDR);
	memcpy(local->dev->dev_addr, mfg_data->mfg_mac_addr, SZ_PHY_ADDR);
	/* No PCF support right now */
	mib_p->StationConfig.CfPollable = 0;
	mib_p->StationConfig.CfPeriod = 0;
	mib_p->StationConfig.CfpMax = 0;
	mib_p->StationConfig.PrivOption = 1;
	mib_p->StationConfig.PwrMgtMode = 0;
	if (cfg_data->ssid[0] == 0x0) {
		memcpy(cfg_data->ssid, GET_STR_PARM(essid, dev_idx), SZ_SSID);
	}
	memcpy(mib_p->StationConfig.DesiredSsId, cfg_data->ssid, SZ_SSID);
	for (i = 0; (i < SZ_SSID) && (cfg_data->ssid[i] != 0); i++);
	local->essid_len = i;
	mib_p->StationConfig.DesiredBssType = 0;

	local->fixed_tx_data_rate = cfg_data->fixed_tx_data_rate;
	local->fixed_tx_b_rate_idx = cfg_data->fixed_tx_b_rate_idx;
	local->fixed_tx_g_rate_idx = cfg_data->fixed_tx_g_rate_idx;

	{
		for (i = 0, mask = 1, len = 0; i < WLAN_RATE_COUNT;
		     i++, mask <<= 1) {
			if ((cfg_data->oper_rate_set & mask) == mask) {
				switch (mask) {
				case WLAN_RATE_1M:
					rate = 2;
					break;
				case WLAN_RATE_2M:
					rate = 4;
					break;
				case WLAN_RATE_5M5:
					rate = 11;
					break;
				case WLAN_RATE_11M:
					rate = 22;
					break;
				case WLAN_RATE_22M:
					rate = 44;
					break;
				case WLAN_RATE_6M:
					rate = 12;
					break;
				case WLAN_RATE_9M:
					rate = 18;
					break;
				case WLAN_RATE_12M:
					rate = 24;
					break;
				case WLAN_RATE_18M:
					rate = 36;
					break;
				case WLAN_RATE_24M:
					rate = 48;
					break;
				case WLAN_RATE_36M:
					rate = 72;
					break;
				case WLAN_RATE_48M:
					rate = 96;
					break;
				case WLAN_RATE_54M:
					rate = 108;
					break;
				default:
					rate = 0;
					break;
				}
				if ((cfg_data->basic_rate_set & mask) == mask) {
					rate |= 0x80;
					local->max_basic_rate_idx = i;
				}
				mib_p->StationConfig.OpRateSet[len++] = rate;
			}
		}
		mib_p->StationConfig.OpRateSet[len] = 0;
	}

	mib_p->StationConfig.BcnPeriod = (cfg_data->beacon_interval == 0) ?
					    GET_INT_PARM(beacon_int, dev_idx) :
					    cfg_data->beacon_interval;
	mib_p->StationConfig.DtimPeriod = (cfg_data->dtim_period == 0) ?
					    GET_INT_PARM(dtim_period, dev_idx) :
					    cfg_data->dtim_period;

	local->wireless_enable = cfg_data->wireless_enable;

#ifdef TURBO_SETUP
	switch (cfg_data->auto_link) {

		case 0:
		  mib_p->StationConfig.mib_TSMode = AUTOLINK_OFF;
		  break;

		case 1:
		  mib_p->StationConfig.mib_TSMode = AUTOLINK_START;
		  break;

		case 2:
		  mib_p->StationConfig.mib_TSMode = AUTOLINK_FINISH;
		  break;

		default:
		  break;
	}

	if (cfg_data->ssid_patch)
		mib_p->StationConfig.mib_SSIDPatchDisable = 0;
	else
		mib_p->StationConfig.mib_SSIDPatchDisable = 1;
#endif

#ifdef BOOSTER_MODE
	mib_p->StationConfig.mib_BoosterMode = cfg_data->speed_booster;
	MVWLAN_BOOSTER_SetBoosterMode(0, local);
#endif

	/* For WEP Default Keys Table */
	mib_p->WepDefaultKeys[0].WepDefaultKeyIdx = 1;
	memcpy(mib_p->WepDefaultKeys[0].WepDefaultKeyValue,
	       cfg_data->wep_default_key1, 13);
	mib_p->WepDefaultKeys[1].WepDefaultKeyIdx = 2;
	memcpy(mib_p->WepDefaultKeys[1].WepDefaultKeyValue,
	       cfg_data->wep_default_key2, 13);
	mib_p->WepDefaultKeys[2].WepDefaultKeyIdx = 3;
	memcpy(mib_p->WepDefaultKeys[2].WepDefaultKeyValue,
	       cfg_data->wep_default_key3, 13);
	mib_p->WepDefaultKeys[3].WepDefaultKeyIdx = 4;
	memcpy(mib_p->WepDefaultKeys[3].WepDefaultKeyValue,
	       cfg_data->wep_default_key4, 13);

	/* For Privacy Table */
	mib_p->Privacy.PrivInvoked = cfg_data->privacy_invoked;
	/* TX WEP is always host based */
	if (cfg_data->privacy_invoked)
		local->host_encrypt = 1;
	mib_p->Privacy.WepDefaultKeyId = cfg_data->wep_default_key_index;
	mib_p->Privacy.ExcludeUnencrypt = cfg_data->exclude_unencrypted;

	/* For MAC Operation Table */
	memcpy(&mib_p->OperationTable.StaMacAddr[0], mfg_data->mfg_mac_addr,
	       SZ_PHY_ADDR);
	memcpy(&mib_p->OperationTable.StaMacAddr[0], mfg_data->mfg_mac_addr,
	       SZ_PHY_ADDR);
	mib_p->OperationTable.RtsThresh = cfg_data->rts_threshold > 2347 ?
			GET_INT_PARM(rts_threshold, dev_idx) :
				cfg_data->rts_threshold;
	mib_p->OperationTable.ShortRetryLim =
			cfg_data->short_retry_limit == 0 ?
				GET_INT_PARM(short_retry_limit, dev_idx) :
				cfg_data->short_retry_limit;
	mib_p->OperationTable.LongRetryLim = cfg_data->long_retry_limit == 0 ?
				GET_INT_PARM(long_retry_limit, dev_idx) :
				cfg_data->long_retry_limit;
	mib_p->OperationTable.FragThresh =
			(cfg_data->frag_threshold < 256) ||
			(cfg_data->frag_threshold > 2346) ?
				GET_INT_PARM(frag_threshold, dev_idx) :
				cfg_data->frag_threshold;
	mib_p->OperationTable.MaxTxMsduLife =
			GET_INT_PARM(max_tx_msdu_lifetime, dev_idx);
	mib_p->OperationTable.MaxRxLife =
			GET_INT_PARM(max_rx_lifetime, dev_idx);

	/* For PHY */
	if (! getRegDomainInfo((u8) mfg_data->domain, &minChannel, &maxChannel,
			       &defaultChannel)) {
		if ((cfg_data->channel < minChannel) ||
		    (cfg_data->channel > maxChannel))
			cfg_data->channel = defaultChannel;
		mib_p->PhyDSSSTable.CurrChan = cfg_data->channel;
		for (i = minChannel, mask = (1 << (minChannel - 1));
			i <= maxChannel; i++) {
		     local->channel_mask |= mask;
		}
	} else {
		mfg_data->domain = GET_INT_PARM(current_regdomain, dev_idx);
		if (! getRegDomainInfo((u8) mfg_data->domain, &minChannel,
				       &maxChannel, &defaultChannel)) {
			mib_p->PhyDSSSTable.CurrChan = defaultChannel;
			for (i = minChannel, mask = (1 << (minChannel - 1));
				i <= maxChannel; i++) {
			     local->channel_mask |= mask;
			}
		}
	}

	/* Local misc configuration */
	switch (mfg_data->antenna_cfg & 0x0F) {
	case 0x00:
	case 0x05:
	case 0x0a:
	case 0x08:
	case 0x09:
	case 0x04:
	case 0x01:
		break;
	default:
		mfg_data->antenna_cfg = GET_INT_PARM(antenna_mode, dev_idx);
		break;
	}
	switch (mfg_data->antenna_cfg & 0x0F) {
	case 0x00:
		local->mib_rxAntenna = AP_ANT_A;
		local->mib_txAntenna = AP_ANT_A;
		break;
	case 0x05:
		local->mib_rxAntenna = AP_ANT_B;
		local->mib_txAntenna = AP_ANT_B;
		break;
	case 0x0a:
		local->mib_rxAntenna = AP_ANT_DIVERSITY;
		local->mib_txAntenna = AP_ANT_DIVERSITY;
		break;
	case 0x08:
		local->mib_rxAntenna = AP_ANT_DIVERSITY;
		local->mib_txAntenna = AP_ANT_A;
		break;
	case 0x09:
		local->mib_rxAntenna = AP_ANT_DIVERSITY;
		local->mib_txAntenna = AP_ANT_B;
		break;
	case 0x04:
		local->mib_rxAntenna = AP_ANT_B;
		local->mib_txAntenna = AP_ANT_A;
		break;
	case 0x01:
		local->mib_rxAntenna = AP_ANT_A;
		local->mib_txAntenna = AP_ANT_B;
		break;
	default:
		break;
	}

	local->mib_ApMode = cfg_data->ap_mode;
	/* Only enable ERP protection mechanism when the AP is running in
	 * 802.11b/g mixed mode. */
	if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY)
		local->mib_ErpProtEnabled = 0;
	else
		local->mib_ErpProtEnabled = cfg_data->g_protect;
	memcpy(local->mib_DeviceName, cfg_data->dev_name, SZ_DEVICE_NAME);

	local->mib_phyHRDSSSTable.ShortPreambleOptionImplemented =
		cfg_data->short_preamble;
	local->mib_phyHRDSSSTable.PBCCOptionImplemented = 0;
	local->mib_phyHRDSSSTable.ChannelAgilityPresent = 0;
	local->mib_phyHRDSSSTable.ChannelAgilityEnabled = 0;
	local->mib_phyHRDSSSTable.HRCCAModeSupported = 0;
	local->mib_phyERPTable.ERPBCCOptionImplemented = 0;
	local->mib_phyERPTable.ERPBCCOptionEnabled = 0;
	local->mib_phyERPTable.DSSSOFDMOptionImplemented = 0;
	local->mib_phyERPTable.DSSSOFDMOptionEnabled = 0;
	local->mib_phyERPTable.ShortSlotTimeOptionImplemented = 1;
	/* The 802.11 standards requires the slot time should be 20 us
	 * when AP is running in 802.11b only mode. It is fixed at 9 us
	 * when AP is running in 802.11g only mode. When running in mixed
	 * mode, the value will be 9 us by default and changed to 20 us
	 * after a 802.11b client associated. */
	if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY)
		local->mib_phyERPTable.ShortSlotTimeOptionEnabled = 0;
	else
		local->mib_phyERPTable.ShortSlotTimeOptionEnabled = 1;

	/* For Authentication */
	local->auth_mode = cfg_data->auth_mode;
	/* For WEP */
	local->wep_key_len = cfg_data->wep_key_size;
#ifndef AP_WPA2
	/* For WPA */
	mib_p->Privacy.RSNEnabled = cfg_data->wpa_mode;
	if (cfg_data->wpa_mode) {
		local->host_encrypt = 1;
		local->host_decrypt = 1;
		mib_p->Privacy.PrivInvoked = 0; /* disable WEP. once if WPA is enabled, WEP setting is ignored */
	}
	mib_p->RSNConfig.GroupRekeyTime = cfg_data->wpa_group_rekey_time;
	mib_p->RSNConfig.MulticastCipher[3] = cfg_data->wpa_encry;
	mib_p->UnicastCiphers.UnicastCipher[3] = cfg_data->wpa_encry;

	strcpy(mib_p->RSNConfig.PSKPassPhrase, cfg_data->wpa_passphrase);
#else
	mib_p->Privacy.RSNEnabled = (cfg_data->wpa_mode | cfg_data->wpa2_mode);
	if (mib_p->Privacy.RSNEnabled) {
		local->host_encrypt = 1;
		local->host_decrypt = 1;
		local->ccmp_encry_delay_wait = 0;
		mib_p->Privacy.PrivInvoked = 0; /* disable WEP. once if WPA or WPA2 is enabled, WEP setting is ignored */
	}
	if (cfg_data->wpa2_mode && !cfg_data->wpa_mode) {
		mib_p->RSNConfigWPA2.WPA2Enabled = 0;
		mib_p->RSNConfigWPA2.WPA2OnlyEnabled = 1;
	} else if (cfg_data->wpa2_mode && cfg_data->wpa_mode) {
		mib_p->RSNConfigWPA2.WPA2Enabled = 1;
		mib_p->RSNConfigWPA2.WPA2OnlyEnabled = 0;
	} else {
		mib_p->RSNConfigWPA2.WPA2Enabled = 0;
		mib_p->RSNConfigWPA2.WPA2OnlyEnabled = 0;
	}
	mib_p->RSNConfig.GroupRekeyTime = cfg_data->group_rekey_time;
	mib_p->UnicastCiphers.UnicastCipher[3] = cfg_data->wpa_encry;
	mib_p->WPA2UnicastCiphers.UnicastCipher[3] = WPA2_ENCRY_CCMP;
	if (cfg_data->wpa2_encry == WPA2_ENCRY_TKIP_OR_CCMP) {
		mib_p->WPA2UnicastCiphers2.UnicastCipher[3] = WPA2_ENCRY_TKIP;
	} else if (cfg_data->wpa2_encry == WPA2_ENCRY_CCMP_ONLY) {
		mib_p->WPA2UnicastCiphers2.UnicastCipher[3] = WPA2_ENCRY_CCMP;
	} else
		goto fail;
	strcpy(mib_p->RSNConfig.PSKPassPhrase, cfg_data->wpa_passphrase);
	strcpy(mib_p->RSNConfigWPA2.PSKPassPhrase, cfg_data->wpa2_passphrase);
	if (mib_p->Privacy.RSNEnabled &&
		!mib_p->RSNConfigWPA2.WPA2Enabled &&
		!mib_p->RSNConfigWPA2.WPA2OnlyEnabled) {
		mib_p->RSNConfig.MulticastCipher[3] = cfg_data->wpa_encry;
	} else if (mib_p->RSNConfigWPA2.WPA2Enabled) {
		if (cfg_data->wpa_encry == WPA_ENCRY_CCMP && cfg_data->wpa2_encry == WPA2_ENCRY_CCMP) {
			mib_p->RSNConfig.MulticastCipher[3] = WPA_ENCRY_CCMP;
			mib_p->RSNConfigWPA2.MulticastCipher[3] = WPA2_ENCRY_CCMP;
		} else {
			mib_p->RSNConfig.MulticastCipher[3] = WPA_ENCRY_TKIP;
			mib_p->RSNConfigWPA2.MulticastCipher[3] = WPA2_ENCRY_TKIP;
		}
	} else if (mib_p->RSNConfigWPA2.WPA2OnlyEnabled) {
		if (cfg_data->wpa2_encry == WPA2_ENCRY_CCMP_ONLY) {
			mib_p->RSNConfigWPA2.MulticastCipher[3] = WPA2_ENCRY_CCMP;
		} else {
			mib_p->RSNConfigWPA2.MulticastCipher[3] = WPA2_ENCRY_TKIP;
		}
	} else {
		mib_p->RSNConfig.MulticastCipher[3] = WPA_ENCRY_TKIP;
		mib_p->RSNConfigWPA2.MulticastCipher[3] = WPA2_ENCRY_TKIP;
	}
#endif

	local->wlan_tx_gpio = cfg_data->wlan_tx_gpio;
	local->wlan_rx_gpio = cfg_data->wlan_rx_gpio;
	{
	extern UINT8 mib_rxAntenna;

	mib_rxAntenna = cfg_data->antenna;
	}
	local->watchdog_timer = cfg_data->watchdog_timer;

	if (cfg_data->hide_ssid)
		local->enh_sec |= MVWLAN_ENHSEC_BCAST_SSID;
	local->wds_max_connections = 16;

	/* Initialize task queue structures */
	MVWLAN_QUEUE_INIT(&local->set_multicast_list_queue,
			  mvWLAN_set_multicast_list_queue, local->dev);

	/* Enable the following when we are ready */
#if 0
	/* For passive scan */
	init_timer(&local->passive_scan_timer);
	local->passive_scan_timer.data = (unsigned long) local;
	local->passive_scan_timer.function = mvWLAN_passive_scan;

	/* For host scan */
	init_waitqueue_head(&local->hostscan_wq);

	/* For software watchdog */
	init_timer(&local->tick_timer);
	local->tick_timer.data = (unsigned long) local;
	local->tick_timer.function = mvWLAN_tick_timer;
	local->tick_timer.expires = jiffies + 2 * HZ;
	add_timer(&local->tick_timer);
#endif

	/* For routine function */
	init_timer(&local->routine_timer);
	local->routine_timer.data = (unsigned long) local;
	local->routine_timer.function = mvWLAN_routine_timer;
	local->routine_timer.expires = jiffies + (HZ / 20);
	add_timer(&local->routine_timer);

	/* Initialize the crypt libraries used by kernel. */
	mvWLAN_init_crypt_lib(local);

	if (mib_p->Privacy.PrivInvoked || mib_p->Privacy.RSNEnabled) {
#ifndef AP_WPA2
		struct mvwlan_crypt_data *new_crypt;
		char *crypt_name;

		/* Take WEP/WPA into use */
		new_crypt = (struct mvwlan_crypt_data *)
			kmalloc(sizeof(struct mvwlan_crypt_data), GFP_KERNEL);
		if (new_crypt == NULL)
			goto fail;
		memset(new_crypt, 0, sizeof(struct mvwlan_crypt_data));
		if (mib_p->Privacy.RSNEnabled) {
			if (cfg_data->wpa_encry == WPA_ENCRY_TKIP)
				crypt_name = "TKIP";
			else if (cfg_data->wpa_encry == WPA_ENCRY_CCMP)
				crypt_name = "CCMP";
			else {
				printk(KERN_WARNING "%s: could not know WPA encryption method %d\n",
				       local->dev->name, cfg_data->wpa_encry);
				goto fail;
			}
		} else
			crypt_name = "WEP";
		new_crypt->ops = mvWLAN_get_crypto_ops(crypt_name);
		if (new_crypt->ops)
			new_crypt->priv = new_crypt->ops->init();
		if (!new_crypt->ops || (mib_p->Privacy.PrivInvoked && !new_crypt->priv)) {
			kfree(new_crypt);
			new_crypt = NULL;

			printk(KERN_WARNING "%s: could not find %s library\n",
			       local->dev->name, crypt_name);
			goto fail;
		}
		local->crypt = new_crypt;

		if (mib_p->Privacy.PrivInvoked) {
			for (i = 0; i < MVWLAN_WEP_KEYS; i++) {
				MIB_WEP_DEFAULT_KEYS *mib_WepDefaultKey_p;

				mib_WepDefaultKey_p =
					&mib_p->WepDefaultKeys[i];
				local->crypt->ops->set_key(i,
					mib_WepDefaultKey_p->WepDefaultKeyValue,
					local->wep_key_len,
					local->crypt->priv);
			}
			local->crypt->ops->set_key_idx(
					mib_p->Privacy.WepDefaultKeyId-1,
					local->crypt->priv);
		}
#else
		if (mib_p->Privacy.RSNEnabled) {

			local->tkip_crypt = (struct mvwlan_crypt_data *)
				kmalloc(sizeof(struct mvwlan_crypt_data), GFP_KERNEL);
			if (local->tkip_crypt == NULL)
				goto fail;
			memset(local->tkip_crypt, 0, sizeof(struct mvwlan_crypt_data));
			local->tkip_crypt->ops = mvWLAN_get_crypto_ops("TKIP");
			if (local->tkip_crypt->ops) {

				local->tkip_crypt->priv = local->tkip_crypt->ops->init();
			} else {

				kfree(local->tkip_crypt);
				local->tkip_crypt = NULL;

				printk(KERN_WARNING "%s: could not find %s library\n",
					local->dev->name, "TKIP");
				goto fail;
			}

			local->ccmp_crypt = (struct mvwlan_crypt_data *)
				kmalloc(sizeof(struct mvwlan_crypt_data), GFP_KERNEL);
			if (local->ccmp_crypt == NULL)
				goto fail;
			memset(local->ccmp_crypt, 0, sizeof(struct mvwlan_crypt_data));
			local->ccmp_crypt->ops = mvWLAN_get_crypto_ops("CCMP");
			if (local->ccmp_crypt->ops) {

				local->ccmp_crypt->priv = local->ccmp_crypt->ops->init();
			} else {

				kfree(local->tkip_crypt);
				kfree(local->ccmp_crypt);
				local->tkip_crypt = NULL;
				local->ccmp_crypt = NULL;

				printk(KERN_WARNING "%s: could not find %s library\n",
					local->dev->name, "CCMP");
				goto fail;
			}

			if (cfg_data->wpa_mode && cfg_data->wpa2_mode) {

				if ((cfg_data->wpa_encry == WPA_ENCRY_CCMP) &&
					(cfg_data->wpa2_encry == WPA2_ENCRY_CCMP_ONLY))
					local->multicast_crypt = local->ccmp_crypt;
				else
					local->multicast_crypt = local->tkip_crypt;
			} else if (cfg_data->wpa_mode) {

				if (cfg_data->wpa_encry == WPA_ENCRY_CCMP)
					local->multicast_crypt = local->ccmp_crypt;
				else
					local->multicast_crypt = local->tkip_crypt;
			} else if (cfg_data->wpa2_mode) {

				if (cfg_data->wpa2_encry == WPA2_ENCRY_CCMP_ONLY)
					local->multicast_crypt = local->ccmp_crypt;
				else
					local->multicast_crypt = local->tkip_crypt;
			} else {

				local->multicast_crypt = NULL;
			}

			local->crypt = local->multicast_crypt;

		} else if (mib_p->Privacy.PrivInvoked) {

			local->crypt = (struct mvwlan_crypt_data *)
				kmalloc(sizeof(struct mvwlan_crypt_data), GFP_KERNEL);
			if (local->crypt == NULL)
				goto fail;
			memset(local->crypt, 0, sizeof(struct mvwlan_crypt_data));
			local->crypt->ops = mvWLAN_get_crypto_ops("WEP");
			if (local->crypt->ops)
				local->crypt->priv = local->crypt->ops->init();

			if (!local->crypt->ops || (mib_p->Privacy.PrivInvoked && !local->crypt->priv)) {
				kfree(local->crypt);
				local->crypt = NULL;

				printk(KERN_WARNING "%s: could not find %s library\n",
					local->dev->name, "WEP");
				goto fail;
			}

			for (i = 0; i < MVWLAN_WEP_KEYS; i++) {
				MIB_WEP_DEFAULT_KEYS *mib_WepDefaultKey_p;

				mib_WepDefaultKey_p =
					&mib_p->WepDefaultKeys[i];
				local->crypt->ops->set_key(i,
					mib_WepDefaultKey_p->WepDefaultKeyValue,
					local->wep_key_len,
					local->crypt->priv);
			}

			local->crypt->ops->set_key_idx(
					mib_p->Privacy.WepDefaultKeyId-1,
					local->crypt->priv);
		}
#endif
	}

    /* pass the cfg_data for next initialization routine */
    local->cfg_data = cfg_data;    
    
	/* Initialize the hw resources used by kernel. */
	if (mvWLAN_setup_hw_resources(local))
		goto fail;

	/* Initialize the hardware device */
	local->func->hw_init(local->dev);

	mvWLAN_setup_dev(local->dev, local, 1);

	return local;

 fail:
	kfree(local->ap);
	kfree(local->dev);
	kfree(local->apdev);
	kfree(local->stadev);
	kfree(local);
	return NULL;
}


int mvWLAN_init_dev(void *data)
{
	struct mvwlan_cfg_param *cfg_data;
	struct mvwlan_mfg_param *mfg_data;
	local_info_t *local;

	mfg_data = (struct mvwlan_mfg_param *) data;
	cfg_data = (struct mvwlan_cfg_param *)
			(data + sizeof(struct mvwlan_mfg_param));
	local = init_local_data(cfg_data, mfg_data, dev_index);
	if (! local) {
		PDEBUG(DEBUG_FLOW, "mvWLAN: Failed to initialize private "
		       "data\n");
		return -1;
	} else {
		dev_local[dev_index++] = local;
	}

	if (register_netdev(local->dev)) {
		PDEBUG(DEBUG_FLOW, "mvWLAN: register_netdev() failed!\n");
		return -1;
	}
	PDEBUG(DEBUG_FLOW, "mvWLAN: Registered netdevice %s\n",
	       local->dev->name);

	mvWLAN_init_dev_proc(local);

	mvWLAN_init_ap_data(local);

    /* not available anymore after initialization finished */
    local->cfg_data = NULL;
    
	return 0;
}


static int free_local_data(local_info_t *local)
{
	mvwlan_wds_info_t *wds, *prev;
	struct mvwlan_tx_callback_info *tx_cb, *tx_cb_prev;
	int i;

	if (local == NULL)
		return -1;

#if 0
	if (timer_pending(&local->passive_scan_timer))
		del_timer(&local->passive_scan_timer);

	if (timer_pending(&local->tick_timer))
		del_timer(&local->tick_timer);

	if (local->dev_enabled)
		mvWLAN_callback(local, MVWLAN_CALLBACK_DISABLE);
#endif

	if (timer_pending(&local->routine_timer))
		del_timer(&local->routine_timer);

	mvWLAN_deinit_crypt_lib(local);

	if (local->ap != NULL)
		mvWLAN_free_ap_data(local->ap);

	mvWLAN_remove_dev_proc(local);

	wds = local->wds;
	local->wds = NULL;
	while (wds != NULL) {
		unregister_netdev(&wds->dev);
		prev = wds;
		wds = wds->next;
		kfree(prev);
	}

	tx_cb = local->tx_callback;
	while (tx_cb != NULL) {
		tx_cb_prev = tx_cb;
		tx_cb = tx_cb->next;
		kfree(tx_cb_prev);
	}

	if (local->dev && local->dev->name && local->dev->name[0]) {
		unregister_netdev(local->dev);
		printk(KERN_DEBUG "mvWLAN: Netdevice %s unregistered\n",
		       local->dev->name);
	}
	kfree(local->dev);

	for (i = 0; i < MVWLAN_FRAG_CACHE_LEN; i++) {
		if (local->frag_cache[i].skb != NULL)
			dev_kfree_skb(local->frag_cache[i].skb);
	}

	kfree(local->ap);
	kfree(local->last_scan_results);
	kfree(local->sysConfig->CalData);
	kfree(local);

	return 0;
}


int mvWLAN_free_dev()
{
	int i, rc = 0;

	for (i = 0; i < dev_index; i++) {
		rc |= free_local_data(dev_local[i]);
		dev_local[i] = NULL;
	}
	dev_index = 0;

	return rc;
}


static void * ap_crypt_get_ptrs(struct ap_data *ap, u8 *addr, int permanent,
				struct mvwlan_crypt_data ***crypt)
{
	return NULL;

#if 0
	struct sta_info *sta;

	spin_lock_bh(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, addr);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&ap->sta_table_lock);

	if (!sta && permanent)
		sta = mvWLAN_ap_add_sta(ap, addr);

	if (!sta)
		return NULL;

	if (permanent)
		sta->flags |= WLAN_STA_PERM;

	*crypt = &sta->crypt;

	return sta;
#endif	
}


static int mvWLAN_set_encryption(local_info_t *local)
{
	int i, keylen = 0, len, idx;
	MIB_PRIVACY_TABLE *mib_Privacy_p = 
			&(local->sysConfig->Mib802dot11->Privacy);
	char keybuf[MVWLAN_WEP_KEY_LEN + 1];
	int encrypt_type, hw_encrypt_type;

	if (local->crypt == NULL || local->crypt->ops == NULL) {
		encrypt_type = MVWLAN_ENCRYPT_TYPE_NONE;
		hw_encrypt_type = MVWLAN_ENCRYPT_TYPE_NONE;
	}
	else if (strcmp(local->crypt->ops->name, "WEP") == 0) {
		encrypt_type = MVWLAN_ENCRYPT_TYPE_WEP;
		hw_encrypt_type = MVWLAN_ENCRYPT_TYPE_WEP;
	}
	else if (strcmp(local->crypt->ops->name, "WPA") == 0) {
		encrypt_type = MVWLAN_ENCRYPT_TYPE_WPA;
		hw_encrypt_type = MVWLAN_ENCRYPT_TYPE_WPA;
	}
	else
		encrypt_type = MVWLAN_ENCRYPT_TYPE_OTHER;

	if (mib_Privacy_p->PrivInvoked) {
		encrypt_type = MVWLAN_ENCRYPT_TYPE_WEP;
		if (mib_Privacy_p->ExcludeUnencrypt)
			hw_encrypt_type = MVWLAN_ENCRYPT_TYPE_WPA;
		else
			hw_encrypt_type = MVWLAN_ENCRYPT_TYPE_WEP;
	}
	if (local->ieee_802_1x && local->host_decrypt) {
		hw_encrypt_type = MVWLAN_ENCRYPT_TYPE_WPA;
	}
	if (hw_encrypt_type != MVWLAN_ENCRYPT_TYPE_NONE) {
		if (local->func->set_cmd(local->dev, APCMD_SET_ENCRYPTION, 0,
					 &hw_encrypt_type, sizeof(int))) {
			printk(KERN_DEBUG "Could not set encryption mode to "
			       "hardware (mode=%d)\n", hw_encrypt_type);
			goto fail;
		}
	}

	if (encrypt_type == MVWLAN_ENCRYPT_TYPE_NONE) {
		/* XXX, Anything else ? */
	} else if (encrypt_type == MVWLAN_ENCRYPT_TYPE_WEP) {
		/* 104-bit support seems to require that all the keys are 
		 * set to the same keylen */
		if (local->wep_key_len == 5)
			/* first 5 octets */
			keylen = 6;
		else if (local->wep_key_len == 13)
			/* first 13 octets */
			keylen = MVWLAN_WEP_KEY_LEN + 1;
		else {
			printk(KERN_DEBUG "Invalid key length (len=%d)\n", 
			       keylen);
			goto fail;
		}
		len = keylen - 1;
		if (local->func->set_cmd(local->dev, APCMD_SET_WEP_KEY_LEN, 0,
					 &len, sizeof(int))) {
			printk(KERN_DEBUG "Could not set key len=%d\n", len);
			goto fail;
		}
		idx = local->crypt->ops->get_key_idx(local->crypt->priv);
		len = local->crypt->ops->get_key(idx, keybuf, sizeof(keybuf),
						 local->crypt->priv);

		for (i = 0; i < MVWLAN_WEP_KEYS; i++) {
			memset(keybuf, 0, sizeof(keybuf));
			(void) local->crypt->ops->get_key(i, keybuf, 
							  sizeof(keybuf),
							  local->crypt->priv);
			if (local->func->set_cmd(local->dev,
						 APCMD_SET_WEP_KEY, i,
						 keybuf, keylen-1)) {
				printk(KERN_DEBUG "Could not set key %d "
				       "(len=%d)\n", i, keylen);
				goto fail;
			}
			if ((i == idx) &&
			    (local->func->set_cmd(local->dev, 
						  APCMD_SET_WEP_DEFAULT_KEY, 
						  idx, keybuf, keylen-1))) {
				printk(KERN_DEBUG "Could not set default "
				       "keyid %d\n", idx);
				goto fail;
			}
		}
	} else if (encrypt_type == MVWLAN_ENCRYPT_TYPE_WPA) {
		/* XXX, TBD */
	} else	
		return 0;


	return 0;

 fail:
	printk(KERN_DEBUG "%s: encryption setup failed\n", local->dev->name);
	return -1;
}


static int mvWLAN_set_antsel(local_info_t *local, int antenna, int control)
{
	int cmd, ret = 0;

	switch (antenna) {
	case MVWLAN_RX_ANTENNA:
		cmd = APCMD_SET_RX_ANTENNA; break;
	case MVWLAN_TX_ANTENNA:
		cmd = APCMD_SET_TX_ANTENNA; break;
	default:
		return -EINVAL;
	}
	switch (control) {
	case MVWLAN_ANTSEL_DIVERSITY:
		break;
	case MVWLAN_ANTSEL_LOW:
	case MVWLAN_ANTSEL_HIGH:
		/* XXX, need to handle enabling/disabling the antenna here */
		return -EINVAL;
	}
	if (local->func->set_cmd(local->dev, cmd, 0, &control, 
				 sizeof(int)) == 0) {
		printk(KERN_INFO "%s: setting %s antenna failed\n",
		       antenna == MVWLAN_RX_ANTENNA ? "RX" : "TX", 
		       local->dev->name);
		ret = -EOPNOTSUPP;
	}

	return ret;
}


static int mvWLAN_hostscan(local_info_t *local, int rate)
{
	struct mvwlan_hostscan_request scan_req;
	u16 rateIdx;

	memset(&scan_req, 0, sizeof(scan_req));
	scan_req.channel_list = __constant_cpu_to_le16(0x3fff);
	switch (rate) {
	case 1: rateIdx = WLAN_RATE_1M; break;
	case 2: rateIdx = WLAN_RATE_2M; break;
	case 3: rateIdx = WLAN_RATE_5M5; break;
	case 4: rateIdx = WLAN_RATE_11M; break;
	default: rateIdx = WLAN_RATE_1M; break;
	}
	scan_req.txrate = cpu_to_le16(rate);
	/* leave SSID empty to accept all SSIDs */

	/* XXX, TBD */ 
	return 0;
}


static int mvWLAN_set_roaming(local_info_t *local)
{
	/* XXX, TBD */
	return 0;
}


static int mvWLAN_enable_hostapd(local_info_t *local, int rtnl_locked)
{
	struct net_device *dev = local->dev;
	int ret;

	if (local->hostapd)
		return 0;

	printk(KERN_DEBUG "%s: enabling hostapd mode\n", dev->name);

	local->apdev = kmalloc(sizeof(struct net_device) + MVWLAN_NETDEV_EXTRA,
			       GFP_KERNEL);
	if (local->apdev == NULL)
		return -ENOMEM;
	memset(local->apdev, 0, sizeof(struct net_device) +
	       MVWLAN_NETDEV_EXTRA);
	mvWLAN_set_dev_name(local->apdev, local->apdev + 1);

	local->apdev->priv = local;
	memcpy(local->apdev->dev_addr, dev->dev_addr, ETH_ALEN);
	mvWLAN_setup_dev(local->apdev, local, 0);
	local->apdev->hard_start_xmit = local->func->tx_80211;
	local->apdev->type = ARPHRD_IEEE80211;
	local->apdev->hard_header_parse = mvWLAN_80211_header_parse;

	local->apdev->base_addr = dev->base_addr;
	local->apdev->irq = dev->irq;
	local->apdev->mem_start = dev->mem_start;
	local->apdev->mem_end = dev->mem_end;
	sprintf(local->apdev->name, "%sap", dev->name);
	if (rtnl_locked)
		ret = register_netdevice(local->apdev);
	else
		ret = register_netdev(local->apdev);
	if (ret) {
		printk(KERN_WARNING "%s: register_netdevice(AP) failed!\n",
		       dev->name);
		return -1;
	}
	printk(KERN_DEBUG "%s: Registered netdevice %s for AP management\n",
	       dev->name, local->apdev->name);


	local->stadev = kmalloc(sizeof(struct net_device) +
				MVWLAN_NETDEV_EXTRA,
				GFP_KERNEL);
	if (local->stadev == NULL)
		return -ENOMEM;
	memset(local->stadev, 0, sizeof(struct net_device) +
	       MVWLAN_NETDEV_EXTRA);
	mvWLAN_set_dev_name(local->stadev, local->stadev + 1);

	local->stadev->priv = local;
	memcpy(local->stadev->dev_addr, dev->dev_addr, ETH_ALEN);
	mvWLAN_setup_dev(local->stadev, local, 0);

	local->stadev->base_addr = dev->base_addr;
	local->stadev->irq = dev->irq;
	local->stadev->mem_start = dev->mem_start;
	local->stadev->mem_end = dev->mem_end;
	sprintf(local->stadev->name, "%ssta", dev->name);
	if (rtnl_locked)
		ret = register_netdevice(local->stadev);
	else
		ret = register_netdev(local->stadev);
	if (ret) {
		printk(KERN_WARNING "%s: register_netdevice(STA) failed!\n",
		       dev->name);
		return -1;
	}
	printk(KERN_DEBUG "%s: Registered netdevice %s for STA use\n",
	       dev->name, local->stadev->name);

	local->hostapd = 1;

	return 0;
}


static int mvWLAN_disable_hostapd(local_info_t *local, int rtnl_locked)
{
	struct net_device *dev = local->dev;

	if (! local->hostapd)
		return 0;

	printk(KERN_DEBUG "%s: disabling hostapd mode\n", dev->name);

	if (local->apdev && local->apdev->name && local->apdev->name[0]) {
		if (rtnl_locked)
			unregister_netdevice(local->apdev);
		else
			unregister_netdev(local->apdev);
		printk(KERN_DEBUG "%s: Netdevice %s unregistered\n",
		       dev->name, local->apdev->name);
	}
	kfree(local->apdev);
	local->apdev = NULL;

	if (local->stadev && local->stadev->name && local->stadev->name[0]) {
		if (rtnl_locked)
			unregister_netdevice(local->stadev);
		else
			unregister_netdev(local->stadev);
		printk(KERN_DEBUG "%s: Netdevice %s unregistered\n",
		       dev->name, local->stadev->name);
	}
	kfree(local->stadev);
	local->stadev = NULL;

	local->hostapd = 0;

	return 0;
}


static int mvWLAN_get_datarates(struct net_device *dev, u8 *rates)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg = 
			&(local->sysConfig->Mib802dot11->StationConfig);
	int index;

	for (index = 0; mib_StaCfg->OpRateSet[index] != 0; index++) {
		rates[index] = (mib_StaCfg->OpRateSet[index] & 
				IEEEtypes_BASIC_RATE_MASK);
	}
	return index;
}


static int mvWLAN_iw_ioctl_giwname(struct net_device *dev,
				   struct iw_request_info *info,
				   char *name, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;

	switch (local->mib_ApMode) {
	case MVWLAN_AP_MODE_B_ONLY:
		strncpy(name, "IEEE 802.11b-DS", IFNAMSIZ);
		break;
	case MVWLAN_AP_MODE_G_ONLY:
		strncpy(name, "IEEE 802.11g-OFDM", IFNAMSIZ);
		break;
	case MVWLAN_AP_MODE_MIXED:
		strncpy(name, "IEEE 802.11bg-OFDM/DS", IFNAMSIZ);
		break;
	}
	return 0;
}


static void mvWLAN_crypt_delayed_deinit(local_info_t *local,
					struct mvwlan_crypt_data **crypt)
{
	struct mvwlan_crypt_data *tmp;
	unsigned long flags;

	tmp = *crypt;
	*crypt = NULL;

	if (tmp == NULL)
		return;

	/* must not run ops->deinit() while there may be pending encrypt or
	 * decrypt operations. Use a list of delayed deinits to avoid needing
	 * locking. */

	spin_lock_irqsave(&local->lock, flags);
	list_add(&tmp->list, &local->crypt_deinit_list);
	if (!timer_pending(&local->crypt_deinit_timer)) {
		local->crypt_deinit_timer.expires = jiffies + HZ;
		add_timer(&local->crypt_deinit_timer);
	}
	spin_unlock_irqrestore(&local->lock, flags);
}


static int mvWLAN_iw_ioctl_siwencode(struct net_device *dev,
				     struct iw_request_info *info,
				     struct iw_point *erq, char *keybuf)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg_p = 
			&(local->sysConfig->Mib802dot11->StationConfig);
	MIB_PRIVACY_TABLE *mib_Privacy_p = 
			&(local->sysConfig->Mib802dot11->Privacy);
	int i;
	int first = 0;

	if (! mib_StaCfg_p->PrivOption)
		return -EOPNOTSUPP;

	if (erq->flags & IW_ENCODE_DISABLED) {
		mvWLAN_crypt_delayed_deinit(local, &local->crypt);
		goto done;
	}

	if (local->crypt != NULL && local->crypt->ops != NULL &&
	    strcmp(local->crypt->ops->name, "WEP") != 0) {
		/* changing to use WEP; deinit previously used algorithm */
		mvWLAN_crypt_delayed_deinit(local, &local->crypt);
	}

	if (local->crypt == NULL) {
		struct mvwlan_crypt_data *new_crypt;

		/* take WEP into use */
		new_crypt = (struct mvwlan_crypt_data *)
			kmalloc(sizeof(struct mvwlan_crypt_data), GFP_KERNEL);
		if (new_crypt == NULL)
			return -ENOMEM;
		memset(new_crypt, 0, sizeof(struct mvwlan_crypt_data));
		new_crypt->ops = mvWLAN_get_crypto_ops("WEP");
		if (new_crypt->ops)
			new_crypt->priv = new_crypt->ops->init();
		if (!new_crypt->ops || !new_crypt->priv) {
			kfree(new_crypt);
			new_crypt = NULL;

			printk(KERN_WARNING "%s: could not find WEP library\n",
			       dev->name);
			return -EOPNOTSUPP;
		}
		first = 1;
		local->crypt = new_crypt;
		mib_Privacy_p->PrivInvoked = 1;
	}

	i = erq->flags & IW_ENCODE_INDEX;
	if (i < 1 || i > 4)
		i = local->crypt->ops->get_key_idx(local->crypt->priv);
	else
		i--;
	if (i < 0 || i >= MVWLAN_WEP_KEYS)
		return -EINVAL;

	if (erq->length > 0) {
		int len = erq->length <= 5 ? 5 : 13;
		if (len > erq->length)
			memset(keybuf + erq->length, 0, len - erq->length);
		local->crypt->ops->set_key(i, keybuf, len, local->crypt->priv);
		if (first) {
			local->crypt->ops->set_key_idx(i, local->crypt->priv);
		mib_Privacy_p->WepDefaultKeyId = i;
		}
	} else {
		if (local->crypt->ops->set_key_idx(i, local->crypt->priv) < 0)
			return -EINVAL; /* keyidx not valid */
	}

 done:
	mib_Privacy_p->ExcludeUnencrypt = erq->flags & IW_ENCODE_OPEN ? 0: 1;

	if (mvWLAN_set_encryption(local)) {
		printk(KERN_DEBUG "%s: set_encryption failed\n", dev->name);
		return -EINVAL;
	}

	return 0;
}


static int mvWLAN_iw_ioctl_giwencode(struct net_device *dev,
				     struct iw_request_info *info,
				     struct iw_point *erq, char *key)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_PRIVACY_TABLE *mib_Privacy_p = 
				&(local->sysConfig->Mib802dot11->Privacy);
	int i, len;

	if (local->crypt == NULL || local->crypt->ops == NULL) {
		erq->length = 0;
		erq->flags = IW_ENCODE_DISABLED;
		return 0;
	}

	if (strcmp(local->crypt->ops->name, "WEP") != 0) {
		/* only WEP is supported with wireless extensions, so just
		 * report that encryption is used */
		erq->length = 0;
		erq->flags = IW_ENCODE_ENABLED;
		return 0;
	}

	i = erq->flags & IW_ENCODE_INDEX;
	if (i < 1 || i > 4)
		i = local->crypt->ops->get_key_idx(local->crypt->priv);
	else
		i--;
	if (i < 0 || i >= MVWLAN_WEP_KEYS)
		return -EINVAL;

	erq->flags = i + 1;

	/* Reads from driver buffer */
	len = local->crypt->ops->get_key(i, key, MVWLAN_WEP_KEY_LEN,
					 local->crypt->priv);
	erq->length = (len >= 0 ? len : 0);

	if (mib_Privacy_p->PrivInvoked)
		erq->flags |= IW_ENCODE_ENABLED;
	else
		erq->flags |= IW_ENCODE_DISABLED;
	if (mib_Privacy_p->ExcludeUnencrypt)
		erq->flags |= IW_ENCODE_RESTRICTED;
	else
		erq->flags |= IW_ENCODE_OPEN;

	return 0;
}


static int ap_get_sta_qual(local_info_t *local, struct sockaddr addr[],
			   struct iw_quality qual[], int buf_size,
			   int aplist)
{
	struct ap_data *ap = local->ap;
	struct list_head *ptr;
	int count = 0;

	spin_lock_bh(&ap->sta_table_lock);

	for (ptr = ap->sta_list.next; ptr != NULL && ptr != &ap->sta_list;
	     ptr = ptr->next) {
		struct sta_info *sta = (struct sta_info *) ptr;

		if (aplist && !sta->ap)
			continue;
		addr[count].sa_family = ARPHRD_ETHER;
		memcpy(addr[count].sa_data, sta->addr, ETH_ALEN);
		/* XXX, need to find a way to get this field */
		if (sta->last_rx_silence == 0)
			qual[count].qual = sta->last_rx_signal < 27 ?
				0 : (sta->last_rx_signal - 27) * 92 / 127;
		else
			qual[count].qual = sta->last_rx_signal -
				sta->last_rx_silence - 35;
		qual[count].level = sta->last_rx_signal;
		qual[count].noise = sta->last_rx_silence;
		qual[count].updated = sta->last_rx_updated;

		sta->last_rx_updated = 0;

		count++;
		if (count >= buf_size)
			break;
	}

	spin_unlock_bh(&ap->sta_table_lock);

	return count;
}


#if WIRELESS_EXT <= 15
static int mvWLAN_iw_ioctl_giwspy(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_point *srq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct sockaddr addr[IW_MAX_SPY];
	struct iw_quality qual[IW_MAX_SPY];

	if (local->iw_mode != IW_MODE_MASTER) {
		printk("SIOCGIWSPY is currently only supported in master "
		       "mode\n");
		srq->length = 0;
		return -EOPNOTSUPP;
	}

	srq->length = ap_get_sta_qual(local, addr, qual, IW_MAX_SPY, 0);

	memcpy(extra, &addr, sizeof(addr[0]) * srq->length);
	memcpy(extra + sizeof(addr[0]) * srq->length, &qual,
	       sizeof(qual[0]) * srq->length);

	return 0;
}
#endif /* WIRELESS_EXT <= 15 */


static int mvWLAN_set_rate(local_info_t *local, u32 tx_rate_control)
{
	MIB_STA_CFG *mib_StaCfg = 
			&(local->sysConfig->Mib802dot11->StationConfig);
	int i, mask;

	/* AP/STA specific configuration */
	if (local->func->set_cmd(local->dev, APCMD_SET_OPER_RATE_SET, 0,
				 (void *) &tx_rate_control, sizeof(u32))) {
		return -EINVAL;
	}

	local->tx_rate_control = tx_rate_control;

	for (i = 0, mask = 1; i < WLAN_RATE_COUNT; i++, mask <<= 1) {
		if ((tx_rate_control & mask) == mask) {
			switch (mask) {
			case WLAN_RATE_1M:
				mib_StaCfg->OpRateSet[i] = 2;
				break;
			case WLAN_RATE_2M:
				mib_StaCfg->OpRateSet[i] = 4;
				break;
			case WLAN_RATE_5M5:
				mib_StaCfg->OpRateSet[i] = 11;
				break;
			case WLAN_RATE_11M:
				mib_StaCfg->OpRateSet[i] = 22;
				break;
			case WLAN_RATE_6M:
				mib_StaCfg->OpRateSet[i] = 12;
				break;
			case WLAN_RATE_9M:
				mib_StaCfg->OpRateSet[i] = 18;
				break;
			case WLAN_RATE_12M:
				mib_StaCfg->OpRateSet[i] = 24;
				break;
			case WLAN_RATE_18M:
				mib_StaCfg->OpRateSet[i] = 36;
				break;
			case WLAN_RATE_24M:
				mib_StaCfg->OpRateSet[i] = 48;
				break;
			case WLAN_RATE_36M:
				mib_StaCfg->OpRateSet[i] = 72;
				break;
			case WLAN_RATE_48M:
				mib_StaCfg->OpRateSet[i] = 96;
				break;
			case WLAN_RATE_54M:
				mib_StaCfg->OpRateSet[i] = 108;
				break;
			}
			if (local->basic_rate_set & mask)
				mib_StaCfg->OpRateSet[i] |= 
					IEEEtypes_BASIC_RATE_MASK;
		}
	}
	/* Null terminated the last supported rate */
	mib_StaCfg->OpRateSet[i] = 0;

	/* Update TX rate configuration for all STAs based on new operational
	 * rate set. */
	mvWLAN_update_rates(local);

	return 0;
}


static int mvWLAN_set_basic_rate(local_info_t *local, u32 basic_rate_control)
{
	MIB_STA_CFG *mib_StaCfg = 
			&(local->sysConfig->Mib802dot11->StationConfig);
	int i;
	u32 mask;

	if (local->func->set_cmd(local->dev, APCMD_SET_BASIC_RATE, 0, 
				 &basic_rate_control, sizeof(u32))) {
		return -EOPNOTSUPP;
	}

	mask = 1 << (WLAN_RATE_COUNT - 1);
	for (i = (WLAN_RATE_COUNT) - 1; i >= 0; i--, mask >>= 1) {
		if ((mask & basic_rate_control) == mask) {
			local->max_basic_rate_idx = i;
			break;
		}
	}
	local->basic_rate_set = basic_rate_control;

	for (i = 0; i < WLAN_RATE_COUNT; i++) {
		if (! mib_StaCfg->OpRateSet[i])
			break;
		mask = mvWLAN_rate2mask(mib_StaCfg->OpRateSet[i]);
		if ((basic_rate_control & mask) == mask) {
			mib_StaCfg->OpRateSet[i] |= 0x80;
		} else {
			mib_StaCfg->OpRateSet[i] &= 0x7F;
		}
	}

	return 0;
}


static int mvWLAN_iw_ioctl_siwrate(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_param *rrq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int ret = 0;
	u32 tx_rate_control = 0;

	if (rrq->fixed) {
		switch (rrq->value) {
		case 54000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_54M;
			break;
		case 48000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_48M;
			break;
		case 3600000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_36M;
			break;
		case 2400000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_24M;
			break;
		case 18000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_18M;
			break;
		case 12000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_12M;
			break;
		case 9000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_9M;
			break;
		case 6000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_6M;
			break;
		case 11000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_11M;
			break;
		case 5500000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_5M5;
			break;
		case 2000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_2M;
			break;
		case 1000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_1M;
			break;
		default:
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M | WLAN_RATE_18M |
				WLAN_RATE_24M | WLAN_RATE_36M | WLAN_RATE_48M |
				WLAN_RATE_54M | WLAN_RATE_22M | WLAN_RATE_72M;
			break;
		}
	} else {
		switch (rrq->value) {
		case 54000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5 | 
					WLAN_RATE_11M;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M | WLAN_RATE_12M | 
					WLAN_RATE_18M | WLAN_RATE_24M | 
					WLAN_RATE_36M | WLAN_RATE_48M |
					WLAN_RATE_54M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M | WLAN_RATE_18M |
				WLAN_RATE_24M | WLAN_RATE_36M | WLAN_RATE_48M |
				WLAN_RATE_54M | WLAN_RATE_22M | WLAN_RATE_72M;
			break;
		case 48000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5 | 
					WLAN_RATE_11M;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M | WLAN_RATE_12M | 
					WLAN_RATE_18M | WLAN_RATE_24M | 
					WLAN_RATE_36M | WLAN_RATE_48M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M | WLAN_RATE_18M |
				WLAN_RATE_24M | WLAN_RATE_36M | WLAN_RATE_48M |
				WLAN_RATE_22M;
			break;
		case 36000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M |
					WLAN_RATE_2M | WLAN_RATE_5M5 | 
					WLAN_RATE_11M;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M | WLAN_RATE_12M | 
					WLAN_RATE_18M | WLAN_RATE_24M | 
					WLAN_RATE_36M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M | WLAN_RATE_18M |
				WLAN_RATE_24M | WLAN_RATE_36M | WLAN_RATE_22M;
			break;
		case 24000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5 | 
					WLAN_RATE_11M;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M | WLAN_RATE_12M | 
					WLAN_RATE_18M | WLAN_RATE_24M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M | WLAN_RATE_18M |
				WLAN_RATE_24M | WLAN_RATE_22M;
			break;
		case 18000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5 |
					WLAN_RATE_11M;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M | WLAN_RATE_12M | 
					WLAN_RATE_18M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M | WLAN_RATE_18M;
			break;
		case 12000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5 | 
					WLAN_RATE_11M;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M | WLAN_RATE_12M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M;
			break;
		case 9000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M;
			break;
		case 6000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M;
			break;
		case 11000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
				tx_rate_control = WLAN_RATE_1M | 
					WLAN_RATE_2M | WLAN_RATE_5M5 | 
					WLAN_RATE_11M;
				break;
			}
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				tx_rate_control = WLAN_RATE_6M |
					WLAN_RATE_9M;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M;
			break;
		case 5500000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5;
			break;
		case 2000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M;
			break;
		case 1000000:
			if (local->mib_ApMode == MVWLAN_AP_MODE_G_ONLY) {
				ret = -EINVAL;
				break;
			}
			tx_rate_control = WLAN_RATE_1M;
			break;
		default:
			tx_rate_control = WLAN_RATE_1M | WLAN_RATE_2M |
				WLAN_RATE_5M5 | WLAN_RATE_11M | WLAN_RATE_6M |
				WLAN_RATE_9M | WLAN_RATE_12M | WLAN_RATE_18M |
				WLAN_RATE_24M | WLAN_RATE_36M | WLAN_RATE_48M |
				WLAN_RATE_54M | WLAN_RATE_22M | WLAN_RATE_72M;
			break;
		}
	}

	if (! ret)
		ret = mvWLAN_set_rate(local, tx_rate_control);

	return ret;
}


static int mvWLAN_iw_ioctl_giwrate(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_param *rrq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int i, num, mask;

	for (i = 0, num = 0, mask = 1; i < WLAN_RATE_COUNT; i++, mask <<= 1) {
		if ((local->tx_rate_control & mask) == mask)
			num++;
	}
	if (num > 1)
		rrq->fixed = 0;
	else
		rrq->fixed = 1;

	if (local->iw_mode == IW_MODE_MASTER && local->ap != NULL) {
		/* Use the recorded TX rate of the last sent frame */
		rrq->value = local->ap->last_tx_rate > 0 ?
			local->ap->last_tx_rate * 100000 : 11000000;
		return 0;
	}

	/* XXX, Need the way to detect the current TX rate for station 
	 * device */

	return 0;
}


static int mvWLAN_iw_ioctl_siwsens(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_param *sens, char *extra)
{
	/* TBD */
	return 0;
}


static int mvWLAN_iw_ioctl_giwsens(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_param *sens, char *extra)
{
	/* Get the current AP density */
	/* TBD */
	sens->value = 0;
	sens->fixed = 1;
 
	return 0;
}


/* Deprecated in new wireless extension API */
static int mvWLAN_iw_ioctl_giwaplist(struct net_device *dev,
				     struct iw_request_info *info,
				     struct iw_point *data, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct sockaddr addr[IW_MAX_AP];
	struct iw_quality qual[IW_MAX_AP];

	if (local->iw_mode != IW_MODE_MASTER) {
		printk(KERN_DEBUG "SIOCGIWAPLIST is currently only supported "
		       "in master mode\n");
		data->length = 0;
		return -EOPNOTSUPP;
	}

	data->length = ap_get_sta_qual(local, addr, qual, IW_MAX_AP, 1);

	memcpy(extra, &addr, sizeof(addr[0]) * data->length);
	data->flags = 1; /* has quality information */
	memcpy(extra + sizeof(addr[0]) * data->length, &qual,
	       sizeof(qual[0]) * data->length);

	return 0;
}


static int mvWLAN_iw_ioctl_siwrts(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_param *rts, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_OP_DATA *mib_OpData = 
			&(local->sysConfig->Mib802dot11->OperationTable);
	u32 value;

	if (rts->disabled)
		value = 2347;
	else if (rts->value < 0 || rts->value > 2347)
		return -EINVAL;
	else
		value = rts->value;

	mib_OpData->RtsThresh = value;

	return 0;
}

static int mvWLAN_iw_ioctl_giwrts(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_param *rts, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_OP_DATA *mib_OpData = 
			&(local->sysConfig->Mib802dot11->OperationTable);

	rts->value = mib_OpData->RtsThresh;
	rts->disabled = (rts->value == 2347);
	rts->fixed = 1;

	return 0;
}


static int mvWLAN_iw_ioctl_siwfrag(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_param *rts, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_OP_DATA *mib_OpData = 
			&(local->sysConfig->Mib802dot11->OperationTable);
	u32 value;

	if (rts->disabled)
		value = 2346;
	else if (rts->value < 256 || rts->value > 2346)
		return -EINVAL;
	else
		value = (rts->value & ~0x1); /* even numbers only */

	mib_OpData->FragThresh = value;

	return 0;
}


static int mvWLAN_iw_ioctl_giwfrag(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_param *rts, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_OP_DATA *mib_OpData = 
			&(local->sysConfig->Mib802dot11->OperationTable);

	rts->value = mib_OpData->FragThresh;
	rts->disabled = (rts->value == 2346);
	rts->fixed = 1;

	return 0;
}


#ifndef MVWLAN_NO_STATION_MODES
static int mvWLAN_join_ap(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct mvwlan_join_request req;
	unsigned long flags;
	int i;
	struct mvwlan_scan_result *entry;

	memcpy(req.bssid, local->preferred_ap, ETH_ALEN);
	req.channel = 0;

	spin_lock_irqsave(&local->lock, flags);
	for (i = 0; i < local->last_scan_results_count; i++) {
		if (!local->last_scan_results)
			break;
		entry = &local->last_scan_results[i];
		if (memcmp(local->preferred_ap, entry->bssid, ETH_ALEN) == 0) {
			req.channel = entry->chid;
			break;
		}
	}
	spin_unlock_irqrestore(&local->lock, flags);

	/* XXX, To fill appropriate API here */

	printk(KERN_DEBUG "%s: Trying to join BSSID " MACSTR "\n",
	       dev->name, MAC2STR(local->preferred_ap));

	return 0;
}
#endif /* MVWLAN_NO_STATION_MODES */


static int mvWLAN_iw_ioctl_siwap(struct net_device *dev,
				 struct iw_request_info *info,
				 struct sockaddr *ap_addr, char *extra)
{
#ifdef MVWLAN_NO_STATION_MODES
	return -EOPNOTSUPP;
#else /* MVWLAN_NO_STATION_MODES */
	local_info_t *local = (local_info_t *) dev->priv;

	memcpy(local->preferred_ap, &ap_addr->sa_data, ETH_ALEN);

	if (local->host_roaming == 1 && local->iw_mode == IW_MODE_INFRA) {
		struct mvwlan_scan_request scan_req;
		memset(&scan_req, 0, sizeof(scan_req));
		scan_req.channel_list = (WLAN_CHANNEL_1 | WLAN_CHANNEL_2 |
					 WLAN_CHANNEL_3 | WLAN_CHANNEL_4 |
					 WLAN_CHANNEL_5 | WLAN_CHANNEL_6 |
					 WLAN_CHANNEL_7 | WLAN_CHANNEL_8 |
					 WLAN_CHANNEL_9 | WLAN_CHANNEL_10 |
					 WLAN_CHANNEL_11 | WLAN_CHANNEL_12 |
					 WLAN_CHANNEL_13 | WLAN_CHANNEL_14);
		scan_req.txrate = WLAN_RATE_1M;
		/* XXX, To fill appropriate API here */
	} else if (local->host_roaming == 2 &&
		   local->iw_mode == IW_MODE_INFRA) {
		if (mvWLAN_join_ap(dev))
			return -EINVAL;
	} else {
		printk(KERN_DEBUG "%s: Preferred AP (SIOCSIWAP) is used only "
		       "in Managed mode when host_roaming is enabled\n",
		       dev->name);
	}

	return 0;
#endif /* MVWLAN_NO_STATION_MODES */
}

static int mvWLAN_iw_ioctl_giwap(struct net_device *dev,
				 struct iw_request_info *info,
				 struct sockaddr *ap_addr, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
#ifdef MVWLAN_NO_STATION_MODES
	MIB_OP_DATA *mib_OpData = 
			&(local->sysConfig->Mib802dot11->OperationTable);

	memcpy(&ap_addr->sa_data, &(mib_OpData->StaMacAddr[0]), ETH_ALEN);
	ap_addr->sa_family = ARPHRD_ETHER;
	return 0;
#else /* MVWLAN_NO_STATION_MODES */
	if (dev == local->stadev) {
		memcpy(&ap_addr->sa_data, local->assoc_ap_addr, ETH_ALEN);
		ap_addr->sa_family = ARPHRD_ETHER;
		return 0;
	}

	/* XXX, To fill appropriate API here */

	return 0;
#endif /* MVWLAN_NO_STATION_MODES */
}


static int mvWLAN_iw_ioctl_siwessid(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_point *data, char *ssid)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg_p = 
			&(local->sysConfig->Mib802dot11->StationConfig);

	if (data->flags == 0)
		ssid[0] = '\0'; /* ANY */

	/* Check the size of the string */
	if (data->length > IW_ESSID_MAX_SIZE)
		return -E2BIG;

	/* AP/STA specific configuration */
	memcpy(mib_StaCfg_p->DesiredSsId, ssid, data->length);
	local->essid_len = data->length;

	/* Since we configure one of the parameters used to form
	 * a BSS, we need to restart the MLME. */
	mvWLAN_restart_mlme(dev);

	return 0;
}

static int mvWLAN_iw_ioctl_giwessid(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_point *data, char *essid)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg_p = 
			&(local->sysConfig->Mib802dot11->StationConfig);

	data->flags = 1; /* active */
	data->length = local->essid_len;
	memcpy(essid, mib_StaCfg_p->DesiredSsId, local->essid_len);

	return 0;
}


static int mvWLAN_iw_ioctl_siwnickn(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_point *data, char *nickname)
{
	local_info_t *local = (local_info_t *) dev->priv;

	if (data->flags == 0) {
		local->mib_DeviceName[0] = '\0'; /* NULL */
	} else {
		/* Check the size of the string */
		if (data->length > 16)
			return -E2BIG;
		/* AP/STA specific configuration */
		memcpy(local->mib_DeviceName, nickname, data->length);
	}

	return 0;
}

static int mvWLAN_iw_ioctl_giwnickn(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_point *data, char *nickname)
{
	local_info_t *local = (local_info_t *) dev->priv;

	data->flags = 1; /* active */
	data->length = strlen(local->mib_DeviceName);
	memcpy(nickname, local->mib_DeviceName, data->length);

	return 0;
}


static int mvWLAN_iw_ioctl_siwfreq(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_freq *freq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_PHY_DSSS_TABLE *PhyDSSSTable_p = 
				&(local->sysConfig->Mib802dot11->PhyDSSSTable);

	if (freq->e > 1)
		return -EINVAL;
	if (freq->e == 1) {
		/* freq => chan. */
		int fr = freq->m / 100000;
		int ch;

		if (fr < freq_list[0] || fr <= freq_list[FREQ_COUNT - 1])
			return -EINVAL;
		for (ch = 0; ch < FREQ_COUNT; ch++) {
			if (fr == freq_list[ch]) {
				freq->e = 0;
				freq->m = ch + 1;
				break;
			}
		}
	} else {
		/* use chan. */
		if (freq->m < 1 || freq->m > FREQ_COUNT)
			return -EINVAL;
	}
	/* check for channel mask */
	if ( !(local->channel_mask & (1 << (freq->m - 1))))
		return -EINVAL;

	PhyDSSSTable_p->CurrChan = freq->m;

	/* Since we configure one of the parameters used to form
	 * a BSS, we need to restart the MLME. */
	mvWLAN_restart_mlme(dev);

	return 0;
}


static int mvWLAN_iw_ioctl_giwfreq(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_freq *freq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_PHY_DSSS_TABLE *PhyDSSSTable_p = 
				&(local->sysConfig->Mib802dot11->PhyDSSSTable);

	freq->m = freq_list[PhyDSSSTable_p->CurrChan - 1] * 100000;
	freq->e = 1;

	return 0;
}


static int mvWLAN_iw_ioctl_giwrange(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_point *data, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct iw_range *range = (struct iw_range *) extra;
	u8 rates[WLAN_RATE_COUNT];
	u16 val;
	int i, len;

	data->length = sizeof(struct iw_range);
	memset(range, 0, sizeof(struct iw_range));

#if WIRELESS_EXT > 9
	/* TODO: could fill num_txpower and txpower array with
	 * something; however, there are 128 different values.. */

	range->txpower_capa = IW_TXPOW_DBM;

	if (local->iw_mode == IW_MODE_INFRA || local->iw_mode == IW_MODE_ADHOC)
	{
		range->min_pmp = 1 * 1024;
		range->max_pmp = 65535 * 1024;
		range->min_pmt = 1 * 1024;
		range->max_pmt = 1000 * 1024;
		range->pmp_flags = IW_POWER_PERIOD;
		range->pmt_flags = IW_POWER_TIMEOUT;
		range->pm_capa = IW_POWER_PERIOD | IW_POWER_TIMEOUT |
			IW_POWER_UNICAST_R | IW_POWER_ALL_R;
	}
#endif /* WIRELESS_EXT > 9 */

#if WIRELESS_EXT > 10
	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 13;

	range->retry_capa = IW_RETRY_LIMIT;
	range->retry_flags = IW_RETRY_LIMIT;
	range->min_retry = 0;
	range->max_retry = 255;
#endif /* WIRELESS_EXT > 10 */

	range->num_channels = FREQ_COUNT;

	val = 0;
	for (i = 0; i < FREQ_COUNT; i++) {
		if (local->channel_mask & (1 << i)) {
			range->freq[val].i = i + 1;
			range->freq[val].m = freq_list[i] * 100000;
			range->freq[val].e = 1;
			val++;
		}
		if (val == IW_MAX_FREQUENCIES)
			break;
	}
	range->num_frequency = val;

	range->max_qual.qual = 92; /* 0 .. 92 */
	range->max_qual.level = 154; /* 27 .. 154 */
	range->max_qual.noise = 154; /* 27 .. 154 */
	range->sensitivity = 3;

	range->max_encoding_tokens = MVWLAN_WEP_KEYS;
	range->num_encoding_sizes = 2;
	range->encoding_size[0] = 5;
	range->encoding_size[1] = 13;

	len = mvWLAN_get_datarates(dev, rates);
	range->num_bitrates = 0;
	for (i = 0; i < len; i++) {
		if (range->num_bitrates < IW_MAX_BITRATES) {
			range->bitrate[range->num_bitrates] =
				rates[i] * 500000;
			range->num_bitrates++;
		}
	}
	/* estimated maximum TCP throughput values (bps) */
	range->throughput = 5500000;

	range->min_rts = 0;
	range->max_rts = 2347;
	range->min_frag = 256;
	range->max_frag = 2346;

	return 0;
}


static int mvWLAN_iw_ioctl_siwmode(struct net_device *dev,
				   struct iw_request_info *info,
				   __u32 *mode, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg = 
			&(local->sysConfig->Mib802dot11->StationConfig);

	if (*mode != IW_MODE_ADHOC && *mode != IW_MODE_INFRA &&
	    *mode != IW_MODE_MASTER && *mode != IW_MODE_REPEAT &&
	    *mode != IW_MODE_MONITOR)
		return -EOPNOTSUPP;

#ifdef MVWLAN_NO_STATION_MODES
	if (*mode == IW_MODE_ADHOC || *mode == IW_MODE_INFRA)
		return -EOPNOTSUPP;
#endif /* MVWLAN_NO_STATION_MODES */

	if (*mode == local->iw_mode)
		return 0;

	if (*mode == IW_MODE_MASTER && mib_StaCfg->DesiredSsId[0] == '\0') {
		printk(KERN_WARNING "%s: empty SSID not allowed in Master "
		       "mode\n", dev->name);
		return -EINVAL;
	}

	printk(KERN_DEBUG "%s: operating mode changed %d -> %d\n", 
	       dev->name, local->iw_mode, *mode);
	local->iw_mode = *mode;

	/* Since we configure one of the parameters used to form
	 * a BSS, we need to restart the MLME. */
	if (*mode != IW_MODE_REPEAT)
		mvWLAN_restart_mlme(dev);

	return 0;
}


static int mvWLAN_iw_ioctl_giwmode(struct net_device *dev,
				   struct iw_request_info *info,
				   __u32 *mode, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;

	if (dev == local->stadev) {
		*mode = IW_MODE_INFRA;
		return 0;
	}

	*mode = local->iw_mode;
	return 0;
}


static int mvWLAN_iw_ioctl_siwpower(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_param *wrq, char *extra)
{
#ifdef MVWLAN_NO_STATION_MODES
	return -EOPNOTSUPP;
#else /* MVWLAN_NO_STATION_MODES */
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg = 
			&(local->sysConfig->Mib802dot11->StationConfig);
	int ret;
	u32 value;

	if (local->iw_mode == IW_MODE_MASTER) {
		/* In master mode, we cannot power down */
		return -EOPNOTSUPP;
	}
	if (wrq->disabled) {
		/* Turn off power saving mode */
		value = WLAN_POWER_MODE_ACTIVE;
		ret = local->func->set_cmd(dev, 
					   APCMD_SET_POWER_MGMT_MODE,
					   0,
					   (void *) &value, 
					   sizeof(u32));
		if (! ret) {
			printk(KERN_DEBUG 
			       "%s: Turning radio on: %s\n",
			       dev->name, ret ? "failed" : "OK");
		}
		mib_StaCfg->PwrMgtMode = PWR_MODE_ACTIVE;
		return 0;
	}
	switch (wrq->flags & IW_POWER_MODE) {
	case IW_POWER_UNICAST_R:
		return -EOPNOTSUPP;
	case IW_POWER_ALL_R:
		break;
	case IW_POWER_ON:
		break;
	default:
		return -EINVAL;
	}

	if (wrq->flags & IW_POWER_TIMEOUT) {
		/* Need extra software effort. Not implemented yet */
		return -EOPNOTSUPP;
	}
	if (wrq->flags & IW_POWER_PERIOD) {
		/* Need extra software effort. Not implemented yet */
		return -EOPNOTSUPP;
	}

	return 0;
#endif /* MVWLAN_NO_STATION_MODES */
}


static int mvWLAN_iw_ioctl_giwpower(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_param *rrq, char *extra)
{
#ifdef MVWLAN_NO_STATION_MODES
	return -EOPNOTSUPP;
#else /* MVWLAN_NO_STATION_MODES */
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg = 
			&(local->sysConfig->Mib802dot11->StationConfig);

	if (mib_StaCfg->PwrMgtMode == PWR_MODE_ACTIVE) {
		rrq->disabled = 1;
		return 0;
	}

	rrq->disabled = 0;

	if ((rrq->flags & IW_POWER_TYPE) == IW_POWER_TIMEOUT) {
		rrq->flags = IW_POWER_TIMEOUT;
		/* Need extra software effort. Not implemented yet 
		 * so we return zero. */
		rrq->value = 0 * 1024;
	} else {
		rrq->flags = IW_POWER_PERIOD;
		/* Need extra software effort. Not implemented yet 
		 * so we return zero. */
		rrq->value = 0 * 1024;
	}

	rrq->flags |= IW_POWER_ALL_R;

	return 0;
#endif /* MVWLAN_NO_STATION_MODES */
}


#if WIRELESS_EXT > 10
static int mvWLAN_iw_ioctl_siwretry(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_param *rrq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_OP_DATA *mib_OpData = 
			&(local->sysConfig->Mib802dot11->OperationTable);
	int value, ret = 0;

	if (rrq->disabled)
		return -EINVAL;

	if (rrq->flags == IW_RETRY_LIMIT) {
		if (rrq->value < 0) {
			return -EINVAL;
		} else {
			value = rrq->value;
			/* There is no way to tell whether short or long  
			 * retry is being changed, here we change both. */
			ret = local->func->set_cmd(dev, 
						APCMD_SET_SHORT_RETRY_LIMIT,
						0, (void *) &value, 
						sizeof(u32));
			if (ret) {
				printk(KERN_DEBUG "%s: short retry count "
				       "setting to %d failed\n",
				       dev->name, rrq->value);
				return -EOPNOTSUPP;
			}
			ret = local->func->set_cmd(dev, 
						APCMD_SET_LONG_RETRY_LIMIT,
						0, (void *) &value, 
						sizeof(u32));
			if (ret) {
				printk(KERN_DEBUG "%s: long retry count "
				       "setting to %d failed\n",
				       dev->name, rrq->value);
				return -EOPNOTSUPP;
			}
			mib_OpData->ShortRetryLim = rrq->value;
			mib_OpData->LongRetryLim = rrq->value;
		}
		return 0;
	}
	if (rrq->flags == IW_RETRY_LIFETIME) {
		if (rrq->value < 0) {
			return -EINVAL;
		} else {
			ret = local->func->set_cmd(dev, 
					    APCMD_SET_MAX_TX_MSDU_LIFETIME,
					    0, (void *) &value, 
					    sizeof(u32));
			if (ret) {
				printk(KERN_DEBUG "%s: max tx MSDU lifetime "
				       "setting to %d failed\n",
				       dev->name, rrq->value);
				return -EOPNOTSUPP;
			}
			mib_OpData->MaxTxMsduLife = rrq->value;
		}
	}
	return 0;
}


static int mvWLAN_iw_ioctl_giwretry(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_param *rrq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_OP_DATA *mib_OpData = 
			&(local->sysConfig->Mib802dot11->OperationTable);

	rrq->disabled = 0;

	if ((rrq->flags & IW_RETRY_TYPE) == IW_RETRY_LIFETIME) {
		rrq->flags = IW_RETRY_LIFETIME;
		rrq->value = mib_OpData->MaxTxMsduLife;
	} else {
		if ((rrq->flags & IW_RETRY_MAX)) {
			rrq->flags = IW_RETRY_LIMIT | IW_RETRY_MAX;
			rrq->value = mib_OpData->LongRetryLim;
		} else {
			rrq->flags = IW_RETRY_LIMIT;
			rrq->value = mib_OpData->ShortRetryLim;
			if (mib_OpData->ShortRetryLim != 
				mib_OpData->LongRetryLim)
				rrq->flags |= IW_RETRY_MIN;
		}
	}
	return 0;
}
#endif /* WIRELESS_EXT > 10 */


#if WIRELESS_EXT > 9

/* Note! This TX power controlling is experimental and should not be used in
 * production use. It just sets raw power register and does not use any kind of
 * feedback information from the measured TX power (CR58). This is now
 * commented out to make sure that it is not used by accident. TX power
 * configuration will be enabled again after proper algorithm using feedback
 * has been implemented. */

static int mvWLAN_iw_ioctl_siwtxpow(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_param *rrq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg_p =
			&(local->sysConfig->Mib802dot11->StationConfig);
	char *tmp;
	MIB_PHY_TX_POWER_TABLE *mib_PhyTxPwrData_p = 
				&(local->sysConfig->Mib802dot11->PhyPowerTable);
	int value, ret = 0;

	if (rrq->disabled) {
		if (local->iw_mode == IW_MODE_MASTER) {
			/* In master mode, we cannot power down */
			return -EOPNOTSUPP;
		}
		if (! mib_StaCfg_p->PwrMgtMode) {
			value = WLAN_POWER_MODE_SAVE;
			/* use all standby and sleep modes */
			ret = local->func->set_cmd(dev, 
						   APCMD_SET_POWER_MGMT_MODE,
						   0, (void *) &value,
						   sizeof(u32));
			printk(KERN_DEBUG "%s: Turning radio off: %s\n",
			       dev->name, ret ? "failed" : "OK");
			if (! ret) {
				mib_StaCfg_p->PwrMgtMode = 1;
			}
		}
		return ret;
	}

	if (!rrq->fixed) {
		return -EOPNOTSUPP;
	}

	if (mib_StaCfg_p->PwrMgtMode) {
		/* disable all standby and sleep modes */
		value = WLAN_POWER_MODE_ACTIVE;
		/* use all standby and sleep modes */
		ret = local->func->set_cmd(dev, 
					   APCMD_SET_POWER_MGMT_MODE,
					   0, (void *) &value, 
					   sizeof(u32));
		printk(KERN_DEBUG "%s: Turning radio on: %s\n",
		       dev->name, ret ? "failed" : "OK");
		if (! ret)
			mib_StaCfg_p->PwrMgtMode = 0;
	}

	if (rrq->flags == IW_TXPOW_DBM)
		tmp = "dBm";
	else if (rrq->flags == IW_TXPOW_MWATT)
		tmp = "mW";
	else
		tmp = "UNKNOWN";
	printk(KERN_DEBUG "Setting TX power to %d %s\n", rrq->value, tmp);

	if (rrq->flags != IW_TXPOW_DBM) {
		printk("SIOCSIWTXPOW with mW is not supported; use dBm\n");
		return -EOPNOTSUPP;
	}

	switch (mib_PhyTxPwrData_p->CurrTxPwrLevel) {
	case 1:
		mib_PhyTxPwrData_p->TxPwrLevel1 = (u16) value;
		break;
	case 2:
		mib_PhyTxPwrData_p->TxPwrLevel2 = (u16) value;
		break;
	case 3:
		mib_PhyTxPwrData_p->TxPwrLevel3 = (u16) value;
		break;
	case 4:
		mib_PhyTxPwrData_p->TxPwrLevel4 = (u16) value;
		break;
	case 5:
		mib_PhyTxPwrData_p->TxPwrLevel5 = (u16) value;
		break;
	case 6:
		mib_PhyTxPwrData_p->TxPwrLevel6 = (u16) value;
		break;
	case 7:
		mib_PhyTxPwrData_p->TxPwrLevel7 = (u16) value;
		break;
	case 8:
		mib_PhyTxPwrData_p->TxPwrLevel8 = (u16) value;
		break;
	}

	value = rrq->value;
	ret = local->func->set_cmd(dev, APCMD_SET_TX_POWER, 0,
				   (void *) &value, sizeof(u32));

	return ret;
}

static int mvWLAN_iw_ioctl_giwtxpow(struct net_device *dev,
				    struct iw_request_info *info,
				    struct iw_param *rrq, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_STA_CFG *mib_StaCfg_p =
			&(local->sysConfig->Mib802dot11->StationConfig);
	MIB_PHY_TX_POWER_TABLE *mib_PhyTxPwrData_p = 
				&(local->sysConfig->Mib802dot11->PhyPowerTable);

	rrq->flags = IW_TXPOW_DBM;
	rrq->disabled = 0;
	rrq->fixed = 0;

	if (mib_StaCfg_p->PwrMgtMode) {
		switch (mib_PhyTxPwrData_p->CurrTxPwrLevel) {
		case 1:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel1;
			break;
		case 2:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel2;
			break;
		case 3:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel3;
			break;
		case 4:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel4;
			break;
		case 5:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel5;
			break;
		case 6:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel6;
			break;
		case 7:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel7;
			break;
		case 8:
			rrq->value = mib_PhyTxPwrData_p->TxPwrLevel8;
			break;
		}
		rrq->fixed = 1;
	} else {
		rrq->value = 0;
		rrq->disabled = 1;
	}

	return 0;
}
#endif /* WIRELESS_EXT > 9 */


#if WIRELESS_EXT > 13
static int mvWLAN_iw_ioctl_siwscan(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_point *data, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
#ifndef MVWLAN_NO_STATION_MODES
	struct mvwlan_scan_request scan_req;
#endif /* !MVWLAN_NO_STATION_MODES */

	if (local->iw_mode == IW_MODE_MASTER) {
		/* In master mode, we just return the results of our local
		 * tables, so we don't need to start anything...
		 * Jean II */
		data->length = 0;
		return 0;
	}

#ifdef MVWLAN_NO_STATION_MODES
	return -EOPNOTSUPP;
#else /* MVWLAN_NO_STATION_MODES */

	memset(&scan_req, 0, sizeof(scan_req));
	scan_req.channel_list = __constant_cpu_to_le16(0x3fff);
	scan_req.txrate = __constant_cpu_to_le16(WLAN_RATE_1M);

	/* XXX, TBD */
	return 0;
#endif /* MVWLAN_NO_STATION_MODES */
}


#ifndef MVWLAN_NO_STATION_MODES
/* Translate scan data returned from the card to a card independant
 * format that the Wireless Tools will understand - Jean II */
static inline int mvWLAN_translate_scan(struct net_device *dev, char *buffer,
					char *scan, int scan_len)
{
	struct mvwlan_scan_result *atom;
	int left, i;
	struct iw_event iwe;
	char *current_ev = buffer;
	char *end_buf = buffer + IW_SCAN_MAX_DATA;
	char *current_val;

	left = scan_len;

	if (left % sizeof(*atom) != 0) {
		printk(KERN_DEBUG "%s: invalid total scan result length %d "
		       "(entry length %d)\n",
		       dev->name, left, (int) sizeof(*atom));
		return -EINVAL;
	}

	atom = (struct mvwlan_scan_result *) scan;
	while (left > 0) {
		u16 capabilities;

		/* First entry *MUST* be the AP MAC address */
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
		memcpy(iwe.u.ap_addr.sa_data, atom->bssid, ETH_ALEN);
		/* FIX:
		 * I do not know how this is possible, but iwe_stream_add_event
		 * seems to re-order memcpy execution so that len is set only
		 * after copying.. Pre-setting len here "fixes" this, but real
		 * problems should be solved (after which these iwe.len
		 * settings could be removed from this function). */
		iwe.len = IW_EV_ADDR_LEN;
		current_ev = iwe_stream_add_event(current_ev, end_buf, &iwe,
						  IW_EV_ADDR_LEN);

		/* Other entries will be displayed in the order we give them */

		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.length = le16_to_cpu(atom->ssid_len);
		if (iwe.u.data.length > 32)
			iwe.u.data.length = 32;
		iwe.u.data.flags = 1;
		iwe.len = IW_EV_POINT_LEN + iwe.u.data.length;
		current_ev = iwe_stream_add_point(current_ev, end_buf, &iwe,
						  atom->ssid);

		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWMODE;
		capabilities = le16_to_cpu(atom->capability);
		if (capabilities & (WLAN_CAPABILITY_ESS |
				    WLAN_CAPABILITY_IBSS)) {
			if (capabilities & WLAN_CAPABILITY_ESS)
				iwe.u.mode = IW_MODE_MASTER;
			else
				iwe.u.mode = IW_MODE_ADHOC;
			iwe.len = IW_EV_UINT_LEN;
			current_ev = iwe_stream_add_event(current_ev, end_buf,
							  &iwe,
							  IW_EV_UINT_LEN);
		}

		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWFREQ;
		iwe.u.freq.m = freq_list[le16_to_cpu(atom->chid) - 1] * 100000;
		iwe.u.freq.e = 1;
		iwe.len = IW_EV_FREQ_LEN;
		current_ev = iwe_stream_add_event(current_ev, end_buf, &iwe,
						  IW_EV_FREQ_LEN);

		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = IWEVQUAL;
		iwe.u.qual.level = le16_to_cpu(atom->sl);
		iwe.u.qual.noise = le16_to_cpu(atom->anl);
		iwe.len = IW_EV_QUAL_LEN;
		current_ev = iwe_stream_add_event(current_ev, end_buf, &iwe,
						  IW_EV_QUAL_LEN);

		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWENCODE;
		if (capabilities & WLAN_CAPABILITY_PRIVACY)
			iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		else
			iwe.u.data.flags = IW_ENCODE_DISABLED;
		iwe.u.data.length = 0;
		iwe.len = IW_EV_POINT_LEN + iwe.u.data.length;
		current_ev = iwe_stream_add_point(
			current_ev, end_buf, &iwe,
			atom->ssid /* memcpy 0 bytes */);

		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWRATE;
		current_val = current_ev + IW_EV_LCP_LEN;
		for (i = 0; i < sizeof(atom->sup_rates); i++) {
			if (atom->sup_rates[i] == 0)
				break;
			/* Bit rate given in 500 kb/s units (+ 0x80) */
			iwe.u.bitrate.value =
				((atom->sup_rates[i] & 0x7f) * 500000);
			current_val = iwe_stream_add_value(
				current_ev, current_val, end_buf, &iwe,
				IW_EV_PARAM_LEN);
		}
		/* Check if we added any event */
		if ((current_val - current_ev) > IW_EV_LCP_LEN)
			current_ev = current_val;

		/* Could add beacon_interval and rate (of the received
		 * ProbeResp) to scan results. */

		atom++;
		left -= sizeof(*atom);
	}

	return current_ev - buffer;
}
#endif /* MVWLAN_NO_STATION_MODES */


static inline int mvWLAN_ioctl_giwscan_sta(struct net_device *dev,
					   struct iw_request_info *info,
					   struct iw_point *data, char *extra)
{
#ifdef MVWLAN_NO_STATION_MODES
	return -EOPNOTSUPP;
#else /* MVWLAN_NO_STATION_MODES */
	local_info_t *local = (local_info_t *) dev->priv;

	/* Wait until the scan is finished. We can probably do better
	 * than that - Jean II */
	if (local->scan_timestamp &&
	    time_before(jiffies, local->scan_timestamp + 3 * HZ)) {
		/* Important note : we don't want to block the caller
		 * until results are ready for various reasons.
		 * First, managing wait queues is complex and racy
		 * (there may be multiple simultaneous callers).
		 * Second, we grab some rtnetlink lock before comming
		 * here (in dev_ioctl()).
		 * Third, the caller can wait on the Wireless Event
		 * - Jean II */
		return -EAGAIN;
	}
	local->scan_timestamp = 0;

	/* XXX, TBD */
	return 0;
#endif /* MVWLAN_NO_STATION_MODES */
}


/* Translate our list of Access Points & Stations to a card independant
 * format that the Wireless Tools will understand - Jean II */
static int mvWLAN_ap_translate_scan(struct net_device *dev, char *buffer)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct ap_data *ap = local->ap;
	struct list_head *ptr;
	struct iw_event iwe;
	char *current_ev = buffer;
	char *end_buf = buffer + IW_SCAN_MAX_DATA;
#if !defined(MVWLAN_NO_KERNEL_IEEE80211_MGMT) && (WIRELESS_EXT > 14)
	char buf[64];
#endif

	spin_lock_bh(&ap->sta_table_lock);

	for (ptr = ap->sta_list.next; ptr != NULL && ptr != &ap->sta_list;
	     ptr = ptr->next) {
		struct sta_info *sta = (struct sta_info *) ptr;

		/* First entry *MUST* be the AP MAC address */
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
		memcpy(iwe.u.ap_addr.sa_data, sta->addr, ETH_ALEN);
		iwe.len = IW_EV_ADDR_LEN;
		current_ev = iwe_stream_add_event(current_ev, end_buf, &iwe,
						  IW_EV_ADDR_LEN);

		/* Use the mode to indicate if it's a station or
		 * an Access Point */
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWMODE;
		if (sta->ap)
			iwe.u.mode = IW_MODE_MASTER;
		else
			iwe.u.mode = IW_MODE_INFRA;
		iwe.len = IW_EV_UINT_LEN;
		current_ev = iwe_stream_add_event(current_ev, end_buf, &iwe,
						  IW_EV_UINT_LEN);

		/* Some quality */
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = IWEVQUAL;
		if (sta->last_rx_silence == 0)
			iwe.u.qual.qual = sta->last_rx_signal < 27 ?
				0 : (sta->last_rx_signal - 27) * 92 / 127;
		else
			iwe.u.qual.qual = sta->last_rx_signal -
				sta->last_rx_silence - 35;
		iwe.u.qual.level = sta->last_rx_signal;
		iwe.u.qual.noise = sta->last_rx_silence;
		iwe.u.qual.updated = sta->last_rx_updated;
		iwe.len = IW_EV_QUAL_LEN;
		current_ev = iwe_stream_add_event(current_ev, end_buf, &iwe,
						  IW_EV_QUAL_LEN);

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
		if (sta->ap) {
			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = SIOCGIWESSID;
			iwe.u.data.length = sta->u.ap.ssid_len;
			iwe.u.data.flags = 1;
			current_ev = iwe_stream_add_point(current_ev, end_buf,
							  &iwe,
							  sta->u.ap.ssid);

			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = SIOCGIWENCODE;
			if (sta->capability & WLAN_CAPABILITY_PRIVACY)
				iwe.u.data.flags =
					IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
			else
				iwe.u.data.flags = IW_ENCODE_DISABLED;
			current_ev = iwe_stream_add_point(current_ev, end_buf,
							  &iwe,
							  sta->u.ap.ssid
							  /* 0 byte memcpy */);

			if (sta->u.ap.channel > 0 &&
			    sta->u.ap.channel <= FREQ_COUNT) {
				memset(&iwe, 0, sizeof(iwe));
				iwe.cmd = SIOCGIWFREQ;
				iwe.u.freq.m = freq_list[sta->u.ap.channel - 1]
					* 100000;
				iwe.u.freq.e = 1;
				current_ev = iwe_stream_add_event(
					current_ev, end_buf, &iwe,
					IW_EV_FREQ_LEN);
			}

#if WIRELESS_EXT > 14
			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			sprintf(buf, "beacon_interval=%d",
				sta->listen_interval);
			iwe.u.data.length = strlen(buf);
			current_ev = iwe_stream_add_point(current_ev, end_buf,
							  &iwe, buf);
#endif /* WIRELESS_EXT > 14 */
		}
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

		sta->last_rx_updated = 0;

		/* To be continued, we should make good use of IWEVCUSTOM */
	}

	spin_unlock_bh(&ap->sta_table_lock);

	return current_ev - buffer;
}


static int mvWLAN_iw_ioctl_giwscan(struct net_device *dev,
				   struct iw_request_info *info,
				   struct iw_point *data, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int res;

	if (local->iw_mode == IW_MODE_MASTER) {
		/* In MASTER mode, it doesn't make sense to go around
		 * scanning the frequencies and make the stations we serve
		 * wait when what the user is really interested about is the
		 * list of stations and access points we are talking to.
		 * So, just extract results from our cache...
		 * Jean II */

		/* Translate to WE format */
		res = mvWLAN_ap_translate_scan(dev, extra);
		if (res >= 0) {
			printk(KERN_DEBUG "Scan result translation succeeded "
			       "(length=%d)\n", res);
			data->length = res;
			return 0;
		} else {
			printk(KERN_DEBUG
			       "Scan result translation failed (res=%d)\n",
			       res);
			data->length = 0;
			return res;
		}
	} else {
		/* Station mode */
		return mvWLAN_ioctl_giwscan_sta(dev, info, data, extra);
	}
}
#endif /* WIRELESS_EXT > 13 */


static const struct iw_priv_args mvwlan_iw_ioctl_priv[] = {
	{ MVWLAN_IOCTL_MONITOR,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "monitor" },
	{ MVWLAN_IOCTL_RESET,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "reset" },
	{ MVWLAN_IOCTL_MACCMD,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "maccmd" },
#ifdef MVWLAN_USE_WE_TYPE_ADDR
	{ MVWLAN_IOCTL_WDS_ADD,
	  IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "wds_add" },
	{ MVWLAN_IOCTL_WDS_DEL,
	  IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "wds_del" },
	{ MVWLAN_IOCTL_ADDMAC,
	  IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "addmac" },
	{ MVWLAN_IOCTL_DELMAC,
	  IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "delmac" },
	{ MVWLAN_IOCTL_KICKMAC,
	  IW_PRIV_TYPE_ADDR | IW_PRIV_SIZE_FIXED | 1, 0, "kickmac" },
#else /* MVWLAN_USE_WE_TYPE_ADDR */
	{ MVWLAN_IOCTL_WDS_ADD,
	  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 18, 0, "wds_add" },
	{ MVWLAN_IOCTL_WDS_DEL,
	  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 18, 0, "wds_del" },
	{ MVWLAN_IOCTL_ADDMAC,
	  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 18, 0, "addmac" },
	{ MVWLAN_IOCTL_DELMAC,
	  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 18, 0, "delmac" },
	{ MVWLAN_IOCTL_KICKMAC,
	  IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | 18, 0, "kickmac" },
#endif /* MVWLAN_USE_WE_TYPE_ADDR */
	/* --- raw access to sub-ioctls --- */
	{ MVWLAN_IOCTL_SET_PARAM,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "set_param" },
#if WIRELESS_EXT >= 12
	{ MVWLAN_IOCTL_GET_PARAM,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "get_param" },
#ifdef MVWLAN_USE_WE_SUB_IOCTLS
	/* --- sub-ioctls handlers --- */
	{ MVWLAN_IOCTL_SET_PARAM,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "" },
	{ MVWLAN_IOCTL_GET_PARAM,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "" },
	/* --- sub-ioctls definitions --- */
	{ MVWLAN_PARAM_APMODE,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "apmode" },
	{ MVWLAN_PARAM_APMODE,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getapmode" },
	{ MVWLAN_PARAM_BEACON_INT,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "beacon_int" },
	{ MVWLAN_PARAM_BEACON_INT,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getbeacon_int" },
	{ MVWLAN_PARAM_DTIM_PERIOD,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "dtim_period" },
	{ MVWLAN_PARAM_DTIM_PERIOD,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getdtim_period" },
	{ MVWLAN_PARAM_TXPOWER,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "txpower" },
	{ MVWLAN_PARAM_TXPOWER,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getxpower" },
	{ MVWLAN_PARAM_DUMP,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "dump" },
	{ MVWLAN_PARAM_DUMP,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getdump" },
	{ MVWLAN_PARAM_OTHER_AP_POLICY,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "other_ap_policy" },
	{ MVWLAN_PARAM_OTHER_AP_POLICY,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getother_ap_pol" },
	{ MVWLAN_PARAM_AP_MAX_INACTIVITY,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "max_inactivity" },
	{ MVWLAN_PARAM_AP_MAX_INACTIVITY,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getmax_inactivi" },
	{ MVWLAN_PARAM_AP_BRIDGE_PACKETS,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "bridge_packets" },
	{ MVWLAN_PARAM_AP_BRIDGE_PACKETS,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getbridge_packe" },
	{ MVWLAN_PARAM_AP_NULLFUNC_ACK,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "nullfunc_ack" },
	{ MVWLAN_PARAM_AP_NULLFUNC_ACK,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getnullfunc_ack" },
	{ MVWLAN_PARAM_MAX_WDS,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "max_wds" },
	{ MVWLAN_PARAM_MAX_WDS,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getmax_wds" },
	{ MVWLAN_PARAM_AP_AUTOM_AP_WDS,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "autom_ap_wds" },
	{ MVWLAN_PARAM_AP_AUTOM_AP_WDS,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getautom_ap_wds" },
	{ MVWLAN_PARAM_AUTH_MODE,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "ap_auth_mode" },
	{ MVWLAN_PARAM_AUTH_MODE,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getap_auth_mode" },
	{ MVWLAN_PARAM_HOST_ENCRYPT,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "host_encrypt" },
	{ MVWLAN_PARAM_HOST_ENCRYPT,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "gethost_encrypt" },
	{ MVWLAN_PARAM_HOST_DECRYPT,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "host_decrypt" },
	{ MVWLAN_PARAM_HOST_DECRYPT,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "gethost_decrypt" },
#ifndef MVWLAN_NO_STATION_MODES
	{ MVWLAN_PARAM_HOST_ROAMING,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "host_roaming" },
	{ MVWLAN_PARAM_HOST_ROAMING,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "gethost_roaming" },
#endif /* MVWLAN_NO_STATION_MODES */
	{ MVWLAN_PARAM_IEEE_802_1X,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "ieee_802_1x" },
	{ MVWLAN_PARAM_IEEE_802_1X,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getieee_802_1x" },
	{ MVWLAN_PARAM_ANTSEL_TX,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "antsel_tx" },
	{ MVWLAN_PARAM_ANTSEL_TX,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getantsel_tx" },
	{ MVWLAN_PARAM_ANTSEL_RX,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "antsel_rx" },
	{ MVWLAN_PARAM_ANTSEL_RX,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getantsel_rx" },
	{ MVWLAN_PARAM_WDS_TYPE,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wds_type" },
	{ MVWLAN_PARAM_WDS_TYPE,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getwds_type" },
	{ MVWLAN_PARAM_HOSTSCAN,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "hostscan" },
	{ MVWLAN_PARAM_HOSTSCAN,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "gethostscan" },
	{ MVWLAN_PARAM_AP_SCAN,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "ap_scan" },
	{ MVWLAN_PARAM_AP_SCAN,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getap_scan" },
	{ MVWLAN_PARAM_ENH_SEC,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "enh_sec" },
	{ MVWLAN_PARAM_ENH_SEC,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getenh_sec" },
	{ MVWLAN_PARAM_BASIC_RATES,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "basic_rates" },
	{ MVWLAN_PARAM_BASIC_RATES,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getbasic_rates" },
	{ MVWLAN_PARAM_OPER_RATES,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "oper_rates" },
	{ MVWLAN_PARAM_OPER_RATES,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "getoper_rates" },
	{ MVWLAN_PARAM_HOSTAPD,
	  IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "hostapd" },
	{ MVWLAN_PARAM_HOSTAPD,
	  0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "gethostapd" },
#endif /* MVWLAN_USE_WE_SUB_IOCTLS */
#endif /* WIRELESS_EXT >= 12 */
};


#if WIRELESS_EXT <= 12
static int mvWLAN_iw_ioctl_giwpriv(struct net_device *dev, 
				   struct iw_point *data)
{

	if (!data->pointer ||
	    verify_area(VERIFY_WRITE, data->pointer, 
		        sizeof(mvwlan_iw_ioctl_priv)))
		return -EINVAL;

	data->length = sizeof(mvwlan_iw_ioctl_priv) / 
		       sizeof(mvwlan_iw_ioctl_priv[0]);
	if (copy_to_user(data->pointer, mvwlan_iw_ioctl_priv, 
			 sizeof(mvwlan_iw_ioctl_priv)))
		return -EINVAL;
	return 0;
}
#endif /* WIRELESS_EXT <= 12 */


static int mvWLAN_ioctl_priv_set_param(struct net_device *dev,
				       struct iw_request_info *info,
				       void *wrqu, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int *i = (int *) extra;
	int param = *i;
	int value = *(i + 1);
	int ret = 0;

	switch (param) {
	case MVWLAN_PARAM_APMODE:
		if ((value < MVWLAN_AP_MODE_B_ONLY) || 
		    (value > MVWLAN_AP_MODE_MIXED)) {
			ret = -EINVAL;
			break;
		}
		if (local->func->set_cmd(dev, APCMD_SET_AP_MODE, 0, &value,
					 sizeof(value))) {
			ret = -EOPNOTSUPP;
			break;
		}
		local->mib_ApMode = value;
		break;

	case MVWLAN_PARAM_BEACON_INT:
		if (local->func->set_cmd(dev, APCMD_SET_BEACON_PERIOD, 0,
					 &value, sizeof(value))) {
			ret = -EINVAL;
			break;
		}
		local->sysConfig->Mib802dot11->StationConfig.BcnPeriod = value;
		/* Need to reset hw to satisfy time synchronation function */
		local->func->hw_reset(dev);
		break;

	case MVWLAN_PARAM_DTIM_PERIOD:
		if (value < 0 || value > 65535) {
			ret = -EINVAL;
			break;
		}
		local->sysConfig->Mib802dot11->StationConfig.DtimPeriod = value;
		/* Need to reset hw to satisfy time synchronation function */
		local->func->hw_reset(dev);
		break;

	case MVWLAN_PARAM_TXPOWER:
		if (local->func->set_cmd(dev, APCMD_SET_TX_POWER, 0,
					 &value, sizeof(value)))
			ret = -EOPNOTSUPP;
		break;

	case MVWLAN_PARAM_DUMP:
		local->frame_dump = value;
		break;

	case MVWLAN_PARAM_OTHER_AP_POLICY:
		if (value < 0 || value > 3) {
			ret = -EINVAL;
			break;
		}
		if (local->ap != NULL)
			local->ap->ap_policy = value;
		break;

	case MVWLAN_PARAM_AP_MAX_INACTIVITY:
		if (value < 0 || value > 7 * 24 * 60 * 60) {
			ret = -EINVAL;
			break;
		}
		if (local->ap != NULL)
			local->ap->max_inactivity = value * HZ;
		break;

	case MVWLAN_PARAM_AP_BRIDGE_PACKETS:
		if (local->ap != NULL)
			local->ap->bridge_packets = value;
		break;

	case MVWLAN_PARAM_AP_NULLFUNC_ACK:
		if (local->ap != NULL)
			local->ap->nullfunc_ack = value;
		break;

	case MVWLAN_PARAM_MAX_WDS:
		local->wds_max_connections = value;
		break;

	case MVWLAN_PARAM_AP_AUTOM_AP_WDS:
		if (local->ap != NULL) {
			if (!local->ap->autom_ap_wds && value) {
				/* add WDS link to all APs in STA table */
				mvWLAN_add_wds_links(local);
			}
			local->ap->autom_ap_wds = value;
		}
		break;

	case MVWLAN_PARAM_AUTH_MODE:
		local->auth_mode = value;
		break;

	case MVWLAN_PARAM_HOST_ENCRYPT:
		local->host_encrypt = value;
		if (mvWLAN_set_encryption(local))
			ret = -EINVAL;
		break;

	case MVWLAN_PARAM_HOST_DECRYPT:
		local->host_decrypt = value;
		if (mvWLAN_set_encryption(local))
			ret = -EINVAL;
		break;

#ifndef MVWLAN_NO_STATION_MODES
	case MVWLAN_PARAM_HOST_ROAMING:
		if (value < 0 || value > 2) {
			ret = -EINVAL;
			break;
		}
		local->host_roaming = value;
		if (mvWLAN_set_roaming(local))
			ret = -EINVAL;
		break;
#endif /* MVWLAN_NO_STATION_MODES */

	case MVWLAN_PARAM_IEEE_802_1X:
		local->ieee_802_1x = value;
		break;

	case MVWLAN_PARAM_ANTSEL_TX:
		if (value < 0 || value > MVWLAN_ANTSEL_HIGH) {
			ret = -EINVAL;
			break;
		}
		local->mib_txAntenna = value;
		ret = mvWLAN_set_antsel(local, MVWLAN_TX_ANTENNA, value);
		break;

	case MVWLAN_PARAM_ANTSEL_RX:
		if (value < 0 || value > MVWLAN_ANTSEL_HIGH) {
			ret = -EINVAL;
			break;
		}
		local->mib_rxAntenna = value;
		ret = mvWLAN_set_antsel(local, MVWLAN_RX_ANTENNA, value);
		break;

	case MVWLAN_PARAM_WDS_TYPE:
		local->wds_type = value;
		break;

	case MVWLAN_PARAM_HOSTSCAN:
		ret = mvWLAN_hostscan(local, value);
		break;

	case MVWLAN_PARAM_AP_SCAN:
		local->passive_scan_interval = value;
		if (timer_pending(&local->passive_scan_timer))
			del_timer(&local->passive_scan_timer);
		if (value > 0) {
			local->passive_scan_timer.expires = jiffies +
				local->passive_scan_interval * HZ;
			add_timer(&local->passive_scan_timer);
		}
		break;

	case MVWLAN_PARAM_ENH_SEC:
		if (value < 0 || value > 1) {
			ret = -EINVAL;
			break;
		}
		if (local->func->set_cmd(dev, APCMD_SET_SSID_HIDDEN, 0,
					 &value, sizeof(value))) {
			ret = -EOPNOTSUPP;
			break;
		}
		if (value)
			local->enh_sec |= MVWLAN_ENHSEC_BCAST_SSID;
		else
			local->enh_sec &= ~MVWLAN_ENHSEC_BCAST_SSID;
		break;

	case MVWLAN_PARAM_BASIC_RATES:
		if ((value & local->tx_rate_control) != value || value == 0) {
			printk(KERN_INFO "%s: invalid basic rate set - basic "
			       "rates must be in supported rate set\n",
			       dev->name);
			ret = -EINVAL;
			break;
		}
		ret = mvWLAN_set_basic_rate(local, value);
		break;

	case MVWLAN_PARAM_OPER_RATES:
		ret = mvWLAN_set_rate(local, value);
		break;

	case MVWLAN_PARAM_HOSTAPD:
		if (value)
			ret = mvWLAN_enable_hostapd(local, 1);
		else
			ret = mvWLAN_disable_hostapd(local, 1);
		break;

	default:
		printk(KERN_DEBUG "%s: unknown param %d\n",
		       dev->name, param);
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}


#if WIRELESS_EXT >= 12
static int mvWLAN_ioctl_priv_get_param(struct net_device *dev,
				       struct iw_request_info *info,
				       void *wrqu, char *extra)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int *param = (int *) extra;
	int ret = 0;
	u32 value;

	switch (*param) {
	case MVWLAN_PARAM_APMODE:
		*param = local->mib_ApMode;
		break;

	case MVWLAN_PARAM_BEACON_INT:
		*param = local->sysConfig->Mib802dot11->StationConfig.BcnPeriod;
		break;

	case MVWLAN_PARAM_DTIM_PERIOD:
		*param = local->sysConfig->Mib802dot11->StationConfig.DtimPeriod;
		break;

	case MVWLAN_PARAM_TXPOWER:
		if (local->func->get_cmd(dev, APCMD_GET_TX_POWER, 0,
					 &value, sizeof(value)) < 0) {
			ret = -EOPNOTSUPP;
			break;
		}
		*param = value;
		break;

	case MVWLAN_PARAM_DUMP:
		*param = local->frame_dump;
		break;

	case MVWLAN_PARAM_OTHER_AP_POLICY:
		if (local->ap != NULL)
			*param = local->ap->ap_policy;
		else
			ret = -EOPNOTSUPP;
		break;

	case MVWLAN_PARAM_AP_MAX_INACTIVITY:
		if (local->ap != NULL)
			*param = local->ap->max_inactivity / HZ;
		else
			ret = -EOPNOTSUPP;
		break;

	case MVWLAN_PARAM_AP_BRIDGE_PACKETS:
		if (local->ap != NULL)
			*param = local->ap->bridge_packets;
		else
			ret = -EOPNOTSUPP;
		break;

	case MVWLAN_PARAM_AP_NULLFUNC_ACK:
		if (local->ap != NULL)
			*param = local->ap->nullfunc_ack;
		else
			ret = -EOPNOTSUPP;
		break;

	case MVWLAN_PARAM_MAX_WDS:
		*param = local->wds_max_connections;
		break;

	case MVWLAN_PARAM_AP_AUTOM_AP_WDS:
		if (local->ap != NULL)
			*param = local->ap->autom_ap_wds;
		else
			ret = -EOPNOTSUPP;
		break;

	case MVWLAN_PARAM_AUTH_MODE:
		*param = local->auth_mode;
		break;

	case MVWLAN_PARAM_HOST_ENCRYPT:
		*param = local->host_encrypt;
		break;

	case MVWLAN_PARAM_HOST_DECRYPT:
		*param = local->host_decrypt;
		break;

	case MVWLAN_PARAM_HOST_ROAMING:
		*param = local->host_roaming;
		break;

	case MVWLAN_PARAM_IEEE_802_1X:
		*param = local->ieee_802_1x;
		break;

	case MVWLAN_PARAM_ANTSEL_TX:
		*param = local->mib_txAntenna;
		break;

	case MVWLAN_PARAM_ANTSEL_RX:
		*param = local->mib_rxAntenna;
		break;

	case MVWLAN_PARAM_WDS_TYPE:
		*param = local->wds_type;
		break;

	case MVWLAN_PARAM_HOSTSCAN:
		ret = -EOPNOTSUPP;
		break;

	case MVWLAN_PARAM_AP_SCAN:
		*param = local->passive_scan_interval;
		break;

	case MVWLAN_PARAM_ENH_SEC:
		*param = local->enh_sec;
		break;

	case MVWLAN_PARAM_BASIC_RATES:
		*param = local->basic_rate_set;
		break;

	case MVWLAN_PARAM_OPER_RATES:
		*param = local->tx_rate_control;
		break;

	case MVWLAN_PARAM_HOSTAPD:
		*param = local->hostapd;
		break;

	default:
		printk(KERN_DEBUG "%s: get_param: unknown param %d\n",
		       dev->name, *param);
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}
#endif /* WIRELESS_EXT >= 12 */


static int mvWLAN_ioctl_priv_monitor(struct net_device *dev, int *i)
{
	int ret = 0;
	u32 mode;

	printk(KERN_DEBUG "%s: process %d (%s) used deprecated iwpriv monitor "
	       "- update software to use iwconfig mode monitor\n",
	       dev->name, current->pid, current->comm);

	/* Backward compatibility code - this can be removed at some point */

	if (*i == 0) {
		/* Disable monitor mode - old mode was not saved, so go to
		 * Master mode */
		mode = IW_MODE_MASTER;
		ret = mvWLAN_iw_ioctl_siwmode(dev, NULL, &mode, NULL);
	} else if (*i == 1) {
		mode = IW_MODE_MONITOR;
		ret = mvWLAN_iw_ioctl_siwmode(dev, NULL, &mode, NULL);
	} else
		ret = -EINVAL;

	return ret;
}


static int mvWLAN_ioctl_priv_reset(struct net_device *dev, int *i)
{
	local_info_t *local = (local_info_t *) dev->priv;

	printk(KERN_DEBUG "%s: manual reset request(%d)\n", dev->name, *i);

	local->func->hw_reset(dev);

	return 0;
}


#ifndef MVWLAN_USE_WE_TYPE_ADDR
static inline int hex2int(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return -1;
}

static int macstr2addr(char *macstr, u8 *addr)
{
	int i, val, val2;
	char *pos = macstr;

	for (i = 0; i < 6; i++) {
		val = hex2int(*pos++);
		if (val < 0)
			return -1;
		val2 = hex2int(*pos++);
		if (val2 < 0)
			return -1;
		addr[i] = (val * 16 + val2) & 0xff;

		if (i < 5 && *pos++ != ':')
			return -1;
	}

	return 0;
}


static int mvWLAN_ioctl_priv_wds(struct net_device *dev, int add, char *macstr)
{
	local_info_t *local = (local_info_t *) dev->priv;
	u8 addr[6];

	if (macstr2addr(macstr, addr)) {
		printk(KERN_DEBUG "Invalid MAC address\n");
		return -EINVAL;
	}

	if (add)
		return mvWLAN_wds_add(local, addr, 1);
	else
		return mvWLAN_wds_del(local, addr, 1, 0);
}
#endif /* MVWLAN_USE_WE_TYPE_ADDR */


#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
static int ap_mac_cmd_ioctl(local_info_t *local, int *cmd)
{
	int ret = 0;

	switch (*cmd) {
	case AP_MAC_CMD_POLICY_OPEN:
		local->ap->mac_restrictions.policy = MAC_POLICY_OPEN;
		break;
	case AP_MAC_CMD_POLICY_ALLOW:
		local->ap->mac_restrictions.policy = MAC_POLICY_ALLOW;
		break;
	case AP_MAC_CMD_POLICY_DENY:
		local->ap->mac_restrictions.policy = MAC_POLICY_DENY;
		break;
	case AP_MAC_CMD_FLUSH:
		mvWLAN_ap_control_flush_macs(&local->ap->mac_restrictions);
		break;
	case AP_MAC_CMD_KICKALL:
		mvWLAN_ap_control_kickall(local->ap);
		mvWLAN_deauth_all_stas(local->dev, local->ap, 0);
		break;
	default:
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}


enum { AP_CTRL_MAC_ADD, AP_CTRL_MAC_DEL, AP_CTRL_MAC_KICK };

#ifndef MVWLAN_USE_WE_TYPE_ADDR
static int ap_mac_ioctl(local_info_t *local, char *macstr, int cmd)
{
	u8 addr[6];

	if (macstr2addr(macstr, addr)) {
		printk(KERN_DEBUG "Invalid MAC address '%s'\n", macstr);
		return -EINVAL;
	}

	switch (cmd) {
	case AP_CTRL_MAC_ADD:
		return mvWLAN_ap_control_add_mac(&local->ap->mac_restrictions, 
						 addr);
	case AP_CTRL_MAC_DEL:
		return mvWLAN_ap_control_del_mac(&local->ap->mac_restrictions, 
						 addr);
	case AP_CTRL_MAC_KICK:
		return mvWLAN_ap_control_kick_mac(local->ap, local->dev, addr);
	default:
		return -EOPNOTSUPP;
	}
}
#endif /* MVWLAN_USE_WE_TYPE_ADDR */
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */


static int mvWLAN_ioctl_set_encryption(local_info_t *local,
				       struct mvwlan_hostapd_param *param,
				       int param_len)
{
	int ret = 0;
	struct mvwlan_crypto_ops *ops;
	struct mvwlan_crypt_data **crypt;
	void *sta_ptr;

	param->u.crypt.err = 0;
	param->u.crypt.alg[MVWLAN_CRYPT_ALG_NAME_LEN - 1] = '\0';

	if (param_len !=
	    (int) ((char *) param->u.crypt.key - (char *) param) +
	    param->u.crypt.key_len)
		return -EINVAL;

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		sta_ptr = NULL;
		crypt = &local->crypt;
	} else {
		sta_ptr = ap_crypt_get_ptrs(
			local->ap, param->sta_addr,
			(param->u.crypt.flags & MVWLAN_CRYPT_FLAG_PERMANENT),
			&crypt);

		if (sta_ptr == NULL) {
			param->u.crypt.err = MVWLAN_CRYPT_ERR_UNKNOWN_ADDR;
			return -EINVAL;
		}
	}

	if (strcmp(param->u.crypt.alg, "none") == 0) {
		mvWLAN_crypt_delayed_deinit(local, crypt);
		goto done;
	}

	ops = mvWLAN_get_crypto_ops(param->u.crypt.alg);
	if (ops == NULL && strcmp(param->u.crypt.alg, "WEP") == 0) {
		ops = mvWLAN_get_crypto_ops(param->u.crypt.alg);
	}
	if (ops == NULL) {
		printk(KERN_DEBUG "%s: unknown crypto alg '%s'\n",
		       local->dev->name, param->u.crypt.alg);
		param->u.crypt.err = MVWLAN_CRYPT_ERR_UNKNOWN_ALG;
		ret = -EINVAL;
		goto done;
	}

	/* station based encryption and other than WEP algorithms require
	 * host-based encryption, so force them on automatically */
	local->host_decrypt = local->host_encrypt = 1;

	if (*crypt == NULL || (*crypt)->ops != ops) {
		struct mvwlan_crypt_data *new_crypt;

		mvWLAN_crypt_delayed_deinit(local, crypt);

		new_crypt = (struct mvwlan_crypt_data *)
			kmalloc(sizeof(struct mvwlan_crypt_data), GFP_KERNEL);
		if (new_crypt == NULL) {
			ret = -ENOMEM;
			goto done;
		}
		memset(new_crypt, 0, sizeof(struct mvwlan_crypt_data));
		new_crypt->ops = ops;
		new_crypt->priv = new_crypt->ops->init();
		if (new_crypt->priv == NULL) {
			kfree(new_crypt);
			param->u.crypt.err =
				MVWLAN_CRYPT_ERR_CRYPT_INIT_FAILED;
			ret = -EINVAL;
			goto done;
		}

		*crypt = new_crypt;
	}

	if ((!(param->u.crypt.flags & MVWLAN_CRYPT_FLAG_SET_TX_KEY) ||
	     param->u.crypt.key_len > 0) && (*crypt)->ops->set_key &&
	    (*crypt)->ops->set_key(param->u.crypt.idx, param->u.crypt.key,
			      param->u.crypt.key_len, (*crypt)->priv) < 0) {
		printk(KERN_DEBUG "%s: key setting failed\n",
		       local->dev->name);
		param->u.crypt.err = MVWLAN_CRYPT_ERR_KEY_SET_FAILED;
		ret = -EINVAL;
		goto done;
	}

	if ((param->u.crypt.flags & MVWLAN_CRYPT_FLAG_SET_TX_KEY) &&
	    (*crypt)->ops->set_key_idx &&
	    (*crypt)->ops->set_key_idx(param->u.crypt.idx, (*crypt)->priv) < 0)
	{
		printk(KERN_DEBUG "%s: TX key idx setting failed\n",
		       local->dev->name);
		param->u.crypt.err = MVWLAN_CRYPT_ERR_TX_KEY_SET_FAILED;
		ret = -EINVAL;
		goto done;
	}

 done:
	if (sta_ptr)
		mvWLAN_handle_sta_release(sta_ptr);

	if (ret == 0 && mvWLAN_set_encryption(local)) {
		param->u.crypt.err = MVWLAN_CRYPT_ERR_HW_CONF_FAILED;
		return -EINVAL;
	}

	return ret;
}


static int mvWLAN_ioctl_get_encryption(local_info_t *local,
				       struct mvwlan_hostapd_param *param,
				       int param_len)
{
	struct mvwlan_crypt_data **crypt;
	void *sta_ptr;
	int max_key_len;

	param->u.crypt.err = 0;

	max_key_len = param_len -
		(int) ((char *) param->u.crypt.key - (char *) param);
	if (max_key_len < 0)
		return -EINVAL;

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff) {
		sta_ptr = NULL;
		crypt = &local->crypt;
	} else {
		sta_ptr = ap_crypt_get_ptrs(local->ap, param->sta_addr, 0,
					    &crypt);

		if (sta_ptr == NULL) {
			param->u.crypt.err = MVWLAN_CRYPT_ERR_UNKNOWN_ADDR;
			return -EINVAL;
		}
	}

	if (*crypt == NULL || (*crypt)->ops == NULL) {
		memcpy(param->u.crypt.alg, "none", 5);
		param->u.crypt.key_len = 0;
		param->u.crypt.idx = 0xff;
	} else {
		strncpy(param->u.crypt.alg, (*crypt)->ops->name,
			MVWLAN_CRYPT_ALG_NAME_LEN);
		param->u.crypt.key_len = 0;
		if (param->u.crypt.idx >= MVWLAN_WEP_KEYS &&
		    (*crypt)->ops->get_key_idx)
			param->u.crypt.idx =
				(*crypt)->ops->get_key_idx((*crypt)->priv);

		if (param->u.crypt.idx < MVWLAN_WEP_KEYS && (*crypt)->ops->get_key)
			param->u.crypt.key_len =
				(*crypt)->ops->get_key(param->u.crypt.idx,
						       param->u.crypt.key,
						       max_key_len,
						       (*crypt)->priv);
	}

	if (sta_ptr)
		mvWLAN_handle_sta_release(sta_ptr);

	return 0;
}


static int mvWLAN_ioctl_get_command(local_info_t *local,
				    struct mvwlan_hostapd_param *param,
				    int param_len)
{
	int res;

	res = local->func->get_cmd(local->dev, param->u.cmd.cmd,
				   param->u.cmd.param, param->u.cmd.data, 
				   param->u.cmd.len);
	if (res >= 0) {
		param->u.cmd.len = res;
	}

	return res;
}


static int mvWLAN_ioctl_set_command(local_info_t *local,
				    struct mvwlan_hostapd_param *param,
				    int param_len)
{
	return local->func->set_cmd(local->dev, param->u.cmd.cmd,
				    param->u.cmd.param, param->u.cmd.data, 
				    param->u.cmd.len);
}


static int mvWLAN_ioctl_set_assoc_ap_addr(local_info_t *local,
					  struct mvwlan_hostapd_param *param,
					  int param_len)
{
	printk(KERN_DEBUG "%ssta: associated as client with AP " MACSTR "\n",
	       local->dev->name, MAC2STR(param->sta_addr));
	memcpy(local->assoc_ap_addr, param->sta_addr, ETH_ALEN);
	return 0;
}


static int mvWLAN_ioctl_priv_hostapd(local_info_t *local, struct iw_point *p)
{
	struct mvwlan_hostapd_param *param;
	int ret = 0, need_copy = 0;

	if (p->length < sizeof(struct mvwlan_hostapd_param) ||
	    p->length > MVWLAN_HOSTAPD_MAX_BUF_SIZE || !p->pointer)
		return -EINVAL;

	param = (struct mvwlan_hostapd_param *) kmalloc(p->length, GFP_KERNEL);
	if (param == NULL)
		return -ENOMEM;

	if (copy_from_user(param, p->pointer, p->length)) {
		ret = -EFAULT;
		goto out;
	}

	switch (param->cmd) {
	case MVWLAN_SET_ENCRYPTION:
		ret = mvWLAN_ioctl_set_encryption(local, param, p->length);
		break;
	case MVWLAN_GET_ENCRYPTION:
		ret = mvWLAN_ioctl_get_encryption(local, param, p->length);
		need_copy = 1;
		break;
	case MVWLAN_HOSTAPD_SET_CMD:
		ret = mvWLAN_ioctl_set_command(local, param, p->length);
		break;
	case MVWLAN_HOSTAPD_GET_CMD:
		ret = mvWLAN_ioctl_get_command(local, param, p->length);
		need_copy = 1;
		break;
	case MVWLAN_HOSTAPD_SET_ASSOC_AP_ADDR:
		ret = mvWLAN_ioctl_set_assoc_ap_addr(local, param, p->length);
		break;
	default:
		ret = mvWLAN_hostapd(local->ap, param);

		if (param->cmd == MVWLAN_HOSTAPD_GET_INFO_STA)
			need_copy = 1;
		break;
	}

	if (need_copy && (ret > 0)) {
		if (copy_to_user(p->pointer, param, p->length))
			ret = -EFAULT;
	}

 out:
	if (param != NULL)
		kfree(param);

	return ret;
}


#if WIRELESS_EXT > 12
/* Structures to export the Wireless Handlers */

static const iw_handler mvwlan_iw_ioctl_handler[] =
{
	(iw_handler) NULL,                  	/* SIOCSIWCOMMIT */
	(iw_handler) mvWLAN_iw_ioctl_giwname,  	/* SIOCGIWNAME */
	(iw_handler) NULL,                  	/* SIOCSIWNWID */
	(iw_handler) NULL,                  	/* SIOCGIWNWID */
	(iw_handler) mvWLAN_iw_ioctl_siwfreq,  	/* SIOCSIWFREQ */
	(iw_handler) mvWLAN_iw_ioctl_giwfreq,  	/* SIOCGIWFREQ */
	(iw_handler) mvWLAN_iw_ioctl_siwmode,  	/* SIOCSIWMODE */
	(iw_handler) mvWLAN_iw_ioctl_giwmode,  	/* SIOCGIWMODE */
	(iw_handler) mvWLAN_iw_ioctl_siwsens,  	/* SIOCSIWSENS */
	(iw_handler) mvWLAN_iw_ioctl_giwsens,  	/* SIOCGIWSENS */
	(iw_handler) NULL /* not used */,   	/* SIOCSIWRANGE */
	(iw_handler) mvWLAN_iw_ioctl_giwrange, 	/* SIOCGIWRANGE */
	(iw_handler) NULL /* not used */,   	/* SIOCSIWPRIV */
	(iw_handler) NULL /* kernel code */,	/* SIOCGIWPRIV */
	(iw_handler) NULL /* not used */,   	/* SIOCSIWSTATS */
	(iw_handler) NULL /* kernel code */,	/* SIOCGIWSTATS */
#if WIRELESS_EXT > 15
	iw_handler_set_spy,                	/* SIOCSIWSPY */
	iw_handler_get_spy,                 	/* SIOCGIWSPY */
	iw_handler_set_thrspy,	            	/* SIOCSIWTHRSPY */
	iw_handler_get_thrspy,              	/* SIOCGIWTHRSPY */
#else /* WIRELESS_EXT > 15 */
	(iw_handler) NULL,                  	/* SIOCSIWSPY */
	(iw_handler) mvWLAN_iw_ioctl_giwspy,   	/* SIOCGIWSPY */
	(iw_handler) NULL,                  	/* -- hole -- */
	(iw_handler) NULL,                  	/* -- hole -- */
#endif /* WIRELESS_EXT > 15 */
	(iw_handler) mvWLAN_iw_ioctl_siwap,   	/* SIOCSIWAP */
	(iw_handler) mvWLAN_iw_ioctl_giwap,    	/* SIOCGIWAP */
	(iw_handler) NULL,                  	/* -- hole -- */
	(iw_handler) mvWLAN_iw_ioctl_giwaplist,	/* SIOCGIWAPLIST */
#if WIRELESS_EXT > 13
	(iw_handler) mvWLAN_iw_ioctl_siwscan,  	/* SIOCSIWSCAN */
	(iw_handler) mvWLAN_iw_ioctl_giwscan,  	/* SIOCGIWSCAN */
#else /* WIRELESS_EXT > 13 */
	(iw_handler) NULL,                  	/* SIOCSIWSCAN */
	(iw_handler) NULL,                  	/* SIOCGIWSCAN */
#endif /* WIRELESS_EXT > 13 */
	(iw_handler) mvWLAN_iw_ioctl_siwessid, 	/* SIOCSIWESSID */
	(iw_handler) mvWLAN_iw_ioctl_giwessid,	/* SIOCGIWESSID */
	(iw_handler) mvWLAN_iw_ioctl_siwnickn, 	/* SIOCSIWNICKN */
	(iw_handler) mvWLAN_iw_ioctl_giwnickn, 	/* SIOCGIWNICKN */
	(iw_handler) NULL,                  	/* -- hole -- */
	(iw_handler) NULL,                  	/* -- hole -- */
	(iw_handler) mvWLAN_iw_ioctl_siwrate, 	/* SIOCSIWRATE */
	(iw_handler) mvWLAN_iw_ioctl_giwrate,  	/* SIOCGIWRATE */
	(iw_handler) mvWLAN_iw_ioctl_siwrts,   	/* SIOCSIWRTS */
	(iw_handler) mvWLAN_iw_ioctl_giwrts,   	/* SIOCGIWRTS */
	(iw_handler) mvWLAN_iw_ioctl_siwfrag,  	/* SIOCSIWFRAG */
	(iw_handler) mvWLAN_iw_ioctl_giwfrag,  	/* SIOCGIWFRAG */
	(iw_handler) mvWLAN_iw_ioctl_siwtxpow, 	/* SIOCSIWTXPOW */
	(iw_handler) mvWLAN_iw_ioctl_giwtxpow, 	/* SIOCGIWTXPOW */
	(iw_handler) mvWLAN_iw_ioctl_siwretry, 	/* SIOCSIWRETRY */
	(iw_handler) mvWLAN_iw_ioctl_giwretry, 	/* SIOCGIWRETRY */
	(iw_handler) mvWLAN_iw_ioctl_siwencode,	/* SIOCSIWENCODE */
	(iw_handler) mvWLAN_iw_ioctl_giwencode,	/* SIOCGIWENCODE */
	(iw_handler) mvWLAN_iw_ioctl_siwpower, 	/* SIOCSIWPOWER */
	(iw_handler) mvWLAN_iw_ioctl_giwpower, 	/* SIOCGIWPOWER */
};

static const iw_handler mvwlan_iw_ioctl_private_handler[] =
{						/* SIOCIWFIRSTPRIV + */
	(iw_handler) mvWLAN_ioctl_priv_set_param,	/* 0 */
	(iw_handler) mvWLAN_ioctl_priv_get_param,	/* 1 */
};

const struct iw_handler_def mvwlan_iw_handler_def =
{
	.num_standard	= sizeof(mvwlan_iw_ioctl_handler)/sizeof(iw_handler),
	.num_private	= sizeof(mvwlan_iw_ioctl_private_handler)/
				sizeof(iw_handler),
	.num_private_args = sizeof(mvwlan_iw_ioctl_priv)/
				sizeof(struct iw_priv_args),
	.standard	= (iw_handler *) mvwlan_iw_ioctl_handler,
	.private	= (iw_handler *) mvwlan_iw_ioctl_private_handler,
	.private_args	= (struct iw_priv_args *) mvwlan_iw_ioctl_priv,
#if WIRELESS_EXT > 15
	.spy_offset	= ((void *) (&((local_info_t *) NULL)->spy_data) -
			   (void *) NULL),
#endif /* WIRELESS_EXT > 15 */
};
#endif	/* WIRELESS_EXT > 12 */


int mvWLAN_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct iwreq *wrq = (struct iwreq *) ifr;
	local_info_t *local = (local_info_t *) dev->priv;
	int ret = 0;

	switch (cmd) {

#if WIRELESS_EXT <= 12
	case SIOCGIWNAME:
		ret = mvWLAN_iw_ioctl_giwname(dev, NULL, (char *) &wrq->u, 
					      NULL);
		break;

	case SIOCSIWFREQ:
		ret = mvWLAN_iw_ioctl_siwfreq(dev, NULL, &wrq->u.freq, 
					      NULL);
		break;
	case SIOCGIWFREQ:
		ret = mvWLAN_iw_ioctl_giwfreq(dev, NULL, &wrq->u.freq, 
					      NULL);
		break;

	case SIOCSIWAP:
		ret = mvWLAN_iw_ioctl_siwap(dev, NULL, &wrq->u.ap_addr, 
					    NULL);
		break;
	case SIOCGIWAP:
		ret = mvWLAN_iw_ioctl_giwap(dev, NULL, &wrq->u.ap_addr, 
					    NULL);
		break;

	case SIOCSIWESSID:
		if (wrq->u.essid.length > IW_ESSID_MAX_SIZE) {
			ret = -E2BIG;
			break;
		}
		if (wrq->u.essid.pointer) {
			char ssid[IW_ESSID_MAX_SIZE];

			if (copy_from_user(ssid, wrq->u.essid.pointer,
					   wrq->u.essid.length)) {
				ret = -EFAULT;
				break;
			}
			ret = mvWLAN_iw_ioctl_siwessid(dev, NULL, 
						       &wrq->u.essid,
						       ssid);
		}
		break;
	case SIOCGIWESSID:
		if (wrq->u.essid.length > IW_ESSID_MAX_SIZE) {
			ret = -E2BIG;
			break;
		}
		if (wrq->u.essid.pointer) {
			char ssid[IW_ESSID_MAX_SIZE];

			ret = mvWLAN_iw_ioctl_giwessid(dev, NULL, 
						       &wrq->u.essid,
						       ssid);
			if (copy_to_user(wrq->u.essid.pointer, ssid,
					 wrq->u.essid.length))
				ret = -EFAULT;
		}
		break;

	case SIOCSIWRATE:
		ret = mvWLAN_iw_ioctl_siwrate(dev, NULL, &wrq->u.bitrate, 
					      NULL);
		break;
	case SIOCGIWRATE:
		ret = mvWLAN_iw_ioctl_giwrate(dev, NULL, &wrq->u.bitrate, 
					      NULL);
		break;

	case SIOCSIWRTS:
		ret = mvWLAN_iw_ioctl_siwrts(dev, NULL, &wrq->u.rts, NULL);
		break;
	case SIOCGIWRTS:
		ret = mvWLAN_iw_ioctl_giwrts(dev, NULL, &wrq->u.rts, NULL);
		break;

	case SIOCSIWFRAG:
		ret = mvWLAN_iw_ioctl_siwfrag(dev, NULL, &wrq->u.rts, NULL);
		break;
	case SIOCGIWFRAG:
		ret = mvWLAN_iw_ioctl_giwfrag(dev, NULL, &wrq->u.rts, NULL);
		break;

	case SIOCSIWENCODE:
		if (wrq->u.encoding.length > WEP_KEY_LEN) {
			ret = -E2BIG;
			break;
		}
		if (wrq->u.encoding.pointer) {
			char keybuf[WEP_KEY_LEN];

			if (copy_from_user(keybuf,
					   wrq->u.encoding.pointer,
					   wrq->u.encoding.length)) {
				ret = -EFAULT;
				break;
			}
			ret = mvWLAN_iw_ioctl_siwencode(dev, NULL,
							&wrq->u.encoding, 
							keybuf);
		}
		break;
	case SIOCGIWENCODE:
		if (!capable(CAP_NET_ADMIN))
			ret = -EPERM;
		else if (wrq->u.encoding.pointer) {
			char keybuf[WEP_KEY_LEN];

			ret = mvWLAN_iw_ioctl_giwencode(dev, NULL,
							&wrq->u.encoding, 
							keybuf);
			if (copy_to_user(wrq->u.encoding.pointer, keybuf,
					 wrq->u.encoding.length))
				ret = -EFAULT;
		}
		break;

	case SIOCSIWNICKN:
		if (wrq->u.essid.length > IW_ESSID_MAX_SIZE)
			ret = -E2BIG;
		else if (wrq->u.essid.pointer) {
			char nickbuf[IW_ESSID_MAX_SIZE + 1];

			if (copy_from_user(nickbuf, wrq->u.essid.pointer,
					   wrq->u.essid.length)) {
				ret = -EFAULT;
				break;
			}
			ret =  mvWLAN_iw_ioctl_siwnickn(dev, NULL, 
							&wrq->u.essid,
							nickbuf);
		}
		break;
	case SIOCGIWNICKN:
		if (wrq->u.essid.pointer) {
			char nickbuf[IW_ESSID_MAX_SIZE + 1];

			ret = mvWLAN_iw_ioctl_giwnickn(dev, NULL, 
						       &wrq->u.essid,
						       nickbuf);
			if (copy_to_user(wrq->u.essid.pointer, nickbuf,
					 wrq->u.essid.length))
				ret = -EFAULT;
		}
		break;

	case SIOCGIWSPY:
		if (wrq->u.data.pointer) {
			char buffer[IW_MAX_SPY * (sizeof(struct sockaddr) +
						  sizeof(struct iw_quality))];

			ret = mvWLAN_iw_ioctl_giwspy(dev, NULL, &wrq->u.data,
						     buffer);
			if (ret == 0 && 
			    copy_to_user(wrq->u.data.pointer, buffer,
					 wrq->u.data.length *
					 (sizeof(struct sockaddr) +
					  sizeof(struct iw_quality))))
				ret = -EFAULT;
		}
		break;

	case SIOCGIWRANGE:
		if (wrq->u.data.pointer) {
			struct iw_range range;

			ret = mvWLAN_iw_ioctl_giwrange(dev, NULL, 
						       &wrq->u.data,
						       (char *) &range);
			if (copy_to_user(wrq->u.data.pointer, &range,
					 sizeof(struct iw_range)))
				ret = -EFAULT;
		}
		break;

	case SIOCSIWSENS:
		ret = mvWLAN_iw_ioctl_siwsens(dev, NULL, &wrq->u.sens, NULL);
		break;
	case SIOCGIWSENS:
		ret = mvWLAN_iw_ioctl_giwsens(dev, NULL, &wrq->u.sens, NULL);
		break;

	case SIOCGIWAPLIST:
		if (wrq->u.data.pointer) {
			char buffer[IW_MAX_AP * (sizeof(struct sockaddr) +
						 sizeof(struct iw_quality))];

			ret = mvWLAN_iw_ioctl_giwaplist(dev, NULL, 
							&wrq->u.data,
							buffer);
			if (copy_to_user(wrq->u.data.pointer, buffer,
					 (wrq->u.data.length *
					  (sizeof(struct sockaddr) +
					   sizeof(struct iw_quality)))))
				ret = -EFAULT;
		}
		break;

	case SIOCSIWMODE:
		ret = mvWLAN_iw_ioctl_siwmode(dev, NULL, &wrq->u.mode, 
					      NULL);
		break;
	case SIOCGIWMODE:
		ret = mvWLAN_iw_ioctl_giwmode(dev, NULL, &wrq->u.mode, 
					      NULL);
		break;

	case SIOCSIWPOWER:
		ret = mvWLAN_iw_ioctl_siwpower(dev, NULL, &wrq->u.power, 
					       NULL);
		break;
	case SIOCGIWPOWER:
		ret = mvWLAN_iw_ioctl_giwpower(dev, NULL, &wrq->u.power, 
					       NULL);
		break;

	case SIOCGIWPRIV:
		ret = mvWLAN_iw_ioctl_giwpriv(dev, &wrq->u.data);
		break;

#if WIRELESS_EXT > 9
	case SIOCSIWTXPOW:
		ret = mvWLAN_iw_ioctl_siwtxpow(dev, NULL, &wrq->u.txpower, 
					       NULL);
		break;
	case SIOCGIWTXPOW:
		ret = mvWLAN_iw_ioctl_giwtxpow(dev, NULL, &wrq->u.txpower, 
					       NULL);
		break;
#endif /* WIRELESS_EXT > 9 */

#if WIRELESS_EXT > 10
	case SIOCSIWRETRY:
		ret = mvWLAN_iw_ioctl_siwretry(dev, NULL, &wrq->u.retry, 
					       NULL);
		break;
	case SIOCGIWRETRY:
		ret = mvWLAN_iw_ioctl_giwretry(dev, NULL, &wrq->u.retry, 
					       NULL);
		break;
#endif /* WIRELESS_EXT > 10 */

	/* not supported wireless extensions */
	case SIOCSIWNWID:
	case SIOCGIWNWID:
		ret = -EOPNOTSUPP;
		break;

	/* FIX: add support for this: */
	case SIOCSIWSPY:
		printk(KERN_DEBUG "%s unsupported WIRELESS_EXT ioctl(0x%04x)\n"
		       , dev->name, cmd);
		ret = -EOPNOTSUPP;
		break;


		/* Private ioctls (iwpriv); these are in SIOCDEVPRIVATE range
		 * if WIRELESS_EXT < 12, so better check privileges */

	case MVWLAN_IOCTL_SET_PARAM:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_ioctl_priv_set_param(dev, NULL, &wrq->u,
						  (char *) &wrq->u);
		break;
#if WIRELESS_EXT >= 12
	case MVWLAN_IOCTL_GET_PARAM:
		ret = mvWLAN_ioctl_priv_get_param(dev, NULL, &wrq->u,
						  (char *) &wrq->u);
		break;
#endif /* WIRELESS_EXT >= 12 */

#endif /* WIRELESS_EXT <= 12 */


		/* Private ioctls (iwpriv) that have not yet been converted
		 * into new wireless extensions API */

	case MVWLAN_IOCTL_MONITOR:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_ioctl_priv_monitor(dev, (int *) wrq->u.name);
		break;

	case MVWLAN_IOCTL_RESET:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_ioctl_priv_reset(dev, (int *) wrq->u.name);
		break;

#ifdef MVWLAN_USE_WE_TYPE_ADDR
	case MVWLAN_IOCTL_WDS_ADD:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_wds_add(local, wrq->u.ap_addr.sa_data, 1);
		break;

	case MVWLAN_IOCTL_WDS_DEL:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_wds_del(local, wrq->u.ap_addr.sa_data, 1, 0);
		break;
#else /* MVWLAN_USE_WE_TYPE_ADDR */
	case MVWLAN_IOCTL_WDS_ADD:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		if (wrq->u.data.pointer) {
			char addrbuf[18];

			if (copy_from_user(addrbuf, wrq->u.data.pointer, 18)) {
				ret = -EFAULT;
				break;
			}
			ret = mvWLAN_ioctl_priv_wds(dev, 1, addrbuf);
		}
		break;

	case MVWLAN_IOCTL_WDS_DEL:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		if (wrq->u.data.pointer) {
			char addrbuf[18];

			if (copy_from_user(addrbuf, wrq->u.data.pointer, 18)) {
				ret = -EFAULT;
				break;
			}
			ret = mvWLAN_ioctl_priv_wds(dev, 0, addrbuf);
		}
		break;
#endif /* MVWLAN_USE_WE_TYPE_ADDR */

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	case MVWLAN_IOCTL_MACCMD:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = ap_mac_cmd_ioctl(local, (int *) wrq->u.name);
		break;

#ifdef MVWLAN_USE_WE_TYPE_ADDR
	case MVWLAN_IOCTL_ADDMAC:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_ap_control_add_mac(&local->ap->mac_restrictions,
						wrq->u.ap_addr.sa_data);
		break;
	case MVWLAN_IOCTL_DELMAC:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_ap_control_del_mac(&local->ap->mac_restrictions,
						wrq->u.ap_addr.sa_data);
		break;
	case MVWLAN_IOCTL_KICKMAC:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_ap_control_kick_mac(local->ap, local->dev,
						 wrq->u.ap_addr.sa_data);
		break;
#else /* MVWLAN_USE_WE_TYPE_ADDR */
	case MVWLAN_IOCTL_ADDMAC:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		if (wrq->u.data.pointer) {
			char addrbuf[18];

			if (copy_from_user(addrbuf, wrq->u.data.pointer, 18)) {
				ret = -EFAULT;
				break;
			}
			ret = ap_mac_ioctl(local, addrbuf, AP_CTRL_MAC_ADD);
		}
		break;

	case MVWLAN_IOCTL_DELMAC:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		if (wrq->u.data.pointer) {
			char addrbuf[18];

			if (copy_from_user(addrbuf, wrq->u.data.pointer, 18)) {
				ret = -EFAULT;
				break;
			}
			ret = ap_mac_ioctl(local, addrbuf, AP_CTRL_MAC_DEL);
		}
		break;

	case MVWLAN_IOCTL_KICKMAC:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		if (wrq->u.data.pointer) {
			char addrbuf[18];

			if (copy_from_user(addrbuf, wrq->u.data.pointer, 18)) {
				ret = -EFAULT;
				break;
			}
			ret = ap_mac_ioctl(local, addrbuf, AP_CTRL_MAC_KICK);
		}
		break;
#endif /* MVWLAN_USE_WE_TYPE_ADDR */
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */


		/* Private ioctls that are not used with iwpriv;
		 * in SIOCDEVPRIVATE range */

	case MVWLAN_IOCTL_HOSTAPD:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		ret = mvWLAN_ioctl_priv_hostapd(local, &wrq->u.data);
		break;

	default:
#if WIRELESS_EXT > 12
		if (cmd >= SIOCSIWCOMMIT && cmd <= SIOCGIWPOWER) {
			/* unsupport wireless extensions get through here - do
			 * not report these to debug log */
			ret = -EOPNOTSUPP;
			break;
		}
#endif /* WIRELESS_EXT > 12 */
		printk(KERN_DEBUG "%s unsupported ioctl(0x%04x)\n",
		       dev->name, cmd);
		ret = -EOPNOTSUPP;
		break;
	}

	return ret;
}

#endif /* WIRELESS_EXT */
