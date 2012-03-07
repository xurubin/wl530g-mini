#include <Copyright.h>

/********************************************************************************
* msApiPrototype.h
*
* DESCRIPTION:
*       API Prototypes for QuarterDeck Device
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __msApiInternal_h
#define __msApiInternal_h

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_QD
#define DBG_INFO(x) gtDbgPrint x
#else
#define DBG_INFO(x)
#endif /* DEBUG_QD */


#define GT_LPORT_2_PORT(lport)      (GT_U8)(lport & 0xff)
#define GT_PORT_2_LPORT(port)       (GT_32)(port & 0xff)

/*
 *  definition for MEMBER_TAG
 */
#define MEMBER_EGRESS_UNMODIFIED	0
#define NOT_A_MEMBER				1
#define MEMBER_EGRESS_UNTAGGED		2
#define MEMBER_EGRESS_TAGGED		3

/*
 *	Type definition for MIB counter operation
 */
typedef enum 
{
	STATS_FLUSH_ALL,		/* Flush all counters for all ports */
	STATS_FLUSH_PORT,		/* Flush all counters for a port */
	STATS_READ_COUNTER,	/* Read a specific counter from a port */
	STATS_READ_ALL		/* Read all counters from a port */

} GT_STATS_OPERATION;


/*
 * typedef: enum GT_EVENT_TYPE
 *
 * Description: Enumeration of the available hardware driven events.
 *
 * Enumerations:
 *   GT_STATS_DONE - Statistics Operation Done interrrupt Enable 
 *   GT_VTU_PORB - VLAN Problem/Violation Interrupt Enable
 *   GT_VTU_DONE - VALN Table Operation Done Interrupt Enable
 *   GT_ATU_FULL - ATU full interrupt enable.
 *   GT_ATU_DONE - ATU Done interrupt enable.
 *   GT_PHY_INT  - PHY interrupt enable.
 *   GT_EE_INT   - EEPROM Done interrupt enable.
 */
#define GT_STATS_DONE           0x40
#define GT_VTU_PROB             0x20
#define GT_VTU_DONE             0x10
#define GT_ATU_FULL 		0x8
#define GT_ATU_DONE		0x4
#define GT_PHY_INTERRUPT	0x2
#define GT_EE_INTERRUPT		0x1

#define GT_INT_MASK		(GT_STATS_DONE | GT_VTU_PROB | GT_VTU_DONE | GT_ATU_FULL | GT_ATU_DONE | GT_PHY_INTERRUPT | GT_EE_INTERRUPT)

/* The following macro converts a binary    */
/* value (of 1 bit) to a boolean one.       */
/* 0 --> GT_FALSE                           */
/* 1 --> GT_TRUE                            */
#define BIT_2_BOOL(binVal,boolVal)                                  \
            (boolVal) = (((binVal) == 0) ? GT_FALSE : GT_TRUE)

/* The following macro converts a boolean   */
/* value to a binary one (of 1 bit).        */
/* GT_FALSE --> 0                           */
/* GT_TRUE --> 1                            */
#define BOOL_2_BIT(boolVal,binVal)                                  \
            (binVal) = (((boolVal) == GT_TRUE) ? 1 : 0)




/* device name - devName */
#define DEV_88E6051                      0x0001    /* quarterdeck 6051      */
#define DEV_88E6052                      0x0002    /* quarterdeck 6052      */
#define DEV_88E6021                      0x0004    /* fullsail              */
#define DEV_88E6060                      0x0008    /* Gondola               */
#define DEV_88E6061                      0x0010    /* clippership 6061      */
#define DEV_88E6062                      0x0020    /* clippership 6062      */
#define DEV_88E6063                      0x0040    /* clippership 6063      */
#define DEV_FF_XP                        0x0080    /* FireFox-XG            */
#define DEV_FF_EG                        0x0100    /* FireFox-EG            */
#define DEV_FF_HG                        0x0200    /* FireFox-HG            */
#define DEV_FH_VPN_L                     0x0400    /* FireHawk-VPN-L        */
#define DEV_FH_VPN                       0x0800    /* FireHawk-VPN          */

