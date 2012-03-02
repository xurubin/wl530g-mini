/*
 * Host AP (software wireless LAN access point) driver for
 * Intersil Prism2/2.5/3.
 *
 * Copyright (c) 2001-2002, SSH Communications Security Corp and Jouni Malinen
 * <jkmaline@cc.hut.fi>
 * Copyright (c) 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 *
 * FIX:
 * - there is currently no way of associating TX packets to correct wds device
 *   when TX Exc/OK event occurs, so all tx_packets and some
 *   tx_errors/tx_dropped are added to the main netdevice; using sw_support
 *   field in txdesc might be used to fix this (using Alloc event to increment
 *   tx_packets would need some further info in txfid table)
 *
 * Buffer Access Path (BAP) usage:
 *   Prism2 cards have two separate BAPs for accessing the card memory. These
 *   should allow concurrent access to two different frames and the driver
 *   previously used BAP0 for sending data and BAP1 for receiving data.
 *   However, there seems to be number of issues with concurrent access and at
 *   least one know hardware bug in using BAP0 and BAP1 concurrently with PCI
 *   Prism2.5. Therefore, the driver now only uses BAP0 for moving data between
 *   host and card memories. BAP0 accesses are protected with local->baplock
 *   (spin_lock_bh) to prevent concurrent use.
 */


#include <linux/config.h>
#include <linux/version.h>

#include <asm/delay.h>
#include <asm/uaccess.h>

#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/proc_fs.h>
#include <linux/if_arp.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/wireless.h>
#if WIRELESS_EXT > 12
#include <net/iw_handler.h>
#endif /* WIRELESS_EXT > 12 */
#include <asm/irq.h>


#include "hostap.h"
#include "hostap_ap.h"


/* #define final_version */

static int mtu = 1500;
MODULE_PARM(mtu, "i");
MODULE_PARM_DESC(mtu, "Maximum transfer unit");

static int channel[MAX_PARM_DEVICES] = { 3, DEF_INTS };
MODULE_PARM(channel, PARM_MIN_MAX "i");
MODULE_PARM_DESC(channel, "Initial channel");

static char *essid[MAX_PARM_DEVICES] = { "test" };
MODULE_PARM(essid, PARM_MIN_MAX "s");
MODULE_PARM_DESC(essid, "Host AP's ESSID");

static int iw_mode[MAX_PARM_DEVICES] = { IW_MODE_MASTER, DEF_INTS };
MODULE_PARM(iw_mode, PARM_MIN_MAX "i");
MODULE_PARM_DESC(iw_mode, "Initial operation mode");

static int beacon_int[MAX_PARM_DEVICES] = { 100, DEF_INTS };
MODULE_PARM(beacon_int, PARM_MIN_MAX "i");
MODULE_PARM_DESC(beacon_int, "Beacon interval (1 = 1024 usec)");

static int dtim_period[MAX_PARM_DEVICES] = { 1, DEF_INTS };
MODULE_PARM(dtim_period, PARM_MIN_MAX "i");
MODULE_PARM_DESC(dtim_period, "DTIM period");

static int delayed_enable /* = 0 */;
MODULE_PARM(delayed_enable, "i");
MODULE_PARM_DESC(delayed_enable, "Delay MAC port enable until netdevice open");

static int disable_on_close /* = 0 */;
MODULE_PARM(disable_on_close, "i");
MODULE_PARM_DESC(disable_on_close, "Disable MAC port on netdevice close");

#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
static int bus_master_threshold_rx[MAX_PARM_DEVICES] = { 100, DEF_INTS };
MODULE_PARM(bus_master_threshold_rx, "i");
MODULE_PARM_DESC(bus_master_threshold_rx, "Packet length threshold for using "
		 "PCI bus master on RX");

static int bus_master_threshold_tx[MAX_PARM_DEVICES] = { 100, DEF_INTS };
MODULE_PARM(bus_master_threshold_tx, "i");
MODULE_PARM_DESC(bus_master_threshold_tx, "Packet length threshold for using "
		 "PCI bus master on TX");
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */


/* See IEEE 802.1H for LLC/SNAP encapsulation/decapsulation */
/* Ethernet-II snap header (RFC1042 for most EtherTypes) */
static unsigned char rfc1042_header[] =
{ 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
/* Bridge-Tunnel header (for EtherTypes ETH_P_AARP and ETH_P_IPX) */
static unsigned char bridge_tunnel_header[] =
{ 0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8 };
/* No encapsulation header if EtherType < 0x600 (=length) */


#ifdef final_version
#define EXTRA_EVENTS_WTERR 0
#else
/* check WTERR events (Wait Time-out) in development versions */
#define EXTRA_EVENTS_WTERR HFA384X_EV_WTERR
#endif

#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
#define EXTRA_EVENTS_BUS_MASTER (HFA384X_EV_PCI_M0 | HFA384X_EV_PCI_M1)
#else
#define EXTRA_EVENTS_BUS_MASTER 0
#endif

/* Events that will be using BAP0 */
#define HFA384X_BAP0_EVENTS \
	(HFA384X_EV_TXEXC | HFA384X_EV_RX | HFA384X_EV_INFO | HFA384X_EV_TX)

/* event mask, i.e., events that will result in an interrupt */
#define HFA384X_EVENT_MASK \
	(HFA384X_BAP0_EVENTS | HFA384X_EV_ALLOC | HFA384X_EV_INFDROP | \
	HFA384X_EV_CMD | HFA384X_EV_TICK | \
	EXTRA_EVENTS_WTERR | EXTRA_EVENTS_BUS_MASTER)

/* Default TX control flags: use 802.11 headers and request interrupt for
 * failed transmits. Frames that request ACK callback, will add
 * _TX_OK flag and _ALT_RTRY flag may be used to select different retry policy.
 */
#define HFA384X_TX_CTRL_FLAGS \
	(HFA384X_TX_CTRL_802_11 | HFA384X_TX_CTRL_TX_EX)


/* ca. 1 usec */
#define HFA384X_CMD_BUSY_TIMEOUT 5000

/* ca. 10 usec */
#define HFA384X_BAP_BUSY_TIMEOUT 5000
#define HFA384X_INIT_TIMEOUT 50000
#define HFA384X_CMD_COMPL_TIMEOUT 20000
#define HFA384X_DL_COMPL_TIMEOUT 1000000
#define HFA384X_ALLOC_COMPL_TIMEOUT 5000


static void prism2_hw_reset(struct net_device *dev);
static void prism2_check_sta_fw_version(local_info_t *local);

#ifdef PRISM2_DOWNLOAD_SUPPORT
/* hostap_download.c */
static u8 * prism2_read_pda(struct net_device *dev);
static int prism2_download(local_info_t *local,
			   struct prism2_download_param *param);
#endif /* PRISM2_DOWNLOAD_SUPPORT */




#ifndef final_version
/* magic value written to SWSUPPORT0 reg. for detecting whether card is still
 * present */
#define HFA384X_MAGIC 0x8A32
#endif


static u16 hfa384x_read_reg(struct net_device *dev, u16 reg)
{
	return HFA384X_INW(reg);
}


static void hfa384x_read_regs(struct net_device *dev,
			      struct hfa384x_regs *regs)
{
	regs->cmd = HFA384X_INW(HFA384X_CMD_OFF);
	regs->evstat = HFA384X_INW(HFA384X_EVSTAT_OFF);
	regs->offset0 = HFA384X_INW(HFA384X_OFFSET0_OFF);
	regs->offset1 = HFA384X_INW(HFA384X_OFFSET1_OFF);
	regs->swsupport0 = HFA384X_INW(HFA384X_SWSUPPORT0_OFF);
}


/* local->cmdlock must be locked when calling this helper function */
static inline void __hostap_cmd_queue_free(local_info_t *local,
					   struct hostap_cmd_queue *entry,
					   int del_req)
{
	if (del_req) {
		entry->del_req = 1;
		if (!list_empty(&entry->list)) {
			list_del_init(&entry->list);
			local->cmd_queue_len--;
		}
	}

	if (atomic_dec_and_test(&entry->usecnt) && entry->del_req)
		kfree(entry);
}

static inline void hostap_cmd_queue_free(local_info_t *local,
					 struct hostap_cmd_queue *entry,
					 int del_req)
{
	unsigned long flags;

	spin_lock_irqsave(&local->cmdlock, flags);
	__hostap_cmd_queue_free(local, entry, del_req);
	spin_unlock_irqrestore(&local->cmdlock, flags);
}


static inline int hfa384x_cmd_issue(struct net_device *dev,
				    struct hostap_cmd_queue *entry)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int tries;
	u16 reg;
	unsigned long flags;

	if (entry->issued) {
		printk(KERN_DEBUG "%s: driver bug - re-issuing command @%p\n",
		       dev->name, entry);
	}

	/* wait until busy bit is clear; this should always be clear since the
	 * commands are serialized */
	tries = HFA384X_CMD_BUSY_TIMEOUT;
	while (HFA384X_INW(HFA384X_CMD_OFF) & HFA384X_CMD_BUSY && tries > 0) {
		tries--;
		udelay(1);
	}
#ifndef final_version
	if (tries != HFA384X_CMD_BUSY_TIMEOUT) {
		prism2_io_debug_error(dev, 1);
		printk(KERN_DEBUG "%s: hfa384x_cmd_issue: cmd reg was busy "
		       "for %d usec\n", dev->name,
		       HFA384X_CMD_BUSY_TIMEOUT - tries);
	}
#endif
	if (tries == 0) {
		reg = HFA384X_INW(HFA384X_CMD_OFF);
		prism2_io_debug_error(dev, 2);
		printk(KERN_DEBUG "%s: hfa384x_cmd_issue - timeout - "
		       "reg=0x%04x\n", dev->name, reg);
		return -ETIMEDOUT;
	}

	/* write command */
	spin_lock_irqsave(&local->cmdlock, flags);
	HFA384X_OUTW(entry->param0, HFA384X_PARAM0_OFF);
	HFA384X_OUTW(entry->param1, HFA384X_PARAM1_OFF);
	HFA384X_OUTW(entry->cmd, HFA384X_CMD_OFF);
	entry->issued = 1;
	spin_unlock_irqrestore(&local->cmdlock, flags);

	return 0;
}


/* Issue given command (possibly after waiting in command queue) and sleep
 * until the command is completed (or timed out). This can be called only
 * from user context. */
static int hfa384x_cmd(struct net_device *dev, u16 cmd, u16 param0,
		       u16 *param1, u16 *resp0)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int err, res, issue, issued = 0;
	unsigned long flags;
	struct hostap_cmd_queue *entry;
	DECLARE_WAITQUEUE(wait, current);

	if (in_interrupt()) {
		printk(KERN_DEBUG "%s: hfa384x_cmd called from interrupt "
		       "context\n", dev->name);
		return -1;
	}

	if (local->cmd_queue_len >= HOSTAP_CMD_QUEUE_MAX_LEN) {
		printk(KERN_DEBUG "%s: hfa384x_cmd: cmd_queue full\n",
		       dev->name);
		return -1;
	}

	if (signal_pending(current))
		return -EINTR;

	entry = (struct hostap_cmd_queue *)
		kmalloc(sizeof(*entry), GFP_ATOMIC);
	if (entry == NULL) {
		printk(KERN_DEBUG "%s: hfa384x_cmd - kmalloc failed\n",
		       dev->name);
		return -ENOMEM;
	}
	memset(entry, 0, sizeof(*entry));
	atomic_set(&entry->usecnt, 1);
	entry->type = CMD_SLEEP;
	entry->cmd = cmd;
	entry->param0 = param0;
	if (param1)
		entry->param1 = *param1;
	init_waitqueue_head(&entry->compl);

	/* prepare to wait for command completion event, but do not sleep yet
	 */
	add_wait_queue(&entry->compl, &wait);
	set_current_state(TASK_INTERRUPTIBLE);

	spin_lock_irqsave(&local->cmdlock, flags);
	issue = list_empty(&local->cmd_queue);
	if (issue)
		entry->issuing = 1;
	list_add_tail(&entry->list, &local->cmd_queue);
	local->cmd_queue_len++;
	spin_unlock_irqrestore(&local->cmdlock, flags);

	err = 0;
	if (!issue)
		goto wait_completion;

	if (signal_pending(current))
		err = -EINTR;

	if (!err) {
		if (hfa384x_cmd_issue(dev, entry))
			err = -ETIMEDOUT;
		else
			issued = 1;
	}

 wait_completion:
	if (!err && entry->type != CMD_COMPLETED) {
		/* sleep until command is completed or timed out */
		res = schedule_timeout(2 * HZ);
	} else
		res = -1;

	if (!err && signal_pending(current))
		err = -EINTR;

	if (err && issued) {
		/* the command was issued, so a CmdCompl event should occur
		 * soon; however, there's a pending signal and
		 * schedule_timeout() would be interrupted; wait a short period
		 * of time to avoid removing entry from the list before
		 * CmdCompl event */
		udelay(300);
	}

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&entry->compl, &wait);

	/* If entry->list is still in the list, it must be removed
	 * first and in this case prism2_cmd_ev() does not yet have
	 * local reference to it, and the data can be kfree()'d
	 * here. If the command completion event is still generated,
	 * it will be assigned to next (possibly) pending command, but
	 * the driver will reset the card anyway due to timeout
	 *
	 * If the entry is not in the list prism2_cmd_ev() has a local
	 * reference to it, but keeps cmdlock as long as the data is
	 * needed, so the data can be kfree()'d here. */

	/* FIX: if the entry->list is in the list, it has not been completed
	 * yet, so removing it here is somewhat wrong.. this could cause
	 * references to freed memory and next list_del() causing NULL pointer
	 * dereference.. it would probably be better to leave the entry in the
	 * list and the list should be emptied during hw reset */

	spin_lock_irqsave(&local->cmdlock, flags);
	if (!list_empty(&entry->list)) {
		printk(KERN_DEBUG "%s: hfa384x_cmd: entry still in list? "
		       "(entry=%p, type=%d, res=%d)\n", dev->name, entry,
		       entry->type, res);
		list_del_init(&entry->list);
		local->cmd_queue_len--;
	}
	spin_unlock_irqrestore(&local->cmdlock, flags);

	if (err) {
		printk(KERN_DEBUG "%s: hfa384x_cmd: interrupted; err=%d\n",
		       dev->name, err);
		res = err;
		goto done;
	}

	if (entry->type != CMD_COMPLETED) {
		u16 reg = HFA384X_INW(HFA384X_EVSTAT_OFF);
		printk(KERN_DEBUG "%s: hfa384x_cmd: command was not "
		       "completed (res=%d, entry=%p, type=%d, cmd=0x%04x, "
		       "param0=0x%04x, EVSTAT=%04x)\n", dev->name, res,
		       entry, entry->type, entry->cmd, entry->param0, reg);
		if (reg & HFA384X_EV_CMD) {
			/* Command completion event is pending, but the
			 * interrupt was not delivered - probably an issue
			 * with pcmcia-cs configuration. */
			printk(KERN_WARNING "%s: interrupt delivery does not "
			       "seem to work\n", dev->name);
		}
		prism2_io_debug_error(dev, 3);
		res = -ETIMEDOUT;
		goto done;
	}

	if (resp0 != NULL)
		*resp0 = entry->resp0;
#ifndef final_version
	if (entry->res) {
		printk(KERN_DEBUG "%s: CMD=0x%04x => res=0x%02x, "
		       "resp0=0x%04x\n",
		       dev->name, cmd, entry->res, entry->resp0);
	}
#endif /* final_version */

	res = entry->res;
 done:
	hostap_cmd_queue_free(local, entry, 1);
	return res;
}


/* Issue given command (possibly after waiting in command queue) and use
 * callback function to indicate command completion. This can be called both
 * from user and interrupt context. */
static int hfa384x_cmd_callback(struct net_device *dev, u16 cmd, u16 param0,
				void (*callback)(struct net_device *dev,
						 void *context, u16 resp0,
						 u16 status),
				void *context)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int issue, ret;
	unsigned long flags;
	struct hostap_cmd_queue *entry;

	if (local->cmd_queue_len >= HOSTAP_CMD_QUEUE_MAX_LEN + 2) {
		printk(KERN_DEBUG "%s: hfa384x_cmd: cmd_queue full\n",
		       dev->name);
		return -1;
	}

	entry = (struct hostap_cmd_queue *)
		kmalloc(sizeof(*entry), GFP_ATOMIC);
	if (entry == NULL) {
		printk(KERN_DEBUG "%s: hfa384x_cmd_callback - kmalloc "
		       "failed\n", dev->name);
		return -ENOMEM;
	}
	memset(entry, 0, sizeof(*entry));
	atomic_set(&entry->usecnt, 1);
	entry->type = CMD_CALLBACK;
	entry->cmd = cmd;
	entry->param0 = param0;
	entry->callback = callback;
	entry->context = context;

	spin_lock_irqsave(&local->cmdlock, flags);
	issue = list_empty(&local->cmd_queue);
	if (issue)
		entry->issuing = 1;
	list_add_tail(&entry->list, &local->cmd_queue);
	local->cmd_queue_len++;
	spin_unlock_irqrestore(&local->cmdlock, flags);

	if (issue && hfa384x_cmd_issue(dev, entry))
		ret = -ETIMEDOUT;
	else
		ret = 0;

	hostap_cmd_queue_free(local, entry, ret);

	return ret;
}


static int hfa384x_cmd_wait(struct net_device *dev, u16 cmd, u16 param0)
{
	int res, tries;
	u16 reg;

	/* wait until busy bit is clear; this should always be clear since the
	 * commands are serialized */
	tries = HFA384X_CMD_BUSY_TIMEOUT;
	while (HFA384X_INW(HFA384X_CMD_OFF) & HFA384X_CMD_BUSY && tries > 0) {
		tries--;
		udelay(1);
	}
	if (tries == 0) {
		prism2_io_debug_error(dev, 4);
		printk(KERN_DEBUG "%s: hfa384x_cmd_wait - timeout - "
		       "reg=0x%04x\n", dev->name,
		       HFA384X_INW(HFA384X_CMD_OFF));
		return -ETIMEDOUT;
	}

	/* write command */
	HFA384X_OUTW(param0, HFA384X_PARAM0_OFF);
	HFA384X_OUTW(cmd, HFA384X_CMD_OFF);

        /* wait for command completion */
	if ((cmd & HFA384X_CMDCODE_MASK) == HFA384X_CMDCODE_DOWNLOAD)
		tries = HFA384X_DL_COMPL_TIMEOUT;
	else
		tries = HFA384X_CMD_COMPL_TIMEOUT;

        while (!(HFA384X_INW(HFA384X_EVSTAT_OFF) & HFA384X_EV_CMD) &&
               tries > 0) {
                tries--;
                udelay(10);
        }
        if (tries == 0) {
                reg = HFA384X_INW(HFA384X_EVSTAT_OFF);
		prism2_io_debug_error(dev, 5);
                printk(KERN_DEBUG "%s: hfa384x_cmd_wait - timeout2 - "
		       "reg=0x%04x\n", dev->name, reg);
                return -ETIMEDOUT;
        }

        res = (HFA384X_INW(HFA384X_STATUS_OFF) &
               (BIT(14) | BIT(13) | BIT(12) | BIT(11) | BIT(10) | BIT(9) |
                BIT(8))) >> 8;
#ifndef final_version
	if (res) {
		printk(KERN_DEBUG "%s: CMD=0x%04x => res=0x%02x\n",
		       dev->name, cmd, res);
	}
#endif

	HFA384X_OUTW(HFA384X_EV_CMD, HFA384X_EVACK_OFF);

	return res;
}


