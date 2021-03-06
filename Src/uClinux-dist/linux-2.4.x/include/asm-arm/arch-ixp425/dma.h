/*
 * linux/include/asm-arm/arch-80200fcc/dma.h
 *
 * Copyright (C) 2001 MontaVista Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * 2002: Modified for IXP425 by Intel Corporation.
 *
 */
#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

#include <linux/config.h>
#include <asm/page.h>
#include <asm/sizes.h>
#include <asm/arch/hardware.h>

#define MAX_DMA_ADDRESS		(PAGE_OFFSET + SZ_64M)

/* No DMA */
#define MAX_DMA_CHANNELS	0

/*
 * Only first 64MB of memory can be accessed via PCI.
 * We use GFP_DMA to allocate safe buffers to do map/unmap
 */
static inline void __arch_adjust_zones(int node, unsigned long *zone_size, 
	unsigned long *zhole_size) 
{
	unsigned int sz = SZ_64M >> PAGE_SHIFT;

	if (node)
		return;

	zone_size[1] = zone_size[0] - sz;
	zone_size[0] = sz;
	zhole_size[1] = zhole_size[0];
	zhole_size[0] = 0;
}

#ifdef CONFIG_IXP425_LARGE_SDRAM
#define arch_adjust_zones(node, size, holes) \
	__arch_adjust_zones(node, size, holes)
#endif

#endif /* _ASM_ARCH_DMA_H */
