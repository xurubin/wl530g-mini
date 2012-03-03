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
#include <linux/ctype.h>
#include <linux/kmod.h>
#include <linux/byteorder/generic.h>
#include <linux/etherdevice.h>
#include <asm/uaccess.h>

#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_rate.h"

#ifdef BOOSTER_MODE
#include "mvWLAN_booster.h"
#endif


typedef enum {
	AP_RX_CONTINUE, AP_RX_DROP, AP_RX_EXIT, AP_RX_CONTINUE_NOT_AUTHORIZED
} ap_rx_ret;

void mvWLAN_dump_raw(void *data, int len)
{
    int idx;
    int jdx;
    char *ptr = (char *)data;
    int row = len/16;
    int remain = len%16;

    printk("\n\r");  
    
    for (jdx=0; jdx<row; jdx++) 
    {
        for (idx=0; idx<16; idx++)
            printk("%02X ", ptr[(jdx<<4)+idx]);

        printk(" | ");
        
        for (idx=0; idx<16; idx++)
        {
            char c = ptr[(jdx<<4)+idx];

            if (isprint(c))
                printk("%c", c);
            else
                printk("%c", '.');
        }
        
        printk("\n\r");        
    }

    for (idx=0; idx<remain; idx++)
        printk("%02X ", ptr[(jdx<<4)+idx]);

    for (idx=0; idx<(16 - remain); idx++)
        printk("   ");
        
    printk(" | ");
        
    for (idx=0; idx<remain; idx++)
    {
        char c = ptr[(jdx<<4)+idx];

        if (isprint(c))
            printk("%c", c);
        else
            printk("%c", '.');
    }
    
    printk("\n\r");
    printk("\n\r");  
    printk("\n\r");  
    return;
}

void mvWLAN_dump_rx_80211(const char *name, struct sk_buff *skb,
			  struct mvwlan_80211_rx_status *rx_stats)
{
	IEEEtypes_Frame_t *rx_frame;
	struct mvwlan_ieee80211_hdr *hdr;
	u16 fc;
	int hdrlen;

	rx_frame = (IEEEtypes_Frame_t *) skb->data;
	hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);

	printk(KERN_DEBUG "%s: RX signal=%d noise=%d rate=%d len=%d "
	       "jiffies=%ld\n",
	       name, rx_stats->signal, rx_stats->noise, rx_stats->rate,
	       skb->len, jiffies);

	if (skb->len < 2)
		return;

	fc = le16_to_cpu(hdr->frame_control);
	hdrlen = mvWLAN_80211_get_hdrlen(fc);

	printk(KERN_DEBUG "   FC=0x%04x (type=%d:%d)%s%s",
	       fc, WLAN_FC_GET_TYPE(fc), WLAN_FC_GET_STYPE(fc),
	       fc & WLAN_FC_TODS ? " [ToDS]" : "",
	       fc & WLAN_FC_FROMDS ? " [FromDS]" : "");

	if (skb->len < IEEE80211_DATA_HDR3_LEN) {
		printk("\n");
		return;
	}

	printk(" dur=0x%04x seq=0x%04x\n", le16_to_cpu(hdr->duration_id),
	       le16_to_cpu(hdr->seq_ctrl));

	printk(KERN_DEBUG "   A1=" MACSTR " A2=" MACSTR " A3=" MACSTR,
	       MAC2STR(hdr->addr1), MAC2STR(hdr->addr2), MAC2STR(hdr->addr3));
	if (hdrlen == IEEE80211_DATA_HDR4_LEN)
		printk(" A4=" MACSTR, MAC2STR(hdr->addr4));
	printk("\n");
}


/* Send RX frame to netif with 802.11 altered header for use with AP daemon.
 * Called from hardware or software IRQ context. */
int mvWLAN_rx_apd(struct net_device *dev, struct sk_buff *skb,
		  struct mvwlan_80211_rx_status *rx_stats)
{
	u16 fc;
	int hdrlen, ret, shift_len;
	IEEEtypes_Frame_t *rx_frame;
	struct mvwlan_ieee80211_hdr *hdr;

	dev->last_rx = jiffies;

	rx_frame = (IEEEtypes_Frame_t *) skb->data;
	hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);
	fc = le16_to_cpu(hdr->frame_control);

	if (fc & WLAN_FC_PVER) {
		printk(KERN_DEBUG "%s: dropped management frame with header "
		       "version %d\n", dev->name, fc & WLAN_FC_PVER);
		dev_kfree_skb_any(skb);
		return 0;
	}

	/* The underlying hardware always passes frames of
	 * which the IEEE 802.11 header uses 4 addresses;
	 * however, the hostap daemon would use 4 addresses
	 * only when both FromDS and ToDS are true.
	 */

	hdrlen = mvWLAN_80211_get_hdrlen(fc);
	shift_len = IEEE80211_DATA_HDR4_LEN - hdrlen;

	if (shift_len) {

		int i;
		u8 *dst, *src;

		dst = ((u8 *) hdr) + hdrlen + shift_len - 1;
		src = ((u8 *) hdr) + hdrlen - 1;

		for (i = 0; i < hdrlen; i++)
			*dst-- = *src--;
	}

	skb_pull(skb, shift_len+2); /* 2 means Frame Body length */

	ret = skb->len;
	skb->dev = dev;
	skb->mac.raw = skb->data;
	skb_pull(skb, hdrlen);
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = __constant_htons(ETH_P_802_2);
	memset(skb->cb, 0, sizeof(skb->cb));
	netif_rx(skb);

	return ret;
}


/* Called only as a tasklet. */
static void handle_ap_item(local_info_t *local, struct sk_buff *skb,
			   struct mvwlan_80211_rx_status *rx_stats)
{
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	struct net_device *dev = local->dev;
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
	u16 fc, type, stype;
	struct mvwlan_ieee80211_hdr *hdr;

