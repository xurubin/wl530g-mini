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
 * FILENAME:    $Workfile: mv_qd.h $
 * REVISION:    $Revision: 4 $
 * LAST UPDATE: $Modtime: 2/03/03 5:53p $
 *
 * DESCRIPTION:
 *     This file is the QD driver header.
 */

#ifndef __mv_qd_h
#define __mv_qd_h


#include "mv_types.h"

/* Define the different device type that may exist in system        */
typedef enum
{
    GT_88E6021  = 0x021,
    GT_88E6051  = 0x051,
    GT_88E6052  = 0x052,
    GT_88E6060  = 0x060,
    GT_88E6061	= 0x151,
    GT_88E6062	= 0x152,
    GT_88E6063	= 0x153,
    GT_FH_VPN_L	= 0xF51,
    GT_FH_VPN	= 0xF53,
    GT_FF_EG	= 0xF91,
    GT_FF_XP	= 0xF92,
    GT_FF_HG	= 0xF93
}GT_DEVICE;

/*
 *   Logical Port value based on a Port
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |0|  reserved                                   |    port       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 *   The following macros should be used to extract specific info
 *   from a Logical Port index
 */
typedef GT_U32 GT_LPORT;

/* Maximal number of ports a switch may have.   */
#define GT_NUM_OF_SWITCH_PORTS    7

#define GT_QD_VERSION_MAX_LEN 30

typedef struct _GT_QD_DEV	GT_QD_DEV;
/*
 * Typedef: struct GT_VERSION
 *
 * Description: This struct holds the package version.
 *
 * Fields:
 *      version - string array holding the version.
 *
 */
typedef struct
{
    GT_U8   version[GT_QD_VERSION_MAX_LEN];
}GT_VERSION;

/*
 * typedef: enum GT_FLUSH_CMD
 *
 * Description: Enumeration of the address translation unit flush operation.
 *
 * Enumerations:
 *   GT_FLUSH_ALL       - flush all entries.
 *   GT_FLUSH_ALL_UNBLK - flush all unblocked.
 */
typedef enum
{
    GT_FLUSH_ALL       = 1,
    GT_FLUSH_ALL_UNBLK
}GT_FLUSH_CMD;
    

/********************** ATU Table Definitions ********************/

/*
 * Typedef:
 *
 * Description: Defines the different sizes of the Mac address table.
 *
 * Fields:
 *      ATU_SIZE_256    -   256 entries Mac address table.
 *      ATU_SIZE_512    -   512 entries Mac address table.
 *      ATU_SIZE_1024   -   1024 entries Mac address table.
 *      ATU_SIZE_2048   -   2048 entries Mac address table.
 *      ATU_SIZE_4096   -   4096 entries Mac address table.
 *
 */
typedef enum
{
    ATU_SIZE_256,
    ATU_SIZE_512,
    ATU_SIZE_1024,
    ATU_SIZE_2048,
    ATU_SIZE_4096
}ATU_SIZE;

/*
 * Typedef: enum GT_ATU_OPERARION
 *
 * Description: Defines the different ATU and VTU operations
 *
 * Fields:
 *      FLUSH_ALL           - Flush all entries.
 *      FLUSH_UNLOCKED      - Flush all unlocked entries in ATU.
 *      LOAD_PURGE_ENTRY    - Load / Purge entry.
 *      GET_NEXT_ENTRY      - Get next ATU or VTU  entry.
 *      FLUSH_ALL_IN_DB     - Flush all entries in a particular DBNum.
 *      FLUSH_UNLOCKED_IN_DB - Flush all unlocked entries in a particular DBNum.
 *      SERVICE_VIOLATONS   - sevice violations of VTU
 *
 */
typedef enum
{
    FLUSH_ALL = 1,	/* for both atu and vtu */
    FLUSH_UNLOCKED,	/* for atu only */
    LOAD_PURGE_ENTRY,	/* for both atu and vtu */
    GET_NEXT_ENTRY,	/* for both atu and vtu */
	FLUSH_ALL_IN_DB,	/* for atu only */
    FLUSH_UNLOCKED_IN_DB,	/* for atu only */
    SERVICE_VIOLATIONS 	/* for vtu only */
}GT_ATU_OPERARION, GT_VTU_OPERATION;

/*
 * typedef: enum GT_ATU_UC_STATE
 *
 * Description:
 *      Enumeration of the address translation unit entry state of unicast
 *      entris.
 *
 * Enumerations:
 *      GT_UC_INVALID   - invalid entry.
 *      GT_UC_DYNAMIC   - unicast dynamic entry.
 *      GT_UC_STATIC    - static unicast entry.
 */
typedef enum
{
    GT_UC_INVALID      = 0,
    GT_UC_DYNAMIC      = 0x1,
    GT_UC_STATIC       = 0xF
} GT_ATU_UC_STATE;

/*
 * typedef: enum GT_ATU_MC_STATE
 *
 * Description:
 *      Enumeration of the address translation unit entry state of multicast
 *      entris.
 *
 * Enumerations:
 *      GT_MC_INVALID         - invalid entry.
 *      GT_MC_MGM_STATIC      - static multicast management entries.
 *      GT_MC_STATIC          - static multicast regular entris.
 *      GT_MC_PRIO_MGM_STATIC - static multicast management entries with
 *                              priority.
 *      GT_MC_PRIO_STATIC     - static multicast regular entris with priority.
 */
typedef enum
{
    GT_MC_INVALID         = 0,
    GT_MC_MGM_STATIC      = 0x6,
    GT_MC_STATIC          = 0x7,
    GT_MC_PRIO_MGM_STATIC = 0xE,
    GT_MC_PRIO_STATIC     = 0xF
} GT_ATU_MC_STATE;

/*
 *  typedef: struct GT_ATU_ENTRY
 *
 *  Description: address tarnslaton unit Entry
 *
 *  Fields:
 *      macAddr    - mac address
 *      portVec    - port Vector.
 *      prio       - entry priority.
 *      entryState - the entry state.
 *		DBNum	   - ATU MAC Address Database number. If multiple address 
 *					databases are not being used, DBNum should be zero.
 *					If multiple address databases are being used, this value
 *					should be set to the desired address database number.
 *
 *  Comment:
 *      The entryState union Type is determine according to the Mac Type.
 */
typedef struct
{
    GT_ETHERADDR  macAddr;
    GT_U8         portVec;
    GT_U8         prio;
    GT_U8         DBNum;
    union
    {
        GT_ATU_UC_STATE ucEntryState;
        GT_ATU_MC_STATE mcEntryState;
    }             entryState;
} GT_ATU_ENTRY;

/*********************** VTU Table Definitions *************************/

/*
 *  typedef: struct GT_VTU_DATA
 *
 *  Description: VLAN  tarnslaton unit Data Register
 *
 *  Fields:
 *      memberTagP - Membership and Egress Tagging
 *                   memberTagP[0] is for Port 0, MemberTagP[1] is for port 1, and so on 
 *
 *  Comment:
 * 	GT_NUM_OF_SWITCH_PORTS is 7 for ClipperShip. 
 * 	In the case of FullSail, there are 3 ports. So, the rest 4 is ignored in memeberTagP
 */
typedef struct
{
    GT_U8     memberTagP[GT_NUM_OF_SWITCH_PORTS];
    GT_U8     portStateP[GT_NUM_OF_SWITCH_PORTS];
} GT_VTU_DATA;

/*
 *  typedef: struct GT_VTU_ENTRY
 *
 *  Description: VLAN  tarnslaton unit Entry
 *
 *  Fields:
 *      DBNum      - database number 
 *      vid        - VLAN ID 
 *      vtuData    - VTU data
 */
typedef struct
{
    GT_U8         DBNum;
    GT_U16         vid;
    GT_VTU_DATA   vtuData;
} GT_VTU_ENTRY;

/*
 *  typedef: struct GT_VTU_INT_STATUS
 *
 *  Description: VLAN tarnslaton unit interrupt status
 *
 *  Fields:
 *      intCause  - VTU Interrupt Cause
 *					GT_VTU_FULL_VIOLATION,GT_MEMEBER_VIOLATION,or
 *					GT_MISS_VIOLATION
 *      SPID      - source port number
 * 					if intCause is GT_VTU_FULL_VIOLATION, it means nothing
 *      vid       - VLAN ID 
 * 					if intCause is GT_VTU_FULL_VIOLATION, it means nothing
 */
