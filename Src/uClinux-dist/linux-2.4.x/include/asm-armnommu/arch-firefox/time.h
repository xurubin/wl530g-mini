/*
 * linux/include/asm-arm/arch-firefox/time.h
 * 2002 Marvell
 */

#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/timex.h>

extern struct irqaction timer_irq;

#define KERNEL_TIMER 1

#if (KERNEL_TIMER==0)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC0
#elif (KERNEL_TIMER==1)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC1
#elif (KERNEL_TIMER==2)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC2
#else
#error Wierd -- KERNEL_TIMER is not defined or something....
#endif

extern unsigned long firefox_gettimeoffset(void);
extern void firefox_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);

extern /* __inline__ */ void setup_timer (void)
{
#if 0
    outl(10000, FIREFOX_TIMER2_LENGTH);
    outl((volatile unsigned int)(0xf << 4), FIREFOX_TIMER_CONTROL); /* load, int, active, continuous t1 */
    outl((volatile unsigned int)(1<<1), FIREFOX_TIMER_INT_SOURCE); /* Intr source */
    outl((volatile unsigned int)(1<<1), FIREFOX_TIMER_INT_MASK); /* Not marked as RW or as a mask in the doc, this is the mask... */
    outl(IRQ_MASK_TIMER2, FIREFOX_INT_MASK);
    /* gettimeoffset = firefox_gettimeoffset; */
    timer_irq.handler = firefox_timer_interrupt;
    setup_arm_irq(IRQ_TIMER2, &timer_irq);
#else
    outl(10, FIREFOX_TIMER1_LENGTH);
    outl((volatile unsigned int)(0xf), FIREFOX_TIMER_CONTROL); /* load, int, active, continuous t1 */
    outl((volatile unsigned int)(1<<0), FIREFOX_TIMER_INT_SOURCE); /* Intr source */
    outl((volatile unsigned int)(1<<0), FIREFOX_TIMER_INT_MASK); /* Not marked as RW or as a mask in the doc, this is the mask... */
    outl(IRQ_MASK_TIMER1, FIREFOX_INT_MASK);
    /* gettimeoffset = firefox_gettimeoffset; */
    timer_irq.handler = firefox_timer_interrupt;
    setup_arm_irq(IRQ_TIMER1, &timer_irq);

#endif
    
}

#endif /* __ASM_ARCH_TIME_H__ */





