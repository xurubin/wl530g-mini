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
#include <asm/memory.h>
#include <asm/uaccess.h>

#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"

#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_hw.h"
#include "mvWLAN_rate.h"


typedef enum {
	AP_TX_CONTINUE, AP_TX_DROP, AP_TX_RETRY, AP_TX_BUFFERED,
	AP_TX_CONTINUE_NOT_AUTHORIZED
} ap_tx_ret;


#define FRAGMENT_HEADER_SIZE      32
#define IEEE80211_SIFS            10    /* 10us */
#define IEEE80211_SHORT_PREAM     56    /* 56us */
#define IEEE80211_LONG_PREAM      128   /* 128us */
#define IEEE80211b_SHORT_PREAM    56    /* 56us */
#define IEEE80211b_LONG_PREAM     128   /* 128us */
#define IEEE80211g_SHORT_PREAM    8     /* 56us */
#define IEEE80211g_LONG_PREAM     16    /* 128us */
#define CTS_BASIC_DUR             112   /* 14*8 */

#define IEEE80211_FCS_SIZE        4

/*CTS & ACK durations in microsecond at 1,2,5.5,11,22,54 Mbit/s */
static UINT8 Cts_Ack_Dur[IEEEtypes_MAX_DATA_RATES_G] = {
	112,
	56,
	21,
	11,
	6,
	19,
	13,
	10,
	7,
	5,
	4,
	3,
	3,
	2
};

/* Group Key -- defined in wl_mib.c */
extern MRVL_MIB_RSN_GRP_KEY mib_MrvlRSN_GrpKey;

static u8 GetCtsAckDur(u8 rateid)
{
	if (rateid <= (IEEEtypes_MAX_DATA_RATES_G-1))
		return (Cts_Ack_Dur[rateid]);
	return (0);
}


static u16 GetDataRate(u8 rateid)
{
	switch (rateid) {
	case 0:   /* 1 Mbit/s */
		return (10);
	case 1:   /* 2 Mbit/s */
		return (20);
	case 2:   /* 5.5 Mbit/s */
		return (55);
	case 3:   /* 11 Mbit/s */
		return (110);
	case 4:   /* 22 Mbit/s */
		return (220);
	case 5:   /* 6 Mbit/s */
		return (60);
	case 6:   /* 9 Mbit/s */
		return (90);
	case 7:   /* 12 Mbit/s */
		return (120);
	case 8:   /* 18 Mbit/s */
		return (180);
	case 9:   /* 24 Mbit/s */
		return (240);
	case 10:  /* 36 Mbit/s */
		return (360);
	case 11:  /* 48 Mbit/s */
		return (480);
	case 12:  /* 54 Mbit/s */
		return (540);
	case 13:  /* 72 Mbit/s */
		return (720);
	}
	return (20);
}


void mvWLAN_dump_tx_header(const char *name, MRVL_TX_DONE_INFO *txdone,
			   IEEEtypes_Frame_t *frame802dot11)
{
	u16 fc, hdr_len;
	u8 *addr1, *addr2, *addr3, *addr4;

	if (txdone)
		printk(KERN_DEBUG "%s: TX status=0x%04x retry_count=%d "
		       "tx_rate=%d; jiffies=%ld\n", name,
		       __le16_to_cpu(txdone->status), txdone->retryCnt,
		       txdone->rateID, jiffies);

	fc = __le16_to_cpu(*((u16 *) &(frame802dot11->Hdr.FrmCtl)));
	printk(KERN_DEBUG "   FC=0x%04x (type=%d:%d) dur=0x%04x seq=0x%04x "
	       "data_len=%d%s%s\n",
	       fc, WLAN_FC_GET_TYPE(fc), WLAN_FC_GET_STYPE(fc),
	       __le16_to_cpu(frame802dot11->Hdr.DurationId),
	       __le16_to_cpu(frame802dot11->Hdr.SeqCtl),
	       __le16_to_cpu(frame802dot11->Hdr.FrmBodyLen),
	       fc & WLAN_FC_TODS ? " [ToDS]" : "",
	       fc & WLAN_FC_FROMDS ? " [FromDS]" : "");

	hdr_len = mvWLAN_80211_get_hdrlen(fc);
	addr1 = (u8 *)&frame802dot11->Hdr.Addr1;
	addr2 = (u8 *)&frame802dot11->Hdr.Addr2;
	addr3 = (u8 *)&frame802dot11->Hdr.Addr3;
	addr4 = (u8 *)&frame802dot11->Hdr.Addr4;
	printk(KERN_DEBUG "   A1=" MACSTR, MAC2STR(addr1));
	if (hdr_len >= 16)
		printk(" A2=" MACSTR, MAC2STR(addr2));
	if (hdr_len >= 24)
		printk(" A3=" MACSTR, MAC2STR(addr3));
	if (hdr_len >= 32)
		printk(" A4=" MACSTR, MAC2STR(addr4));
	printk("\n");
}


/* Called for each TX frame prior possible
 * encryption and transmit. */