typedef struct
{
    GT_U16   vtuIntCause;
    GT_U8    spid;
    GT_U16   vid;
} GT_VTU_INT_STATUS;

/*
* Definition for VTU interrupt
*/
#define GT_MEMBER_VIOLATION		0x4
#define GT_MISS_VIOLATION		0x2
#define GT_VTU_FULL_VIOLATION	0x1

/*
 * typedef: enum GT_PORT_STP_STATE
 *
 * Description: Enumeration of the port spanning tree state.
 *
 * Enumerations:
 *   GT_PORT_DISABLE    - port is disabled.
 *   GT_PORT_BLOCKING   - port is in blocking/listening state.
 *   GT_PORT_LEARNING   - port is in learning state.
 *   GT_PORT_FORWARDING - port is in forwarding state.
 */
typedef enum
{
    GT_PORT_DISABLE = 0,
    GT_PORT_BLOCKING,
    GT_PORT_LEARNING,
    GT_PORT_FORWARDING
} GT_PORT_STP_STATE;

/*
 * typedef: enum GT_EGRESS_MODE
 *
 * Description: Enumeration of the port egress mode.
 *
 * Enumerations:
 *   GT_UNMODIFY_EGRESS - frames are transmited unmodified.
 *   GT_TAGGED_EGRESS   - all frames are transmited tagged.
 *   GT_UNTAGGED_EGRESS - all frames are transmited untagged.
 *   GT_ADD_TAG         - always add a tag. (or double tag)
 */
typedef enum
{
    GT_UNMODIFY_EGRESS = 0,
    GT_TAGGED_EGRESS,
    GT_UNTAGGED_EGRESS,
    GT_ADD_TAG
} GT_EGRESS_MODE;

/*  typedef: enum GT_DOT1Q_MODE */

typedef enum
{
	GT_DISABLE = 0,
    GT_FALLBACK,
	GT_CHECK,
	GT_SECURE
} GT_DOT1Q_MODE;

/* typedef: enum GT_SW_MODE */

typedef enum
{
	GT_CPU_ATTATCHED = 0, /* ports come up disabled */
	GT_BACKOFF,           /* EEPROM attac mode with old half duplex backoff mode */
	GT_STAND_ALONE,       /* ports come up enabled, ignore EEPROM */
        GT_EEPROM_ATTATCHED   /* EEPROM defined prot states */
} GT_SW_MODE;

/* typedef: enum GT_RATE_LIMIT_MODE
 * The ingress limit mode in the rate control register (0xA)
 */

typedef enum
{
	GT_LIMT_ALL = 0, 	/* limit and count all frames */
	GT_LIMIT_FLOOD,      	/* limit and count Broadcast, Multicast and flooded unicast frames */
	GT_LIMIT_BRDCST_MLTCST,	/* limit and count Broadcast and Multicast frames */
        GT_LIMIT_BRDCST   	/* limit and count Broadcast frames */
} GT_RATE_LIMIT_MODE;

/* typedef: enum GT_PRI0_RATE
 * The ingress data rate limit for priority 0 frames 
 */

typedef enum
{
	GT_NO_LIMIT = 0, 	/* Not limited   */
	GT_128K,      		/* 128K bits/sec */
	GT_256K,      		/* 256K bits/sec */
	GT_512,      		/* 512 bits/sec */
	GT_2M,      		/* 1M  bits/sec */
	GT_4M,      		/* 4M  bits/sec */
	GT_8M      		/* 8M  bits/sec */
} GT_PRI0_RATE,GT_EGRESS_RATE;

/*
* typedef: enum GT_PHY_INT
*
* Description: Enumeration of PHY interrupt
*/

#define GT_SPEED_CHANGED 		0x4000
#define GT_DUPLEX_CHANGED		0x2000
#define GT_PAGE_RECEIVED		0x1000
#define GT_AUTO_NEG_COMPLETED	0x800
#define GT_LINK_STATUS_CHANGED	0x400
#define GT_SYMBOL_ERROR			0x200
#define GT_FALSE_CARRIER		0x100
#define GT_FIFO_FLOW			0x80
#define GT_CROSSOVER_CHANGED	0x40
#define GT_POLARITY_CHANGED		0x2
#define GT_JABBER				0x1


/*
* typedef: enum GT_PHY_AUTO_MODE
*
* Description: Enumeration of Autonegotiation mode.
*	Auto for both speed and duplex.
*	Auto for speed only and Full duplex.
*	Auto for speed only and Half duplex.
*	Auto for duplex only and speed 100Mbps.
*	Auto for duplex only and speed 10Mbps.
*/

typedef enum
{
	SPEED_AUTO_DUPLEX_AUTO,
	SPEED_100_DUPLEX_AUTO,
	SPEED_10_DUPLEX_AUTO,
	SPEED_AUTO_DUPLEX_FULL,
	SPEED_AUTO_DUPLEX_HALF
}GT_PHY_AUTO_MODE;



/*
 * typedef: enum GT_INGRESS_MODE
 *
 * Description: Enumeration of the port ingress mode.
 *
 * Enumerations:
 *   GT_UNMODIFY_INGRESS - frames are receive unmodified.
 *   GT_TRAILER_INGRESS  - all frames are received with trailer.
 *   GT_REM_TAG_INGRESS  - remove tag on receive (for double tagging).
 */
typedef enum
{
    GT_UNMODIFY_INGRESS = 0,  /* 0x00 */
    GT_TRAILER_INGRESS,       /* 0x01 */
    GT_UNTAGGED_INGRESS,      /* 0x10 */
    GT_CPUPORT_INGRESS        /* 0x11 */
} GT_INGRESS_MODE;


/*
 *  typedef: enum GT_MC_RATE
 *
 *  Description: Enumeration of the port ingress mode.
 *
 *  Enumerations:
 *      GT_MC_3_PERCENT_RL   - multicast rate is limited to 3 percent.
 *      GT_MC_6_PERCENT_RL   - multicast rate is limited to 6 percent.
 *      GT_MC_12_PERCENT_RL  - multicast rate is limited to 12 percent.
 *      GT_MC_100_PERCENT_RL - unlimited multicast rate.
 */
typedef enum
{
    GT_MC_3_PERCENT_RL = 0,
    GT_MC_6_PERCENT_RL,
    GT_MC_12_PERCENT_RL,
    GT_MC_100_PERCENT_RL,
} GT_MC_RATE;

/*
 *  typedef: struct GT_PORT_COUNTERS
 *
 *  Description: port statistic struct.
 *
 *  Fields:
 *      rxCtr    - port receive counter.
 *      txCtr    - port transmit counter.
 *
 *  Comment:
 */
typedef struct
{
    GT_U16  rxCtr;
    GT_U16  txCtr;
} GT_PORT_STAT;


/*
 **  typedef: struct GT_PORT_Q_COUNTERS
 **
 **  Description: port queue statistic struct.
 **
 **  Fields:
 **      OutQ_Size - port egress queue size coi
 **      Rsv_Size  - ingress reserved e counter
 **
 **/
typedef struct
{
    GT_U16  OutQ_Size;
    GT_U16  Rsv_Size;
} GT_PORT_Q_STAT;

/*
 * typedef: enum GT_CTR_MODE
 *
 * Description: Enumeration of the port counters mode.
 *
 * Enumerations:
 *   GT_CTR_ALL    - In this mode the counters counts Rx receive and transmit
 *                   frames.
 *   GT_CTR_ERRORS - In this mode the counters counts Rx Errors and collisions.
 */
typedef enum
{
    GT_CTR_ALL = 0,
    GT_CTR_ERRORS,
} GT_CTR_MODE;

typedef GT_U32	GT_SEM;

/*
 * semaphore related definitions.
 * User Applications may register Semaphore functions using following definitions
 */
typedef enum
{
	GT_SEM_EMPTY,
	GT_SEM_FULL
} GT_SEM_BEGIN_STATE;

typedef GT_SEM    (*FGT_SEM_CREATE)(GT_SEM_BEGIN_STATE state);
typedef GT_STATUS (*FGT_SEM_DELETE)(GT_SEM semId);
typedef GT_STATUS (*FGT_SEM_TAKE)(GT_SEM semId, GT_U32 timOut);
typedef GT_STATUS (*FGT_SEM_GIVE)(GT_SEM semId);


/*
 * definitions for registering MII access functions.
 *
*/
typedef GT_BOOL (*FGT_READ_MII)(
						GT_QD_DEV*   dev,
						unsigned int phyAddr, 
						unsigned int miiReg, 
						unsigned int* value);
