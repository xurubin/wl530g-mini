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
#include "mvWLAN.h"
#include "mvWLAN_hw.h"


static inline int mvWLAN_wds_special_addr(u8 *addr)
{
	if (addr[0] || addr[1] || addr[2] || addr[3] || addr[4] || addr[5])
		return 0;

	return 1;
}


/* Called only as a tasklet (software IRQ) */
static int add_sta(struct ap_data *ap, u8 *sta_addr)
{
	struct sta_info *sta;
	int ret = 1;

	if (!ap)
		return -1;

	spin_lock(&ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(ap, sta_addr);
	if (sta)
		ret = 0;
	spin_unlock(&ap->sta_table_lock);

	if (ret == 1) {
		sta = mvWLAN_ap_add_sta(ap, sta_addr);
		if (!sta)
			ret = -1;
		sta->flags = WLAN_STA_AUTH | WLAN_STA_ASSOC;
		sta->ap = 1;
		memset(sta->supported_rates, 0, sizeof(sta->supported_rates));
		/* No way of knowing which rates are supported since we did not
		 * get supported rates element from beacon/assoc req. Assume
		 * that remote end supports all 802.11b rates. */
		sta->supported_rates[0] = 0x82;
		sta->supported_rates[1] = 0x84;
		sta->supported_rates[2] = 0x0b;
		sta->supported_rates[3] = 0x16;
		sta->tx_supp_rates = WLAN_RATE_1M | WLAN_RATE_2M |
			WLAN_RATE_5M5 | WLAN_RATE_11M;
		sta->tx_rate = 110;
		sta->tx_max_rate = sta->tx_rate_idx = 3;
	}

	return ret;
}

int mvWLAN_wds_is_wds_link(local_info_t *local, u8 *remote_addr)
{
    mvwlan_wds_info_t *wds = NULL;

    wds = local->wds;
    
	while (wds != NULL && memcmp(wds->remote_addr, remote_addr, ETH_ALEN) != 0) {		
	    wds = wds->next;
	}

    if (wds)
        return 0;
    else
        return 1;    
}

/* update the peer AP's info from listened its Beacon */
void mvWLAN_wds_update_sta_info(local_info_t *local, u8* remote_addr, IEEEtypes_SuppRatesElement_t *Rates_p, IEEEtypes_ExtSuppRatesElement_t *ExtRates_p, IEEEtypes_CapInfo_t CapInfo)
{
     mvwlan_wds_info_t *wds = NULL;
	 unsigned long flags;
     struct sta_info *sta = NULL;
	
	
	 wds = local->wds;
	 while (wds != NULL && memcmp(wds->remote_addr, remote_addr, ETH_ALEN) != 0) {		
	     wds = wds->next;
	 }
    
    if (!wds)
        return;

    spin_lock(&local->ap->sta_table_lock);
	sta = mvWLAN_ap_get_sta(local->ap, remote_addr);
	if (!sta)
	{
        /* station record not found */
        printk(KERN_ALERT "sta::%02x:%02x:%02x:%02x:%02x:%02x not fould in sta db\n", remote_addr[0],
                                                                  remote_addr[1],
                                                                  remote_addr[2],
                                                                  remote_addr[3],
                                                                  remote_addr[4],
                                                                  remote_addr[5]);  
        
        spin_unlock(&local->ap->sta_table_lock);
        return;
	}

    /*start to updat the station info */    
    sta->capability = *((UINT16 *)&CapInfo);
    memcpy(sta->supported_rates, Rates_p->Rates, Rates_p->Len);
    memcpy(&sta->supported_rates[Rates_p->Len], ExtRates_p->Rates, ExtRates_p->Len);

    ap_check_tx_rates(sta);
	spin_unlock(&local->ap->sta_table_lock);
  
    return;
}

int mvWLAN_wds_add(local_info_t *local, u8 *remote_addr, int rtnl_locked)
{
	mvwlan_wds_info_t *wds, *wds2 = NULL;
	unsigned long flags;
	int i, ret;

	spin_lock_irqsave(&local->wdslock, flags);
	wds = local->wds;
	while (wds != NULL &&
	       memcmp(wds->remote_addr, remote_addr, ETH_ALEN) != 0) {
		if (!wds2 && mvWLAN_wds_special_addr(wds->remote_addr))
			wds2 = wds;
		wds = wds->next;
	}
	if (!wds && wds2) {
		/* take pre-allocated entry into use */
		memcpy(wds2->remote_addr, remote_addr, ETH_ALEN);
	}
	spin_unlock_irqrestore(&local->wdslock, flags);

	if (!mvWLAN_wds_special_addr(remote_addr)) {
		if (wds)
			return -EEXIST;
		add_sta(local->ap, remote_addr);
	}

	if (!wds && wds2) {
		printk(KERN_DEBUG "%s: using pre-allocated WDS netdevice %s\n",
		       local->dev->name, wds2->dev.name);
		return 0;
	}

	if (local->wds_connections >= local->wds_max_connections)
		return -ENOBUFS;

	/* verify that there is room for wds# postfix in the interface name */
	if (strlen(local->dev->name) > IFNAMSIZ - 5) {
		printk(KERN_DEBUG "'%s' too long base device name\n",
		       local->dev->name);
		return -EINVAL;
	}

	wds = (mvwlan_wds_info_t *) kmalloc(sizeof(*wds) + MVWLAN_NETDEV_EXTRA,
					    GFP_ATOMIC);
	if (wds == NULL)
		return -ENOMEM;

	memset(wds, 0, sizeof(*wds) + MVWLAN_NETDEV_EXTRA);
	mvWLAN_set_dev_name(&wds->dev, wds + 1);

	memcpy(wds->remote_addr, remote_addr, ETH_ALEN);

	mvWLAN_setup_dev(&wds->dev, local, 0);

	wds->dev.priv = local;
	memcpy(wds->dev.dev_addr, local->dev->dev_addr, ETH_ALEN);
	wds->dev.base_addr = local->dev->base_addr;
	wds->dev.irq = local->dev->irq;
	wds->dev.mem_start = local->dev->mem_start;
	wds->dev.mem_end = local->dev->mem_end;

	i = 0;
	do {
		sprintf(wds->dev.name, "%swds%d", local->dev->name, i++);
	} while (i < 10000 && dev_get(wds->dev.name));

	if (rtnl_locked)
		ret = register_netdevice(&wds->dev);
	else
		ret = register_netdev(&wds->dev);

	if (ret) {
		printk(KERN_WARNING "%s: registering WDS device '%s' failed\n",
		       local->dev->name, wds->dev.name);
		kfree(wds);
		return -EINVAL;
	}

	spin_lock_irqsave(&local->wdslock, flags);
	local->wds_connections++;
	wds->next = local->wds;
	local->wds = wds;
	spin_unlock_irqrestore(&local->wdslock, flags);

	printk(KERN_DEBUG "%s: registered WDS netdevice %s\n",
	       local->dev->name, wds->dev.name);

	return 0;
}


int mvWLAN_wds_del(local_info_t *local, u8 *remote_addr, int rtnl_locked, 
		   int do_not_remove)
{
	mvwlan_wds_info_t *wds, *prev = NULL;
	unsigned long flags;

	spin_lock_irqsave(&local->wdslock, flags);
	wds = local->wds;
	while (wds != NULL &&
	       memcmp(wds->remote_addr, remote_addr, ETH_ALEN) != 0) {
		prev = wds;
		wds = wds->next;
	}
	if (wds && !do_not_remove) {
		if (prev != NULL)
			prev->next = wds->next;
		else
			local->wds = wds->next;
	}
	spin_unlock_irqrestore(&local->wdslock, flags);

	if (!wds)
		return -ENODEV;

	if (do_not_remove) {
		memset(wds->remote_addr, 0, ETH_ALEN);
		return 0;
	}

	if (rtnl_locked)
		unregister_netdevice(&wds->dev);
	else
		unregister_netdev(&wds->dev);
	local->wds_connections--;

	printk(KERN_DEBUG "%s: unregistered WDS netdevice %s\n",
	       local->dev->name, wds->dev.name);

	kfree(wds);

	return 0;
}


#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT

static void handle_add_wds_queue(void *data)
{
	local_info_t *local = data;
	struct add_wds_data *entry, *prev;

	spin_lock_bh(&local->lock);
	entry = local->ap->add_wds_entries;
	local->ap->add_wds_entries = NULL;
	spin_unlock_bh(&local->lock);

	while (entry) {
		PDEBUG(DEBUG_AP, "%s: adding automatic WDS connection "
		       "to AP " MACSTR "\n",
		       local->dev->name, MAC2STR(entry->addr));
		mvWLAN_wds_add(local, entry->addr, 0);

		prev = entry;
		entry = entry->next;
		kfree(prev);
	}

#ifndef NEW_MODULE_CODE
	MOD_DEC_USE_COUNT;
#endif
}

#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */


void mvWLAN_add_wds_links(local_info_t *local)
{
	struct ap_data *ap = local->ap;
	struct list_head *ptr;
	struct add_wds_data *entry;

	spin_lock_bh(&ap->sta_table_lock);
	for (ptr = ap->sta_list.next; ptr != &ap->sta_list; ptr = ptr->next) {
		struct sta_info *sta = (struct sta_info *) ptr;
		if (!sta->ap)
			continue;
		entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
		if (!entry)
			break;
		memcpy(entry->addr, sta->addr, ETH_ALEN);
		spin_lock_bh(&local->lock);
		entry->next = local->ap->add_wds_entries;
		local->ap->add_wds_entries = entry;
		spin_unlock_bh(&local->lock);
	}
	spin_unlock_bh(&ap->sta_table_lock);

	MVWLAN_SCHEDULE_TASK(&local->ap->add_wds_queue);
}


void mvWLAN_add_wds_link(local_info_t *local, u8 *addr)
{
	struct add_wds_data *entry;

	entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
	if (!entry)
		return;
	memcpy(entry->addr, addr, ETH_ALEN);
	spin_lock_bh(&local->lock);
	entry->next = local->ap->add_wds_entries;
	local->ap->add_wds_entries = entry;
	spin_unlock_bh(&local->lock);

	MVWLAN_SCHEDULE_TASK(&local->ap->add_wds_queue);
}


void mvWLAN_init_wds(local_info_t *local)
{
#ifndef MVWLAN_NO_KERNEL_IEEE80211_MGMT
	MVWLAN_QUEUE_INIT(&local->ap->add_wds_queue,
			  handle_add_wds_queue, local);
#endif /* MVWLAN_NO_KERNEL_IEEE80211_MGMT */
}


EXPORT_SYMBOL(mvWLAN_add_wds_links);
EXPORT_SYMBOL(mvWLAN_add_wds_link);
EXPORT_SYMBOL(mvWLAN_init_wds);
