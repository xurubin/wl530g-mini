/*******************************************************************************
*              Copyright 2002, GALILEO TECHNOLOGY, LTD.
*
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL. NO RIGHTS ARE GRANTED
* HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT OF MARVELL OR ANY THIRD
* PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE DISCRETION TO REQUEST THAT THIS
* CODE BE IMMEDIATELY RETURNED TO MARVELL. THIS CODE IS PROVIDED "AS IS".
* MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS
* ACCURACY, COMPLETENESS OR PERFORMANCE. MARVELL COMPRISES MARVELL TECHNOLOGY
* GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, MARVELL INTERNATIONAL LTD. (MIL),
* MARVELL TECHNOLOGY, INC. (MTI), MARVELL SEMICONDUCTOR, INC. (MSI), MARVELL
* ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K. (MJKK), GALILEO TECHNOLOGY LTD. (GTL)
* AND GALILEO TECHNOLOGY, INC. (GTI).
********************************************************************************
* gtVct.h
*
* DESCRIPTION:
*       API definitions for Marvell Virtual Cable Tester functionality.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/

#ifndef __gtVcth
#define __gtVcth

#include "msApi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MARVELL_OUI_MSb		0x0141
#define MARVELL_OUI_LSb		0x0C00
#define OUI_LSb_MASK		0xFC00
#define PHY_MODEL_MASK		0x03F0
#define PHY_REV_MASK		0x000F

#define DEV_E3082		0x8 << 4
#define DEV_E104X		0x2 << 4
#define DEV_E1111		0xC << 4
#define DEV_E114X		0xD << 4
#define DEV_E1180		0xE << 4

/* definition for formula to calculate actual distance */
#ifdef USING_FP
#define FORMULA_PHY100M(_data)	((_data)*0.7897 - 16.8799)
#define FORMULA_PHY1000M(_data)	((_data)*0.8018 - 28.751)
#else
#define FORMULA_PHY100M(_data)	(((_data)*7897 - 168799)/10000)
#define FORMULA_PHY1000M(_data)	(((_data)*8018 - 287510)/10000)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gtVcth */
