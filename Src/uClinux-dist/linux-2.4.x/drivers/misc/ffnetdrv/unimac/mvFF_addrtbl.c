/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* 
* FILENAME:    $Workfile: mvFF_addrtbl.c $ 
* REVISION:    $Revision: 3 $ 
* LAST UPDATE: $Modtime: 3/02/03 10:20p $ 
* 
* mvFF_addrtbl.c
*
* DESCRIPTION:
*	        Address Table driver for the Marvell ethernet unit.
*
* DEPENDENCIES:   
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/types.h>

#include <asm/arch/hardware.h>

#include "mv_unimac.h"
#include "mvFF_addrtbl.h"
#include "mvFF_eth.h"

/*
 * We have only one MAC port in FF 
 */
#define MAX_NUMBER_OF_ETHERNET_PORTS            1

/*
 * An address table entry 64bit representation 
 */
typedef struct addressTableEntryStruct {
	u32 hi;
	u32 lo;
} addrTblEntry;

/*
 * Per port hash mode
 */
static u32 addressTableHashMode[MAX_NUMBER_OF_ETHERNET_PORTS];

/*
 * Per port hash size
 */
static u32 addressTableHashSize[MAX_NUMBER_OF_ETHERNET_PORTS];

/*
 * Per port address table base
 */
static addrTblEntry *addressTableBase[MAX_NUMBER_OF_ETHERNET_PORTS];

/*
 * Hash length size options
 */
static u32 hashLength[2] = {
  (0x8000),		/* 32K entries */
  (0x8000 / 16),/*  2K entries */
};

/*
 * A hash function that acts exactly as the HW hash 
 */
static u32 hashTableFunction(u32 macH, u32 macL, u32 HashSize, u32 hash_mode);

/*
 * ----------------------------------------------------------------------------
 * This function will add an entry to the address table.
 * depends on the hash mode and hash size that was initialized.
 * Inputs
 * port - ETHERNET port number.
 * macH - the 2 most significant bytes of the MAC address.
 * macL - the 4 least significant bytes of the MAC address.
 * skip - if 1, skip this address.
 * rd   - the RD field in the address table.
 * Outputs
 * address table entry is added.
 * 1 if success.
 * 0 if table full
 */
static int
addAddressTableEntry(u32 port, u32 macH, u32 macL, u32 rd, u32 skip);


/*
 * ----------------------------------------------------------------------------
 *  Create an addressTable entry from MAC address info
 *  found in the specifed net_device struct
 *
 *  Input : pointer to ethernet interface network device structure
 *  Output : N/A
 */
void
mvFF_eth_update_mac_address(u8 *byte, u8 port)
{
  u32 macH;
  u32 macL;

  macH = byte[0];
  macH = (macH << 8) | byte[1];
  macL = byte[2];
  macL = (macL << 8) | byte[3];
  macL = (macL << 8) | byte[4];
  macL = (macL << 8) | byte[5];

  /*
   * addAddressTableEntry() will flush Dcache and sync
   */
  if (addAddressTableEntry(port, macH, macL, 1, 0) == 0)
	printk("Table is full.\n");
}


/*
 * clear the address table
 * port - ETHERNET port number.
 */
void
addressTableClear(u32 port)
{
  u32 addr, size;
  memset((void *) addressTableBase[port],
	 0, (hashLength[addressTableHashSize[port]] * MAC_ENTRY_SIZE));

  addr = ((u32)(addressTableBase[port]));
  size = (hashLength[addressTableHashSize[port]] * MAC_ENTRY_SIZE);
  flush_dcache_range( addr, ((u32)(addr)+(size)) );
  /*
  flush_dcache_addr_size((u32) (addressTableBase[port]),
			 (hashLength[addressTableHashSize[port]] * MAC_ENTRY_SIZE));
  */
}