	/* FIX: should give skb->len to handler functions and check that the
	 * buffer is long enough */
	hdr = (struct mvwlan_ieee80211_hdr *) skb->data;
	fc = le16_to_cpu(hdr->frame_control);
	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	if (!local->hostapd && type == WLAN_FC_TYPE_DATA) {
		PDEBUG(DEBUG_AP, "handle_ap_item - data frame\n");

		if (!(fc & WLAN_FC_TODS) || (fc & WLAN_FC_FROMDS)) {
			if (stype == WLAN_FC_STYPE_NULLFUNC) {
				/* no ToDS nullfunc seems to be used to check
				 * AP association; so send reject message to
				 * speed up re-association */
				mvWLAN_ap_handle_dropped_data(local, hdr);
				goto done;
			}
			PDEBUG(DEBUG_AP, "   not ToDS frame (fc=0x%04x)\n",
			       fc);
			goto done;
		}

		if (memcmp(hdr->addr1, dev->dev_addr, ETH_ALEN)) {
			PDEBUG(DEBUG_AP, "handle_ap_item - addr1(BSSID)="
			       MACSTR " not own MAC\n",
			       MAC2STR(hdr->addr1));
			goto done;
		}

		if (local->ap->nullfunc_ack && stype == WLAN_FC_STYPE_NULLFUNC)
			mvWLAN_ap_handle_data_nullfunc(local, hdr);
		else
			mvWLAN_ap_handle_dropped_data(local, hdr);
		goto done;
	}

	if (type == WLAN_FC_TYPE_MGMT && stype == WLAN_FC_STYPE_BEACON) {
		mvWLAN_ap_handle_beacon(local, skb, rx_stats);
		goto done;
	}
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

	if (type == WLAN_FC_TYPE_CTRL && stype == WLAN_FC_STYPE_PSPOLL) {
		mvWLAN_ap_handle_pspoll(local, hdr, rx_stats);
		goto done;
	}

	if (type == WLAN_FC_TYPE_MGMT && stype == WLAN_FC_STYPE_PROBE_REQ) {
		mvWLAN_ap_handle_probereq(local, skb, rx_stats);
		goto done;
	}

	if (local->hostapd) {
		PDEBUG(DEBUG_AP, "Unknown frame in AP queue: type=0x%02x "
		       "subtype=0x%02x\n", type, stype);
		goto done;
	}

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	if (type != WLAN_FC_TYPE_MGMT) {
		PDEBUG(DEBUG_AP, "handle_ap_item - not a management frame?\n");
		goto done;
	}

	if (memcmp(hdr->addr1, dev->dev_addr, ETH_ALEN)) {
		PDEBUG(DEBUG_AP, "handle_ap_item - addr1(DA)=" MACSTR
		       " not own MAC\n", MAC2STR(hdr->addr1));
		goto done;
	}

	if (memcmp(hdr->addr3, dev->dev_addr, ETH_ALEN)) {
		PDEBUG(DEBUG_AP, "handle_ap_item - addr3(BSSID)=" MACSTR
		       " not own MAC\n", MAC2STR(hdr->addr3));
		goto done;
	}

	switch (stype) {
	case WLAN_FC_STYPE_ASSOC_REQ:
		mvWLAN_ap_handle_assoc(local, skb, rx_stats, 0);
		break;
	case WLAN_FC_STYPE_ASSOC_RESP:
		PDEBUG(DEBUG_AP, "==> ASSOC RESP (ignored)\n");
		break;
	case WLAN_FC_STYPE_REASSOC_REQ:
		mvWLAN_ap_handle_assoc(local, skb, rx_stats, 1);
		break;
	case WLAN_FC_STYPE_REASSOC_RESP:
		PDEBUG(DEBUG_AP, "==> REASSOC RESP (ignored)\n");
		break;
	case WLAN_FC_STYPE_ATIM:
		PDEBUG(DEBUG_AP, "==> ATIM (ignored)\n");
		break;
	case WLAN_FC_STYPE_DISASSOC:
		mvWLAN_ap_handle_disassoc(local, skb, rx_stats);
		break;
	case WLAN_FC_STYPE_AUTH:
		mvWLAN_ap_handle_authen(local, skb, rx_stats);
		break;
	case WLAN_FC_STYPE_DEAUTH:
		mvWLAN_ap_handle_deauth(local, skb, rx_stats);
		break;
	default:
		PDEBUG(DEBUG_AP, "Unknown mgmt frame subtype 0x%02x\n",
		       stype);
		break;
	}
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

 done:
	dev_kfree_skb(skb);
}


/* Called only as a tasklet (software IRQ) */
static void mvWLAN_rx(struct net_device *dev, struct sk_buff *skb,
		      struct mvwlan_80211_rx_status *rx_stats)
{
	local_info_t *local = (local_info_t *) dev->priv;
	u16 fc;
	int hdrlen, shift_len;
	IEEEtypes_Frame_t *rx_frame;
	struct mvwlan_ieee80211_hdr *hdr;

	if (skb->len < 16)
		goto drop;

	local->stats.rx_packets++;

	rx_frame = (IEEEtypes_Frame_t *) skb->data;
	hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);
	fc = le16_to_cpu(hdr->frame_control);

	if (local->ap->ap_policy == AP_OTHER_AP_SKIP_ALL &&
	    WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_MGMT &&
	    WLAN_FC_GET_STYPE(fc) == WLAN_FC_STYPE_BEACON)
		goto drop;

	/* The underlying hardware always passes frames of
	 * which the IEEE 802.11 header uses 4 addresses;
	 * however, the hostap daemon would use 4 addresses
	 * only when both FromDS and ToDS are true.
	 */

	hdrlen = mvWLAN_80211_get_hdrlen(fc);
	shift_len = IEEE80211_DATA_HDR4_LEN - hdrlen;

	if (shift_len) {

		int i;
		u8 *dst, *src;

		dst = ((u8 *) hdr) + hdrlen + shift_len - 1;
		src = ((u8 *) hdr) + hdrlen - 1;

		for (i = 0; i < hdrlen; i++)
			*dst-- = *src--;
	}

	skb_pull(skb, shift_len+2); /* 2 means Frame Body length */

	skb->protocol = __constant_htons(ETH_P_MVWLAN);
	handle_ap_item(local, skb, rx_stats);
	return;

 drop:
	dev_kfree_skb(skb);
}


/* Called only as a tasklet (software IRQ) */
static void mvWLAN_rx_monitor_frame(struct net_device *dev,
				    struct sk_buff *skb,
				    struct mvwlan_80211_rx_status *rx_stats)
{
	struct net_device_stats *stats;
	int len;

	/* XXX, fix this when we know more about how to deal with the
	 * monitor mode */
	len = 0;
	stats = mvWLAN_get_stats(dev);
	stats->rx_packets++;
	stats->rx_bytes += len;
}