typedef GT_BOOL (*FGT_WRITE_MII)(
						GT_QD_DEV*   dev,
						unsigned int phyAddr, 
						unsigned int miiReg, 
						unsigned int value);

typedef struct _BSP_FUNCTIONS
{
	FGT_READ_MII 	readMii;	/* read MII Registers */
	FGT_WRITE_MII 	writeMii;	/* write MII Registers */
	FGT_SEM_CREATE	semCreate; 	/* create semapore */
	FGT_SEM_DELETE	semDelete; 	/* delete the semapore */
	FGT_SEM_TAKE	semTake;	/* try to get a semapore */
	FGT_SEM_GIVE	semGive;	/* return semaphore */
}BSP_FUNCTIONS;

typedef struct _GT_STATS_COUNTER_SET
{
	GT_U32	InUnicasts;
	GT_U32	InBroadcasts;
	GT_U32	InPause;
	GT_U32	InMulticasts;
	GT_U32	InFCSErr;
	GT_U32	AlignErr;
	GT_U32	InGoodOctets;
	GT_U32	InBadOctets;
	GT_U32	Undersize;
	GT_U32	Fragments;
	GT_U32	In64Octets;
	GT_U32	In127Octets;
	GT_U32	In255Octets;
	GT_U32	In511Octets;
	GT_U32	In1023Octets;
	GT_U32	InMaxOctets;
	GT_U32	Jabber;
	GT_U32	Oversize;
	GT_U32	InDiscards;
	GT_U32	Filtered;
	GT_U32	OutUnicasts;
	GT_U32	OutBroadcasts;
	GT_U32	OutPause;
	GT_U32	OutMulticasts;
	GT_U32	OutFCSErr;
	GT_U32	OutGoodOctets;
	GT_U32	Out64Octets;
	GT_U32	Out127Octets;
	GT_U32	Out255Octets;
	GT_U32	Out511Octets;
	GT_U32	Out1023Octets;
	GT_U32	OutMaxOctets;
	GT_U32	Collisions;
	GT_U32	Late;
	GT_U32	Excessive;
	GT_U32	Multiple;
	GT_U32	Single;
	GT_U32	Deferred;
	GT_U32	OutDiscards;

} GT_STATS_COUNTER_SET;


typedef enum
{
	STATS_InUnicasts = 0,
	STATS_InBroadcasts,
	STATS_InPause,
	STATS_InMulticasts,
	STATS_InFCSErr,
	STATS_AlignErr,
	STATS_InGoodOctets,
	STATS_InBadOctets,
	STATS_Undersize,
	STATS_Fragments,
	STATS_In64Octets,
	STATS_In127Octets,
	STATS_In255Octets,
	STATS_In511Octets,
	STATS_In1023Octets,
	STATS_InMaxOctets,
	STATS_Jabber,
	STATS_Oversize,
	STATS_InDiscards,
	STATS_Filtered,
	STATS_OutUnicasts,
	STATS_OutBroadcasts,
	STATS_OutPause,
	STATS_OutMulticasts,
	STATS_OutFCSErr,
	STATS_OutGoodOctets,
	STATS_Out64Octets,
	STATS_Out127Octets,
	STATS_Out255Octets,
	STATS_Out511Octets,
	STATS_Out1023Octets,
	STATS_OutMaxOctets,
	STATS_Collisions,
	STATS_Late,
	STATS_Excessive,
	STATS_Multiple,
	STATS_Single,
	STATS_Deferred,
	STATS_OutDiscards

} GT_STATS_COUNTERS;

#define GT_MDI_PAIR_NUM     4

/*
 * typedef: enum GT_TEST_STATUS
 *
 * Description: Enumeration of VCT test status
 *
 * Enumerations:
 *      GT_TEST_FAIL    - virtual cable test failed.
 *      GT_NORMAL_CABLE - normal cable.
 *      GT_OPEN_CABLE   - open in cable.
 *      GT_SHORT_CABLE  - short in cable.
 *
 */
typedef enum
{
    GT_TEST_FAIL,
    GT_NORMAL_CABLE,
    GT_OPEN_CABLE,
    GT_SHORT_CABLE,

} GT_TEST_STATUS;


/*
 * typedef: enum GT_NORMAL_CABLE_LEN
 *
 * Description: Enumeration for normal cable length
 *
 * Enumerations:
 *      GT_LESS_THAN_50M - cable length less than 50 meter.
 *      GT_50M_80M       - cable length between 50 - 80 meter.
 *      GT_80M_110M      - cable length between 80 - 110 meter.
 *      GT_110M_140M     - cable length between 110 - 140 meter.
 *      GT_MORE_THAN_140 - cable length more than 140 meter.
 *      GT_UNKNOWN_LEN   - unknown length.
 *
 */
typedef enum
{
    GT_LESS_THAN_50M,
    GT_50M_80M,
    GT_80M_110M,
    GT_110M_140M,
    GT_MORE_THAN_140,
    GT_UNKNOWN_LEN,

} GT_NORMAL_CABLE_LEN;



/*
 * typedef: enum GT_CABLE_LEN
 *
 * Description: Enumeration cable length
 *
 * Enumerations:
 *      normCableLen - cable lenght for normal cable.
 *      errCableLen  - for cable failure the estimate fault distance in meters.
 *
 */
typedef union
{
    GT_NORMAL_CABLE_LEN normCableLen;
    GT_U8               errCableLen;

} GT_CABLE_LEN;



/*
 * typedef: struct GT_CABLE_STATUS
 *
 * Description: virtual cable diagnostic status per MDI pair.
 *
 * Enumerations:
 *      cableStatus - VCT cable status.
 *      cableLen    - VCT cable length.
 *	phyType	    - type of phy (100M phy or Gigabit phy)
 */
typedef struct
{
    GT_TEST_STATUS  cableStatus[GT_MDI_PAIR_NUM];
    GT_CABLE_LEN    cableLen[GT_MDI_PAIR_NUM];
    GT_U16	    phyType;

} GT_CABLE_STATUS;

/*
 * if isGigPhy in GT_CABLE_STATUS is not GT_TRUE, cableStatus and cableLen 
 * will have only 2 pairs available.
 * One is RX Pair and the other is TX Pair.
 */
#define MDI_RX_PAIR		0	/* cableStatus[0] or cableLen[0] */
#define MDI_TX_PAIR		1	/* cableStatus[1] or cableLen[1] */

/* definition for Phy Type */
#define PHY_100M		0 /* 10/100M phy, E3082 or E3083 */
#define PHY_1000M		1 /* Gigabit phy, the rest phys */
#define PHY_10000M		2 /* 10 Gigabit phy, unused */
#define PHY_1000M_B		3 /* Gigabit phy which needs work-around */


/*
 * Typedef: struct GT_SYS_CONFIG
 *
 * Description: System configuration Parameters struct.
 *
 * Fields:
 *	devNum		- Switch Device Number 
 *      cpuPortNum      - The physical port used to connect the device to CPU.
 *                        This is the port to which packets destined to CPU are
 *                        forwarded.
 *      initPorts       - Whether to initialize the ports state.
 *                          GT_FALSE    - leave in default state.
 *                          GT_TRUE     - Initialize to Forwarding state.
 *	BSPFunctions	- Group of BSP specific functions.
 *				SMI Read/Write and Semaphore Related functions.
 */
typedef struct
{
    GT_U8         devNum;
    GT_U8         cpuPortNum;
    GT_BOOL       initPorts;
    BSP_FUNCTIONS BSPFunctions;
}GT_SYS_CONFIG;

/*
 * Typedef: struct GT_QD_DEV
 *
 * Description: Includes Tapi layer switch configuration data.
 *
 * Fields:
 *      deviceId        - The device type identifier.
 *      revision        - The device revision number.
 *      baseRegAddr     - Switch Base Register address.
 *      numOfPorts      - Number of active ports.
 *      cpuPortNum      - The physical port connected to the CPU.
 *      maxPhyNum       - max configurable Phy address.
 *	stpMode         - current switch STP mode (0 none, 1 en, 2 dis)
 *      portsList       - List of all logical ports in system.
 *      devName	        - name of the device
 *	atuRegsSem	- Semaphore for ATU access
 *	vtuRegsSem	- Semaphore for VTU access
 *	statsRegsSem	- Semaphore for RMON counter access
 *	fgtReadMii	- platform specific SMI register Read function
 *	fgtWriteMii	- platform specific SMI register Write function
 *	semCreate	- function to create semapore
 *	semDelete	- function to delete the semapore
 *	semTake		- function to get a semapore
 *	semGive		- function to return semaphore
 *	appData	        - application data that user may use
 */
