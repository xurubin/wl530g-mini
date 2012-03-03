/*
 * Marvell Libertas Wireless driver with Host AP (software access point)
 * support.
 *
 * Host AP driver was originally developed by:
 * Copyright (c) 2001-2002, SSH Communications Security Corp and Jouni Malinen
 * <jkmaline@cc.hut.fi>
 * Copyright (c) 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This file is where S/W AP functionality is implemented.
 *
 * AP:  FIX:
 * - if unicast Class 2 (assoc,reassoc,disassoc) frame received from
 *   unauthenticated STA, send deauth. frame (8802.11: 5.5)
 * - if unicast Class 3 (data with to/from DS,deauth,pspoll) frame received
 *   from authenticated, but unassoc STA, send disassoc frame (8802.11: 5.5)
 * - if unicast Class 3 received from unauthenticated STA, send deauth. frame
 *   (8802.11: 5.5)
 */
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
#include <linux/rtnetlink.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN_crypt.h"
#include "mvWLAN.h"
#include "mvWLAN_hw.h"
#include "mvWLAN_rate.h"


static int other_ap_policy[MAX_PARM_DEVICES] = { AP_OTHER_AP_SKIP_ALL,
						 DEF_INTS };
MODULE_PARM(other_ap_policy, PARM_MIN_MAX "i");
MODULE_PARM_DESC(other_ap_policy, "Other AP beacon monitoring policy (0-3)");

static int ap_max_inactivity[MAX_PARM_DEVICES] = { AP_MAX_INACTIVITY / HZ,
						   DEF_INTS };
MODULE_PARM(ap_max_inactivity, PARM_MIN_MAX "i");
MODULE_PARM_DESC(ap_max_inactivity, "AP timeout (in seconds) for station "
		 "inactivity");

static int ap_bridge_packets[MAX_PARM_DEVICES] = { 1, DEF_INTS };
MODULE_PARM(ap_bridge_packets, PARM_MIN_MAX "i");
MODULE_PARM_DESC(ap_bridge_packets, "Bridge packets directly between "
		 "stations");

static int autom_ap_wds[MAX_PARM_DEVICES] = { 0, DEF_INTS };
MODULE_PARM(autom_ap_wds, PARM_MIN_MAX "i");
MODULE_PARM_DESC(autom_ap_wds, "Add WDS connections to other APs "
		 "automatically");


static void send_event_expired_sta(struct net_device *dev,
				   struct sta_info *sta)
{
#if WIRELESS_EXT >= 15
	union iwreq_data wrqu;
	memset(&wrqu, 0, sizeof(wrqu));
	memcpy(wrqu.addr.sa_data, sta->addr, ETH_ALEN);
	wrqu.addr.sa_family = ARPHRD_ETHER;
	wireless_send_event(dev, IWEVEXPIRED, &wrqu, NULL);
#endif /* WIRELESS_EXT >= 15 */
}


static void send_event_new_sta(struct net_device *dev, struct sta_info *sta)
{
#if WIRELESS_EXT >= 15
	union iwreq_data wrqu;
	memset(&wrqu, 0, sizeof(wrqu));
	memcpy(wrqu.addr.sa_data, sta->addr, ETH_ALEN);
	wrqu.addr.sa_family = ARPHRD_ETHER;
	wireless_send_event(dev, IWEVREGISTERED, &wrqu, NULL);
#endif /* WIRELESS_EXT >= 15 */
}


static void ap_sta_hash_add(struct ap_data *ap, struct sta_info *sta)
{
	sta->hnext = ap->sta_hash[STA_HASH(sta->addr)];
	ap->sta_hash[STA_HASH(sta->addr)] = sta;
}


static void ap_sta_hash_del(struct ap_data *ap, struct sta_info *sta)
{
	struct sta_info *s;

	s = ap->sta_hash[STA_HASH(sta->addr)];
	if (s == NULL) return;
	if (memcmp(s->addr, sta->addr, ETH_ALEN) == 0) {
		ap->sta_hash[STA_HASH(sta->addr)] = s->hnext;
		return;
	}

	while (s->hnext != NULL && memcmp(s->hnext->addr, sta->addr, ETH_ALEN)
	       != 0)
		s = s->hnext;
	if (s->hnext != NULL)
		s->hnext = s->hnext->hnext;
	else
		printk("AP: could not remove STA " MACSTR " from hash table\n",
		       MAC2STR(sta->addr));
}


static void ap_free_sta(struct ap_data *ap, struct sta_info *sta)
{
	struct sk_buff *skb;

	if (sta->ap && sta->local)
		send_event_expired_sta(sta->local->dev, sta);

	if (ap->proc != NULL) {
		char name[20];
		sprintf(name, MACSTR, MAC2STR(sta->addr));
		remove_proc_entry(name, ap->proc);
	}

#if 0
	if (sta->crypt) {
		sta->crypt->ops->deinit(sta->crypt->priv);
		kfree(sta->crypt);
		sta->crypt = NULL;
	}
#endif

	while ((skb = skb_dequeue(&sta->tx_buf)) != NULL)
		dev_kfree_skb(skb);

	ap->num_sta--;

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	if (sta->aid > 0)
		ap->sta_aid[sta->aid - 1] = NULL;

	if (!sta->ap && sta->u.sta.challenge)
		kfree(sta->u.sta.challenge);
	del_timer(&sta->timer);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

	kfree(sta);
}


struct set_tim_data {
	struct list_head list;
	int aid;
	int set;
};


void mvWLAN_set_tim(local_info_t *local, int aid, int set)
{
	struct list_head *ptr;
	struct set_tim_data *new_entry;

	new_entry = (struct set_tim_data *)
		kmalloc(sizeof(*new_entry), GFP_ATOMIC);
	if (new_entry == NULL) {
		printk(KERN_DEBUG "%s: mvWLAN_set_tim: kmalloc failed\n",
		       local->dev->name);
		return;
	}
	memset(new_entry, 0, sizeof(*new_entry));
	new_entry->aid = aid;
	new_entry->set = set;

	spin_lock_bh(&local->ap->set_tim_lock);
	for (ptr = local->ap->set_tim_list.next;
	     ptr != &local->ap->set_tim_list;
	     ptr = ptr->next) {
		struct set_tim_data *entry = (struct set_tim_data *) ptr;
		if (entry->aid == aid) {
			PDEBUG(DEBUG_PS2, "%s: mvWLAN_set_tim: aid=%d "
			       "set=%d ==> %d\n",
			       local->dev->name, aid, entry->set, set);
			entry->set = set;
			kfree(new_entry);
			new_entry = NULL;
			break;
		}
	}
	if (new_entry)
		list_add_tail(&new_entry->list, &local->ap->set_tim_list);
	spin_unlock_bh(&local->ap->set_tim_lock);

	MVWLAN_SCHEDULE_TASK(&local->ap->set_tim_queue);
}


static void handle_set_tim_queue(void *data)
{
	local_info_t *local = (local_info_t *) data;
	struct set_tim_data *entry;
	u32 val;

	for (;;) {
		entry = NULL;
		spin_lock_bh(&local->ap->set_tim_lock);
		if (!list_empty(&local->ap->set_tim_list)) {
			entry = list_entry(local->ap->set_tim_list.next,
					   struct set_tim_data, list);
			list_del(&entry->list);
		}
		spin_unlock_bh(&local->ap->set_tim_lock);
		if (!entry)
			break;

		PDEBUG(DEBUG_PS2, "%s: handle_set_tim_queue: aid=%d set=%d\n",
		       local->dev->name, entry->aid, entry->set);

		val = (u32) entry->aid;

		if (local->func->set_cmd(local->dev, APCMD_SET_TIM,
					 entry->set, (void *) &val,
					 sizeof(u32))) {
			printk(KERN_DEBUG "%s: set_tim failed (aid=%d "
			       "set=%d)\n",
			       local->dev->name, entry->aid, entry->set);
		}

		kfree(entry);
	}

#ifndef NEW_MODULE_CODE
	MOD_DEC_USE_COUNT;
#endif
}


#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT

static void ap_handle_timer(unsigned long data)
{
	struct sta_info *sta = (struct sta_info *) data;
	local_info_t *local;
	struct ap_data *ap;
	unsigned long next_time = 0;
	int was_assoc;

	if (sta == NULL || sta->local == NULL || sta->local->ap == NULL) {
		PDEBUG(DEBUG_AP, "ap_handle_timer() called with NULL data\n");
		return;
	}

	local = sta->local;
	ap = local->ap;
	was_assoc = sta->flags & WLAN_STA_ASSOC;

	if (atomic_read(&sta->users) != 0)
		next_time = jiffies + HZ;
	else if ((sta->flags & WLAN_STA_PERM) && !(sta->flags & WLAN_STA_AUTH))
		next_time = jiffies + ap->max_inactivity;

	if (sta->last_rx + ap->max_inactivity > jiffies) {
		/* station activity detected; reset timeout state */
		sta->timeout_next = STA_NULLFUNC;
		next_time = sta->last_rx + ap->max_inactivity;
	} else if (sta->timeout_next == STA_DISASSOC &&
		   !(sta->flags & WLAN_STA_PENDING_POLL)) {
		/* STA ACKed data nullfunc frame poll */
		sta->timeout_next = STA_NULLFUNC;
		next_time = jiffies + ap->max_inactivity;
	}

	if (next_time) {
		sta->timer.expires = next_time;
		add_timer(&sta->timer);
		return;
	}

	if (sta->ap)
		sta->timeout_next = STA_DEAUTH;

	if (sta->timeout_next == STA_DEAUTH && !(sta->flags & WLAN_STA_PERM)) {
		spin_lock(&ap->sta_table_lock);
		ap_sta_hash_del(ap, sta);
		list_del(&sta->list);
		spin_unlock(&ap->sta_table_lock);
		sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
	} else if (sta->timeout_next == STA_DISASSOC)
		sta->flags &= ~WLAN_STA_ASSOC;

	if (was_assoc && !(sta->flags & WLAN_STA_ASSOC) && !sta->ap)
		send_event_expired_sta(local->dev, sta);

	if (sta->timeout_next == STA_DEAUTH && sta->aid > 0 &&
	    !skb_queue_empty(&sta->tx_buf)) {
		mvWLAN_set_tim(local, sta->aid, 0);
		sta->flags &= ~WLAN_STA_TIM;
	}

	if (sta->ap) {
		if (ap->autom_ap_wds) {
			PDEBUG(DEBUG_AP, "%s: removing automatic WDS "
			       "connection to AP " MACSTR "\n",
			       local->dev->name, MAC2STR(sta->addr));
			mvWLAN_wds_del(local, sta->addr, 0, 1);
		}
	} else if (sta->timeout_next == STA_NULLFUNC) {
		/* send data frame to poll STA and check whether this frame
		 * is ACKed */
		/* FIX: WLAN_FC_STYPE_NULLFUNC would be more appropriate, but
		 * it is apparently not retried so TX Exc events are not
		 * received for it */
		sta->flags |= WLAN_STA_PENDING_POLL;
		mvWLAN_send_mgmt(local->dev, WLAN_FC_TYPE_DATA,
				 WLAN_FC_STYPE_DATA, NULL, 0, 1,
				 sta->addr, ap->tx_callback_poll);
	} else {
		int deauth = sta->timeout_next == STA_DEAUTH;
		u16 resp;
		PDEBUG(DEBUG_AP, "%s: sending %s info to STA " MACSTR
		       "(last=%lu, jiffies=%lu)\n",
		       local->dev->name,
		       deauth ? "deauthentication" : "disassociation",
		       MAC2STR(sta->addr), sta->last_rx, jiffies);

		resp = cpu_to_le16(deauth ? WLAN_REASON_PREV_AUTH_NOT_VALID :
				   WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY);
		mvWLAN_send_mgmt(local->dev, WLAN_FC_TYPE_MGMT,
				 (deauth ? WLAN_FC_STYPE_DEAUTH :
				  WLAN_FC_STYPE_DISASSOC),
				 (char *) &resp, 2, 1, sta->addr, 0);
	}

	if (sta->timeout_next == STA_DEAUTH) {
		if (sta->flags & WLAN_STA_PERM) {
			PDEBUG(DEBUG_AP, "%s: STA " MACSTR " would have been "
			       "removed, but it has 'perm' flag\n",
			       local->dev->name, MAC2STR(sta->addr));
		} else
			ap_free_sta(ap, sta);
		return;
	}

	if (sta->timeout_next == STA_NULLFUNC) {
		sta->timeout_next = STA_DISASSOC;
		sta->timer.expires = jiffies + AP_DISASSOC_DELAY;
	} else {
		sta->timeout_next = STA_DEAUTH;
		sta->timer.expires = jiffies + AP_DEAUTH_DELAY;
	}

	add_timer(&sta->timer);
}


