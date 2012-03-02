/*
 * arch/v850/kernel/intv.S -- Interrupt vectors
 *
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#include <asm/clinkage.h>
#include <asm/irq.h>
#include <asm/machdep.h>
#include <asm/entry.h>

	/* These are compiled and loaded into high memory, then
	   copied into place in mach_early_setup */
	.section	.intv.common, "ax"
	.org	0x0
	brai	C_SYMBOL_NAME(start);	// Kernel entry point
	brai	C_SYMBOL_NAME(trap);	// Exception handler
	brai	C_SYMBOL_NAME(irq);	// Interrupt handler

