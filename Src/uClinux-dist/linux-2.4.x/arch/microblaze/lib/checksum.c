/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		MIPS specific IP/TCP/UDP checksumming routines
 *
 * Authors:	Ralf Baechle, <ralf@waldorf-gmbh.de>
 *		Lots of code moved from tcp.c and ip.c; see those files
 *		for more names.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */
#include <net/checksum.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <asm/string.h>
#include <asm/uaccess.h>

/* Lifted from Xilinx library code ip.c. 
   No guarantee that it works, but at least it compiles */
static inline unsigned int do_csum(const unsigned char * buf, int len)
{
  
   unsigned int sum = 0;
   unsigned short w16 = 0;
   int i;

   for (i = 0; i < len; i = i + 2) {
      w16 = ((buf[i] << 8) & 0xFF00) + (buf[i+1] & 0x00FF);
      sum = sum + (unsigned int) w16;
   }
  
   while (sum >> 16) 
      sum = (sum & 0xFFFF) + (sum >> 16);
  
   return (unsigned short) (~sum);

}

/*
 *	This is a version of ip_compute_csum() optimized for IP headers,
 *	which always checksum on 4 octet boundaries.
 */
unsigned short ip_fast_csum(unsigned char * iph, unsigned int ihl)
{
	return ~do_csum(iph,ihl*4);
}

/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */
unsigned short ip_compute_csum(const unsigned char * buff, int len)
{
	return ~do_csum(buff,len);
}

/*
 * computes a partial checksum, e.g. for TCP/UDP fragments
 */
unsigned int csum_partial(const unsigned char *buff, int len, unsigned int sum)
{
	unsigned int result = do_csum(buff, len);

	/* add in old sum, and carry.. */
	result += sum;
	if(sum > result)
		result += 1;
	return result;
}

/*
 * copy while checksumming, otherwise like csum_partial
 */
unsigned int csum_partial_copy(const char *src, char *dst, 
                               int len, unsigned int sum)
{
	/*
	 * It's 2:30 am and I don't feel like doing it real ...
	 * This is lots slower than the real thing (tm)
	 */
	sum = csum_partial(src, len, sum);
	memcpy(dst, src, len);

	return sum;
}

/*
 * Copy from userspace and compute checksum.  If we catch an exception
 * then zero the rest of the buffer.
 */
unsigned int csum_partial_copy_from_user (const char *src, char *dst,
                                          int len, unsigned int sum,
                                          int *err_ptr)
{
	int missing;

	missing = copy_from_user(dst, src, len);
	if (missing) {
		memset(dst + len - missing, 0, missing);
		*err_ptr = -EFAULT;
	}
		
	return csum_partial(dst, len, sum);
}
