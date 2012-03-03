/*
 * Host AP crypto routines
 *
 * Copyright (c) 2002, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
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
#include <asm/uaccess.h>

#include "IEEE_types.h"
#include "wl_mib.h"
#include "wl_hal.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_crypt.h"


extern int mvWLAN_crypto_wep_init(void);
extern int mvWLAN_crypto_tkip_init(void);
#ifdef AP_WPA2
extern int mvWLAN_crypto_ccmp_init(void);
#endif


struct mvwlan_crypto_alg {
	struct list_head list;
	struct mvwlan_crypto_ops *ops;
};


struct mvwlan_crypto {
	struct list_head algs;
	spinlock_t lock;
};

static struct mvwlan_crypto *hcrypt;


int mvWLAN_register_crypto_ops(struct mvwlan_crypto_ops *ops)
{
	unsigned long flags;
	struct mvwlan_crypto_alg *alg;

	if (hcrypt == NULL)
		return -1;

	alg = (struct mvwlan_crypto_alg *) kmalloc(sizeof(*alg), GFP_KERNEL);
	if (alg == NULL)
		return -ENOMEM;

	memset(alg, 0, sizeof(*alg));
	alg->ops = ops;

	spin_lock_irqsave(&hcrypt->lock, flags);
	list_add(&alg->list, &hcrypt->algs);
	spin_unlock_irqrestore(&hcrypt->lock, flags);

	printk(KERN_DEBUG "mvWLAN_crypt: registered algorithm '%s'\n",
	       ops->name);

	return 0;
}


int mvWLAN_unregister_crypto_ops(struct mvwlan_crypto_ops *ops)
{
	unsigned long flags;
	struct list_head *ptr;
	struct mvwlan_crypto_alg *del_alg = NULL;

	if (hcrypt == NULL)
		return -1;

	spin_lock_irqsave(&hcrypt->lock, flags);
	for (ptr = hcrypt->algs.next; ptr != &hcrypt->algs; ptr = ptr->next) {
		struct mvwlan_crypto_alg *alg =
			(struct mvwlan_crypto_alg *) ptr;
		if (alg->ops == ops) {
			list_del(&alg->list);
			del_alg = alg;
			break;
		}
	}
	spin_unlock_irqrestore(&hcrypt->lock, flags);

	if (del_alg) {
		printk(KERN_DEBUG "mvWLAN_crypt: unregistered algorithm "
		       "'%s'\n", ops->name);
		kfree(del_alg);
	}

	return del_alg ? 0 : -1;
}


struct mvwlan_crypto_ops * mvWLAN_get_crypto_ops(const char *name)
{
	unsigned long flags;
	struct list_head *ptr;
	struct mvwlan_crypto_alg *found_alg = NULL;

	if (hcrypt == NULL)
		return NULL;

	spin_lock_irqsave(&hcrypt->lock, flags);
	for (ptr = hcrypt->algs.next; ptr != &hcrypt->algs; ptr = ptr->next) {
		struct mvwlan_crypto_alg *alg =
			(struct mvwlan_crypto_alg *) ptr;
		if (strcmp(alg->ops->name, name) == 0) {
			found_alg = alg;
			break;
		}
	}
	spin_unlock_irqrestore(&hcrypt->lock, flags);

	if (found_alg)
		return found_alg->ops;
	else
		return NULL;
}


static int crypto_init(void)
{
	hcrypt = (struct mvwlan_crypto *) kmalloc(sizeof(*hcrypt), GFP_KERNEL);
	if (hcrypt == NULL)
		return -ENOMEM;

	memset(hcrypt, 0, sizeof(*hcrypt));
	INIT_LIST_HEAD(&hcrypt->algs);
	spin_lock_init(&hcrypt->lock);

	if (mvWLAN_crypto_wep_init() != 0)
		return -ENOMEM;

	if (mvWLAN_crypto_tkip_init() != 0)
		return -ENOMEM;

#ifdef AP_WPA2
	if (mvWLAN_crypto_ccmp_init() != 0)
		return -ENOMEM;
#endif

	return 0;
}


static void crypto_deinit(void)
{
	struct list_head *ptr, *n;

	if (hcrypt == NULL)
		return;

	for (ptr = hcrypt->algs.next, n = ptr->next; ptr != &hcrypt->algs;
	     ptr = n, n = ptr->next) {
		struct mvwlan_crypto_alg *alg =
			(struct mvwlan_crypto_alg *) ptr;
		list_del(ptr);
		printk(KERN_DEBUG "mvWLAN_crypt: unregistered algorithm "
		       "'%s' (deinit)\n", alg->ops->name);
		kfree(alg);
	}

	kfree(hcrypt);
}


static void crypt_deinit_entries(local_info_t *local, int force)
{
	struct list_head *ptr, *n;
	struct mvwlan_crypt_data *entry;

	for (ptr = local->crypt_deinit_list.next, n = ptr->next;
	     ptr != &local->crypt_deinit_list; ptr = n, n = ptr->next) {
		entry = list_entry(ptr, struct mvwlan_crypt_data, list);

		if (atomic_read(&entry->refcnt) != 0 && !force)
			continue;

		list_del(ptr);

		if (entry->ops)
			entry->ops->deinit(entry->priv);
		kfree(entry);
	}
}


static void crypt_deinit_handler(unsigned long data)
{
	local_info_t *local = (local_info_t *) data;
	unsigned long flags;

	spin_lock_irqsave(&local->lock, flags);

	crypt_deinit_entries(local, 0);
	if (!list_empty(&local->crypt_deinit_list)) {
		printk(KERN_DEBUG "%s: entries remaining in delayed crypt "
		       "deletion list\n", local->dev->name);
		local->crypt_deinit_timer.expires = jiffies + HZ;
		add_timer(&local->crypt_deinit_timer);
	}

	spin_unlock_irqrestore(&local->lock, flags);
}


void mvWLAN_init_crypt_lib(local_info_t *local)
{
	crypto_init();

	INIT_LIST_HEAD(&local->crypt_deinit_list);
	init_timer(&local->crypt_deinit_timer);
	local->crypt_deinit_timer.data = (unsigned long) local;
	local->crypt_deinit_timer.function = crypt_deinit_handler;
}


void mvWLAN_deinit_crypt_lib(local_info_t *local)
{
	if (timer_pending(&local->crypt_deinit_timer))
		del_timer(&local->crypt_deinit_timer);
	crypt_deinit_entries(local, 1);

#ifndef AP_WPA2
	if (local->crypt) {
		if (local->crypt->ops)
			local->crypt->ops->deinit(local->crypt->priv);
		kfree(local->crypt);
		local->crypt = NULL;
	}
#else
	if (local->sysConfig->Mib802dot11->Privacy.RSNEnabled) {

		if (local->tkip_crypt) {
			if (local->tkip_crypt->ops)
				local->tkip_crypt->ops->deinit(local->tkip_crypt->priv);
			kfree(local->tkip_crypt);
			local->tkip_crypt = NULL;
		}

		if (local->ccmp_crypt) {
			if (local->ccmp_crypt->ops)
				local->ccmp_crypt->ops->deinit(local->ccmp_crypt->priv);
			kfree(local->ccmp_crypt);
			local->ccmp_crypt = NULL;
		}

		local->crypt = local->multicast_crypt = NULL;

	} else {

		if (local->crypt) {
			if (local->crypt->ops)
				local->crypt->ops->deinit(local->crypt->priv);
			kfree(local->crypt);
			local->crypt = NULL;
		}

	}
#endif

	crypto_deinit();
}


EXPORT_SYMBOL(mvWLAN_register_crypto_ops);
EXPORT_SYMBOL(mvWLAN_unregister_crypto_ops);
EXPORT_SYMBOL(mvWLAN_get_crypto_ops);
EXPORT_SYMBOL(mvWLAN_init_crypt_lib);
EXPORT_SYMBOL(mvWLAN_deinit_crypt_lib);

