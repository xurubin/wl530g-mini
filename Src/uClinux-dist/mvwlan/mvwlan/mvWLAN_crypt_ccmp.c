/*
 * Host AP crypt: host-based CCMP encryption implementation for Host AP driver
 *
 * Copyright (c) 2002, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */
#ifdef AP_WPA2

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
#include "wl_macros.h"
#include "wlmac.h"
#include "mcu.h"
#include "memmap.h"
#include "ccmp.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_hw.h"
#include "mvWLAN_crypt.h"


#define CCMP_HW_ENCR     1


extern MRVL_MIB_RSN_GRP_KEY mib_MrvlRSN_GrpKey;


static void *mvWLAN_ccmp_init(void);
static void  mvWLAN_ccmp_deinit(void *priv);
static int   mvWLAN_ccmp_encrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv, int broadcast);
static int   mvWLAN_ccmp_decrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv);
static void  mvWLAN_ccmp_flush_dcache(UINT32 start, UINT32 end);


static struct mvwlan_crypto_ops mvwlan_crypt_ccmp = {
	.name			= "CCMP",
	.init			= mvWLAN_ccmp_init,
	.deinit			= mvWLAN_ccmp_deinit,
	.encrypt_mpdu		= mvWLAN_ccmp_encrypt_mpdu,
	.decrypt_mpdu		= mvWLAN_ccmp_decrypt_mpdu,
	.encrypt_msdu		= NULL,
	.decrypt_msdu		= NULL,
	.set_key		= NULL,
	.get_key		= NULL,
	.set_key_idx		= NULL,
	.get_key_idx		= NULL,
	.extra_prefix_len	= 4 + 4 /* IV + EIV */,
	.extra_postfix_len	= 8 /* MIC */
};


int mvWLAN_crypto_ccmp_init(void)
{
	if (mvWLAN_register_crypto_ops(&mvwlan_crypt_ccmp) < 0)
		return -1;

	return 0;
}


static void * mvWLAN_ccmp_init(void)
{
	return NULL;
}


static void mvWLAN_ccmp_deinit(void *priv)
{
	return;
}


static int mvWLAN_ccmp_encrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv, int broadcast)
{
	IEEEtypes_Frame_t *tx_frame = (IEEEtypes_Frame_t *) buf;
	UINT8 *pEncrKey;
	local_info_t *local = NULL;
	struct sta_info *sta = NULL;

	local_irq_disable();

	if (broadcast) {

		local = (local_info_t *) priv;

		if (local == NULL) {

			local_irq_enable();
			return -1;
		}

		pEncrKey = mib_MrvlRSN_GrpKey.EncryptKey;

		InsertCCMPHdr(tx_frame->Body,
			1,
			mib_MrvlRSN_GrpKey.g_IV16,
			mib_MrvlRSN_GrpKey.g_IV32);

		mib_MrvlRSN_GrpKey.g_IV16++;
		if ( mib_MrvlRSN_GrpKey.g_IV16 == 0 )
			mib_MrvlRSN_GrpKey.g_IV32++;

	} else {

		sta = (struct sta_info *) priv;
		local = sta->local;

		if ((sta == NULL) || (local == NULL)) {

			local_irq_enable();
			return -1;
		}

		if (sta->key_mgmt_info.RSNDataTrafficEnabled) {

			pEncrKey = sta->key_mgmt_info.PairwiseTempKey1;

			InsertCCMPHdr(tx_frame->Body,
				0,
				sta->key_mgmt_info.TxIV16,
				sta->key_mgmt_info.TxIV32);

			sta->key_mgmt_info.TxIV16++;
			if (sta->key_mgmt_info.TxIV16 == 0)
				sta->key_mgmt_info.TxIV32++;

		} else {

			local_irq_enable();
			return -1;
		}

	}


#if CCMP_HW_ENCR

	DoCCMPEncrypt(tx_frame, len, pEncrKey, obuf, mvWLAN_ccmp_flush_dcache);

#ifdef CCMP_NO_WAIT
	local->ccmp_encry_delay_wait = 1;
#endif

#else

	{

	UINT32 MIC[4], EncrKey[44];
	UINT8 MICIV[16], MICHdr1[16], MICHdr2[16], CCMCtrNonce[16];

	memcpy(&tx_frame->Body[IV_SIZE + EIV_SIZE], obuf, len);
	MakeCCMPCfgRegs(tx_frame, MICIV, MICHdr1, MICHdr2, CCMCtrNonce, len);
	AES_SetKey((UINT32 *) pEncrKey, EncrKey);
	GenerateMIC(MICIV, MICHdr1, MICHdr2, CCMCtrNonce,
		&tx_frame->Body[IV_SIZE + EIV_SIZE], MIC, len, EncrKey);
	GenerateEncrData(CCMCtrNonce, &tx_frame->Body[IV_SIZE + EIV_SIZE],
		&tx_frame->Body[IV_SIZE + EIV_SIZE], len, EncrKey);
	memcpy(&tx_frame->Body[IV_SIZE + EIV_SIZE + len], MIC, MIC_SIZE);

	}

#endif

	local_irq_enable();

	return (len + IV_SIZE + EIV_SIZE + MIC_SIZE);

}