static int hfa384x_cmd_no_wait(struct net_device *dev, u16 cmd, u16 param0)
{
	int tries;
	u16 reg;

	/* wait until busy bit is clear */
	tries = HFA384X_CMD_BUSY_TIMEOUT;
	while (HFA384X_INW(HFA384X_CMD_OFF) & HFA384X_CMD_BUSY && tries > 0) {
		tries--;
		udelay(1);
	}
	if (tries == 0) {
		reg = HFA384X_INW(HFA384X_CMD_OFF);
		prism2_io_debug_error(dev, 6);
		printk("%s: hfa384x_cmd - timeout - reg=0x%04x\n", dev->name,
		       reg);
		return -ETIMEDOUT;
	}

	/* write command */
	HFA384X_OUTW(param0, HFA384X_PARAM0_OFF);
	HFA384X_OUTW(cmd, HFA384X_CMD_OFF);

	return 0;
}


static inline int hfa384x_wait_offset(struct net_device *dev, u16 o_off)
{
	int tries = HFA384X_BAP_BUSY_TIMEOUT;

	while ((HFA384X_INW(o_off) & HFA384X_OFFSET_BUSY) && tries > 0) {
		tries--;
		udelay(10);
	}
	return (HFA384X_INW(o_off) & HFA384X_OFFSET_BUSY);
}


/* Offset must be even */
static int hfa384x_setup_bap(struct net_device *dev, u16 bap, u16 id,
			     int offset)
{
	u16 o_off, s_off;
	int ret = 0;

	if (offset % 2 || bap > 1)
		return -EINVAL;

	if (bap == BAP1) {
		o_off = HFA384X_OFFSET1_OFF;
		s_off = HFA384X_SELECT1_OFF;
	} else {
		o_off = HFA384X_OFFSET0_OFF;
		s_off = HFA384X_SELECT0_OFF;
	}

	if (hfa384x_wait_offset(dev, o_off)) {
		prism2_io_debug_error(dev, 7);
		printk(KERN_DEBUG "%s: hfa384x_setup_bap - timeout before\n",
		       dev->name);
		ret = -ETIMEDOUT;
		goto out;
	}

	HFA384X_OUTW(id, s_off);
	HFA384X_OUTW(offset, o_off);

	if (hfa384x_wait_offset(dev, o_off)) {
		prism2_io_debug_error(dev, 8);
		printk(KERN_DEBUG "%s: hfa384x_setup_bap - timeout after\n",
		       dev->name);
		ret = -ETIMEDOUT;
		goto out;
	}
#ifndef final_version
	if (HFA384X_INW(o_off) & HFA384X_OFFSET_ERR) {
		prism2_io_debug_error(dev, 9);
		printk(KERN_DEBUG "%s: hfa384x_setup_bap - offset error "
		       "(%d,%d,%d)\n",
		       dev->name, bap, id, offset);
		ret = -EINVAL;
	}
#endif

 out:
	return ret;
}


static int hfa384x_get_rid(struct net_device *dev, u16 rid, void *buf, int len,
			   int exact_len)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int res, rlen = 0;
	struct hfa384x_rid_hdr rec;

	res = down_interruptible(&local->rid_bap_sem);
	if (res)
		return res;

	res = hfa384x_cmd(dev, HFA384X_CMDCODE_ACCESS, rid, NULL, NULL);
	if (res) {
		printk(KERN_DEBUG "%s: hfa384x_get_rid: CMDCODE_ACCESS failed "
		       "(res=%d, rid=%04x, len=%d)\n",
		       dev->name, res, rid, len);
		up(&local->rid_bap_sem);
		return res;
	}

	spin_lock_bh(&local->baplock);

	res = hfa384x_setup_bap(dev, BAP0, rid, 0);
	if (!res)
		res = hfa384x_from_bap(dev, BAP0, &rec, sizeof(rec));

	if (le16_to_cpu(rec.len) == 0) {
		/* RID not available */
		res = -ENODATA;
	}

	rlen = (le16_to_cpu(rec.len) - 1) * 2;
	if (!res && exact_len && rlen != len) {
		printk(KERN_DEBUG "%s: hfa384x_get_rid - RID len mismatch: "
		       "rid=0x%04x, len=%d (expected %d)\n",
		       dev->name, rid, rlen, len);
		res = -ENODATA;
	}

	if (!res)
		res = hfa384x_from_bap(dev, BAP0, buf, len);

	spin_unlock_bh(&local->baplock);
	up(&local->rid_bap_sem);

	if (res) {
		if (res != -ENODATA)
			printk(KERN_DEBUG "%s: hfa384x_get_rid (rid=%04x, "
			       "len=%d) - failed - res=%d\n", dev->name, rid,
			       len, res);
		if (res == -ETIMEDOUT)
			prism2_hw_reset(dev);
		return res;
	}

	return rlen;
}


static int hfa384x_set_rid(struct net_device *dev, u16 rid, void *buf, int len)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct hfa384x_rid_hdr rec;
	int res;

	rec.rid = cpu_to_le16(rid);
	/* RID len in words and +1 for rec.rid */
	rec.len = cpu_to_le16(len / 2 + len % 2 + 1);

	res = down_interruptible(&local->rid_bap_sem);
	if (res)
		return res;

	spin_lock_bh(&local->baplock);
	res = hfa384x_setup_bap(dev, BAP0, rid, 0);
	if (!res)
		res = hfa384x_to_bap(dev, BAP0, &rec, sizeof(rec));
	if (!res)
		res = hfa384x_to_bap(dev, BAP0, buf, len);
	spin_unlock_bh(&local->baplock);

	if (res) {
		printk(KERN_DEBUG "%s: hfa384x_set_rid (rid=%04x, len=%d) - "
		       "failed - res=%d\n", dev->name, rid, len, res);
		up(&local->rid_bap_sem);
		return res;
	}

	res = hfa384x_cmd(dev, HFA384X_CMDCODE_ACCESS_WRITE, rid, NULL, NULL);
	up(&local->rid_bap_sem);
	if (res) {
		printk(KERN_DEBUG "%s: hfa384x_set_rid: CMDCODE_ACCESS_WRITE "
		       "failed (res=%d, rid=%04x, len=%d)\n",
		       dev->name, res, rid, len);
		return res;
	}

	if (res == -ETIMEDOUT)
		prism2_hw_reset(dev);

	return res;
}


static void hfa384x_disable_interrupts(struct net_device *dev)
{
	/* disable interrupts and clear event status */
	HFA384X_OUTW(0, HFA384X_INTEN_OFF);
	HFA384X_OUTW(0xffff, HFA384X_EVACK_OFF);
}


static void hfa384x_enable_interrupts(struct net_device *dev)
{
	/* ack pending events and enable interrupts from selected events */
	HFA384X_OUTW(0xffff, HFA384X_EVACK_OFF);
	HFA384X_OUTW(HFA384X_EVENT_MASK, HFA384X_INTEN_OFF);
}


static void hfa384x_events_no_bap0(struct net_device *dev)
{
	HFA384X_OUTW(HFA384X_EVENT_MASK & ~HFA384X_BAP0_EVENTS,
		     HFA384X_INTEN_OFF);
}


static void hfa384x_events_all(struct net_device *dev)
{
	HFA384X_OUTW(HFA384X_EVENT_MASK, HFA384X_INTEN_OFF);
}


static void hfa384x_events_only_cmd(struct net_device *dev)
{
	HFA384X_OUTW(HFA384X_EV_CMD, HFA384X_INTEN_OFF);
}


static u16 hfa384x_allocate_fid(struct net_device *dev, int len)
{
	int tries;
	u16 fid;

	/* FIX: this could be replace with hfa384x_cmd() if the Alloc event
	 * below would be handled like CmdCompl event (sleep here, wake up from
	 * interrupt handler */
	if (hfa384x_cmd_wait(dev, HFA384X_CMDCODE_ALLOC, len)) {
		printk(KERN_DEBUG "%s: cannot allocate fid, len=%d\n",
		       dev->name, len);
		return 0xffff;
	}

	tries = HFA384X_ALLOC_COMPL_TIMEOUT;
	while (!(HFA384X_INW(HFA384X_EVSTAT_OFF) & HFA384X_EV_ALLOC) &&
	       tries > 0) {
		tries--;
		udelay(10);
	}
	if (tries == 0) {
		printk("%s: fid allocate, len=%d - timeout\n", dev->name, len);
		return 0xffff;
	}

	fid = HFA384X_INW(HFA384X_ALLOCFID_OFF);
	HFA384X_OUTW(HFA384X_EV_ALLOC, HFA384X_EVACK_OFF);

	return fid;
}


static int prism2_reset_port(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int res;

	if (!local->dev_enabled)
		return 0;

	res = hfa384x_cmd(dev, HFA384X_CMDCODE_DISABLE, 0,
			  NULL, NULL);
	if (res)
		printk(KERN_DEBUG "%s: reset port failed to disable port\n",
		       dev->name);
	else {
		res = hfa384x_cmd(dev, HFA384X_CMDCODE_ENABLE, 0,
				  NULL, NULL);
		if (res)
			printk(KERN_DEBUG "%s: reset port failed to enable "
			       "port\n", dev->name);
	}

	return res;
}


static int prism2_get_version_info(struct net_device *dev, u16 rid,
				   const char *txt)
{
	struct hfa384x_comp_ident comp;

	if (hfa384x_get_rid(dev, rid, &comp, sizeof(comp), 1) < 0) {
		printk(KERN_DEBUG "Could not get RID for component %s\n", txt);
		return -1;
	}

	printk(KERN_INFO "%s: %s: id=0x%02x v%d.%d.%d\n", dev->name, txt,
	       __le16_to_cpu(comp.id), __le16_to_cpu(comp.major),
	       __le16_to_cpu(comp.minor), __le16_to_cpu(comp.variant));
	return 0;
}


static int prism2_setup_rids(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	u16 tmp;
	int ret = 0;

	hostap_set_word(dev, HFA384X_RID_TICKTIME, 2000);

	if (!local->fw_ap) {
		tmp = hostap_get_porttype(local);
		ret = hostap_set_word(dev, HFA384X_RID_CNFPORTTYPE, tmp);
		if (ret) {
			printk("%s: Port type setting to %d failed\n",
			       dev->name, tmp);
			goto fail;
		}
	}

	/* Setting SSID to empty string seems to kill the card in Host AP mode
	 */
	if (local->iw_mode != IW_MODE_MASTER || local->essid[0] != '\0') {
		ret = hostap_set_string(dev, HFA384X_RID_CNFOWNSSID,
					local->essid);
		if (ret) {
			printk("%s: AP own SSID setting failed\n", dev->name);
			goto fail;
		}
	}

	ret = hostap_set_word(dev, HFA384X_RID_CNFMAXDATALEN,
			      PRISM2_DATA_MAXLEN);
	if (ret) {
		printk("%s: MAC data length setting to %d failed\n",
		       dev->name, PRISM2_DATA_MAXLEN);
		goto fail;
	}

	if (hfa384x_get_rid(dev, HFA384X_RID_CHANNELLIST, &tmp, 2, 1) < 0) {
		printk("%s: Channel list read failed\n", dev->name);
		ret = -EINVAL;
		goto fail;
	}
	local->channel_mask = __le16_to_cpu(tmp);

	if (local->channel < 1 || local->channel > 14 ||
	    !(local->channel_mask & (1 << (local->channel - 1)))) {
		printk(KERN_WARNING "%s: Channel setting out of range "
		       "(%d)!\n", dev->name, local->channel);
		ret = -EBUSY;
		goto fail;
	}

	ret = hostap_set_word(dev, HFA384X_RID_CNFOWNCHANNEL, local->channel);
	if (ret) {
		printk("%s: Channel setting to %d failed\n",
		       dev->name, local->channel);
		goto fail;
	}

	ret = hostap_set_word(dev, HFA384X_RID_CNFBEACONINT,
			      local->beacon_int);
	if (ret) {
		printk("%s: Beacon interval setting to %d failed\n",
		       dev->name, local->beacon_int);
		/* this may fail with Symbol/Lucent firmware */
		if (ret == -ETIMEDOUT)
			goto fail;
	}

	ret = hostap_set_word(dev, HFA384X_RID_CNFOWNDTIMPERIOD,
			      local->dtim_period);
	if (ret) {
		printk("%s: DTIM period setting to %d failed\n",
		       dev->name, local->dtim_period);
		/* this may fail with Symbol/Lucent firmware */
		if (ret == -ETIMEDOUT)
			goto fail;
	}

	ret = hostap_set_word(dev, HFA384X_RID_PROMISCUOUSMODE,
			      local->is_promisc);
	if (ret)
		printk(KERN_INFO "%s: Setting promiscuous mode (%d) failed\n",
		       dev->name, local->is_promisc);

	if (!local->fw_ap) {
		ret = hostap_set_string(dev, HFA384X_RID_CNFDESIREDSSID,
					local->essid);
		if (ret) {
			printk("%s: Desired SSID setting failed\n", dev->name);
			goto fail;
		}
	}

	/* Setup TXRateControl, defaults to allow use of 1, 2, 5.5, and
	 * 11 Mbps in automatic TX rate fallback and 1 and 2 Mbps as basic
	 * rates */
	if (local->tx_rate_control == 0) {
		local->tx_rate_control =
			HFA384X_RATES_1MBPS |
			HFA384X_RATES_2MBPS |
			HFA384X_RATES_5MBPS |
			HFA384X_RATES_11MBPS;
	}
	if (local->basic_rates == 0)
		local->basic_rates = HFA384X_RATES_1MBPS | HFA384X_RATES_2MBPS;

	if (!local->fw_ap) {
		ret = hostap_set_word(dev, HFA384X_RID_TXRATECONTROL,
				      local->tx_rate_control);
		if (ret) {
			printk("%s: TXRateControl setting to %d failed\n",
			       dev->name, local->tx_rate_control);
			goto fail;
		}

		ret = hostap_set_word(dev, HFA384X_RID_CNFSUPPORTEDRATES,
				      local->tx_rate_control);
		if (ret) {
			printk("%s: cnfSupportedRates setting to %d failed\n",
			       dev->name, local->tx_rate_control);
		}

		ret = hostap_set_word(dev, HFA384X_RID_CNFBASICRATES,
				      local->basic_rates);
		if (ret) {
			printk("%s: cnfBasicRates setting to %d failed\n",
			       dev->name, local->basic_rates);
		}

		ret = hostap_set_word(dev, HFA384X_RID_CREATEIBSS, 1);
		if (ret) {
			printk("%s: Create IBSS setting to 1 failed\n",
			       dev->name);
		}
	}

	if (local->name_set)
		(void) hostap_set_string(dev, HFA384X_RID_CNFOWNNAME,
					 local->name);

	if (hostap_set_encryption(local)) {
		printk(KERN_INFO "%s: could not configure encryption\n",
		       dev->name);
	}

	(void) hostap_set_antsel(local);

	if (local->host_roaming &&
	    hostap_set_word(dev, HFA384X_RID_CNFROAMINGMODE,
			    HFA384X_ROAMING_HOST)) {
		printk(KERN_INFO "%s: could not set host roaming\n",
		       dev->name);
	}

	if (local->sta_fw_ver >= PRISM2_FW_VER(1,6,3) &&
	    hostap_set_word(dev, HFA384X_RID_CNFENHSECURITY, local->enh_sec))
		printk(KERN_INFO "%s: cnfEnhSecurity setting to 0x%x failed\n",
		       dev->name, local->enh_sec);

 fail:
	return ret;
}


static void prism2_clear_cmd_queue(local_info_t *local)
{
	struct list_head *ptr, *n;
	unsigned long flags;
	struct hostap_cmd_queue *entry;

	spin_lock_irqsave(&local->cmdlock, flags);
	for (ptr = local->cmd_queue.next, n = ptr->next;
	     ptr != &local->cmd_queue; ptr = n, n = ptr->next) {
		entry = list_entry(ptr, struct hostap_cmd_queue, list);
		atomic_inc(&entry->usecnt);
		printk(KERN_DEBUG "%s: removed pending cmd_queue entry "
		       "(type=%d, cmd=0x%04x, param0=0x%04x)\n",
		       local->dev->name, entry->type, entry->cmd,
		       entry->param0);
		__hostap_cmd_queue_free(local, entry, 1);
	}
	if (local->cmd_queue_len) {
		printk(KERN_DEBUG "%s: cmd_queue_len (%d) not zero after "
		       "flush\n", local->dev->name, local->cmd_queue_len);
		local->cmd_queue_len = 0;
	}
	spin_unlock_irqrestore(&local->cmdlock, flags);
}


static int prism2_hw_init(struct net_device *dev, int initial)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int ret, i, first = 1;

	PDEBUG(DEBUG_FLOW, "prism2_hw_init()\n");

	clear_bit(HOSTAP_BITS_TRANSMIT, &local->bits);

 init:
	/* initialize HFA 384x */
	ret = hfa384x_cmd_no_wait(dev, HFA384X_CMDCODE_INIT, 0);
	if (ret) {
		printk("%s: first command failed - is the card compatible?\n",
		       dev_info);
		goto failed;
	}
	i = HFA384X_INIT_TIMEOUT;
	while (!(HFA384X_INW(HFA384X_EVSTAT_OFF) & HFA384X_EV_CMD) && i > 0) {
		i--;
		udelay(10);
	}
	if (first && i == HFA384X_INIT_TIMEOUT) {
		/* EvStat has Cmd bit set in some cases, so retry once if no
		 * wait was needed */
		HFA384X_OUTW(HFA384X_EV_CMD, HFA384X_EVACK_OFF);
		printk(KERN_DEBUG "%s: init command completed too quickly - "
		       "retrying\n", dev->name);
		first = 0;
		goto init;
	}
	if (i == 0) {
		printk("%s: card initialization timed out\n", dev_info);
		goto failed;
	}
	printk(KERN_DEBUG "prism2_hw_init: initialized in %d iterations\n",
	       HFA384X_INIT_TIMEOUT - i);
	HFA384X_OUTW(HFA384X_EV_CMD, HFA384X_EVACK_OFF);

	if (local->hw_downloading)
		return 0;

#ifdef PRISM2_DOWNLOAD_SUPPORT
	local->pda = prism2_read_pda(dev);
#endif /* PRISM2_DOWNLOAD_SUPPORT */

	hfa384x_disable_interrupts(dev);

#ifndef final_version
	HFA384X_OUTW(HFA384X_MAGIC, HFA384X_SWSUPPORT0_OFF);
	if (HFA384X_INW(HFA384X_SWSUPPORT0_OFF) != HFA384X_MAGIC) {
		printk("SWSUPPORT0 write/read failed: %04X != %04X\n",
		       HFA384X_INW(HFA384X_SWSUPPORT0_OFF), HFA384X_MAGIC);
		goto failed;
	}
