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
 * FILENAME:    $Workfile: mv_debug.h $
 * REVISION:    $Revision: 3 $
 * LAST UPDATE: $Modtime: 3/02/03 10:20p $
 *
 * DESCRIPTION:
 *     This file defines few types and functions useful for debug purposes.
 */
#ifndef MV_DEBUG_H
#define MV_DEBUG_H


#include "mv_types.h"


/*	time measurement structure used to check how much time pass between
 *  two points
 */
typedef struct {
    char            name[20];	/* name of the entry */
    unsigned long   begin;		/* time measured on begin point */
    unsigned long   end;		/* time measured on end point */
    unsigned long   total;		/* Accumulated time */
    unsigned long   left;		/* The rest measurement actions */
    unsigned long   count;		/* Maximum measurement actions */
    unsigned long   min;		/* Minimum time from begin to end */
    unsigned long   max;		/* Maximum time from begin to end */
} MV_DEBUG_TIMES;

/* MACRO to print Debug messages */
#ifdef MV_DEBUG
#	define MV_DEBUG_PRINT	gtOsPrintf
#else
#       define MV_DEBUG_PRINT
#endif


/****** Error Recording ******/

/* Dump memory in specific format: 
 * address: X1X1X1X1 X2X2X2X2 ... X8X8X8X8 
 */
void      mvDebugMemoryDump(void* addr, int size);

/**** There are three functions deals with MV_DEBUG_TIMES structure ****/

/* Reset MV_DEBUG_TIMES entry */
void    mvDebugResetTimeEntry(MV_DEBUG_TIMES* pTimeEntry, int count, char* name);

/* Update MV_DEBUG_TIMES entry */
void    mvDebugUpdateTimeEntry(MV_DEBUG_TIMES* pTimeEntry);

/* Print out MV_DEBUG_TIMES entry */
void    mvDebugPrintTimeEntry(MV_DEBUG_TIMES* pTimeEntry, GT_BOOL isTitle);


/******** General ***********/

/* Change value of mvDebugPrint global variable */
void      mvDebugPrintEnable(GT_BOOL isEnable);


#endif /* MV_DEBUG_H */