struct _GT_QD_DEV
{
    GT_U32          deviceId;
    GT_U8   		revision;
    GT_U8		    devNum;
    GT_U8		    devEnabled;
    GT_U8      		baseRegAddr;
    GT_U8           numOfPorts;
    GT_U8           cpuPortNum;
    GT_U8           maxPhyNum;
    GT_U8		    stpMode;
    GT_U32		    devName;
    GT_SEM		    atuRegsSem;
    GT_SEM		    vtuRegsSem;
    GT_SEM		    statsRegsSem;

    FGT_READ_MII 	fgtReadMii;
    FGT_WRITE_MII 	fgtWriteMii;

    FGT_SEM_CREATE	semCreate; 	/* create semapore */
    FGT_SEM_DELETE	semDelete; 	/* delete the semapore */
    FGT_SEM_TAKE	semTake;	/* try to get a semapore */
    FGT_SEM_GIVE	semGive;	/* return semaphore */
    void*		    appData;

};

/*******************************************************************************
* gfdbSetAtuSize
*
* DESCRIPTION:
*       Sets the Mac address table size.
*
* INPUTS:
*       size    - Mac address table size.
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
GT_STATUS gfdbSetAtuSize
(
    IN GT_QD_DEV *dev,
    IN ATU_SIZE size
);


/*******************************************************************************
* gfdbGetAgingTimeRange
*
* DESCRIPTION:
*       Gets the maximal and minimum age times that the hardware can support.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       maxTimeout - max aging time in secounds.
*       minTimeout - min aging time in secounds.
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
GT_STATUS gfdbGetAgingTimeRange
(
    IN GT_QD_DEV *dev,
    OUT GT_U32 *maxTimeout,
    OUT GT_U32 *minTimeout
);


/*******************************************************************************
* gfdbSetAgingTimeout
*
* DESCRIPTION:
*       Sets the timeout period in seconds for aging out dynamically learned
*       forwarding information. The standard recommends 300 sec.
*
* INPUTS:
*       timeout - aging time in seconds.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbSetAgingTimeout
(
    IN GT_QD_DEV *dev,
    IN GT_U32 timeout
);



/*******************************************************************************
* gfdbGetAtuDynamicCount
*
* DESCRIPTION:
*       Gets the current number of dynamic unicast entries in this
*       Filtering Database.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numDynEntries - number of dynamic entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - vlan does not exist.
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetAtuDynamicCount
(
    IN GT_QD_DEV *dev,
    OUT GT_U32 *numDynEntries
);



/*******************************************************************************
* gfdbGetAtuEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic MAC address entry from the ATU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*
* COMMENTS:
*       Search starts from Mac[00:00:00:00:00:00]
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryFirst
(
    IN GT_QD_DEV *dev,
    OUT GT_ATU_ENTRY    *atuEntry
);



/*******************************************************************************
* gfdbGetAtuEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic MAC address from the specified Mac Addr.
*
* INPUTS:
*       atuEntry - the Mac Address to start the search.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*       Search starts from atu.macAddr[xx:xx:xx:xx:xx:xx] specified by the
*       user.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryNext
(
    IN GT_QD_DEV *dev,
    INOUT GT_ATU_ENTRY  *atuEntry
);



/*******************************************************************************
* gfdbFindAtuMacEntry
*
* DESCRIPTION:
*       Find FDB entry for specific MAC address from the ATU.
*
* INPUTS:
*       atuEntry - the Mac address to search.
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       atuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFindAtuMacEntry
(
    IN GT_QD_DEV *dev,
    INOUT GT_ATU_ENTRY  *atuEntry,
    OUT GT_BOOL         *found
);



/*******************************************************************************
* gfdbFlush
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the MAC Address
*       Table.
*
* INPUTS:
*       flushCmd - the flush operation type.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlush
(
    IN GT_QD_DEV *dev,
    IN GT_FLUSH_CMD flushCmd
);

/*******************************************************************************
* gfdbFlushInDB
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the particular
*       ATU Database (DBNum). If multiple address databases are being used, this
*		API can be used to flush entries in a particular DBNum database.
*
* INPUTS:
*       flushCmd - the flush operation type.
*		DBNum	 - ATU MAC Address Database Number. 
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlushInDB
(
    IN GT_QD_DEV *dev,
    IN GT_FLUSH_CMD flushCmd,
	IN GT_U8 DBNum
);

/*******************************************************************************
* gfdbAddMacEntry
*
* DESCRIPTION:
*       Creates the new entry in MAC address table.
*
* INPUTS:
*       macEntry    - mac address entry to insert to the ATU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_FAIL           - on error
*       GT_NO_RESOURCE    - failed to allocate a t2c struct
*       GT_OUT_OF_CPU_MEM - oaMalloc failed
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbAddMacEntry
(
    IN GT_QD_DEV *dev,
    IN GT_ATU_ENTRY *macEntry
);



/*******************************************************************************
* gfdbDelMacEntry
*
* DESCRIPTION:
*       Deletes MAC address entry.
*
* INPUTS:
*       macAddress - mac address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*       For SVL mode vlan Id is ignored.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbDelMacEntry
(
    IN GT_QD_DEV *dev,
    IN GT_ETHERADDR  *macAddress
);

/*******************************************************************************
* gfdbDelAtuEntry
*
* DESCRIPTION:
*       Deletes ATU entry.
*
* INPUTS:
*       atuEntry - the ATU entry to be deleted.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbDelAtuEntry
(
    IN GT_QD_DEV *dev,
    IN GT_ATU_ENTRY  *atuEntry
);

/*******************************************************************************
* gfdbLearnEnable
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       en - GT_TRUE for enable  or GT_FALSE otherwise
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbLearnEnable
(
    IN GT_QD_DEV *dev,
    IN GT_BOOL  en
);



/*******************************************************************************
* gstpSetMode
*
* DESCRIPTION:
*       This routine Enable the Spanning tree.
*
* INPUTS:
*       en - GT_TRUE for enable, GT_FALSE for disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       when enabled, this function sets all port to blocking state, and inserts
*       the BPDU MAC into the ATU to be captured to CPU, on disable all port are
*       being modified to be in forwarding state.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpSetMode
(
    IN GT_QD_DEV *dev,
    IN GT_BOOL  en
);



/*******************************************************************************
* gstpSetPortState
*
* DESCRIPTION:
*       This routine set the port state.
*
* INPUTS:
*       port  - the logical port number.
*       state - the port state to set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstpSetPortState
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT           port,
    IN GT_PORT_STP_STATE  state
);



/*******************************************************************************
* gstpGetPortState
*
* DESCRIPTION:
*       This routine returns the port state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       state - the current port state.
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
GT_STATUS gstpGetPortState
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT           port,
    OUT GT_PORT_STP_STATE  *state
);

/*******************************************************************************
* gprtSetEgressMode
*
* DESCRIPTION:
*       This routine set the egress mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the egress mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetEgressMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT        port,
    IN GT_EGRESS_MODE  mode
);



/*******************************************************************************
* gprtGetEgressMode
*
* DESCRIPTION:
*       This routine get the egress mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the egress mode.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetEgressMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT        port,
    OUT GT_EGRESS_MODE  *mode
);



/*******************************************************************************
* gprtSetVlanTunnel
*
* DESCRIPTION:
*       This routine sets the vlan tunnel mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the vlan tunnel mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetVlanTunnel
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);



/*******************************************************************************
* gprtGetVlanTunnel
*
* DESCRIPTION:
*       This routine get the vlan tunnel mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the vlan tunnel mode..
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetVlanTunnel
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);


/*******************************************************************************
* gprtSetIGMPSnoop
*
* DESCRIPTION:
*       This routine set the IGMP Snoop. When set to one and this port receives
*       IGMP frame, the frame is switched to the CPU port, overriding all other 
*     	switching decisions, with exception for CPU's Trailer.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for IGMP Snoop or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIGMPSnoop
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* gprtGetIGMPSnoop
*
* DESCRIPTION:
*       This routine get the IGMP Snoop mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: IGMP Snoop enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIGMPSnoop
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/* the following two APIs are added to support clippership */