/*
 * ----------------------------------------------------------------------------
 * This function will initialize the address table and will enableFiltering.
 * Inputs
 * hashMode         - hash mode 0 or hash mode 1.
 * hashSizeSelector - indicates number of hash table entries (0=0x8000,1=0x800)
 * hashDefaultMode  - 0 = discard addresses not found in the address table,
 *                    1 = pass addresses not found in the address table.
 * port - ETHERNET port number.
 * Outputs
 * address table is allocated and initialized.
 * Always returns 1
 */
int
initAddressTable(u32 port,
		 u32 hashMode, u32 hashSizeSelector, u32 hashDefaultMode)
{
	u32 addr;
	u32 bytes;
	ETHERNET_PCR portControlReg;

	addressTableHashMode[port] = hashMode;
	addressTableHashSize[port] = hashSizeSelector;

	/*
	 * Allocate memory for the address table, which must reside
	 * on an 8-byte boundary.
	 */
	bytes = MAC_ENTRY_SIZE * hashLength[hashSizeSelector];
	if (bytes & ~PAGE_MASK) {
		panic("initAddressTable: computed size isn't page-multiple.\n");
	}
	addr = uncachedPages(bytes >> PAGE_SHIFT);
	memset((void *) addr, 0, bytes);
	flush_dcache_addr_size(addr, bytes);
#ifdef CONFIG_ARCH_FIREFOX
	firefox_reg_write(FIREFOX_ENET_E0HTPR
		 + (ETHERNET_PORTS_DIFFERENCE_OFFSETS * port),
		 virt_to_phys((void *) addr));
#endif
#ifdef CONFIG_ARCH_LIBERTAS
	libertas_reg_write(LIBERTAS_ENET_E0HTPR
		 + (ETHERNET_PORTS_DIFFERENCE_OFFSETS * port),
		 virt_to_phys((void *) addr));
#endif
	invalidate_dcache_range(addr, addr + bytes);
	mb();
	addressTableBase[port] = (addrTblEntry *) addr;

	/*
	 * set hash {size,mode} and HDM in the PCR
	 */
	portControlReg = get_port_config(port);
	portControlReg &= ~((1 << HASH_DEFAULT_MODE) | (1 << HASH_MODE)
			    | (1 << HASH_SIZE));

	portControlReg |= ((hashDefaultMode << HASH_DEFAULT_MODE)
			   | (hashMode << HASH_MODE)
			   | (hashSizeSelector << HASH_SIZE));

	set_port_config(portControlReg, port);
	enableFiltering(port);

	return (1);
}

/*
 * ----------------------------------------------------------------------------
 * This function will calculate the hash function of the address.
 * depends on the hash mode and hash size.
 * Inputs
 * macH             - the 2 most significant bytes of the MAC address.
 * macL             - the 4 least significant bytes of the MAC address.
 * hashMode         - hash mode 0 or hash mode 1.
 * hashSizeSelector - indicates number of hash table entries (0=0x8000,1=0x800)
 * Outputs
 * return the calculated entry.
 */