#endif

	/* FIX: could convert allocate_fid to use sleeping CmdCompl wait and
	 * enable interrupts before this. This would also require some sort of
	 * sleeping AllocEv waiting */

	/* allocate TX FIDs */
	local->txfid_len = PRISM2_TXFID_LEN;
	for (i = 0; i < PRISM2_TXFID_COUNT; i++) {
		local->txfid[i] = hfa384x_allocate_fid(dev, local->txfid_len);
		if (local->txfid[i] == 0xffff && local->txfid_len > 1600) {
			local->txfid[i] = hfa384x_allocate_fid(dev, 1600);
			if (local->txfid[i] != 0xffff) {
				printk(KERN_DEBUG "%s: Using shorter TX FID "
				       "(1600 bytes)\n", dev->name);
				local->txfid_len = 1600;
			}
		}
		if (local->txfid[i] == 0xffff)
			goto failed;
		local->intransmitfid[i] = PRISM2_TXFID_EMPTY;
	}

	hfa384x_events_only_cmd(dev);

	if (initial) {
		/* get card version information */
		prism2_get_version_info(dev, HFA384X_RID_NICID, "NIC");
		prism2_get_version_info(dev, HFA384X_RID_PRIID, "PRI");
		prism2_get_version_info(dev, HFA384X_RID_STAID, "STA");

		prism2_check_sta_fw_version(local);

		if (hfa384x_get_rid(dev, HFA384X_RID_CNFOWNMACADDR,
				    &dev->dev_addr, 6, 1) < 0) {
			printk("%s: could not get own MAC address\n",
			       dev->name);
		}
		if (local->apdev)
			memcpy(local->apdev->dev_addr, dev->dev_addr,
			       ETH_ALEN);
		if (local->stadev)
			memcpy(local->stadev->dev_addr, dev->dev_addr,
			       ETH_ALEN);
	} else if (local->fw_ap)
		prism2_check_sta_fw_version(local);

	prism2_setup_rids(dev);

	/* MAC is now configured, but port 0 is not yet enabled */
	return 0;

 failed:
	printk(KERN_WARNING "%s: Initialization failed\n", dev_info);
	return 1;
}


static int prism2_hw_enable(struct net_device *dev, int initial)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int was_resetting = local->hw_resetting;

	if (hfa384x_cmd(dev, HFA384X_CMDCODE_ENABLE, 0, NULL, NULL)) {
		printk("%s: MAC port 0 enabling failed\n", dev->name);
		return 1;
	}

	local->hw_ready = 1;
	local->hw_reset_tries = 0;
	local->hw_resetting = 0;
	hfa384x_enable_interrupts(dev);

	/* at least D-Link DWL-650 seems to require additional port reset
	 * before it starts acting as an AP, so reset port automatically
	 * here just in case */
	if (initial && prism2_reset_port(dev)) {
		printk("%s: MAC port 0 reseting failed\n", dev->name);
		return 1;
	}

	if (was_resetting && netif_queue_stopped(dev)) {
		/* If hw_reset() was called during pending transmit, netif
		 * queue was stopped. Wake it up now since the wlan card has
		 * been resetted. */
		hostap_netif_wake_queues(dev);
	}

	return 0;
}


static int prism2_hw_config(struct net_device *dev, int initial)
{
	local_info_t *local = (local_info_t *) dev->priv;
	if (local->hw_downloading)
		return 1;

	if (prism2_hw_init(dev, initial))
		return 1;

	if (!initial || !delayed_enable) {
		if (!local->dev_enabled)
			prism2_callback(local, PRISM2_CALLBACK_ENABLE);
		local->dev_enabled = 1;
		return prism2_hw_enable(dev, initial);
	}

	return 0;
}


static void prism2_hw_shutdown(struct net_device *dev, int no_disable)
{
	local_info_t *local = (local_info_t *) dev->priv;

	/* Allow only command completion events during disable */
	hfa384x_events_only_cmd(dev);

	local->hw_ready = 0;
	if (local->dev_enabled)
		prism2_callback(local, PRISM2_CALLBACK_DISABLE);
	local->dev_enabled = 0;

	if (local->func->card_present && !local->func->card_present(local)) {
		printk(KERN_DEBUG "%s: card already removed or not configured "
		       "during shutdown\n", dev->name);
		return;
	}

	if ((no_disable & HOSTAP_HW_NO_DISABLE) == 0 &&
	    hfa384x_cmd(dev, HFA384X_CMDCODE_DISABLE, 0, NULL, NULL))
		printk(KERN_WARNING "%s: Shutdown failed\n", dev_info);

	hfa384x_disable_interrupts(dev);

	if (no_disable & HOSTAP_HW_ENABLE_CMDCOMPL)
		hfa384x_events_only_cmd(dev);
	else
		prism2_clear_cmd_queue(local);
}


static void prism2_hw_reset(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;

#if 0
	static long last_reset = 0;

	/* do not reset card more than once per second to avoid ending up in a
	 * busy loop reseting the card */
	if (last_reset + HZ >= jiffies)
		return;
	last_reset = jiffies;
#endif

	if (in_interrupt()) {
		printk(KERN_DEBUG "%s: driver bug - prism2_hw_reset() called "
		       "in interrupt context\n", dev->name);
		return;
	}

	if (local->hw_downloading)
		return;

	if (local->hw_resetting) {
		printk(KERN_WARNING "%s: %s: already resetting card - "
		       "ignoring reset request\n", dev_info, dev->name);
		return;
	}

	local->hw_reset_tries++;
	if (local->hw_reset_tries > 10) {
		printk(KERN_WARNING "%s: too many reset tries, skipping\n",
		       dev->name);
		return;
	}

	printk(KERN_WARNING "%s: %s: resetting card\n", dev_info, dev->name);
	hfa384x_disable_interrupts(dev);
	local->hw_resetting = 1;
	if (local->func->cor_sreset) {
		/* Host system seems to hang in some cases with high traffic
		 * load or shared interrupts during COR sreset. Disable shared
		 * interrupts during reset to avoid these crashes. COS sreset
		 * takes quite a long time, so it is unfortunate that this
		 * seems to be needed. Anyway, I do not know of any better way
		 * of avoiding the crash. */
		disable_irq(dev->irq);
		local->func->cor_sreset(local);
		enable_irq(dev->irq);
	}
	prism2_hw_shutdown(dev, 1);
	prism2_hw_config(dev, 0);
	local->hw_resetting = 0;
}


static void prism2_schedule_reset(local_info_t *local)
{
	PRISM2_SCHEDULE_TASK(&local->reset_queue);
}


/* Called only as scheduled task after noticing card timeout in interrupt
 * context */
static void handle_reset_queue(void *data)
{
	local_info_t *local = (local_info_t *) data;

	printk(KERN_DEBUG "%s: scheduled card reset\n", local->dev->name);
	prism2_hw_reset(local->dev);

	if (netif_queue_stopped(local->dev)) {
		int i;

		for (i = 0; i < PRISM2_TXFID_COUNT; i++)
			if (local->intransmitfid[i] == PRISM2_TXFID_EMPTY) {
				PDEBUG(DEBUG_EXTRA, "prism2_tx_timeout: "
				       "wake up queue\n");
				hostap_netif_wake_queues(local->dev);
				break;
			}
	}

#ifndef NEW_MODULE_CODE
	MOD_DEC_USE_COUNT;
#endif
}


/* update trans_start for all used devices */
static void prism2_netif_update_trans_start(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	prism2_wds_info_t *wds;

	if (local->dev)
		local->dev->trans_start = jiffies;
	if (local->apdev)
		local->apdev->trans_start = jiffies;
	if (local->stadev)
		local->stadev->trans_start = jiffies;

	wds = local->wds;
	while (wds != NULL) {
		wds->dev.trans_start = jiffies;
		wds = wds->next;
	}
}


static int prism2_get_txfid_idx(local_info_t *local)
{
	int idx, end;

	spin_lock_bh(&local->txfidlock);
	end = idx = local->next_txfid;
	do {
		if (local->intransmitfid[idx] == PRISM2_TXFID_EMPTY) {
			local->intransmitfid[idx] = PRISM2_TXFID_RESERVED;
			spin_unlock_bh(&local->txfidlock);
			return idx;
		}
		idx++;
		if (idx >= PRISM2_TXFID_COUNT)
			idx = 0;
	} while (idx != end);
	spin_unlock_bh(&local->txfidlock);

	PDEBUG(DEBUG_EXTRA2, "prism2_get_txfid_idx: no room in txfid buf: "
	       "packet dropped\n");
	local->stats.tx_dropped++;

	return -1;
}


/* Called only from hardware IRQ */
static void prism2_transmit_cb(struct net_device *dev, void *context,
			       u16 resp0, u16 res)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int idx = (int) context;

	if (res) {
		printk(KERN_DEBUG "%s: prism2_transmit_cb - res=0x%02x\n",
		       dev->name, res);
		return;
	}

	if (idx < 0 || idx >= PRISM2_TXFID_COUNT) {
		printk(KERN_DEBUG "%s: prism2_transmit_cb called with invalid "
		       "idx=%d\n", dev->name, idx);
		return;
	}

	if (!test_and_clear_bit(HOSTAP_BITS_TRANSMIT, &local->bits)) {
		printk(KERN_DEBUG "%s: driver bug: prism2_transmit_cb called "
		       "with no pending transmit\n", dev->name);
	}

	if (netif_queue_stopped(dev)) {
		/* ready for next TX, so wake up queue that was stopped in
		 * prism2_transmit() */
		hostap_netif_wake_queues(dev);
	}

	/* FIX: is some locking needed for txfid data? */

	/* With reclaim, Resp0 contains new txfid for transmit; the old txfid
	 * will be automatically allocated for the next TX frame */
	local->intransmitfid[idx] = resp0;

	PDEBUG(DEBUG_FID, "%s: prism2_cmd_ev: txfid[%d]=0x%04x, resp0=0x%04x, "
	       "transmit_txfid=0x%04x\n", dev->name, idx, local->txfid[idx],
	       resp0, local->intransmitfid[local->next_txfid]);

	idx++;
	if (idx >= PRISM2_TXFID_COUNT)
		idx = 0;
	local->next_txfid = idx;

	/* check if all TX buffers are occupied */
	do {
		if (local->intransmitfid[idx] == PRISM2_TXFID_EMPTY) {
			return;
		}
		idx++;
		if (idx >= PRISM2_TXFID_COUNT)
			idx = 0;
	} while (idx != local->next_txfid);

	/* no empty TX buffers, stop queue */
	hostap_netif_stop_queues(dev);
}


/* Called only from software IRQ if PCI bus master is not used (with bus master
 * this can be called both from software and hardware IRQ) */
static int prism2_transmit(struct net_device *dev, int idx)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int res;

	/* The driver tries to stop netif queue so that there would not be
	 * more than one attempt to transmit frames going on; check that this
	 * is really the case */

	if (test_and_set_bit(HOSTAP_BITS_TRANSMIT, &local->bits)) {
		printk(KERN_DEBUG "%s: driver bug - prism2_transmit() called "
		       "when previous TX was pending\n", dev->name);
		return -1;
	}

	/* stop the queue for the time that transmit is pending */
	hostap_netif_stop_queues(dev);

	/* transmit packet */
	res = hfa384x_cmd_callback(
		dev,
		HFA384X_CMDCODE_TRANSMIT | HFA384X_CMD_TX_RECLAIM,
		local->txfid[idx],
		prism2_transmit_cb, (void *) idx);

	if (res) {
		struct net_device_stats *stats;
		printk(KERN_DEBUG "%s: prism2_transmit: CMDCODE_TRANSMIT "
		       "failed (res=%d)\n", dev->name, res);
		stats = hostap_get_stats(dev);
		stats->tx_dropped++;
		hostap_netif_wake_queues(dev);
		return -1;
	}
	prism2_netif_update_trans_start(dev);

	/* Since we did not wait for command completion, the card continues
	 * to process on the background and we will finish handling when
	 * command completion event is handled (prism2_cmd_ev() function) */

	return 0;
}


/* Called only from hardware IRQ */
static void prism2_cmd_ev(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct hostap_cmd_queue *entry = NULL;

	spin_lock(&local->cmdlock);
	if (!list_empty(&local->cmd_queue)) {
		entry = list_entry(local->cmd_queue.next,
				   struct hostap_cmd_queue, list);
		atomic_inc(&entry->usecnt);
		list_del_init(&entry->list);
		local->cmd_queue_len--;

		if (!entry->issued) {
			printk(KERN_DEBUG "%s: Command completion event, but "
			       "cmd not issued\n", dev->name);
			__hostap_cmd_queue_free(local, entry, 1);
			entry = NULL;
		}
	}
	spin_unlock(&local->cmdlock);

	if (!entry) {
		HFA384X_OUTW(HFA384X_EV_CMD, HFA384X_EVACK_OFF);
		printk(KERN_DEBUG "%s: Command completion event, but no "
		       "pending commands\n", dev->name);
		return;
	}

	entry->resp0 = HFA384X_INW(HFA384X_RESP0_OFF);
	entry->res = (HFA384X_INW(HFA384X_STATUS_OFF) &
		      (BIT(14) | BIT(13) | BIT(12) | BIT(11) | BIT(10) |
		       BIT(9) | BIT(8))) >> 8;
	HFA384X_OUTW(HFA384X_EV_CMD, HFA384X_EVACK_OFF);

	/* TODO: rest of the CmdEv handling could be moved to tasklet */
	if (entry->type == CMD_SLEEP) {
		entry->type = CMD_COMPLETED;
		wake_up_interruptible(&entry->compl);
	} else if (entry->type == CMD_CALLBACK) {
		if (entry->callback)
			entry->callback(dev, entry->context, entry->resp0,
					entry->res);
	} else {
		printk(KERN_DEBUG "%s: Invalid command completion type %d\n",
		       dev->name, entry->type);
	}
	hostap_cmd_queue_free(local, entry, 1);

	/* issue next command, if pending */
	entry = NULL;
	spin_lock(&local->cmdlock);
	if (!list_empty(&local->cmd_queue)) {
		entry = list_entry(local->cmd_queue.next,
				   struct hostap_cmd_queue, list);
		if (entry->issuing) {
			/* hfa384x_cmd() has already started issuing this
			 * command, so do not start here */
			entry = NULL;
		}
		if (entry)
			atomic_inc(&entry->usecnt);
	}
	spin_unlock(&local->cmdlock);

	if (entry) {
		/* issue next command; if command issuing fails, remove the
		 * entry from cmd_queue */
		int res = hfa384x_cmd_issue(dev, entry);
		spin_lock(&local->cmdlock);
		__hostap_cmd_queue_free(local, entry, res);
		spin_unlock(&local->cmdlock);
	}
}


#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
static void prism2_tx_cb(struct net_device *dev, void *context,
			 u16 resp0, u16 res)
{
	local_info_t *local = (local_info_t *) dev->priv;
	unsigned long addr;
	int buf_len = (int) context;

	if (res) {
		printk(KERN_DEBUG "%s: prism2_tx_cb - res=0x%02x\n",
		       dev->name, res);
		return;
	}

	addr = virt_to_phys(local->bus_m0_buf);
	HFA384X_OUTW((addr & 0xffff0000) >> 16, HFA384X_PCI_M0_ADDRH_OFF);
	HFA384X_OUTW(addr & 0x0000ffff, HFA384X_PCI_M0_ADDRL_OFF);
	HFA384X_OUTW(buf_len / 2, HFA384X_PCI_M0_LEN_OFF);
	HFA384X_OUTW(HFA384X_PCI_CTL_TO_BAP, HFA384X_PCI_M0_CTL_OFF);
}
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */


/* Called only from software IRQ */
static int prism2_tx(struct sk_buff *skb, struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int res, idx = -1, ret = 1, data_len;
	struct hfa384x_tx_frame txdesc;
	u16 fc, ethertype = 0;
	prism2_wds_info_t *wds = NULL;
	enum { WDS_NO, WDS_OWN_FRAME, WDS_COMPLIANT_FRAME } use_wds = WDS_NO;
	struct net_device_stats *stats;
	u8 *wepbuf = NULL;
	int wepbuf_len = 0, host_encrypt = 0;
	struct prism2_crypt_data *crypt = NULL;
	void *sta = NULL;
	u8 *encaps_data;
	int encaps_len, skip_header_bytes;
	int no_encrypt = 0;
	int to_assoc_ap = 0;

	prism2_callback(local, PRISM2_CALLBACK_TX_START);
	stats = hostap_get_stats(dev);

	if ((local->func->card_present && !local->func->card_present(local)) ||
	    !local->hw_ready) {
		if (net_ratelimit())
			printk(KERN_DEBUG "%s: prism2_tx: hw not ready - "
			       "skipping\n", dev->name);
		ret = 0;
		goto fail;
	}

	if (skb->len < ETH_HLEN) {
		printk(KERN_DEBUG "%s: prism2_tx: short skb (len=%d)\n",
		       dev->name, skb->len);
		ret = 0;
		goto fail;
	}

	if (local->dev != dev) {
		wds = (prism2_wds_info_t *) dev;
		use_wds = (local->iw_mode == IW_MODE_MASTER &&
			   !(local->wds_type & HOSTAP_WDS_STANDARD_FRAME)) ?
			WDS_OWN_FRAME : WDS_COMPLIANT_FRAME;
		if (dev == local->stadev) {
			to_assoc_ap = 1;
			wds = NULL;
			use_wds = WDS_NO;
		}
	} else {
		if (local->iw_mode == IW_MODE_REPEAT) {
			printk(KERN_DEBUG "%s: prism2_tx: trying to use "
			       "non-WDS link in Repeater mode\n", dev->name);
			ret = 0;
			goto fail;
		} else if (local->iw_mode == IW_MODE_INFRA &&
			   (local->wds_type & HOSTAP_WDS_AP_CLIENT) &&
			   memcmp(skb->data + ETH_ALEN, dev->dev_addr,
				  ETH_ALEN) != 0) {
			/* AP client mode: send frames with foreign src addr
			 * using 4-addr WDS frames */
			use_wds = WDS_COMPLIANT_FRAME;
		}
	}

	if (local->host_encrypt) {
		/* Set crypt to default algorithm and key; will be replaced in
		 * AP code if STA has own alg/key */
		crypt = local->crypt;
		host_encrypt = 1;
	}

	if (skb->protocol == __constant_htons(ETH_P_HOSTAP)) {
		/* frame from prism2_send_mgmt() */
		if (skb->len < sizeof(txdesc)) {
			printk(KERN_DEBUG "%s: prism2_tx: short ETH_P_HOSTAP "
			       "skb\n", dev->name);
			ret = 0;
			goto fail;
		}
		memcpy(&txdesc, skb->data, sizeof(txdesc));
		skb_pull(skb, sizeof(txdesc));
		encaps_data = NULL;
		encaps_len = 0;
		skip_header_bytes = 0;
		data_len = skb->len;

		fc = le16_to_cpu(txdesc.frame_control);

		/* data frames use normal host encryption, if needed */
		if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_DATA)
			goto data_txdesc_set;

		/* mgmt/ctrl frames do not need further processing, so skip to
		 * frame transmit */
		goto frame_processing_done;
	}

	/* Incoming skb->data: dst_addr[6], src_addr[6], proto[2], payload
	 * ==>
	 * Prism2 TX frame with 802.11 header:
	 * txdesc (address order depending on used mode; includes dst_addr and
	 * src_addr), possible encapsulation (RFC1042/Bridge-Tunnel;
	 * proto[2], payload {, possible addr4[6]} */

	ethertype = (skb->data[12] << 8) | skb->data[13];

	memset(&txdesc, 0, sizeof(txdesc));

	txdesc.tx_control = __cpu_to_le16(local->tx_control);

	/* Length of data after txdesc */
	data_len = skb->len - ETH_HLEN;
	encaps_data = NULL;
	encaps_len = 0;
	skip_header_bytes = ETH_HLEN;
	if (ethertype == ETH_P_AARP || ethertype == ETH_P_IPX) {
		encaps_data = bridge_tunnel_header;
		encaps_len = sizeof(bridge_tunnel_header);
		data_len += encaps_len + 2;
		skip_header_bytes -= 2;
	} else if (ethertype >= 0x600) {
		encaps_data = rfc1042_header;
		encaps_len = sizeof(rfc1042_header);
		data_len += encaps_len + 2;
		skip_header_bytes -= 2;
	}

	fc = (WLAN_FC_TYPE_DATA << 2) | (WLAN_FC_STYPE_DATA << 4);
	if (use_wds != WDS_NO) {
		/* Note! Prism2 station firmware has problems with sending real
		 * 802.11 frames with four addresses; until these problems can
		 * be fixed or worked around, 4-addr frames needed for WDS are
		 * using incompatible format: FromDS flag is not set and the
		 * fourth address is added after the frame payload; it is
		 * assumed, that the receiving station knows how to handle this
		 * frame format */

		if (use_wds == WDS_COMPLIANT_FRAME) {
			fc |= WLAN_FC_FROMDS | WLAN_FC_TODS;
			/* From&To DS: Addr1 = RA, Addr2 = TA, Addr3 = DA,
			 * Addr4 = SA */
			memcpy(&txdesc.addr4, skb->data + ETH_ALEN, ETH_ALEN);
		} else {
			/* bogus 4-addr format to workaround Prism2 station
			 * f/w bug */
			fc |= WLAN_FC_TODS;
			/* From DS: Addr1 = DA (used as RA),
			 * Addr2 = BSSID (used as TA), Addr3 = SA (used as DA),
			 */

			/* SA from skb->data + ETH_ALEN will be added after
			 * frame payload */
			data_len += ETH_ALEN;
		}

		/* send broadcast and multicast frames to broadcast RA, if
		 * configured; otherwise, use unicast RA of the WDS link */
		if ((local->wds_type & HOSTAP_WDS_BROADCAST_RA) &&
		    skb->data[0] & 0x01)
			memset(&txdesc.addr1, 0xff, ETH_ALEN);
		else if (wds)
			memcpy(&txdesc.addr1, wds->remote_addr, ETH_ALEN);
		else
			memcpy(&txdesc.addr1, local->bssid, ETH_ALEN);
		memcpy(&txdesc.addr2, dev->dev_addr, ETH_ALEN);
		memcpy(&txdesc.addr3, skb->data, ETH_ALEN);

		memcpy(&txdesc.dst_addr, &txdesc.addr3, ETH_ALEN);
		memcpy(&txdesc.src_addr, &txdesc.addr2, ETH_ALEN);
	} else if (local->iw_mode == IW_MODE_MASTER && !to_assoc_ap) {
		fc |= WLAN_FC_FROMDS;
		/* From DS: Addr1 = DA, Addr2 = BSSID, Addr3 = SA */
		memcpy(&txdesc.addr1, skb->data, ETH_ALEN);
		/* FIX - addr2 replaced by f/w, so no need to fill it now(?) */
		memcpy(&txdesc.addr2, dev->dev_addr, ETH_ALEN);
		memcpy(&txdesc.addr3, skb->data + ETH_ALEN, ETH_ALEN);
	} else if (local->iw_mode == IW_MODE_INFRA || to_assoc_ap) {
		fc |= WLAN_FC_TODS;
		/* To DS: Addr1 = BSSID, Addr2 = SA, Addr3 = DA */
		memcpy(&txdesc.addr1, to_assoc_ap ?
		       local->assoc_ap_addr : local->bssid, ETH_ALEN);
		memcpy(&txdesc.addr2, skb->data + ETH_ALEN, ETH_ALEN);
		memcpy(&txdesc.addr3, skb->data, ETH_ALEN);
	} else if (local->iw_mode == IW_MODE_ADHOC) {
		/* not From/To DS: Addr1 = DA, Addr2 = SA, Addr3 = BSSID */
		memcpy(&txdesc.addr1, skb->data, ETH_ALEN);
		memcpy(&txdesc.addr2, skb->data + ETH_ALEN, ETH_ALEN);
		memcpy(&txdesc.addr3, local->bssid, ETH_ALEN);
	}

	txdesc.frame_control = __cpu_to_le16(fc);
	txdesc.data_len = __cpu_to_le16(data_len);
	txdesc.len = __cpu_to_be16(data_len);
	if (use_wds == WDS_NO)
		memcpy(&txdesc.dst_addr, skb->data, 2 * ETH_ALEN);

	skb->dev = dev;

 data_txdesc_set:
	if (to_assoc_ap)
		goto skip_ap_processing;

	switch (hostap_handle_sta_tx(local, skb, &txdesc, use_wds != WDS_NO,
				     host_encrypt, &crypt, &sta)) {
	case AP_TX_CONTINUE:
		break;
	case AP_TX_CONTINUE_NOT_AUTHORIZED:
		if (local->ieee_802_1x && ethertype != ETH_P_PAE &&
		    use_wds == WDS_NO) {
			printk(KERN_DEBUG "%s: dropped frame to unauthorized "
			       "port (IEEE 802.1X): ethertype=0x%04x\n",
			       dev->name, ethertype);
			hostap_dump_tx_header(dev->name, &txdesc);

			ret = 0; /* drop packet */
			stats->tx_dropped++;
			goto fail;
		}
		break;
	case AP_TX_DROP:
		ret = 0; /* drop packet */
		stats->tx_dropped++;
		goto fail;
	case AP_TX_RETRY:
		goto fail;
	case AP_TX_BUFFERED:
		/* do not free skb here, it will be freed when the
		 * buffered frame is sent/timed out */
		ret = 0;
		goto tx_exit;
	}

 skip_ap_processing:

	if (local->ieee_802_1x && ethertype == ETH_P_PAE) {
		if (crypt) {
			no_encrypt = 1;
			printk(KERN_DEBUG "%s: TX: IEEE 802.1X - passing "
			       "unencrypted EAPOL frame\n", dev->name);
		}
		crypt = NULL; /* no encryption for IEEE 802.1X frames */
	}

	if (crypt && (!crypt->ops || !crypt->ops->encrypt))
		crypt = NULL;
	else if ((crypt || local->crypt) && !no_encrypt) {
		/* Add ISWEP flag both for firmware and host based encryption
		 */
		fc |= WLAN_FC_ISWEP;
		txdesc.frame_control = cpu_to_le16(fc);
	}

	if (crypt) {
		/* Perform host driver -based encryption */
		u8 *pos;
		int olen;

		olen = data_len;
		data_len += crypt->ops->extra_prefix_len +
			crypt->ops->extra_postfix_len;
		txdesc.data_len = cpu_to_le16(data_len);
		txdesc.len = cpu_to_be16(data_len);

		wepbuf_len = data_len;
		wepbuf = (u8 *) kmalloc(wepbuf_len, GFP_ATOMIC);
		if (wepbuf == NULL) {
			printk(KERN_DEBUG "%s: could not allocate TX wepbuf\n",
			       dev->name);
			goto fail;
		}

		pos = wepbuf + crypt->ops->extra_prefix_len;
		if (encaps_len > 0) {
			memcpy(pos, encaps_data, encaps_len);
			pos += encaps_len;
		}
		memcpy(pos, skb->data + skip_header_bytes,
		       skb->len - skip_header_bytes);
		if (use_wds == WDS_OWN_FRAME) {
			memcpy(pos + skb->len - skip_header_bytes,
			       skb->data + ETH_ALEN, ETH_ALEN);
		}

		atomic_inc(&crypt->refcnt);
		olen = crypt->ops->encrypt(wepbuf, olen, crypt->priv);
		atomic_dec(&crypt->refcnt);
		if (olen > wepbuf_len) {
			printk(KERN_WARNING "%s: encrypt overwrote wepbuf "
			       "(%d > %d)\n", dev->name, olen, wepbuf_len);
		}
		if (olen < 0)
			goto fail;

		data_len = wepbuf_len = olen;
		txdesc.data_len = cpu_to_le16(data_len);
		txdesc.len = cpu_to_be16(data_len);
	}

 frame_processing_done:
	idx = prism2_get_txfid_idx(local);
	if (idx < 0)
		goto fail;

	if (local->frame_dump & PRISM2_DUMP_TX_HDR)
		hostap_dump_tx_header(dev->name, &txdesc);

	spin_lock(&local->baplock);
	res = hfa384x_setup_bap(dev, BAP0, local->txfid[idx], 0);

#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
	if (!res && skb->len >= local->bus_master_threshold_tx) {
		u8 *pos;
		int buf_len;

		local->bus_m0_tx_idx = idx;

		/* FIX: BAP0 should be locked during bus master transfer, but
		 * baplock with BH's disabled is not OK for this; netif queue
		 * stopping is not enough since BAP0 is used also for RID
		 * read/write */

		/* stop the queue for the time that bus mastering on BAP0 is
		 * in use */
		hostap_netif_stop_queues(dev);

		spin_unlock(&local->baplock);

		/* Copy frame data to bus_m0_buf */
		pos = local->bus_m0_buf;
		memcpy(pos, &txdesc, sizeof(txdesc));
		pos += sizeof(txdesc);

		if (!wepbuf) {
			if (encaps_len > 0) {
				memcpy(pos, encaps_data, encaps_len);
				pos += encaps_len;
			}
			memcpy(pos, skb->data + skip_header_bytes,
			       skb->len - skip_header_bytes);
			pos += skb->len - skip_header_bytes;
		}
		if (!wepbuf && use_wds == WDS_OWN_FRAME) {
			/* add addr4 (SA) to bogus frame format if WDS is used
			 */
			memcpy(pos, skb->data + ETH_ALEN, ETH_ALEN);
			pos += ETH_ALEN;
		}

		if (wepbuf) {
			memcpy(pos, wepbuf, wepbuf_len);
			pos += wepbuf_len;
		}

		buf_len = pos - local->bus_m0_buf;
		if (buf_len & 1)
			buf_len++;

#ifdef PRISM2_ENABLE_BEFORE_TX_BUS_MASTER
		/* Any RX packet seems to break something with TX bus
		 * mastering; enable command is enough to fix this.. */
		if (hfa384x_cmd_callback(dev, HFA384X_CMDCODE_ENABLE, 0,
					 prism2_tx_cb, (void *) buf_len)) {
			printk(KERN_DEBUG "%s: TX: enable port0 failed\n",
			       dev->name);
		}
#else /* PRISM2_ENABLE_BEFORE_TX_BUS_MASTER */
		prism2_tx_cb(dev, (void *) buf_len, 0, 0);
#endif /* PRISM2_ENABLE_BEFORE_TX_BUS_MASTER */

		/* Bus master transfer will be started from command completion
		 * event handler and TX handling will be finished by calling
		 * prism2_transmit() from bus master event handler */
		goto tx_stats;
	}
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */

	if (!res)
		res = hfa384x_to_bap(dev, BAP0, &txdesc, sizeof(txdesc));
	if (!res && !wepbuf && encaps_len > 0)
		res = hfa384x_to_bap(dev, BAP0, encaps_data, encaps_len);
	if (!res && !wepbuf && use_wds != WDS_OWN_FRAME)
		res = hfa384x_to_bap(dev, BAP0, skb->data + skip_header_bytes,
				     skb->len - skip_header_bytes);
	else if (!res && !wepbuf && use_wds == WDS_OWN_FRAME) {
		int wlen, is_odd;

		wlen = skb->len - skip_header_bytes;
		is_odd = wlen & 1;

		if (is_odd)
			wlen--; /* need to avoid using odd offset */

		res = hfa384x_to_bap(dev, BAP0, skb->data + skip_header_bytes,
				     wlen);

		/* add addr4 (SA) to bogus frame format if WDS is used */
		if (!res && is_odd) {
			char tmpbuf[ETH_ALEN + 1];
			tmpbuf[0] = *(skb->data + skb->len - 1);
			memcpy(tmpbuf + 1, skb->data + ETH_ALEN, ETH_ALEN);
			res = hfa384x_to_bap(dev, BAP0, tmpbuf, ETH_ALEN + 1);
		} else if (!res) {
			res = hfa384x_to_bap(dev, BAP0, skb->data + ETH_ALEN,
					     ETH_ALEN);
		}
	}

	if (!res && wepbuf)
		res = hfa384x_to_bap(dev, BAP0, wepbuf, wepbuf_len);
	spin_unlock(&local->baplock);

	if (!res)
		res = prism2_transmit(dev, idx);
	if (res) {
		printk(KERN_DEBUG "%s: prism2_tx - to BAP0 failed\n",
		       dev->name);
		local->intransmitfid[idx] = PRISM2_TXFID_EMPTY;
		PRISM2_SCHEDULE_TASK(&local->reset_queue);
		ret = 0; /* do not retry failed frames to avoid problems */
		goto fail;
	}

#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
 tx_stats:
#endif
	stats->tx_packets++;
	stats->tx_bytes += data_len + 36;

	ret = 0;

 fail:
	if (wepbuf)
		kfree(wepbuf);

	if (!ret)
		dev_kfree_skb(skb);

 tx_exit:
	if (sta)
		hostap_handle_sta_release(sta);

	prism2_callback(local, PRISM2_CALLBACK_TX_END);
	return ret;
}


/* Called only from software IRQ */
static int prism2_tx_80211(struct sk_buff *skb, struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct hfa384x_tx_frame txdesc;
	int hdr_len, data_len, ret = 1, idx, res;
	u16 fc, tx_control;

	if ((local->func->card_present && !local->func->card_present(local)) ||
	    !local->hw_ready) {
		printk(KERN_DEBUG "%s: prism2_tx_80211: hw not ready - "
		       "skipping\n", dev->name);
		ret = 0;
		local->apdevstats.tx_dropped++;
		goto fail;
	}

	if (skb->len < 24) {
		printk(KERN_DEBUG "%s: prism2_tx_80211: short skb (len=%d)\n",
		       dev->name, skb->len);
		ret = 0;
		local->apdevstats.tx_dropped++;
		goto fail;
	}

	memset(&txdesc, 0, sizeof(txdesc));
	/* txdesc.tx_rate might need to be set if f/w does not select suitable
	 * TX rate */

	/* skb->data starts with txdesc->frame_control */
	hdr_len = 24;
	memcpy(&txdesc.frame_control, skb->data, hdr_len);
 	fc = le16_to_cpu(txdesc.frame_control);
	if (WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_DATA &&
	    (fc & WLAN_FC_FROMDS) && (fc & WLAN_FC_TODS) && skb->len >= 30) {
		/* Addr4 */
		memcpy(txdesc.addr4, skb->data + hdr_len, ETH_ALEN);
		hdr_len += ETH_ALEN;
	}

	tx_control = local->tx_control;
	/* Request TX callback if protocol version is 2 in 802.11 header;
	 * this version 2 is a special case used between hostapd and kernel
	 * driver */
	if (((fc & WLAN_FC_PVER) == BIT(1)) &&
	    local->ap && local->ap->tx_callback_idx) {
		tx_control |= HFA384X_TX_CTRL_TX_OK;
		txdesc.sw_support = cpu_to_le16(local->ap->tx_callback_idx);

		/* remove special version from the frame header */
		fc &= ~WLAN_FC_PVER;
		txdesc.frame_control = cpu_to_le16(fc);
	}
	txdesc.tx_control = cpu_to_le16(tx_control);
	
	data_len = skb->len - hdr_len;
	txdesc.data_len = __cpu_to_le16(data_len);
	txdesc.len = __cpu_to_be16(data_len);

	idx = prism2_get_txfid_idx(local);
	if (idx < 0) {
		local->apdevstats.tx_dropped++;
		goto fail;
	}

	spin_lock(&local->baplock);
	res = hfa384x_setup_bap(dev, BAP0, local->txfid[idx], 0);
	if (!res)
		res = hfa384x_to_bap(dev, BAP0, &txdesc, sizeof(txdesc));
	if (!res)
		res = hfa384x_to_bap(dev, BAP0, skb->data + hdr_len,
				     skb->len - hdr_len);
	spin_unlock(&local->baplock);

	if (!res)
		res = prism2_transmit(dev, idx);
	if (res) {
		printk(KERN_DEBUG "%s: prism2_tx_80211 - to BAP0 failed\n",
		       dev->name);
		local->intransmitfid[idx] = PRISM2_TXFID_EMPTY;
		PRISM2_SCHEDULE_TASK(&local->reset_queue);
		local->apdevstats.tx_dropped++;
		ret = 0;
		goto fail;
	}

	ret = 0;

	local->apdevstats.tx_packets++;
	local->apdevstats.tx_bytes += skb->len;

 fail:
	if (!ret)
		dev_kfree_skb(skb);

	return ret;
}


/* Send RX frame to netif with 802.11 (and possible prism) header.
 * Called from hardware or software IRQ context. */
static int prism2_rx_80211(struct net_device *dev, struct sk_buff *skb,
			   int type, char *extra, int extra_len)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int hdrlen, phdrlen, head_need, tail_need;
	u16 len, fc;
	int prism_header;
	struct hfa384x_rx_frame rxdesc;

	dev->last_rx = jiffies;

	if (skb->len < sizeof(rxdesc)) {
		printk(KERN_DEBUG "%s: prism2_rx_80211() called with too "
		       "short skb (len=%d)\n", dev->name, skb->len);
		dev_kfree_skb_any(skb);
		return 0;
	}

	/* make a local copy of rxdesc, since we will be changing the head of
	 * the skb data */
	memcpy(&rxdesc, skb->data, sizeof(rxdesc));
	skb_pull(skb, sizeof(rxdesc));

	if (dev->type == ARPHRD_IEEE80211_PRISM) {
		if (local->monitor_type == PRISM2_MONITOR_PRISM) {
			prism_header = 1;
			phdrlen = sizeof(struct linux_wlan_ng_prism_hdr);
		} else { /* local->monitor_type == PRISM2_MONITOR_CAPHDR */
			prism_header = 2;
			phdrlen = sizeof(struct linux_wlan_ng_cap_hdr);
		}
	} else {
		prism_header = 0;
		phdrlen = 0;
	}

	fc = le16_to_cpu(rxdesc.frame_control);

	if (type == PRISM2_RX_MGMT && (fc & WLAN_FC_PVER)) {
		printk(KERN_DEBUG "%s: dropped management frame with header "
		       "version %d\n", dev->name, fc & WLAN_FC_PVER);
		dev_kfree_skb_any(skb);
		return 0;
	}

	hdrlen = hostap_80211_get_hdrlen(fc);

	if (extra) {
		/* set 802.11 protocol version to 3 to indicate extra data
		 * after the payload */
		fc |= WLAN_FC_PVER;
		rxdesc.frame_control = cpu_to_le16(fc);
	}

	len = le16_to_cpu(rxdesc.data_len);
	/* Monitor mode reports different data_len values for packets. For
	 * example ctrl::ack frames may get values like -14 to 92 when heard
	 * from neighboring channel. Start of the frames seems to be OK anyway,
	 * so pass them through. Set signed negative values to zero so that
	 * hdrlen is used to get the proper length of data. */
	if (type == PRISM2_RX_MONITOR && len >= (u16) -14 && len != 0xffff)
		len = 0;
	if (len > PRISM2_DATA_MAXLEN || extra_len > 65535) {
		printk(KERN_WARNING "%s: prism2_rx_80211: len(%d) > "
		       "MAX(%d)\n", dev->name, len, PRISM2_DATA_MAXLEN);
		dev_kfree_skb_any(skb);
		return 0;
	}

	/* check if there is enough room for extra data; if not, expand skb
	 * buffer to be large enough for the changes */
	head_need = phdrlen + hdrlen;
	tail_need = extra_len;
	if (extra_len)
		tail_need += 2;
