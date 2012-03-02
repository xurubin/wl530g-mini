/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* 
* FILENAME:    $Workfile: mvFF_eth.h $ 
* REVISION:    $Revision: 4 $ 
* LAST UPDATE: $Modtime: 3/03/03 12:14p $ 
* 
* mvFF_eth.h
*
* DESCRIPTION:
*		Ethernet driver for FireFox/Libertas
*
* DEPENDENCIES:   
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#ifndef _MVFF_ETH_H_
#define _MVFF_ETH_H_

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <asm/arch/hardware.h>

#include <mv_os.h>
#include <mv_unimac.h>


#undef  ETH_DBG_ECHO
#undef  ETH_DBG_INFO
#undef  ETH_DBG_MC	/* MC dbg information */
#undef  ETH_DBG_WARN
#define ETH_DBG_ERROR
#undef  ETH_DBG_TX_OUTSTANDING

#define ENABLE_MC	/* Enable Promiscuous mode for MC */
#undef  ENABLE_MC	/* Enable Promiscuous mode for MC */

#define DESCRIPTOR_SIZE            sizeof(gt_dma_desc)
/* #define Q_INDEX_LIMIT              (PAGE_SIZE / DESCRIPTOR_SIZE) */
#define Q_INDEX_LIMIT (64)

#define NUM_TX_QUEUES  2
#define NUM_RX_QUEUES  4
#define MVFF_TX_QUEUE  0
#ifdef CONFIG_ARCH_LIBERTAS
#define MVFF_RX_QUEUE  0
#else
#define MVFF_RX_QUEUE  3
#endif

#define FIREFOX_INT_THRE           5
#define GT_ENET_DESC_OWNERSHIP     (1<<31)
#define GT_ENET_DESC_INT_ENABLE    (1<<23)
#define GT_ENET_DESC_ERROR_SUMMARY (1<<15)
#define MAX_BUFF_SIZE              1536

#define ETH_ADDR_GAP 0x400 /* This is dummy - code must be fixed */

#define HEADER_SIZE (2)
#define TRAILER_SIZE (4)
#define MIN_ETH_PACKET_LEN (60)

s32 mvFF_eth_init(struct net_device *);
s32 mvFF_eth_open(struct net_device *);
s32 mvFF_eth_start_xmit(struct sk_buff *, struct net_device *);
s32 mvFF_eth_stop(struct net_device *);
void mvFF_eth_int_handler(s32, void *, struct pt_regs *);
s32 mvFF_eth_set_mac_address(struct net_device *, void *);
struct net_device_stats *mvFF_eth_get_stats(struct net_device *);


GT_STATUS getNetConfig(OUT MV_UNM_CONFIG* unmConfig);
GT_STATUS mvDisassociatePort(int qdPort, int fromVlanId, int newPortsBitMask);
GT_STATUS mvAssociatePort(int qdPort, int toVlanId, int newPortsBitMask, int numOfPorts);

typedef volatile struct gt_dma_desc_struct {
	volatile u32 command_status;
	volatile union {
		volatile struct {
			u16 reserved;
			u16 bytes;
		} tx;
		volatile struct {
			u16 bytesReceived;
			u16 bufferBytes;
		} rx;
	} count;

	volatile void *data;	/* Physical address, only read by the DMA engine. */
	volatile u32 next;	/* Physical address, only read by the DMA engine. */
	/*
	 * Force sizeof(firefoxenetDMAdescriptor) == sizeof(cacheline) 
	 * Not yet sure exactly why this is necessary but the FIREFOX-B
	 * part apparently has (yet another) bug that shows itself without
	 * this padding.  The symptom is that all Enet comms simply stop.
	 */
	u32 cachelineSizePadding[4];
} gt_dma_desc;

typedef enum {
  HW_UNKNOWN = 0,
  HW_INIT,
  HW_READY,
  HW_ACTIVE
} HW_STATE;

/* HW SW IF logicall structure */
/* A single instance           */
typedef struct mv_eth_HwSwIf {
  gt_dma_desc *TXqueue[NUM_TX_QUEUES];
  gt_dma_desc *RXqueue[NUM_RX_QUEUES];
  struct sk_buff *TXskbuff[NUM_TX_QUEUES][Q_INDEX_LIMIT];
  struct sk_buff *RXskbuff[NUM_RX_QUEUES][Q_INDEX_LIMIT];
  u32 TXindex[NUM_TX_QUEUES];
  u32 RXindex[NUM_RX_QUEUES];
  u32 TXskbIndex[NUM_TX_QUEUES];
  HW_STATE hwState;
  
  struct tq_struct rx_task;
} mv_eth_hw;

