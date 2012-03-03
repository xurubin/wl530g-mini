/*
 * Host AP crypt: host-based TKIP encryption implementation for Host AP driver
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
#include "wl_macros.h"
#include "wlmac.h"
#include "mcu.h"
#include "memmap.h"

#include "mvWLAN_wlan.h"
#include "mvWLAN_80211.h"
#include "mvWLAN_mgmt.h"
#include "mvWLAN.h"
#include "mvWLAN_hw.h"
#include "mvWLAN_crypt.h" 


#define MACADDR_CPY(macaddr1,macaddr2) { *(UINT16*)macaddr1 = *(UINT16*)macaddr2; \
                    *(UINT16 *)((UINT16*)macaddr1+1) = *(UINT16 *)((UINT16*)macaddr2+1); \
                    *(UINT16 *)((UINT16*)macaddr1+2) = *(UINT16 *)((UINT16*)macaddr2+2);}


extern const UINT16 Sbox[2][256];
extern MRVL_MIB_RSN_GRP_KEY mib_MrvlRSN_GrpKey;
extern MIC_Error_t MIC_Error;


inline static UINT32 pad(UINT8 *data,UINT32 length);
inline static void Phase2(UINT8 *RC4KEY, const UINT8 *TK, const UINT16 *P1K, UINT16 IV16);
inline static void GenerateEncrKey(UINT8 *RC4Key,
                     UINT8 *TemporalKey, UINT32 IV32, UINT16 IV16,
                     IEEEtypes_MacAddr_t *pAddr, UINT16 *pPhase1Key);
inline static void GenerateDecrptKey(UINT8 *trans_addr, UINT8 *frame, UINT8 *pRC4Key,
                       keyMgmtInfo_t *pkeyInfo);
static void  mvWLAN_tkip_MIC_err_info(local_info_t *local);
static void *mvWLAN_tkip_init(void);
static void  mvWLAN_tkip_deinit(void *priv);
static int   mvWLAN_tkip_encrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv, int broadcast);
static int   mvWLAN_tkip_decrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv);
static int   mvWLAN_tkip_encrypt_msdu(u8 *da, u8 *sa, u8 *buf, int len, void *priv, int broadcast);
static int   mvWLAN_tkip_decrypt_msdu(u8 *da, u8 *sa, u8 *buf, int len, void *priv);


static struct mvwlan_crypto_ops mvwlan_crypt_tkip = {
	.name			= "TKIP",
	.init			= mvWLAN_tkip_init,
	.deinit			= mvWLAN_tkip_deinit,
	.encrypt_mpdu		= mvWLAN_tkip_encrypt_mpdu,
	.decrypt_mpdu		= mvWLAN_tkip_decrypt_mpdu,
	.encrypt_msdu		= mvWLAN_tkip_encrypt_msdu,
	.decrypt_msdu		= mvWLAN_tkip_decrypt_msdu,
	.set_key		= NULL,
	.get_key		= NULL,
	.set_key_idx		= NULL,
	.get_key_idx		= NULL,
	.extra_prefix_len	= 4 + 4 /* IV + ExtIV */,
	.extra_postfix_len	= 8 + 4 /* MIC + ICV */
};


int mvWLAN_crypto_tkip_init(void)
{
	if (mvWLAN_register_crypto_ops(&mvwlan_crypt_tkip) < 0)
		return -1;

	return 0;
}


inline void MichaelTx(UINT8 *da, UINT8 *sa, const UINT32 *Key,
	UINT32 *data, UINT32 len, UINT32 *res)
{
	UINT32 left, right; /* cannot remove */
	UINT32 i, filler[4], padded_length;

	MACADDR_CPY(&filler, da);
	MACADDR_CPY(((UINT8 *)filler + 6), sa);
	filler[3] = 0;

	padded_length = pad((UINT8 *)data, len);
	padded_length = padded_length >> 2;
	left = Key[0];
	right = Key[1];

	/*Loop unrolling not recommended cause the function is in itcm
	 *and bloc_function is inlined
	 */
	for (i=0; i<4; i++)
	{
		left ^= filler[i];
		block_function(&left,&right);
	}

	for (i=0; i<padded_length; i++)
	{
		left ^= data[i];
		block_function(&left,&right);
	}

	res[0] = left;
	res[1] = right;

}