/* Called only as a tasklet (software IRQ) */
static struct mvwlan_frag_entry *
mvWLAN_frag_cache_find(local_info_t *local, unsigned int seq,
		       unsigned int frag, u8 *src, u8 *dst)
{
	struct mvwlan_frag_entry *entry;
	int i;

	for (i = 0; i < MVWLAN_FRAG_CACHE_LEN; i++) {
		entry = &local->frag_cache[i];
		if (entry->skb != NULL &&
		    entry->first_frag_time + 2 * HZ < jiffies) {
			printk(KERN_DEBUG "%s: expiring fragment cache entry "
			       "seq=%u last_frag=%u\n",
			       local->dev->name, entry->seq, entry->last_frag);
			dev_kfree_skb(entry->skb);
			entry->skb = NULL;
		}

		if (entry->skb != NULL && entry->seq == seq &&
		    (entry->last_frag + 1 == frag || frag == -1) &&
		    memcmp(entry->src_addr, src, ETH_ALEN) == 0 &&
		    memcmp(entry->dst_addr, dst, ETH_ALEN) == 0)
			return entry;
	}

	return NULL;
}


/* Called only as a tasklet (software IRQ) */
static struct sk_buff *
mvWLAN_frag_cache_get(local_info_t *local, struct mvwlan_ieee80211_hdr *hdr)
{
	struct sk_buff *skb = NULL;
	u16 sc;
	unsigned int frag, seq;
	struct mvwlan_frag_entry *entry;

	sc = le16_to_cpu(hdr->seq_ctrl);
	frag = WLAN_GET_SEQ_FRAG(sc);
	seq = WLAN_GET_SEQ_SEQ(sc);

	if (frag == 0) {
		/* Reserve enough space to fit maximum frame length */
		skb = dev_alloc_skb(local->dev->mtu +
				    sizeof(struct mvwlan_ieee80211_hdr) +
				    8 /* LLC */ +
				    2 /* alignment */ +
				    8 /* WEP */ + ETH_ALEN /* WDS */);
		if (skb == NULL)
			return NULL;

		entry = &local->frag_cache[local->frag_next_idx];
		local->frag_next_idx++;
		if (local->frag_next_idx >= MVWLAN_FRAG_CACHE_LEN)
			local->frag_next_idx = 0;

		if (entry->skb != NULL)
			dev_kfree_skb(entry->skb);

		entry->first_frag_time = jiffies;
		entry->seq = seq;
		entry->last_frag = frag;
		entry->skb = skb;
                skb->dev = local->dev;
		memcpy(entry->src_addr, hdr->addr2, ETH_ALEN);
		memcpy(entry->dst_addr, hdr->addr1, ETH_ALEN);
	} else {
		/* received a fragment of a frame for which the head fragment
		 * should have already been received */
		entry = mvWLAN_frag_cache_find(local, seq, frag, hdr->addr2,
					       hdr->addr1);
		if (entry != NULL) {
			entry->last_frag = frag;
			skb = entry->skb;
		}
	}

	return skb;
}


/* Called only as a tasklet (software IRQ) */
static int mvWLAN_frag_cache_invalidate(local_info_t *local,
					struct mvwlan_ieee80211_hdr *hdr)
{
	u16 sc;
	unsigned int seq;
	struct mvwlan_frag_entry *entry;

	sc = le16_to_cpu(hdr->seq_ctrl);
	seq = WLAN_GET_SEQ_SEQ(sc);

	entry = mvWLAN_frag_cache_find(local, seq, -1, hdr->addr2, hdr->addr1);

	if (entry == NULL) {
		printk(KERN_DEBUG "%s: could not invalidate fragment cache "
		       "entry (seq=%u)\n",
		       local->dev->name, seq);
		return -1;
	}

	entry->skb = NULL;
	return 0;
}


/* Called only as a tasklet (software IRQ) */
static void schedule_packet_send(local_info_t *local, struct sta_info *sta)
{
	struct sk_buff *skb;
	struct mvwlan_ieee80211_hdr *hdr;
	struct mvwlan_80211_rx_status rx_stats;

	if (skb_queue_empty(&sta->tx_buf))
		return;

	skb = dev_alloc_skb(16);
	if (skb == NULL) {
		printk(KERN_DEBUG "%s: schedule_packet_send: skb alloc "
		       "failed\n", local->dev->name);
		return;
	}

	hdr = (struct mvwlan_ieee80211_hdr *) skb_put(skb, 16);

	/* Generate a fake pspoll frame to start packet delivery */
	hdr->frame_control = __constant_cpu_to_le16(
		(WLAN_FC_TYPE_CTRL << 2) | (WLAN_FC_STYPE_PSPOLL << 4));
	memcpy(hdr->addr1, local->dev->dev_addr, ETH_ALEN);
	memcpy(hdr->addr2, sta->addr, ETH_ALEN);
	hdr->duration_id = cpu_to_le16(sta->aid | BIT(15) | BIT(14));

	PDEBUG(DEBUG_PS2, "%s: Scheduling buffered packet delivery for "
	       "STA " MACSTR "\n", local->dev->name, MAC2STR(sta->addr));

	skb->dev = local->dev;

	memset(&rx_stats, 0, sizeof(rx_stats));
	mvWLAN_rx(local->dev, skb, &rx_stats);
}


static void update_sta_ps2(local_info_t *local, struct sta_info *sta,
			   int pwrmgt, int type, int stype)
{
	if (pwrmgt && !(sta->flags & WLAN_STA_PS)) {
		sta->flags |= WLAN_STA_PS;
		local->ap->ps_sta += 1;
		PDEBUG(DEBUG_PS2, "STA " MACSTR " changed to use PS "
		       "mode (type=0x%02X, stype=0x%02X)\n",
		       MAC2STR(sta->addr), type, stype);
	} else if (!pwrmgt && (sta->flags & WLAN_STA_PS)) {
		sta->flags &= ~WLAN_STA_PS;
		local->ap->ps_sta -= 1;
		PDEBUG(DEBUG_PS2, "STA " MACSTR " changed to not use "
		       "PS mode (type=0x%02X, stype=0x%02X)\n",
		       MAC2STR(sta->addr), type, stype);
		if (type != WLAN_FC_TYPE_CTRL || stype != WLAN_FC_STYPE_PSPOLL)
			schedule_packet_send(local, sta);
	}
}


/* Called only as a tasklet (software IRQ). Called for each RX frame to update
 * STA power saving state. pwrmgt is a flag from 802.11 frame_control field. */
