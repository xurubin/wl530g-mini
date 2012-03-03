/* /proc routines for Marvell Host AP driver */
#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

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
#include <linux/proc_fs.h>

#include "IEEE_types.h"
#include "hal_sys.h"
#include "wl_mib.h"
#include "wl_hal.h"
#include "wl_macros.h"
#include "wlmac.h"
#include "memmap.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"

#ifdef BOOSTER_MODE
#include "mvWLAN_booster.h"
#endif


#define PROC_LIMIT (PAGE_SIZE - 80)


struct proc_dir_entry *mvWLAN_proc = NULL;
static struct proc_dir_entry *mvWLAN_apdaemon_ctrl;


/* PRIVATE FUNCTION DECLARATION
 */

static int MVWLAN_PROC_ReadAllRegisters(char *page, char **start, off_t off, int count, int *eof, void *data);
static char *MVWLAN_PROC_ReadCPURegisters(char *start);
static char *MVWLAN_PROC_Read11GMACRegisters(char *start);
static char *MVWLAN_PROC_ReadBBPRegisters(char *start);
static char *MVWLAN_PROC_ReadRFRegisters(char *start);
static int MVWLAN_PROC_DumpMAC1Registers(char *page, char **start, off_t off, int count, int *eof, void *data);
static int MVWLAN_PROC_DumpMAC2Registers(char *page, char **start, off_t off, int count, int *eof, void *data);
static int MVWLAN_PROC_DumpMAC3Registers(char *page, char **start, off_t off, int count, int *eof, void *data);
static int MVWLAN_PROC_DumpBBRegisters(char *page, char **start, off_t off, int count, int *eof, void *data);
static int MVWLAN_PROC_DumpRFRegisters(char *page, char **start, off_t off, int count, int *eof, void *data);
static int MVWLAN_PROC_DumpMFGData(char *page, char **start, off_t off, int count, int *eof, void *data);

/********************************************************************
 * mvWLAN_apd_ctrl_write
 *
 * DESCRIPTION:
 *       When written to the /proc/mvwlan/apd file this function
 *       is called.
 *
 * INPUTS:
 *       file   - Not used.
 *       buffer - pointer to the input string
 *       data   - Not used.
 *       count  - length of the input string
 *
 * OUTPUTS:
 *       None.
 *
 * RETURNS:
 *       Length of the input string if the operation is successful.
 *       Otherwise 0xFFFFFFFF is returned.
 *
 * COMMENTS:
 *       None.
 *
 *********************************************************************/
static int mvWLAN_apd_ctrl_write(struct file *file, const char *buffer,
				 unsigned long count, void *data)
{
	int rc;
	unsigned int code;

	code = *((unsigned int *) buffer);

	if (! code) {
		/* Release the driver specific resources. */
		rc = mvWLAN_free_dev();
	} else {
		/* AP daemon should pass the configuration and manufacture
		 * data for us. */
		rc = mvWLAN_init_dev((void *)(buffer + sizeof(code)));
	}

	if (rc)
		printk(KERN_DEBUG "apdio: Error (%d) for IOCTL %x\n", code, rc);

	return count;
}

/********************************************************************
 * mvWLAN_apd_ctrl_read
 *
 * DESCRIPTION:
 *       When read from the /proc/mvwlan/apd file this function
 *       is called.
 *
 * INPUTS:
 *       file   - Not used.
 *       buffer - pointer to the input string
 *       data   - Not used.
 *       count  - length of the input string
 *
 * OUTPUTS:
 *       None.
 *
 * RETURNS:
 *       Length of the input string if the operation is successful.
 *       Otherwise 0xFFFFFFFF is returned.
 *
 * COMMENTS:
 *       None.
 *
 *********************************************************************/
static int mvWLAN_apd_ctrl_read(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	/* XXX, TBD (maybe fill in the current configuration) */
	*eof = 1;
	return 0;
}


#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
static int mvWLAN_ap_proc_read(char *page, char **start, off_t off,
			       int count, int *eof, void *data)
{
	char *p = page;
	struct ap_data *ap = (struct ap_data *) data;
	struct list_head *ptr;
	int i;

	if (off > PROC_LIMIT) {
		*eof = 1;
		return 0;
	}

	p += sprintf(p, "# BSSID CHAN SIGNAL NOISE RATE SSID FLAGS\n");
	spin_lock_bh(&ap->sta_table_lock);
	for (ptr = ap->sta_list.next; ptr != &ap->sta_list; ptr = ptr->next) {
		struct sta_info *sta = (struct sta_info *) ptr;

		if (!sta->ap)
			continue;

		p += sprintf(p, MACSTR " %d %d %d %d '", MAC2STR(sta->addr),
			     sta->u.ap.channel, sta->last_rx_signal,
			     sta->last_rx_silence, sta->last_rx_rate);
		for (i = 0; i < sta->u.ap.ssid_len; i++)
			p += sprintf(p, ((sta->u.ap.ssid[i] >= 32 &&
					  sta->u.ap.ssid[i] < 127) ?
					 "%c" : "<%02x>"),
				     sta->u.ap.ssid[i]);
		p += sprintf(p, "'");
		if (sta->capability & WLAN_CAPABILITY_ESS)
			p += sprintf(p, " [ESS]");
		if (sta->capability & WLAN_CAPABILITY_IBSS)
			p += sprintf(p, " [IBSS]");
		if (sta->capability & WLAN_CAPABILITY_PRIVACY)
			p += sprintf(p, " [WEP]");
		if (sta->capability & WLAN_CAPABILITY_PBCC)
			p += sprintf(p, " [PBCC]");
		if (sta->capability & WLAN_CAPABILITY_DSSS_OFDM)
			p += sprintf(p, " [DSSS/OFDM]");
		p += sprintf(p, "\n");

		if ((p - page) > PROC_LIMIT) {
			printk(KERN_DEBUG "mvWLAN: ap proc did not fit\n");
			break;
		}
	}
	spin_unlock_bh(&ap->sta_table_lock);

	if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}

	*start = page + off;

	return (p - page - off);
}