static ap_tx_ret mvWLAN_handle_sta_tx(local_info_t *local,
				      struct sk_buff *skb,
				      IEEEtypes_Frame_t *tx_frame,
				      int wds, void **sta_ptr, int *rate_idx, int * broadcast)
{
	struct sta_info *sta = NULL;
	int set_tim, rate;
	u16 fc, type, stype;
	int ret = AP_TX_CONTINUE;

	fc = __le16_to_cpu(*((u16 *) &(tx_frame->Hdr.FrmCtl)));
	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);

	if (local->ap == NULL)
		goto out;

	if (*((u8 *)&(tx_frame->Hdr.Addr1)) & 0x01) {

		if (local->sysConfig->Mib802dot11->Privacy.RSNEnabled && !local->ap->gtk_installed) {

			ret = AP_TX_DROP;
		} else {

			/* Use the highest rate in Basic Rate Set */
			if (rate_idx)
				*rate_idx = local->max_basic_rate_idx;

			if (broadcast)
				*broadcast = 1;
		}

		/* broadcast/multicast frame - no AP related processing */
		goto out;
	}

	/* unicast packet - check whether destination STA is associated */
	spin_lock_bh(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap,
				(u8 *)&(tx_frame->Hdr.Addr1));
	if (sta)
		atomic_inc(&sta->users);
	spin_unlock_bh(&local->ap->sta_table_lock);

	if (local->iw_mode == IW_MODE_MASTER && sta == NULL && !wds) {
		printk(KERN_DEBUG "AP: drop packet to non-associated STA "
		       MACSTR "\n",
		       MAC2STR((u8 *)&(tx_frame->Hdr.Addr1)));
		ret = AP_TX_DROP;
		goto out;
	}

	if (sta == NULL) {
		/* The STA might not be associated when the probe request
		 * frame is sent.
		 */
		if (type == WLAN_FC_TYPE_MGMT &&
		    stype == WLAN_FC_STYPE_PROBE_RESP) {
			*rate_idx = 1;
			ret = AP_TX_CONTINUE_NOT_AUTHORIZED;
		}
		goto out;
	}

	if (!(sta->flags & WLAN_STA_AUTHORIZED))
		ret = AP_TX_CONTINUE_NOT_AUTHORIZED;

	if (local->fixed_tx_data_rate) {

		/* Set fixed tx_rate based on client mode */
		if (((sta->tx_supp_rates & 0x0000000F) != 0) &&
			((sta->tx_supp_rates & 0xFFFFFFF0) == 0)) {
			if (rate_idx)
				*rate_idx = local->fixed_tx_b_rate_idx;
		} else {
			if (rate_idx)
				*rate_idx = local->fixed_tx_g_rate_idx;
		}

		MVWLAN_RATE_UpdatePower(*rate_idx, sta->aid, 0);
	} else {

		/* Set tx_rate using host-based TX rate control */
		sta->tx_rate_idx = MVWLAN_RATE_GetStationRate(sta);

		local->ap->last_tx_rate = rate = sta->tx_rate =
			GetDataRate(sta->tx_rate_idx);
		if (rate_idx)
			*rate_idx = sta->tx_rate_idx;
	}

	if (local->iw_mode != IW_MODE_MASTER)
		goto out;

	if (!(sta->flags & WLAN_STA_PS))
		goto out;

	if (!skb)
		goto out;

	if (memcmp(skb->cb, AP_SKB_CB_MAGIC, AP_SKB_CB_MAGIC_LEN) == 0) {
		if (skb->cb[AP_SKB_CB_MAGIC_LEN] & AP_SKB_CB_BUFFERED_FRAME) {
			/* packet was already buffered and now send due to
			 * PS poll, so do not rebuffer it */
			tx_frame->Hdr.FrmCtl.MoreData = 1;
			goto out;
		}
	}

	if (skb_queue_len(&sta->tx_buf) >= STA_MAX_TX_BUFFER) {
		PDEBUG(DEBUG_PS, "%s: No more space in STA (" MACSTR ")'s PS "
		       "mode buffer\n", local->dev->name, MAC2STR(sta->addr));
		/* Make sure that TIM is set for the station (it might not be
		 * after AP wlan hw reset). */
		mvWLAN_set_tim(local, sta->aid, 1);
		sta->flags |= WLAN_STA_TIM;
		ret = AP_TX_DROP;
		goto out;
	}

	/* STA in PS mode, buffer frame for later delivery */
	set_tim = skb_queue_empty(&sta->tx_buf);
	skb_queue_tail(&sta->tx_buf, skb);
	/* FIX: could save RX time to skb and expire buffered frames after
	 * some time if STA does not poll for them */

	if (set_tim) {
		if (sta->flags & WLAN_STA_TIM)
			PDEBUG(DEBUG_PS2, "Re-setting TIM for aid %d\n",
			       sta->aid);
		mvWLAN_set_tim(local, sta->aid, 1);
		sta->flags |= WLAN_STA_TIM;
	}

	ret = AP_TX_BUFFERED;

 out:
	if (sta != NULL) {
		if (ret == AP_TX_CONTINUE ||
		    ret == AP_TX_CONTINUE_NOT_AUTHORIZED) {
			sta->tx_packets++;
			sta->tx_bytes += le16_to_cpu(tx_frame->Hdr.FrmBodyLen) +
					 mvWLAN_80211_get_hdrlen(fc);
			sta->last_tx = jiffies;
			if (sta_ptr)
				*sta_ptr = sta;  /* mvWLAN_handle_sta_release() will be called to release sta info later */
		} else
			atomic_dec(&sta->users);
	}

	return ret;

}


/* Called only as a tasklet (software IRQ) */
static void mvWLAN_tx_callback(local_info_t *local,
			       IEEEtypes_Frame_t *tx_frame,
			       int cb_idx, int ok)
{
	u16 fc, hdrlen, len;
	struct sk_buff *skb;
	struct mvwlan_tx_callback_info *cb;

	/* Make sure that frame was from us. */
	if (memcmp(tx_frame->Hdr.Addr2, local->dev->dev_addr, ETH_ALEN)) {
		printk(KERN_DEBUG "%s: TX callback - foreign frame\n",
		       local->dev->name);
		return;
	}

	spin_lock_bh(&local->cblock);
	cb = local->tx_callback;
	while (cb != NULL && cb->idx != cb_idx)
		cb = cb->next;
	spin_unlock_bh(&local->cblock);

	if (cb == NULL) {
		printk(KERN_DEBUG "%s: could not find TX callback (idx %d)\n",
		       local->dev->name, cb_idx);
		return;
	}

	fc = __le16_to_cpu(*((u16 *)&(tx_frame->Hdr.FrmCtl)));
	hdrlen = mvWLAN_80211_get_hdrlen(fc);
	len = le16_to_cpu(tx_frame->Hdr.FrmBodyLen);

	skb = dev_alloc_skb(hdrlen + len);
	if (skb == NULL) {
		printk(KERN_DEBUG "%s: mvWLAN_tx_callback failed to allocate "
		       "skb\n", local->dev->name);
		return;
	}

	/* Duplicate the frame that was sent */
	memcpy(skb_put(skb, hdrlen), (void *)&(tx_frame->Hdr.FrmCtl), hdrlen);
	memcpy(skb_put(skb, len), (void *) &tx_frame->Body, len);

	skb->dev = local->dev;
	skb->mac.raw = skb->data;

	cb->func(skb, ok, cb->data);
}


