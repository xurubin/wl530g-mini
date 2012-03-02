/*
 *  linux/arch/arm/mach-libertas/arch.c
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
extern void fixup_libertas(struct machine_desc *desc, struct param_struct *params, char **cmdline, struct meminfo *mi);

MACHINE_START(LIBERTAS, "MV88W85x0")
     BOOT_MEM(0xD0000000, 0xffffffff, 0xffffffff)
     /* If the last 2 parmas are 0... head-armv.S will return 0 for the arch... */
     MAINTAINER("Marvell")
     FIXUP(fixup_libertas)
     INITIRQ(genarch_init_irq)
MACHINE_END