static int mvWLAN_ap_control_proc_read(char *page, char **start, off_t off,
				       int count, int *eof, void *data)
{
	char *p = page;
	struct ap_data *ap = (struct ap_data *) data;
	char *policy_txt;
	struct list_head *ptr;
	struct mac_entry *entry;

	if (off != 0) {
		*eof = 1;
		return 0;
	}

	switch (ap->mac_restrictions.policy) {
	case MAC_POLICY_OPEN:
		policy_txt = "open";
		break;
	case MAC_POLICY_ALLOW:
		policy_txt = "allow";
		break;
	case MAC_POLICY_DENY:
		policy_txt = "deny";
		break;
	default:
		policy_txt = "unknown";
		break;
	};
	p += sprintf(p, "MAC policy: %s\n", policy_txt);
	p += sprintf(p, "MAC entries: %u\n", ap->mac_restrictions.entries);
	p += sprintf(p, "MAC list:\n");
	spin_lock_bh(&ap->mac_restrictions.lock);
	for (ptr = ap->mac_restrictions.mac_list.next;
	     ptr != &ap->mac_restrictions.mac_list; ptr = ptr->next) {
		if (p - page > PAGE_SIZE - 80) {
			p += sprintf(p, "All entries did not fit one page.\n");
			break;
		}

		entry = list_entry(ptr, struct mac_entry, list);
		p += sprintf(p, MACSTR "\n", MAC2STR(entry->addr));
	}
	spin_unlock_bh(&ap->mac_restrictions.lock);

	return (p - page);
}
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */


static int mvWLAN_sta_proc_read(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	char *p = page;
	struct sta_info *sta = (struct sta_info *) data;
	int i;

	/* FIX: possible race condition.. the STA data could have just expired,
	 * but proc entry was still here so that the read could have started;
	 * some locking should be done here.. */

    if (off > PROC_LIMIT) {
		*eof = 1;
		return 0;
	}

	p += sprintf(p, "%s=" MACSTR "\nusers=%d\naid=%d\n"
		     "flags=0x%04x%s%s%s%s%s%s%s\n"
		     "capability=0x%02x\nlisten_interval=%d\nsupported_rates=",
		     sta->ap ? "AP" : "STA",
		     MAC2STR(sta->addr), atomic_read(&sta->users), sta->aid,
		     sta->flags,
		     sta->flags & WLAN_STA_AUTH ? " AUTH" : "",
		     sta->flags & WLAN_STA_ASSOC ? " ASSOC" : "",
		     sta->flags & WLAN_STA_PS ? " PS" : "",
		     sta->flags & WLAN_STA_TIM ? " TIM" : "",
		     sta->flags & WLAN_STA_PERM ? " PERM" : "",
		     sta->flags & WLAN_STA_AUTHORIZED ? " AUTHORIZED" : "",
		     sta->flags & WLAN_STA_PENDING_POLL ? " POLL" : "",
		     sta->capability, sta->listen_interval);
	/* supported_rates: 500 kbit/s units with msb ignored */
	for (i = 0; i < sizeof(sta->supported_rates); i++)
		if (sta->supported_rates[i] != 0)
			p += sprintf(p, "%d%sMbps ",
				     (sta->supported_rates[i] & 0x7f) / 2,
				     sta->supported_rates[i] & 1 ? ".5" : "");
	p += sprintf(p, "\njiffies=%lu\nlast_auth=%lu\nlast_assoc=%lu\n"
		     "last_rx=%lu\nlast_tx=%lu\nrx_packets=%lu\n"
		     "tx_packets=%lu\n"
		     "rx_bytes=%lu\ntx_bytes=%lu\nbuffer_count=%d\n"
		     "last_rx: silence=%d signal=%d rate=%d\n"
		     "tx_rate=%d\n"
		     "tx[1M]=%d\ntx[2M]=%d\ntx[5.5M]=%d\ntx[11M]=%d\n"
		     "tx[6M]=%d\ntx[9M]=%d\ntx[12M]=%d\ntx[18M]=%d\n"
		     "tx[24M]=%d\ntx[36M]=%d\ntx[48M]=%d\ntx[54M]=%d\n"
		     "rx[1M]=%d\nrx[2M]=%d\nrx[5.5M]=%d\nrx[11M]=%d\n"
		     "rx[6M]=%d\nrx[9M]=%d\nrx[12M]=%d\nrx[18M]=%d\n"
		     "rx[24M]=%d\nrx[36M]=%d\nrx[48M]=%d\nrx[54M]=%d\n",
		     jiffies, sta->last_auth, sta->last_assoc, sta->last_rx,
		     sta->last_tx,
		     sta->rx_packets, sta->tx_packets, sta->rx_bytes,
		     sta->tx_bytes, skb_queue_len(&sta->tx_buf),
		     sta->last_rx_silence,
		     sta->last_rx_signal, sta->last_rx_rate,
		     sta->tx_rate, sta->tx_count[0], sta->tx_count[1],
		     sta->tx_count[2], sta->tx_count[3], sta->tx_count[5],
		     sta->tx_count[6], sta->tx_count[7], sta->tx_count[8],
		     sta->tx_count[9], sta->tx_count[10], sta->tx_count[11],
		     sta->tx_count[12], sta->rx_count[0], sta->rx_count[1],
		     sta->rx_count[2], sta->rx_count[3], sta->rx_count[5],
		     sta->rx_count[6], sta->rx_count[7], sta->rx_count[8],
		     sta->rx_count[9], sta->rx_count[10], sta->rx_count[11],
		     sta->rx_count[12]);
#if 0
	if (sta->crypt && sta->crypt->ops)
		p += sprintf(p, "crypt=%s\n", sta->crypt->ops->name);
#endif
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	if (sta->ap) {
		if (sta->u.ap.channel >= 0)
			p += sprintf(p, "channel=%d\n", sta->u.ap.channel);
		p += sprintf(p, "ssid=");
		for (i = 0; i < sta->u.ap.ssid_len; i++)
			p += sprintf(p, ((sta->u.ap.ssid[i] >= 32 &&
					  sta->u.ap.ssid[i] < 127) ?
					 "%c" : "<%02x>"),
				     sta->u.ap.ssid[i]);
		p += sprintf(p, "\n");
	}
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

	if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
	return (p - page);
}


