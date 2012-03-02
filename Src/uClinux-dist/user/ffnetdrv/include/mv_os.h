/* 
 * Copyright 2002, Marvell International Ltd.
 * 
 * THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 * NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 * OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 * DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 * THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESS, IMPLIED
 * OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
 */

/*
 * FILENAME:    $Workfile: mv_os.h $
 * REVISION:    $Revision: 4 $
 * LAST UPDATE: $Modtime: 12/22/02 11:07a $
 *
 * DESCRIPTION:
 *     This file is the OS Layer header, provided for OS independence.
 */

#ifndef MV_OS_H
#define MV_OS_H


/*************/
/* Constants */
/*************/

#define GT_OS_WAIT_FOREVER				0


/*************/
/* Datatypes */
/*************/

typedef enum {
	GT_OS_OK		= 0,
	GT_OS_FAIL		= 1, 
	GT_OS_TIMEOUT	= 2
} GT_OS_STATUS;


/*********************/
/* OS Specific Stuff */
/*********************/

#if defined(CISCO_IOS)

#include "mv_os_ios.h"

#elif defined(_WIN32_WCE)

#include "mv_os_wince.h"

#elif defined(_VXWORKS)

#include "mv_os_vx.h"

#elif defined(NETGX)

#include "mv_os_netgx.h"

#elif defined(WIN32)

#include "mv_os_win32.h"

#elif defined(LINUX)

#include "mv_os_linux.h"

#else /* No OS */

#include "mv_os_none.h"

#endif /* OS specific */


#ifdef HOST_LE /* HOST works in Little Endian mode */

#define gtOsWordHtoN(data)   GT_BYTESWAP4(data)
#define gtOsShortHtoN(data)  GT_BYTESWAP2(data)

#else   /* HOST_BE - HOST works in Little Endian mode */

#define gtOsWordHtoN(data)   (data)
#define gtOsShortHtoN(data)  (data)

#endif  /* HOST_LE || HOST_BE */

/*	Useful functions to deal with Physical memory using OS dependent inline
 *	functions defined in the file mv_os_???.h
 */

/*  Set physical memory with specified value. Use this function instead of
 *  memset when physical memory should be set.
 */
static INLINE void gtOsSetPhysMem(GT_U32 hpaddr, GT_U8 val, int size)
{
	int	i;

	for(i=0; i<size; i++)
	{
		gtOsIoMemWriteByte( (hpaddr+i), val);
	}
}

/*	Copy data from Physical memory. Use this fuction instead of memcpy,
 *  when data should be copied from Physical memory to Local (virtual) memory. 
 */
static INLINE void gtOsCopyFromPhysMem(GT_U8* localAddr, GT_U32 hpaddr, int size)
{
	int		i;
	GT_U8	byte;

	for(i=0; i<size; i++)
	{
		byte = gtOsIoMemReadByte(hpaddr+i);
		localAddr[i] = byte;
	}
}

/*	Copy data to Physical memory. Use this fuction instead of memcpy,
 *  when data should be copied from Local (virtual) memory to Physical memory. 
 */
static INLINE void gtOsCopyToPhysMem(GT_U32 hpaddr, const GT_U8* localAddr, int size)
{
	int		i;

	for(i=0; i<size; i++)
	{
		gtOsIoMemWriteByte( (hpaddr+i), localAddr[i]);
	}
}


/***********/
/* General */
/***********/

/* gtOsInit
 *
 * DESCRIPTION:
 *     Creates and initializes all the internal components of the OS Layer.
 *
 * RETURN VALUES:
 *     GT_OS_OK -- if succeeds.
 */
#ifndef gtOsInit
long gtOsInit(void);
#endif

/* gtOsFinish
 *
 * DESCRIPTION:
 *     Clears and deletes all the internal components of the OS Layer.
 *
 * RETURN VALUES:
 *     GT_OS_OK -- if succeeds.
 */
#ifndef gtOsFinish 
long gtOsFinish(void);
#endif