inline void MichaelRx(u8 *da, u8 *sa, const UINT32 *Key,
	UINT32 *data, UINT32 len, UINT32 *res)
{
	UINT32 left, right; /* cannot remove */
	UINT32 i, filler[4], padded_length;

	MACADDR_CPY(&filler, da);
	MACADDR_CPY(((UINT8 *)filler + 6), sa);
	filler[3] = 0;

	padded_length = pad((UINT8 *)data, len);
	padded_length = padded_length >> 2;
	left = Key[0];
	right = Key[1];

	/* Loop unrolling not recommended cause the function is in itcm
	 * and bloc_function is inlined
	 */
	for (i=0; i<4; i++)
	{
		left ^= filler[i];
		block_function(&left,&right);
	}

	for (i=0; i<padded_length; i++)
	{
		left ^= data[i];
		block_function(&left,&right);
	}

	res[0] = left;
	res[1] = right;

}


inline static UINT32 pad(UINT8 *data,UINT32 length)
{
	data[length++] = 0x5a;
	data[length++] = 0x00;
	data[length++] = 0x00;
	data[length++] = 0x00;
	data[length++] = 0x00;
	/* over here len would definately be greater than 4 */
	while (length & 0x03) /* do untill the length is a multiple of 4 */
	{
		data[length++] = 0x00;
	}

	return length;
}


static void (*Phase2Fp)(UINT8 *RC4KEY, const UINT8 *TK, const UINT16 *P1K, UINT16 IV16) = Phase2;
inline static void Phase2(UINT8 *RC4KEY, const UINT8 *TK, const UINT16 *P1K, UINT16 IV16)
{
	UINT16 *PPK;

	PPK = (UINT16 *)(RC4KEY + 4);

	/* Unrolling of loop for performance */
	PPK[0] = P1K[0];
	PPK[1] = P1K[1];
	PPK[2] = P1K[2];
	PPK[3] = P1K[3];
	PPK[4] = P1K[4];

	PPK[5] = PPK[4] + IV16;           /* next,  add in IV16          */

	/* Bijective non-linear mixing of the 96 bits of PPK[0..5]         */
	PPK[0] += _S_(PPK[5] ^ TK16(0)); /* Mix key in each "round"     */
	PPK[1] += _S_(PPK[0] ^ TK16(1));
	PPK[2] += _S_(PPK[1] ^ TK16(2));
	PPK[3] += _S_(PPK[2] ^ TK16(3));
	PPK[4] += _S_(PPK[3] ^ TK16(4));
	PPK[5] += _S_(PPK[4] ^ TK16(5)); /* Total # S-box lookups == 6  */

	/* Final sweep: bijective, linear. Rotates kill LSB correlations   */
	PPK[0] += RotR1(PPK[5] ^ TK16(6));
	PPK[1] += RotR1(PPK[0] ^ TK16(7)); /* Use all of TK[] in Phase2   */
	PPK[2] += RotR1(PPK[1]);
	PPK[3] += RotR1(PPK[2]);
	PPK[4] += RotR1(PPK[3]);
	PPK[5] += RotR1(PPK[4]);

	/* At this point, for a given key TK[0..15], the 96-bit output */
	/*     value PPK[0..5] is guaranteed to be unique, as a function   */
	/*     of the 96-bit "input" value   {TA,IV32,IV16}. That is, P1K  */
	/*     is now a keyed permutation of {TA,IV32,IV16}.               */

	/* Set RC4KEY[0..3], which includes cleartext portion of RC4 key   */
	RC4KEY[0] = Hi8(IV16);              /* RC4KEY[0..2] is the WEP IV  */
	RC4KEY[1] = (Hi8(IV16) | 0x20) & 0x7F; /* Help avoid FMS weak keys  */
	RC4KEY[2] = Lo8(IV16);
	RC4KEY[3] = Lo8((PPK[5] ^ TK16(0)) >> 1);

}