#ifndef MVWLAN_NO_PROCFS_DEBUG
static int mvWLAN_ap_debug_proc_read(char *page, char **start, off_t off,
				     int count, int *eof, void *data)
{
	char *s, *p = page;
	struct ap_data *ap = (struct ap_data *) data;
	local_info_t *local = (local_info_t *) ap->local;
	MIB_PRIVACY_TABLE *mib_Privacy_p =
			&(local->sysConfig->Mib802dot11->Privacy);

	if (off != 0) {
		*eof = 1;
		return 0;
	}

	p += sprintf(p, "num_sta=%u\n", ap->num_sta);
	p += sprintf(p, "ps_sta=%u\n", ap->ps_sta);
	p += sprintf(p, "b_only_sta=%u\n", ap->b_only_sta);
	p += sprintf(p, "b_sta_around_cnt=%u\n", ap->b_sta_around_cnt);
	p += sprintf(p, "b_ap_cnt=%u\n", ap->b_ap_cnt);
	p += sprintf(p, "ap_erp_cnt=%u\n", ap->ap_erp_cnt);
	p += sprintf(p, "barker_preamble_set=%u\n", ap->barker_preamble_set);
	p += sprintf(p, "BridgedUnicastFrames=%u\n", ap->bridged_unicast);
	p += sprintf(p, "BridgedMulticastFrames=%u\n", ap->bridged_multicast);
	p += sprintf(p, "max_inactivity=%u\n", ap->max_inactivity / HZ);
	p += sprintf(p, "bridge_packets=%u\n", ap->bridge_packets);
	p += sprintf(p, "nullfunc_ack=%u\n", ap->nullfunc_ack);
	p += sprintf(p, "autom_ap_wds=%u\n", ap->autom_ap_wds);
	if (! mib_Privacy_p->PrivInvoked) {
		s = "open system";
	}
	else {
		if (mib_Privacy_p->ExcludeUnencrypt)
			s = "shared key";
		else
			s = "open system/shared key";
	}
	p += sprintf(p, "auth_algs=%s\n", s);
    p += sprintf(p, "sta_bridge=%u\n", ap->sta_bridge);

	return (p - page);
}

extern txBcnInfo_t *BcnTxInfo;
extern txBcnInfo_t *PrbRspTxInfo;