static int update_sta_ps(local_info_t *local, struct mvwlan_ieee80211_hdr *hdr)
{
	struct sta_info *sta;
	u16 fc;

	spin_lock(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock(&local->ap->sta_table_lock);

	if (!sta)
		return -1;

	fc = le16_to_cpu(hdr->frame_control);
	update_sta_ps2(local, sta, fc & WLAN_FC_PWRMGT,
		       WLAN_FC_GET_TYPE(fc), WLAN_FC_GET_STYPE(fc));

	atomic_dec(&sta->users);
	return 0;
}


static inline int
mvWLAN_rx_mgmt_frame(local_info_t *local, struct sk_buff *skb,
		     struct mvwlan_80211_rx_status *rx_stats, u16 type,
		     u16 stype)
{
	IEEEtypes_Frame_t *rx_frame;

	rx_frame = (IEEEtypes_Frame_t *) skb->data;

	if (local->iw_mode == IW_MODE_MASTER) {
		update_sta_ps(local, (struct mvwlan_ieee80211_hdr *)
			      &(rx_frame->Hdr.FrmCtl));
	}

	if (local->hostapd && type == WLAN_FC_TYPE_MGMT) {
		/* Special software handling for probe request frames
		 * when the hidden SSID option is enabled.
		 */
		if ((stype == WLAN_FC_STYPE_PROBE_REQ) &&
		    (local->iw_mode == IW_MODE_MASTER) &&
		    ((local->enh_sec & MVWLAN_ENHSEC_BCAST_SSID) ==
		    MVWLAN_ENHSEC_BCAST_SSID)) {
			mvWLAN_rx(skb->dev, skb, rx_stats);
			return 0;
		}

		if (stype == WLAN_FC_STYPE_BEACON &&
		    local->iw_mode == IW_MODE_MASTER) {
			struct sk_buff *skb2;
			/* Process beacon frames also in kernel driver to
			 * update STA(AP) table statistics */
			skb2 = skb_clone(skb, GFP_ATOMIC);
			if (skb2)
				mvWLAN_rx(skb2->dev, skb2, rx_stats);
		}

		/* send management frames to the user space daemon for
		 * processing */
		local->apdevstats.rx_packets++;
		local->apdevstats.rx_bytes += skb->len;
		mvWLAN_rx_apd(local->apdev, skb, rx_stats);
		return 0;
	}

	if (local->iw_mode == IW_MODE_MASTER) {
		if (type != WLAN_FC_TYPE_MGMT && type != WLAN_FC_TYPE_CTRL) {
			printk(KERN_DEBUG "%s: unknown management frame "
			       "(type=0x%02x, stype=0x%02x) dropped\n",
			       skb->dev->name, type, stype);
			return -1;
		}

		mvWLAN_rx(skb->dev, skb, rx_stats);
		return 0;
	} else {
		printk(KERN_DEBUG "%s: mvWLAN_rx_mgmt_frame: management frame "
		       "received in non-Host AP mode\n", skb->dev->name);
		return -1;
	}
}


/* Called only as a tasklet (software IRQ) */
static inline mvwlan_wds_info_t *mvWLAN_rx_get_wds(local_info_t *local,
						   u8 *addr)
{
	mvwlan_wds_info_t *wds;

	spin_lock(&local->wdslock);
	wds = local->wds;
	while (wds != NULL && memcmp(wds->remote_addr, addr, ETH_ALEN) != 0)
		wds = wds->next;
	spin_unlock(&local->wdslock);

	return wds;
}


static inline int
mvWLAN_rx_wds_frame(local_info_t *local, struct mvwlan_ieee80211_hdr *hdr,
		    u16 fc, mvwlan_wds_info_t **wds)
{
	if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) !=
	    (WLAN_FC_TODS | WLAN_FC_FROMDS) &&
	    (!local->wds || local->iw_mode != IW_MODE_MASTER ||
	     !(fc & WLAN_FC_TODS)))
		return 0; /* not a WDS frame */

	/* Possible WDS frame: either IEEE 802.11 compliant (if FromDS) */
	if (memcmp(hdr->addr1, local->dev->dev_addr, ETH_ALEN) != 0 &&
	    (hdr->addr1[0] != 0xff || hdr->addr1[1] != 0xff ||
	     hdr->addr1[2] != 0xff || hdr->addr1[3] != 0xff ||
	     hdr->addr1[4] != 0xff || hdr->addr1[5] != 0xff)) {
		/* RA (or BSSID) is not ours - drop */
		PDEBUG(DEBUG_EXTRA, "%s: received WDS frame with "
		       "not own or broadcast %s=" MACSTR "\n",
		       local->dev->name, fc & WLAN_FC_FROMDS ? "RA" : "BSSID",
		       MAC2STR(hdr->addr1));
		return -1;
	}

	/* check if the frame came from a registered WDS connection */
	*wds = mvWLAN_rx_get_wds(local, hdr->addr2);
	if (*wds == NULL && fc & WLAN_FC_FROMDS &&
	    (local->iw_mode != IW_MODE_INFRA ||
	     !(local->wds_type & MVWLAN_WDS_AP_CLIENT) ||
	     memcmp(hdr->addr2, local->bssid, ETH_ALEN) != 0)) {
		/* require that WDS link has been registered with TA or the
		 * frame is from current AP when using 'AP client mode' */
		PDEBUG(DEBUG_EXTRA, "%s: received WDS[4 addr] frame "
		       "from unknown TA=" MACSTR "\n",
		       local->dev->name, MAC2STR(hdr->addr2));
		if (local->ap && local->ap->autom_ap_wds)
			mvWLAN_add_wds_link(local, hdr->addr2);
		return -1;
	}

	if (*wds && !(fc & WLAN_FC_FROMDS) && local->ap &&
	    mvWLAN_is_sta_assoc(local->ap, hdr->addr2)) {
		/* STA is actually associated with us even though it has a
		 * registered WDS link. Assume it is in 'AP client' mode.
		 * Since this is a 3-addr frame, assume it is not WDS
		 * frame and process it like any normal ToDS frame from
		 * associated STA. */
		*wds = NULL;
	}

	return 0;
}


static int mvWLAN_is_eapol_frame(local_info_t *local,
				 struct mvwlan_ieee80211_hdr *hdr, u8 *buf,
				 int len)
{
	struct net_device *dev = local->dev;
	u16 fc, ethertype;

	fc = le16_to_cpu(hdr->frame_control);

	/* check that the frame is unicast frame to us */
	if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) == WLAN_FC_TODS &&
	    memcmp(hdr->addr1, dev->dev_addr, ETH_ALEN) == 0 &&
	    memcmp(hdr->addr3, dev->dev_addr, ETH_ALEN) == 0) {
		/* ToDS frame with own addr BSSID and DA */
	} else if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) == WLAN_FC_FROMDS &&
		   memcmp(hdr->addr1, dev->dev_addr, ETH_ALEN) == 0) {
		/* FromDS frame with own addr as DA */
	} else
		return 0;

	if (len < 8)
		return 0;

	/* check for port access entity Ethernet type */
	ethertype = (buf[6] << 8) | buf[7];
	if (ethertype == ETH_P_PAE)
		return 1;

	return 0;
}