/*******************************************************************************
* gprtSetHeaderMode
*
* DESCRIPTION:
*       This routine set ingress and egress header mode of a switch port. 
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for header mode  or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetHeaderMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* gprtGetHeaderMode
*
* DESCRIPTION:
*       This routine gets ingress and egress header mode of a switch port. 
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: header mode enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHeaderMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);


/*******************************************************************************
* gvlnSetPortVlanPorts
*
* DESCRIPTION:
*       This routine sets the port VLAN group port membership list.
*
* INPUTS:
*       port        - logical port number to set.
*       memPorts    - array of logical ports.
*       memPortsLen - number of members in memPorts array
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanPorts
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_LPORT memPorts[],
    IN GT_U8    memPortsLen
);



/*******************************************************************************
* gvlnGetPortVlanPorts
*
* DESCRIPTION:
*       This routine gets the port VLAN group port membership list.
*
* INPUTS:
*       port        - logical port number to set.
*
* OUTPUTS:
*       memPorts    - array of logical ports.
*       memPortsLen - number of members in memPorts array
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanPorts
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_LPORT memPorts[],
    OUT GT_U8    *memPortsLen
);




/*******************************************************************************
* gvlnSetPortUserPriLsb
*
* DESCRIPTION:
*       This routine Set the user priority (VPT) LSB bit, to be added to the
*       user priority on the egress.
*
* INPUTS:
*       port       - logical port number to set.
*       userPriLsb - GT_TRUE for 1, GT_FALSE for 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortUserPriLsb
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  userPriLsb
);



/*******************************************************************************
* gvlnGetPortUserPriLsb
*
* DESCRIPTION:
*       This routine gets the user priority (VPT) LSB bit.
*
* INPUTS:
*       port       - logical port number to set.
*
* OUTPUTS:
*       userPriLsb - GT_TRUE for 1, GT_FALSE for 0.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortUserPriLsb
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *userPriLsb
);


/*******************************************************************************
* gvlnSetPortVid
*
* DESCRIPTION:
*       This routine Set the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*       vid  - the port vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVid
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_U16   vid
);


/*******************************************************************************
* gvlnGetPortVid
*
* DESCRIPTION:
*       This routine Get the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*
* OUTPUTS:
*       vid  - the port vlan id.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVid
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_U16   *vid
);

/*******************************************************************************
* gvlnSetPortVlanDBNum
*
* DESCRIPTION:
*       This routine sets the port VLAN database number (DBNum).
    IN GT_QD_DEV *dev,
*
* INPUTS:
*       port	- logical port number to set.
*       DBNum 	- database number for this port 
*
* OUTPUTS:
*       None.
*
* RETURNS:IN GT_INGRESS_MODE mode
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanDBNum
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_U8    DBNum
);


/*******************************************************************************
* gvlnGetPortVlanDBNum
*
* DESCRIPTION:IN GT_INGRESS_MODE mode
*       This routine gets the port VLAN database number (DBNum).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       DBNum 	- database number for this port 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanDBNum
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_U8    *DBNum
);

/********************************************************************
* gvlnSetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine sets the port 802.1q mode (11:10) 
    IN GT_QD_DEV *dev,
*
* INPUTS:
*       port	- logical port number to set.
*       mode 	- 802.1q mode for this port 
*
* OUTPUTS:
*       None.
*
* RETURNS:IN GT_INGRESS_MODE mode
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanDot1qMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT 	port,
    IN GT_DOT1Q_MODE	mode
);

/*******************************************************************************
* gvlnGetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine gets the port 802.1q mode (bit 11:10).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       mode 	- 802.1q mode for this port 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanDot1qMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_DOT1Q_MODE    *mode
);


/********************************************************************
* gvlnSetPortVlanForceDefaultVID
*
* DESCRIPTION:
*       This routine sets the port 802.1q mode (11:10) 
    IN GT_QD_DEV *dev,
*
* INPUTS:
*       port	- logical port number to set.
*       mode 	- 802.1q mode for this port 
*
* OUTPUTS:
*       None.
*
* RETURNS:IN GT_INGRESS_MODE mode
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanForceDefaultVID
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT 	port,
    IN GT_BOOL  	mode
);

/*******************************************************************************
* gvlnGetPortVlanForceDefaultVID
*
* DESCRIPTION:
*       This routine gets the port mode for ForceDefaultVID (bit 12).
    IN GT_QD_DEV *dev,
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       mode 	- ForceDefaultVID mode for this port 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanForceDefaultVID
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT 	 port,
    OUT GT_BOOL    	*mode
);

/*******************************************************************************
* eventSetActive
*
* DESCRIPTION:
*       This routine enables/disables the receive of an hardware driven event.
*
* INPUTS:
*       eventType - the event type. any combination of the folowing: 
*       	GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,  
*       	GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT
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
GT_STATUS eventSetActive
(
    IN GT_QD_DEV *dev,
    IN GT_U32 		eventType
);

/*******************************************************************************
* eventGetIntStatus
*
* DESCRIPTION:
*       This routine reads an hardware driven event status.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       intCause -  It provides the source of interrupt of the following:
*       GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,  
*       GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT
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
GT_STATUS eventGetIntStatus
(
    IN GT_QD_DEV *dev,
    OUT GT_U16 	   *intCause
);

/*******************************************************************************
* gvtuGetIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of VTU interrupt occured
*
* INPUTS:
*       	intType - the type of interrupt which causes an interrupt.
*			any combination of 
*			GT_MEMEBER_VIOLATION,
*			GT_MISS_VIOLATION,
*			GT_VTU_FULL_VIOLATION
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gvtuGetIntStatus
(
    IN GT_QD_DEV *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
);

/*******************************************************************************
* gvtuGetEntryCount
*
* DESCRIPTION:
*       Gets the current number of entries in the VTU table
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numEntries - number of VTU entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - vlan does not exist.
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryCount
(
    IN GT_QD_DEV *dev,
    OUT GT_U32 *numEntries
);

/*******************************************************************************
* gvtuGetEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic entry from the VTU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       vtuEntry - match VTU entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*
* COMMENTS:
*       Search starts from vid of all one's
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryFirst
(
    IN GT_QD_DEV *dev,
    OUT GT_VTU_ENTRY    *vtuEntry
);

/*******************************************************************************
* gvtuGetEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic VTU entry from the specified VID.
*
* INPUTS:
*       vtuEntry - the VID to start the search.
*
* OUTPUTS:
*       vtuEntry - match VTU  entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*       Search starts from the VID specified by the user.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuGetEntryNext
(
    IN GT_QD_DEV *dev,
    INOUT GT_VTU_ENTRY  *vtuEntry
);

/*******************************************************************************
* gvtuFindVidEntry
*
* DESCRIPTION:
*       Find VTU entry for a specific VID, it will return the entry, if found, 
*       along with its associated data 
*
* INPUTS:
*       vtuEntry - contains the VID to search for.
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       vtuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuFindVidEntry
(
    IN GT_QD_DEV *dev,
    INOUT GT_VTU_ENTRY  *vtuEntry,
    OUT GT_BOOL         *found
);

/*******************************************************************************
* gvtuFlush
*
* DESCRIPTION:
*       This routine removes all entries from VTU Table.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuFlush
(
    IN GT_QD_DEV *dev
);

/*******************************************************************************
* gvtuAddEntry
*
* DESCRIPTION:
*       Creates the new entry in VTU table based on user input.
*
* INPUTS:
*       vtuEntry    - vtu entry to insert to the VTU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK             - on success
*       GT_FAIL           - on error
*       GT_OUT_OF_CPU_MEM - oaMalloc failed
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuAddEntry
(
    IN GT_QD_DEV *dev,
    IN GT_VTU_ENTRY *vtuEntry
);

/*******************************************************************************
* gvtuDelEntry
*
* DESCRIPTION:
*       Deletes VTU entry specified by user.
*
* INPUTS:
*       vtuEntry - the VTU entry to be deleted 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvtuDelEntry
(
    IN GT_QD_DEV *dev,
    IN GT_VTU_ENTRY *vtuEntry
);


/*******************************************************************************
* gprtPhyReset
*
* DESCRIPTION:
*       This routine preforms PHY reset.
*
* INPUTS:
* port - The logical port number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 0.15 - Reset
*
*******************************************************************************/

GT_STATUS gprtPhyReset
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port
);