static int mvWLAN_debug_proc_read(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;
	MIB_STA_CFG *mib_StaCfg_p =
			&(local->sysConfig->Mib802dot11->StationConfig);
	MIB_OP_DATA *mib_OpData_p =
			&(local->sysConfig->Mib802dot11->OperationTable);
	MIB_PRIVACY_TABLE *mib_Privacy_p =
			&(local->sysConfig->Mib802dot11->Privacy);
	MIB_RSNCONFIG *mib_RSNConfig_p =
			&(local->sysConfig->Mib802dot11->RSNConfig);
	MIB_RSNCONFIG_UNICAST_CIPHERS *mib_RSNConfigUnicast_p =
			&(local->sysConfig->Mib802dot11->UnicastCiphers);
	MIB_PHY_DSSS_TABLE *mib_PhyDSSSTable_p =
			&(local->sysConfig->Mib802dot11->PhyDSSSTable);
#ifdef AP_WPA2
	MIB_RSNCONFIGWPA2 *mib_RSNConfigWPA2_p =
			&(local->sysConfig->Mib802dot11->RSNConfigWPA2);
	MIB_RSNCONFIGWPA2_UNICAST_CIPHERS *mib_RSNConfigWPA2Unicast_p =
			&(local->sysConfig->Mib802dot11->WPA2UnicastCiphers);
	MIB_RSNCONFIGWPA2_UNICAST_CIPHERS *mib_RSNConfigWPA2Unicast2_p =
			&(local->sysConfig->Mib802dot11->WPA2UnicastCiphers2);
#endif

    if (off > PROC_LIMIT) {
		*eof = 1;
		return 0;
	}

	p += sprintf(p, "wireless_enable=%d\n", local->wireless_enable);
	p += sprintf(p, "iw_mode=%d\n", local->iw_mode);
	p += sprintf(p, "ap_mode=%d\n", local->mib_ApMode);
	p += sprintf(p, "ssid=%s\n", mib_StaCfg_p->DesiredSsId);
	p += sprintf(p, "channel=%d\n", mib_PhyDSSSTable_p->CurrChan);
#ifdef TURBO_SETUP
	{
	char *disp_str[3] = { "Start", "Finish", "Disable" };

	p += sprintf(p, "auto_link=%s\n", disp_str[mib_StaCfg_p->mib_TSMode/2]);
	p += sprintf(p, "ssid_patchdisable=%d\n", mib_StaCfg_p->mib_SSIDPatchDisable);
	}
#endif
#ifdef BOOSTER_MODE
	p += sprintf(p, "speed_booster=%d\n", mib_StaCfg_p->mib_BoosterMode);
	p += sprintf(p, "booster_mode_on=%d\n", MVWLAN_BOOSTER_GetBoosterMode(local));
#endif
	p += sprintf(p, "erp_protected=%d\n", local->mib_ErpProtEnabled);
	p += sprintf(p, "wds_max_connections=%d\n",
		     local->wds_max_connections);
	p += sprintf(p, "dev_enabled=%d\n", local->dev_enabled);
	p += sprintf(p, "sw_tick_stuck=%d\n", local->sw_tick_stuck);
	if (local->crypt && local->crypt->ops)
		p += sprintf(p, "crypt=%s\n", local->crypt->ops->name);
	p += sprintf(p, "beacon_int=%d\n", mib_StaCfg_p->BcnPeriod);
	p += sprintf(p, "dtim_period=%d\n", mib_StaCfg_p->DtimPeriod);
	p += sprintf(p, "rts_threshold=%d\n", mib_OpData_p->RtsThresh);
	p += sprintf(p, "frag_threshold=%d\n", mib_OpData_p->FragThresh);
	p += sprintf(p, "short_retry_limit=%d\n", mib_OpData_p->ShortRetryLim);
	p += sprintf(p, "long_retry_limit=%d\n", mib_OpData_p->LongRetryLim);
	p += sprintf(p, "privacy_invoked=%d\n", mib_Privacy_p->PrivInvoked);
	p += sprintf(p, "exclude_unencrypt=%d\n", mib_Privacy_p->ExcludeUnencrypt);
	if (mib_Privacy_p->PrivInvoked) {
		p += sprintf(p, "wep_default_key_idx=%d\n", mib_Privacy_p->WepDefaultKeyId);
	}
#ifndef AP_WPA2
	p += sprintf(p, "wpa_mode=%d\n", mib_Privacy_p->RSNEnabled);
	p += sprintf(p, "wpa_encry=%d\n", mib_RSNConfig_p->MulticastCipher[3]);
	p += sprintf(p, "wpa_group_rekey_time=%d\n", (int) mib_RSNConfig_p->GroupRekeyTime);
	p += sprintf(p, "wpa_passphrase=%s\n", mib_RSNConfig_p->PSKPassPhrase);
#else
	if (mib_Privacy_p->RSNEnabled) {
		if (!mib_RSNConfigWPA2_p->WPA2Enabled && !mib_RSNConfigWPA2_p->WPA2OnlyEnabled) {
			p += sprintf(p, "rsn_mode=%s\n", "WPA");
			p += sprintf(p, "wpa_unicast_encry=%d\n", mib_RSNConfigUnicast_p->UnicastCipher[3]);
			p += sprintf(p, "wpa_passphrase=%s\n", mib_RSNConfig_p->PSKPassPhrase);
			p += sprintf(p, "wpa_multicast_encry=%d\n", mib_RSNConfig_p->MulticastCipher[3]);
		} else if (mib_RSNConfigWPA2_p->WPA2Enabled) {
			p += sprintf(p, "rsn_mode=%s\n", "WPA_WPA2_Mixed");
			p += sprintf(p, "wpa_unicast_encry=%d\n", mib_RSNConfigUnicast_p->UnicastCipher[3]);
			p += sprintf(p, "wpa_passphrase=%s\n", mib_RSNConfig_p->PSKPassPhrase);
			p += sprintf(p, "wpa2_unicast_encry=%d\n", mib_RSNConfigWPA2Unicast_p->UnicastCipher[3]);
			p += sprintf(p, "wpa2_unicast_encry2=%d\n", mib_RSNConfigWPA2Unicast2_p->UnicastCipher[3]);
			p += sprintf(p, "wpa2_passphrase=%s\n", mib_RSNConfigWPA2_p->PSKPassPhrase);
			p += sprintf(p, "multicast_encry=%d\n", mib_RSNConfig_p->MulticastCipher[3]);
		} else {
			p += sprintf(p, "rsn_mode=%s\n", "WPA2 Only");
			p += sprintf(p, "wpa2_unicast_encry=%d\n", mib_RSNConfigWPA2Unicast_p->UnicastCipher[3]);
			p += sprintf(p, "wpa2_unicast_encry2=%d\n", mib_RSNConfigWPA2Unicast2_p->UnicastCipher[3]);
			p += sprintf(p, "wpa2_passphrase=%s\n", mib_RSNConfigWPA2_p->PSKPassPhrase);
			p += sprintf(p, "wpa2_multicast_encry=%d\n", mib_RSNConfigWPA2_p->MulticastCipher[3]);
		}
		p += sprintf(p, "group_rekey_time=%d\n", (int) mib_RSNConfig_p->GroupRekeyTime);
	} else {
		p += sprintf(p, "wpa_mode=%s\n", "None");
	}
#endif
	p += sprintf(p, "wlan_tx_gpio=%d\n", local->wlan_tx_gpio);
	p += sprintf(p, "wlan_rx_gpio=%d\n", local->wlan_rx_gpio);
	{
	extern UINT8 mib_rxAntenna;

	p += sprintf(p, "antenna=%d\n", mib_rxAntenna);
	}
	p += sprintf(p, "watchdog_timer=%d\n", local->watchdog_timer);
	p += sprintf(p, "beacon_tx_rate=%d\n", BcnTxInfo->Rate);
	p += sprintf(p, "probe_rsp_rate=%d\n", PrbRspTxInfo->Rate);

    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
	return (p - page);
}
#endif /* MVWLAN_NO_PROCFS_DEBUG */