u16 mvWLAN_tx_callback_register(local_info_t *local,
				void (*func)(struct sk_buff *, int ok, void *),
				void *data)
{
	unsigned long flags;
	struct mvwlan_tx_callback_info *entry;

	entry = (struct mvwlan_tx_callback_info *) kmalloc(sizeof(*entry),
							   GFP_ATOMIC);
	if (entry == NULL)
		return 0;

	entry->func = func;
	entry->data = data;

	spin_lock_irqsave(&local->cblock, flags);
	entry->idx = local->tx_callback ? local->tx_callback->idx + 1 : 1;
	entry->next = local->tx_callback;
	local->tx_callback = entry;
	spin_unlock_irqrestore(&local->cblock, flags);

	return entry->idx;
}


int mvWLAN_tx_callback_unregister(local_info_t *local, u16 idx)
{
	unsigned long flags;
	struct mvwlan_tx_callback_info *cb, *prev = NULL;

	spin_lock_irqsave(&local->cblock, flags);
	cb = local->tx_callback;
	while (cb != NULL && cb->idx != idx) {
		prev = cb;
		cb = cb->next;
	}
	if (cb) {
		if (prev == NULL)
			local->tx_callback = cb->next;
		else
			prev->next = cb->next;
		kfree(cb);
	}
	spin_unlock_irqrestore(&local->cblock, flags);

	return cb ? 0 : -1;
}

extern int txQueueNumber;
struct mvwlan_tx_queue * mvWLAN_tx_queue_alloc(local_info_t *local)
{
	struct mvwlan_tx_queue *tx_queue = NULL;

	spin_lock_bh(&local->txqlock);
    //printk("%d ", txQueueNumber);
    if (list_empty(&local->tx_queue_list)) {
		/* no empty TX buffers, stop queue */
		mvWLAN_netif_stop_queues(local);
        spin_unlock_bh(&local->txqlock);
        return NULL;
    }

	list_for_each_entry(tx_queue, &local->tx_queue_list, list) {
#if 0     
		//list_del(&tx_queue->list);
		if (list_empty(&local->tx_queue_list)) {
			/* no empty TX buffers, stop queue */
			mvWLAN_netif_stop_queues(local);
            return NULL;
		}
#endif        
        list_del(&tx_queue->list);
		break;
	}
    //printk("%d, ", txQueueNumber);
    txQueueNumber--;

	spin_unlock_bh(&local->txqlock);
#if 0
	if (tx_queue == ((struct mvwlan_tx_queue *) &local->tx_queue_list)) {
		tx_queue = NULL;
	}
#endif
	if (tx_queue) {
		tx_queue->flags |= MVWLAN_TX_QUEUE_BIT_USED;
		tx_queue->cb_index = 0;
		memset(&(tx_queue->tx_desc), 0, sizeof(WLAN_TX_INFO));
		tx_queue->tx_desc.StnId = 0;
		tx_queue->tx_desc.TxParam = 0;
#ifdef TRUBO_SETUP
		if (local->sysConfig->Mib802dot11->StationConfig.mib_TSMode == AUTOLINK_START)
		{
			tx_queue->tx_desc.RateIdx = AUTOLINK_RATE;
		}
		else
#else
		{
			tx_queue->tx_desc.RateIdx = 1;
		}
#endif
		tx_queue->tx_desc.Preample = WLAN_TX_PREAMPLE_LONG;
		/* Our IEEE frame starts with frame body length */
		tx_queue->tx_frame = (IEEEtypes_Frame_t *)
				     ((u32)&tx_queue->frame_body_length);
	}

	return tx_queue;
}


void mvWLAN_tx_queue_free(local_info_t *local, WLAN_TX_FRAME *wlanFrame,
			  int status)
{
	struct mvwlan_tx_queue *tx_queue = NULL;

	spin_lock_bh(&local->txqlock);

	/* Since the virtual memory address is the same with the physical
	 * memory address, we use the simplest way to get the actual
	 * pointer to the TX queue.
	 */
	tx_queue = list_entry(wlanFrame, struct mvwlan_tx_queue, tx_desc);
	if (tx_queue->cb_index) {
		mvWLAN_tx_callback(local,
				   (IEEEtypes_Frame_t *)((u32)
					 &tx_queue->frame_body_length),
				   tx_queue->cb_index, status);
	}
	tx_queue->flags &= ~MVWLAN_TX_QUEUE_BIT_USED;
	if (list_empty(&local->tx_queue_list)) {
		/* Wake it up now since there is at least one TX buffer
		 * has been returned.
		 */
		mvWLAN_netif_wake_queues(local);
	}
	list_add_tail(&tx_queue->list, &local->tx_queue_list);
    txQueueNumber++;
	spin_unlock_bh(&local->txqlock);
}


static int fragmentNeeded(IEEEtypes_Frame_t *tx_frame, int len,
			  int fragmentThreshold)
{
	u8 *addr1;
	int fragmentCount = 0;
	int header_len, data_frag_threshold;

	header_len = mvWLAN_80211_get_hdrlen(*((u16 *) &tx_frame->Hdr.FrmCtl));
	header_len += IEEE80211_FCS_SIZE;
	data_frag_threshold = fragmentThreshold - header_len;

	/* IEEE 802.11-1999 (R2003) clause 9.1.4 claims we should not
	 * fragment the broadcast/multicast frames.
	 */
	addr1 = (u8 *)&(tx_frame->Hdr.Addr1);
	if ((addr1[0] & 0x01) != 0x01) {
		while (len > data_frag_threshold) {
			fragmentCount++;
			len -= data_frag_threshold;
		}
	}

	return fragmentCount;
}


/* All fragmented frames share the same TX queue. Each fragmented frame
 * follows one by one in the order of its framgment number and has its
 * own frame header of 32 bytes (4-addresses format plus the frame body
 * length which is 2 bytes). The starting address of each fragmented frame
 * should be 4-bytes aligned. The length for each fragmented frame should
 * also includes the overhead for the encryption (if any).
 */
