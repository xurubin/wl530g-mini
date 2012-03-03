/*
 * Host AP crypt: host-based WEP encryption implementation for Host AP driver
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


struct mvwlan_wep_data {
	u32 iv;
	u8 keys[MVWLAN_WEP_KEYS][16];
	u8 key_lens[MVWLAN_WEP_KEYS];
	int tx_key;
};

#ifdef SELF_CTS
static const __u32 crc32_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};
#endif /* SELF_CTS */

#ifdef SELF_CTS
#define S_ACK_SWAP(a,b) do { unsigned char  t = S_ACK[a]; S_ACK[a] = S_ACK[b]; S_ACK[b] = t; } while(0)
unsigned char S_ACK[256]__attribute__ ((section (".itcmbuf"))) = {0};
void crc32forCts(unsigned char *buf, int len)
{

    unsigned long i, j, k, crc;
    unsigned char  *pos;
    unsigned char tmp[10] = {0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* Prepend 24-bit IV to RC4 key and TX frame */
    for (i = 0; i< 10; i++)
    {
        tmp[i] = buf[i];
    }
    pos = tmp;

    /* Compute CRC32 over unencrypted data and apply RC4 to data */
    crc = ~0;
    i = j = 0;
    for (k = 0; k < len; k++)
    {
        crc = crc32_table[(crc ^ *pos) & 0xff] ^ (crc >> 8);
        i = (i + 1) & 0xff;
        j = (j + S_ACK[i]) & 0xff;
        S_ACK_SWAP(i, j);
        *pos++ ^= S_ACK[(S_ACK[i] + S_ACK[j]) & 0xff];
    }
    crc = ~crc;

    /* Append little-endian CRC32 and encrypt it to produce ICV */
    buf[10] = crc;
    buf[11] = crc >> 8;
    buf[12] = crc >> 16;
    buf[13] = crc >> 24;

}
#endif


static void * mvWLAN_wep_init(void)
{
	struct mvwlan_wep_data *priv;

#ifndef NEW_MODULE_CODE
	MOD_INC_USE_COUNT;
#endif

	priv = (struct mvwlan_wep_data *) kmalloc(sizeof(*priv), GFP_ATOMIC);
	if (priv == NULL) {
#ifndef NEW_MODULE_CODE
		MOD_DEC_USE_COUNT;
#endif
		return NULL;
	}
	memset(priv, 0, sizeof(*priv));

	/* start WEP IV from a random value */
	get_random_bytes(&priv->iv, 4);

	return priv;
}


static void mvWLAN_wep_deinit(void *priv)
{
	kfree(priv);
#ifndef NEW_MODULE_CODE
	MOD_DEC_USE_COUNT;
#endif
}


/* Perform WEP encryption on given buffer. Buffer needs to has 4 bytes of
 * extra space (IV) in the beginning, then len bytes of data, and finally
 * 4 bytes of extra space (ICV). Both IV and ICV will be transmitted, so the
 * payload length increases with 8 bytes.
 *
 * WEP frame payload: IV + TX key idx, RC4(data), ICV = RC4(CRC32(data))
 */
static int mvWLAN_wep_encrypt(u8 *buf, u8 *obuf, int len, void *priv, int broadcast)
{
	IEEEtypes_Frame_t *tx_frame = (IEEEtypes_Frame_t *) buf;
	struct mvwlan_wep_data *wep = priv;
	u8 *wep_key;
	u32 key, klen;
	volatile u16 i;
	volatile u32 loop_cnt = 0;

	local_irq_disable();

	while (WL_REGS32(RX_WEP_CMD) &
	       (WEP_CMD_TXWEP_BUSY | WEP_CMD_TXWEP_TRIG)) {
		for (i = 0; i < 10; i++);
		if (loop_cnt++ > 10000) {
			local_irq_enable();
			return 0;
		}
	}

	wep->iv++;
	klen = 3 + wep->key_lens[wep->tx_key];

	/* Fluhrer, Mantin, and Shamir have reported weaknesses in the key
	 * scheduling algorithm of RC4. At least IVs (KeyByte + 3, 0xff, N)
	 * can be used to speedup attacks, so avoid using them. */
	if ((wep->iv & 0xff00) == 0xff00) {
		u8 B = (wep->iv >> 16) & 0xff;
		if (B >= 3 && B < klen)
			wep->iv += 0x0100;
	}

	/* Set IV */
	tx_frame->Body[3] = wep->tx_key << 6;
	tx_frame->Body[2] = (wep->iv >> 16) & 0xff;
	tx_frame->Body[1] = (wep->iv >> 8) & 0xff;
	tx_frame->Body[0] = wep->iv & 0xff;

	wep_key = wep->keys[wep->tx_key];

	/* Set up the keys */
	key = (u32)(wep_key[0]);
	key <<= 8;
	key |= (u32)(tx_frame->Body[2]);
	key <<= 8;
	key |= (u32)(tx_frame->Body[1]);
	key <<= 8;
	key |= (u32)(tx_frame->Body[0]);
	WL_WRITE_WORD(RX_SW_ENC_KEY0, key);

	flush_dcache_range((u32) obuf, (u32) (obuf + len));

	/* Program read/write addresses and length */
	WL_WRITE_WORD(RX_SW_ENC_RDADDR, (u32) obuf);
	WL_WRITE_WORD(RX_SW_ENC_WRADDR, ((u32) tx_frame->Body) + 4);
	WL_WRITE_WORD(RX_SW_ENC_LEN, len);

	/* Start WEP */
	if (wep->key_lens[wep->tx_key] == 13) {
		/* 128 bit */
		WL_WRITE_WORD(RX_WEP_CMD, WEP_CMD_WEP_START | WEP_CMD_WEP128);
	} else {
		/* 40bit */
		WL_WRITE_WORD(RX_WEP_CMD, WEP_CMD_WEP_START);
        }

	local_irq_enable();

	return (len + 8);
}


static int mvWLAN_wep_decrypt(u8 *buf, u8 *obuf, int len, void *priv)
{
	/* H/W does this for us */

	return len;
}


static int mvWLAN_wep_set_key(int idx, void *key, int len, void *priv)
{
	struct mvwlan_wep_data *wep = priv;

	if (idx < 0 || idx >= MVWLAN_WEP_KEYS || len < 0 ||
	    len > MVWLAN_WEP_KEY_LEN)
		return -1;

	memcpy(wep->keys[idx], key, len);
	wep->key_lens[idx] = len;

	if (len == 5)
		msi_wl_SetRxWEPKey(idx, WEP_40_BIT, key);
	else if (len == 13)
		msi_wl_SetRxWEPKey(idx, WEP_104_BIT, key);

	return 0;
}


static int mvWLAN_wep_get_key(int idx, void *key, int len, void *priv)
{
	struct mvwlan_wep_data *wep = priv;

	if (idx < 0 || idx >= MVWLAN_WEP_KEYS || len < wep->key_lens[idx])
		return -1;

	memcpy(key, wep->keys[idx], wep->key_lens[idx]);

	return wep->key_lens[idx];
}


static int mvWLAN_wep_set_key_idx(int idx, void *priv)
{
	struct mvwlan_wep_data *wep = priv;

	if (idx < 0 || idx >= MVWLAN_WEP_KEYS || wep->key_lens[idx] == 0)
		return -1;

	wep->tx_key = idx;

	if (wep->key_lens[idx] == 5)
		msi_wl_SetTxWEPKey(WEP_40_BIT, wep->keys[idx]);
	else if (wep->key_lens[idx] == 13)
		msi_wl_SetTxWEPKey(WEP_104_BIT, wep->keys[idx]);

	return 0;
}


static int mvWLAN_wep_get_key_idx(void *priv)
{
	struct mvwlan_wep_data *wep = priv;
	return wep->tx_key;
}


static struct mvwlan_crypto_ops mvwlan_crypt_wep = {
	.name			= "WEP",
	.init			= mvWLAN_wep_init,
	.deinit			= mvWLAN_wep_deinit,
	.encrypt_mpdu		= mvWLAN_wep_encrypt,
	.decrypt_mpdu		= mvWLAN_wep_decrypt,
	.encrypt_msdu		= NULL,
	.decrypt_msdu		= NULL,
	.set_key		= mvWLAN_wep_set_key,
	.get_key		= mvWLAN_wep_get_key,
	.set_key_idx		= mvWLAN_wep_set_key_idx,
	.get_key_idx		= mvWLAN_wep_get_key_idx,
	.extra_prefix_len	= 4 /* IV */,
	.extra_postfix_len	= 4 /* ICV */
};


int mvWLAN_crypto_wep_init(void)
{
	if (mvWLAN_register_crypto_ops(&mvwlan_crypt_wep) < 0)
		return -1;

	return 0;
}


EXPORT_SYMBOL(mvWLAN_crypto_wep_init);
