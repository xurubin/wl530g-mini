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
* FILENAME:    $Workfile: mvFF_addrtbl.h $ 
* REVISION:    $Revision: 3 $ 
* LAST UPDATE: $Modtime: 3/02/03 10:20p $ 
* 
* mvFF_addrtbl.h
*
* DESCRIPTION:
*	        Address Table driver header file for the Marvell ethernet unit.
*
* DEPENDENCIES:   
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#ifndef _MVFF_ADDRTBL_H_
#define _MVFF_ADDRTBL_H_


/*
 * ----------------------------------------------------------------------------
 * addressTable.h - this file has all the declarations of the address table
 */

#define _8K_TABLE                           0
#define ADDRESS_TABLE_ALIGNMENT             8
#define HASH_DEFAULT_MODE                   14
#define HASH_MODE                           13
#define HASH_SIZE                           12
#define HOP_NUMBER                          12
#define MAC_ADDRESS_STRING_SIZE             12
#define MAC_ENTRY_SIZE                      sizeof(addrTblEntry)
#define MAX_NUMBER_OF_ADDRESSES_TO_STORE    1000
#define PROMISCUOUS_MODE                    0
#define SKIP                                1<<1
#define SKIP_BIT                            1
#define VALID                               1



/*
 * ----------------------------------------------------------------------------
 * potential sign-extension bugs lurk here...
 */
#define NIBBLE_SWAPPING_32_BIT(X) ( (((X) & 0xf0f0f0f0) >> 4) \
                                  | (((X) & 0x0f0f0f0f) << 4) )

#define NIBBLE_SWAPPING_16_BIT(X) ( (((X) & 0x0000f0f0) >> 4) \
                                  | (((X) & 0x00000f0f) << 4) )

#define FLIP_4_BITS(X)  ( (((X) & 0x01) << 3) | (((X) & 0x002) << 1) \
                        | (((X) & 0x04) >> 1) | (((X) & 0x008) >> 3) )

#define FLIP_6_BITS(X)  ( (((X) & 0x01) << 5) | (((X) & 0x020) >> 5) \
                        | (((X) & 0x02) << 3) | (((X) & 0x010) >> 3) \
                        | (((X) & 0x04) << 1) | (((X) & 0x008) >> 1) )

#define FLIP_9_BITS(X)  ( (((X) & 0x01) << 8) | (((X) & 0x100) >> 8) \
                        | (((X) & 0x02) << 6) | (((X) & 0x080) >> 6) \
                        | (((X) & 0x04) << 4) | (((X) & 0x040) >> 4) \
         | ((X) & 0x10) | (((X) & 0x08) << 2) | (((X) & 0x020) >> 2) )

/*
 * V: value we're operating on
 * O: offset of rightmost bit in field
 * W: width of field to shift
 * S: distance to shift left
 */
#define MASK( fieldWidth )                            ((1 << (fieldWidth)) - 1)
#define leftShiftedBitfield( V,O,W,S)        (((V) & (MASK(W) << (O)))  << (S))
#define rightShiftedBitfield(V,O,W,S)  (((u32)((V) & (MASK(W) << (O)))) >> (S))

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
int initAddressTable(u32 port, u32 hashMode, u32 hashSize, u32 hashDefaultMode);


/*
 * ----------------------------------------------------------------------------
 *  Create an addressTable entry from MAC address info
 *  found in the specifed net_device struct
 *
 *  Input : pointer to ethernet interface network device structure
 *  Output : N/A
 */
void mvFF_eth_update_mac_address(u8 *byte, u8 port);

/*
 * clear the address table
 * port - ETHERNET port number.
 */
void addressTableClear(u32 port);


#endif /* _MVFF_ADDRTBL_H_ */