static int doFragmentation(u8 *data, int len, int fragmentThreshold,
			   struct mvwlan_crypt_data *crypt,
			   struct mvwlan_tx_queue *tx_queue, int broadcast)
{
	int fragment_id = 0;
	u8 datarate;
	u8 ackdur;
	u8 preAmble;
	u8 *tx_frame_ptr;
	IEEEtypes_Frame_t *tx_frame_original;
	int header_len, data_frag_threshold;

	tx_frame_original = tx_queue->tx_frame;

	header_len = mvWLAN_80211_get_hdrlen(*((u16 *) &tx_frame_original->Hdr.FrmCtl));
	header_len += IEEE80211_FCS_SIZE;
	data_frag_threshold = fragmentThreshold - header_len;

	tx_frame_ptr = (u8 *)((u32) tx_frame_original);

	datarate = GetDataRate(1);
	ackdur = GetCtsAckDur(1);
	preAmble = IEEE80211_LONG_PREAM;

	while (len > 0) {
		IEEEtypes_Frame_t *tx_frame;
		int data_len;
		u8 *src, *dst;

		if (len > data_frag_threshold)
			data_len = data_frag_threshold;
		else
			data_len = len;

		/* Prepare the frame pointer */
		tx_frame = (IEEEtypes_Frame_t *) tx_frame_ptr;

		/* Prepare the frame payload */
		if (crypt) {
			/* Perform host driver -based encryption */
			u8 *pos, *opos;
			int olen;

			olen = data_len;

			pos = (u8 *) tx_frame;
			opos = data;

			atomic_inc(&crypt->refcnt);
			olen = crypt->ops->encrypt_mpdu(pos, opos, olen,
						   crypt->priv, broadcast);
			atomic_dec(&crypt->refcnt);

			if (olen < 0)
				break;

			data_len = olen;
		} else {
			/* Copy the rest of the frame directly */
			memcpy(tx_frame->Body, data, data_len);
		}

		/* Prepare the frame eader */
		tx_frame->Hdr.DurationId = ackdur + preAmble + IEEE80211_SIFS;

		/* Update the frame body length */
		tx_frame->Hdr.FrmBodyLen = __cpu_to_le16(data_len);

		/* The first fragmented frame already has correct IEEE
		 * 802.11 frame header; however, the others do not have
		 * any information in their frame header so we simply
		 * copy from the first one.
		 */
		if (fragment_id) {
			dst = (u8 *)((u32)&(tx_frame->Hdr.FrmCtl));
			src = (u8 *)((u32)&(tx_frame_original->Hdr.FrmCtl));
			memcpy(dst, src, sizeof(IEEEtypes_GenHdr_t) -
			       sizeof(UINT16));
		}
		tx_frame->Hdr.SeqCtl = fragment_id++;

		if (len <= data_frag_threshold) {
			/* Remove MOREFRAG flag since it is the last one */
			tx_frame->Hdr.FrmCtl.MoreFrag = 0;
			len = 0;
		} else {
			int next_frame_len;

			len -= data_frag_threshold;
			data += data_frag_threshold;
			tx_frame_ptr += (data_len + FRAGMENT_HEADER_SIZE);
			/* Make the pointer 4-bytes alignment */
			if (((u32) tx_frame_ptr) & 0x3) {
				tx_frame_ptr = (u8 *)
					       (((u32)(tx_frame_ptr + 4)) &
						0xFFFFFFFC);
			}
			next_frame_len = (len > data_frag_threshold) ?
					 data_frag_threshold : len;
			/* Add MOREFRAG flag to indicate there are still
			 * fragmented frames to come.
			 */
			tx_frame->Hdr.FrmCtl.MoreFrag = 1;
			tx_frame->Hdr.DurationId += (next_frame_len * 80 /
						     datarate + ackdur +
						     IEEE80211_SIFS * 2 +
						     preAmble * 2);
		}
	}

	if (len)
		return -1;

	return 0;
}