void mvWLAN_deauth_all_stas(struct net_device *dev, struct ap_data *ap,
			    int resend)
{
	u8 addr[ETH_ALEN];
	u16 resp;
	int i;

	PDEBUG(DEBUG_AP, "%s: Deauthenticate all stations\n", dev->name);
	memset(addr, 0xff, ETH_ALEN);

	resp = __constant_cpu_to_le16(WLAN_REASON_PREV_AUTH_NOT_VALID);

	/* deauth message sent; try to resend it few times; the message is
	 * broadcast, so it may be delayed until next DTIM; there is not much
	 * else we can do at this point since the driver is going to be shut
	 * down */
	for (i = 0; i < 5; i++) {
		mvWLAN_send_mgmt(dev, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_DEAUTH,
				 (char *) &resp, 2, 1, addr, 0);

		if (!resend || ap->num_sta <= 0)
			return;

		mdelay(50);
	}
}


int mvWLAN_ap_control_add_mac(struct mac_restrictions *mac_restrictions,
			      u8 *mac)
{
	struct mac_entry *entry;

	entry = kmalloc(sizeof(struct mac_entry), GFP_KERNEL);
	if (entry == NULL)
		return -1;

	memcpy(entry->addr, mac, ETH_ALEN);

	spin_lock_bh(&mac_restrictions->lock);
	list_add_tail(&entry->list, &mac_restrictions->mac_list);
	mac_restrictions->entries++;
	spin_unlock_bh(&mac_restrictions->lock);

	return 0;
}


int mvWLAN_ap_control_del_mac(struct mac_restrictions *mac_restrictions,
			      u8 *mac)
{
	struct list_head *ptr;
	struct mac_entry *entry;

	spin_lock_bh(&mac_restrictions->lock);
	for (ptr = mac_restrictions->mac_list.next;
	     ptr != &mac_restrictions->mac_list; ptr = ptr->next) {
		entry = list_entry(ptr, struct mac_entry, list);

		if (memcmp(entry->addr, mac, ETH_ALEN) == 0) {
			list_del(ptr);
			kfree(entry);
			mac_restrictions->entries--;
			spin_unlock_bh(&mac_restrictions->lock);
			return 0;
		}
	}
	spin_unlock_bh(&mac_restrictions->lock);
	return -1;
}


static int ap_control_mac_deny(struct mac_restrictions *mac_restrictions,
			       u8 *mac)
{
	struct list_head *ptr;
	struct mac_entry *entry;
	int found = 0;

	if (mac_restrictions->policy == MAC_POLICY_OPEN)
		return 0;

	spin_lock_bh(&mac_restrictions->lock);
	for (ptr = mac_restrictions->mac_list.next;
	     ptr != &mac_restrictions->mac_list; ptr = ptr->next) {
		entry = list_entry(ptr, struct mac_entry, list);

		if (memcmp(entry->addr, mac, ETH_ALEN) == 0) {
			found = 1;
			break;
		}
	}
	spin_unlock_bh(&mac_restrictions->lock);

	if (mac_restrictions->policy == MAC_POLICY_ALLOW)
		return !found;
	else
		return found;
}


void mvWLAN_ap_control_flush_macs(struct mac_restrictions *mac_restrictions)
{
	struct list_head *ptr, *n;
	struct mac_entry *entry;

	if (mac_restrictions->entries == 0)
		return;

	spin_lock_bh(&mac_restrictions->lock);
	for (ptr = mac_restrictions->mac_list.next, n = ptr->next;
	     ptr != &mac_restrictions->mac_list;
	     ptr = n, n = ptr->next) {
		entry = list_entry(ptr, struct mac_entry, list);
		list_del(ptr);
		kfree(entry);
	}
	mac_restrictions->entries = 0;
	spin_unlock_bh(&mac_restrictions->lock);
}


int mvWLAN_ap_control_kick_mac(struct ap_data *ap, struct net_device *dev,
			       u8 *mac)
{
	struct sta_info *sta;
	u16 resp;

	spin_lock_bh(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, mac);
	if (sta) {
		ap_sta_hash_del(ap, sta);
		list_del(&sta->list);
	}
	spin_unlock_bh(&ap->sta_table_lock);

	if (!sta)
		return -EINVAL;

	resp = cpu_to_le16(WLAN_REASON_PREV_AUTH_NOT_VALID);
	mvWLAN_send_mgmt(dev, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_DEAUTH,
			 (char *) &resp, 2, 1, sta->addr, 0);

	if ((sta->flags & WLAN_STA_ASSOC) && !sta->ap)
		send_event_expired_sta(dev, sta);

	ap_free_sta(ap, sta);

	return 0;
}

#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */


void mvWLAN_ap_control_kickall(struct ap_data *ap)
{
	struct list_head *ptr, *n;
	struct sta_info *sta;
  
	spin_lock_bh(&ap->sta_table_lock);
	for (ptr = ap->sta_list.next, n = ptr->next; ptr != &ap->sta_list;
	     ptr = n, n = ptr->next) {
		sta = list_entry(ptr, struct sta_info, list);
		ap_sta_hash_del(ap, sta);
		list_del(&sta->list);
		if ((sta->flags & WLAN_STA_ASSOC) && !sta->ap && sta->local)
			send_event_expired_sta(sta->local->dev, sta);
		ap_free_sta(ap, sta);
	}
	spin_unlock_bh(&ap->sta_table_lock);
}


/* Called only as a tasklet (software IRQ) */
static void mvWLAN_ap_tx_cb(struct sk_buff *skb, int ok, void *data)
{
	struct ap_data *ap = data;
	u16 fc;
	struct mvwlan_ieee80211_hdr *hdr;

	if (!ap->local->hostapd || !ap->local->apdev) {
		dev_kfree_skb(skb);
		return;
	}

	hdr = (struct mvwlan_ieee80211_hdr *) skb->data;
	fc = le16_to_cpu(hdr->frame_control);

	/* Pass the TX callback frame to the AP daemon; use 802.11 header 
	 * version 1 to indicate failure (no ACK) and 2 success (frame
	 * ACKed) */

	fc &= ~WLAN_FC_PVER;
	fc |= ok ? BIT(1) : BIT(0);
	hdr->frame_control = cpu_to_le16(fc);

	skb->dev = ap->local->apdev;
	skb_pull(skb, mvWLAN_80211_get_hdrlen(fc));
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = __constant_htons(ETH_P_802_2);
	memset(skb->cb, 0, sizeof(skb->cb));
	netif_rx(skb);
}


#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
/* Called only as a tasklet (software IRQ) */
static void mvWLAN_ap_tx_cb_auth(struct sk_buff *skb, int ok, void *data)
{
	struct ap_data *ap = data;
	struct net_device *dev = ap->local->dev;
	struct mvwlan_ieee80211_hdr *hdr;
	u16 fc, *pos, auth_alg, auth_transaction, status;
	struct sta_info *sta = NULL;
	char *txt = NULL;

	if (ap->local->hostapd) {
		dev_kfree_skb(skb);
		return;
	}

	hdr = (struct mvwlan_ieee80211_hdr *) skb->data;
	fc = le16_to_cpu(hdr->frame_control);
	if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT ||
	    WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_AUTH ||
	    skb->len < IEEE80211_MGMT_HDR_LEN + 6) {
		printk(KERN_DEBUG "%s: mvWLAN_ap_tx_cb_auth received invalid "
		       "frame\n", dev->name);
		dev_kfree_skb(skb);
		return;
	}

	if (!ok) {
		txt = "frame was not ACKed";
		goto done;
	}

	spin_lock(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, hdr->addr1);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock(&ap->sta_table_lock);

	if (!sta) {
		txt = "STA not found";
		goto done;
	}

	pos = (u16 *) (skb->data + IEEE80211_MGMT_HDR_LEN);
	auth_alg = le16_to_cpu(*pos++);
	auth_transaction = le16_to_cpu(*pos++);
	status = le16_to_cpu(*pos++);
	if (status == WLAN_STATUS_SUCCESS &&
	    ((auth_alg == WLAN_AUTH_OPEN && auth_transaction == 2) ||
	     (auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 4))) {
		txt = "STA authenticated";
		sta->flags |= WLAN_STA_AUTH;
		sta->last_auth = jiffies;
	} else if (status != WLAN_STATUS_SUCCESS)
		txt = "authentication failed";

 done:
	if (sta)
		atomic_dec(&sta->users);
	if (txt) {
		PDEBUG(DEBUG_AP, "%s: " MACSTR " auth_cb - %s\n",
		       dev->name, MAC2STR(hdr->addr1), txt);
	}
	dev_kfree_skb(skb);
}


