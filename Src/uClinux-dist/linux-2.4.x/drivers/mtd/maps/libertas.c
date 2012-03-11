/*--rx201_wbr204-*/
/*
 *	libertas.c - mapper for Marvell Libertas AP-32 board.
 *
 * Copyright (C) 2003 - 2005 Marvell Co.
 *
 * This code is GPL
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <linux/fs.h>

#include <asm/io.h>


#define NAME         "Marvell Libertas AP-32"
#define WINDOW_ADDR  0xFFF00000
#define WINDOW_SIZE  0x00100000
#define BUSWIDTH     2
#define SUCCESS      0


static struct mtd_info *get_mtd_named(char *name);


/* 
 * MAP DRIVER STUFF
 */

struct map_info libertas_map = {
	name: NAME,
	size: WINDOW_SIZE,
	buswidth: BUSWIDTH,
	phys: WINDOW_ADDR
};


/*
 * MTD 'PARTITIONING' STUFF 
 */
 
static struct mtd_partition libertas_partitions[] = {
/*
	{
		name: "Libertas AP-32 compressed kernel",
#ifdef PROTECTION
		size:   0x000A0000,
		offset: 0x00000000,
		mask_flags: MTD_WRITEABLE
#else
		size:   0x000A0000,
		offset: 0x00000000
#endif

	},
	{
		name: "Libertas AP-32 cramfs root file system",
#ifdef PROTECTION
		size:   0x002E0000,
		offset: 0x000A0000,
		mask_flags: MTD_WRITEABLE
#else
		size:   0x00040000,
		offset: 0x000A0000
#endif
	},
*/
	{
		name: "Libertas AP-32 compressed kernel",
		size:   0x00068000,
		offset: 0x00008000
	},
	{
		name: "Libertas AP-32 squashfs root file system",
		size:   0x00080000,
		offset: 0x00070000
	},
	{
		name: "Libertas AP-32 config storage",
		size:   0x00004000,
		offset: 0x00004000,
	},
	{
		name: "Libertas AP-32 xz decompressor",
		size:   0x00004000,
		offset: 0x00000000
	},
	{
		name: "Libertas AP-32 master bootloader",
		size:   0x00010000,
		offset: 0x000F0000,
	}
};


#define PARTITION_NUMBER  (sizeof(libertas_partitions)/sizeof(struct mtd_partition))

/* 
 * This is the master MTD device for which all the others are just
 * auto-relocating aliases.
 */
 
static struct mtd_info *mymtd;

int __init init_libertas(void)
{
	struct mtd_info *mtd;
	int             ret_val;
	
	printk(KERN_NOTICE "Marvell Libertas AP-32 flash mapping: %x at %x\n", WINDOW_SIZE, WINDOW_ADDR);

	libertas_map.virt = (unsigned long) ioremap(WINDOW_ADDR, WINDOW_SIZE);

	if (!libertas_map.virt) {
		printk("%s: failed to ioremap 0x%x\n", NAME, WINDOW_ADDR);
		return -EIO;
	}

	simple_map_init(&libertas_map);

	mymtd = do_map_probe("cfi_probe", &libertas_map);

	if (!mymtd) 
		return -ENXIO;

	mymtd->owner = THIS_MODULE;

	ret_val = add_mtd_partitions(mymtd, libertas_partitions, PARTITION_NUMBER);

	if (ret_val != SUCCESS)
		return ret_val;

	mtd = get_mtd_named("Libertas AP-32 squashfs root file system");
	
	if (mtd) {
		ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, mtd->index);
		put_mtd_device(mtd);
		ret_val = SUCCESS;
	} else {
		printk("Can not find root partition and assign ROOT_DEV\n");
		ret_val = -ENODEV;
	}

	return ret_val;
}

static void __exit cleanup_libertas(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
	}
}


/*
 * Find the MTD device with the given name
 */

static struct mtd_info *get_mtd_named(char *name)
{
	int i;
	struct mtd_info *mtd;

	for (i = 0; i < MAX_MTD_DEVICES; i++) {
		mtd = get_mtd_device(NULL, i);
		if (mtd) {
			if (strcmp(mtd->name, name) == 0)
				return(mtd);
			put_mtd_device(mtd);
		}
	}
	
	return(NULL);
}


module_init(init_libertas);
module_exit(cleanup_libertas);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marvell Co.");
MODULE_DESCRIPTION("MTD map driver for Marvell Libertas AP-32 board");