inline static void GenerateEncrKey(UINT8 *RC4Key,
                     UINT8 *TemporalKey, UINT32 IV32, UINT16 IV16,
                     IEEEtypes_MacAddr_t *pAddr, UINT16 *pPhase1Key)
{

	if (IV16 <= 0x0001)
	{
		Phase1(pPhase1Key, TemporalKey, (const UINT8 *) pAddr, IV32);
	}

	(*Phase2Fp)(RC4Key, TemporalKey, pPhase1Key, IV16);
}


inline static void GenerateDecrptKey(UINT8 *trans_addr, UINT8 *frame, UINT8 *pRC4Key,
                       keyMgmtInfo_t *pkeyInfo)
{
	UINT32 RxIV32;
	UINT16 RxIV16;

	RxIV16 = (frame[0] << 8) | (frame[2]);
	RxIV32 = (UINT32)(*(frame + 4));

	if (RxIV32 != pkeyInfo->RxIV32)
	{
		Phase1(pkeyInfo->Phase1KeyRx, pkeyInfo->PairwiseTempKey1, trans_addr, RxIV32);
		pkeyInfo->RxIV32 = RxIV32;
	}

	(*Phase2Fp)(pRC4Key, pkeyInfo->PairwiseTempKey1, pkeyInfo->Phase1KeyRx, RxIV16);
}


static void mvWLAN_tkip_MIC_err_info(local_info_t *local)
{
	struct sk_buff *skb;
	struct mvwlan_ieee80211_hdr *hdr;
	u16 fc;

	skb = dev_alloc_skb(sizeof(*hdr));

	if (skb == NULL) {

		printk("mvWLAN_tkip_MIC_err_info: MIC countermeasure info failed, lack memory\n");
		return;
	}

	hdr = (struct mvwlan_ieee80211_hdr *) skb_put(skb, sizeof(*hdr));
	skb->mac.raw = skb->data;

	fc = WLAN_FC_TYPE_MGMT << 2;
	fc |= (BIT(0) | BIT(1));  /* version 3 is used for MIC error inform */
	hdr->frame_control = cpu_to_le16(fc);

	memcpy(&hdr->addr2, local->bssid, 6);
	memcpy(&hdr->addr3, local->bssid, 6);
	memcpy(&hdr->addr4, "MICERR", 6); /* extra data used for checking */

	skb->dev = local->apdev;
	skb_pull(skb, mvWLAN_80211_get_hdrlen(fc));
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = __constant_htons(ETH_P_802_2);
	memset(skb->cb, 0, sizeof(skb->cb));
	netif_rx(skb);
}



static void * mvWLAN_tkip_init(void)
{
	return NULL;
}


static void mvWLAN_tkip_deinit(void *priv)
{
	return;
}


static int mvWLAN_tkip_encrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv, int broadcast)
{
	IEEEtypes_Frame_t *tx_frame = (IEEEtypes_Frame_t *) buf;
	UINT8 genEncrKey[16];
	local_info_t *local = NULL;
	struct sta_info *sta = NULL;

	local_irq_disable();

	if (broadcast) {

		local = (local_info_t *) priv;

		if (local == NULL) {

			local_irq_enable();
			return -1;
		}

		GenerateEncrKey(genEncrKey,
			mib_MrvlRSN_GrpKey.EncryptKey,
			mib_MrvlRSN_GrpKey.g_IV32,
			mib_MrvlRSN_GrpKey.g_IV16,
			(IEEEtypes_MacAddr_t *) &local->bssid[0],
			mib_MrvlRSN_GrpKey.g_Phase1Key);

        	InsertIVintoFrmBody(tx_frame->Body,
			genEncrKey,
			mib_MrvlRSN_GrpKey.g_IV32,
			1);

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

			GenerateEncrKey(genEncrKey,
				sta->key_mgmt_info.PairwiseTempKey1,
				sta->key_mgmt_info.TxIV32,
				sta->key_mgmt_info.TxIV16,
				(IEEEtypes_MacAddr_t *) &local->bssid[0],
				sta->key_mgmt_info.Phase1KeyTx);

			InsertIVintoFrmBody(tx_frame->Body,
				genEncrKey,
				sta->key_mgmt_info.TxIV32,
				0);

			sta->key_mgmt_info.TxIV16++;
			if (sta->key_mgmt_info.TxIV16 == 0)
				sta->key_mgmt_info.TxIV32++;

		} else {

			local_irq_enable();
			return -1;
		}
	}

	flush_dcache_range((u32) obuf, (u32) (obuf + len + IV_SIZE + EIV_SIZE + ICV_SIZE));

	DoTKIPEncrypt(tx_frame->Body + IV_SIZE + EIV_SIZE, obuf,
		len, genEncrKey);

	local_irq_enable();

	return (len + IV_SIZE + EIV_SIZE + ICV_SIZE);

}