/* Called only as a tasklet (software IRQ) */
static void mvWLAN_ap_tx_cb_assoc(struct sk_buff *skb, int ok, void *data)
{
	struct ap_data *ap = data;
	struct net_device *dev = ap->local->dev;
	struct mvwlan_ieee80211_hdr *hdr;
	u16 fc, *pos, status;
	struct sta_info *sta = NULL;
	char *txt = NULL;

	if (ap->local->hostapd) {
		dev_kfree_skb(skb);
		return;
	}

	hdr = (struct mvwlan_ieee80211_hdr *) skb->data;
	fc = le16_to_cpu(hdr->frame_control);
	if (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT ||
	    (WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_ASSOC_RESP &&
	     WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_REASSOC_RESP) ||
	    skb->len < IEEE80211_MGMT_HDR_LEN + 4) {
		printk(KERN_DEBUG "%s: mvWLAN_ap_tx_cb_assoc received invalid "
		       "frame\n", dev->name);
		dev_kfree_skb(skb);
		return;
	}

	if (!ok) {
		txt = "frame was not ACKed";
		goto done;
	}

	spin_lock(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, hdr->addr1);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock(&ap->sta_table_lock);

	if (!sta) {
		txt = "STA not found";
		goto done;
	}

	pos = (u16 *) (skb->data + IEEE80211_MGMT_HDR_LEN);
	pos++;
	status = le16_to_cpu(*pos++);
	if (status == WLAN_STATUS_SUCCESS) {
		if (!(sta->flags & WLAN_STA_ASSOC))
			send_event_new_sta(dev, sta);
		txt = "STA associated";
		sta->flags |= WLAN_STA_ASSOC;
		sta->last_assoc = jiffies;
	} else
		txt = "association failed";

 done:
	if (sta)
		atomic_dec(&sta->users);
	if (txt) {
		PDEBUG(DEBUG_AP, "%s: " MACSTR " assoc_cb - %s\n",
		       dev->name, MAC2STR(hdr->addr1), txt);
	}
	dev_kfree_skb(skb);
}

/* Called only as a tasklet (software IRQ); TX callback for poll frames used
 * in verifying whether the STA is still present. */
static void mvWLAN_ap_tx_cb_poll(struct sk_buff *skb, int ok, void *data)
{
	struct ap_data *ap = data;
	struct net_device *dev = ap->local->dev;
	struct mvwlan_ieee80211_hdr *hdr;
	struct sta_info *sta;

	if (skb->len < 24)
		goto fail;
	hdr = (struct mvwlan_ieee80211_hdr *) skb->data;
	if (ok) {
		spin_lock(&ap->sta_table_lock);
		sta = mvWLAN_ap_get_sta(ap, hdr->addr1);
		if (sta)
			sta->flags &= ~WLAN_STA_PENDING_POLL;
		spin_unlock(&ap->sta_table_lock);
	} else {
		PDEBUG(DEBUG_AP, "%s: STA " MACSTR " did not ACK activity "
		       "poll frame\n", dev->name, MAC2STR(hdr->addr1));
	}

 fail:
	dev_kfree_skb(skb);
}
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */


/* caller should have mutex for AP STA list handling */
struct sta_info * mvWLAN_ap_get_sta(struct ap_data *ap, u8 *sta)
{
	struct sta_info *s;

	s = ap->sta_hash[STA_HASH(sta)];
	while (s != NULL && memcmp(s->addr, sta, ETH_ALEN) != 0)
		s = s->hnext;
	return s;
}


struct sta_info * mvWLAN_ap_add_sta(struct ap_data *ap, u8 *addr)
{
	struct sta_info *sta;

	sta = (struct sta_info *)
		kmalloc(sizeof(struct sta_info), GFP_ATOMIC);
	if (sta == NULL) {
		PDEBUG(DEBUG_AP, "AP: kmalloc failed\n");
		return NULL;
	}

	/* initialize STA info data */
	memset(sta, 0, sizeof(struct sta_info));
	sta->local = ap->local;
	skb_queue_head_init(&sta->tx_buf);
	memcpy(sta->addr, addr, ETH_ALEN);

	atomic_inc(&sta->users);
	spin_lock_bh(&ap->sta_table_lock);
	list_add(&sta->list, &ap->sta_list);
	ap->num_sta++;
	ap_sta_hash_add(ap, sta);
	spin_unlock_bh(&ap->sta_table_lock);

	if (ap->proc) {
		struct add_sta_proc_data *entry;
		/* schedule a non-interrupt context process to add a procfs
		 * entry for the STA since procfs code use GFP_KERNEL */
		entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
		if (entry) {
			memcpy(entry->addr, sta->addr, ETH_ALEN);
			entry->next = ap->add_sta_proc_entries;
			ap->add_sta_proc_entries = entry;
			MVWLAN_SCHEDULE_TASK(&ap->add_sta_proc_queue);
		} else
			printk(KERN_DEBUG "Failed to add STA proc data\n");
	}

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	init_timer(&sta->timer);
	sta->timer.expires = jiffies + ap->max_inactivity;
	sta->timer.data = (unsigned long) sta;
	sta->timer.function = ap_handle_timer;
	if (!ap->local->hostapd)
		add_timer(&sta->timer);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

	return sta;
}


int mvWLAN_ap_tx_rate_ok(int rateidx, struct sta_info *sta,
			 local_info_t *local)
{
	if (rateidx > sta->tx_max_rate ||
	    !(sta->tx_supp_rates & (1 << rateidx)))
		return 0;

	if (local->tx_rate_control != 0 &&
	    !(local->tx_rate_control & (1 << rateidx)))
		return 0;

	return 1;
}