/* gtOsSleep
 *
 * DESCRIPTION:
 *     Sends the current task to sleep.
 *
 * INPUTS:
 *     mils -- number of miliseconds to sleep.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSleep
void	gtOsSleep(unsigned long mils);
#endif




/*********/
/* Misc. */
/*********/

/* gtOsGetCurrentTime
 *
 * DESCRIPTION:
 *     Returns current system's up-time.
 *
 * INPUTS:
 *     None.
 *
 * OUTPUTS:
 *     None.
 *
 * RETURN VALUES:
 *     current up-time in miliseconds.
 */
#ifndef gtOsGetCurrentTime
unsigned long gtOsGetCurrentTime(void);
#endif

/* Returns last error number */
#ifndef gtOsGetErrNo
unsigned long	gtOsGetErrNo(void);
#endif

/* Returns random 32-bit number. */
#ifndef gtOsRand
unsigned int	gtOsRand(void);
#endif

/*****************/
/* File Transfer */
/*****************/

/* gtOsOpenFile
 *
 * DESCRIPTION:
 *     Opens a file on a remote machine.
 *
 * INPUTS:
 *     machine  -- name or IP-address string of the file server.
 *     path     -- path on the machine to the directory of the file.
 *     filename -- name of file to be opened.
 *
 * OUTPUTS:
 *     None.
 *
 * RETURN VALUES:
 *     Handle to the opened file, or NULL on failure.
 */
#ifndef gtOsOpenFile
void* gtOsOpenFile(char* machine, char* path, char* filename);
#endif

/* gtOsCloseFile
 *
 * DESCRIPTION:
 *     Closes a file opened by gtOsOpenFile.
 *
 * INPUTS:
 *     hfile -- file handle.
 *
 * OUTPUTS:
 *     None.
 *
 * RETURN VALUES:
 *     None.
 */
#ifndef gtOsCloseFile
void gtOsCloseFile(void* hfile);
#endif

/* gtOsGetFileLine
 *
 * DESCRIPTION:
 *     Reads up to len bytes from the file into the buffer, terminated with zero.
 *
 * INPUTS:
 *     buf   -- buffer pointer.
 *     len   -- buffer length (in bytes).
 *     hfile -- file handle.
 *
 * RETURN VALUES:
 *     The given 'buf' pointer.
 */
#ifndef gtOsGetFileLine
char* gtOsGetFileLine(void* hfile, char* buf, int len);
#endif

/**************/
/* Semaphores */
/**************/

/* gtOsSemCreate
 *
 * DESCRIPTION:
 *     Creates a semaphore.
 *
 * INPUTS:
 *     name  -- semaphore name.
 *     init  -- init value of semaphore counter.
 *     count -- max counter value (must be positive).
 *
 * OUTPUTS:
 *     smid -- pointer to semaphore ID.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemCreate
long gtOsSemCreate(char *name,unsigned long init,unsigned long count,
					 unsigned long *smid);
#endif


/* gtOsSemDelete
 *
 * DESCRIPTION:
 *     Deletes a semaphore.
 *
 * INPUTS:
 *     smid -- semaphore ID.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemDelete
long gtOsSemDelete(unsigned long smid);
#endif

/* gtOsSemWait
 *
 * DESCRIPTION:
 *     Waits on a semaphore.
 *
 * INPUTS:
 *     smid     -- semaphore ID.
 *     time_out -- time out in miliseconds, or 0 to wait forever.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemWait
long gtOsSemWait(unsigned long smid, unsigned long time_out);
#endif

/* gtOsSemSignal
 *
 * DESCRIPTION:
 *     Signals a semaphore.
 *
 * INPUTS:
 *     smid -- semaphore ID.
 *
 * RETURN VALUES:
 *     GT_OS_OK if succeeds.
 */
#ifndef gtOsSemSignal
long gtOsSemSignal(unsigned long smid);
#endif

#endif MV_OS_H