static int mvWLAN_stats_proc_read(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;
	MIB_COUNTERS *mib_Counters_p =
			&(local->sysConfig->Mib802dot11->CountersTable);
	u32 value = 0;

    if (off > PROC_LIMIT) {
		*eof = 1;
		return 0;
	}

	p += sprintf(p, "TxFrames=%u\n", (u32) mib_Counters_p->TxFrmCnt);
	p += sprintf(p, "TxMulticastframes=%u\n",
		     (u32) mib_Counters_p->MulticastTxFrmCnt);
	p += sprintf(p, "TxFragments=%u\n", value);
	p += sprintf(p, "TxSingleRetryFrames=%u\n",
		     value);
	p += sprintf(p, "TxMultipleRetryFrames=%u\n",
		     value);
	p += sprintf(p, "TxRetryLimitExceeded=%u\n",
		     value);
	p += sprintf(p, "TxDiscards=%u\n", value);
	p += sprintf(p, "RxUnicastFrames=%u\n", value);
	p += sprintf(p, "RxMulticastFrames=%u\n", value);
	p += sprintf(p, "RxFragments=%u\n", value);
	p += sprintf(p, "RxFCSErrors=%u\n", value);
	p += sprintf(p, "RxDiscardsNoBuffer=%u\n",
		     value);
	p += sprintf(p, "TxDiscardsWrongSA=%u\n", value);
	p += sprintf(p, "RxDiscardsWEPUndecryptable=%u\n",
		     value);
	p += sprintf(p, "RxMessageInMsgFragments=%u\n",
		     value);
	p += sprintf(p, "RxMessageInBadMsgFragments=%u\n",
		     value);
	/* FIX: this may grow too long for one page(?) */

	if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
	return (p - page);
}


static int mvWLAN_wds_proc_read(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;
	mvwlan_wds_info_t *wds;
	unsigned long flags;

	if (off > PROC_LIMIT) {
		*eof = 1;
		return 0;
	}

	spin_lock_irqsave(&local->wdslock, flags);
	wds = local->wds;
	while (wds != NULL) {
		p += sprintf(p, "%s\t" MACSTR "\n",
			     wds->dev.name, MAC2STR(wds->remote_addr));
		if ((p - page) > PROC_LIMIT) {
			printk(KERN_DEBUG "%s: wds proc did not fit\n",
			       local->dev->name);
			break;
		}
		wds = wds->next;
	}
	spin_unlock_irqrestore(&local->wdslock, flags);

	if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}

	return (p - page);
}


void mvWLAN_init_dev_proc(local_info_t *local)
{
	if (mvWLAN_proc == NULL) {
		printk(KERN_WARNING "%s: mvWLAN proc directory not created\n",
		       local->dev->name);
		return;
	}

#ifndef MVWLAN_NO_PROCFS_DEBUG
	create_proc_read_entry("debug", 0, mvWLAN_proc,
			       mvWLAN_debug_proc_read, local);
#endif /* MVWLAN_NO_PROCFS_DEBUG */
	create_proc_read_entry("stats", 0, mvWLAN_proc,
			       mvWLAN_stats_proc_read, local);
	create_proc_read_entry("wds", 0, mvWLAN_proc,
			       mvWLAN_wds_proc_read, local);
	create_proc_read_entry("registers", 0, mvWLAN_proc,
			       MVWLAN_PROC_ReadAllRegisters, local);
	create_proc_read_entry("dump_mac1", 0, mvWLAN_proc,
			       MVWLAN_PROC_DumpMAC1Registers, local);
	create_proc_read_entry("dump_mac2", 0, mvWLAN_proc,
			       MVWLAN_PROC_DumpMAC2Registers, local);
	create_proc_read_entry("dump_mac3", 0, mvWLAN_proc,
			       MVWLAN_PROC_DumpMAC3Registers, local);
	create_proc_read_entry("dump_bb", 0, mvWLAN_proc,
			       MVWLAN_PROC_DumpBBRegisters, local);
	create_proc_read_entry("dump_rf", 0, mvWLAN_proc,
			       MVWLAN_PROC_DumpRFRegisters, local);
    create_proc_read_entry("mfg", 0, mvWLAN_proc,
			       MVWLAN_PROC_DumpMFGData, local);
}


void mvWLAN_remove_dev_proc(local_info_t *local)
{
	if (mvWLAN_proc != NULL) {
		remove_proc_entry("dump_rf", mvWLAN_proc);
		remove_proc_entry("dump_bb", mvWLAN_proc);
		remove_proc_entry("dump_mac3", mvWLAN_proc);
		remove_proc_entry("dump_mac2", mvWLAN_proc);
		remove_proc_entry("dump_mac1", mvWLAN_proc);
		remove_proc_entry("registers", mvWLAN_proc);
		remove_proc_entry("wds", mvWLAN_proc);
		remove_proc_entry("stats", mvWLAN_proc);
#ifndef MVWLAN_NO_PROCFS_DEBUG
		remove_proc_entry("debug", mvWLAN_proc);
#endif /* MVWLAN_NO_PROCFS_DEBUG */
	}
}


static void mvWLAN_handle_add_proc_queue(void *data)
{
	struct ap_data *ap = (struct ap_data *) data;
	struct sta_info *sta;
	char name[20];
	struct add_sta_proc_data *entry, *prev;

	entry = ap->add_sta_proc_entries;
	ap->add_sta_proc_entries = NULL;

	while (entry) {
		spin_lock_bh(&ap->sta_table_lock);
		sta = mvWLAN_ap_get_sta(ap, entry->addr);
		if (sta)
			atomic_inc(&sta->users);
		spin_unlock_bh(&ap->sta_table_lock);

		if (sta) {
			sprintf(name, MACSTR, MAC2STR(sta->addr));
			sta->proc = create_proc_read_entry(
				name, 0, ap->proc,
				mvWLAN_sta_proc_read, sta);

			atomic_dec(&sta->users);
		}

		prev = entry;
		entry = entry->next;
		kfree(prev);
	}

#ifndef NEW_MODULE_CODE
	MOD_DEC_USE_COUNT;
#endif
}


