/*
 * include/asm-microblaze/cache.h -- Cache operations
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 */

#ifndef __MICROBLAZE_CACHE_H__
#define __MICROBLAZE_CACHE_H__

/* All cache operations are machine-dependent.  */
#include <asm/machdep.h>

#ifndef L1_CACHE_BYTES
/* This processor has no cache, so just choose an arbitrary value.  */
#define L1_CACHE_BYTES		16
#endif

/* Define MSR enable bits for instruction and data caches */
#define ICACHE_MSR_BIT (1 << 5)
#define DCACHE_MSR_BIT (1 << 7)

#define __enable_icache()						\
	__asm__ __volatile__ ("						\
				mfs	r12, rmsr;			\
				ori	r12, r12, %0;			\
				mts	rmsr, r12"			\
				: 					\
				: "i" (ICACHE_MSR_BIT)			\
				: "memory", "r12")

#define __disable_icache()						\
	__asm__ __volatile__ ("						\
				mfs	r12, rmsr;			\
				andi	r12, r12, ~%0;			\
				mts	rmsr, r12"			\
				: 					\
				: "i" (ICACHE_MSR_BIT)			\
				: "memory", "r12")

 
#define __enable_dcache()						\
	__asm__ __volatile__ ("						\
				mfs	r12, rmsr;			\
				ori	r12, r12, %0;			\
				mts	rmsr, r12"			\
				: 					\
				: "i" (DCACHE_MSR_BIT)			\
				: "memory", "r12")

#define __disable_dcache()						\
	__asm__ __volatile__ ("						\
				mfs	r12, rmsr;			\
				andi	r12, r12, ~%0;			\
				mts	rmsr, r12"			\
				: 					\
				: "i" (DCACHE_MSR_BIT)			\
				: "memory", "r12")

#endif /* __MICROBLAZE_CACHE_H__ */