/* Called only as a tasklet (software IRQ) */
static inline int
mvWLAN_decrypt_rx_frame(local_info_t *local, int iswep, struct sk_buff *skb)
{
	IEEEtypes_Frame_t *rx_frame;
	struct mvwlan_ieee80211_hdr *hdr;
	struct mvwlan_crypt_data *crypt;
	int ret = 0, olen, len;
	char *payload;
	MIB_COUNTERS *mib_Counters_p =
			&(local->sysConfig->Mib802dot11->CountersTable);
	MIB_PRIVACY_TABLE *mib_Privacy_p =
			&(local->sysConfig->Mib802dot11->Privacy);

	rx_frame = (IEEEtypes_Frame_t *) skb->data;
	hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);

	len = skb->len - sizeof(rx_frame->Hdr);
	payload = rx_frame->Body;
	crypt = local->crypt;

	/* allow NULL decrypt to indicate an station specific override for
	 * default encryption */
	if (crypt && (crypt->ops == NULL || crypt->ops->decrypt_mpdu == NULL))
		crypt = NULL;

	if (!crypt && iswep) {
		printk(KERN_DEBUG "%s: WEP decryption failed (not set) (SA="
		       MACSTR ")\n", local->dev->name, MAC2STR(hdr->addr2));
		mib_Counters_p->WepUndecryptCnt++;
		ret = -1;
		goto done;
	}

	if (!crypt)
		goto done;

	if (!iswep && (mib_Privacy_p->PrivInvoked || mib_Privacy_p->RSNEnabled)) {
		if ((local->ieee_802_1x || mib_Privacy_p->RSNEnabled) &&
		    mvWLAN_is_eapol_frame(local, hdr, payload, len)) {
			/* pass unencrypted EAPOL frames even if encryption is
			 * configured */
			printk(KERN_DEBUG "%s: RX: IEEE 802.1X - passing "
			       "unencrypted EAPOL frame\n", local->dev->name);
			goto done;
		}

#if defined(BOOSTER_MODE) || defined(TURBO_SETUP)
		if (memcmp(payload, "\xAA\xAA\x03\x00\x50\x43", 6) == 0) {
			/* pass unencrypted MARVELL OUI frames even if encryption is
			 * configured */
			printk(KERN_DEBUG "%s: RX: Marvell Private - passing "
			       "unencrypted MARVELL OUI frame\n", local->dev->name);
			goto done;
		}
#endif

		printk(KERN_DEBUG "%s: encryption configured, but RX frame "
		       "not encrypted (SA=" MACSTR ")\n",
		       local->dev->name, MAC2STR(hdr->addr2));
		ret = -1;
		goto done;
	}

	/* decrypt WEP part of the frame: IV (4 bytes), encrypted
	 * payload (including SNAP header), ICV (4 bytes) */
	atomic_inc(&crypt->refcnt);
	olen = crypt->ops->decrypt_mpdu(skb->data, payload, len, crypt->priv);
	atomic_dec(&crypt->refcnt);
	if (olen < 0) {
		printk(KERN_DEBUG "%s: WEP decryption failed (SA=" MACSTR
		       ")\n", local->dev->name, MAC2STR(hdr->addr2));
		mib_Counters_p->WepUndecryptCnt++;
		ret = -1;
		goto done;
	}

	skb_trim(skb, skb->len - (len - olen));

 done:

	return ret;
}


/* Called only as a tasklet (software IRQ) */
static inline int
mvWLAN_decrypt_rx_frame_msdu(local_info_t *local, int iswep, struct sk_buff *skb)
{
	IEEEtypes_Frame_t *rx_frame;
	struct mvwlan_ieee80211_hdr *hdr;
	struct mvwlan_crypt_data *crypt;
	int ret = 0, olen, len;
	char *payload;
	MIB_COUNTERS *mib_Counters_p =
			&(local->sysConfig->Mib802dot11->CountersTable);

	rx_frame = (IEEEtypes_Frame_t *) skb->data;
	hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);

	len = skb->len - sizeof(rx_frame->Hdr);
	payload = rx_frame->Body;
	crypt = local->crypt;

	/* allow NULL decrypt to indicate an station specific override for
	 * default encryption */
	if (crypt && (crypt->ops == NULL || crypt->ops->decrypt_msdu == NULL))
		crypt = NULL;

	if (!crypt)
		goto done;

	if (!iswep)   /* Only EAPOL frames will come to here */
		goto done;

	atomic_inc(&crypt->refcnt);
	olen = crypt->ops->decrypt_msdu(hdr->addr3, hdr->addr2, payload, len, crypt->priv);
	atomic_dec(&crypt->refcnt);
	if (olen < 0) {
		printk(KERN_DEBUG "%s: WEP decryption failed (SA=" MACSTR
		       ")\n", local->dev->name, MAC2STR(hdr->addr2));
		mib_Counters_p->WepUndecryptCnt++;
		ret = -1;
		goto done;
	}

	skb_trim(skb, skb->len - (len - olen));

 done:

	return ret;
}


/* Called only as a tasklet (software IRQ). Called for each RX frame after
 * getting RX header and payload from hardware. */