#define DEV_BROADCAST_INVALID	( DEV_88E6051 | DEV_88E6052 )

#define DEV_ATU_256_2048 		( DEV_88E6021 | DEV_88E6060 )
#define DEV_ATU_562_2048 		\
						( DEV_88E6052 | DEV_88E6062 | DEV_88E6063 | 	\
						  DEV_FF_HG | DEV_FH_VPN)

#define DEV_DBNUM_FULL 	\
						( DEV_88E6021 | DEV_88E6060 | DEV_88E6061 | 	\
						  DEV_88E6062 | DEV_88E6063 | 					\
						  DEV_FH_VPN_L | DEV_FH_VPN )

#define DEV_STATIC_ADDR	\
						( DEV_88E6021 | DEV_FF_XP | DEV_FF_EG | DEV_FF_HG |	\
						  DEV_88E6052 | DEV_88E6061 | DEV_88E6062 |			\
						  DEV_88E6063 | DEV_FH_VPN_L | DEV_FH_VPN )

#define DEV_TRAILER		\
						( DEV_88E6021 | DEV_FF_XP | DEV_FF_HG |		\
						  DEV_88E6052 | DEV_88E6061 | 				\
						  DEV_88E6062 | DEV_88E6063 | DEV_FH_VPN )
#define DEV_TRAILER_P5		( DEV_FF_EG | DEV_FH_VPN_L )
#define DEV_TRAILER_P4P5	( DEV_88E6060 )

#define DEV_HEADER		\
						( DEV_FF_XP | DEV_FF_HG | DEV_88E6061 |		\
						  DEV_88E6062 | DEV_88E6063 | DEV_FH_VPN )
#define DEV_HEADER_P5		( DEV_FF_EG | DEV_FH_VPN_L )
#define DEV_HEADER_P4P5		( DEV_88E6060 )

#define DEV_QoS			\
						( DEV_88E6021 | DEV_FF_XP | DEV_FF_HG |		\
						  DEV_88E6051 | DEV_88E6052 | DEV_88E6061 |	\
						  DEV_88E6062 | DEV_88E6063 | DEV_FH_VPN )

#define DEV_TAGGING			DEV_QoS
#define DEV_DOUBLE_TAGGING	(DEV_QoS & (~DEV_88E6051))

#define DEV_802_1Q		( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN )
#define DEV_802_1S		( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN )

#define DEV_RMON		( DEV_88E6021 | DEV_88E6063 | DEV_FH_VPN )

#define DEV_IGMP_SNOOPING	\
						( DEV_88E6021 | DEV_88E6061 | DEV_88E6062 |			\
						  DEV_88E6063 | DEV_FH_VPN_L | DEV_FH_VPN )

#define DEV_PORT_MONITORING	\
						( DEV_88E6060 | DEV_88E6061 | DEV_88E6062 |			\
						  DEV_88E6063 | DEV_FH_VPN_L | DEV_FH_VPN )

#define DEV_MC_RATE_PERCENT	\
						( DEV_88E6021 | DEV_88E6051 | DEV_88E6052 )

#define DEV_MC_RATE_KBPS	\
						( DEV_FF_XP | DEV_FF_HG | DEV_88E6061 | 			\
						  DEV_88E6062 | DEV_88E6063 | DEV_FH_VPN )

#define DEV_INGRESS_RATE_KBPS	\
						( DEV_FF_XP | DEV_FF_HG | DEV_88E6061 | 			\
						  DEV_88E6062 | DEV_88E6063 | DEV_FH_VPN )

#define DEV_EGRESS_RATE_KBPS	\
						( DEV_FF_XP | DEV_FF_HG | DEV_88E6061 | 			\
						  DEV_88E6062 | DEV_88E6063 | DEV_FH_VPN )

#define DEV_MII_DUPLEX_CONFIG	\
						( DEV_88E6021 | DEV_88E6061 | DEV_88E6062 |			\
						  DEV_88E6063 | DEV_FH_VPN_L | DEV_FH_VPN )

