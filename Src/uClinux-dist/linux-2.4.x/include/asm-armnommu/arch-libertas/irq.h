/*
 * asm/arch-libertas/irq.h:
 * 2002 Marvell
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__


#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>

#define fixup_irq(x) (x)

extern void libertas_mask_irq(unsigned int irq);
extern void libertas_unmask_irq(unsigned int irq);
extern void libertas_mask_ack_irq(unsigned int irq);

extern void irq_init_irq(void);

#endif /* __ASM_ARCH_IRQ_H__ */