static ap_rx_ret mvWLAN_handle_sta_rx(local_info_t *local,
				      struct net_device *dev,
				      struct sk_buff *skb,
				      struct mvwlan_80211_rx_status *rx_stats,
				      int wds)
{
	int ret;
	struct sta_info *sta;
	u16 fc, type, stype;
	IEEEtypes_Frame_t *rx_frame;
	struct mvwlan_ieee80211_hdr *hdr;

	if (local->ap == NULL)
		return AP_RX_CONTINUE;

	rx_frame = (IEEEtypes_Frame_t *) skb->data;
	hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);

	fc = le16_to_cpu(hdr->frame_control);
	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);

	spin_lock(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, hdr->addr2);
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock(&local->ap->sta_table_lock);

	if (sta && !(sta->flags & WLAN_STA_AUTHORIZED))
		ret = AP_RX_CONTINUE_NOT_AUTHORIZED;
	else
		ret = AP_RX_CONTINUE;


	if (fc & WLAN_FC_TODS) {
		if (!wds && (sta == NULL || !(sta->flags & WLAN_STA_ASSOC))) {
			if (local->hostapd) {
				mvWLAN_rx_apd(local->apdev, skb, rx_stats);
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
			} else {
				printk(KERN_DEBUG "%s: dropped received packet"
				       " from non-associated STA " MACSTR
				       " (type=0x%02x, subtype=0x%02x)\n",
				       dev->name, MAC2STR(hdr->addr2), type,
				       stype);
				mvWLAN_rx(dev, skb, rx_stats);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
			}
			ret = AP_RX_EXIT;
			goto out;
		}
	} else if (fc & WLAN_FC_FROMDS) {
		if (!wds) {
			/* FromDS frame - not for us; probably
			 * broadcast/multicast in another BSS - drop */
			if (memcmp(hdr->addr1, dev->dev_addr, ETH_ALEN) == 0) {
				printk(KERN_DEBUG "Odd.. FromDS packet "
				       "received with own BSSID\n");
				mvWLAN_dump_rx_80211(dev->name, skb, rx_stats);
			}
			ret = AP_RX_DROP;
			goto out;
		}
	} else if (stype == WLAN_FC_STYPE_NULLFUNC && sta == NULL &&
		   memcmp(hdr->addr1, dev->dev_addr, ETH_ALEN) == 0) {

		if (local->hostapd) {
			mvWLAN_rx_apd(local->apdev, skb, rx_stats);
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
		} else {
			/* At least Lucent f/w seems to send data::nullfunc
			 * frames with no ToDS flag when the current AP returns
			 * after being unavailable for some time. Speed up
			 * re-association by informing the station about it not
			 * being associated. */
			printk(KERN_DEBUG "%s: rejected received nullfunc "
			       "frame without ToDS from not associated STA "
			       MACSTR "\n",
			       dev->name, MAC2STR(hdr->addr2));
			mvWLAN_rx(dev, skb, rx_stats);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
		}
		ret = AP_RX_EXIT;
		goto out;
	} else if (stype == WLAN_FC_STYPE_NULLFUNC) {
		/* At least Lucent cards seem to send periodic nullfunc
		 * frames with ToDS. Let these through to update SQ
		 * stats and PS state. Nullfunc frames do not contain
		 * any data and they will be dropped below. */
	} else {
		/* If BSSID (Addr3) is foreign, this frame is a normal
		 * broadcast frame from an IBSS network. Drop it silently.
		 * If BSSID is own, report the dropping of this frame. */
		if (memcmp(hdr->addr3, dev->dev_addr, ETH_ALEN) == 0) {
			printk(KERN_DEBUG "%s: dropped received packet from "
			       MACSTR " with no ToDS flag (type=0x%02x, "
			       "subtype=0x%02x)\n", dev->name,
			       MAC2STR(hdr->addr2), type, stype);
			mvWLAN_dump_rx_80211(dev->name, skb, rx_stats);
		}
		ret = AP_RX_DROP;
		goto out;
	}

	if (sta) {

		MVWLAN_RATE_UpdateStationRxStats(sta, rx_stats);

		mvWLAN_update_sq(sta, rx_stats);

		update_sta_ps2(local, sta, fc & WLAN_FC_PWRMGT, type, stype);

		sta->rx_packets++;
		sta->rx_bytes += skb->len;
		sta->last_rx = jiffies;

#ifndef AP_WPA2
		if (local->sysConfig->Mib802dot11->Privacy.RSNEnabled)
			local->crypt->priv = (void *) sta;
#else
		if (sta && local->sysConfig->Mib802dot11->Privacy.RSNEnabled) {

			if ((sta->key_mgmt_info.RsnIEBuf[0] == 48 && sta->key_mgmt_info.RsnIEBuf[13] == 4) ||
				(sta->key_mgmt_info.RsnIEBuf[0] == 221 && sta->key_mgmt_info.RsnIEBuf[17] == 4))
				local->crypt = local->ccmp_crypt;
			else
				local->crypt = local->tkip_crypt;

			local->crypt->priv = (void *) sta;
		}
#endif
	}

	if (local->ap->nullfunc_ack && stype == WLAN_FC_STYPE_NULLFUNC &&
	    fc & WLAN_FC_TODS) {
		if (local->hostapd) {
			mvWLAN_rx_apd(local->apdev, skb, rx_stats);
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
		} else {
			/* XXX, need to very this for our hw */
			/* some STA f/w's seem to require control::ACK frame
			 * for data::nullfunc, but Prism2 f/w 0.8.0 (at least
			 * from Compaq) does not send this.. Try to generate
			 * ACK for these frames from the host driver to make
			 * power saving work with, e.g., Lucent WaveLAN f/w */
			mvWLAN_rx(dev, skb, rx_stats);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
		}
		ret = AP_RX_EXIT;
		goto out;
	}

 out:
	if (sta)
		atomic_dec(&sta->users);

	return ret;
}


/* All received frames are sent to this function. @skb contains the frame in
 * IEEE 802.11 format, i.e., in the format it was sent over air.
 * This function is called only as a tasklet (software IRQ). */
void mvWLAN_rx_80211_frame(struct net_device *dev, struct sk_buff *skb,
			   struct mvwlan_80211_rx_status *rx_stats)
{
	local_info_t *local = dev->priv;
	IEEEtypes_Frame_t *rx_frame;
	struct mvwlan_ieee80211_hdr *hdr;
	size_t hdrlen;
	u16 fc, type, stype, sc;
	mvwlan_wds_info_t *wds = NULL;
	struct net_device_stats *stats;
	unsigned int frag;
	u8 *payload;
	struct sk_buff *skb2 = NULL;
	u16 ethertype;
	int frame_authorized = 0;
	int from_assoc_ap = 0;
	u8 dst[ETH_ALEN];
	u8 src[ETH_ALEN];

	rx_frame = (IEEEtypes_Frame_t *) skb->data;
	hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);
	stats = mvWLAN_get_stats(dev);

	if (skb->len < 10)
		goto rx_dropped;

	fc = le16_to_cpu(hdr->frame_control);
	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);
	sc = le16_to_cpu(hdr->seq_ctrl);
	frag = WLAN_GET_SEQ_FRAG(sc);
	hdrlen = sizeof(rx_frame->Hdr);