/* Called only from device's hard_start_xmit method */
int mvWLAN_tx(struct sk_buff *skb, struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int tx_done = 0;
	int data_len;
	u16 fc;
	u16 ethertype = 0;
	mvwlan_wds_info_t *wds = NULL;
	enum { WDS_NO, WDS_COMPLIANT_FRAME } use_wds = WDS_NO;
	struct net_device_stats *stats;
	int host_encrypt = 0;
	int broadcast = 0;
	struct mvwlan_crypt_data *crypt = NULL;
	void *sta = NULL;
	u8 *encaps_data;
	u8 eth_da[ETH_ALEN], eth_sa[ETH_ALEN];
	int encaps_len;
	int no_encrypt = 0;
	int to_assoc_ap = 0;
	int rate_idx = 0;
	int header_len = 0;
	struct mvwlan_tx_queue *tx_queue;
	IEEEtypes_Frame_t *tx_frame;
	WLAN_TX_FRAME *wlan_frame = NULL;
	MIB_802DOT11 *mib_p = local->sysConfig->Mib802dot11;
	u8 save_data[12], *save_p = NULL, save_len = 0;


	mvWLAN_callback(local, MVWLAN_CALLBACK_TX_START);
	stats = mvWLAN_get_stats(dev);

	if (!local->hw_ready) {
		printk(KERN_DEBUG "%s: mvWLAN_tx: hw not ready - skipping\n",
		       dev->name);
		goto tx_fail;
	}

	if (skb->len < ETH_HLEN) {
		printk(KERN_DEBUG "%s: mvWLAN_tx: short skb (len=%d)\n",
		       dev->name, skb->len);
		goto tx_fail;
	}

	if (local->dev != dev) {
		if (dev == local->stadev) {
			to_assoc_ap = 1;
			wds = NULL;
			use_wds = WDS_NO;
		} else {
			wds = (mvwlan_wds_info_t *) dev;
			use_wds = WDS_COMPLIANT_FRAME;
		}
	} else {
		if (local->iw_mode == IW_MODE_REPEAT) {
			printk(KERN_DEBUG "%s: mvWLAN_tx: trying to use "
			       "non-WDS link in Repeater mode\n", dev->name);
			goto tx_fail;
		} else if (local->iw_mode == IW_MODE_INFRA &&
			   (local->wds_type & MVWLAN_WDS_AP_CLIENT) &&
			   memcmp(skb->data + ETH_ALEN, dev->dev_addr,
				  ETH_ALEN) != 0) {
			/* AP client mode: send frames with foreign src addr
			 * using 4-addr WDS frames */
			use_wds = WDS_COMPLIANT_FRAME;
		}
	}

	tx_queue = mvWLAN_tx_queue_alloc(local);
	if (! tx_queue) {
		printk(KERN_DEBUG "%s: mvWLAN_tx: out of tx queue\n",
		       dev->name);
		stats->tx_dropped++;
		goto tx_fail;
	}

	tx_frame = tx_queue->tx_frame;
	wlan_frame = (WLAN_TX_FRAME *) virt_to_phys((u32)&tx_queue->tx_desc);

 	/* Incoming skb->data: dst_addr[6], src_addr[6], proto[2], payload
	 * ==>
	 * TX frame with 802.11 header:
	 * Address order depending on used mode (includes dst_addr and
	 * src_addr), possible encapsulation (RFC1042/Bridge-Tunnel;
	 * proto[2], payload */
	memcpy(eth_da, skb->data, ETH_ALEN);
	memcpy(eth_sa, &(skb->data[ETH_ALEN]), ETH_ALEN);
	ethertype = (skb->data[2*ETH_ALEN] << 8) | skb->data[2*ETH_ALEN+1];
	header_len = 2 * ETH_ALEN;

	/* Length of data after Ethernet header */
	data_len = skb->len - (ETH_HLEN - 2);
	if (ethertype == ETH_P_AARP || ethertype == ETH_P_IPX) {
		encaps_data = bridge_tunnel_header;
		encaps_len = 6;
		data_len += encaps_len;
	} else if (ethertype >= 0x600) {
		/* Ethernet II frames */
		encaps_data = rfc1042_header;
		encaps_len = 6;
		data_len += encaps_len;
	} else {
		/* LLC frames */
		encaps_data = NULL;
		encaps_len = 0;
		data_len -= 2;
		header_len += 2;
	}

	fc = (WLAN_FC_TYPE_DATA << 2) | (WLAN_FC_STYPE_DATA << 4);

	if (use_wds != WDS_NO) {
		/* Send real 802.11 frames with four addresses needed for
		 * WDS */
		fc |= WLAN_FC_FROMDS | WLAN_FC_TODS;

		/* FromDS & ToDS: Addr1 = RA, Addr2 = TA, Addr3 = DA,
		 * Addr4 = SA */
		memcpy(&tx_frame->Hdr.Addr4, eth_sa, ETH_ALEN);

		/* Send broadcast and multicast frames to broadcast RA, if
		 * configured; otherwise, use unicast RA of the WDS link */
		if ((local->wds_type & MVWLAN_WDS_BROADCAST_RA) &&
		    (skb->data[0] & 0x01))
			memset(&tx_frame->Hdr.Addr1, 0xff, ETH_ALEN);
		else if (wds)
			memcpy(&tx_frame->Hdr.Addr1, wds->remote_addr, ETH_ALEN);
		else
			memcpy(&tx_frame->Hdr.Addr1, local->bssid, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr2, dev->dev_addr, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr3, eth_da, ETH_ALEN);
	} else if (local->iw_mode == IW_MODE_MASTER && !to_assoc_ap) {
		fc |= WLAN_FC_FROMDS;

		/* From DS: Addr1 = DA, Addr2 = BSSID, Addr3 = SA */
		memcpy(&tx_frame->Hdr.Addr1, eth_da, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr2, dev->dev_addr, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr3, eth_sa, ETH_ALEN);
	} else if (local->iw_mode == IW_MODE_INFRA || to_assoc_ap) {
		fc |= WLAN_FC_TODS;

		/* To DS: Addr1 = BSSID, Addr2 = SA, Addr3 = DA */
		memcpy(&tx_frame->Hdr.Addr1, to_assoc_ap ?
		       local->assoc_ap_addr : local->bssid, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr2, eth_sa, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr3, eth_da, ETH_ALEN);
	} else if (local->iw_mode == IW_MODE_ADHOC) {
		/* not From/To DS: Addr1 = DA, Addr2 = SA, Addr3 = BSSID */
		memcpy(&tx_frame->Hdr.Addr1, eth_da, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr2, eth_sa, ETH_ALEN);
		memcpy(&tx_frame->Hdr.Addr3, local->bssid, ETH_ALEN);
	} else {
		/* XXX, anything else? */
	}

	*((u16 *)&(tx_frame->Hdr.FrmCtl)) = __cpu_to_le16(fc);

	if (to_assoc_ap) {
		/* Client mode should set its rate index here */
		goto skip_ap_processing;
	}

	switch (mvWLAN_handle_sta_tx(local, skb, tx_frame, use_wds != WDS_NO,
				     &sta, &rate_idx, &broadcast)) {
	case AP_TX_CONTINUE:
		break;
	case AP_TX_CONTINUE_NOT_AUTHORIZED:
		if ((local->ieee_802_1x || local->sysConfig->Mib802dot11->Privacy.RSNEnabled) &&
		    ethertype != ETH_P_PAE &&
		    use_wds == WDS_NO) {
			printk(KERN_DEBUG "%s: dropped frame to unauthorized "
			       "port (IEEE 802.1X): ethertype=0x%04x\n",
			       dev->name, ethertype);
			mvWLAN_dump_tx_header(dev->name, NULL, tx_frame);

			stats->tx_dropped++;
			goto tx_fail;
		}
		break;
	case AP_TX_DROP:
		stats->tx_dropped++;
		goto tx_fail;
	case AP_TX_RETRY:
		goto tx_fail;
	case AP_TX_BUFFERED:
		/* do not free skb here, it will be freed when the
		 * buffered frame is sent/timed out */
		goto tx_exit;
	}

 skip_ap_processing:

	/* Actual padload starts from bytes that are left after the
	 * DA/SA pair or the 802.11 frame header.
	 */
	skb->data += header_len;

	/* Copy the encapsulation data if any */
	if (encaps_data && encaps_len) {

		skb->data -= encaps_len;

		if (atomic_read(&(skb_shinfo(skb)->dataref)) > 1) {
			/* save modified data */
			save_p = skb->data;
			save_len = encaps_len;
			memcpy(save_data, save_p, save_len);
		}

		memcpy(skb->data, encaps_data, encaps_len);
	}

	tx_queue->tx_desc.RateIdx = rate_idx;

#ifdef AP_WPA2
	if (local->sysConfig->Mib802dot11->Privacy.RSNEnabled) {

		if (broadcast) {

			local->crypt = local->multicast_crypt;
		} else {
			struct sta_info *sta_p = (struct sta_info *) sta;

			if (sta_p != NULL) {
				if ((sta_p->key_mgmt_info.RsnIEBuf[0] == 48 && sta_p->key_mgmt_info.RsnIEBuf[13] == 4) ||
					(sta_p->key_mgmt_info.RsnIEBuf[0] == 221 && sta_p->key_mgmt_info.RsnIEBuf[17] == 4))
					local->crypt = local->ccmp_crypt;
				else
					local->crypt = local->tkip_crypt;
			}
		}
	}
#endif

	if (local->host_encrypt) {
		/* Set crypt to default algorithm and key */
		crypt = local->crypt;
		host_encrypt = 1;
	}

	if ((local->ieee_802_1x || local->sysConfig->Mib802dot11->Privacy.RSNEnabled)
		&& ethertype == ETH_P_PAE) {
		if (crypt) {
			no_encrypt = 1;
			printk(KERN_DEBUG "%s: TX: IEEE 802.1X - passing "
			       "unencrypted EAPOL frame\n", dev->name);
		}
		crypt = NULL; /* no encryption for IEEE 802.1X frames */
	}

	if (crypt && (!crypt->ops || !crypt->ops->encrypt_mpdu))
		crypt = NULL;
	else if ((crypt || local->crypt) && !no_encrypt) {
		/* Add ISWEP flag both for firmware and host based encryption
		 */
		tx_frame->Hdr.FrmCtl.Wep = 1;
	}

	if (crypt) {

		if (local->sysConfig->Mib802dot11->Privacy.RSNEnabled)
			crypt->priv = broadcast ? (void *) local : sta;

		if (crypt->ops->encrypt_msdu) {
			atomic_inc(&crypt->refcnt);
			data_len = crypt->ops->encrypt_msdu(eth_da, eth_sa, skb->data, data_len, crypt->priv, broadcast);
			atomic_dec(&crypt->refcnt);
			if (data_len < 0)
				goto tx_fail;
		}
	}

	if (fragmentNeeded(tx_frame, data_len,
			   mib_p->OperationTable.FragThresh)) {
		if (doFragmentation(skb->data, data_len,
				    mib_p->OperationTable.FragThresh,
				    crypt, tx_queue, broadcast)) {
			stats->tx_errors++;
			goto tx_fail;
		}
	} else {
		if (crypt) {
			/* Perform host driver -based encryption */
			u8 *pos, *opos;
			int olen;

			olen = data_len;

			pos = (u8 *) tx_frame;
			opos = skb->data;

			atomic_inc(&crypt->refcnt);
			olen = crypt->ops->encrypt_mpdu(pos, opos, olen,
						   crypt->priv, broadcast);
			atomic_dec(&crypt->refcnt);
			if (olen > MVWLAN_MAX_TX_FRAME_SIZE) {
				printk(KERN_WARNING "%s: encrypt "
				       "overwrote frame buffer "
				       "(%d > %d)\n", dev->name, olen,
				       MVWLAN_MAX_TX_FRAME_SIZE);
			}
			if (olen < 0)
				goto tx_fail;

			data_len = olen;
		} else {
			/* Copy the rest of the frame directly */
			memcpy(tx_frame->Body, skb->data, data_len);
		}

		tx_frame->Hdr.FrmBodyLen = __cpu_to_le16(data_len);
	}

	if (local->frame_dump & MVWLAN_DUMP_TX_HDR)
		mvWLAN_dump_tx_header(dev->name, NULL, tx_frame);

	wlan_frame->cookie = (UINT32) sta;

	if (mvWLAN_transmit(dev, wlan_frame)) {
		stats->tx_errors++;
		goto tx_fail;
	}

	stats->tx_packets++;
	stats->tx_bytes += data_len + mvWLAN_80211_get_hdrlen(fc);

	tx_done = 1;

 tx_fail:

	if (save_p != NULL)
		memcpy(save_p, save_data, save_len);

	dev_kfree_skb(skb);

 tx_exit:

	/* free the TX queue due to failure */
	if (!tx_done && wlan_frame) {
		mvWLAN_tx_queue_free(local, wlan_frame, 0);
	}

	if (sta)
		mvWLAN_handle_sta_release(sta);

	mvWLAN_callback(local, MVWLAN_CALLBACK_TX_END);

	return 0;
}


