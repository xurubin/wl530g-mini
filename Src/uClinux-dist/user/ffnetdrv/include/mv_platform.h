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
 * FILENAME:    $Workfile: mv_platform.h $
 * REVISION:    $Revision: 2 $
 * LAST UPDATE: $Modtime: 12/30/02 9:16a $
 *
 * DESCRIPTION:
 *     This file defines the support required for CPU and BSP independence.
 */

#ifndef _MV_PLATFORM_H_
#define _MV_PLATFORM_H_

#include "mv_types.h"
#include "mv_common.h"
#include "mv_regs.h"

/*-------------------------------------------------*/
/* Trailers and Headers settings                   */
/*-------------------------------------------------*/
#undef TRAILERS
#define HEADERS

#ifdef TRAILERS
#define QD_TRAILER_MODE
#endif 

#ifdef HEADERS
#define QD_HEADER_MODE
#endif


/*-------------------------------------------------*/
/* Types for IOCTL implementation                  */
/*-------------------------------------------------*/

#define  STATUS_SUCCESS (0)
#define  STATUS_UNSUCCESSFUL (-1)

#ifndef STANDALONE
typedef enum {
  false =0,
  true
} bool;
#endif

typedef void VOID;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef unsigned int DWORD;
typedef char WCHAR;
#define MAX_VLAN_NAME                   (20)



/*
 * The CPU port number within the QD switch
 */
#define GT_CPU_SWITCH_PORT	(5)


#define PLATFORM_GTREG_BASE 0x80000000    
#define GT_OS_IOMEM_PREFIX  0x00000000


/*  Access Device BUS for internal usage only */

#define PLATFORM_DEV_READ_CHAR(offset) *(volatile GT_U8*)((offset)|PLATFORM_DEV_BASE|GT_OS_IOMEM_PREFIX);

#define PLATFORM_DEV_WRITE_CHAR(offset, data) *(volatile GT_U8*)((offset)|PLATFORM_DEV_BASE|GT_OS_IOMEM_PREFIX) = (data);


/* Access to FF registers (Read/Write) */
static inline void gtOsGtRegWrite(GT_U32 gtreg, GT_U32 data)
{
    ((volatile GT_U32)*((volatile GT_U32*)(PLATFORM_GTREG_BASE|GT_OS_IOMEM_PREFIX|gtreg))) = data;
}

static inline GT_U32 gtOsGtRegRead(GT_U32 gtreg)
{
	return ( (volatile GT_U32)*((volatile GT_U32*)(PLATFORM_GTREG_BASE|GT_OS_IOMEM_PREFIX|gtreg)) );
}

#endif /* _MV_PLATFORM_H_ */