void mvWLAN_init_ap_proc(local_info_t *local)
{
	local->ap->proc = proc_mkdir(local->dev->name, mvWLAN_proc);

	if (! mvWLAN_proc) {
		printk(KERN_WARNING "Failed to mkdir "
		       "/proc/net/mvwlan/%s\n", local->dev->name);
	} else {
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
		create_proc_read_entry("ap", 0, local->ap->proc,
				       mvWLAN_ap_proc_read,
				       local->ap);

		create_proc_read_entry("ap_control", 0, local->ap->proc,
				       mvWLAN_ap_control_proc_read,
				       local->ap);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

#ifndef MVWLAN_NO_PROCFS_DEBUG
		create_proc_read_entry("ap_debug", 0, local->ap->proc,
				       mvWLAN_ap_debug_proc_read,
				       local->ap);
#endif /* MVWLAN_NO_PROCFS_DEBUG */

		MVWLAN_QUEUE_INIT(&local->ap->add_sta_proc_queue,
				  mvWLAN_handle_add_proc_queue, local->ap);
	}
}


void mvWLAN_remove_ap_proc(local_info_t *local)
{
	if (local->ap->proc != NULL) {
#ifndef MVWLAN_NO_PROCFS_DEBUG
		remove_proc_entry("ap_debug", local->ap->proc);
#endif /* MVWLAN_NO_PROCFS_DEBUG */

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
		remove_proc_entry("ap_control", local->ap->proc);

		remove_proc_entry("ap", local->ap->proc);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

		if (local->dev->name != NULL && mvWLAN_proc != NULL)
			remove_proc_entry(local->dev->name, mvWLAN_proc);
	}
}


void mvWLAN_init_proc(void)
{
	if (proc_net != NULL) {
		mvWLAN_proc = proc_mkdir("mvwlan", proc_net);
		if (! mvWLAN_proc) {
			printk(KERN_WARNING "Failed to mkdir "
			       "/proc/net/mvwlan\n");
		} else {
			mvWLAN_apdaemon_ctrl = create_proc_entry("apdio",
								 0666 ,
								 mvWLAN_proc);
			if (! mvWLAN_apdaemon_ctrl) {
				printk(KERN_WARNING "Failed to mkdir "
				       "/proc/net/mvwlan/apdio\n");
			} else {
				mvWLAN_apdaemon_ctrl->read_proc =
					mvWLAN_apd_ctrl_read;
				mvWLAN_apdaemon_ctrl->write_proc =
					mvWLAN_apd_ctrl_write;
				mvWLAN_apdaemon_ctrl->nlink = 1;
			}
		}
	} else {
		mvWLAN_proc = NULL;
	}
}


void mvWLAN_remove_proc(void)
{
	if (mvWLAN_proc != NULL) {
		remove_proc_entry("apdio", mvWLAN_proc);
		remove_proc_entry("mvwlan", proc_net);
		mvWLAN_proc = NULL;
	}
}


/* PRIVATE FUNCTION DEFINITION
 */

static int MVWLAN_PROC_ReadAllRegisters(char *page, char **start, off_t off, int count, int *eof, void *data)
{
   char *p = page;

    if (off > PROC_LIMIT) {
      *eof = 1;
      return 0;
   }

   p = MVWLAN_PROC_ReadCPURegisters(p);
   p = MVWLAN_PROC_Read11GMACRegisters(p);
   p = MVWLAN_PROC_ReadBBPRegisters(p);
   p = MVWLAN_PROC_ReadRFRegisters(p);

    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
   return (p - page);
}


typedef struct
{
   unsigned short reg_offset;
   char           description[20];

} MVWLAN_PROC_REGS_SET;


MVWLAN_PROC_REGS_SET cpu_regs[] =
{
   { 0x0000, "Sys config & ctrl  " },
   { 0x000C, "Clock enable reg   " },
   { 0x001C, "Advn feature ctrl 1" },
   { 0x0020, "Advn feature ctrl 2" },
   { 0x0024, "Arbiter advn ctrl 1" },
   { 0x0028, "Arbiter advn ctrl 2" },
   { 0x002C, "Arbiter advn ctrl 3" },
   { 0x0030, "Sys clock ctrl reg " }
};

#define CPU_REGS_NO   (sizeof(cpu_regs)/sizeof(MVWLAN_PROC_REGS_SET))


static char *MVWLAN_PROC_ReadCPURegisters(char *start)
{
   char *p = start;
   int  reg_idx;

   p += sprintf(p, "\nCPU Regs:\n\n");

   for (reg_idx = 0; reg_idx < CPU_REGS_NO; reg_idx++)
   {
      p += sprintf(p, "%s (%04x): 0x%08x\n", cpu_regs[reg_idx].description, cpu_regs[reg_idx].reg_offset,
                                             (unsigned int) WL_REGS32(WL_CIU_CFG_BASE | cpu_regs[reg_idx].reg_offset));

      udelay(1);
   }

   return p;
}


MVWLAN_PROC_REGS_SET mac_regs[] =
{
   { 0x0510, "Interrupt status   " },
   { 0x0514, "Interrupt mask     " },
   { 0x0518, "Reset select       " },
   { 0x0824, "MTD bypass register" },
   { 0x0828, "MTD bypass value   " },
   { 0x0880, "TX test pattern    " },
   { 0x0884, "TX test length     " },
   { 0x0888, "TX test rate       " },
   { 0x088C, "TX test control    " },
   { 0x046C, "Frame block        " },
   { 0x0444, "RX_DNF_WR_PTR      " },
   { 0x0430, "RX_DNF_RD_PTR      " },
   { 0x0454, "RX_MNF_WR_PTR      " },
   { 0x0440, "RX_MNF_RD_PTR      " }
};

#define MAC_REGS_NO   (sizeof(mac_regs)/sizeof(MVWLAN_PROC_REGS_SET))


static char *MVWLAN_PROC_Read11GMACRegisters(char *start)
{
   char *p = start;
   int  reg_idx;

   p += sprintf(p, "\nMAC Regs:\n\n");

   for (reg_idx = 0; reg_idx < MAC_REGS_NO; reg_idx++)
   {
      p += sprintf(p, "%s (%04x): 0x%08x\n", mac_regs[reg_idx].description, mac_regs[reg_idx].reg_offset,
                                             (unsigned int) WL_REGS32(MAC_REG_ADDR(mac_regs[reg_idx].reg_offset)));

      udelay(1);
   }

   return p;
}