/* Called only from device's hard_start_xmit method */
int mvWLAN_tx_80211(struct sk_buff *skb, struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct mvwlan_tx_queue *tx_queue;
	IEEEtypes_Frame_t *tx_frame;
	struct mvwlan_ieee80211_hdr *old_frame_hdr;
	WLAN_TX_FRAME *wlan_frame = NULL;
	EAPOL_KeyMsg_t *tx_eapol_ptr;
	u16 eapol_key_info;
	void *sta = NULL;
	int rate_idx = 0;
	int tx_done = 0;
	int header_len, data_len;
	u16 fc;

	if (!local->hw_ready) {
		printk(KERN_DEBUG "%s: mvWLAN_tx_80211: hw not ready - "
		       "skipping\n", dev->name);
		local->apdevstats.tx_dropped++;
		goto tx_80211_fail;
	}

	if (skb->len < IEEE80211_MGMT_HDR_LEN) {
		printk(KERN_DEBUG "%s: mvWLAN_tx_80211: short skb (len=%d)\n",
		       dev->name, skb->len);
		local->apdevstats.tx_dropped++;
		goto tx_80211_fail;
	}

	tx_queue = mvWLAN_tx_queue_alloc(local);
	if (! tx_queue) {
		printk(KERN_DEBUG "%s: mvWLAN_tx_80211: out of tx queue\n",
		       dev->name);
		local->apdevstats.tx_dropped++;
		goto tx_80211_fail;
	}

	/* Our IEEE frame starts with frame body length */
	tx_frame = tx_queue->tx_frame;
	wlan_frame = (WLAN_TX_FRAME *) virt_to_phys((u32)&tx_queue->tx_desc);

	/* skb->data starts with 802.11 frame control */
	old_frame_hdr = (struct mvwlan_ieee80211_hdr *) skb->data;
 	fc = old_frame_hdr->frame_control;

	header_len = mvWLAN_80211_get_hdrlen(fc);
	data_len = skb->len - header_len;

	/* Copy the frame header */
	memcpy((u8 *)&(tx_frame->Hdr.FrmCtl), skb->data, header_len);

	/* Request TX callback if protocol version is 2 in 802.11 header;
	 * this version 2 is a special case used between hostapd and kernel
	 * driver */
	if (((fc & WLAN_FC_PVER) == BIT(1)) &&
	    local->ap && local->ap->tx_callback_idx) {
		tx_queue->cb_index = local->ap->tx_callback_idx;

		/* remove special version from the frame header */
		fc &= ~WLAN_FC_PVER;
		*((u16 *)&(tx_frame->Hdr.FrmCtl)) = __cpu_to_le16(fc);
	}

	/* We do not need to care about frame authorization etc. here since
	 * hostapd has full knowledge of auth/assoc status. However, we need to
	 * buffer the frame is the destination STA is in power saving mode.
	 *
	 * Wi-Fi 802.11b test plan suggests that AP should ignore power save
	 * bit in authentication and (re)association frames and assume tha
	 * STA remains awake for the response. */

	if ((mvWLAN_handle_sta_tx(local, skb, tx_frame, 0, &sta, &rate_idx, NULL) == AP_TX_BUFFERED) &&
	    (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT ||
	     (WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_AUTH &&
	      WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_ASSOC_RESP &&
	      WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_REASSOC_RESP))) {
		/* do not free skb here, it will be freed when the
		 * buffered frame is sent/timed out */
		goto tx_80211_exit;
	}

	if ((WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_DATA) && (data_len != 0)) {

		tx_queue->tx_desc.RateIdx = rate_idx;

		{
		u8 *snap_oui_p;
		u8 MIC[EAPOL_MIC_SIZE + 4];

		snap_oui_p = skb->data+header_len+3;

		if ((snap_oui_p[0] == 0x00) && (snap_oui_p[1] == 0x00) &&
			(snap_oui_p[2] == 0x00) && (snap_oui_p[3] == 0x88) &&
			(snap_oui_p[4] == 0x8E)) {

			/* 802.11 header + LLC encapsulation(6) + type field(2) + 802.1x header(4) */
			tx_eapol_ptr = ((EAPOL_KeyMsg_t *) (skb->data+header_len+12));

			/* data come here must be 802.1x data, we will not do check for that */
			eapol_key_info = tx_eapol_ptr->k.key_info16;

			if ((eapol_key_info & 0x0800) == 0) { /* EAPOL Key - Group key */

				tx_eapol_ptr->key_RSC[0] = mib_MrvlRSN_GrpKey.g_IV16 & 0x00FF;
				tx_eapol_ptr->key_RSC[1] = (mib_MrvlRSN_GrpKey.g_IV16 >> 8) & 0x00FF;
				memcpy(tx_eapol_ptr->key_RSC + 2, &mib_MrvlRSN_GrpKey.g_IV32, 4);

#ifdef AP_WPA2
				ComputeEAPOL_MIC(skb->data+header_len+8, data_len-8,
					((struct sta_info *)sta)->key_mgmt_info.EAPOL_MIC_Key, EAPOL_MIC_KEY_SIZE,
					MIC, ((struct sta_info *)sta)->key_mgmt_info.RsnIEBuf);
#else
				ComputeEAPOL_MIC(skb->data+header_len+8, data_len-8,
					((struct sta_info *)sta)->key_mgmt_info.EAPOL_MIC_Key, EAPOL_MIC_KEY_SIZE,
					MIC, local->sysConfig->Mib802dot11->UnicastCiphers.UnicastCipher[3]);
#endif

				apppendEAPOL_MIC((UINT8*) &(tx_eapol_ptr->key_MIC), MIC);
			}

		}

		}

		if (sta && local->sysConfig->Mib802dot11->Privacy.RSNEnabled) {

			if (local->crypt && (((struct sta_info *)sta)->flags & WLAN_STA_AUTHORIZED)) {
				u8 *data_p = (skb->data+header_len);
				struct sta_info *sta_p = (struct sta_info *) sta;

				/* Add ISWEP flag both for firmware and host based encryption
			 	 */
				fc |= WLAN_FC_ISWEP;
				*((u16 *)&(tx_frame->Hdr.FrmCtl)) = __cpu_to_le16(fc);

#ifdef AP_WPA2
				if ((sta_p->key_mgmt_info.RsnIEBuf[0] == 48 && sta_p->key_mgmt_info.RsnIEBuf[13] == 4) ||
					(sta_p->key_mgmt_info.RsnIEBuf[0] == 221 && sta_p->key_mgmt_info.RsnIEBuf[17] == 4))
					local->crypt = local->ccmp_crypt;
				else
					local->crypt = local->tkip_crypt;
#endif

				local->crypt->priv = sta;

				if ((((u32) data_p) & 0x3) != 0) { /* check 4 byte alignment */

					((u32) data_p) &= ~0x3;
					memcpy(data_p, skb->data+header_len, data_len);
				}

				if (local->crypt->ops->encrypt_msdu) {

					atomic_inc(&local->crypt->refcnt);
					data_len = local->crypt->ops->encrypt_msdu(tx_frame->Hdr.Addr1, tx_frame->Hdr.Addr3,
						data_p, data_len, local->crypt->priv, FALSE);
					atomic_dec(&local->crypt->refcnt);

					if (data_len < 0)
						goto tx_80211_fail;
				}

				if (local->crypt->ops->encrypt_mpdu) {
					u8 *pos, *opos;
					int olen;

					olen = data_len;
					pos = (u8 *) tx_frame;
					opos = data_p;

					atomic_inc(&local->crypt->refcnt);
					olen = local->crypt->ops->encrypt_mpdu(pos, opos, olen, local->crypt->priv, FALSE);
					atomic_dec(&local->crypt->refcnt);

					if (olen > MVWLAN_MAX_TX_FRAME_SIZE) {
						printk(KERN_WARNING "%s: encrypt "
							"overwrote frame buffer "
							"(%d > %d)\n", dev->name, olen,
							MVWLAN_MAX_TX_FRAME_SIZE);
					}

					if (olen < 0)
						goto tx_80211_fail;

					data_len = olen;
				}

				goto encry_done;
			}
		}
	}

	/* Copy the rest of the frame */
	memcpy(tx_frame->Body, skb->data+header_len, data_len);

encry_done:

	tx_frame->Hdr.FrmBodyLen = __cpu_to_le16(data_len);

	if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_DATA)
		wlan_frame->cookie = (UINT32) sta;
	else
		wlan_frame->cookie = (UINT32) NULL;

	if (mvWLAN_transmit(dev, wlan_frame)) {
		local->apdevstats.tx_errors++;
		goto tx_80211_fail;
	}

	tx_done = 1;

	local->apdevstats.tx_packets++;
	local->apdevstats.tx_bytes += skb->len;

 tx_80211_fail:

	dev_kfree_skb(skb);

	/* free the TX queue due to failure */
	if (!tx_done && wlan_frame)
		mvWLAN_tx_queue_free(local, wlan_frame, 0);

 tx_80211_exit:

	if (sta)
		mvWLAN_handle_sta_release(sta);

	return 0;
}