#define DEV_QD_PLUS 	\
					( DEV_88E6021 | DEV_FF_XP | DEV_FF_EG | DEV_FF_HG |	\
					  DEV_88E6060 | DEV_88E6061 | 						\
					  DEV_88E6062 | DEV_88E6063 | DEV_FH_VPN_L | DEV_FH_VPN )

#define IS_IN_DEV_GROUP(dev,_group) (dev->devName & (_group))

/* need to check port number(_hwPort) later */
#define IS_VALID_API_CALL(dev,_hwPort, _devName)	    	\
	(( (!(dev->devName & (_devName))) 		\
	 ) ? GT_NOT_SUPPORTED : GT_OK)

#define IS_CONFIGURABLE_PHY(dev,_hwPort)	(dev->maxPhyNum > (_hwPort))

/*******************************************************************************
* gvtuGetViolation
*
* DESCRIPTION:
*       Get VTU Violation data
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetViolation
(
    IN GT_QD_DEV*       dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
);

/*******************************************************************************
* gsysSetRetransmitMode
*
* DESCRIPTION:
*       This routine set the Retransmit Mode.
*
* INPUTS:
*       en - GT_TRUE Retransimt Mode is enabled, GT_FALSE otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRetransmitMode
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL en
);

/*******************************************************************************
* gsysGetRetransmitMode
*
* DESCRIPTION:
*       This routine get the Retransmit Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Retransmit Mode is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRetransmitMode
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *en
);

/*******************************************************************************
* gsysSetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetLimitBackoff
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL en
);

/*******************************************************************************
* gsysGetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetLimitBackoff
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *en
);

/*******************************************************************************
* gsysSetRsvRegPri
*
* DESCRIPTION:
*       This routine set the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvReqPri
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL en
);

/*******************************************************************************
* gsysGetRsvReqPri
*
* DESCRIPTION:
*       This routine get the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvReqPri
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *en
);

/*******************************************************************************
* gsysGetPtrCollision
*
* DESCRIPTION:
*       This routine get the QC Pointer Collision.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 		This feature is for both clippership and fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPtrCollision
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *mode
);

/*******************************************************************************
* gsysGetDpvCorrupt
*
* DESCRIPTION:
*       This routine get the DpvCorrupt bit. This bit is set to a one when the 
*       QC detects a destination vector error
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: destination vector corrupt, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 	This feature is on clippership, but not on fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDpvCorrupt
(
    IN GT_BOOL *mode
);

/*******************************************************************************
* gsysGetMissingPointers
*
* DESCRIPTION:
*       This routine get the Missing Pointer bit. This bit is set to a one when  
*       the Register File detects less than 64 pointers in the Link List. 
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: Missing Pointers error, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
* 	This feature is on clippership, but not on fullsail
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetMissingPointers
(
    IN GT_QD_DEV*       dev,
    IN GT_BOOL *mode
);

/*******************************************************************************
* gtDbgPrint
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*       None
*
*******************************************************************************/
void gtDbgPrint(char* format, ...);

/*******************************************************************************
* gtMemSet
*
* DESCRIPTION:
*       Set a block of memory
*
* INPUTS:
*       start  - start address of memory block for setting
*       simbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * gtMemSet
(
    IN void * start,
    IN int    symbol,
    IN GT_U32 size
);

/*******************************************************************************
* gtMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
void * gtMemCpy
(
    IN void *       destination,
    IN const void * source,
    IN GT_U32       size
);


/*******************************************************************************
* gtMemCmp
*
* DESCRIPTION:
*       Compares given memories.
*
* INPUTS:
*       src1 - source 1
*       src2 - source 2
*       size - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0, if equal.
*		negative number, if src1 < src2.
*		positive number, if src1 > src2.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int gtMemCmp
(
    IN char src1[],
    IN char src2[],
    IN GT_U32 size
);

/*******************************************************************************
* gtStrlen
*
* DESCRIPTION:
*       Determine the length of a string
* INPUTS:
*       source  - string
*
* OUTPUTS:
*       None
*
* RETURNS:
*       size    - number of characters in string, not including EOS.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_U32 gtStrlen
(
    IN const void * source
);



#ifdef __cplusplus
}
#endif

#endif /* __msApiInternal_h */