MVWLAN_PROC_REGS_SET bbp_regs[] =
{
   { 0x0001, "BB ID register     " },
   { 0x0002, "BB ID register low " },
   { 0x0007, "device control 0   " },
   { 0x0008, "receive control 0  " },
   { 0x0009, "receive control 1  " },
   { 0x000A, "receive control 2  " },
   { 0x000B, "receive control 3  " },
   { 0x000C, "receive control 4  " },
   { 0x000D, "receive control 5  " },
   { 0x000E, "receive control 6  " },
   { 0x0011, "transmit control 0 " },
   { 0x0012, "transmit control 1 " },
   { 0x0013, "transmit control 2 " },
   { 0x0014, "transmit control 3 " },
   { 0x0015, "transmit control 4 " },
   { 0x0016, "transmit control 5 " },
   { 0x0017, "transmit control 6 " },
   { 0x0018, "transmit control 7 " },
   { 0x0019, "transmit control 8 " },
   { 0x001A, "transmit control 9 " },
   { 0x001B, "transmit control 10" },
   { 0x001C, "transmit control 11" },
   { 0x003F, "Rx ANT.            " },
   { 0x0040, "Tx ANT.            " }
};

#define BBP_REGS_NO   (sizeof(bbp_regs)/sizeof(MVWLAN_PROC_REGS_SET))


static char *MVWLAN_PROC_ReadBBPRegisters(char *start)
{
   char *p = start;
   int  reg_idx;
   unsigned char reg_addr, value8;

   sys_InitBBPAccess();

   p += sprintf(p, "\nBBP Regs:\n\n");

   for (reg_idx = 0; reg_idx < BBP_REGS_NO; reg_idx++)
   {
      reg_addr = bbp_regs[reg_idx].reg_offset;

      msi_wl_WriteBBP(0, reg_addr);
      msi_wl_ReadBBP(reg_addr, &value8);

      p += sprintf(p, "%s (%02x): 0x%02x\n", bbp_regs[reg_idx].description, reg_addr, value8);

      udelay(1);
   }

   return p;
}


MVWLAN_PROC_REGS_SET rf_regs[] =
{
   { 0x0001, "RF ID register     " },
   { 0x0005, "mode register      " },
   { 0x0006, "calibration reg    " },
   { 0x000E, "receive peak detect" },
   { 0x001A, "power" },
   { 0x001B, "power" },
   { 0x001C, "power" },
   { 0x001D, "power" },
   { 0x001E, "power" },
   { 0x001F, "power" }, 
   { 0x0020, "transmit PA control" },
   { 0x0021, "transmit PA gain PW" },
   { 0x0022, "transmit PA current" },
   { 0x0023, "transmit VGA gain E" },
   { 0x0024, "transmit VGA gain I" },
   { 0x0025, "transmit PA gain   " },
   { 0x0026, "transmit pre-drive " },
   { 0x0029, "polarity control   " }
};

#define RF_REGS_NO   (sizeof(rf_regs)/sizeof(MVWLAN_PROC_REGS_SET))


static char *MVWLAN_PROC_ReadRFRegisters(char *start)
{
   char *p = start;
   int  reg_idx;
   unsigned char reg_addr, value8;

   sys_InitPHYAccess();

   p += sprintf(p, "\nRF Regs:\n\n");

   for (reg_idx = 0; reg_idx < RF_REGS_NO; reg_idx++)
   {
      reg_addr = rf_regs[reg_idx].reg_offset;

      msi_wl_ReadPHY(reg_addr, &value8);

      p += sprintf(p, "%s (%02x): 0x%02x\n", rf_regs[reg_idx].description, reg_addr, value8);

      udelay(1);
   }

   return p;
}


static int MVWLAN_PROC_DumpMAC1Registers(char *page, char **start, off_t off, int count, int *eof, void *data)
{
   char *p = page;
   int i;

   if (off > PROC_LIMIT) {
      *eof = 1;
      return 0;
   }

   p += sprintf(p, "macdump 0x8000a000 0x8000a2fc\n");

   for (i = 0; i < 0x300; i += 4)
   {
      p += sprintf(p, "0x%08x\n", (unsigned int) WL_REGS32(MAC_REG_ADDR((0x8000a000+i))));

      udelay(1);
   }

    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
   return (p - page);
}


static int MVWLAN_PROC_DumpMAC2Registers(char *page, char **start, off_t off, int count, int *eof, void *data)
{
   char *p = page;
   int i;

   if (off > PROC_LIMIT) {
      *eof = 1;
      return 0;
   }

   p += sprintf(p, "macdump 0x8000a300 0x8000a5fc\n");

   for (i = 0; i < 0x300; i += 4)
   {
      p += sprintf(p, "0x%08x\n", (unsigned int) WL_REGS32(MAC_REG_ADDR((0x8000a300+i))));

      udelay(1);
   }

    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
   return (p - page);
}


static int MVWLAN_PROC_DumpMAC3Registers(char *page, char **start, off_t off, int count, int *eof, void *data)
{
   char *p = page;
   int i;

   if (off > PROC_LIMIT) {
      *eof = 1;
      return 0;
   }

   p += sprintf(p, "macdump 0x8000a600 0x8000a900\n");

   for (i = 0; i <= 0x300; i += 4)
   {
      p += sprintf(p, "0x%08x\n", (unsigned int) WL_REGS32(MAC_REG_ADDR((0x8000a600+i))));

      udelay(1);
   }

    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
   return (p - page);
}