#ifdef PRISM2_ADD_BOGUS_CRC
	tail_need += 4;
#endif /* PRISM2_ADD_BOGUS_CRC */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
	if (head_need > skb_headroom(skb) || tail_need > skb_tailroom(skb)) {
		struct sk_buff *nskb;

		nskb = dev_alloc_skb(skb->len + head_need + tail_need);
		if (nskb == NULL) {
			dev_kfree_skb_any(skb);
			return 0;
		}

		skb_reserve(nskb, head_need);
		memcpy(skb_put(nskb, skb->len), skb->data, skb->len);
		dev_kfree_skb_any(skb);
		skb = nskb;
	}
#else /* Linux 2.4.0 or newer */
	head_need -= skb_headroom(skb);
	tail_need -= skb_tailroom(skb);

	if (head_need > 0 || tail_need > 0) {
		if (pskb_expand_head(skb, head_need > 0 ? head_need : 0,
				     tail_need > 0 ? tail_need : 0,
				     GFP_ATOMIC)) {
			printk(KERN_DEBUG "%s: prism2_rx_80211 failed to "
			       "reallocate skb buffer\n", dev->name);
			dev_kfree_skb_any(skb);
			return 0;
		}
	}
#endif

	/* We now have an skb with enough head and tail room, so just insert
	 * the extra data */
	memcpy(skb_push(skb, hdrlen), &rxdesc.frame_control, hdrlen);

	if (prism_header == 1) {
		struct linux_wlan_ng_prism_hdr *hdr;
		hdr = (struct linux_wlan_ng_prism_hdr *)
			skb_push(skb, phdrlen);
		memset(hdr, 0, phdrlen);
		hdr->msgcode = LWNG_CAP_DID_BASE;
		hdr->msglen = sizeof(*hdr);
		memcpy(hdr->devname, dev->name, sizeof(hdr->devname));
#define LWNG_SETVAL(f,i,s,l,d) \
hdr->f.did = LWNG_CAP_DID_BASE | (i << 12); \
hdr->f.status = s; hdr->f.len = l; hdr->f.data = d
		LWNG_SETVAL(hosttime, 1, 0, 4, jiffies);
		LWNG_SETVAL(mactime, 2, 0, 0, le32_to_cpu(rxdesc.time));
		LWNG_SETVAL(channel, 3, 1 /* no value */, 4, 0);
		LWNG_SETVAL(rssi, 4, 1 /* no value */, 4, 0);
		LWNG_SETVAL(sq, 5, 1 /* no value */, 4, 0);
		LWNG_SETVAL(signal, 6, 0, 4, rxdesc.signal);
		LWNG_SETVAL(noise, 7, 0, 4, rxdesc.silence);
		LWNG_SETVAL(rate, 8, 0, 4, rxdesc.rate / 5);
		LWNG_SETVAL(istx, 9, 0, 4, 0);
#ifdef PRISM2_ADD_BOGUS_CRC
		LWNG_SETVAL(frmlen, 10, 0, 4, hdrlen + len + 4 /* CRC */);
#else /* PRISM2_ADD_BOGUS_CRC */
		LWNG_SETVAL(frmlen, 10, 0, 4, hdrlen + len);
#endif /* PRISM2_ADD_BOGUS_CRC */
#undef LWNG_SETVAL
	} else if (prism_header == 2) {
		struct linux_wlan_ng_cap_hdr *hdr;
		hdr = (struct linux_wlan_ng_cap_hdr *)
			skb_push(skb, phdrlen);
		memset(hdr, 0, phdrlen);
		hdr->version    = htonl(LWNG_CAPHDR_VERSION);
		hdr->length     = htonl(phdrlen);
		hdr->mactime    = __cpu_to_be64(rxdesc.time);
		hdr->hosttime   = __cpu_to_be64(jiffies);
		hdr->phytype    = htonl(4); /* dss_dot11_b */
		hdr->channel    = htonl(local->channel);
		hdr->datarate   = htonl(rxdesc.rate);
		hdr->antenna    = htonl(0); /* unknown */
		hdr->priority   = htonl(0); /* unknown */
		hdr->ssi_type   = htonl(3); /* raw */
		hdr->ssi_signal = htonl(rxdesc.signal);
		hdr->ssi_noise  = htonl(rxdesc.silence);
		hdr->preamble   = htonl(0); /* unknown */
		hdr->encoding   = htonl(1); /* cck */
	}

#ifdef PRISM2_ADD_BOGUS_CRC
	memset(skb_put(skb, 4), 0xff, 4); /* Prism2 strips CRC */
#endif /* PRISM2_ADD_BOGUS_CRC */

	/* TODO: could add 'type' information into the end of the data if it
	 * is needed in the user space daemon */
	if (extra != NULL) {
		u16 *elen;
		memcpy(skb_put(skb, extra_len), extra, extra_len);
		elen = (u16 *) skb_put(skb, 2);
		*elen = __cpu_to_le16(extra_len);
	}

	skb->dev = dev;
	skb->mac.raw = skb->data;
	skb_pull(skb, hdrlen);
	if (prism_header)
		skb_pull(skb, phdrlen);
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = __constant_htons(ETH_P_802_2);
	memset(skb->cb, 0, sizeof(skb->cb));
	netif_rx(skb);

	return (hdrlen + len);
}


/* Called only as a tasklet (software IRQ) */
static void monitor_rx(struct net_device *dev, struct sk_buff *skb)
{
	local_info_t *local = (local_info_t *) dev->priv;
	struct net_device_stats *stats;
	int len;
	struct hfa384x_rx_frame *rxdesc =
		(struct hfa384x_rx_frame *) skb->data;

	if (le16_to_cpu(rxdesc->status) & HFA384X_RX_STATUS_FCSERR &&
	    !local->monitor_allow_fcserr) {
		dev_kfree_skb(skb);
		return;
	}

	len = prism2_rx_80211(dev, skb, PRISM2_RX_MONITOR, NULL, 0);
	stats = hostap_get_stats(dev);
	stats->rx_packets++;
	stats->rx_bytes += len;
}


