/*
 * linux/include/asm-arm/arch-libertas/hardware.h
 * for Marvell Firefox 88W8100
 * 2002 Hedley Rainnie
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#define HARD_RESET_NOW()

/* 
 * interrupts 
 */


#define INTBase_Addr  0x90008000

#define Stat          0x0
#define Msk           0x8
#define Set 	      0x8
#define Clr 	      0xc

#define	LIBERTAS_INT_MASK        (INTBase_Addr+Msk) 
#define	LIBERTAS_INT_ENABLE_CLR  (INTBase_Addr+Clr) 
#define	LIBERTAS_INT_ENABLE_SET  (INTBase_Addr+Set) 
#define	LIBERTAS_INT_STATUS      (INTBase_Addr+Stat)


#define LIBERTAS_TIMER_BASE      0x90009000

#define LIBERTAS_TIMER1_LENGTH   	(LIBERTAS_TIMER_BASE + 0x00)
#define LIBERTAS_TIMER2_LENGTH   	(LIBERTAS_TIMER_BASE + 0x04)
#define LIBERTAS_TIMER3_LENGTH   	(LIBERTAS_TIMER_BASE + 0x08)
#define LIBERTAS_TIMER4_LENGTH   	(LIBERTAS_TIMER_BASE + 0x0c)
#define LIBERTAS_TIMER_CONTROL   	(LIBERTAS_TIMER_BASE + 0x10)
#define LIBERTAS_TIMER1_VALUE    	(LIBERTAS_TIMER_BASE + 0x14)
#define LIBERTAS_TIMER2_VALUE    	(LIBERTAS_TIMER_BASE + 0x18)
#define LIBERTAS_TIMER3_VALUE    	(LIBERTAS_TIMER_BASE + 0x1C)
#define LIBERTAS_TIMER4_VALUE    	(LIBERTAS_TIMER_BASE + 0x18)
#define LIBERTAS_TIMER_INT_SOURCE	(LIBERTAS_TIMER_BASE + 0x24)
#define LIBERTAS_TIMER_INT_MASK  	(LIBERTAS_TIMER_BASE + 0x28)

/*
 *****************************************************************************
 *
 *	Fast Ethernet Controller Interface Registers
 *
 *****************************************************************************
 */

#define libertas_reg_read(x)   	*(volatile unsigned int *)(0x80008000 + x)
#define libertas_reg_write(x,y) *(volatile unsigned int *)(0x80008000 + x) = y;

#define	LIBERTAS_ENET_PHY_ADDR  		0x000
#define	LIBERTAS_ENET_ESMIR     		0x010

#define	LIBERTAS_ENET_E0PCR     		0x400
#define	LIBERTAS_ENET_E0PCXR    		0x408
#define	LIBERTAS_ENET_E0PCMR    		0x410
#define	LIBERTAS_ENET_E0PSR     		0x418
#define	LIBERTAS_ENET_E0SPR     		0x420
#define	LIBERTAS_ENET_E0HTPR    		0x428
#define	LIBERTAS_ENET_E0FCSAL   		0x430
#define	LIBERTAS_ENET_E0FCSAH   		0x438
#define	LIBERTAS_ENET_E0SDCR    		0x440
#define	LIBERTAS_ENET_E0SDCMR   		0x448
#define	LIBERTAS_ENET_E0ICR     		0x450
#define	LIBERTAS_ENET_E0IW2CR   		0x454
#define	LIBERTAS_ENET_E0IMR     		0x458
#define	LIBERTAS_ENET_E0FRDP0   		0x480
#define	LIBERTAS_ENET_E0FRDP1   		0x484
#define	LIBERTAS_ENET_E0FRDP2   		0x488
#define	LIBERTAS_ENET_E0FRDP3   		0x48c
#define	LIBERTAS_ENET_E0CRDP0   		0x4a0
#define	LIBERTAS_ENET_E0CRDP1   		0x4a4
#define	LIBERTAS_ENET_E0CRDP2   		0x4a8
#define	LIBERTAS_ENET_E0CRDP3   		0x4ac
#define	LIBERTAS_ENET_E0CTDP0   		0x4e0
#define	LIBERTAS_ENET_E0CTDP1   		0x4e4
#define	LIBERTAS_ENET_0_DSCP2P0L		0x460
#define	LIBERTAS_ENET_0_DSCP2P0H		0x464
#define	LIBERTAS_ENET_0_DSCP2P1L		0x468
#define	LIBERTAS_ENET_0_DSCP2P1H		0x46c
#define	LIBERTAS_ENET_0_VPT2P   		0x470
#define	LIBERTAS_ENET_0_MIB_CTRS		0x500

#endif  /* _ASM_ARCH_HARDWARE_H */


