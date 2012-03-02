/*
 * Copyright 1999 Epigram, Inc.
 *
 * $Id: epivers.h.in,v 13.19 2001/04/09 18:37:14 henryp Exp $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#ifdef	linux
#include <linux/config.h>
#endif

/* Vendor Name, ASCII, 32 chars max */
#ifdef COMPANYNAME
#define	HPNA_VENDOR 		COMPANYNAME
#else
#define	HPNA_VENDOR 		"Broadcom Corporation"
#endif

/* Driver Date, ASCII, 32 chars max */
#define HPNA_DRV_BUILD_DATE	__DATE__

/* Hardware Manufacture Date, ASCII, 32 chars max */
#define HPNA_HW_MFG_DATE	"Not Specified"

/* See documentation for Device Type values, 32 values max */
#ifndef	HPNA_DEV_TYPE

#if	defined(CONFIG_BRCM_VJ)
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_DISPLAY }

#elif	defined(CONFIG_BCRM_93725)
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_CM_BRIDGE, CDCF_V0_DEVICE_DISPLAY }

#else
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_PCINIC }

#endif

#endif	/* !HPNA_DEV_TYPE */


#define	EPI_MAJOR_VERSION	3

#define	EPI_MINOR_VERSION	31

#define	EPI_RC_NUMBER		12

#define	EPI_INCREMENTAL_NUMBER	0

#define	EPI_BUILD_NUMBER	0

#define	EPI_VERSION		3,31,12,0

/* Driver Version String, ASCII, 32 chars max */
#define	EPI_VERSION_STR		"3.31.12.0"

#endif /* _epivers_h_ */
