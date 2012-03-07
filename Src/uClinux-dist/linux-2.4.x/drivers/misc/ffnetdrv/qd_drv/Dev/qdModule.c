/*******************************************************************************
 *                Copyright 2001, Marvell International Ltd.
 * This code contains confidential information of Marvell semiconductor, inc.
 * no rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party.
 * Marvell reserves the right at its sole discretion to request that this code
 * be immediately returned to Marvell. This code is provided "as is".
 * Marvell makes no warranties, express, implied or otherwise, regarding its
 * accuracy, completeness or performance.
 ********************************************************************************
 * qdModule.c
 *
 * DESCRIPTION:
 *		Defines the entry point for the QD module
 *
 * DEPENDENCIES:   Platform.
 *
 * FILE REVISION NUMBER:
 *
 *******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include "qdModule.h"
#include <mvFF_netconf.h>

/*
 * #define IOCTL_DBG
 *
 */


/************************************************************************/
/* file io API                                                          */
/*                                                                      */
/* The io API is a proc-fs implementation of ioctl                      */
/************************************************************************/
static struct proc_dir_entry *evb_resource_dump;

/*
 * The lengh of the result buffer
 */
DWORD evb_resource_dump_result_len;

/*
 * A 1024 bytes, (in dwords because of alignment) of the result buffer.  
 */
DWORD dwevb_resource_dump_result[1024/sizeof(DWORD)];

/*
 * The result buffer pointer
 */
static char *evb_resource_dump_result = (char*)dwevb_resource_dump_result ;

/********************************************************************
 * evb_resource_dump_write -
 *
 * When written to the /proc/resource_dump file this function is called
 *
 * Inputs: file / data are not used. Buffer and count are the pointer
 *         and length of the input string
 * Returns: Read from GT register
 * Outputs: count
 *********************************************************************/
static int evb_resource_dump_write (struct file *file, const char *buffer, unsigned long count, void *data) 
{
  bool rc;
  unsigned int lenIn = 0 , code;

  evb_resource_dump_result_len = 0;
  memset(evb_resource_dump_result, 0, sizeof(dwevb_resource_dump_result));

  code = *((unsigned int*)buffer);
  if(!code)
    return count;
  lenIn = *((unsigned int*)(buffer + sizeof(code) ));
#ifdef IOCTL_DBG
  printk("Got IOCTL for code=%x, len=%d count = %d\n", code, lenIn, count);
#endif /* IOCTL_DBG */
  rc = UNM_IOControl( 0, code, (char*)(buffer + sizeof(code) + sizeof(lenIn) ), lenIn, 
		      evb_resource_dump_result,    
		      sizeof(dwevb_resource_dump_result) , 
		      &evb_resource_dump_result_len 
		      );
  if(rc == false) {
    evb_resource_dump_result_len = 0xFFFFFFFF;
    printk("Error for IOCTL code %x\n", code);
  }
#ifdef IOCTL_DBG
  else {
    int i;
    printk("Success for IOCTL code %x outlen = %d\n", code, evb_resource_dump_result_len);
    for(i=0;i<evb_resource_dump_result_len; i++) {
      printk("%x ",evb_resource_dump_result[i]);
    }
    printk("\n"); 
  }
#endif /* IOCTL_DBG */
  return count;
}

/********************************************************************
 * evb_resource_dump_read -
 *
 * When read from the /proc/resource_dump file this function is called
 *
 * Inputs: buffer_location and buffer_length and zero are not used.
 *         buffer is the pointer where to post the result
 * Returns: N/A
 * Outputs: length of string posted
 *********************************************************************/
static int evb_resource_dump_read (char *buffer, char **buffer_location, off_t offset, 
				   int buffer_length, int *zero, void *ptr) 
{
  if (offset > 0)
    return 0;
  /* first four bytes are the len of the out buffer */
  memcpy(buffer, (char*)&evb_resource_dump_result_len, sizeof(evb_resource_dump_result_len) );

  /* in offset of 4 bytes the out buffer begins */
  memcpy((buffer + sizeof (evb_resource_dump_result_len )), 
	 evb_resource_dump_result, 
	 sizeof(dwevb_resource_dump_result)
	 );
  
  return ( evb_resource_dump_result_len + sizeof(evb_resource_dump_result_len));
}



/************************************************************************/
/* module API                                                           */
/************************************************************************/
/*
 * The qd start actually strarts the qd operation.
 * It calls the qdInit() to the the actual work (see qdInit.c).
 */
int qdModuleStart(void)
{
  GT_STATUS status;

  if( (status=qdInit()) != GT_OK) {
    DBG_INFO(("\n qdModuleInit: Cannot start the QD switch!\n"));
    return -1;
  }
	
  DBG_INFO(("\n qdModuleInit is done!\n\n"));	
  return 0;
}

/*
 * At the entry point we init only the IOCTL hook.
 * An init IOCTL call will triger the real initialization of
 * the QD, with the start function above
 */
int qdEntryPoint(void)
{

  /* start_regdump_memdump -
   *
   * Register the /proc/rgcfgio file at the /proc filesystem
   */
  evb_resource_dump = create_proc_entry (RG_IO_FILENAME , 0666 , &proc_root);
  if(!evb_resource_dump)
    panic("Can't allocate UNM FILE-IO device\n");
  evb_resource_dump->read_proc = evb_resource_dump_read;
  evb_resource_dump->write_proc = evb_resource_dump_write;
  evb_resource_dump->nlink = 1;


  DBG_INFO(("\n qdEntryPoint is done!\n\n"));	
  return 0;
}

/*
 * Exit point of the module.
 */
void qdExitPoint(void)
{
  printk("QD Switch driver exited!\n");
}