typedef struct gt_eth_priv_stat {
  u32 tx_needed_copy;
  /* more ... */
}priv_stat;

typedef struct gt_eth_priv_struct {
  u8 port;
  struct net_device_stats stat;
  struct gt_eth_priv_stat privstat; 
  struct mii_if_info mii_if;
  bool inited;
  int vid;       /* the VLAN ID (VID) */
} gt_eth_priv;


/* cache flushing helpers */

#define flush_dcache_addr_size( A, N )     \
    flush_dcache_range( ((u32)A), ((u32)(A)+(N)) )

#define flush_dcache(x) flush_dcache_range( ((u32)x) , ((u32)x) + sizeof(*(x)))

#define invalidate_dcache(x) invalidate_dcache_range( ((u32)x) , ((u32)x) + sizeof(*(x)))

u32 uncachedPages(u32 pages);




/* this file has all the ethernet low level definitions */


#define ETHERNET_PORTS_DIFFERENCE_OFFSETS       0x400

#define ETHERNET0_HH_PRIORITY                   11
#define ETHERNET0_LL_PRIORITY                   2
#define ETHERNET1_HH_PRIORITY                   12

/* this macros are used to enable access to SMI_REG */
#define SMI_OP_CODE_BIT_READ                    1
#define SMI_OP_CODE_BIT_WRITE                   0
#define SMI_BUSY                                1<<28
#define READ_VALID                              1<<27

#ifdef CONFIG_ZUMA_V2
#define PHY_LXT97x
#define PHY_ADD0                                0
#define PHY_ADD1  				1
#elif CONFIG_MOT_MVP
#define PHY_LXT97x
#define PHY_ADD0                                4
#define PHY_ADD1                                5
#else
#define PHY_ADD0                                4
#define PHY_ADD1                                5
#define PHY_ADD2                                6
#endif

/* this macros are used to enable access to ETHERNET_PCXR */
#define OVERRIDE_RX_PRIORITY                    1<<8
#define MIB_CLEAR_MODE                          1<<16

/* this macros are used to enable access to ETHERNET_SDCMR */
#define START_TX_HIGH       1<<23
#define START_TX_LOW        1<<24
#define ENABLE_RX_DMA       1<<7
#define ABORT_RECEIVE       1<<15
#define STOP_TX_HIGH        1<<16
#define STOP_TX_LOW         1<<17
#define ABORT_TRANSMIT      1<<31

/* this macros are used to enable access to ETHERNET_SDCR */
#define ETHERNET_SDMA_BURST_SIZE          3

typedef unsigned int ETHERNET_PCR;
typedef unsigned int ETHERNET_PCXR;
typedef unsigned int ETHERNET_PCMR;
typedef unsigned int ETHERNET_PSR;
typedef unsigned int ETHERNET_SDCMR;
typedef unsigned int ETHERNET_SDCR;

typedef unsigned int PHY_ADD_REG;
typedef unsigned int SMI_REG;

typedef struct mibCounters {
	unsigned int byteReceived;
	unsigned int byteSent;
	unsigned int framesReceived;
	unsigned int framesSent;
	unsigned int totalByteReceived;
	unsigned int totalFramesReceived;
	unsigned int broadcastFramesReceived;
	unsigned int multicastFramesReceived;
	unsigned int cRCError;
	unsigned int oversizeFrames;
	unsigned int fragments;
	unsigned int jabber;
	unsigned int collision;
	unsigned int lateCollision;
	unsigned int frames64;
	unsigned int frames65_127;
	unsigned int frames128_255;
	unsigned int frames256_511;
	unsigned int frames512_1023;
	unsigned int frames1024_MaxSize;
	unsigned int macRxError;
	unsigned int droppedFrames;
	unsigned int outMulticastFrames;
	unsigned int outBroadcastFrames;
	unsigned int undersizeFrames;
} STRUCT_MIB_COUNTERS;

/* set port config value */
inline void
set_port_config(u32 value, unsigned int port);

/* get port config value */
inline u32
get_port_config(unsigned int port);

inline void
enableFiltering(u32 port);

#ifdef PHY_LXT97x
#undef THREE_ETHERNET_RMII_PORTS
#define TWO_ETHERNET_MII_PORTS
#else
#define THREE_ETHERNET_RMII_PORTS
#undef TWO_ETHERNET_MII_PORTS
#endif

#define ETHERNET_PORT2                      2
#define ETHERNET_PORT1                      1
#define ETHERNET_PORT0                      0

