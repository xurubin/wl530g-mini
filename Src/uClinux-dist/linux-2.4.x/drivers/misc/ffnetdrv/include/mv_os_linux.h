/* 
 * Copyright 2002, Marvell International Ltd.
 * 
 * THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 * NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 * OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 * DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 * THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED
 * OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
 */

/*
 * FILENAME:    $Workfile: mv_os_linux.h $
 * REVISION:    $Revision: 3 $
 * LAST UPDATE: $Modtime: 3/02/03 10:20p $
 *
 * DESCRIPTION:
 *     This file is the OS Layer header, for uClinux.
 */

#ifndef _MV_OS_LINUX_H_
#define _MV_OS_LINUX_H_

/*
 * LINUX includes
 */
#include <linux/kernel.h>

#include <linux/config.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/ip.h>
#include <linux/mii.h>

#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/pgtable.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>
#include <asm/arch/hardware.h>

#include "msApi.h"
#include "mv_types.h"
#include "mv_common.h"
#include "mv_platform.h"

#define INLINE	inline

#ifdef MV_DEBUG
#define ASSERT(assert) { do { if(!(assert)) { BUG(); } }while(0); }
#else
#define ASSERT(assert) 
#endif 

#define IN
#define OUT

#define INLINE inline

#define gtOsPrintf				printk

/*
 *  Physical memory access
 */
static INLINE GT_U32 gtOsIoMemReadWord(GT_U32 hpaddr)	
{
	return *(volatile GT_U32*)(hpaddr|GT_OS_IOMEM_PREFIX);
}

static INLINE void gtOsIoMemWriteWord(GT_U32 hpaddr, GT_U32 data) 
{
    *(volatile GT_U32*)(hpaddr|GT_OS_IOMEM_PREFIX) = data;
}

static INLINE GT_U16 gtOsIoMemReadShort(GT_U32 hpaddr)	
{
	return *(volatile GT_U16*)(hpaddr|GT_OS_IOMEM_PREFIX);
}

static INLINE void gtOsIoMemWriteShort(GT_U32 hpaddr, GT_U16 data) 
{
    *(volatile GT_U16*)(hpaddr|GT_OS_IOMEM_PREFIX) = data;
}

static INLINE GT_U8 gtOsIoMemReadByte(GT_U32 hpaddr)	
{
	return *(volatile GT_U8*)(hpaddr|GT_OS_IOMEM_PREFIX);
}

static INLINE void gtOsIoMemWriteByte(GT_U32 hpaddr, GT_U8 data) 
{
    *(volatile GT_U8*)(hpaddr|GT_OS_IOMEM_PREFIX) = data;
}

#endif /* _MV_OS_LINUX_H_ */