static int mvWLAN_ccmp_decrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv)
{
	IEEEtypes_Frame_t *rx_frame = (IEEEtypes_Frame_t *) buf;
	UINT32 pKey;
	local_info_t *local = NULL;
	struct sta_info *sta = NULL;

	local_irq_disable();

	sta = (struct sta_info *) priv;
	local = sta->local;

	if ((sta == NULL) || (local == NULL)) {
		local_irq_enable();
		return -1;
	}

	if (!sta->key_mgmt_info.RSNDataTrafficEnabled) {
		local_irq_enable();
		return -1;
	}

	if (IS_GROUP(rx_frame->Hdr.Addr1)) {

		pKey = mib_MrvlRSN_GrpKey.EncryptKey;
	} else {

		pKey = (UINT32 *) sta->key_mgmt_info.PairwiseTempKey1;
	}

	len -= (IV_SIZE + EIV_SIZE + MIC_SIZE);

#ifdef CCMP_HW_ENCR

	if (DoCCMPDecrypt(rx_frame, rx_frame->Body, len, pKey, mvWLAN_ccmp_flush_dcache) == 0) /* ICV match failure */
	{
		printk("Decrypt Frame Error\n");
		local->sysConfig->Mib802dot11->RSNStats.TKIPICVErrors++; /* Commented out by Rahul */
		local_irq_enable();
		return -1;
	}

#else

	{

	UINT8  rx_MIC[MIC_SIZE];
	UINT32 calc_MIC[4];
	UINT32 status, EncrKey[44];
	UINT8  MICIV[16], MICHdr1[16], MICHdr2[16], CCMCtrNonce[16];

	{
	int i;
	UINT8 *header = (UINT8 *) &rx_frame->Hdr;

	printk("Frame Header:\n");
	for (i = 0; i < sizeof(rx_frame->Hdr); i++) printk("%02x ", header[i]);
	printk("\n");
	printk("Frame Body:\n");
	for (i = 0; i < len + 16; i++) printk("%02x ", rx_frame->Body[i]);
	printk("\n");
	}

	AES_SetKey(pKey, EncrKey);
	MakeMICIV(MICIV, rx_frame, len);
	MakeMICHdr1(MICHdr1, &rx_frame->Hdr);
	MakeMICHdr2(MICHdr2, &rx_frame->Hdr);
	MakeCCMCounterNonce(CCMCtrNonce, rx_frame);
	memcpy(rx_MIC, &rx_frame->Body[IV_SIZE + EIV_SIZE + len], 8);
	GenerateEncrData(CCMCtrNonce, &rx_frame->Body[IV_SIZE + EIV_SIZE],
		&rx_frame->Body[0], len, EncrKey);

	GenerateMIC(MICIV, MICHdr1, MICHdr2, CCMCtrNonce, &rx_frame->Body[0],
		(UINT8 *) calc_MIC, len, EncrKey);

	status = CheckMIC(calc_MIC, rx_MIC);

	if (status == 0) {
		local->sysConfig->Mib802dot11->RSNStats.TKIPICVErrors++; /* Commented out by Rahul */
		local_irq_enable();
		return -1;
	}

	{
	int i;

	printk("After Decrypt:\n");
	for (i = 0; i < len; i++) printk("%02x ", rx_frame->Body[i]);
	printk("\n");
	}

	}

#endif

	local_irq_enable();

	return len;

}


static void  mvWLAN_ccmp_flush_dcache(UINT32 start, UINT32 end)
{
	flush_dcache_range((u32)start, (u32)end);
}


EXPORT_SYMBOL(mvWLAN_crypto_ccmp_init);

#endif