/*
 * asm/arch-firefox/irqs.h:
 * 2002 Marvell
 */
#ifndef __ASM_ARCH_IRQS_H__
#define __ASM_ARCH_IRQS_H__

#define IRQ_FIRQ	0
#define IRQ_PROG_IRQ	1
#define IRQ_COMM_Rx	2
#define IRQ_COMM_Tx	3
#define IRQ_TIMER1	4
#define IRQ_TIMER2	5
#define IRQ_TIMER3	6
#define IRQ_TIMER4	7
#define IRQ_HOST	8
#define IRQ_FE		9
#define IRQ_WLAN	10
#define IRQ_UART	11
#define IRQ_GPIO	12
#define IRQ_CLK_SER	13
#define IRQ_PMU		14
#define IRQ_DMA0	15
#define IRQ_DMA1	16
#define IRQ_AEU		17

#define IRQ_MASK_FIRQ	(1 << IRQ_FIRQ)
#define IRQ_MASK_PROG_IRQ	(1 << IRQ_PROG_IRQ)
#define IRQ_MASK_COMM_Rx	(1 << IRQ_COMM_Rx)
#define IRQ_MASK_COMM_Tx	(1 << IRQ_COMM_Tx)
#define IRQ_MASK_TIMER1	(1 << IRQ_TIMER1)
#define IRQ_MASK_TIMER2	(1 << IRQ_TIMER2)
#define IRQ_MASK_TIMER3	(1 << IRQ_TIMER3)
#define IRQ_MASK_TIMER4	(1 << IRQ_TIMER4)
#define IRQ_MASK_HOST	(1 << IRQ_HOST)
#define IRQ_MASK_FE		(1 << IRQ_FE)
#define IRQ_MASK_WLAN	(1 << IRQ_WLAN)
#define IRQ_MASK_UART	(1 << IRQ_UART)
#define IRQ_MASK_GPIO	(1 << IRQ_GPIO)
#define IRQ_MASK_CLK_SER	(1 << IRQ_CLK_SER)
#define IRQ_MASK_PMU		(1 << IRQ_PMU)
#define IRQ_MASK_DMA0	(1 << IRQ_DMA0)
#define IRQ_MASK_DMA1	(1 << IRQ_DMA1)
#define IRQ_MASK_AEU		(1 << IRQ_AEU)

#define NR_IRQS           (IRQ_AEU + 1)

#endif /* __ASM_ARCH_IRQS_H__ */