static int MVWLAN_PROC_DumpBBRegisters(char *page, char **start, off_t off, int count, int *eof, void *data)
{
   char *p = page;
   unsigned char value8;
   int i;

    if (off > PROC_LIMIT) {
      *eof = 1;
      return 0;
   }

   sys_InitBBPAccess();

   p += sprintf(p, "bbdump 0x00 0xff\n");

   for (i = 0; i <= 0xff; i++)
   {
      msi_wl_WriteBBP(0, i);
      msi_wl_ReadBBP(i, &value8);

      p += sprintf(p, "0x%02x\n", value8);

      udelay(1);
   }

    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
   return (p - page);
}


static int MVWLAN_PROC_DumpRFRegisters(char *page, char **start, off_t off, int count, int *eof, void *data)
{
   char *p = page;
   unsigned char value8;
   int i;

   if (off > PROC_LIMIT) {
      *eof = 1;
      return 0;
   }

   sys_InitPHYAccess();

   p += sprintf(p, "phydump 0x00 0x49\n");

   for (i = 0; i <= 0x49; i++)
   {
      msi_wl_ReadPHY(i, &value8);

      p += sprintf(p, "0x%02x\n", value8);

      udelay(1);
   }

    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
   return (p - page);
}

static int MVWLAN_PROC_DumpMFGData(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    char *p = page;
    local_info_t *local = (local_info_t *) data;
    MFG_CAL_DATA *mfg_data = local->sysConfig->CalData;
    int i;
    
    if (off > PROC_LIMIT) {
		*eof = 1;
		return 0;
	}
    
    p += sprintf(p, "%c%c %d %d %c%c%c %d\n", mfg_data->BrdDscrpt[0],
                                              mfg_data->BrdDscrpt[1],
                                              mfg_data->BrdDscrpt[2],
                                              mfg_data->BrdDscrpt[3],
                                              mfg_data->BrdDscrpt[4],
                                              mfg_data->BrdDscrpt[5],
                                              mfg_data->BrdDscrpt[6],
                                              mfg_data->BrdDscrpt[7]);
    p += sprintf(p, "REV=%#x\n", mfg_data->Rev);
    p += sprintf(p, "IntPaCalTabOpt=%#x\n", mfg_data->PaOpt.IntPaCalTabOpt);
    p += sprintf(p, "ant2NotCalSep=%#x\n", mfg_data->PaOpt.ant2NotCalSep);
	p += sprintf(p, "ExtPaNegPol=%#x\n", mfg_data->PaOpt.ExtPaNegPol);
	p += sprintf(p, "ExtPaUsed=%#x\n", mfg_data->PaOpt.ExtPaUsed);
	p += sprintf(p, "ExtPAp=%#x\n", mfg_data->ExtPAp[1]);
    p += sprintf(p, "Antp=%#x\n", mfg_data->Antp);    
    for (i=0; i<SZ_INTERNAL_PA_CFG; i++)
        p += sprintf(p, "Pa[%d]:C1 = %#x C0 = %#x\n", i+1, mfg_data->Pa[i].Delta.CalVal_C1, mfg_data->Pa[i].Delta.CalVal_C0);
    p += sprintf(p, "PaConfig:Reg_1E=%#x\n", mfg_data->PaConfig.Reg_1E);
    p += sprintf(p, "PaConfig:Reg_1F=%#x\n", mfg_data->PaConfig.Reg_1F);
    p += sprintf(p, "PaConfig:Reg_20=%#x\n", mfg_data->PaConfig.Reg_20);
    p += sprintf(p, "PaConfig:Reg_1A=%#x\n", mfg_data->PaConfig.Reg_1A);
    p += sprintf(p, "PaConfig:Reg_1D_0=%#x\n", mfg_data->PaConfig.Reg_1D_0);
    p += sprintf(p, "PaConfig:_2CalValTbl=%#x\n", mfg_data->PaConfig._2CalValTbl);
    p += sprintf(p, "AssmSN:AssmCode=%#x\n", mfg_data->AssmSN.AssmCode);
    p += sprintf(p, "AssmSN:SerialNum=%d\n", mfg_data->AssmSN.SerialNum);
    p += sprintf(p, "Domainp=%#x\n", mfg_data->Domainp);
    p += sprintf(p, "ECO=%#x\n", mfg_data->ECO);
    p += sprintf(p, "LCT_cal:LctCalCh11=%#x\n", mfg_data->LCT_cal.LctCalCh11);
    p += sprintf(p, "LCT_cal:LctCalCh4=%#x\n", mfg_data->LCT_cal.LctCalCh4);
    p += sprintf(p, "LCT_cal:Valid0=%#x\n", mfg_data->LCT_cal.Valid0);
    p += sprintf(p, "LCT_cal:Valid1=%#x\n", mfg_data->LCT_cal.Valid1);
    p += sprintf(p, "SocVer=%#x\n", mfg_data->SocVer);
    p += sprintf(p, "RFVer=%#x\n", mfg_data->RFVer);
    p += sprintf(p, "SPISize=%#x\n", mfg_data->SPISize);
    p += sprintf(p, "AntPart=%#x\n", mfg_data->AntPart);
    p += sprintf(p, "TestVer=%#x\n", mfg_data->TestVer);
    p += sprintf(p, "DllVer=%#x\n", mfg_data->DllVer);
    p += sprintf(p, "Xoscp=%#x\n", mfg_data->Xoscp);
    if ((p - page) <= off) {
		*eof = 1;
		return 0;
	}
	
	return (p - page);
}


EXPORT_SYMBOL(mvWLAN_init_dev_proc);
EXPORT_SYMBOL(mvWLAN_remove_dev_proc);
EXPORT_SYMBOL(mvWLAN_init_ap_proc);
EXPORT_SYMBOL(mvWLAN_remove_ap_proc);
EXPORT_SYMBOL(mvWLAN_init_proc);
EXPORT_SYMBOL(mvWLAN_remove_proc);
