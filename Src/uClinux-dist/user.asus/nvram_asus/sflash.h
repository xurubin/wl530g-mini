/*
 * Broadcom SiliconBackplane chipcommon serial flash interface
 *
 * Copyright 2002, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id: sflash.h,v 13.3 2003/03/31 23:34:15 mhuang Exp $
 */

#ifndef _sflash_h_
#define _sflash_h_

#include <typedefs.h>
#include <sbchipc.h>

/* GPIO based bank selection (1 GPIO bit) */
#define SFLASH_MAX_BANKS	2
#define SFLASH_GPIO_SHIFT	2
#define SFLASH_GPIO_MASK	((SFLASH_MAX_BANKS - 1) << SFLASH_GPIO_SHIFT)

struct sflash_bank {
	uint offset;					/* Byte offset */
	uint erasesize;					/* Block size */
	uint numblocks;					/* Number of blocks */
	uint size;					/* Total bank size in bytes */
};

struct sflash {
	struct sflash_bank banks[SFLASH_MAX_BANKS];	/* GPIO selectable banks */
	uint32 type;					/* Type */
	uint size;					/* Total array size in bytes */
};

/* Utility functions */
extern int sflash_poll(chipcregs_t *cc, uint offset);
extern int sflash_read(chipcregs_t *cc, uint offset, uint len, uchar *buf);
extern int sflash_write(chipcregs_t *cc, uint offset, uint len, const uchar *buf);
extern int sflash_erase(chipcregs_t *cc, uint offset);
extern struct sflash * sflash_init(chipcregs_t *cc);

#endif /* _sflash_h_ */