/*******************************************************************************
* gprtSetPortLoopback
*
* DESCRIPTION:
* Enable/Disable Internal Port Loopback
*
* INPUTS:
* port - logical port number
* enable - If GT_TRUE, enable loopback mode
* If GT_FALSE, disable loopback mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/

GT_STATUS gprtSetPortLoopback
(
IN GT_QD_DEV *dev,
IN GT_LPORT port,
IN GT_BOOL enable
);


/*******************************************************************************
* gprtSetPortSpeed
*
* DESCRIPTION:
* Sets speed for a specific logical port.
*
* INPUTS:
* port - logical port number
* speed - port speed,  	GT_TRUE=100Mb/s
* 			GT_FALSE=10Mb/s
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.13 - Speed Selection (LSB)
*
*******************************************************************************/

GT_STATUS gprtSetPortSpeed
(
IN GT_QD_DEV *dev,
IN GT_LPORT port,
IN GT_BOOL  speed
);


/*******************************************************************************
* gprtPortAutoNegEnable
*
* DESCRIPTION:
* Enable/disable an Auto-Negotiation for duplex mode on specific
* logical port.
*
* INPUTS:
* port - logical port number
* state - GT_TRUE for enable Auto-Negotiation for duplex mode,
* GT_FALSE otherwise
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.12 - Auto-Negotiation Enable
*
*******************************************************************************/

GT_STATUS gprtPortAutoNegEnable
(
IN GT_QD_DEV *dev,
IN GT_LPORT port,
IN GT_BOOL state
);


/*******************************************************************************
* gprtPortPowerDown
*
* DESCRIPTION:
* Enable/disable (power down) on specific logical port.
*
* INPUTS:
* port - logical port number
* state - GT_TRUE: power down
*         GT_FALSE: normal operation
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.11 - Power Down
*
*******************************************************************************/

GT_STATUS gprtPortPowerDown
(
IN GT_QD_DEV *dev,
IN GT_LPORT port,
IN GT_BOOL state
);


/*******************************************************************************
* gprtPortRestartAutoNeg
*
* DESCRIPTION:
* Enable/disable an Auto-Negotiation of interface speed on specific
* logical port.
*
* INPUTS:
* port - logical port number
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.9 - Restart Auto-Negotiation
*
*******************************************************************************/

GT_STATUS gprtPortRestartAutoNeg
(
IN GT_QD_DEV *dev,
IN GT_LPORT port
);


/*******************************************************************************
* gprtSetPortDuplexMode
*
* DESCRIPTION:
* Sets duplex mode for a specific logical port.
*
* INPUTS:
* port - logical port number
* dMode - dulpex mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/

GT_STATUS gprtSetPortDuplexMode
(
IN GT_QD_DEV *dev,
IN GT_LPORT port,
IN GT_BOOL dMode
);


/*******************************************************************************
* gprtSetPortAutoMode
*
* DESCRIPTION:
*       This routine sets up the port with given Auto Mode.
*		Supported mode is as follows:
*		- Auto for both speed and duplex.
*		- Auto for speed only and Full duplex.
*		- Auto for speed only and Half duplex.
*		- Auto for duplex only and speed 100Mbps.
*		- Auto for duplex only and speed 10Mbps.
*		
*
* INPUTS:
* port - The logical port number
* mode - Auto Mode to be written
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.8, 4.7, 4.6, and 4.5 Autonegotiation Advertisement Register
*******************************************************************************/

GT_STATUS gprtSetPortAutoMode
(
IN GT_QD_DEV *dev,
IN GT_LPORT port,
IN GT_PHY_AUTO_MODE mode
);

/*******************************************************************************
* gprtSetPause
*
* DESCRIPTION:
*       This routine will set the pause bit in Autonegotiation Advertisement
*		Register. And restart the autonegotiation.
*
* INPUTS:
* port - The logical port number
* state - either GT_TRUE(for enable) or GT_FALSE(for disable)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

GT_STATUS gprtSetPause
(
IN GT_QD_DEV *dev,
IN GT_LPORT port,
IN GT_BOOL state
);


/*******************************************************************************
* gprtPhyIntEnable
*
* DESCRIPTION:
* Enable/Disable one PHY Interrupt
* This register determines whether the INT# pin is asserted when an interrupt 
* event occurs. When an interrupt occurs, the corresponding bit is set and
* remains set until register 19 is read via the SMI. When interrupt enable
* bits are not set in register 18, interrupt status bits in register 19 are 
* still set when the corresponding interrupt events occur. However, the INT# 
* is not asserted.
*
* INPUTS:
* port    - logical port number
* intType - the type of interrupt to enable/disable. any combination of 
*			GT_SPEED_CHANGED,
*			GT_DUPLEX_CHANGED,
*			GT_PAGE_RECEIVED,
*			GT_AUTO_NEG_COMPLETED,
*			GT_LINK_STATUS_CHANGED,
*			GT_SYMBOL_ERROR,
*			GT_FALSE_CARRIER,
*			GT_FIFO_FLOW,
*			GT_CROSSOVER_CHANGED,
*			GT_POLARITY_CHANGED, and
*			GT_JABBER
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 18
*
*******************************************************************************/

GT_STATUS gprtPhyIntEnable
(
IN GT_QD_DEV *dev,
IN GT_LPORT   port,
IN GT_U16	intType
);


/*******************************************************************************
* gprtGetPhyIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of  interrupt occured
*
* INPUTS:
* port - logical port number
* intType - the type of interrupt which causes an interrupt.
*			any combination of 
*			GT_SPEED_CHANGED,
*			GT_DUPLEX_CHANGED,
*			GT_PAGE_RECEIVED,
*			GT_AUTO_NEG_COMPLETED,
*			GT_LINK_STATUS_CHANGED,
*			GT_SYMBOL_ERROR,
*			GT_FALSE_CARRIER,
*			GT_FIFO_FLOW,
*			GT_CROSSOVER_CHANGED,
*			GT_POLARITY_CHANGED, and
*			GT_JABBER
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 19
*
*******************************************************************************/

GT_STATUS gprtGetPhyIntStatus
(
IN GT_QD_DEV *dev,
IN  GT_LPORT port,
OUT  GT_U16* intType
);

/*******************************************************************************
* gprtGetPhyIntPortSummary
*
* DESCRIPTION:
* Lists the ports that have active interrupts. It provides a quick way to 
* isolate the interrupt so that the MAC or switch does not have to poll the
* interrupt status register (19) for all ports. Reading this register does not
* de-assert the INT# pin
*
* INPUTS:
* none
*
* OUTPUTS:
* GT_U8 *intPortMask - bit Mask with the bits set for the corresponding 
* phys with active interrupt. E.g., the bit number 0 and 2 are set when 
* port number 0 and 2 have active interrupt
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 88E3081 data sheet register 20
*
*******************************************************************************/

GT_STATUS gprtGetPhyIntPortSummary
(
IN GT_QD_DEV *dev,
OUT GT_U16 *intPortMask
);