/* Called only as a tasklet (software IRQ) */
static struct prism2_frag_entry *
prism2_frag_cache_find(local_info_t *local, unsigned int seq,
		       unsigned int frag, u8 *src, u8 *dst)
{
	struct prism2_frag_entry *entry;
	int i;

	for (i = 0; i < PRISM2_FRAG_CACHE_LEN; i++) {
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
static struct sk_buff *prism2_frag_cache_get(local_info_t *local,
					     struct hfa384x_rx_frame *rxdesc)
{
	struct sk_buff *skb = NULL;
	u16 sc;
	unsigned int frag, seq;
	struct prism2_frag_entry *entry;

	sc = le16_to_cpu(rxdesc->seq_ctrl);
	frag = WLAN_GET_SEQ_FRAG(sc);
	seq = WLAN_GET_SEQ_SEQ(sc);

	if (frag == 0) {
		/* Reserve enough space to fit maximum frame length */
		skb = dev_alloc_skb(local->dev->mtu +
				    sizeof(struct hfa384x_rx_frame) +
				    8 /* LLC */ +
				    2 /* alignment */ +
				    8 /* WEP */ + ETH_ALEN /* WDS */);
		if (skb == NULL)
			return NULL;

		entry = &local->frag_cache[local->frag_next_idx];
		local->frag_next_idx++;
		if (local->frag_next_idx >= PRISM2_FRAG_CACHE_LEN)
			local->frag_next_idx = 0;

		if (entry->skb != NULL)
			dev_kfree_skb(entry->skb);

		entry->first_frag_time = jiffies;
		entry->seq = seq;
		entry->last_frag = frag;
		entry->skb = skb;
		memcpy(entry->src_addr, rxdesc->addr2, ETH_ALEN);
		memcpy(entry->dst_addr, rxdesc->addr1, ETH_ALEN);
	} else {
		/* received a fragment of a frame for which the head fragment
		 * should have already been received */
		entry = prism2_frag_cache_find(local, seq, frag, rxdesc->addr2,
					       rxdesc->addr1);
		if (entry != NULL) {
			entry->last_frag = frag;
			skb = entry->skb;
		}
	}

	return skb;
}


/* Called only as a tasklet (software IRQ) */
static int prism2_frag_cache_invalidate(local_info_t *local,
					struct hfa384x_rx_frame *rxdesc)
{
	u16 sc;
	unsigned int seq;
	struct prism2_frag_entry *entry;

	sc = le16_to_cpu(rxdesc->seq_ctrl);
	seq = WLAN_GET_SEQ_SEQ(sc);

	entry = prism2_frag_cache_find(local, seq, -1, rxdesc->addr2,
				       rxdesc->addr1);

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
static inline prism2_wds_info_t *prism2_rx_get_wds(local_info_t *local,
						   u8 *addr)
{
	prism2_wds_info_t *wds;

	spin_lock(&local->wdslock);
	wds = local->wds;
	while (wds != NULL && memcmp(wds->remote_addr, addr, ETH_ALEN) != 0)
		wds = wds->next;
	spin_unlock(&local->wdslock);

	return wds;
}


/* Called only as a tasklet (software IRQ) */
static void prism2_rx(local_info_t *local)
{
	struct net_device *dev = local->dev;
	int res, rx_pending = 0;
	u16 len, rxfid, status, macport;
	struct net_device_stats *stats;
	struct hfa384x_rx_frame rxdesc;
	struct sk_buff *skb = NULL;

	prism2_callback(local, PRISM2_CALLBACK_RX_START);
	stats = hostap_get_stats(dev);

	rxfid = HFA384X_INW(HFA384X_RXFID_OFF);
#ifndef final_version
	if (rxfid == 0) {
		rxfid = HFA384X_INW(HFA384X_RXFID_OFF);
		printk(KERN_DEBUG "prism2_rx: rxfid=0 (next 0x%04x)\n",
		       rxfid);
		PRISM2_SCHEDULE_TASK(&local->reset_queue);
		goto rx_dropped;
	}
#endif

	spin_lock(&local->baplock);
	res = hfa384x_setup_bap(dev, BAP0, rxfid, 0);
	if (!res)
		res = hfa384x_from_bap(dev, BAP0, &rxdesc, sizeof(rxdesc));

	if (res) {
		spin_unlock(&local->baplock);
		printk(KERN_DEBUG "%s: copy from BAP0 failed %d\n", dev->name,
		       res);
		if (res == -ETIMEDOUT) {
			PRISM2_SCHEDULE_TASK(&local->reset_queue);
		}
		goto rx_dropped;
	}

	len = le16_to_cpu(rxdesc.data_len);
	status = le16_to_cpu(rxdesc.status);
	macport = (status >> 8) & 0x07;

	/* Drop frames with too large reported payload length. Monitor mode
	 * seems to sometimes pass frames (e.g., ctrl::ack) with signed and
	 * negative value, so allow also values 65522 .. 65534 (-14 .. -2) for
	 * macport 7 */
	if ((len & 0x8000) &&
	    (macport != 7 || ((len < (u16) -14) && len != 0xffff))) {
		spin_unlock(&local->baplock);
		printk(KERN_DEBUG "%s: Received frame with invalid length "
		       "0x%04x\n", dev->name, len);
		hostap_dump_rx_header(dev->name, &rxdesc);
		goto rx_dropped;
	}

	skb = dev_alloc_skb(sizeof(rxdesc) + len);
	if (!skb) {
		spin_unlock(&local->baplock);
		printk(KERN_DEBUG "%s: RX failed to allocate skb\n",
		       dev->name);
		goto rx_dropped;
	}
	skb->dev = dev;
	memcpy(skb_put(skb, sizeof(rxdesc)), &rxdesc, sizeof(rxdesc));

#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
	if (len >= local->bus_master_threshold_rx) {
		unsigned long addr;

		hfa384x_events_no_bap1(dev);

		local->rx_skb = skb;
		/* Internal BAP0 offset points to the byte following rxdesc;
		 * copy rest of the data using bus master */
		addr = virt_to_phys(skb_put(skb, len));
		HFA384X_OUTW((addr & 0xffff0000) >> 16,
			     HFA384X_PCI_M0_ADDRH_OFF);
		HFA384X_OUTW(addr & 0x0000ffff, HFA384X_PCI_M0_ADDRL_OFF);
		if (len & 1)
			len++;
		HFA384X_OUTW(len / 2, HFA384X_PCI_M0_LEN_OFF);
		HFA384X_OUTW(HFA384X_PCI_CTL_FROM_BAP, HFA384X_PCI_M0_CTL_OFF);

		/* pci_bus_m1 event will be generated when data transfer is
		 * complete and the frame will then be added to rx_list and
		 * rx_tasklet is scheduled */
		rx_pending = 1;

		/* Have to release baplock before returning, although BAP0
		 * should really not be used before DMA transfer has been
		 * completed. */
		spin_unlock(&local->baplock);
	} else
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */
	{
		res = hfa384x_from_bap(dev, BAP0, skb_put(skb, len), len);
		spin_unlock(&local->baplock);
		if (res) {
			printk(KERN_DEBUG "%s: RX failed to read "
			       "frame data\n", dev->name);
			goto rx_dropped;
		}

		skb_queue_tail(&local->rx_list, skb);
		HOSTAP_TASKLET_SCHEDULE(&local->rx_tasklet);
	}

 rx_exit:
	prism2_callback(local, PRISM2_CALLBACK_RX_END);
	if (!rx_pending) {
		HFA384X_OUTW(HFA384X_EV_RX, HFA384X_EVACK_OFF);
	}

	return;

 rx_dropped:
	stats->rx_dropped++;
	if (skb)
		dev_kfree_skb(skb);
	goto rx_exit;
}


static inline int
hostap_rx_frame_invalid(struct net_device *dev, u16 macport, u16 status,
			u16 len)
{
	if (macport != 0) {
		printk(KERN_DEBUG "%s: RX: unknown MACPort %d\n",
		       dev->name, macport);
		return -1;
	}

	/* FCS errors should not come this far, but let's make sure that frames
	 * with errors will be dropped even in Host AP mode */
	if (status & HFA384X_RX_STATUS_FCSERR) {
		printk(KERN_DEBUG "%s: RX: dropped FCSErr frame "
		       "(status=%02X)\n", dev->name, status);
		return -1;
	}

	if (len > PRISM2_DATA_MAXLEN) {
		printk(KERN_DEBUG "%s: RX: len(%d) > MAX(%d)\n",
		       dev->name, len, PRISM2_DATA_MAXLEN);
		return -1;
	}

	return 0;
}


static inline int
hostap_rx_frame_invalid_data(struct net_device *dev, u16 msg_type, u16 type,
			     u16 stype)
{
	if (msg_type != HFA384X_RX_MSGTYPE_NORMAL &&
	    msg_type != HFA384X_RX_MSGTYPE_RFC1042 &&
	    msg_type != HFA384X_RX_MSGTYPE_BRIDGETUNNEL) {
		printk(KERN_DEBUG "%s: RX: dropped frame "
		       "(msg_type=%d)\n", dev->name, msg_type);
		return -1;
	}

	if (type != WLAN_FC_TYPE_DATA) {
		printk(KERN_DEBUG "%s: RX: dropped non-data frame "
		       "(type=0x%02x, subtype=0x%02x)\n",
		       dev->name, type, stype);
		return -1;
	}


	return 0;
}


static inline int
hostap_rx_frame_mgmt(local_info_t *local, struct sk_buff *skb, u16 type,
		     u16 stype)
{
	if (local->iw_mode == IW_MODE_MASTER) {
		struct hfa384x_rx_frame *rxdesc;
		rxdesc = (struct hfa384x_rx_frame *) skb->data;
		hostap_update_sta_ps(local, (struct hostap_ieee80211_hdr *)
				     &rxdesc->frame_control);
	}

	if (local->hostapd && type == WLAN_FC_TYPE_MGMT) {
		/* send management frames to the user space daemon for
		 * processing */
		local->apdevstats.rx_packets++;
		local->apdevstats.rx_bytes += skb->len;
		prism2_rx_80211(local->apdev, skb, PRISM2_RX_MGMT, NULL, 0);
		return 0;
	}

	if (local->iw_mode == IW_MODE_MASTER) {
		if (type != WLAN_FC_TYPE_MGMT && type != WLAN_FC_TYPE_CTRL) {
			printk(KERN_DEBUG "%s: unknown management frame "
			       "(type=0x%02x, stype=0x%02x) dropped\n",
			       skb->dev->name, type, stype);
			return -1;
		}

		skb->protocol = __constant_htons(ETH_P_HOSTAP);
		hostap_rx(skb->dev, skb);
		return 0;
	} else {
		printk(KERN_DEBUG "%s: prism2_rx: management frame "
		       "received in non-Host AP mode\n", skb->dev->name);
		return -1;
	}
}


static inline int
hostap_rx_frame_wds(local_info_t *local, struct hfa384x_rx_frame *rxdesc,
		    u16 fc, prism2_wds_info_t **wds)
{
	if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) !=
	    (WLAN_FC_TODS | WLAN_FC_FROMDS) &&
	    (!local->wds || local->iw_mode != IW_MODE_MASTER ||
	     !(fc & WLAN_FC_TODS)))
		return 0; /* not a WDS frame */

	/* Possible WDS frame: either IEEE 802.11 compliant (if FromDS)
	 * or own non-standard frame with 4th address after payload */
	if (memcmp(rxdesc->addr1, local->dev->dev_addr, ETH_ALEN) != 0 &&
	    (rxdesc->addr1[0] != 0xff || rxdesc->addr1[1] != 0xff ||
	     rxdesc->addr1[2] != 0xff || rxdesc->addr1[3] != 0xff ||
	     rxdesc->addr1[4] != 0xff || rxdesc->addr1[5] != 0xff)) {
		/* RA (or BSSID) is not ours - drop */
		PDEBUG(DEBUG_EXTRA, "%s: received WDS frame with "
		       "not own or broadcast %s=" MACSTR "\n",
		       local->dev->name, fc & WLAN_FC_FROMDS ? "RA" : "BSSID",
		       MAC2STR(rxdesc->addr1));
		return -1;
	}

	/* check if the frame came from a registered WDS connection */
	*wds = prism2_rx_get_wds(local, rxdesc->addr2);
	if (*wds == NULL && fc & WLAN_FC_FROMDS &&
	    (local->iw_mode != IW_MODE_INFRA ||
	     !(local->wds_type & HOSTAP_WDS_AP_CLIENT) ||
	     memcmp(rxdesc->addr2, local->bssid, ETH_ALEN) != 0)) {
		/* require that WDS link has been registered with TA or the
		 * frame is from current AP when using 'AP client mode' */
		PDEBUG(DEBUG_EXTRA, "%s: received WDS[4 addr] frame "
		       "from unknown TA=" MACSTR "\n",
		       local->dev->name, MAC2STR(rxdesc->addr2));
		if (local->ap && local->ap->autom_ap_wds)
			hostap_add_wds_link(local, rxdesc->addr2);
		return -1;
	}

	if (*wds && !(fc & WLAN_FC_FROMDS) && local->ap &&
	    hostap_is_sta_assoc(local->ap, rxdesc->addr2)) {
		/* STA is actually associated with us even though it has a
		 * registered WDS link. Assume it is in 'AP client' mode.
		 * Since this is a 3-addr frame, assume it is not (bogus) WDS
		 * frame and process it like any normal ToDS frame from
		 * associated STA. */
		*wds = NULL;
	}

	return 0;
}


static int hostap_is_eapol_frame(local_info_t *local,
				 struct hfa384x_rx_frame *rxdesc, u8 *buf,
				 int len)
{
	struct net_device *dev = local->dev;
	u16 fc, ethertype;

	fc = le16_to_cpu(rxdesc->frame_control);

	/* check that the frame is unicast frame to us */
	if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) == WLAN_FC_TODS &&
	    memcmp(rxdesc->addr1, dev->dev_addr, ETH_ALEN) == 0 &&
	    memcmp(rxdesc->addr3, dev->dev_addr, ETH_ALEN) == 0) {
		/* ToDS frame with own addr BSSID and DA */
	} else if ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) == WLAN_FC_FROMDS &&
		   memcmp(rxdesc->addr1, dev->dev_addr, ETH_ALEN) == 0) {
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
hostap_rx_frame_decrypt(local_info_t *local, int iswep, struct sk_buff *skb)
{
	struct hfa384x_rx_frame *rxdesc;
	struct prism2_crypt_data *crypt;
	void *sta = NULL;
	int ret = 0, olen, len;
	char *payload;

	rxdesc = (struct hfa384x_rx_frame *) skb->data;
	len = skb->len - sizeof(*rxdesc);
	payload = (char *) (rxdesc + 1);
	crypt = local->crypt;
	sta = NULL;

	/* Use station specific key to override default keys if the receiver
	 * address is a unicast address ("individual RA"). If bcrx_sta_key
	 * parameter is set, station specific key is used even with
	 * broad/multicast targets (this is against IEEE 802.11, but makes it
	 * easier to use different keys with stations that do not support WEP
	 * key mapping). */
	if (!(rxdesc->addr1[0] & 0x01) || local->bcrx_sta_key)
		(void) hostap_handle_sta_crypto(local, rxdesc, &crypt, &sta);

	/* allow NULL decrypt to indicate an station specific override for
	 * default encryption */
	if (crypt && (crypt->ops == NULL || crypt->ops->decrypt == NULL))
		crypt = NULL;

	if (!crypt && iswep) {
		printk(KERN_DEBUG "%s: WEP decryption failed (not set) (SA="
		       MACSTR ")\n", local->dev->name, MAC2STR(rxdesc->addr2));
		local->comm_tallies.rx_discards_wep_undecryptable++;
		ret = -1;
		goto done;
	}

	if (!crypt)
		goto done;

	if (!iswep && !local->open_wep) {
		if (local->ieee_802_1x &&
		    hostap_is_eapol_frame(local, rxdesc, payload, len)) {
			/* pass unencrypted EAPOL frames even if encryption is
			 * configured */
			printk(KERN_DEBUG "%s: RX: IEEE 802.1X - passing "
			       "unencrypted EAPOL frame\n", local->dev->name);
			goto done;
		}
		printk(KERN_DEBUG "%s: encryption configured, but RX frame "
		       "not encrypted (SA=" MACSTR ")\n",
		       local->dev->name, MAC2STR(rxdesc->addr2));
		ret = -1;
		goto done;
	}

	/* decrypt WEP part of the frame: IV (4 bytes), encrypted
	 * payload (including SNAP header), ICV (4 bytes) */
	atomic_inc(&crypt->refcnt);
	olen = crypt->ops->decrypt(payload, len, crypt->priv);
	atomic_dec(&crypt->refcnt);
	if (olen < 0) {
		printk(KERN_DEBUG "%s: WEP decryption failed (SA=" MACSTR
		       ")\n", local->dev->name, MAC2STR(rxdesc->addr2));
		local->comm_tallies.rx_discards_wep_undecryptable++;
		ret = -1;
		goto done;
	}

	skb_trim(skb, skb->len - (len - olen));

 done:
	if (sta)
		hostap_handle_sta_release(sta);

	return ret;
}


/* Called only as a tasklet (software IRQ) */
static void hostap_rx_skb(local_info_t *local, struct sk_buff *skb)
{
	u16 status, macport, msg_type, fc, type, stype, sc;
	prism2_wds_info_t *wds = NULL;
	struct hfa384x_rx_frame *rxdesc;
	struct net_device *dev = skb->dev;
	struct net_device_stats *stats;
	unsigned int frag;
	u8 *payload;
	struct sk_buff *skb2 = NULL;
	u16 ethertype;
	int frame_authorized = 0;
	int from_assoc_ap = 0;

	stats = hostap_get_stats(dev);
	rxdesc = (struct hfa384x_rx_frame *) skb->data;
	status = le16_to_cpu(rxdesc->status);
	macport = HFA384X_RX_STATUS_GET_MACPORT(status);
	msg_type = HFA384X_RX_STATUS_GET_MSGTYPE(status);
	fc = le16_to_cpu(rxdesc->frame_control);
	type = WLAN_FC_GET_TYPE(fc);
	stype = WLAN_FC_GET_STYPE(fc);
	sc = le16_to_cpu(rxdesc->seq_ctrl);
	frag = WLAN_GET_SEQ_FRAG(sc);

	if (local->frame_dump & PRISM2_DUMP_RX_HDR)
		hostap_dump_rx_header(dev->name, rxdesc);

#if WIRELESS_EXT > 15
	/* Put this code here so that we avoid duplicating it in all
	 * Rx paths. - Jean II */
#ifdef IW_WIRELESS_SPY		/* defined in iw_handler.h */
	/* If spy monitoring on */
	if (local->spy_data.spy_number > 0) {
		struct iw_quality wstats;
		wstats.level = HFA384X_RSSI_LEVEL_TO_dBm(rxdesc->signal);
		wstats.noise = HFA384X_RSSI_LEVEL_TO_dBm(rxdesc->silence);
		wstats.updated = 6;	/* No qual value */
		/* Update spy records */
		wireless_spy_update(dev, rxdesc->addr2, &wstats);
	}
#endif /* IW_WIRELESS_SPY */
#endif /* WIRELESS_EXT > 15 */

	if (macport == 7) {
		monitor_rx(dev, skb);
		return;
	}

	if (hostap_rx_frame_invalid(dev, macport, status, skb->len -
				    sizeof(struct hfa384x_rx_frame)))
		goto rx_dropped;

	if (msg_type == HFA384X_RX_MSGTYPE_MGMT || type != WLAN_FC_TYPE_DATA) {
		if (type == WLAN_FC_TYPE_MGMT && stype == WLAN_FC_STYPE_AUTH &&
		    fc & WLAN_FC_ISWEP && local->host_decrypt &&
		    hostap_rx_frame_decrypt(local, fc & WLAN_FC_ISWEP, skb)) {
			printk(KERN_DEBUG "%s: failed to decrypt mgmt::auth "
			       "from " MACSTR "\n", dev->name,
			       MAC2STR(rxdesc->addr2));
			/* TODO: could inform hostapd about this so that it
			 * could send auth failure report */
			goto rx_dropped;
		}

		if (hostap_rx_frame_mgmt(local, skb, type, stype))
			goto rx_dropped;
		else
			goto rx_exit;
	}

	if (hostap_rx_frame_invalid_data(dev, msg_type, type, stype))
		goto rx_dropped;

	if (hostap_rx_frame_wds(local, rxdesc, fc, &wds))
		goto rx_dropped;
	if (wds) {
		skb->dev = dev = &wds->dev;
		stats = hostap_get_stats(dev);
	}

	if (local->iw_mode == IW_MODE_MASTER && !wds &&
	    (fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) == WLAN_FC_FROMDS &&
	    local->stadev &&
	    memcmp(rxdesc->addr2, local->assoc_ap_addr, ETH_ALEN) == 0) {
		/* Frame from BSSID of the AP for which we are a client */
		skb->dev = dev = local->stadev;
		stats = hostap_get_stats(dev);
		from_assoc_ap = 1;
	}

	dev->last_rx = jiffies;

	if (local->iw_mode == IW_MODE_MASTER && !from_assoc_ap) {
		switch (hostap_handle_sta_rx(local, dev, skb, wds != NULL))
		{
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
		   local->wds_type & HOSTAP_WDS_AP_CLIENT)
		hostap_update_rx_stats(local->ap, rxdesc);

	/* Nullfunc frames may have PS-bit set, so they must be passed to
	 * hostap_handle_sta_rx() before being dropped here. */
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

	/* skb: rxdesc + (possibly fragmented, possibly encrypted) payload */

	if (local->host_decrypt &&
	    hostap_rx_frame_decrypt(local, fc & WLAN_FC_ISWEP, skb))
		goto rx_dropped;

	/* skb: rxdesc + (possibly fragmented) plaintext payload */

	if (local->host_decrypt && (fc & WLAN_FC_ISWEP) &&
	    (frag != 0 || (fc & WLAN_FC_MOREFRAG))) {
		int flen;
		struct sk_buff *frag_skb =
			prism2_frag_cache_get(local, rxdesc);
		if (!frag_skb) {
			printk(KERN_DEBUG "%s: Rx cannot get skb from "
			       "fragment cache (morefrag=%d seq=%u frag=%u)\n",
			       dev->name, (fc & WLAN_FC_MOREFRAG) != 0,
			       WLAN_GET_SEQ_SEQ(sc), frag);
			goto rx_dropped;
		}

		flen = skb->len;
		if (frag != 0)
			flen -= sizeof(struct hfa384x_rx_frame);

		if (frag_skb->tail + flen > frag_skb->end) {
			printk(KERN_WARNING "%s: host decrypted and "
			       "reassembled frame did not fit skb\n",
			       dev->name);
			prism2_frag_cache_invalidate(local, rxdesc);
			goto rx_dropped;
		}

		if (frag == 0) {
			/* copy first fragment (including full headers) into
			 * beginning of the fragment cache skb */
			memcpy(skb_put(frag_skb, flen), skb->data, flen);
		} else {
			/* append frame payload to the end of the fragment
			 * cache skb */
			memcpy(skb_put(frag_skb, flen),
			       skb->data + sizeof(struct hfa384x_rx_frame),
			       flen);
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
		rxdesc = (struct hfa384x_rx_frame *) skb->data;
		prism2_frag_cache_invalidate(local, rxdesc);
	}

	/* skb: rxdesc + (possible reassembled) full plaintext payload */

	payload = skb->data + sizeof(*rxdesc);
	ethertype = (payload[6] << 8) | payload[7];

	/* If IEEE 802.1X is used, check whether the port is authorized to send
	 * the received frame. */
	if (local->ieee_802_1x && local->iw_mode == IW_MODE_MASTER) {
		if (ethertype == ETH_P_PAE) {
			printk(KERN_DEBUG "%s: RX: IEEE 802.1X frame\n",
			       dev->name);
			if (local->hostapd && local->apdev) {
				/* Send IEEE 802.1X frames to the user
				 * space daemon for processing */
				prism2_rx_80211(local->apdev, skb,
						PRISM2_RX_MGMT, NULL, 0);
				local->apdevstats.rx_packets++;
				local->apdevstats.rx_bytes += skb->len;
				goto rx_exit;
			}
		} else if (!frame_authorized && !wds) {
			printk(KERN_DEBUG "%s: dropped frame from "
			       "unauthorized port (IEEE 802.1X): "
			       "ethertype=0x%04x\n",
			       dev->name, ethertype);
			hostap_dump_rx_header(dev->name, rxdesc);
			goto rx_dropped;
		}
	}

	/* convert rxdesc + possible LLC headers into Ethernet header */
	if (skb->len - sizeof(struct hfa384x_rx_frame) >= 8 &&
	    ((memcmp(payload, rfc1042_header, 6) == 0 &&
	      ethertype != ETH_P_AARP && ethertype != ETH_P_IPX) ||
	     memcmp(payload, bridge_tunnel_header, 6) == 0)) {
		/* remove RFC1042 or Bridge-Tunnel encapsulation and
		 * replace EtherType */
		skb_pull(skb, sizeof(*rxdesc) + 6);
		memmove(skb_push(skb, 2 * ETH_ALEN), rxdesc->dst_addr,
			2 * ETH_ALEN);
	} else {
		/* Leave Ethernet header part of rxdesc and full payload */
		skb_pull(skb, sizeof(*rxdesc) - ETH_HLEN);
	}

	if (wds || ((fc & (WLAN_FC_TODS | WLAN_FC_FROMDS)) ==
		    (WLAN_FC_TODS | WLAN_FC_FROMDS))) {
		/* Get original source address (Addr4 = SA) */
		if (fc & WLAN_FC_FROMDS) {
			/* IEEE 802.11 compliant WDS frame */
			memcpy(skb->data + ETH_ALEN, rxdesc->addr4, ETH_ALEN);
		} else if (skb->len >= ETH_HLEN + ETH_ALEN) {
			/* Non-standard frame: get addr4 from its bogus
			 * location after the payload */
			memcpy(skb->data + ETH_ALEN,
			       skb->data + skb->len - ETH_ALEN, ETH_ALEN);
			skb_trim(skb, skb->len - ETH_ALEN);
		}
	}

	stats->rx_packets++;
	stats->rx_bytes += skb->len;

	if (local->iw_mode == IW_MODE_MASTER && !wds &&
	    local->ap->bridge_packets) {
		if (rxdesc->addr3[0] & 0x01) {
			/* copy multicast frame both to the higher layers and
			 * to the wireless media */
			local->ap->bridged_multicast++;
			skb2 = skb_clone(skb, GFP_ATOMIC);
			if (skb2 == NULL)
				printk(KERN_DEBUG "%s: skb_clone failed for "
				       "multicast frame\n", dev->name);
		} else if (hostap_is_sta_assoc(local->ap, rxdesc->addr3)) {
			/* send frame directly to the associated STA using
			 * wireless media and not passing to higher layers */
			local->ap->bridged_unicast++;
			skb2 = skb;
			skb = NULL;
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


/* Called only as a tasklet (software IRQ) */
static void hostap_rx_tasklet(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&local->rx_list)) != NULL)
		hostap_rx_skb(local, skb);
}


/* Called only from hardware IRQ */
static void prism2_alloc_ev(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	int idx;
	u16 fid;

	fid = HFA384X_INW(HFA384X_ALLOCFID_OFF);

	PDEBUG(DEBUG_FID, "FID: interrupt: ALLOC - fid=0x%04x\n", fid);

	idx = local->next_alloc;

	do {
		if (local->txfid[idx] == fid) {
			PDEBUG(DEBUG_FID, "FID: found matching txfid[%d]\n",
			       idx);

#ifndef final_version
			if (local->intransmitfid[idx] == PRISM2_TXFID_EMPTY)
				printk("Already released txfid found at idx "
				       "%d\n", idx);
			if (local->intransmitfid[idx] == PRISM2_TXFID_RESERVED)
				printk("Already reserved txfid found at idx "
				       "%d\n", idx);
#endif
			local->intransmitfid[idx] = PRISM2_TXFID_EMPTY;
			idx++;
			local->next_alloc = idx >= PRISM2_TXFID_COUNT ? 0 :
				idx;

			if (!test_bit(HOSTAP_BITS_TRANSMIT, &local->bits) &&
			    netif_queue_stopped(dev))
				hostap_netif_wake_queues(dev);

			return;
		}

		idx++;
		if (idx >= PRISM2_TXFID_COUNT)
			idx = 0;
	} while (idx != local->next_alloc);

	printk(KERN_WARNING "%s: could not find matching txfid (0x%04x) for "
	       "alloc event\n", dev->name, HFA384X_INW(HFA384X_ALLOCFID_OFF));
}


/* TX callback data stored in skb->cb; this must not grow over the maximum
 * skb->cb size (48 bytes) */
struct hostap_tx_callback_skb_cb {
	struct hostap_tx_callback_info *cb;
	int ok;
};


/* Called only as a tasklet (software IRQ) */
static void hostap_tx_callback_tasklet(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	struct sk_buff *skb;
	struct hostap_tx_callback_skb_cb *cb_data;

	while ((skb = skb_dequeue(&local->tx_callback_list)) != NULL) {
		cb_data = (struct hostap_tx_callback_skb_cb *) skb->cb;
		cb_data->cb->func(skb, cb_data->ok, cb_data->cb->data);
	}
}


/* Called only as a tasklet (software IRQ) */
static void hostap_tx_callback(local_info_t *local,
			       struct hfa384x_tx_frame *txdesc, int ok,
			       char *payload)
{
	u16 sw_support, hdrlen, len;
	struct sk_buff *skb;
	struct hostap_tx_callback_info *cb;
	struct hostap_tx_callback_skb_cb *cb_data;

	/* Make sure that frame was from us. */
	if (memcmp(txdesc->addr2, local->dev->dev_addr, ETH_ALEN)) {
		printk(KERN_DEBUG "%s: TX callback - foreign frame\n",
		       local->dev->name);
		return;
	}

	sw_support = le16_to_cpu(txdesc->sw_support);

	spin_lock(&local->lock);
	cb = local->tx_callback;
	while (cb != NULL && cb->idx != sw_support)
		cb = cb->next;
	spin_unlock(&local->lock);

	if (cb == NULL) {
		printk(KERN_DEBUG "%s: could not find TX callback (idx %d)\n",
		       local->dev->name, sw_support);
		return;
	}

	hdrlen = hostap_80211_get_hdrlen(le16_to_cpu(txdesc->frame_control));
	len = le16_to_cpu(txdesc->data_len);
	skb = dev_alloc_skb(hdrlen + len);
	if (skb == NULL) {
		printk(KERN_DEBUG "%s: hostap_tx_callback failed to allocate "
		       "skb\n", local->dev->name);
		return;
	}

	memcpy(skb_put(skb, hdrlen), (void *) &txdesc->frame_control, hdrlen);
	if (payload)
		memcpy(skb_put(skb, len), payload, len);

	skb->dev = local->dev;
	skb->mac.raw = skb->data;

	/* Store TX callback info pointer to private data in the skb and finish
	 * TX callback handling in tasklet so that hw IRQ can be ACKed now */
	cb_data = (struct hostap_tx_callback_skb_cb *) skb->cb;
	cb_data->cb = cb;
	cb_data->ok = ok;
	skb_queue_tail(&local->tx_callback_list, skb);
	HOSTAP_TASKLET_SCHEDULE(&local->tx_callback_tasklet);
}


/* Called only as a tasklet (software IRQ) */
static int hostap_tx_compl_read(local_info_t *local, int error,
				struct hfa384x_tx_frame *txdesc,
				char **payload)
{
	u16 fid, len;
	int res, ret = 0;
	struct net_device *dev = local->dev;

	fid = HFA384X_INW(HFA384X_TXCOMPLFID_OFF);

	PDEBUG(DEBUG_FID, "interrupt: TX (err=%d) - fid=0x%04x\n", fid, error);

	spin_lock(&local->baplock);
	res = hfa384x_setup_bap(dev, BAP0, fid, 0);
	if (!res)
		res = hfa384x_from_bap(dev, BAP0, txdesc, sizeof(*txdesc));
	if (res) {
		PDEBUG(DEBUG_EXTRA, "%s: TX (err=%d) - fid=0x%04x - could not "
		       "read txdesc\n", dev->name, error, fid);
		if (res == -ETIMEDOUT) {
			PRISM2_SCHEDULE_TASK(&local->reset_queue);
		}
		ret = -1;
		goto fail;
	}
	if (txdesc->sw_support) {
		len = le16_to_cpu(txdesc->data_len);
		if (len < PRISM2_DATA_MAXLEN) {
			*payload = (char *) kmalloc(len, GFP_ATOMIC);
			if (*payload == NULL ||
			    hfa384x_from_bap(dev, BAP0, *payload, len)) {
				PDEBUG(DEBUG_EXTRA, "%s: could not read TX "
				       "frame payload\n", dev->name);
				kfree(*payload);
				*payload = NULL;
				ret = -1;
				goto fail;
			}
		}
	}

 fail:
	spin_unlock(&local->baplock);

	return ret;
}


/* Called only as a tasklet (software IRQ) */
static void prism2_tx_ev(local_info_t *local)
{
	struct net_device *dev = local->dev;
	char *payload = NULL;
	struct hfa384x_tx_frame txdesc;

	if (hostap_tx_compl_read(local, 0, &txdesc, &payload))
		goto fail;

	if (local->frame_dump & PRISM2_DUMP_TX_HDR) {
		PDEBUG(DEBUG_EXTRA, "%s: TX - status=0x%04x "
		       "retry_count=%d tx_rate=%d seq_ctrl=%d "
		       "duration_id=%d\n",
		       dev->name, le16_to_cpu(txdesc.status),
		       txdesc.retry_count, txdesc.tx_rate,
		       le16_to_cpu(txdesc.seq_ctrl),
		       le16_to_cpu(txdesc.duration_id));
	}

	if (txdesc.sw_support)
		hostap_tx_callback(local, &txdesc, 1, payload);
	kfree(payload);

 fail:
	HFA384X_OUTW(HFA384X_EV_TX, HFA384X_EVACK_OFF);
}


/* Called only as a tasklet (software IRQ) */
static void hostap_sta_tx_exc_tasklet(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&local->sta_tx_exc_list)) != NULL) {
		struct hfa384x_tx_frame *txdesc =
			(struct hfa384x_tx_frame *) skb->data;
		hostap_handle_sta_tx_exc(local, txdesc);
		dev_kfree_skb(skb);
	}
}