#if WIRELESS_EXT > 15
	/* Put this code here so that we avoid duplicating it in all
	 * Rx paths. - Jean II */
#ifdef IW_WIRELESS_SPY		/* defined in iw_handler.h */
	/* If spy monitoring on */
	if (local->spy_data.spy_number > 0) {
		struct iw_quality wstats;
		wstats.level = rx_stats->signal;
		wstats.noise = rx_stats->noise;
		wstats.updated = 6;	/* No qual value */
		/* Update spy records */
		wireless_spy_update(dev, hdr->addr2, &wstats);
	}
#endif /* IW_WIRELESS_SPY */
#endif /* WIRELESS_EXT > 15 */

	if (local->iw_mode == IW_MODE_MONITOR) {
		mvWLAN_rx_monitor_frame(dev, skb, rx_stats);
		return;
	}

	/* Mgmt frames go to the following handler */
	if (type != WLAN_FC_TYPE_DATA) {
		/* Only one kind of mgmt frames that need to be decrypted */
		if (type == WLAN_FC_TYPE_MGMT && stype == WLAN_FC_STYPE_AUTH &&
		    fc & WLAN_FC_ISWEP && local->host_decrypt &&
		    mvWLAN_decrypt_rx_frame(local, fc & WLAN_FC_ISWEP, skb)) {
			printk(KERN_DEBUG "%s: failed to decrypt mgmt(auth) "
			       "from " MACSTR "\n", dev->name,
			       MAC2STR(hdr->addr2));
			/* TODO: could inform AP daemon about this so that it
			 * could send auth failure report */
			goto rx_dropped;
		}

		if (mvWLAN_rx_mgmt_frame(local, skb, rx_stats, type, stype))
			goto rx_dropped;
		else
			goto rx_exit;
	}

	/* Data frame - extract src/dst addresses */
	if (skb->len < IEEE80211_DATA_HDR3_LEN)
		goto rx_dropped;

	switch (fc & (WLAN_FC_FROMDS | WLAN_FC_TODS)) {
	case WLAN_FC_FROMDS:
		memcpy(dst, hdr->addr1, ETH_ALEN);
		memcpy(src, hdr->addr3, ETH_ALEN);
		break;
	case WLAN_FC_TODS:
		memcpy(dst, hdr->addr3, ETH_ALEN);
		memcpy(src, hdr->addr2, ETH_ALEN);
		break;
	case WLAN_FC_FROMDS | WLAN_FC_TODS:
		if (skb->len < IEEE80211_DATA_HDR4_LEN)
			goto rx_dropped;
		memcpy(dst, hdr->addr3, ETH_ALEN);
		memcpy(src, hdr->addr4, ETH_ALEN);
		break;
	case 0:
		memcpy(dst, hdr->addr1, ETH_ALEN);
		memcpy(src, hdr->addr2, ETH_ALEN);
		break;
	}

	/* Perform WDS checking */
	if (mvWLAN_rx_wds_frame(local, hdr, fc, &wds))
		goto rx_dropped;
	if (wds) {
		skb->dev = dev = &wds->dev;
		stats = mvWLAN_get_stats(dev);
	}

	if (local->iw_mode == IW_MODE_MASTER && !wds &&
	    (fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) == WLAN_FC_FROMDS &&
	    local->stadev &&
	    memcmp(hdr->addr2, local->assoc_ap_addr, ETH_ALEN) == 0) {
		/* Frame from BSSID of the AP for which we are a client */
		skb->dev = dev = local->stadev;
		stats = mvWLAN_get_stats(dev);
		from_assoc_ap = 1;
	}

	dev->last_rx = jiffies;

	if (local->iw_mode == IW_MODE_MASTER && !from_assoc_ap) {
		switch (mvWLAN_handle_sta_rx(local, dev, skb, rx_stats,
					     wds != NULL)) {
		case AP_RX_CONTINUE_NOT_AUTHORIZED:
			frame_authorized = 0;
			break;
		case AP_RX_CONTINUE:
			frame_authorized = 1;
			break;
		case AP_RX_DROP:
			goto rx_dropped;
		case AP_RX_EXIT:
			goto rx_exit;
		}
	} else if (local->iw_mode == IW_MODE_REPEAT ||
		   local->wds_type & MVWLAN_WDS_AP_CLIENT)
		mvWLAN_update_rx_stats(local->ap, hdr, rx_stats);

	/* Nullfunc frames may have PS-bit set, so they must be passed to
	 * mvWLAN_handle_sta_rx() before being dropped here. */
	if (stype != WLAN_FC_STYPE_DATA &&
	    stype != WLAN_FC_STYPE_DATA_CFACK &&
	    stype != WLAN_FC_STYPE_DATA_CFPOLL &&
	    stype != WLAN_FC_STYPE_DATA_CFACKPOLL) {
		if (stype != WLAN_FC_STYPE_NULLFUNC)
			printk(KERN_DEBUG "%s: RX: dropped data frame "
			       "with no data (type=0x%02x, subtype=0x%02x)\n",
			       dev->name, type, stype);
		goto rx_dropped;
	}

	/* skb: hdr + (possibly fragmented, possibly encrypted) payload */

	if (local->host_decrypt &&
	    mvWLAN_decrypt_rx_frame(local, fc & WLAN_FC_ISWEP, skb))
		goto rx_dropped;

	/* skb: hdr + (possibly fragmented) plaintext payload */

	if (local->host_decrypt && (fc & WLAN_FC_ISWEP) &&
	    (frag != 0 || (fc & WLAN_FC_MOREFRAG))) {
		int flen;
		struct sk_buff *frag_skb =
			mvWLAN_frag_cache_get(local, hdr);
		if (!frag_skb) {
			printk(KERN_DEBUG "%s: Rx cannot get skb from "
			       "fragment cache (morefrag=%d seq=%u frag=%u)\n",
			       dev->name, (fc & WLAN_FC_MOREFRAG) != 0,
			       WLAN_GET_SEQ_SEQ(sc), frag);
			goto rx_dropped;
		}

		flen = skb->len;
		if (frag != 0)
			flen -= hdrlen;

		if (frag_skb->tail + flen > frag_skb->end) {
			printk(KERN_WARNING "%s: host decrypted and "
			       "reassembled frame did not fit skb\n",
			       dev->name);
			mvWLAN_frag_cache_invalidate(local, hdr);
			dev_kfree_skb(frag_skb);
			goto rx_dropped;
		}

		if (frag == 0) {
			/* copy first fragment (including full headers) into
			 * beginning of the fragment cache skb */
			memcpy(skb_put(frag_skb, flen), skb->data, flen);
		} else {
			/* append frame payload to the end of the fragment
			 * cache skb */
			memcpy(skb_put(frag_skb, flen), skb->data + sizeof(rx_frame->Hdr), flen);
		}
		dev_kfree_skb(skb);
		skb = NULL;

		if (fc & WLAN_FC_MOREFRAG) {
			/* more fragments expected - leave the skb in fragment
			 * cache for now; it will be delivered to upper layers
			 * after all fragments have been received */
			goto rx_exit;
		}

		/* this was the last fragment and the frame will be
		 * delivered, so remove skb from fragment cache */
		skb = frag_skb;
		rx_frame = (IEEEtypes_Frame_t *) skb->data;
		hdr = (struct mvwlan_ieee80211_hdr *) &(rx_frame->Hdr.FrmCtl);
		mvWLAN_frag_cache_invalidate(local, hdr);
	}

	/* check if we need to do MSDU checking, if yes, according to the checking result to
	 * handle packet
	 */
	if (local->host_decrypt &&
	    mvWLAN_decrypt_rx_frame_msdu(local, fc & WLAN_FC_ISWEP, skb))
		goto rx_dropped;

	/* skb: hdr + (possible reassembled) full plaintext payload */

	payload = rx_frame->Body;
	ethertype = (payload[6] << 8) | payload[7];

	if (memcmp(payload, "\xAA\xAA\x03\x00\x50\x43", 6) == 0) {

#ifdef TURBO_SETUP
	if (local->iw_mode == IW_MODE_MASTER) {
		if (ethertype == 0x0001) {
			printk(KERN_DEBUG "%s: RX: Marvell OUI frame for auto link\n",
			       dev->name);
			if (local->hostapd && local->apdev) {
				/* Send Marvell OUI frames to the user
				 * space daemon for processing */
				mvWLAN_rx_apd(local->apdev, skb, rx_stats);
				local->apdevstats.rx_packets++;
				local->apdevstats.rx_bytes += skb->len;
				goto rx_exit;
			}
		}
	}
#endif

#ifdef BOOSTER_MODE
	if (MVWLAN_BOOSTER_GetBoosterMode(local) == 0) {
		if (local->sysConfig->Mib802dot11->StationConfig.mib_BoosterMode != 0) {
			/* check for MARVELL OUI */
			if (payload[9] == 2) {
				MVWLAN_BOOSTER_CheckToSetBoosterMode(local);
				goto rx_dropped;
			}
		}
	}
#endif
	}

	/* If IEEE 802.1X is used, check whether the port is authorized to send
	 * the received frame. */
	if ((local->ieee_802_1x || local->sysConfig->Mib802dot11->Privacy.RSNEnabled )
		&& local->iw_mode == IW_MODE_MASTER) {
		if (ethertype == ETH_P_PAE) {
			printk(KERN_DEBUG "%s: RX: IEEE 802.1X frame\n",
			       dev->name);
			if (local->hostapd && local->apdev) {
				/* Send IEEE 802.1X frames to the user
				 * space daemon for processing */
				mvWLAN_rx_apd(local->apdev, skb, rx_stats);
				local->apdevstats.rx_packets++;
				local->apdevstats.rx_bytes += skb->len;
				goto rx_exit;
			}
		} else if (!frame_authorized) {
			printk(KERN_DEBUG "%s: dropped frame from "
			       "unauthorized port (IEEE 802.1X): "
			       "ethertype=0x%04x\n",
			       dev->name, ethertype);
			goto rx_dropped;
		}
	}

	/* convert hdr + possible LLC headers into Ethernet header */
	if (skb->len - hdrlen >= 8 &&
	    ((memcmp(payload, rfc1042_header, 6) == 0 &&
	      ethertype != ETH_P_AARP && ethertype != ETH_P_IPX) ||
	     memcmp(payload, bridge_tunnel_header, 6) == 0)) {
		/* remove RFC1042 or Bridge-Tunnel encapsulation and
		 * replace EtherType */
		skb_pull(skb, hdrlen + 6);
		memcpy(skb_push(skb, ETH_ALEN), src, ETH_ALEN);
		memcpy(skb_push(skb, ETH_ALEN), dst, ETH_ALEN);
	} else {
		u16 len;
		/* Leave Ethernet header part of hdr and full payload */
		skb_pull(skb, hdrlen);
		len = htons(skb->len);
		memcpy(skb_push(skb, 2), &len, 2);
		memcpy(skb_push(skb, ETH_ALEN), src, ETH_ALEN);
		memcpy(skb_push(skb, ETH_ALEN), dst, ETH_ALEN);
	}

	skb->tail = skb->data + skb->len;

	stats->rx_packets++;
	stats->rx_bytes += skb->len;

	if (local->iw_mode == IW_MODE_MASTER && !wds &&
	    local->ap->bridge_packets) {
		if (dst[0] & 0x01) {
			/* copy multicast frame both to the higher layers and
			 * to the wireless media */
			local->ap->bridged_multicast++;

            if (local->ap->sta_bridge == 1)
            {
    			skb2 = skb_clone(skb, GFP_ATOMIC);
    			if (skb2 == NULL)
    				printk(KERN_DEBUG "%s: skb_clone failed for "
    				       "multicast frame\n", dev->name);
            }
		} else if (mvWLAN_is_sta_assoc(local->ap, dst)) {
		    if (local->ap->sta_bridge == 1)
            {
    			/* send frame directly to the associated STA using
    			 * wireless media and not passing to higher layers */
    			local->ap->bridged_unicast++;
    			skb2 = skb;
    			skb = NULL;
		    }
            else
                goto rx_dropped;
		}
	}

	if (skb2 != NULL) {
		/* send to wireless media */
		skb2->protocol = __constant_htons(ETH_P_802_3);
		skb2->mac.raw = skb2->nh.raw = skb2->data;
		/* skb2->nh.raw = skb2->data + ETH_HLEN; */
		dev_queue_xmit(skb2);
	}

	if (skb) {
		skb->protocol = eth_type_trans(skb, dev);
		memset(skb->cb, 0, sizeof(skb->cb));
		skb->dev = dev;
		netif_rx(skb);
	}

 rx_exit:
	return;

 rx_dropped:
	dev_kfree_skb(skb);

	stats->rx_dropped++;
	goto rx_exit;
}


EXPORT_SYMBOL(mvWLAN_rx_80211_frame);
