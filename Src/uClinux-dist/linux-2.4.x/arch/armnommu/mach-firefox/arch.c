/*
 *  linux/arch/arm/mach-firefox/arch.c
 *
 *  Architecture specific fixups.  This is where any
 *  parameters in the params struct are fixed up, or
 *  any additional architecture specific information
 *  is pulled from the params struct.
 */
#include <linux/tty.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/init.h>

#include <asm/elf.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>

extern void genarch_init_irq(void);

MACHINE_START(FIREFOX, "MV88E62x8")
     BOOT_MEM(0x00020000, 0xffffffff, 0xffffffff)
     /* If the last 2 parmas are 0... head-armv.S will return 0 for the arch... */
     MAINTAINER("Marvell")
     INITIRQ(genarch_init_irq)
MACHINE_END
