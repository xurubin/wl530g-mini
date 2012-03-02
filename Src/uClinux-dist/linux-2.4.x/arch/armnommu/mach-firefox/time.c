/*
 * time.c  Timer functions for Firefox
 */

#include <linux/time.h>
#include <linux/timex.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

extern unsigned long firefox_gettimeoffset(void)
{
	u16 elapsed;
	u16 latch;
	/* 
	 * Compute the elapsed count. The current count tells us how
	 * many counts remain until the next interrupt. latch tells us
	 * how many counts total occur between interrupts. Subtract to
	 * get the counts since the last interrupt. Borrowed for Firefox
	 * from dsc21
	 */
	latch = inw(FIREFOX_TIMER1_LENGTH);
	elapsed = latch - inw(FIREFOX_TIMER1_VALUE);
	return (unsigned long)((elapsed * tick) / latch);
}

void firefox_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	do_timer(regs);
}