static u32
hashTableFunction(u32 macH, u32 macL, u32 HashSize, u32 hash_mode)
{
	u32 hashResult;
	u32 addrH;
	u32 addrL;
	u32 addr0;
	u32 addr1;
	u32 addr2;
	u32 addr3;
	u32 addrHSwapped;
	u32 addrLSwapped;

	addrH = NIBBLE_SWAPPING_16_BIT(macH);
	addrL = NIBBLE_SWAPPING_32_BIT(macL);

	addrHSwapped = FLIP_4_BITS(addrH & 0xf)
	    + ((FLIP_4_BITS((addrH >> 4) & 0xf)) << 4)
	    + ((FLIP_4_BITS((addrH >> 8) & 0xf)) << 8)
	    + ((FLIP_4_BITS((addrH >> 12) & 0xf)) << 12);

	addrLSwapped = FLIP_4_BITS(addrL & 0xf)
	    + ((FLIP_4_BITS((addrL >> 4) & 0xf)) << 4)
	    + ((FLIP_4_BITS((addrL >> 8) & 0xf)) << 8)
	    + ((FLIP_4_BITS((addrL >> 12) & 0xf)) << 12)
	    + ((FLIP_4_BITS((addrL >> 16) & 0xf)) << 16)
	    + ((FLIP_4_BITS((addrL >> 20) & 0xf)) << 20)
	    + ((FLIP_4_BITS((addrL >> 24) & 0xf)) << 24)
	    + ((FLIP_4_BITS((addrL >> 28) & 0xf)) << 28);

	addrH = addrHSwapped;
	addrL = addrLSwapped;

	if (hash_mode == 0) {
		addr0 = (addrL >> 2) & 0x03f;
		addr1 = (addrL & 0x003) | ((addrL >> 8) & 0x7f) << 2;
		addr2 = (addrL >> 15) & 0x1ff;
		addr3 = ((addrL >> 24) & 0x0ff) | ((addrH & 1) << 8);
	} else {
		addr0 = FLIP_6_BITS(addrL & 0x03f);
		addr1 = FLIP_9_BITS(((addrL >> 6) & 0x1ff));
		addr2 = FLIP_9_BITS((addrL >> 15) & 0x1ff);
		addr3 =
		    FLIP_9_BITS((((addrL >> 24) & 0x0ff) |
				 ((addrH & 0x1) << 8)));
	}

	hashResult = (addr0 << 9) | (addr1 ^ addr2 ^ addr3);

	if (HashSize == _8K_TABLE) {
		hashResult = hashResult & 0xffff;
	} else {
		hashResult = hashResult & 0x07ff;
	}

	return (hashResult);
}

/*
 * ----------------------------------------------------------------------------
 * This function will add an entry to the address table.
 * depends on the hash mode and hash size that was initialized.
 * Inputs
 * port - ETHERNET port number.
 * macH - the 2 most significant bytes of the MAC address.
 * macL - the 4 least significant bytes of the MAC address.
 * skip - if 1, skip this address.
 * rd   - the RD field in the address table.
 * Outputs
 * address table entry is added.
 * 1 if success.
 * 0 if table full
 */
static int
addAddressTableEntry(u32 port, u32 macH, u32 macL, u32 rd, u32 skip)
{
	addrTblEntry *entry;
	u32 newHi;
	u32 newLo;
	u32 i;

	newLo = (((macH >> 4) & 0xf) << 15)
	    | (((macH >> 0) & 0xf) << 11)
	    | (((macH >> 12) & 0xf) << 7)
	    | (((macH >> 8) & 0xf) << 3)
	    | (((macL >> 20) & 0x1) << 31)
	    | (((macL >> 16) & 0xf) << 27)
	    | (((macL >> 28) & 0xf) << 23)
	    | (((macL >> 24) & 0xf) << 19)
	    | (skip << SKIP_BIT) | (rd << 2) | VALID;

	newHi = (((macL >> 4) & 0xf) << 15)
	    | (((macL >> 0) & 0xf) << 11)
	    | (((macL >> 12) & 0xf) << 7)
	    | (((macL >> 8) & 0xf) << 3)
	    | (((macL >> 21) & 0x7) << 0);

	/*
	 * Pick the appropriate table, start scanning for free/reusable
	 * entries at the index obtained by hashing the specified MAC address
	 */
	entry = addressTableBase[port];
	entry += hashTableFunction(macH, macL, addressTableHashSize[port],
				   addressTableHashMode[port]);
	for (i = 0; i < HOP_NUMBER; i++, entry++) {
		if (!(entry->lo & VALID) /*|| (entry->lo & SKIP) */ ) {
			break;
		} else {	/* if same address put in same position */
			if (((entry->lo & 0xfffffff8) == (newLo & 0xfffffff8))
			    && (entry->hi == newHi)) {
				break;
			}
		}
	}

	if (i == HOP_NUMBER) {
		printk("add address Table Entry: table section is full\n");
		return (0);
	}

	/*
	 * Update the selected entry
	 */
#if 1
	entry->hi = newLo;
	entry->lo = newHi;
#else
	entry->hi = newHi;
	entry->lo = newLo;
#endif
	flush_dcache_addr_size((u32) entry, MAC_ENTRY_SIZE);
	return (1);
}
