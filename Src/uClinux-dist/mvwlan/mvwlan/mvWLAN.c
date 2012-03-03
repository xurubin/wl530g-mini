/*******************************************************************************
*                Copyright 2000 ~ 2003, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
*******************************************************************************/
/*******************************************************************************
* mvWLAN.c
*
* DESCRIPTION:
*       Marvell Host AP (software wireless LAN access point) driver for
*       Libertas 88W85x0 mvwlan.o module, common routines
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: #1 $
*
*******************************************************************************/

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
#include <asm/uaccess.h>


MODULE_AUTHOR("Marvell");
MODULE_DESCRIPTION("Wireless LAN Driver");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

#include "wltypes.h"
#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"
#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_hw.h"
#include "mvWLAN_crypt.h"


#define TX_TIMEOUT              (2 * HZ)


#if WIRELESS_EXT > 12
extern struct iw_handler_def mvwlan_iw_handler_def;
#endif /* WIRELESS_EXT > 12 */


int mvWLAN_80211_header_parse(struct sk_buff *skb, unsigned char *haddr)
{
	memcpy(haddr, skb->mac.raw + 10, ETH_ALEN); /* addr2 */
	return ETH_ALEN;
}


int mvWLAN_80211_get_hdrlen(u16 fc)
{
	int hdrlen = 24;

	switch (WLAN_FC_GET_TYPE(fc)) {
	case WLAN_FC_TYPE_DATA:
		if ((fc & WLAN_FC_FROMDS) && (fc & WLAN_FC_TODS))
			hdrlen = 30; /* Addr4 */
		break;
	case WLAN_FC_TYPE_CTRL:
		switch (WLAN_FC_GET_STYPE(fc)) {
		case WLAN_FC_STYPE_CTS:
		case WLAN_FC_STYPE_ACK:
			hdrlen = 10;
			break;
		default:
			hdrlen = 16;
			break;
		}
		break;
	}

	return hdrlen;
}


static struct iw_statistics *mvWLAN_get_wireless_stats(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;
	MIB_COUNTERS *mib_Counters_p = &(local->sysConfig->Mib802dot11->CountersTable);

	local->wstats.status = 0;
	local->wstats.discard.code = mib_Counters_p->WepUndecryptCnt;
	local->wstats.discard.misc = mib_Counters_p->FcsErrCnt;

#if WIRELESS_EXT > 11
	local->wstats.discard.retries = mib_Counters_p->FailedCnt;
	local->wstats.discard.fragment = 0;
#endif /* WIRELESS_EXT > 11 */

	if (local->iw_mode != IW_MODE_MASTER &&
	    local->iw_mode != IW_MODE_REPEAT) {
		/* XXX Need to get these data from RX descriptor */
		local->wstats.qual.qual = 0;
		local->wstats.qual.level = 0;
		local->wstats.qual.noise = 0;
		local->wstats.qual.updated = 0;
	}

	return &local->wstats;
}


struct net_device_stats *mvWLAN_get_stats(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;

	if (local->apdev == dev)
		return &local->apdevstats;
	if (local->stadev == dev)
		return &local->stadevstats;
	if (local->dev != dev) {
		mvwlan_wds_info_t *wds = (mvwlan_wds_info_t *) dev;
		return &wds->stats;
	}

	return &local->stats;
}


/* wake all netif queues in use */
void mvWLAN_netif_wake_queues(local_info_t *local)
{
	mvwlan_wds_info_t *wds;
	unsigned long flags;

	if (local->dev)
		netif_wake_queue(local->dev);
	if (local->apdev)
		netif_wake_queue(local->apdev);
	if (local->stadev)
		netif_wake_queue(local->stadev);

	spin_lock_irqsave(&local->wdslock, flags);
	wds = local->wds;
	while (wds != NULL) {
		netif_wake_queue(&wds->dev);
		wds = wds->next;
	}
	spin_unlock_irqrestore(&local->wdslock, flags);
}


/* stop all netif queues in use */
void mvWLAN_netif_stop_queues(local_info_t *local)
{
	mvwlan_wds_info_t *wds;
	unsigned long flags;

	if (local->dev)
		netif_stop_queue(local->dev);
	if (local->apdev)
		netif_stop_queue(local->apdev);
	if (local->stadev)
		netif_stop_queue(local->stadev);

	spin_lock_irqsave(&local->wdslock, flags);
	wds = local->wds;
	while (wds != NULL) {
		netif_stop_queue(&wds->dev);
		wds = wds->next;
	}
	spin_unlock_irqrestore(&local->wdslock, flags);
}


static int mvWLAN_close(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;

	PDEBUG(DEBUG_FLOW, "%s: mvWLAN_close\n", dev->name);

#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	if (!local->hostapd && dev == local->dev &&
	    local->hw_ready && local->ap && local->iw_mode == IW_MODE_MASTER)
		mvWLAN_deauth_all_stas(dev, local->ap, 1);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */

	if (local->func->dev_close && local->func->dev_close(dev))
		return 0;

	if (local->disable_on_close) {
		local->func->hw_shutdown(dev, 1);
	}

	if (netif_running(dev)) {
		netif_stop_queue(dev);
		netif_device_detach(dev);
	}

	MVWLAN_FLUSH_SCHEDULED_TASKS();

#ifdef NEW_MODULE_CODE
	module_put(local->hw_module);
#elif MODULE
	__MOD_DEC_USE_COUNT(local->hw_module);
#endif

	return 0;
}


