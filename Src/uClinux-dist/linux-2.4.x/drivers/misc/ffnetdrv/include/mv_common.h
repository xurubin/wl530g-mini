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
 * FILENAME:    $Workfile: mv_common.h $
 * REVISION:    $Revision: 1 $
 * LAST UPDATE: $Modtime: 28/11/02 10:27 $
 *
 * DESCRIPTION:
 *     This file defines general-purpose macros.
 */


#ifndef MV_COMMON_H
#define MV_COMMON_H


#include "mv_types.h"


#define GT_ALIGN_UP(num, aln)   (((num)&((aln)-1)) ? (((num)+(aln))&~((aln)-1)) : (num))
#define GT_ALIGN_DOWN(num, aln) ((num)&~((aln)-1))

#define GT_BYTE0(s) ((s)&0xff)
#define GT_BYTE1(s) (((s)>>8)&0xff)
#define GT_BYTE2(s) (((s)>>16)&0xff)
#define GT_BYTE3(s) (((s)>>24)&0xff)
#define GT_HWORD0(s) ((s)&0xffff)
#define GT_HWORD1(s) (((s)>>16)&0xffff)

#define GT_BYTESWAP2(s) ((GT_BYTE0(s)<<8)|GT_BYTE1(s))

#define GT_BYTESWAP4(s) ((GT_BYTE0(s)<<24)|(GT_BYTE1(s)<<16)|(GT_BYTE2(s)<<8)|GT_BYTE3(s))

#define GT_HWORDSWAP(s) ((GT_HWORD0(s)<<16)|GT_HWORD1(s))

#define GT_LOAD_32_BITS(pBuf)                   \
    (GT_U32)((((pBuf)[0]<<24)&0xff000000) |     \
             (((pBuf)[1]<<16)&0x00ff0000) |     \
             (((pBuf)[2]<<8 )&0x0000ff00) |     \
             (((pBuf)[3])    &0x000000ff))

#define GT_LOAD_32_BITS_WITH_OFFSET(pBuf, offset)   \
    (GT_U32)( ((pBuf)[0]<<(24+(offset))) |          \
              ((pBuf)[1]<<(16+(offset))) |          \
              ((pBuf)[2]<<(8+(offset)))  |          \
              ((pBuf)[3]<<((offset)))    |          \
              ((pBuf)[4]>>(8-(offset))) )

#define GT_STORE_32_BITS(pBuf, val32)       \
do{                                         \
    (pBuf)[0] = (GT_U8)((val32)>>24);       \
    (pBuf)[1] = (GT_U8)((val32)>>16);       \
    (pBuf)[2] = (GT_U8)((val32)>>8);        \
    (pBuf)[3] = (GT_U8)(val32);             \
} while (0);

#define GT_LOAD_16_BITS(pBuf)               \
    (GT_U16)( (((pBuf)[0]<<8)&0xff00) |     \
              (((pBuf)[1])   &0x00ff))


#define GT_STORE_16_BITS(pBuf, val16)       \
do{                                         \
    (pBuf)[0] = (GT_U8)((val16)>>8);        \
    (pBuf)[1] = (GT_U8)(val16);             \
} while (0);

/*---------------------------------------------------------------------------*/
/*           BIT FIELDS MANIPULATION MACROS                                  */
/*---------------------------------------------------------------------------*/
#define GT_BIT_MASK(x)       (1<<(x))     /* integer which its 'x' bit is set */

/*---------------------------------------------------------------------------*/
/* checks wheter bit 'x' in 'a' is set and than returns TRUE,                */
/* otherwise return FALSE.                                                   */
/*---------------------------------------------------------------------------*/
#define GT_CHKBIT(a, x)     (((a) & GT_BIT_MASK(x)) >> (x))     

/*---------------------------------------------------------------------------*/
/* Clear (reset) bit 'x' in integer 'a'                                      */
/*---------------------------------------------------------------------------*/
#define GT_CLRBIT(a, x)     ((a) &= ~(GT_BIT_MASK(x)))

/*---------------------------------------------------------------------------*/
/* SET bit 'x' in integer 'a'                                                */
/*---------------------------------------------------------------------------*/
#define GT_SETBIT(a, x)     ((a) |= GT_BIT_MASK(x))

/*---------------------------------------------------------------------------*/
/*   INVERT bit 'x' in integer 'a'.                                          */
/*---------------------------------------------------------------------------*/
#define GT_INVBIT(a, x)   ((a) = (a) ^ GT_BIT_MASK(x))

/*---------------------------------------------------------------------------*/
/* Get the min between 'a' or 'b'                                             */
/*---------------------------------------------------------------------------*/
#define GT_MIN(a,b)    (((a) < (b)) ? (a) : (b)) 

/*---------------------------------------------------------------------------*/
/* Get the max between 'a' or 'b'                                             */
/*---------------------------------------------------------------------------*/
#define GT_MAX(a,b)    (((a) < (b)) ? (b) : (a)) 


#endif /* MV_COMMON_H */