/* Called only as a tasklet (software IRQ) */
static void prism2_txexc(local_info_t *local)
{
	struct net_device *dev = local->dev;
	u16 status, fc;
	int show_dump, res;
	char *payload = NULL;
	struct hfa384x_tx_frame txdesc;

	show_dump = local->frame_dump & PRISM2_DUMP_TXEXC_HDR;
	local->stats.tx_errors++;

	res = hostap_tx_compl_read(local, 1, &txdesc, &payload);
	HFA384X_OUTW(HFA384X_EV_TXEXC, HFA384X_EVACK_OFF);
	if (res)
		return;

	status = le16_to_cpu(txdesc.status);

#if WIRELESS_EXT > 13
	/* We produce a TXDROP event only for retry or lifetime
	 * exceeded, because that's the only status that really mean
	 * that this particular node went away.
	 * Other errors means that *we* screwed up. - Jean II */
	if (status & (HFA384X_TX_STATUS_RETRYERR | HFA384X_TX_STATUS_AGEDERR))
	{
		union iwreq_data wrqu;

		/* Copy 802.11 dest address. */
		memcpy(wrqu.addr.sa_data, txdesc.addr1, ETH_ALEN);
		wrqu.addr.sa_family = ARPHRD_ETHER;
		wireless_send_event(dev, IWEVTXDROP, &wrqu, NULL);
	} else
		show_dump = 1;
#endif /* WIRELESS_EXT > 13 */

	if (local->iw_mode == IW_MODE_MASTER ||
	    local->iw_mode == IW_MODE_REPEAT ||
	    local->wds_type & HOSTAP_WDS_AP_CLIENT) {
		struct sk_buff *skb;
		skb = dev_alloc_skb(sizeof(txdesc));
		if (skb) {
			memcpy(skb_put(skb, sizeof(txdesc)), &txdesc,
			       sizeof(txdesc));
			skb_queue_tail(&local->sta_tx_exc_list, skb);
			HOSTAP_TASKLET_SCHEDULE(&local->sta_tx_exc_tasklet);
		}
	}

	if (txdesc.sw_support)
		hostap_tx_callback(local, &txdesc, 0, payload);
	kfree(payload);

	if (!show_dump)
		return;

	PDEBUG(DEBUG_EXTRA, "%s: TXEXC - status=0x%04x (%s%s%s%s)"
	       " tx_control=%04x\n",
	       dev->name, status,
	       status & HFA384X_TX_STATUS_RETRYERR ? "[RetryErr]" : "",
	       status & HFA384X_TX_STATUS_AGEDERR ? "[AgedErr]" : "",
	       status & HFA384X_TX_STATUS_DISCON ? "[Discon]" : "",
	       status & HFA384X_TX_STATUS_FORMERR ? "[FormErr]" : "",
	       le16_to_cpu(txdesc.tx_control));

	fc = le16_to_cpu(txdesc.frame_control);
	PDEBUG(DEBUG_EXTRA, "   retry_count=%d tx_rate=%d fc=0x%04x "
	       "(%s%s%s::%d%s%s)\n",
	       txdesc.retry_count, txdesc.tx_rate, fc,
	       WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_MGMT ? "Mgmt" : "",
	       WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_CTRL ? "Ctrl" : "",
	       WLAN_FC_GET_TYPE(fc) == WLAN_FC_TYPE_DATA ? "Data" : "",
	       WLAN_FC_GET_STYPE(fc),
	       fc & WLAN_FC_TODS ? " ToDS" : "",
	       fc & WLAN_FC_FROMDS ? " FromDS" : "");
	PDEBUG(DEBUG_EXTRA, "   A1=" MACSTR " A2=" MACSTR " A3="
	       MACSTR " A4=" MACSTR "\n",
	       MAC2STR(txdesc.addr1), MAC2STR(txdesc.addr2),
	       MAC2STR(txdesc.addr3), MAC2STR(txdesc.addr4));
}


/* Called only as a tasklet (software IRQ) */
static void hostap_info_tasklet(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&local->info_list)) != NULL) {
		hostap_info_process(local, skb);
		dev_kfree_skb(skb);
	}
}


/* Called only as a tasklet (software IRQ) */
static void prism2_info(local_info_t *local)
{
	struct net_device *dev = local->dev;
	u16 fid;
	int res, left;
	struct hfa384x_info_frame info;
	struct sk_buff *skb;

	fid = HFA384X_INW(HFA384X_INFOFID_OFF);

	spin_lock(&local->baplock);
	res = hfa384x_setup_bap(dev, BAP0, fid, 0);
	if (!res)
		res = hfa384x_from_bap(dev, BAP0, &info, sizeof(info));
	if (res) {
		spin_unlock(&local->baplock);
		printk(KERN_DEBUG "Could not get info frame (fid=0x%04x)\n",
		       fid);
		if (res == -ETIMEDOUT) {
			PRISM2_SCHEDULE_TASK(&local->reset_queue);
		}
		goto out;
	}

	le16_to_cpus(&info.len);
	le16_to_cpus(&info.type);
	left = (info.len - 1) * 2;

	if (info.len & 0x8000 || info.len == 0 || left > 2060) {
		/* data register seems to give 0x8000 in some error cases even
		 * though busy bit is not set in offset register;
		 * in addition, length must be at least 1 due to type field */
		spin_unlock(&local->baplock);
		printk(KERN_DEBUG "%s: Received info frame with invalid "
		       "length 0x%04x (type 0x%04x)\n", dev->name, info.len,
		       info.type);
		goto out;
	}

	skb = dev_alloc_skb(sizeof(info) + left);
	if (skb == NULL) {
		spin_unlock(&local->baplock);
		printk(KERN_DEBUG "%s: Could not allocate skb for info "
		       "frame\n", dev->name);
		goto out;
	}

	memcpy(skb_put(skb, sizeof(info)), &info, sizeof(info));
	if (left > 0 && hfa384x_from_bap(dev, BAP0, skb_put(skb, left), left))
	{
		spin_unlock(&local->baplock);
		printk(KERN_WARNING "%s: Info frame read failed (fid=0x%04x, "
		       "len=0x%04x, type=0x%04x\n",
		       dev->name, fid, info.len, info.type);
		dev_kfree_skb(skb);
		goto out;
	}
	spin_unlock(&local->baplock);

	skb_queue_tail(&local->info_list, skb);
	HOSTAP_TASKLET_SCHEDULE(&local->info_tasklet);

 out:
	HFA384X_OUTW(HFA384X_EV_INFO, HFA384X_EVACK_OFF);
}


/* Called only as a tasklet (software IRQ) */
static void hostap_bap_tasklet(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	struct net_device *dev = local->dev;
	u16 ev;
	int frames = 30;

	if (local->func->card_present && !local->func->card_present(local))
		return;

	set_bit(HOSTAP_BITS_BAP_TASKLET, &local->bits);

	/* Process all pending BAP events without generating new interrupts
	 * for them */
	while (frames-- > 0) {
		ev = HFA384X_INW(HFA384X_EVSTAT_OFF);
		if (ev == 0xffff || !(ev & HFA384X_BAP0_EVENTS))
			break;
		if (ev & HFA384X_EV_RX)
			prism2_rx(local);
		if (ev & HFA384X_EV_INFO)
			prism2_info(local);
		if (ev & HFA384X_EV_TX)
			prism2_tx_ev(local);
		if (ev & HFA384X_EV_TXEXC)
			prism2_txexc(local);
	}

	set_bit(HOSTAP_BITS_BAP_TASKLET2, &local->bits);
	clear_bit(HOSTAP_BITS_BAP_TASKLET, &local->bits);

	/* Enable interrupts for new BAP events */
	hfa384x_events_all(dev);
	clear_bit(HOSTAP_BITS_BAP_TASKLET2, &local->bits);
}


#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
/* Called only from hardware IRQ */
static void prism2_bus_master_ev(struct net_device *dev, int bap)
{
	local_info_t *local = (local_info_t *) dev->priv;
	if (bap == BAP1) {
		/* FIX: frame payload was DMA'd to skb->data; might need to
		 * invalidate data cache for that memory area */
		skb_queue_tail(&local->rx_list, local->rx_skb);
		HOSTAP_TASKLET_SCHEDULE(&local->rx_tasklet);
		HFA384X_OUTW(HFA384X_EV_RX, HFA384X_EVACK_OFF);
	} else {
		if (prism2_transmit(dev, local->bus_m0_tx_idx)) {
			printk(KERN_DEBUG "%s: prism2_transmit() failed "
			       "when called from bus master event\n",
			       dev->name);
			local->intransmitfid[local->bus_m0_tx_idx] =
				PRISM2_TXFID_EMPTY;
			PRISM2_SCHEDULE_TASK(&local->reset_queue);
		}
	}
}
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */


/* Called only from hardware IRQ */
static void prism2_infdrop(struct net_device *dev)
{
	static long last_inquire = 0;

	PDEBUG(DEBUG_EXTRA, "%s: INFDROP event\n", dev->name);

	/* some firmware versions seem to get stuck with
	 * full CommTallies in high traffic load cases; every
	 * packet will then cause INFDROP event and CommTallies
	 * info frame will not be sent automatically. Try to
	 * get out of this state by inquiring CommTallies. */
	if (last_inquire + HZ < jiffies) {
		hfa384x_cmd_callback(dev, HFA384X_CMDCODE_INQUIRE,
				     HFA384X_INFO_COMMTALLIES, NULL, NULL);
		last_inquire = jiffies;
	}
}


/* Called only from hardware IRQ */
static void prism2_ev_tick(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	u16 evstat, inten;

	if (local->last_tick_timer + 5 * HZ < jiffies &&
	    local->last_tick_timer) {
		evstat = HFA384X_INW(HFA384X_EVSTAT_OFF);
		inten = HFA384X_INW(HFA384X_INTEN_OFF);
		printk(KERN_INFO "%s: SW TICK stuck? "
		       "bits=0x%lx EvStat=%04x IntEn=%04x\n",
		       dev->name, local->bits, evstat, inten);
		local->sw_tick_stuck++;
		if ((evstat & HFA384X_BAP0_EVENTS) &&
		    (inten & HFA384X_BAP0_EVENTS)) {
			printk(KERN_INFO "%s: trying to recover from IRQ "
			       "hang\n", dev->name);
			hfa384x_events_no_bap0(dev);
		}
	}
}


/* Called only from hardware IRQ */
static inline void prism2_check_magic(local_info_t *local)
{
	/* at least PCI Prism2.5 with bus mastering seems to sometimes
	 * return 0x0000 in SWSUPPORT0 for unknown reason, but re-reading the
	 * register once or twice seems to get the correct value.. PCI cards
	 * cannot anyway be removed during normal operation, so there is not
	 * really any need for this verification with them. */

#ifndef PRISM2_PCI
#ifndef final_version
	static long int last_magic_err = 0;
	struct net_device *dev = local->dev;

	if (HFA384X_INW(HFA384X_SWSUPPORT0_OFF) != HFA384X_MAGIC) {
		if (!local->hw_ready)
			return;
		HFA384X_OUTW(0xffff, HFA384X_EVACK_OFF);
		if (jiffies - last_magic_err > 10 * HZ) {
			printk("%s: Interrupt, but SWSUPPORT0 does not match: "
			       "%04X != %04X - card removed?\n", dev->name,
			       HFA384X_INW(HFA384X_SWSUPPORT0_OFF),
			       HFA384X_MAGIC);
			last_magic_err = jiffies;
		} else if (net_ratelimit()) {
			printk(KERN_DEBUG "%s: interrupt - SWSUPPORT0=%04x "
			       "MAGIC=%04x\n", dev->name,
			       HFA384X_INW(HFA384X_SWSUPPORT0_OFF),
			       HFA384X_MAGIC);
		}
		PRISM2_SCHEDULE_TASK(&local->reset_queue);
		return;
	}
#endif /* final_version */
#endif /* !PRISM2_PCI */
}


/* Called only from hardware IRQ */
static irqreturn_t prism2_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct net_device *dev = (struct net_device *) dev_id;
	local_info_t *local = (local_info_t *) dev->priv;
	int events = 0;
	u16 ev;

	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_INTERRUPT, 0, 0);

	if (local->func->card_present && !local->func->card_present(local)) {
		printk(KERN_DEBUG "%s: Interrupt, but dev not OK\n",
		       dev->name);
		return IRQ_HANDLED;
	}

	prism2_check_magic(local);

	for (;;) {
		ev = HFA384X_INW(HFA384X_EVSTAT_OFF);
		if (ev == 0xffff) {
			if (local->shutdown)
				return IRQ_HANDLED;
			HFA384X_OUTW(0xffff, HFA384X_EVACK_OFF);
			printk(KERN_DEBUG "%s: prism2_interrupt: ev=0xffff\n",
			       dev->name);
			PRISM2_SCHEDULE_TASK(&local->reset_queue);
			return IRQ_HANDLED;
		}

		ev &= HFA384X_INW(HFA384X_INTEN_OFF);
		if (ev == 0)
			break;

		if (ev & HFA384X_EV_CMD) {
			prism2_cmd_ev(dev);
		}

		/* Above events are needed even before hw is ready, but other
		 * events should be skipped during initialization. This may
		 * change for AllocEv if allocate_fid is implemented without
		 * busy waiting. */
		if (!local->hw_ready || local->hw_resetting ||
		    !local->dev_enabled) {
			ev = HFA384X_INW(HFA384X_EVSTAT_OFF);
			if (ev & HFA384X_EV_CMD)
				goto next_event;
			if ((ev & HFA384X_EVENT_MASK) == 0)
				return IRQ_HANDLED;
			if (local->dev_enabled && (ev & ~HFA384X_EV_TICK) &&
			    net_ratelimit()) {
				printk(KERN_DEBUG "%s: prism2_interrupt: hw "
				       "not ready; skipping events 0x%04x "
				       "(IntEn=0x%04x)%s%s%s\n",
				       dev->name, ev,
				       HFA384X_INW(HFA384X_INTEN_OFF),
				       !local->hw_ready ? " (!hw_ready)" : "",
				       local->hw_resetting ?
				       " (hw_resetting)" : "",
				       !local->dev_enabled ?
				       " (!dev_enabled)" : "");
			}
			HFA384X_OUTW(ev, HFA384X_EVACK_OFF);
			return IRQ_HANDLED;
		}

		if (ev & HFA384X_EV_TICK) {
			prism2_ev_tick(dev);
			HFA384X_OUTW(HFA384X_EV_TICK, HFA384X_EVACK_OFF);
		}

#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
		if (ev & HFA384X_EV_PCI_M0) {
			prism2_bus_master_ev(dev, BAP0);
			HFA384X_OUTW(HFA384X_EV_PCI_M0, HFA384X_EVACK_OFF);
		}

		if (ev & HFA384X_EV_PCI_M1) {
			/* previous RX has been copied can be ACKed now */
			HFA384X_OUTW(HFA384X_EV_RX, HFA384X_EVACK_OFF);

			prism2_bus_master_ev(dev, BAP1);
			HFA384X_OUTW(HFA384X_EV_PCI_M1, HFA384X_EVACK_OFF);
		}
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */

		if (ev & HFA384X_EV_ALLOC) {
			prism2_alloc_ev(dev);
			HFA384X_OUTW(HFA384X_EV_ALLOC, HFA384X_EVACK_OFF);
		}

		/* Reading data from the card is quite time consuming, so do it
		 * in tasklets. TX, TXEXC, RX, and INFO events will be ACKed
		 * and unmasked after needed data has been read completely. */
		if (ev & HFA384X_BAP0_EVENTS) {
			hfa384x_events_no_bap0(dev);
			HOSTAP_TASKLET_SCHEDULE(&local->bap_tasklet);
		}

#ifndef final_version
		if (ev & HFA384X_EV_WTERR) {
			PDEBUG(DEBUG_EXTRA, "%s: WTERR event\n", dev->name);
			HFA384X_OUTW(HFA384X_EV_WTERR, HFA384X_EVACK_OFF);
		}
#endif /* final_version */

		if (ev & HFA384X_EV_INFDROP) {
			prism2_infdrop(dev);
			HFA384X_OUTW(HFA384X_EV_INFDROP, HFA384X_EVACK_OFF);
		}

	next_event:
		events++;
		if (events >= PRISM2_MAX_INTERRUPT_EVENTS) {
			PDEBUG(DEBUG_EXTRA, "prism2_interrupt: >%d events "
			       "(EvStat=0x%04x)\n",
			       PRISM2_MAX_INTERRUPT_EVENTS,
			       HFA384X_INW(HFA384X_EVSTAT_OFF));
			break;
		}
	}
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_INTERRUPT, 0, 1);
	return IRQ_RETVAL(events);
}