/*******************************************************************************
* gprtSetForceFc
*
* DESCRIPTION:
*       This routine set the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetForceFc
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  force
);



/*******************************************************************************
* gprtGetForceFc
*
* DESCRIPTION:
*       This routine get the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetForceFc
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *force
);



/*******************************************************************************
* gprtSetTrailerMode
*
* DESCRIPTION:
*       This routine set the egress trailer mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetTrailerMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);



/*******************************************************************************
* gprtGetTrailerMode
*
* DESCRIPTION:
*       This routine get the egress trailer mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetTrailerMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtSetIngressMode
*
* DESCRIPTION:
*       This routine set the ingress mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the ingress mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIngressMode
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT        port,
    IN GT_INGRESS_MODE mode
);



/*******************************************************************************
* gprtGetIngressMode
*
* DESCRIPTION:
*       This routine get the ingress mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the ingress mode.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIngressMode
(
    IN GT_QD_DEV        *dev,
    IN  GT_LPORT        port,
    OUT GT_INGRESS_MODE *mode
);



/*******************************************************************************
* gprtSetMcRateLimit
*
* DESCRIPTION:
*       This routine set the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetMcRateLimit
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT     port,
    IN GT_MC_RATE   rate
);



/*******************************************************************************
* gprtGetMcRateLimit
*
* DESCRIPTION:
*       This routine Get the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetMcRateLimit
(
    IN GT_QD_DEV    *dev,
    IN  GT_LPORT    port,
    OUT GT_MC_RATE  *rate
);



/*******************************************************************************
* gprtSetCtrMode
*
* DESCRIPTION:
*       This routine sets the port counters mode of operation.
*
* INPUTS:
*       mode  - the counter mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetCtrMode
(
    IN GT_QD_DEV *dev,
    IN GT_CTR_MODE  mode
);



/*******************************************************************************
* gprtClearAllCtr
*
* DESCRIPTION:
*       This routine clears all port counters.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtClearAllCtr
(
    IN GT_QD_DEV *dev
);


/*******************************************************************************
* gprtGetPortCtr
*
* DESCRIPTION:
*       This routine gets the port counters.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       ctr - the counters value.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPortCtr
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT        port,
    OUT GT_PORT_STAT    *ctr
);




/*******************************************************************************
* gprtGetPartnerLinkPause
*
* DESCRIPTION:
*       This routine retrives the link partner pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPartnerLinkPause
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetSelfLinkPause
*
* DESCRIPTION:
*       This routine retrives the link pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSelfLinkPause
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetResolve
*
* DESCRIPTION:
*       This routine retrives the resolve state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Done  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetResolve
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetLinkState
*
* DESCRIPTION:
*       This routine retrives the link state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Up  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetLinkState
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *state
);



/*******************************************************************************
* gprtGetPortMode
*
* DESCRIPTION:
*       This routine retrives the port mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPortMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtGetPhyMode
*
* DESCRIPTION:
*       This routine retrives the PHY mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII PHY  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPhyMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtGetDuplex
*
* DESCRIPTION:
*       This routine retrives the port duplex mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for Full  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDuplex
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gprtGetSpeed
*
* DESCRIPTION:
*       This routine retrives the port speed.
*
* INPUTS:
*       speed - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for 100Mb/s  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSpeed
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *speed
);

/*******************************************************************************
* gprtSetDuplex
*
* DESCRIPTION:
*       This routine sets the duplex mode of MII/SNI/RMII ports.
*
* INPUTS:
*       port - 	the logical port number.
*				(for FullSail, it will be port 2, and for ClipperShip, 
*				it could be either port 5 or port 6.)
*       mode -  GT_TRUE for Full Duplex,
*				GT_FALSE for Half Duplex.
*
* OUTPUTS: None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDuplex
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    IN  GT_BOOL  mode
);


/*******************************************************************************
* gqosSetPortDefaultTc
*
* DESCRIPTION:
*       Sets the default traffic class for a specific port.
*
* INPUTS:
*       port      - logical port number
*       trafClass - default traffic class of a port.
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
GT_STATUS gcosSetPortDefaultTc
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_U8    trafClass
);



/*******************************************************************************
* gqosSetPrioMapRule
*
* DESCRIPTION:
*       This routine sets priority mapping rule.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetPrioMapRule
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);



/*******************************************************************************
* gqosGetPrioMapRule
*
* DESCRIPTION:
*       This routine get the priority mapping rule.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetPrioMapRule
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);



/*******************************************************************************
* gqosIpPrioMapEn
*
* DESCRIPTION:
*       This routine enables the IP priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosIpPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  en
);



/*******************************************************************************
* gqosGetIpPrioMapEn
*
* DESCRIPTION:
*       This routine return the IP priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetIpPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *en
);



/*******************************************************************************
* gqosUserPrioMapEn
*
* DESCRIPTION:
*       This routine enables the user priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosUserPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  en
);



/*******************************************************************************
* gqosGetUserPrioMapEn
*
* DESCRIPTION:
*       This routine return the user priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetUserPrioMapEn
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *en
);



/*******************************************************************************
* gcosGetUserPrio2Tc
*
* DESCRIPTION:
*       Gets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosGetUserPrio2Tc
(
    IN GT_QD_DEV *dev,
    IN  GT_U8    userPrior,
    OUT GT_U8   *trClass
);


/*******************************************************************************
* gcosSetUserPrio2Tc
*
* DESCRIPTION:
*       Sets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority of a port.
*       trClass   - the Traffic Class the received frame is assigned.
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
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosSetUserPrio2Tc
(
    IN GT_QD_DEV *dev,
    IN GT_U8    userPrior,
    IN GT_U8    trClass
);


/*******************************************************************************
* gcosGetDscp2Tc
*
* DESCRIPTION:
*       This routine retrieves the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to query.
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosGetDscp2Tc
(
    IN GT_QD_DEV *dev,
    IN  GT_U8   dscp,
    OUT GT_U8   *trClass
);


/*******************************************************************************
* gcosSetDscp2Tc
*
* DESCRIPTION:
*       This routine sets the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to map.
*       trClass - the Traffic Class the received frame is assigned.
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
*       Table - UserPrio2Tc
*
*******************************************************************************/
GT_STATUS gcosSetDscp2Tc
(
    IN GT_QD_DEV *dev,
    IN GT_U8    dscp,
    IN GT_U8    trClass
);


/*******************************************************************************
* qdLoadDriver
*
* DESCRIPTION:
*       QuarterDeck Driver Initialization Routine. 
*       This is the first routine that needs be called by system software. 
*       It takes sysCfg from system software, and retures a pointer (*dev) 
*       to a data structure which includes infomation related to this QuarterDeck
*       device. This pointer (*dev) is then used for all the API functions. 
*
* INPUTS:
*       sysCfg      - Holds system configuration parameters.
*
* OUTPUTS:
*       dev         - Holds general system information.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_ALREADY_EXIST    - if device already started
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
* 	qdUnloadDriver is provided when the driver is not to be used anymore.
*
*******************************************************************************/
GT_STATUS qdLoadDriver
(
    IN  GT_SYS_CONFIG   *sysCfg,
    OUT GT_QD_DEV	*dev
);


/*******************************************************************************
* qdUnloadDriver
*
* DESCRIPTION:
*       This function unloads the QuaterDeck Driver.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       1.  This function should be called only after successful execution of
*           qdLoadDriver().
*
*******************************************************************************/
GT_STATUS qdUnloadDriver
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* sysEnable
*
* DESCRIPTION:
*       This function enables the system for full operation.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS sysEnable
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* gsysSwReset
*
* DESCRIPTION:
*       This routine preforms switch software reset.
*
* INPUTS:
*       None.
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
GT_STATUS gsysSwReset
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* gsysSetDiscardExcessive
*
* DESCRIPTION:
*       This routine set the Discard Excessive state.
*
* INPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
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
GT_STATUS gsysSetDiscardExcessive
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL en
);