static int mvWLAN_open(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;

	PDEBUG(DEBUG_FLOW, "%s: mvWLAN_open\n", dev->name);

	if (local->func->dev_open && local->func->dev_open(dev))
		return 1;

#ifdef NEW_MODULE_CODE
	if (!try_module_get(local->hw_module))
		return -ENODEV;
#elif MODULE
	__MOD_INC_USE_COUNT(local->hw_module);
#endif

	if (!local->dev_enabled && local->func->hw_enable(dev, 1)) {
		printk(KERN_WARNING "%s: could not enable MAC port\n",
		       dev->name);
		mvWLAN_close(dev);
		return 1;
	}
	if (!local->dev_enabled)
		mvWLAN_callback(local, MVWLAN_CALLBACK_ENABLE);
	local->dev_enabled = 1;

	netif_device_attach(dev);
	netif_start_queue(dev);

	return 0;
}


#ifdef HAVE_SET_MAC_ADDR
static int mvWLAN_set_mac_address(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;
	local_info_t *local = (local_info_t *) dev->priv;
	mvwlan_wds_info_t *wds;
	unsigned long flags;

	if (local->func->set_cmd(dev, APCMD_SET_STATION_ID, 0, 
				 addr->sa_data, ETH_ALEN) < 0)
		return -EINVAL;

	dev = local->dev;
	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	if (local->apdev)
		memcpy(local->apdev->dev_addr, dev->dev_addr, ETH_ALEN);
	if (local->stadev)
		memcpy(local->stadev->dev_addr, dev->dev_addr, ETH_ALEN);
	spin_lock_irqsave(&local->wdslock, flags);
	wds = local->wds;
	while (wds != NULL) {
		memcpy(wds->dev.dev_addr, dev->dev_addr, ETH_ALEN);
		wds = wds->next;
	}
	spin_unlock_irqrestore(&local->wdslock, flags);

	return 0;
}
#endif /* HAVE_SET_MAC_ADDR */


static void mvWLAN_set_multicast_list(struct net_device *dev)
{
#if 0
	/* FIX: promiscuous mode seems to be causing a lot of problems with
	 * some station firmware versions (FCSErr frames, invalid MACPort, etc.
	 * corrupted incoming frames). This code is now commented out while the
	 * problems are investigated. */
	local_info_t *local = (local_info_t *) dev->priv;

	if ((dev->flags & IFF_ALLMULTI) || (dev->flags & IFF_PROMISC)) {
		local->is_promisc = 1;
	} else {
		local->is_promisc = 0;
	}

	MVWLAN_SCHEDULE_TASK(&local->set_multicast_list_queue);
#endif
}


static int mvWLAN_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < MVWLAN_MIN_MTU || new_mtu > MVWLAN_MAX_MTU)
		return -EINVAL;

	dev->mtu = new_mtu;

	return 0;
}


#ifdef HAVE_TX_TIMEOUT
static void mvWLAN_tx_timeout(struct net_device *dev)
{
	local_info_t *local = (local_info_t *) dev->priv;

	printk(KERN_WARNING "%s Tx timed out! Resetting MAC Tx\n", dev->name);
	mvWLAN_netif_stop_queues(local);

	local->func->schedule_reset(local);
}
#endif /* HAVE_TX_TIMEOUT */


void mvWLAN_setup_dev(struct net_device *dev, local_info_t *local,
		      int main_dev)
{
	ether_setup(dev);

	/* kernel callbacks */
	dev->get_stats = mvWLAN_get_stats;
#ifdef WIRELESS_EXT
	dev->get_wireless_stats = main_dev ? mvWLAN_get_wireless_stats : NULL;
#if WIRELESS_EXT > 12
	dev->wireless_handlers =
		(struct iw_handler_def *) &mvwlan_iw_handler_def;
#endif /* WIRELESS_EXT > 12 */
#ifdef HAVE_PRIVATE_IOCTL
	dev->do_ioctl = main_dev ? mvWLAN_ioctl : NULL;
#endif
#endif /* WIRELESS_EXT */
	dev->open = mvWLAN_open;
	dev->stop = mvWLAN_close;
	if (local->func)
		dev->hard_start_xmit = local->func->tx;
	else
		printk(KERN_WARNING "mvWLAN_setup_dev: local->func == NULL\n");
#ifdef HAVE_SET_MAC_ADDR
	dev->set_mac_address = mvWLAN_set_mac_address;
#endif /* HAVE_SET_MAC_ADDR */
#ifdef HAVE_MULTICAST
	dev->set_multicast_list = mvWLAN_set_multicast_list;
#endif
#ifdef HAVE_CHANGE_MTU
	dev->change_mtu = mvWLAN_change_mtu;
#endif
#ifdef HAVE_TX_TIMEOUT
	dev->tx_timeout = mvWLAN_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
#endif

	dev->mtu = local->mtu;

	netif_stop_queue(dev);
}


static int __init mvWLAN_init(void)
{
	mvWLAN_init_proc();
	return 0;
}


static void __exit mvWLAN_exit(void)
{
	mvWLAN_remove_proc();
}


EXPORT_SYMBOL(mvWLAN_80211_header_parse);
EXPORT_SYMBOL(mvWLAN_80211_get_hdrlen);
EXPORT_SYMBOL(mvWLAN_get_stats);
EXPORT_SYMBOL(mvWLAN_netif_wake_queues);
EXPORT_SYMBOL(mvWLAN_netif_stop_queues);
EXPORT_SYMBOL(mvWLAN_setup_dev);

module_init(mvWLAN_init);
module_exit(mvWLAN_exit);
