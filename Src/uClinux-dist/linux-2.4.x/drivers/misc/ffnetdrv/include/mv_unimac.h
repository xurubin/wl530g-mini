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
 * FILENAME:    $Workfile: mv_unimac.h $
 * REVISION:    $Revision: 2 $
 * LAST UPDATE: $Modtime: 11/12/02 16:19 $
 *
 * DESCRIPTION:
 *     This file is the OS Layer header, provided for OS independence.
 */

#ifndef MV_UNIMAC_H
#define MV_UNIMAC_H


#include "mv_platform.h"
#include "mv_qd.h"

#define MV_UNM_VID_DISABLED		0
#define MV_UNM_VID_ISOLATED		(GT_NUM_OF_SWITCH_PORTS+1)

#define	MV_UNM_MAX_VID			(MV_UNM_VID_ISOLATED+1)

typedef int MV_UNM_VID;

typedef struct _MV_UNM_CONFIG 
{
    MV_UNM_VID	vidOfPort[GT_NUM_OF_SWITCH_PORTS];     
} MV_UNM_CONFIG;


/*******************************************************************************
*Description:
*	User application function that supplies configuration to UniMAC Manager
*  The function receives a optional pointer to configuration array 
*	Parameters:
*		unmConfig: pointer to VLAN mapping configuration
*					if this parameter is NULL the default parameters
* Return value:
*		GT_OK	on success
*		GT_FAIL otherwise
********************************************************************************/

extern GT_STATUS getNetConfig(OUT MV_UNM_CONFIG* unmConfig);


/*******************************************************************************
*Description:
*	UniMAC manager intialization function
*  The function receives a optional pointer to configuration array and also 
*	pointers to save and restore configurqation functions
*	Parameters:
*		None
* Return value:
*		GT_SUCCESS		on success
*		GT_BAD_PARAM	if if getNetConfig() returns invalid configuration
*		GT_FAIL			otherwise
********************************************************************************/
GT_STATUS  mvUnmInitialize(void);

/*******************************************************************************
*Description:
*	Retrieve current network configuration
*  The function receives a pointer to return configuration 
*	Parameters:
*		config: pointer to VLAN mapping configuration
* Return value:
*		GT_OK	on success
*		GT_FAIL on error
********************************************************************************/
GT_STATUS	mvUnmGetNetConfig(OUT MV_UNM_CONFIG *config);


/*******************************************************************************
*Description:
*	Move port from isolated VLAN to specific VLAN
*	Parameters:
*		portId:  port number 
*		vlanId:	 VLAN id
* Return value:
* GT_OK			on success
* GT_BAD_PARAM	if a port number is invalid or the port 
*						already belongs to some VLAN
* GT_FAIL		on other failures
*
********************************************************************************/
 
GT_STATUS	mvUnmPortMoveTo(IN int qdPort, IN const MV_UNM_VID vlanId);

/* Create port based VLAN */
GT_STATUS mvUnmCreateVlan(int vid, GT_U32 vlanPortMask);

/* Return port mask belong this vid */
GT_U32		mvUnmGetPortMaskOfVid(MV_UNM_VID vid);
                                   

/* Return vid, that port belong to */
MV_UNM_VID	mvUnmGetVidOfPort(int port);

/* Return total number of vids, without ISOLATED and DISABLED  */
int			mvUnmGetNumOfVlans(void);



#endif /* MV_UNIMAC_H */