/*******************************************************************************
* gsysGetDiscardExcessive
*
* DESCRIPTION:
*       This routine get the Discard Excessive state.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
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
GT_STATUS gsysGetDiscardExcessive
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL *en
);



/*******************************************************************************
* gsysSetSchedulingMode
*
* DESCRIPTION:
*       This routine set the Scheduling Mode.
*
* INPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
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
GT_STATUS gsysSetSchedulingMode
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL mode
);



/*******************************************************************************
* gsysGetSchedulingMode
*
* DESCRIPTION:
*       This routine get the Scheduling Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
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
GT_STATUS gsysGetSchedulingMode
(
    IN GT_QD_DEV *dev,
    OUT GT_BOOL *mode
);



/*******************************************************************************
* gsysSetMaxFrameSize
*
* DESCRIPTION:
*       This routine Set the max frame size allowed.
*
* INPUTS:
*       mode - GT_TRUE max size 1522, GT_FALSE max size 1535.
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
GT_STATUS gsysSetMaxFrameSize
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL mode
);



/*******************************************************************************
* gsysGetMaxFrameSize
*
* DESCRIPTION:
*       This routine Get the max frame size allowed.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE max size 1522, GT_FALSE max size 1535.
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
GT_STATUS gsysGetMaxFrameSize
(
    IN GT_QD_DEV *dev,
    OUT GT_BOOL *mode
);



/*******************************************************************************
* gsysReLoad
*
* DESCRIPTION:
*       This routine cause to the switch to reload the EEPROM.
*
* INPUTS:
*       None.
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
GT_STATUS gsysReLoad
(
    IN GT_QD_DEV* dev
);


/*******************************************************************************
* gsysSetWatchDog
*
* DESCRIPTION:
*       This routine Set the the watch dog mode.
*
* INPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
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
GT_STATUS gsysSetWatchDog
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL en
);



/*******************************************************************************
* gsysGetWatchDog
*
* DESCRIPTION:
*       This routine Get the the watch dog mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
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
GT_STATUS gsysGetWatchDog
(
    IN GT_QD_DEV* dev,
    OUT GT_BOOL *en
);


/*******************************************************************************
* gsysSetDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets the full duplex pause src Mac Address.
*
* INPUTS:
*       mac - The Mac address to be set.
*
* OUTPUTS:
*       None.
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
GT_STATUS gsysSetDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    IN GT_ETHERADDR *mac
);


/*******************************************************************************
* gsysGetDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets the full duplex pause src Mac Address.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mac - the Mac address.
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
GT_STATUS gsysGetDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    OUT GT_ETHERADDR *mac
);



/*******************************************************************************
* gsysSetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
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
GT_STATUS gsysSetPerPortDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL en
);



/*******************************************************************************
* gsysGetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
*
* OUTPUTS:
*       None.
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
GT_STATUS gsysGetPerPortDuplexPauseMac
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL *en
);


/*******************************************************************************
* gsysReadMiiRegister
*
* DESCRIPTION:
*       This routine reads QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know exactly which phy address(0 ~ 0x1F) will be read.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysReadMiiReg
(
    IN GT_QD_DEV* dev,
    IN  GT_U32	phyAddr,
    IN  GT_U32	regAddr,
    OUT GT_U32	*data
);

/*******************************************************************************
* gsysWriteMiiRegister
*
* DESCRIPTION:
*       This routine writes QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know exactly which phy address(0 ~ 0x1F) will be read.
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
*       regAddr - The register's address.
*       data    - data to be written.
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysWriteMiiReg
(
    IN GT_QD_DEV* dev,
    IN  GT_U32	phyAddr,
    IN  GT_U32	regAddr,
    IN  GT_U16	data
);

/*******************************************************************************
* gsysGetSW_Mode
*
* DESCRIPTION:
*       This routine get the Switch mode. These two bits returen 
*       the current value of the SW_MODE[1:0] pins.
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
GT_STATUS gsysGetSW_Mode
(
    IN GT_QD_DEV* dev,
    IN GT_SW_MODE *mode
);

/*******************************************************************************
* gsysGetInitReady
*
* DESCRIPTION:
*       This routine get the InitReady bit. This bit is set to a one when the ATU,
*       the Queue Controller and the Statistics Controller are done with their 
*       initialization and are ready to accept frames.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: switch is ready, GT_FALSE otherwise.
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
GT_STATUS gsysGetInitReady
(
    IN GT_QD_DEV* dev,
    IN GT_BOOL *mode
);


/*******************************************************************************
* gstatsFlushAll
*
* DESCRIPTION:
*       Flush All RMON counters for all ports.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsFlushAll
(
    IN GT_QD_DEV* dev
);

/*******************************************************************************
* gstatsFlushPort
*
* DESCRIPTION:
*       Flush All RMON counters for a given port.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsFlushPort
(
    IN GT_QD_DEV* dev,
    IN GT_LPORT	port
);

/*******************************************************************************
* gstatsGetPortCounter
*
* DESCRIPTION:
*       This routine gets a specific RMON counter of the given port
*
* INPUTS:
*       port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*       statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter
(	
        GT_QD_DEV* dev,
	GT_LPORT			port,
	GT_STATS_COUNTERS	counter,
	GT_U32				*statsData
);

/*******************************************************************************
* gstatsGetPortAllCounters
*
* DESCRIPTION:
*       This routine gets all RMON counters of the given port
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       statsCounterSet - points to GT_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounters
(
        GT_QD_DEV* dev,
	GT_LPORT		port,
	GT_STATS_COUNTER_SET	*statsCounterSet
);


/*******************************************************************************
* grcSetLimitMode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*       mode 	- rate control ingress limit mode. 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*******************************************************************************/
GT_STATUS grcSetLimitMode
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT 	     port,
    IN GT_RATE_LIMIT_MODE    mode
);

/*******************************************************************************
* grcGetLimitMode
*
* DESCRIPTION:
*       This routine gets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       mode 	- rate control ingress limit mode. 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetLimitMode
(
    IN GT_QD_DEV* dev,
    IN  GT_LPORT port,
    OUT GT_RATE_LIMIT_MODE    *mode
);

/*******************************************************************************
* grcSetPri3Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS grcSetPri3Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* grcGetPri3Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       
* OUTPUTS:
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri3Rate
(
    IN GT_QD_DEV* dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* grcSetPri2Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS grcSetPri2Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* grcGetPri2Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       
* OUTPUTS:
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri2Rate
(
    IN GT_QD_DEV*            dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* grcSetPri1Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS grcSetPri1Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* grcGetPri1Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       
* OUTPUTS:
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri1Rate
(
    IN GT_QD_DEV*            dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* grcSetPri0Rate
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number.
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached 
*       	  or exceeded. 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcSetPri0Rate
(
    IN GT_QD_DEV*            dev,
    IN GT_LPORT        port,
    IN GT_PRI0_RATE    rate
);

/*******************************************************************************
* grcGetPri0Rate
*
* DESCRIPTION:
*       This routine gets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number to set.
*
* OUTPUTS:
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached 
*       	  or exceeded. 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetPri0Rate
(
    IN GT_QD_DEV*            dev,
    IN  GT_LPORT port,
    OUT GT_PRI0_RATE    *rate
);

/*******************************************************************************
* grcSetBytesCount
*
* DESCRIPTION:
*       This routine sets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	  - logical port number to set.
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcSetBytesCount
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT        	port,
    IN GT_BOOL 		limitMGMT,
    IN GT_BOOL 		countIFG,
    IN GT_BOOL 		countPre
);

/*******************************************************************************
* grcGetBytesCount
*
* DESCRIPTION:
*       This routine gets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	- logical port number 
*
* OUTPUTS:
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetBytesCount
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT        	port,
    IN GT_BOOL 		*limitMGMT,
    IN GT_BOOL 		*countIFG,
    IN GT_BOOL 		*countPre
);

/*******************************************************************************
* grcSetEgressRate
*
* DESCRIPTION:
*       This routine sets the port's egress data limit.
*
* INPUTS:
*       port	- logical port number.
*       rate    - egress data rate limit.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcSetEgressRate
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT        port,
    IN GT_EGRESS_RATE  rate
);

/*******************************************************************************
* grcGetEgressRate
*
* DESCRIPTION:
*       This routine gets the port's egress data limit.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       rate    - egress data rate limit.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS grcGetEgressRate
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT port,
    OUT GT_EGRESS_RATE  *rate
);


/*******************************************************************************
* gpavSetPAV
*
* DESCRIPTION:
*       This routine sets the Port Association Vector 
*
* INPUTS:
*       port	- logical port number.
*       pav 	- Port Association Vector 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS gpavSetPAV
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT	port,
    IN GT_U16	pav
);

/*******************************************************************************
* gpavGetPAV
*
* DESCRIPTION:
*       This routine gets the Port Association Vector 
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       pav 	- Port Association Vector 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS gpavGetPAV
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT port,
    OUT GT_U16    *pav
);

/*******************************************************************************
* gpavSetIngressMonitor
*
* DESCRIPTION:
*       This routine sets the Ingress Monitor bit in the PAV.
*
* INPUTS:
*       port - the logical port number.
*       mode - the ingress monitor bit in the PAV
*              GT_FALSE: Ingress Monitor enabled 
*              GT_TRUE:  Ingress Monitor disabled 
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*******************************************************************************/
GT_STATUS gpavSetIngressMonitor
(
    IN GT_QD_DEV*       dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
);

/*******************************************************************************
* gpavGetIngressMonitor
*
* DESCRIPTION:
*       This routine gets the Ingress Monitor bit in the PAV.
*
* INPUTS:
*       port - the logical port number.
*       
* OUTPUTS:
*       mode - the ingress monitor bit in the PAV
*              GT_FALSE: Ingress Monitor enabled 
*              GT_TRUE:  Ingress Monitor disabled 
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
*******************************************************************************/
GT_STATUS gpavGetIngressMonitor
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
);

/*******************************************************************************
* gvctGetCableStatus
*
* DESCRIPTION:
*       This routine perform the virtual cable test for the requested port,
*       and returns the the status per MDI pair.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*       cableStatus - the port copper cable status.
*       cableLen    - the port copper cable length.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
*******************************************************************************/
GT_STATUS gvctGetCableDiag
(
    IN GT_QD_DEV*       dev,
    IN  GT_LPORT        port,
    OUT GT_CABLE_STATUS *cableStatus
);



/*******************************************************************************
* gtVersion
*
* DESCRIPTION:
*       This function returns the version of the QuarterDeck SW suite.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       version     - QuarterDeck software version.
*
* RETURNS:
*       GT_OK on success,
*       GT_BAD_PARAM on bad parameters,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gtVersion
(
    OUT GT_VERSION   *version
);


#endif /* __mv_qd_h */
