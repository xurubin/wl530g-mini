/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * This module creates an array of name, value pairs
 * and supports updating the nvram space. 
 *
 * This module requires the following support routines
 *
 *      malloc, free, strcmp, strncmp, strcpy, strtol, strchr, printf and sprintf
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_STRING 12
#define MAX_VER 4

typedef struct {
	unsigned char major;
	unsigned char minor; 
} version_t; 

typedef struct {
	version_t kernel;
	version_t fs;
	char	  productid[MAX_STRING];
	version_t hw[MAX_VER*2];
	char 	  reserve[32];
} TAIL;


int
main(int argc, char **argv)
{
    FILE *fp;
    TAIL tail;	
    char name[MAX_STRING+1];
	int i;
        	
    if (argc<=4)
    {
    	printf(" addver [fs file] [kernel ver] [fs ver] [product id] [compatible hardware list]\n");    			
    	return 0;
    }	
    
    fp = fopen(argv[1], "rb+");
    
    if (fp==NULL)
       printf("Open file fail!\n");
       
    memset(&tail, 0, sizeof(tail));

    sscanf(argv[2], "%d.%d", &tail.kernel.major, &tail.kernel.minor);
    sscanf(argv[3], "%d.%d", &tail.fs.major, &tail.fs.minor);   
    strncpy(tail.productid, "            ", MAX_STRING);    
    strncpy(tail.productid, argv[4], (MAX_STRING>strlen(argv[4]))?strlen(argv[4]):MAX_STRING);
    strncpy(name, tail.productid, MAX_STRING);
    name[MAX_STRING+1] = 0;
       
    printf("Append Version Information to file ...\n");
    printf("   Kernel Version      : %d.%d\n", tail.kernel.major, tail.kernel.minor);
    printf("   File system Version : %d.%d\n", tail.fs.major, tail.fs.minor);
    printf("   Product ID          : %s\n",    name);
    printf("   Hardware compatible list:\n");
    
    if (argc>5)
    {
        for(i=0; i< argc-5; i++)
        {	
	      sscanf(argv[5+i], "%d.%d", &tail.hw[i].major, &tail.hw[i].minor);
	      printf("     %d: %d.%d\n", i+1, tail.hw[i].major, tail.hw[i].minor);	   
	   }   
    }    
       
    fseek(fp, 0, SEEK_END);   
    fwrite(&tail, 1, sizeof(TAIL), fp);
    fclose(fp);
            
    return 0;
}
