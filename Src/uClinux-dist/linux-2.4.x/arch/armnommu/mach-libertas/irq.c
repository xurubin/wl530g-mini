/*
 *  linux/arch/arm/mach-libertas/irq.c
 *
 *  Copyright (C) 1999 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/init.h>

#include <asm/mach/irq.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>

void libertas_mask_irq(unsigned int irq)
{
    *((u32*)LIBERTAS_INT_ENABLE_CLR) = (1 << irq);
}	

void libertas_unmask_irq(unsigned int irq)
{
    *((u32*)LIBERTAS_INT_ENABLE_SET) = (1 << irq);
}

void libertas_mask_ack_irq(unsigned int irq)
{
    libertas_mask_irq(irq);
    switch(irq) {
    case IRQ_TIMER1: /* TODO: change timer1 to timer2 for a better resolution (1Khz -> 1Mhz) */
	inl(LIBERTAS_TIMER_INT_SOURCE);
	outl(1, LIBERTAS_TIMER_INT_SOURCE);
	break;
    }

}

void 
irq_init_irq(void)
{
  int irq;
  
  /* 
   * Relocate execption vectors to address 0x0000000.
   * This shouldn't be done here - but in an architecture global place.
   */
  __asm__ ("mrc  p15, 0, r0, c1, c0, 0;
            ldr  r1, =0xffffdfff
            and  r0, r1, r0;
            mcr  p15, 0, r0, c1, c0, 0");

	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= libertas_mask_ack_irq;
		irq_desc[irq].mask	= libertas_mask_irq;
		irq_desc[irq].unmask	= libertas_unmask_irq;
	}
}