static void ap_check_tx_rates(struct sta_info *sta)
{
	local_info_t *local = sta->local;
	int i;

	sta->tx_supp_rates = 0;
	for (i = 0; i < sizeof(sta->supported_rates); i++) {
		if ((sta->supported_rates[i] & 0x7f) == 2)
			sta->tx_supp_rates |= WLAN_RATE_1M;
		if ((sta->supported_rates[i] & 0x7f) == 4)
			sta->tx_supp_rates |= WLAN_RATE_2M;
		if ((sta->supported_rates[i] & 0x7f) == 11)
			sta->tx_supp_rates |= WLAN_RATE_5M5;
		if ((sta->supported_rates[i] & 0x7f) == 22)
			sta->tx_supp_rates |= WLAN_RATE_11M;
		if ((sta->supported_rates[i] & 0x7f) == 44)
			sta->tx_supp_rates |= WLAN_RATE_22M;
		if ((sta->supported_rates[i] & 0x7f) == 12)
			sta->tx_supp_rates |= WLAN_RATE_6M;
		if ((sta->supported_rates[i] & 0x7f) == 18)
			sta->tx_supp_rates |= WLAN_RATE_9M;
		if ((sta->supported_rates[i] & 0x7f) == 24)
			sta->tx_supp_rates |= WLAN_RATE_12M;
		if ((sta->supported_rates[i] & 0x7f) == 36)
			sta->tx_supp_rates |= WLAN_RATE_18M;
		if ((sta->supported_rates[i] & 0x7f) == 48)
			sta->tx_supp_rates |= WLAN_RATE_24M;
		if ((sta->supported_rates[i] & 0x7f) == 72)
			sta->tx_supp_rates |= WLAN_RATE_36M;
		if ((sta->supported_rates[i] & 0x7f) == 96)
			sta->tx_supp_rates |= WLAN_RATE_48M;
		if ((sta->supported_rates[i] & 0x7f) == 108)
			sta->tx_supp_rates |= WLAN_RATE_54M;
	}
	sta->tx_max_rate = sta->tx_rate = sta->tx_rate_idx = 0;
	if ((sta->tx_supp_rates & WLAN_RATE_1M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_G_ONLY)) {
		sta->tx_max_rate = 0;
		if (mvWLAN_ap_tx_rate_ok(0, sta, sta->local)) {
			sta->tx_rate = 10;
			sta->tx_rate_idx = 0;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_2M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_G_ONLY)) {
		sta->tx_max_rate = 1;
		if (mvWLAN_ap_tx_rate_ok(1, sta, sta->local)) {
			sta->tx_rate = 20;
			sta->tx_rate_idx = 1;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_5M5) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_G_ONLY)) {
		sta->tx_max_rate = 2;
		if (mvWLAN_ap_tx_rate_ok(2, sta, sta->local)) {
			sta->tx_rate = 55;
			sta->tx_rate_idx = 2;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_11M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_G_ONLY)) {
		sta->tx_max_rate = 3;
		if (mvWLAN_ap_tx_rate_ok(3, sta, sta->local)) {
			sta->tx_rate = 110;
			sta->tx_rate_idx = 3;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_22M) &&
	    (local->mib_phyERPTable.ERPBCCOptionImplemented &&
	     local->mib_phyERPTable.ERPBCCOptionEnabled)) {
		sta->tx_max_rate = 4;
		if (mvWLAN_ap_tx_rate_ok(4, sta, sta->local)) {
			sta->tx_rate = 220;
			sta->tx_rate_idx = 4;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_6M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 5;
		if (mvWLAN_ap_tx_rate_ok(5, sta, sta->local)) {
			sta->tx_rate = 60;
			sta->tx_rate_idx = 5;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_9M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 6;
		if (mvWLAN_ap_tx_rate_ok(6, sta, sta->local)) {
			sta->tx_rate = 90;
			sta->tx_rate_idx = 6;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_12M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 7;
		if (mvWLAN_ap_tx_rate_ok(7, sta, sta->local)) {
			sta->tx_rate = 120;
			sta->tx_rate_idx = 7;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_18M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 8;
		if (mvWLAN_ap_tx_rate_ok(8, sta, sta->local)) {
			sta->tx_rate = 180;
			sta->tx_rate_idx = 8;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_24M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 9;
		if (mvWLAN_ap_tx_rate_ok(9, sta, sta->local)) {
			sta->tx_rate = 240;
			sta->tx_rate_idx = 9;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_36M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 10;
		if (mvWLAN_ap_tx_rate_ok(10, sta, sta->local)) {
			sta->tx_rate = 360;
			sta->tx_rate_idx = 10;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_48M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 11;
		if (mvWLAN_ap_tx_rate_ok(11, sta, sta->local)) {
			sta->tx_rate = 480;
			sta->tx_rate_idx = 11;
		}
	}
	if ((sta->tx_supp_rates & WLAN_RATE_54M) && 
	    (local->mib_ApMode != MVWLAN_AP_MODE_B_ONLY)) {
		sta->tx_max_rate = 12;
		if (mvWLAN_ap_tx_rate_ok(12, sta, sta->local)) {
			sta->tx_rate = 540;
			sta->tx_rate_idx = 12;
		}
	}
	if (sta->tx_supp_rates & WLAN_RATE_72M) {
		sta->tx_max_rate = 13;
		if (mvWLAN_ap_tx_rate_ok(13, sta, sta->local)) {
			sta->tx_rate = 720;
			sta->tx_rate_idx = 13;
		}
	}

	/* Just in case */
	if (! sta->tx_rate) {
		if (local->mib_ApMode == MVWLAN_AP_MODE_B_ONLY) {
			sta->tx_rate = 60;
			sta->tx_max_rate = 5;
			sta->tx_rate_idx = 5;
		} else {
			sta->tx_rate = 10;
			sta->tx_max_rate = 0;
			sta->tx_rate_idx = 0;
		}
	}
}


#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT

static void ap_crypt_init(struct ap_data *ap)
{
	ap->crypt = mvWLAN_get_crypto_ops("WEP");

	if (ap->crypt) {
		if (ap->crypt->init) {
			ap->crypt_priv = ap->crypt->init();
			if (ap->crypt_priv == NULL)
				ap->crypt = NULL;
			else {
				u8 key[MVWLAN_WEP_KEY_LEN];
				get_random_bytes(key, MVWLAN_WEP_KEY_LEN);
				ap->crypt->set_key(0, key, MVWLAN_WEP_KEY_LEN,
						   ap->crypt_priv);
			}
		}
	}

	if (ap->crypt == NULL) {
		printk(KERN_WARNING "AP could not initialize WEP\n");
	}
}


/* Generate challenge data for shared key authentication. IEEE 802.11 specifies
 * that WEP algorithm is used for generating challange. This should be unique,
 * but otherwise there is not really need for randomness etc. Initialize WEP
 * with pseudo random key and then use increasing IV to get unique challenge
 * streams.
 *
 * Called only as a scheduled task for pending AP frames.
 */
static char * ap_auth_make_challenge(struct ap_data *ap)
{
	char *tmpbuf;
	int olen;

	if (ap->crypt == NULL) {
		ap_crypt_init(ap);
		if (ap->crypt == NULL)
			return NULL;
	}

	tmpbuf = (char *) kmalloc(WLAN_AUTH_CHALLENGE_LEN +
				  ap->crypt->extra_prefix_len +
				  ap->crypt->extra_postfix_len,
				  GFP_ATOMIC);
	if (tmpbuf == NULL) {
		PDEBUG(DEBUG_AP, "AP: kmalloc failed for challenge\n");
		return NULL;
	}
	memset(tmpbuf, 0, WLAN_AUTH_CHALLENGE_LEN +
	       ap->crypt->extra_prefix_len + ap->crypt->extra_postfix_len);
	olen = ap->crypt->encrypt_mpdu(tmpbuf, tmpbuf, WLAN_AUTH_CHALLENGE_LEN,
				  ap->crypt_priv, FALSE);
	if (olen < 0) {
		kfree(tmpbuf);
		return NULL;
	}
	memmove(tmpbuf, tmpbuf + 4, WLAN_AUTH_CHALLENGE_LEN);
	return tmpbuf;
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_authen(local_info_t *local, struct sk_buff *skb,
			     struct mvwlan_80211_rx_status *rx_stats)
{
	struct net_device *dev = local->dev;
	struct mvwlan_ieee80211_hdr *hdr =
		(struct mvwlan_ieee80211_hdr *) skb->data;
	struct ap_data *ap = local->ap;
	char body[8 + WLAN_AUTH_CHALLENGE_LEN], *challenge = NULL;
	int len, olen;
	u16 auth_alg, auth_transaction, status_code, *pos;
	u16 resp = WLAN_STATUS_SUCCESS, fc;
	struct sta_info *sta = NULL;
	struct mvwlan_crypt_data *crypt;
	char *txt = "";

	len = skb->len - IEEE80211_MGMT_HDR_LEN;

	fc = le16_to_cpu(hdr->frame_control);

	if (len < 6) {
		PDEBUG(DEBUG_AP, "%s: handle_authen - too short payload "
		       "(len=%d) from " MACSTR "\n", dev->name, len,
		       MAC2STR(hdr->addr2));
		return;
	}

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&local->ap->sta_table_lock);

	if (sta && sta->crypt)
		crypt = sta->crypt;
	else
		crypt = local->crypt;

	if (crypt && local->host_decrypt && (fc & WLAN_FC_ISWEP)) {
		atomic_inc(&crypt->refcnt);
		olen = crypt->ops->decrypt_mpdu(
			(u8 *) (skb->data + IEEE80211_MGMT_HDR_LEN), 
			(u8 *) (skb->data + IEEE80211_MGMT_HDR_LEN), 
			len, crypt->priv);
		atomic_dec(&crypt->refcnt);
		if (olen < 0) {
			if (sta)
				atomic_dec(&sta->users);
			PDEBUG(DEBUG_AP, "%s: handle_authen: auth frame from "
			       "STA " MACSTR " could not be decrypted\n",
			       dev->name, MAC2STR(hdr->addr2));
			return;
		}
		if (olen < 6) {
			PDEBUG(DEBUG_AP, "%s: handle_authen - too short "
			       "payload (len=%d, decrypted len=%d) from "
			       MACSTR "\n",
			       dev->name, len, olen, MAC2STR(hdr->addr2));
			return;
		}
		len = olen;
	}

	pos = (u16 *) (skb->data + IEEE80211_MGMT_HDR_LEN);
	auth_alg = __le16_to_cpu(*pos);
	pos++;
	auth_transaction = __le16_to_cpu(*pos);
	pos++;
	status_code = __le16_to_cpu(*pos);
	pos++;

	if (ap_control_mac_deny(&ap->mac_restrictions, hdr->addr2)) {
		txt = "authentication denied";
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}

	if (((local->auth_mode & MVWLAN_AUTH_OPEN) &&
	     auth_alg == WLAN_AUTH_OPEN) ||
	    ((local->auth_mode & MVWLAN_AUTH_SHARED_KEY) &&
	     crypt && auth_alg == WLAN_AUTH_SHARED_KEY)) {
	} else {
		txt = "unsupported algorithm";
		resp = WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG;
		goto fail;
	}

	if (len >= 8) {
		u8 *u = (u8 *) pos;
		if (*u == WLAN_EID_CHALLENGE) {
			if (*(u + 1) != WLAN_AUTH_CHALLENGE_LEN) {
				txt = "invalid challenge len";
				resp = WLAN_STATUS_CHALLENGE_FAIL;
				goto fail;
			}
			if (len - 8 < WLAN_AUTH_CHALLENGE_LEN) {
				txt = "challenge underflow";
				resp = WLAN_STATUS_CHALLENGE_FAIL;
				goto fail;
			}
			challenge = (char *) (u + 2);
		}
	}

	if (sta && sta->ap) {
		if (jiffies > sta->u.ap.last_beacon +
		    (10 * sta->listen_interval * HZ) / 1024) {
			PDEBUG(DEBUG_AP, "%s: no beacons received for a while,"
			       " assuming AP " MACSTR " is now STA\n",
			       dev->name, MAC2STR(sta->addr));
			sta->ap = 0;
			sta->flags = 0;
			sta->u.sta.challenge = NULL;
		} else {
			txt = "AP trying to authenticate?";
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			goto fail;
		}
	}

	if ((auth_alg == WLAN_AUTH_OPEN && auth_transaction == 1) ||
	    (auth_alg == WLAN_AUTH_SHARED_KEY &&
	     (auth_transaction == 1 ||
	      (auth_transaction == 3 && sta != NULL &&
	       sta->u.sta.challenge != NULL)))) {
	} else {
		txt = "unknown authentication transaction number";
		resp = WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION;
		goto fail;
	}

	if (sta == NULL) {
		txt = "new STA";

		if (local->ap->num_sta >= MAX_STA_COUNT) {
			/* FIX: might try to remove some old STAs first? */
			txt = "no more room for new STAs";
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			goto fail;
		}

		sta = mvWLAN_ap_add_sta(local->ap, hdr->addr2);
		if (sta == NULL) {
			txt = "ap_add_sta failed";
			resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
			goto fail;
		}
	}

	mvWLAN_update_sq(sta, rx_stats);

	switch (auth_alg) {
	case WLAN_AUTH_OPEN:
		txt = "authOK";
		/* STA will be authenticated, if it ACKs authentication frame
		 */
		break;

	case WLAN_AUTH_SHARED_KEY:
		if (auth_transaction == 1) {
			if (sta->u.sta.challenge == NULL) {
				sta->u.sta.challenge =
					ap_auth_make_challenge(local->ap);
				if (sta->u.sta.challenge == NULL) {
					resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
					goto fail;
				}
			}
		} else {
			if (sta->u.sta.challenge == NULL ||
			    challenge == NULL ||
			    memcmp(sta->u.sta.challenge, challenge,
				   WLAN_AUTH_CHALLENGE_LEN) != 0 ||
			    !(fc & WLAN_FC_ISWEP)) {
				txt = "challenge response incorrect";
				resp = WLAN_STATUS_CHALLENGE_FAIL;
				goto fail;
			}

			txt = "challenge OK - authOK";
			/* STA will be authenticated, if it ACKs authentication
			 * frame */
			kfree(sta->u.sta.challenge);
			sta->u.sta.challenge = NULL;
		}
		break;
	}

 fail:
	pos = (u16 *) body;
	*pos = cpu_to_le16(auth_alg);
	pos++;
	*pos = cpu_to_le16(auth_transaction + 1);
	pos++;
	*pos = cpu_to_le16(resp); /* status_code */
	pos++;
	olen = 6;

	if (resp == WLAN_STATUS_SUCCESS && sta != NULL &&
	    sta->u.sta.challenge != NULL &&
	    auth_alg == WLAN_AUTH_SHARED_KEY && auth_transaction == 1) {
		u8 *tmp = (u8 *) pos;
		*tmp++ = WLAN_EID_CHALLENGE;
		*tmp++ = WLAN_AUTH_CHALLENGE_LEN;
		pos++;
		memcpy(pos, sta->u.sta.challenge, WLAN_AUTH_CHALLENGE_LEN);
		olen += 2 + WLAN_AUTH_CHALLENGE_LEN;
	}

	mvWLAN_send_mgmt(dev, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_AUTH,
			 body, olen, 1, hdr->addr2, ap->tx_callback_auth);

	if (sta) {
		sta->last_rx = jiffies;
		atomic_dec(&sta->users);
	}

#if 0
	PDEBUG(DEBUG_AP, "%s: " MACSTR " auth (alg=%d trans#=%d stat=%d len=%d"
	       " fc=%04x) ==> %d (%s)\n", dev->name, MAC2STR(hdr->addr2),
	       auth_alg, auth_transaction, status_code, len, fc, resp, txt);
#endif
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_assoc(local_info_t *local, struct sk_buff *skb,
			    struct mvwlan_80211_rx_status *rx_stats,
			    int reassoc)
{
	struct net_device *dev = local->dev;
	struct mvwlan_ieee80211_hdr *hdr =
		(struct mvwlan_ieee80211_hdr *) skb->data;
	char body[12], *p, *lpos;
	int len, left;
	u16 *pos;
	u16 resp = WLAN_STATUS_SUCCESS;
	struct sta_info *sta = NULL;
	int send_deauth = 0;
	char *txt = "";
	u8 prev_ap[ETH_ALEN];

	left = len = skb->len - IEEE80211_MGMT_HDR_LEN;

	if (len < (reassoc ? 10 : 4)) {
		PDEBUG(DEBUG_AP, "%s: handle_assoc - too short payload "
		       "(len=%d, reassoc=%d) from " MACSTR "\n",
		       dev->name, len, reassoc, MAC2STR(hdr->addr2));
		return;
	}

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta == NULL || (sta->flags & WLAN_STA_AUTH) == 0) {
		spin_unlock_bh(&local->ap->sta_table_lock);
		txt = "trying to associate before authentication";
		send_deauth = 1;
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		sta = NULL; /* do not decrement sta->users */
		goto fail;
	}
	atomic_inc(&sta->users);
	spin_unlock_bh(&local->ap->sta_table_lock);

	mvWLAN_update_sq(sta, rx_stats);

	pos = (u16 *) (skb->data + IEEE80211_MGMT_HDR_LEN);
	sta->capability = __le16_to_cpu(*pos);
	pos++; left -= 2;
	sta->listen_interval = __le16_to_cpu(*pos);
	pos++; left -= 2;

	if (reassoc) {
		memcpy(prev_ap, pos, ETH_ALEN);
		pos++; pos++; pos++; left -= 6;
	} else
		memset(prev_ap, 0, ETH_ALEN);

	if (left >= 2) {
		unsigned int ileft;
		unsigned char *u = (unsigned char *) pos;
		MIB_STA_CFG *mib_StaCfg_p =
			&(local->sysConfig->Mib802dot11->StationConfig);

		if (*u == WLAN_EID_SSID) {
			u++; left--;
			ileft = *u;
			u++; left--;

			if (ileft > left || ileft > MVWLAN_MAX_SSID_LEN) {
				txt = "SSID overflow";
				resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
				goto fail;
			}

			if (ileft != local->essid_len ||
			    memcmp(mib_StaCfg_p->DesiredSsId, u, ileft) != 0) {
				txt = "not our SSID";
				resp = WLAN_STATUS_ASSOC_DENIED_UNSPEC;
				goto fail;
			}

			u += ileft;
			left -= ileft;
		}

		if (left >= 2 && *u == WLAN_EID_SUPP_RATES) {
			u++; left--;
			ileft = *u;
			u++; left--;

			if (ileft > left || ileft == 0 ||
			    ileft > WLAN_SUPP_RATES_MAX) {
				txt = "SUPP_RATES len error";
				resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
				goto fail;
			}

			memset(sta->supported_rates, 0,
			       sizeof(sta->supported_rates));
			memcpy(sta->supported_rates, u, ileft);
			ap_check_tx_rates(sta);

			u += ileft;
			left -= ileft;
		}

		if (left > 0) {
			PDEBUG(DEBUG_AP, "%s: assoc from " MACSTR " with extra"
			       " data (%d bytes) [",
			       dev->name, MAC2STR(hdr->addr2), left);
			while (left > 0) {
				PDEBUG2(DEBUG_AP, "<%02x>", *u);
				u++; left--;
			}
			PDEBUG2(DEBUG_AP, "]\n");
		}
	} else {
		txt = "frame underflow";
		resp = WLAN_STATUS_UNSPECIFIED_FAILURE;
		goto fail;
	}

	/* get a unique AID */
	if (sta->aid > 0)
		txt = "OK, old AID";
	else {
		spin_lock_bh(&local->ap->sta_table_lock);
		for (sta->aid = 1; sta->aid <= MAX_AID_TABLE_SIZE; sta->aid++)
			if (local->ap->sta_aid[sta->aid - 1] == NULL)
				break;
		if (sta->aid > MAX_AID_TABLE_SIZE) {
			sta->aid = 0;
			spin_unlock_bh(&local->ap->sta_table_lock);
			resp = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;
			txt = "no room for more AIDs";
		} else {
			local->ap->sta_aid[sta->aid - 1] = sta;
			spin_unlock_bh(&local->ap->sta_table_lock);
			txt = "OK, new AID";
		}
	}

 fail:
	pos = (u16 *) body;

	if (send_deauth) {
		*pos = __constant_cpu_to_le16(
			WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH);
		pos++;
	} else {
		/* FIX: CF-Pollable and CF-PollReq should be set to match the
		 * values in beacons/probe responses */
		/* FIX: how about privacy and WEP? */
		/* capability */
		*pos = __constant_cpu_to_le16(WLAN_CAPABILITY_ESS);
		pos++;

		/* status_code */
		*pos = __cpu_to_le16(resp);
		pos++;

		*pos = __cpu_to_le16((sta && sta->aid > 0 ? sta->aid : 0) |
				     BIT(14) | BIT(15)); /* AID */
		pos++;

		/* Supported rates (Information element) */
		p = (char *) pos;
		*p++ = WLAN_EID_SUPP_RATES;
		lpos = p;
		*p++ = 0; /* len */
		if (local->tx_rate_control & WLAN_RATE_1M) {
			*p++ = local->basic_rate_set & WLAN_RATE_1M ? 0x82 : 0x02;
			(*lpos)++;
		}
		if (local->tx_rate_control & WLAN_RATE_2M) {
			*p++ = local->basic_rate_set & WLAN_RATE_2M ? 0x84 : 0x04;
			(*lpos)++;
		}
		if (local->tx_rate_control & WLAN_RATE_5M5) {
			*p++ = local->basic_rate_set & WLAN_RATE_5M5 ?
				0x8b : 0x0b;
			(*lpos)++;
		}
		if (local->tx_rate_control & WLAN_RATE_11M) {
			*p++ = local->basic_rate_set & WLAN_RATE_11M ?
				0x96 : 0x16;
			(*lpos)++;
		}
		pos = (u16 *) p;
	}

	mvWLAN_send_mgmt(dev, WLAN_FC_TYPE_MGMT,
			 (send_deauth ? WLAN_FC_STYPE_DEAUTH :
			  (reassoc ? WLAN_FC_STYPE_REASSOC_RESP :
			   WLAN_FC_STYPE_ASSOC_RESP)),
			 body, (u8 *) pos - (u8 *) body, 1,
			 hdr->addr2,
			 send_deauth ? 0 : local->ap->tx_callback_assoc);

	if (sta) {
		if (resp == WLAN_STATUS_SUCCESS) {
			sta->last_rx = jiffies;
			/* STA will be marked associated from TX callback, if
			 * AssocResp is ACKed */
		}
		atomic_dec(&sta->users);
	}

#if 0
	PDEBUG(DEBUG_AP, "%s: " MACSTR " %sassoc (len=%d prev_ap=" MACSTR
	       ") => %d(%d) (%s)\n",
	       dev->name, MAC2STR(hdr->addr2), reassoc ? "re" : "", len,
	       MAC2STR(prev_ap), resp, send_deauth, txt);
#endif
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_deauth(local_info_t *local, struct sk_buff *skb,
			     struct mvwlan_80211_rx_status *rx_stats)
{
	struct net_device *dev = local->dev;
	struct mvwlan_ieee80211_hdr *hdr =
		(struct mvwlan_ieee80211_hdr *) skb->data;
	char *body = (char *) (skb->data + IEEE80211_MGMT_HDR_LEN);
	int len;
	u16 reason_code, *pos;
	struct sta_info *sta = NULL;

	len = skb->len - IEEE80211_MGMT_HDR_LEN;

	if (len < 2) {
		printk("handle_deauth - too short payload (len=%d)\n", len);
		return;
	}

	pos = (u16 *) body;
	reason_code = __le16_to_cpu(*pos);

	PDEBUG(DEBUG_AP, "%s: deauthentication: " MACSTR " len=%d, "
	       "reason_code=%d\n", dev->name, MAC2STR(hdr->addr2), len,
	       reason_code);

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta != NULL) {
		if ((sta->flags & WLAN_STA_ASSOC) && !sta->ap)
			send_event_expired_sta(local->dev, sta);
		sta->flags &= ~(WLAN_STA_AUTH | WLAN_STA_ASSOC);
		mvWLAN_update_sq(sta, rx_stats);
	}
	spin_unlock_bh(&local->ap->sta_table_lock);
	if (sta == NULL) {
		printk("%s: deauthentication from " MACSTR ", "
	       "reason_code=%d, but STA not authenticated\n", dev->name,
		       MAC2STR(hdr->addr2), reason_code);
	}
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_disassoc(local_info_t *local, struct sk_buff *skb,
			       struct mvwlan_80211_rx_status *rx_stats)
{
	struct net_device *dev = local->dev;
	struct mvwlan_ieee80211_hdr *hdr =
		(struct mvwlan_ieee80211_hdr *) skb->data;
	char *body = skb->data + IEEE80211_MGMT_HDR_LEN;
	int len;
	u16 reason_code, *pos;
	struct sta_info *sta = NULL;

	len = skb->len - IEEE80211_MGMT_HDR_LEN;

	if (len < 2) {
		printk("handle_disassoc - too short payload (len=%d)\n", len);
		return;
	}

	pos = (u16 *) body;
	reason_code = __le16_to_cpu(*pos);

	PDEBUG(DEBUG_AP, "%s: disassociation: " MACSTR " len=%d, "
	       "reason_code=%d\n", dev->name, MAC2STR(hdr->addr2), len,
	       reason_code);

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta != NULL) {
		if ((sta->flags & WLAN_STA_ASSOC) && !sta->ap)
			send_event_expired_sta(local->dev, sta);
		sta->flags &= ~WLAN_STA_ASSOC;
		mvWLAN_update_sq(sta, rx_stats);
	}
	spin_unlock_bh(&local->ap->sta_table_lock);
	if (sta == NULL) {
		printk("%s: disassociation from " MACSTR ", "
		       "reason_code=%d, but STA not authenticated\n",
		       dev->name, MAC2STR(hdr->addr2), reason_code);
	}
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_data_nullfunc(local_info_t *local,
				    struct mvwlan_ieee80211_hdr *hdr)
{
	struct net_device *dev = local->dev;

			/* XXX, need to very this for our hw */
	/* some STA f/w's seem to require control::ACK frame for
	 * data::nullfunc, but at least Prism2 station f/w version 0.8.0 does
	 * not send this..
	 * send control::ACK for the data::nullfunc */

	printk(KERN_DEBUG "Sending control::ACK for data::nullfunc\n");
	mvWLAN_send_mgmt(dev, WLAN_FC_TYPE_CTRL, WLAN_FC_STYPE_ACK,
			 NULL, 0, 0, hdr->addr2, 0);
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_dropped_data(local_info_t *local,
				   struct mvwlan_ieee80211_hdr *hdr)
{
	struct net_device *dev = local->dev;
	struct sta_info *sta;
	u16 reason;

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&local->ap->sta_table_lock);

	if (sta != NULL && (sta->flags & WLAN_STA_ASSOC)) {
		PDEBUG(DEBUG_AP, "ap_handle_dropped_data: STA is now okay?\n");
		atomic_dec(&sta->users);
		return;
	}

	reason = __constant_cpu_to_le16(
		WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA);
	mvWLAN_send_mgmt(dev, WLAN_FC_TYPE_MGMT,
			 ((sta == NULL || !(sta->flags & WLAN_STA_ASSOC)) ?
			  WLAN_FC_STYPE_DEAUTH : WLAN_FC_STYPE_DISASSOC),
			 (char *) &reason, sizeof(reason), 1,
			 hdr->addr2, 0);

	if (sta)
		atomic_dec(&sta->users);
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_beacon(local_info_t *local, struct sk_buff *skb,
			     struct mvwlan_80211_rx_status *rx_stats)
{
	struct mvwlan_ieee80211_hdr *hdr =
		(struct mvwlan_ieee80211_hdr *) skb->data;
	char *body = skb->data + IEEE80211_MGMT_HDR_LEN;
	int len, left;
	u16 *pos, beacon_int, capability;
	char *ssid = NULL;
	unsigned char *supp_rates = NULL;
	int ssid_len = 0, supp_rates_len = 0;
	struct sta_info *sta = NULL;
	int new_sta = 0, channel = -1;

	len = skb->len - IEEE80211_MGMT_HDR_LEN;

	if (len < 8 + 2 + 2) {
		printk(KERN_DEBUG "handle_beacon - too short payload "
		       "(len=%d)\n", len);
		return;
	}

	pos = (u16 *) body;
	left = len;

	/* Timestamp (8 octets) */
	pos += 4; left -= 8;
	/* Beacon interval (2 octets) */
	beacon_int = __le16_to_cpu(*pos);
	pos++; left -= 2;
	/* Capability information (2 octets) */
	capability = __le16_to_cpu(*pos);
	pos++; left -= 2;

	if (local->ap->ap_policy != AP_OTHER_AP_EVEN_IBSS &&
	    capability & WLAN_CAPABILITY_IBSS)
		return;

	if (left >= 2) {
		unsigned int ileft;
		unsigned char *u = (unsigned char *) pos;
		MIB_STA_CFG *mib_StaCfg_p =
			&(local->sysConfig->Mib802dot11->StationConfig);

		if (*u == WLAN_EID_SSID) {
			u++; left--;
			ileft = *u;
			u++; left--;

			if (ileft > left || ileft > MVWLAN_MAX_SSID_LEN) {
				PDEBUG(DEBUG_AP, "SSID: overflow\n");
				return;
			}

			if (local->ap->ap_policy == AP_OTHER_AP_SAME_SSID &&
			    (ileft != local->essid_len ||
			     memcmp(mib_StaCfg_p->DesiredSsId, u,
				    ileft) != 0)) {
				/* not our SSID */
				return;
			}

			ssid = u;
			ssid_len = ileft;

			u += ileft;
			left -= ileft;
		}

		if (*u == WLAN_EID_SUPP_RATES) {
			u++; left--;
			ileft = *u;
			u++; left--;
			
			if (ileft > left || ileft == 0 || ileft > 8) {
				PDEBUG(DEBUG_AP, " - SUPP_RATES len error\n");
				return;
			}

			supp_rates = u;
			supp_rates_len = ileft;

			u += ileft;
			left -= ileft;
		}

		if (*u == WLAN_EID_DS_PARAMS) {
			u++; left--;
			ileft = *u;
			u++; left--;
			
			if (ileft > left || ileft != 1) {
				PDEBUG(DEBUG_AP, " - DS_PARAMS len error\n");
				return;
			}

			channel = *u;

			u += ileft;
			left -= ileft;
		}
	}

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta != NULL)
		atomic_inc(&sta->users);
	spin_unlock_bh(&local->ap->sta_table_lock);

	if (sta == NULL) {
		/* add new AP */
		new_sta = 1;
		sta = mvWLAN_ap_add_sta(local->ap, hdr->addr2);
		if (sta == NULL) {
			printk(KERN_INFO " kmalloc failed for AP "
			       "data structure\n");
			return;
		}
		send_event_new_sta(local->dev, sta);

		/* mark APs authentication and associated for pseudo ad-hoc
		 * style communication */
		sta->flags = WLAN_STA_AUTH | WLAN_STA_ASSOC;

		if (local->ap->autom_ap_wds) {
			/* schedule a non-interrupt context process to add the
			 * WDS device since register_netdevice() can sleep */
			struct add_wds_data *entry;
			entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
			if (entry) {
				memcpy(entry->addr, sta->addr, ETH_ALEN);
				spin_lock_bh(&local->lock);
				entry->next = local->ap->add_wds_entries;
				local->ap->add_wds_entries = entry;
				spin_unlock_bh(&local->lock);
				MVWLAN_SCHEDULE_TASK(&local->ap->
						     add_wds_queue);
			}
		}
	}

	sta->ap = 1;
	if (ssid) {
		sta->u.ap.ssid_len = ssid_len;
		memcpy(sta->u.ap.ssid, ssid, ssid_len);
		sta->u.ap.ssid[ssid_len] = '\0';
	} else {
		sta->u.ap.ssid_len = 0;
		sta->u.ap.ssid[0] = '\0';
	}
	sta->u.ap.channel = channel;
	sta->rx_packets++;
	sta->rx_bytes += len;
	sta->u.ap.last_beacon = sta->last_rx = jiffies;
	sta->capability = capability;
	sta->listen_interval = beacon_int;
	mvWLAN_update_sq(sta, rx_stats);

	atomic_dec(&sta->users);

	if (new_sta) {
		memset(sta->supported_rates, 0, sizeof(sta->supported_rates));
		memcpy(sta->supported_rates, supp_rates, supp_rates_len);
		ap_check_tx_rates(sta);
	}
}

#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */


/* Called only as a scheduled task for pending AP frames. */
static void pspoll_send_buffered(local_info_t *local, struct sta_info *sta,
				 struct sk_buff *skb)
{
	if (!(sta->flags & WLAN_STA_PS)) {
		/* Station has moved to non-PS mode, so send all buffered
		 * frames using normal device queue. */
		dev_queue_xmit(skb);
		return;
	}

	/* add a flag for mvWLAN_handle_sta_tx() to know that this skb should
	 * be passed through even though STA is using PS */
	memcpy(skb->cb, AP_SKB_CB_MAGIC, AP_SKB_CB_MAGIC_LEN);
	skb->cb[AP_SKB_CB_MAGIC_LEN] = AP_SKB_CB_BUFFERED_FRAME;
	if (skb->dev->hard_start_xmit(skb, skb->dev)) {
		PDEBUG(DEBUG_AP, "%s: TX failed for buffered frame (PS Poll)"
		       "\n", skb->dev->name);
		dev_kfree_skb(skb);
	}
}


/* Called only as a scheduled task for pending AP frames. */
void mvWLAN_ap_handle_pspoll(local_info_t *local,
			     struct mvwlan_ieee80211_hdr *hdr,
			     struct mvwlan_80211_rx_status *rx_stats)
{
	struct net_device *dev = local->dev;
	struct sta_info *sta;
	u16 aid;
	struct sk_buff *skb;

	PDEBUG(DEBUG_PS2, "handle_pspoll: BSSID=" MACSTR ", TA=" MACSTR
	       " PWRMGT=%d\n",
	       MAC2STR(hdr->addr1), MAC2STR(hdr->addr2),
	       !!(le16_to_cpu(hdr->frame_control) & WLAN_FC_PWRMGT));

	if (memcmp(hdr->addr1, dev->dev_addr, ETH_ALEN)) {
		PDEBUG(DEBUG_AP, "handle_pspoll - addr1(BSSID)=" MACSTR
		       " not own MAC\n", MAC2STR(hdr->addr1));
		return;
	}

	aid = __le16_to_cpu(hdr->duration_id);
	if ((aid & (BIT(15) | BIT(14))) != (BIT(15) | BIT(14))) {
		PDEBUG(DEBUG_PS, "   PSPOLL and AID[15:14] not set\n");
		return;
	}
	aid &= ~BIT(15) & ~BIT(14);
	if (aid == 0 || aid > MAX_AID_TABLE_SIZE) {
		PDEBUG(DEBUG_PS, "   invalid aid=%d\n", aid);
		return;
	}
	PDEBUG(DEBUG_PS2, "   aid=%d\n", aid);

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&local->ap->sta_table_lock);

	if (sta == NULL) {
		PDEBUG(DEBUG_PS, "   STA not found\n");
		return;
	}
	mvWLAN_update_sq(sta, rx_stats);
	if (sta->aid != aid) {
		PDEBUG(DEBUG_PS, "   received aid=%i does not match with "
		       "assoc.aid=%d\n", aid, sta->aid);
		return;
	}

	/* FIX: todo:
	 * - add timeout for buffering (clear aid in TIM vector if buffer timed
	 *   out (expiry time must be longer than ListenInterval for
	 *   the corresponding STA; "8802-11: 11.2.1.9 AP aging function"
	 * - what to do, if buffered, pspolled, and sent frame is not ACKed by
	 *   sta; store buffer for later use and leave TIM aid bit set? use
	 *   TX event to check whether frame was ACKed?
	 */

	while ((skb = skb_dequeue(&sta->tx_buf)) != NULL) {
		/* send buffered frame .. */
		PDEBUG(DEBUG_PS2, "Sending buffered frame to STA after PS POLL"
		       " (buffer_count=%d)\n", skb_queue_len(&sta->tx_buf));

		pspoll_send_buffered(local, sta, skb);

		if (sta->flags & WLAN_STA_PS) {
			/* send only one buffered packet per PS Poll */
			/* FIX: should ignore further PS Polls until the
			 * buffered packet that was just sent is acknowledged
			 * (Tx or TxExc event) */
			break;
		}
	}

	if (skb_queue_empty(&sta->tx_buf)) {
		/* try to clear aid from TIM */
		if (!(sta->flags & WLAN_STA_TIM))
			PDEBUG(DEBUG_PS2,  "Re-unsetting TIM for aid %d\n",
			       aid);
		mvWLAN_set_tim(local, aid, 0);
		sta->flags &= ~WLAN_STA_TIM;
	}

	atomic_dec(&sta->users);
}


void mvWLAN_ap_handle_probereq(local_info_t *local, struct sk_buff *skb,
			       struct mvwlan_80211_rx_status *rx_stats)
{
	struct net_device *dev = local->dev;
	struct sta_info *sta;
	struct mvwlan_ieee80211_hdr *hdr =
		(struct mvwlan_ieee80211_hdr *) skb->data;
	char *body = skb->data + IEEE80211_MGMT_HDR_LEN;
	int len, left;
	char *ssid = NULL;
	unsigned char *request_info = NULL;
	unsigned char *supp_rates = NULL, *ext_supp_rates = NULL;
	int ssid_len = 0, request_info_len = 0;
	int supp_rates_len = 0, ext_supp_rates_len = 0;
	unsigned int ileft;
	unsigned char *u;
	MIB_STA_CFG *mib_StaCfg_p =
			&(local->sysConfig->Mib802dot11->StationConfig);

	len = skb->len - IEEE80211_MGMT_HDR_LEN;

	if (len < 2) {
		printk(KERN_DEBUG "handle_probereq - too short payload "
		       "(len=%d)\n", len);
		return;
	}

	left = len;
	u = (unsigned char *) body;

	if (*u == WLAN_EID_SSID) {
		u++; left--;
		ileft = *u;
		u++; left--;

		if (ileft > left || ileft > MVWLAN_MAX_SSID_LEN) {
			PDEBUG(DEBUG_AP, "SSID: overflow\n");
			return;
		}

		if ((ileft == local->essid_len) &&
		    (memcmp(mib_StaCfg_p->DesiredSsId, u, ileft) == 0)) {

			ssid = u;
			ssid_len = ileft;

			u += ileft;
			left -= ileft;

		} else {

			return;

		}
	}

	if (left >= 2 && *u == WLAN_EID_SUPP_RATES) {
		u++; left--;
		ileft = *u;
		u++; left--;

		if (ileft > left || ileft == 0 || ileft > 8) {
			PDEBUG(DEBUG_AP, " - SUPP_RATES len error\n");
			return;
		}

		supp_rates = u;
		supp_rates_len = ileft;

		u += ileft;
		left -= ileft;
	}

	if (left >= 2 && *u == WLAN_EID_REQUEST_INFO) {
		u++; left--;
		ileft = *u;
		u++; left--;

		if (ileft > left) {
			PDEBUG(DEBUG_AP, " - REQUEST_INFORMATION len error\n");
			return;
		}

		request_info = u;
		request_info_len = ileft;

		u += ileft;
		left -= ileft;
	}

	if (left >= 2 && *u == WLAN_EID_EXT_SUPP_RATES) {
		u++; left--;
		ileft = *u;
		u++; left--;

		if (ileft > left || ileft == 0 || ileft > 8) {
			PDEBUG(DEBUG_AP, " - EXT_SUPP_RATES len error\n");
			return;
		}

		ext_supp_rates = u;
		ext_supp_rates_len = ileft;

		u += ileft;
		left -= ileft;
	}

	if (left > 0) {
		PDEBUG(DEBUG_AP, "%s: probereq from " MACSTR " with extra"
		       " data (%d bytes) [",
		       dev->name, MAC2STR(hdr->addr2), left);
		while (left > 0) {
			PDEBUG2(DEBUG_AP, "<%02x>", *u);
			u++; left--;
		}
		PDEBUG2(DEBUG_AP, "]\n");
	}

	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&local->ap->sta_table_lock);

	if (sta) {
		mvWLAN_update_sq(sta, rx_stats);
	}
	atomic_dec(&sta->users);

	body = mvWLAN_GetProbeResponseBuffer(ssid, ssid_len, &len);

	mvWLAN_send_mgmt(dev, WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_PROBE_RESP,
			 body, len, 0, hdr->addr2, 0);
}


void mvWLAN_update_sq(struct sta_info *sta,
		      struct mvwlan_80211_rx_status *rx_stats)
{
	sta->last_rx_silence = rx_stats->noise;
	sta->last_rx_signal = rx_stats->signal;
	sta->last_rx_rate = rx_stats->rate;
	sta->last_rx_updated = 7;
	if (rx_stats->rate == 10)
		sta->rx_count[0]++;
	else if (rx_stats->rate == 20)
		sta->rx_count[1]++;
	else if (rx_stats->rate == 55)
		sta->rx_count[2]++;
	else if (rx_stats->rate == 110)
		sta->rx_count[3]++;
	else if (rx_stats->rate == 220)
		sta->rx_count[4]++;
	else if (rx_stats->rate == 60)
		sta->rx_count[5]++;
	else if (rx_stats->rate == 90)
		sta->rx_count[6]++;
	else if (rx_stats->rate == 120)
		sta->rx_count[7]++;
	else if (rx_stats->rate == 180)
		sta->rx_count[8]++;
	else if (rx_stats->rate == 240)
		sta->rx_count[9]++;
	else if (rx_stats->rate == 360)
		sta->rx_count[10]++;
	else if (rx_stats->rate == 480)
		sta->rx_count[11]++;
	else if (rx_stats->rate == 540)
		sta->rx_count[12]++;
}


static int hostapd_add_sta(struct ap_data *ap,
			   struct mvwlan_hostapd_param *param)
{
	local_info_t *local = ap->local;
	struct sta_info *sta;
	int new = 0;

	spin_lock_bh(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, param->sta_addr);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&ap->sta_table_lock);

	if (sta == NULL) {
		new = 1;
		sta = mvWLAN_ap_add_sta(ap, param->sta_addr);
		if (sta == NULL)
			return -1;
	}

	MVWLAN_RATE_InitSTA(sta);

	if (!(sta->flags & WLAN_STA_ASSOC) && !sta->ap && sta->local)
		send_event_new_sta(sta->local->dev, sta);

	sta->flags |= WLAN_STA_AUTH | WLAN_STA_ASSOC;
	sta->last_rx = jiffies;
	sta->aid = param->u.add_sta.aid;
	sta->capability = param->u.add_sta.capability;
	sta->tx_supp_rates = param->u.add_sta.tx_supp_rates;
	if (sta->tx_supp_rates & WLAN_RATE_1M)
		sta->supported_rates[0] = 2;
	if (sta->tx_supp_rates & WLAN_RATE_2M)
		sta->supported_rates[1] = 4;
 	if (sta->tx_supp_rates & WLAN_RATE_5M5)
		sta->supported_rates[2] = 11;
	if (sta->tx_supp_rates & WLAN_RATE_11M)
		sta->supported_rates[3] = 22;
	if (sta->tx_supp_rates & WLAN_RATE_22M)
		sta->supported_rates[4] = 44;
	if (sta->tx_supp_rates & WLAN_RATE_6M)
		sta->supported_rates[5] = 12;
 	if (sta->tx_supp_rates & WLAN_RATE_9M)
		sta->supported_rates[6] = 18;
	if (sta->tx_supp_rates & WLAN_RATE_12M)
		sta->supported_rates[7] = 24;
	if (sta->tx_supp_rates & WLAN_RATE_18M)
		sta->supported_rates[8] = 36;
	if (sta->tx_supp_rates & WLAN_RATE_24M)
		sta->supported_rates[9] = 48;
 	if (sta->tx_supp_rates & WLAN_RATE_36M)
		sta->supported_rates[10] = 72;
	if (sta->tx_supp_rates & WLAN_RATE_48M)
		sta->supported_rates[11] = 96;
	if (sta->tx_supp_rates & WLAN_RATE_54M)
		sta->supported_rates[12] = 108;
	ap_check_tx_rates(sta);

	if (((sta->tx_supp_rates & 0x0000000F) != 0) &&
		((sta->tx_supp_rates & 0xFFFFFFF0) == 0)) {

		if ((sta->capability & 0x20) == 0)
			ap->barker_preamble_set = 1;

		sta->b_only_client = 1;

		if (new)
			mvWLAN_inc_b_sta(local, TRUE);
	} else {

		sta->b_only_client = 0;
	}

	atomic_dec(&sta->users);

	mvWLAN_EnableBcnFreeIntr();

	return 0;
}


static int hostapd_remove_sta(struct ap_data *ap,
			      struct mvwlan_hostapd_param *param)
{
	local_info_t *local = ap->local;
	struct sta_info *sta;

	spin_lock_bh(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, param->sta_addr);
	if (sta) {
		ap_sta_hash_del(ap, sta);
		list_del(&sta->list);
	}
	spin_unlock_bh(&ap->sta_table_lock);

	if (!sta)
		return -ENOENT;

	if (((sta->tx_supp_rates & 0x0000000F) != 0) &&
	    ((sta->tx_supp_rates & 0xFFFFFFF0) == 0)) {

		mvWLAN_dec_b_sta(local, TRUE);
	}

	if ((sta->flags & WLAN_STA_ASSOC) && !sta->ap && sta->local)
		send_event_expired_sta(sta->local->dev, sta);

	ap_free_sta(ap, sta);

	mvWLAN_EnableBcnFreeIntr();

	return 0;
}


static int hostapd_get_info_sta(struct ap_data *ap,
			        struct mvwlan_hostapd_param *param)
{
	struct sta_info *sta;

	spin_lock_bh(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, param->sta_addr);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&ap->sta_table_lock);

	if (!sta)
		return -ENOENT;

	param->u.get_info_sta.inactive_sec = (jiffies - sta->last_rx) / HZ;

	atomic_dec(&sta->users);

	return 1;
}


static int hostapd_set_flags_sta(struct ap_data *ap,
				 struct mvwlan_hostapd_param *param)
{
	struct sta_info *sta;

	spin_lock_bh(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, param->sta_addr);
	if (sta) {
		sta->flags |= param->u.set_flags_sta.flags_or;
		sta->flags &= param->u.set_flags_sta.flags_and;
	}
	spin_unlock_bh(&ap->sta_table_lock);

	if (!sta)
		return -ENOENT;

	return 0;
}


static int hostapd_set_ptk_sta(struct ap_data *ap,
				 struct mvwlan_hostapd_param *param)
{
	struct sta_info *sta;

	spin_lock_bh(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, param->sta_addr);
	if (sta) {
		memcpy(&sta->key_mgmt_info, param->u.cmd.data, param->u.cmd.len);
	}
	spin_unlock_bh(&ap->sta_table_lock);

	if (!sta)
		return -ENOENT;

	return 0;
}


int mvWLAN_hostapd(struct ap_data *ap, struct mvwlan_hostapd_param *param)
{
	switch (param->cmd) {
	case MVWLAN_HOSTAPD_FLUSH:
		mvWLAN_ap_control_kickall(ap);
		return 0;
	case MVWLAN_HOSTAPD_ADD_STA:
		return hostapd_add_sta(ap, param);
	case MVWLAN_HOSTAPD_REMOVE_STA:
		return hostapd_remove_sta(ap, param);
	case MVWLAN_HOSTAPD_GET_INFO_STA:
		return hostapd_get_info_sta(ap, param);
	case MVWLAN_HOSTAPD_SET_FLAGS_STA:
		return hostapd_set_flags_sta(ap, param);
	case MVWLAN_HOSTAPD_SET_PTK_STA:
		return hostapd_set_ptk_sta(ap, param);
	default:
		printk(KERN_WARNING "mvWLAN_hostapd: unknown cmd=%d\n",
		       param->cmd);
		return -EOPNOTSUPP;
	}
}


void mvWLAN_handle_sta_release(void *ptr)
{
	struct sta_info *sta = ptr;
	atomic_dec(&sta->users);
}


/* Called only as a tasklet (software IRQ) */
int mvWLAN_is_sta_assoc(struct ap_data *ap, u8 *sta_addr)
{
	struct sta_info *sta;
	int ret = 0;

	spin_lock(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, sta_addr);
	if (sta != NULL && (sta->flags & WLAN_STA_ASSOC) && !sta->ap)
		ret = 1;
	spin_unlock(&ap->sta_table_lock);

	return ret;
}


/* Called only as a tasklet (software IRQ) */
int mvWLAN_update_rx_stats(struct ap_data *ap,
			   struct mvwlan_ieee80211_hdr *hdr,
			   struct mvwlan_80211_rx_status *rx_stats)
{
	struct sta_info *sta;

	if (!ap)
		return -1;

	spin_lock(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, hdr->addr2);
	if (sta)
		mvWLAN_update_sq(sta, rx_stats);
	spin_unlock(&ap->sta_table_lock);

	return sta ? 0 : -1;
}


void mvWLAN_update_rates(local_info_t *local)
{
	struct list_head *ptr;
	struct ap_data *ap = local->ap;

	if (!ap)
		return;

	spin_lock_bh(&ap->sta_table_lock);
	for (ptr = ap->sta_list.next; ptr != &ap->sta_list; ptr = ptr->next) {
		struct sta_info *sta = (struct sta_info *) ptr;
		ap_check_tx_rates(sta);
	}
	spin_unlock_bh(&ap->sta_table_lock);
}


void mvWLAN_init_ap_data(local_info_t *local)
{
	struct ap_data *ap = local->ap;

	if (ap == NULL) {
		printk(KERN_WARNING "mvWLAN_init_data: ap == NULL\n");
		return;
	}
	memset(ap, 0, sizeof(struct ap_data));

	ap->local = local;
	ap->ap_policy = GET_INT_PARM(other_ap_policy, 0);
	ap->bridge_packets = GET_INT_PARM(ap_bridge_packets, 0);
	ap->max_inactivity =
		GET_INT_PARM(ap_max_inactivity, 0) * HZ;
	ap->autom_ap_wds = GET_INT_PARM(autom_ap_wds, 0);

	spin_lock_init(&ap->sta_table_lock);
	INIT_LIST_HEAD(&ap->sta_list);

	/* Initialize task queue structure for AP management */
	MVWLAN_QUEUE_INIT(&local->ap->set_tim_queue, handle_set_tim_queue,
			  local);
	INIT_LIST_HEAD(&ap->set_tim_list);
	spin_lock_init(&ap->set_tim_lock);

	ap->tx_callback_idx =
		mvWLAN_tx_callback_register(local, mvWLAN_ap_tx_cb, ap);
	if (ap->tx_callback_idx == 0)
		printk(KERN_WARNING "%s: failed to register TX callback for "
		       "AP\n", local->dev->name);

	mvWLAN_init_wds(local);

	mvWLAN_init_ap_proc(local);

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	ap->tx_callback_auth =
		mvWLAN_tx_callback_register(local, mvWLAN_ap_tx_cb_auth, ap);
	ap->tx_callback_assoc =
		mvWLAN_tx_callback_register(local, mvWLAN_ap_tx_cb_assoc, ap);
	ap->tx_callback_poll =
		mvWLAN_tx_callback_register(local, mvWLAN_ap_tx_cb_poll, ap);
	if (ap->tx_callback_auth == 0 || ap->tx_callback_assoc == 0 ||
		ap->tx_callback_poll == 0)
		printk(KERN_WARNING "%s: failed to register TX callback for "
		       "AP\n", local->dev->name);

	spin_lock_init(&ap->mac_restrictions.lock);
	INIT_LIST_HEAD(&ap->mac_restrictions.mac_list);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

    ap->sta_bridge = local->cfg_data->sta_bridge;

	ap->initialized = 1;
}

void mvWLAN_free_ap_data(struct ap_data *ap)
{
	struct list_head *ptr, *n;

	if (ap == NULL || !ap->initialized) {
		printk(KERN_DEBUG "mvWLAN_free_data: ap has not yet been "
		       "initialized - skip resource freeing\n");
		return;
	}

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	if (ap->crypt)
		ap->crypt->deinit(ap->crypt_priv);
	ap->crypt = ap->crypt_priv = NULL;
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

	ptr = ap->sta_list.next;
	while (ptr != NULL && ptr != &ap->sta_list) {
		struct sta_info *sta = (struct sta_info *) ptr;
		ptr = ptr->next;
		ap_sta_hash_del(ap, sta);
		list_del(&sta->list);
		if ((sta->flags & WLAN_STA_ASSOC) && !sta->ap && sta->local)
			send_event_expired_sta(sta->local->dev, sta);
		ap_free_sta(ap, sta);
	}

	for (ptr = ap->set_tim_list.next, n = ptr->next;
	     ptr != &ap->set_tim_list; ptr = n, n = ptr->next) {
		struct set_tim_data *entry;
		entry = list_entry(ptr, struct set_tim_data, list);
		list_del(&entry->list);
		kfree(entry);
	}

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	mvWLAN_ap_control_flush_macs(&ap->mac_restrictions);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

	mvWLAN_remove_ap_proc(ap->local);

	ap->initialized = 0;
}


u32 mvWLAN_rate2mask(u8 rate)
{
	u32 index = 0;

	switch (rate & 0x7F) {
	case 2:
		index = WLAN_RATE_1M;
		break;
	case 4:
		index = WLAN_RATE_2M;
		break;
	case 11:
		index = WLAN_RATE_5M5;
		break;
	case 22:
		index = WLAN_RATE_11M;
		break;
	case 44:
		index = WLAN_RATE_22M;
		break;
	case 12:
		index = WLAN_RATE_6M;
		break;
	case 18:
		index = WLAN_RATE_9M;
		break;
	case 24:
		index = WLAN_RATE_12M;
		break;
	case 36:
		index = WLAN_RATE_18M;
		break;
	case 48:
		index = WLAN_RATE_24M;
		break;
	case 72:
		index = WLAN_RATE_36M;
		break;
	case 96:
		index = WLAN_RATE_48M;
		break;
	case 108:
		index = WLAN_RATE_54M;
	}

	return index;
}


int mvWLAN_sta_existed(struct net_device *dev, u8 *sta_addr, u8 delSta)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct ap_data *ap = local->ap;
	struct sta_info *sta;
	int ret = 0;

	spin_lock(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, sta_addr);
	if (sta != NULL) {
     
		ret = 1;
      
      if (delSta && mvWLAN_wds_is_wds_link((local_info_t *) dev->priv, sta_addr)) {

         ap_sta_hash_del(ap, sta);
         
		 list_del(&sta->list);

         if (((sta->tx_supp_rates & 0x0000000F) != 0) &&
	     ((sta->tx_supp_rates & 0xFFFFFFF0) == 0)) {
		    mvWLAN_dec_b_sta(local, TRUE);
	     }

	     if ((sta->flags & WLAN_STA_ASSOC) && !sta->ap && sta->local)
		    send_event_expired_sta(sta->local->dev, sta);
    
         ap_free_sta(ap, sta);

         mvWLAN_EnableBcnFreeIntr();
         
         printk(KERN_ALERT "mvWLAN_sta_existed: a station "
                     "%02x:%02x:%02x:%02x:%02x:%02x is removed\n", sta_addr[0],
                                                                   sta_addr[1],
                                                                   sta_addr[2],
                                                                   sta_addr[3],
                                                                   sta_addr[4],
                                                                   sta_addr[5]);
      }
		
	}
    
	spin_unlock(&ap->sta_table_lock);

	return ret;
}


#ifdef SELF_CTS
WLAN_TX_FRAME *sendCts(UINT32 duration, int Qnum, local_info_t *local)
{
    struct mvwlan_tx_queue *tx_queue;
    IEEEtypes_Frame_t *tx_frame;
    WLAN_TX_FRAME *Frame_p = NULL;

    {
        tx_queue = mvWLAN_tx_queue_alloc(local);

        if (! tx_queue)
        {
            return NULL;
        }

        tx_frame = tx_queue->tx_frame;
        Frame_p = (WLAN_TX_FRAME *) ((u32)&tx_queue->tx_desc);

        tx_frame->Hdr.FrmBodyLen = 0;
        tx_frame->Hdr.FrmCtl.ProtocolVersion = 0;
        tx_frame->Hdr.FrmCtl.Type = IEEE_TYPE_CONTROL;
        tx_frame->Hdr.FrmCtl.Subtype = CTS;
        tx_frame->Hdr.FrmCtl.ToDs = 0;
        tx_frame->Hdr.FrmCtl.FromDs = 0;
        tx_frame->Hdr.FrmCtl.MoreFrag = 0;
        tx_frame->Hdr.FrmCtl.Retry = 0;
        tx_frame->Hdr.FrmCtl.PwrMgmt = 0;
        tx_frame->Hdr.FrmCtl.MoreData = 0;
        tx_frame->Hdr.FrmCtl.Wep = 0;
        tx_frame->Hdr.FrmCtl.Order = 0;
        tx_frame->Hdr.DurationId = duration;

        memcpy(&tx_frame->Hdr.Addr1, local->bssid, sizeof(IEEEtypes_MacAddr_t));
        crc32forCts((unsigned char *) &(tx_frame->Hdr.FrmCtl), 10);

        Frame_p->wlanTxHdr.StnId = 0;
        Frame_p->wlanTxHdr.RateIdx = 3;
        Frame_p->wlanTxHdr.Preample = 0;
        Frame_p->wlanTxHdr.QosControl = Qnum;
        Frame_p->wlanTxHdr.TxParam = 0x46;

        return Frame_p;
    }

}
#endif


EXPORT_SYMBOL(mvWLAN_handle_sta_release);
EXPORT_SYMBOL(mvWLAN_update_rates);
EXPORT_SYMBOL(mvWLAN_hostapd);
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
EXPORT_SYMBOL(mvWLAN_deauth_all_stas);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
EXPORT_SYMBOL(mvWLAN_init_ap_data);
EXPORT_SYMBOL(mvWLAN_free_ap_data);
