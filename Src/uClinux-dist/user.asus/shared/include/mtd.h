/*
 * MTD utility functions
 *
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id: mtd.h,v 1.5 2003/02/19 02:13:01 mhuang Exp $
 */

#ifndef _mtd_h_
#define _mtd_h_

/*
 * Open an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @param	flags	open() flags
 * @return	return value of open()
 */
extern int mtd_open(const char *mtd, int flags);

/*
 * Erase an MTD device
 * @param	mtd	path to or partition name of MTD device
 * @return	0 on success and errno on failure
 */
extern int mtd_erase(const char *mtd);

/*
 * Write a file or a URL to an MTD device
 * @param	path	file to write or a URL
 * @param	mtd	path to or partition name of MTD device 
 * @return	0 on success and errno on failure
 */
extern int mtd_write(const char *path, const char *mtd);

#endif /* _mtd_h_ */
