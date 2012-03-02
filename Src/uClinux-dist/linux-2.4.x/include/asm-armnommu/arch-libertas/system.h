/*
 * linux/include/asm-armnommu/arch-libertas/system.h
 * 2001 Mindspeed
 * Hacked @Marvell 2003
 */

#define ARM_RESET_VECTOR  0xFFFF0000

static inline void arch_idle(void)
{
	while (!current->need_resched && !hlt_counter)
	  cpu_do_idle(IDLE_WAIT_SLOW);
}

extern inline void arch_reset(char mode)
{

	void (*reset_fun)(void);

	__asm__ __volatile__ (
	"mov    ip, #0\n"
	"mcr    p15, 0, ip, c7, c14, 0\n"
	"mrc    p15, 0, ip, c1, c0, 0\n"
	"bic    ip, ip, #0x000f\n"
	"bic    ip, ip, #0x2100\n"
	"mcr    p15, 0, ip, c1, c0, 0\n");

	reset_fun = (void (*)) ARM_RESET_VECTOR;

	reset_fun();

	/* should not come here
	 */
}