#define MAX_NUMBER_OF_MPSC_PORTS                3


#ifdef THREE_ETHERNET_RMII_PORTS
/********/
/* RMII */
/********/

#define NUMBER_OF_ETHERNET_PORTS                3
#define NUMBER_OF_MPSC_PORTS                    2
#define MRR_REG_VALUE                           0x7ffe38

/* connect MPSC0 + 3 ports of RMII */
#define SERIAL_PORT_MULTIPLEX_REGISTER_VALUE    0x1102
/* GALILEO value */
/* 0000 0000 0001 0001  20 - RMII */
/*                      16 - clear MIB counters */
/* 1000 1000 0000 0000  15:14 - 2048 (10) */
/*#define PORT_CONTROL_EXTEND_VALUE             0x00118000 */

/* PPCBoot value */
/* 0000 0000 0000 0001  20 - RMII */
/* 0100 1101 0000 0000  15:14 - 1536 (01) */
/*                      11 - dont force link pass */
/*                      10 - disable FC AN */
/*                       8 - prio override */
/*#define PORT_CONTROL_EXTEND_VALUE             0x00104d00 */

/* Montavista value */
/* 0000 0000 0011 0000  21 - DSCP */
/*                      20 - RMII */
/* 0100 1100 0010 0010  15:14 - 1536 (01) */
/*                      11 - dont force link pass */
/*                      10 - disable fc AN */
/*                      5:3 - 8pkt high, 1 low (100) */
/*                      1 - bpdu trap */
/*#define PORT_CONTROL_EXTEND_VALUE               0x00304c20 */
#define PORT_CONTROL_EXTEND_VALUE               0x003004420 /* Force Link Pass */

#define ETHERNET_DOWNLOADING_PORT               ETHERNET_PORT2

#else				/* if !THREE_ETHERNET_RMII_PORTS */

#ifdef TWO_ETHERNET_MII_PORTS
/*******/
/* MII */
/*******/

#define NUMBER_OF_ETHERNET_PORTS                2
#define NUMBER_OF_MPSC_PORTS                    2
#define MRR_REG_VALUE                           0x7ffe38
/* connect MPSC0 + 2 ports of MII */
#define SERIAL_PORT_MULTIPLEX_REGISTER_VALUE    0x1101
/* GALILEO value */
/* 0000 0000 0000 0001  16 - clear MIB counters */
/* 1000 1000 0000 0000  15:14 - 2048 (10) */
/*#define PORT_CONTROL_EXTEND_VALUE             0x00018000 */

/* PPCBoot (ZUMA) value */
/* 0000 0000 0000 0000 */
/* 0100 1101 0000 0000  15:14 - 1536 (01) */
/*                      11 - dont force link pass */
/*                      10 - disable FC AN */
/*                       8 - prio override */
/*#define PORT_CONTROL_EXTEND_VALUE             0x00004d00 */

/* Montavista (ZUMA) value */
/* 0000 0000 0010 0000  21 - DSCP */
/* 0100 1100 0010 0010  15:14 - 1536 (01) */
/*                      11 - dont force link pass */
/*                      10 - disable fc AN */
/*                      5:3 - 8pkt high, 1 low (100) */
/*                      1 - bpdu trap */
#ifndef CONFIG_MOT_MVP
#define PORT_CONTROL_EXTEND_VALUE               0x00204c20
#else
/* Hack, don't allow 10mbit for now */
/* 0000 0000 0010 1100  21 - DSCP */
/* 0100 1100 0010 0010  18 - Speed (1-> 100mbit) */
/*                      19 - SpeedEn (1-> disable speed AN) */
/*                      15:14 - 1536 (01) */
/*                      11 - dont force link pass */
/*                      10 - disable fc AN */
/*                      5:3 - 8pkt high, 1 low (100) */
/*                      1 - bpdu trap */
/*  */
#define PORT_CONTROL_EXTEND_VALUE		0x002c4c20
#endif

#define ETHERNET_DOWNLOADING_PORT           ETHERNET_PORT1

#endif				/* endif TWO_ETHERNET_MII_PORTS */
#endif				/* endif !THREE_ETHERNET_RMII_PORTS */

#define LL_QUEUE_PRIORITY                       1
#define L_QUEUE_PRIORITY                        2
#define H_QUEUE_PRIORITY                        3
#define HH_QUEUE_PRIORITY                       4

#define NUMBER_OF_MIB_COUNTERS		25

#define	TIME_OUT			10	/* 1/6 SEC */


#endif				/* #ifndef _MVFF_ETH_H_ */