static int mvWLAN_tkip_decrypt_mpdu(u8 *buf, u8 *obuf, int len, void *priv)
{
	UINT8 RC4Key[16];
	IEEEtypes_Frame_t *rx_frame = (IEEEtypes_Frame_t *) buf;
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

	GenerateDecrptKey(sta->addr, obuf, RC4Key, &sta->key_mgmt_info);

	len -= (IV_SIZE + EIV_SIZE);

	buf = rx_frame->Body;

	flush_dcache_range((u32) obuf, (u32) (obuf + len + IV_SIZE + EIV_SIZE + ICV_SIZE));

	if (DoTKIPDecrypt(buf, obuf + IV_SIZE + EIV_SIZE,  len, RC4Key) == 0) /* ICV match failure */
	{
		local->sysConfig->Mib802dot11->RSNStats.TKIPICVErrors++; /* Commented out by Rahul */
		local_irq_enable();
		return -1;
	}

	len -= ICV_SIZE;

	local_irq_enable();

	return len;

}


static int mvWLAN_tkip_encrypt_msdu(u8 *da, u8 *sa, u8 *buf, int len, void *priv, int broadcast)
{
	UINT32 *pTxMICKey;
	UINT32 computedMIC[2];
	struct sta_info *sta = NULL;

	if (broadcast) {

		pTxMICKey = mib_MrvlRSN_GrpKey.TxMICKey;
	} else {

		sta = (struct sta_info *) priv;

		if (sta == NULL)
			return -1;

		if (sta->key_mgmt_info.RSNDataTrafficEnabled) {

			pTxMICKey = sta->key_mgmt_info.RSNPwkTxMICKey;
		} else {

			return -1;
		}
	}

	MichaelTx(da, sa, pTxMICKey, (UINT32 *) buf, len, computedMIC);

	appendMIC((buf + len), computedMIC);

	return (len + MIC_SIZE);

}


static int mvWLAN_tkip_decrypt_msdu(u8 *da, u8 *sa, u8 *buf, int len, void *priv)
{
	UINT8 rx_MIC[MIC_SIZE];
	UINT32 calc_MIC[2];
	local_info_t *local = NULL;
	struct sta_info *sta = NULL;
	UINT32 status;

	sta = (struct sta_info *) priv;

	if (sta == NULL)
		return -1;

	if (!sta->key_mgmt_info.RSNDataTrafficEnabled)
		return -1;

	local = sta->local;

	len -= MIC_SIZE;

	/* in order to gain performance, we will not check decryption is done after H/W decryption
	 * function. So we need to check here to make sure data is ready for Michael checking
	 */

	if (local->ap->num_sta > 1) {

		while (WL_REGS32(RX_WEP_CMD) & (WEP_CMD_TXWEP_BUSY | WEP_CMD_TXWEP_TRIG)) {

			int i;

			for (i = 0; i < 30; i++);
		}
	}

	memcpy(rx_MIC, buf + len, MIC_SIZE);

	MichaelRx(da, sa, sta->key_mgmt_info.RSNPwkRxMICKey, (UINT32 *) buf, len, calc_MIC);

	status = CheckMIC(calc_MIC, rx_MIC);

	if (status == 0) /* MIC match failure */
	{
		local->sysConfig->Mib802dot11->RSNStats.TKIPLocalMICFailures++;
		mvWLAN_tkip_MIC_err_info(local);

		return -1;
	}

	return len;

}


EXPORT_SYMBOL(mvWLAN_crypto_tkip_init);
