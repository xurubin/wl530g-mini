#ifndef __MICROBLAZE_HW_IRQ_H__
#define __MICROBLAZE_HW_IRQ_H__

#ifndef __OPTIMIZE__
#define extern static
#endif

extern inline void hw_resend_irq (struct hw_interrupt_type *h, unsigned int i)
{
}

#ifndef __OPTIMIZE__
#undef extern
#endif

#endif /* __MICROBLAZE_HW_IRQ_H__ */