static void prism2_check_sta_fw_version(local_info_t *local)
{
	struct hfa384x_comp_ident comp;
	int id, variant, major, minor;

	if (hfa384x_get_rid(local->dev, HFA384X_RID_STAID,
			    &comp, sizeof(comp), 1) < 0)
		return;

	local->fw_ap = 0;
	id = le16_to_cpu(comp.id);
	if (id != HFA384X_COMP_ID_STA) {
		if (id == HFA384X_COMP_ID_FW_AP)
			local->fw_ap = 1;
		return;
	}

	major = __le16_to_cpu(comp.major);
	minor = __le16_to_cpu(comp.minor);
	variant = __le16_to_cpu(comp.variant);
	local->sta_fw_ver = PRISM2_FW_VER(major, minor, variant);

	/* Station firmware versions before 1.4.x seem to have a bug in
	 * firmware-based WEP encryption when using Host AP mode, so use
	 * host_encrypt as a default for them. Firmware version 1.4.9 is the
	 * first one that has been seen to produce correct encryption, but the
	 * bug might be fixed before that (although, at least 1.4.2 is broken).
	 */
	local->fw_encrypt_ok = major > 1 ||
		(major == 1 && (minor > 4 || (minor == 4 && variant >= 9)));

	if (local->iw_mode == IW_MODE_MASTER && !local->host_encrypt &&
	    !local->fw_encrypt_ok) {
		printk(KERN_DEBUG "%s: defaulting to host-based encryption as "
		       "a workaround for firmware bug in Host AP mode WEP\n",
		       local->dev->name);
		local->host_encrypt = 1;
	}

	/* IEEE 802.11 standard compliant WDS frames (4 addresses) were broken
	 * in station firmware versions before 1.5.x. With these versions, the
	 * driver uses a workaround with bogus frame format (4th address after
	 * the payload). This is not compatible with other AP devices. Since
	 * the firmware bug is fixed in the latest station firmware versions,
	 * automatically enable standard compliant mode for cards using station
	 * firmware version 1.5.0 or newer. */
	if (major > 1 || (major == 1 && (minor >= 5)))
		local->wds_type |= HOSTAP_WDS_STANDARD_FRAME;
	else {
		printk(KERN_DEBUG "%s: defaulting to bogus WDS frame as a "
		       "workaround for firmware bug in Host AP mode WDS\n",
		       local->dev->name);
	}

	hostap_check_sta_fw_version(local->ap, major, minor, variant);
}


static void prism2_crypt_deinit_entries(local_info_t *local, int force)
{
	struct list_head *ptr, *n;
	struct prism2_crypt_data *entry;

	for (ptr = local->crypt_deinit_list.next, n = ptr->next;
	     ptr != &local->crypt_deinit_list; ptr = n, n = ptr->next) {
		entry = list_entry(ptr, struct prism2_crypt_data, list);

		if (atomic_read(&entry->refcnt) != 0 && !force)
			continue;

		list_del(ptr);

		if (entry->ops)
			entry->ops->deinit(entry->priv);
		kfree(entry);
	}
}


static void prism2_crypt_deinit_handler(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	unsigned long flags;

	spin_lock_irqsave(&local->lock, flags);
	prism2_crypt_deinit_entries(local, 0);
	if (!list_empty(&local->crypt_deinit_list)) {
		printk(KERN_DEBUG "%s: entries remaining in delayed crypt "
		       "deletion list\n", local->dev->name);
		local->crypt_deinit_timer.expires = jiffies + HZ;
		add_timer(&local->crypt_deinit_timer);
	}
	spin_unlock_irqrestore(&local->lock, flags);

}


static void hostap_passive_scan(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	struct net_device *dev = local->dev;
	u16 channel;

	if (local->passive_scan_interval <= 0)
		return;

	if (local->passive_scan_state == PASSIVE_SCAN_LISTEN) {
		int max_tries = 16;

		/* Even though host system does not really know when the WLAN
		 * MAC is sending frames, try to avoid changing channels for
		 * passive scanning when a host-generated frame is being
		 * transmitted */
		if (test_bit(HOSTAP_BITS_TRANSMIT, &local->bits)) {
			printk(KERN_DEBUG "%s: passive scan detected pending "
			       "TX - delaying\n", dev->name);
			local->passive_scan_timer.expires = jiffies + HZ / 10;
			add_timer(&local->passive_scan_timer);
			return;
		}

		do {
			local->passive_scan_channel++;
			if (local->passive_scan_channel > 14)
				local->passive_scan_channel = 1;
			max_tries--;
		} while (!(local->channel_mask &
			   (1 << (local->passive_scan_channel - 1))) &&
			 max_tries > 0);

		if (max_tries == 0) {
			printk(KERN_INFO "%s: no allowed passive scan channels"
			       " found\n", dev->name);
			return;
		}

		printk(KERN_DEBUG "%s: passive scan channel %d\n",
		       dev->name, local->passive_scan_channel);
		channel = local->passive_scan_channel;
		local->passive_scan_state = PASSIVE_SCAN_WAIT;
		local->passive_scan_timer.expires = jiffies + HZ / 10;
	} else {
		channel = local->channel;
		local->passive_scan_state = PASSIVE_SCAN_LISTEN;
		local->passive_scan_timer.expires = jiffies +
			local->passive_scan_interval * HZ;
	}

	if (hfa384x_cmd_callback(dev, HFA384X_CMDCODE_TEST |
				 (HFA384X_TEST_CHANGE_CHANNEL << 8),
				 channel, NULL, NULL))
		printk(KERN_ERR "%s: passive scan channel set %d "
		       "failed\n", dev->name, channel);

	add_timer(&local->passive_scan_timer);
}


/* Software watchdog - called as a timer. Hardware interrupt (Tick event) is
 * used to monitor that local->last_tick_timer is being updated. If not,
 * interrupt busy-loop is assumed and driver tries to recover by masking out
 * some events. */
static void hostap_tick_timer(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	local->last_tick_timer = jiffies;
	local->tick_timer.expires = jiffies + 2 * HZ;
	add_timer(&local->tick_timer);
}


#ifndef PRISM2_NO_PROCFS_DEBUG
static int prism2_registers_proc_read(char *page, char **start, off_t off,
				      int count, int *eof, void *data)
{
	char *p = page;
	local_info_t *local = (local_info_t *) data;

	if (off != 0) {
		*eof = 1;
		return 0;
	}

#define SHOW_REG(n) \
p += sprintf(p, "%s=%04x\n", #n, \
hfa384x_read_reg(local->dev, HFA384X_##n##_OFF))

	SHOW_REG(CMD);
	SHOW_REG(PARAM0);
	SHOW_REG(PARAM1);
	SHOW_REG(PARAM2);
	SHOW_REG(STATUS);
	SHOW_REG(RESP0);
	SHOW_REG(RESP1);
	SHOW_REG(RESP2);
	SHOW_REG(INFOFID);
	SHOW_REG(CONTROL);
	SHOW_REG(SELECT0);
	SHOW_REG(SELECT1);
	SHOW_REG(OFFSET0);
	SHOW_REG(OFFSET1);
	SHOW_REG(RXFID);
	SHOW_REG(ALLOCFID);
	SHOW_REG(TXCOMPLFID);
	SHOW_REG(SWSUPPORT0);
	SHOW_REG(SWSUPPORT1);
	SHOW_REG(SWSUPPORT2);
	SHOW_REG(EVSTAT);
	SHOW_REG(INTEN);
	SHOW_REG(EVACK);
	/* Do not read data registers, because they change the state of the
	 * MAC (offset += 2) */
	/* SHOW_REG(DATA0); */
	/* SHOW_REG(DATA1); */
	SHOW_REG(AUXPAGE);
	SHOW_REG(AUXOFFSET);
	/* SHOW_REG(AUXDATA); */
#ifdef PRISM2_PCI
	SHOW_REG(PCICOR);
	SHOW_REG(PCIHCR);
	SHOW_REG(PCI_M0_ADDRH);
	SHOW_REG(PCI_M0_ADDRL);
	SHOW_REG(PCI_M0_LEN);
	SHOW_REG(PCI_M0_CTL);
	SHOW_REG(PCI_STATUS);
	SHOW_REG(PCI_M1_ADDRH);
	SHOW_REG(PCI_M1_ADDRL);
	SHOW_REG(PCI_M1_LEN);
	SHOW_REG(PCI_M1_CTL);
#endif /* PRISM2_PCI */

	return (p - page);
}
#endif /* PRISM2_NO_PROCFS_DEBUG */


static local_info_t *
prism2_init_local_data(struct prism2_helper_functions *funcs, int card_idx)
{
	local_info_t *local;
	int len, i;

	if (funcs == NULL)
		return NULL;

	local = kmalloc(sizeof(local_info_t), GFP_KERNEL);
	if (local == NULL)
		return NULL;

	memset(local, 0, sizeof(local_info_t));
	local->hw_module = THIS_MODULE;
	local->ap = kmalloc(sizeof(struct ap_data), GFP_KERNEL);
	if (local->ap == NULL)
		goto fail;

	memset(local->ap, 0, sizeof(struct ap_data));

#ifdef PRISM2_IO_DEBUG
	local->io_debug_enabled = 1;
#endif /* PRISM2_IO_DEBUG */

#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
	local->bus_m0_buf = (u8 *) kmalloc(sizeof(struct hfa384x_tx_frame) +
					   PRISM2_DATA_MAXLEN, GFP_DMA);
	if (local->bus_m0_buf == NULL)
		goto fail;
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */

	local->dev = kmalloc(sizeof(struct net_device) + PRISM2_NETDEV_EXTRA,
			     GFP_KERNEL);
	if (local->dev == NULL)
		goto fail;
	memset(local->dev, 0, sizeof(struct net_device) + PRISM2_NETDEV_EXTRA);
	prism2_set_dev_name(local->dev, local->dev + 1);
	local->dev->priv = local;

	local->func = funcs;
	local->func->cmd = hfa384x_cmd;
	local->func->read_regs = hfa384x_read_regs;
	local->func->get_rid = hfa384x_get_rid;
	local->func->set_rid = hfa384x_set_rid;
	local->func->hw_enable = prism2_hw_enable;
	local->func->hw_config = prism2_hw_config;
	local->func->hw_reset = prism2_hw_reset;
	local->func->hw_shutdown = prism2_hw_shutdown;
	local->func->reset_port = prism2_reset_port;
	local->func->tx = prism2_tx;
	local->func->schedule_reset = prism2_schedule_reset;
#ifdef PRISM2_DOWNLOAD_SUPPORT
	local->func->download = prism2_download;
#endif /* PRISM2_DOWNLOAD_SUPPORT */
	local->func->rx_80211 = prism2_rx_80211;
	local->func->tx_80211 = prism2_tx_80211;

	local->disable_on_close = disable_on_close;
	local->mtu = mtu;

	spin_lock_init(&local->txfidlock);
	spin_lock_init(&local->cmdlock);
	spin_lock_init(&local->baplock);
	spin_lock_init(&local->wdslock);
	spin_lock_init(&local->lock);
	init_MUTEX(&local->rid_bap_sem);

	if (card_idx < 0 || card_idx >= MAX_PARM_DEVICES)
		card_idx = 0;
	local->card_idx = card_idx;

	i = essid[card_idx] == NULL ? 0 : card_idx;
	len = strlen(essid[i]);
	memcpy(local->essid, essid[i],
	       len > MAX_SSID_LEN ? MAX_SSID_LEN : len);
	local->essid[MAX_SSID_LEN] = '\0';
#ifdef WIRELESS_EXT
	i = GET_INT_PARM(iw_mode, card_idx);
	if ((i >= IW_MODE_ADHOC && i <= IW_MODE_REPEAT) ||
	    i == IW_MODE_MONITOR) {
		local->iw_mode = i;
	} else {
		printk(KERN_WARNING "prism2: Unknown iw_mode %d; using "
		       "IW_MODE_MASTER\n", i);
		local->iw_mode = IW_MODE_MASTER;
	}
#endif
	local->channel = GET_INT_PARM(channel, card_idx);
	local->beacon_int = GET_INT_PARM(beacon_int, card_idx);
	local->dtim_period = GET_INT_PARM(dtim_period, card_idx);
	local->wds_max_connections = 16;
	local->tx_control = HFA384X_TX_CTRL_FLAGS;
	local->manual_retry_count = -1;
#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
	local->bus_master_threshold_rx = GET_INT_PARM(bus_master_threshold_rx,
						      card_idx);
	local->bus_master_threshold_tx = GET_INT_PARM(bus_master_threshold_tx,
						      card_idx);
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */

	/* Initialize task queue structures */
	HOSTAP_QUEUE_INIT(&local->reset_queue, handle_reset_queue, local);
	HOSTAP_QUEUE_INIT(&local->set_multicast_list_queue,
			  hostap_set_multicast_list_queue, local->dev);

	/* Initialize tasklets for handling hardware IRQ related operations
	 * outside hw IRQ handler */
	HOSTAP_TASKLET_INIT(&local->bap_tasklet, hostap_bap_tasklet,
			    (unsigned long) local);

	HOSTAP_TASKLET_INIT(&local->info_tasklet, hostap_info_tasklet,
			    (unsigned long) local);
	hostap_info_init(local);

	HOSTAP_TASKLET_INIT(&local->tx_callback_tasklet,
			    hostap_tx_callback_tasklet, (unsigned long) local);
	skb_queue_head_init(&local->tx_callback_list);

	HOSTAP_TASKLET_INIT(&local->rx_tasklet,
			    hostap_rx_tasklet, (unsigned long) local);
	skb_queue_head_init(&local->rx_list);

	HOSTAP_TASKLET_INIT(&local->sta_tx_exc_tasklet,
			    hostap_sta_tx_exc_tasklet, (unsigned long) local);
	skb_queue_head_init(&local->sta_tx_exc_list);

	INIT_LIST_HEAD(&local->cmd_queue);
	init_waitqueue_head(&local->hostscan_wq);
	INIT_LIST_HEAD(&local->crypt_deinit_list);
	init_timer(&local->crypt_deinit_timer);
	local->crypt_deinit_timer.data = (unsigned long) local;
	local->crypt_deinit_timer.function = prism2_crypt_deinit_handler;

	init_timer(&local->passive_scan_timer);
	local->passive_scan_timer.data = (unsigned long) local;
	local->passive_scan_timer.function = hostap_passive_scan;

	init_timer(&local->tick_timer);
	local->tick_timer.data = (unsigned long) local;
	local->tick_timer.function = hostap_tick_timer;
	local->tick_timer.expires = jiffies + 2 * HZ;
	add_timer(&local->tick_timer);

	hostap_setup_dev(local->dev, local, 1);

	local->saved_eth_header_parse = local->dev->hard_header_parse;

	return local;

 fail:
	kfree(local->ap);
	kfree(local->dev);
#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
	kfree(local->bus_m0_buf);
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */
	kfree(local->apdev);
	kfree(local->stadev);
	kfree(local);
	return NULL;
}


static int prism2_init_dev(local_info_t *local)
{
	struct net_device *dev = local->dev;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0))
	{
		int i = 0;
		do {
			sprintf(dev->name, "eth%d", i++);
		} while (dev_get(dev->name));
	}
#else
	memcpy(dev->name, "eth%d", 6);
#endif
	if (register_netdev(dev)) {
		printk(KERN_WARNING "%s: register_netdev() failed!\n",
		       dev_info);
		return 1;
	}
	printk(KERN_INFO "%s: Registered netdevice %s\n", dev_info, dev->name);

	hostap_init_proc(local);
#ifndef PRISM2_NO_PROCFS_DEBUG
	create_proc_read_entry("registers", 0, local->proc,
			       prism2_registers_proc_read, local);
#endif /* PRISM2_NO_PROCFS_DEBUG */
	hostap_init_data(local);

	return 0;
}


static void prism2_free_local_data(local_info_t *local)
{
	prism2_wds_info_t *wds, *prev;
	struct hostap_tx_callback_info *tx_cb, *tx_cb_prev;
	int i;
	struct sk_buff *skb;

	if (local == NULL)
		return;

	if (timer_pending(&local->crypt_deinit_timer))
		del_timer(&local->crypt_deinit_timer);
	prism2_crypt_deinit_entries(local, 1);

	if (timer_pending(&local->passive_scan_timer))
		del_timer(&local->passive_scan_timer);

	if (timer_pending(&local->tick_timer))
		del_timer(&local->tick_timer);

	prism2_clear_cmd_queue(local);

	while ((skb = skb_dequeue(&local->info_list)) != NULL)
		dev_kfree_skb(skb);

	while ((skb = skb_dequeue(&local->tx_callback_list)) != NULL)
		dev_kfree_skb(skb);

	while ((skb = skb_dequeue(&local->rx_list)) != NULL)
		dev_kfree_skb(skb);

	while ((skb = skb_dequeue(&local->sta_tx_exc_list)) != NULL)
		dev_kfree_skb(skb);

	if (local->dev_enabled)
		prism2_callback(local, PRISM2_CALLBACK_DISABLE);

	if (local->crypt) {
		if (local->crypt->ops)
			local->crypt->ops->deinit(local->crypt->priv);
		kfree(local->crypt);
		local->crypt = NULL;
	}

	if (local->ap != NULL)
		hostap_free_data(local->ap);

#ifndef PRISM2_NO_PROCFS_DEBUG
	if (local->proc != NULL)
		remove_proc_entry("registers", local->proc);
#endif /* PRISM2_NO_PROCFS_DEBUG */
	hostap_remove_proc(local);

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

	hostap_set_hostapd(local, 0, 0);

	if (local->dev && local->dev->name && local->dev->name[0]) {
		unregister_netdev(local->dev);
		printk(KERN_INFO "%s: Netdevice %s unregistered\n",
		       dev_info, local->dev->name);
	}
	kfree(local->dev);

	for (i = 0; i < PRISM2_FRAG_CACHE_LEN; i++) {
		if (local->frag_cache[i].skb != NULL)
			dev_kfree_skb(local->frag_cache[i].skb);
	}

	kfree(local->ap);
#if defined(PRISM2_PCI) && defined(PRISM2_BUS_MASTER)
	kfree(local->bus_m0_buf);
#endif /* PRISM2_PCI and PRISM2_BUS_MASTER */
	kfree(local->pda);
	kfree(local->last_scan_results);
	kfree(local);
}


/* These might at some point be compiled separately and used as separate
 * kernel modules or linked into one */
#ifdef PRISM2_DOWNLOAD_SUPPORT
#include "hostap_download.c"
#endif /* PRISM2_DOWNLOAD_SUPPORT */

#ifdef PRISM2_CALLBACK
/* External hostap_callback.c file can be used to, e.g., blink activity led.
 * This can use platform specific code and must define prism2_callback()
 * function (if PRISM2_CALLBACK is not defined, these function calls are not
 * used. */
#include "hostap_callback.c"
#endif /* PRISM2_CALLBACK */
