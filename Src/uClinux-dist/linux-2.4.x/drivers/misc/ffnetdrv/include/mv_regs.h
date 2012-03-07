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
 * FILENAME:    $Workfile: mv_regs.h $
 * REVISION:    $Revision: 4 $
 * LAST UPDATE: $Modtime: 16/12/02 16:41 $
 *
 * DESCRIPTION:
 *     This file contains general information about the GT registers.
 */


#ifndef MV_REGS_H
#define MV_REGS_H

#ifndef MV88E6218
#define MV88E6218
#endif

/* This header file contains readable definitions for the GT-96132 System 
*  Controller internal register addresses:
*   CPU Configuration 
*   CPU Address Decode 
*   CPU Errors Report 
*   Sync Barrier 
*   SDRAM_and Device Address Decode 
*   SDRAM Configuration 
*   SDRAM Parameters 
*   ECC 
*   Device Parameters
*   DMA Record 
*   DMA Arbiter 
*   Timer/Counter 
*   PCI_Internal 
*   PCI Configuration 
*   PCI Configuration, Function 1 
*   Interrupts 
*   I2O Support Registers 
*   Communication Unit registers
*/

/****************************************/
/* General           					*/
/****************************************/

#define FAST_ETH_MAC_OFFSET  		0x8000


/****************************************/
/* CPU Configuration 					*/
/****************************************/


/* Ethernet Ports */
#define GT_REG_ETHER_SMI_REG        (FAST_ETH_MAC_OFFSET+0x10) 


#endif /* MV_REGS_H */