/* Called from timer handler and from scheduled AP queue handlers */
void mvWLAN_send_mgmt(struct net_device *dev, int type, int subtype,
		      char *body, int body_len, int txevent, u8 *addr,
		      u16 tx_cb_idx)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct mvwlan_tx_queue *tx_queue;
	IEEEtypes_Frame_t *tx_frame;
	WLAN_TX_FRAME *wlan_frame;
	u16 fc;
	struct net_device_stats *stats;

	stats = mvWLAN_get_stats(dev);

	if (!(dev->flags & IFF_UP)) {
		PDEBUG(DEBUG_AP, "%s: mvWLAN_send_mgmt - device is not UP - "
		       "cannot send frame\n", dev->name);
		stats->tx_errors++;
		return;
	}

	tx_queue = mvWLAN_tx_queue_alloc(local);
	
	if (!tx_queue) {
		printk(KERN_DEBUG "%s: mvWLAN_send_mgmt: out of tx queue\n",
		       dev->name);
		stats->tx_dropped++;
		return;
	}

	tx_frame = (IEEEtypes_Frame_t *)((u32)&tx_queue->frame_body_length);
	wlan_frame = (WLAN_TX_FRAME *) virt_to_phys((u32)&tx_queue->tx_desc);
	wlan_frame->	cookie = (UINT32) local;

	fc = (type << 2) | (subtype << 4);

	memcpy(tx_frame->Hdr.Addr1, addr, ETH_ALEN); /* DA / RA */
	if (type == WLAN_FC_TYPE_DATA) {
		fc |= WLAN_FC_FROMDS;
		memcpy(tx_frame->Hdr.Addr2, dev->dev_addr, ETH_ALEN); /* BSSID */
		memcpy(tx_frame->Hdr.Addr3, dev->dev_addr, ETH_ALEN); /* SA */
	} else if (type == WLAN_FC_TYPE_CTRL) {
		/* control:ACK does not have addr2 or addr3 */
		memset(tx_frame->Hdr.Addr2, 0, ETH_ALEN);
		memset(tx_frame->Hdr.Addr3, 0, ETH_ALEN);
	} else {
		memcpy(tx_frame->Hdr.Addr2, dev->dev_addr, ETH_ALEN); /* SA */
		memcpy(tx_frame->Hdr.Addr3, dev->dev_addr, ETH_ALEN); /* BSSID */
	}

	*((u16 *)&(tx_frame->Hdr.FrmCtl)) = cpu_to_le16(fc);
	tx_frame->Hdr.FrmBodyLen = cpu_to_le16(body_len);

	if (body)
		memcpy(tx_frame->Body, body, body_len);

	/* We do not need to care about frame authorization etc. here since
	 * hostapd has full knowledge of auth/assoc status. However, we need to
	 * buffer the frame is the destination STA is in power saving mode.
	 *
	 * Wi-Fi 802.11b test plan suggests that AP should ignore power save
	 * bit in authentication and (re)association frames and assume tha
	 * STA remains awake for the response. */

	if ((mvWLAN_handle_sta_tx(local, NULL, tx_frame, 0, NULL, NULL,
				  NULL) == AP_TX_BUFFERED) &&
	    (WLAN_FC_GET_TYPE(fc) != WLAN_FC_TYPE_MGMT ||
	     (WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_AUTH &&
	      WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_ASSOC_RESP &&
	      WLAN_FC_GET_STYPE(fc) != WLAN_FC_STYPE_REASSOC_RESP))) {
		/* do not free skb here, it will be freed when the
		 * buffered frame is sent/timed out */
		return;
	}

	tx_queue->cb_index = tx_cb_idx;

	if (mvWLAN_transmit(dev, wlan_frame)) {
		stats->tx_errors++;
		/* free the TX queue due to hardware failure */
		mvWLAN_tx_queue_free(local, wlan_frame, 0);
	} else {
		stats->tx_packets++;
		stats->tx_bytes += (mvWLAN_80211_get_hdrlen(fc) + body_len);
	}
}


EXPORT_SYMBOL(mvWLAN_tx_callback_register);
EXPORT_SYMBOL(mvWLAN_tx_callback_unregister);

