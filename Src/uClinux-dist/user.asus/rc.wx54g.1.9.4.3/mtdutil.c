

/* Test */
/*
 * Copyright (c) 2d3D, Inc.
 * Written by Abraham vd Merwe <abraham@2d3d.co.za>
 * All rights reserved.
 *
 * $Id: mtdutil.c,v 1.3 2003/10/17 03:33:11 Cheni_Shen Exp $
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/mtd/mtd.h>

#define FBLOCKSIZE	4096

static unsigned long crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};


/*
 * MEMGETINFO
 */
static int getmeminfo (int fd,struct mtd_info_user *mtd)
{
   return (ioctl (fd,MEMGETINFO,mtd));
}

/*
 * MEMERASE
 */
static int memerase (int fd,struct erase_info_user *erase)
{
   return (ioctl (fd,MEMERASE,erase));
}

/*
 * MEMGETREGIONCOUNT
 * MEMGETREGIONINFO
 */
static int getregions (int fd,struct region_info_user *regions,int *n)
{
   int i,err;
   err = ioctl (fd,MEMGETREGIONCOUNT,n);
   if (err) return (err);
   for (i = 0; i < *n; i++)
	 {
		regions[i].regionindex = i;
		err = ioctl (fd,MEMGETREGIONINFO,&regions[i]);
		if (err) return (err);
	 }
   return (0);
}

int erase_flash (int fd,u_int32_t offset,u_int32_t bytes)
{
   int err;
   struct erase_info_user erase;
   erase.start = offset;
   erase.length = bytes;
   err = memerase (fd,&erase);
   if (err < 0)
	 {
		perror ("MEMERASE");
		return (1);
	 }
   fprintf (stderr,"Erased %d bytes from address 0x%.8x in flash\n",bytes,offset);
   return (0);
}

void printsize (u_int32_t x)
{
   int i;
   static const char *flags = "KMGT";
   printf ("%u ",x);
   for (i = 0; x >= 1024 && flags[i] != '\0'; i++) x /= 1024;
   i--;
   if (i >= 0) printf ("(%u%c)",x,flags[i]);
}

int flash_to_file_ex(int fd, u_int32_t offset, const char *filename)
{
   
   u_int8_t buf[FBLOCKSIZE];	
   int outfd,err;
   u_int32_t *size;
   int n;

   if (offset != lseek (fd,offset,SEEK_SET))
   {
	perror ("lseek()");
	goto err0;
   }
   
   outfd = creat (filename,O_WRONLY);
   
   if (outfd < 0)
   {
	perror ("creat()");
	goto err1;
   }
   
   size = (u_int32_t *)&buf[0];
   //crc = (u_int32_t *)&value[4];
      
   err = read(fd, buf, FBLOCKSIZE);
   err = write (outfd, &buf[4], *size);   	
   close (outfd);
   printf ("Copied %d bytes from address 0x%.8x in flash to %s\n", *size, offset, filename);
   return (0);   
err0:
err1:
   return(1);   
}

#define MIN(a, b) ((a<b)?a:b)

int file_to_flash (int fd, u_int32_t offset, u_int32_t len, const char *filename)
{
   u_int8_t buf[4096];
   FILE *fp;
   int err;
   u_int32_t count, n;

   if (offset != lseek (fd,offset,SEEK_SET))
   {
	perror ("lseek()");
	return (1);
   }
   
   if ((fp = fopen (filename,"r")) == NULL)
   {
	perror ("fopen()");
	return (1);
   }
   
   /*printf("Write Start ");  */
   
   n = len;
    
   while ( n>0 ) {
	count = fread(buf, 1, MIN(len, sizeof(buf)), fp);
	
	if (count==0) break; 
	
	n -= count;	
	
	err = write(fd, buf, count);
	/*printf(".");*/
	
	if (err < 0)
	{
		fprintf (stderr, __FUNCTION__ ": write, size %#x, n %#x\n", len, n);
		perror ("write()");
		goto err;
	}				
   }   
   /*printf("\n");*/
err:
   if (fp) fclose(fp);	 
   /*printf ("Copied %d bytes from %s to address 0x%.8x in flash\n",len,filename,offset);*/
   return (0);
}


void getTableEntry(unsigned char in, unsigned long *out1, unsigned char *out2)
{
	int i;
	unsigned char *ptr;

        for(i=0; i<256; i++)
	{
	    ptr = (unsigned char *)&crc32_table[i];

	    if (ptr[3]==in) 
	    {
	    	//printf("TE: %x %x\n", crc32_table[i], ptr[3]);
	        break;
	    }   
	}	

	*out1 = crc32_table[i];
	*out2 = i;
}

unsigned long crc32_reverse(unsigned long orig, unsigned long new)
{	
	unsigned char cstr[8];
	unsigned long *o, *n, *cur, r1;
	unsigned char r2;
	int i;

	o = (unsigned long *)&cstr[0];
	n = (unsigned long *)&cstr[4];

	*o = orig;
	*n = new;

	for(i=4;i>0;i--)
	{
		getTableEntry(cstr[i+3], &r1, &r2);
		cur = (unsigned long *)&cstr[i];
		*cur^=r1;
		cstr[i-1]^=r2;
	}
	cur = &cstr[0];	
	return(*cur);
}


#ifdef FLASH_RW
int kcrc(int fd)
{
   u_int32_t orig_crc;   
   u_int32_t header[3], datas[2];
   u_int32_t count;
      	              	        
   //lseek (fd, 0, SEEK_SET);
   //read(fd, header, 3*sizeof(u_int32_t));
   
   //printf("RECRC: %x %x %x\n", header[0], header[1], header[2]);
   
   // header[0] : magic
   // header[1] : length
   // header[2] : crc
   // header[3] : flash version
   
   // Get original crc
   lseek (fd, 3*sizeof(u_int32_t), SEEK_SET);
   
   orig_crc = 0xffffffff;
   
   count=3*sizeof(u_int32_t);
   
   while(count<1024*768)
   {   	
   	read(fd, datas, 2*sizeof(u_int32_t));
	orig_crc = crc32((u_int8_t *)&datas[0], 4, orig_crc);
		
	if (count<1024*768-4)
	{
	    orig_crc = crc32((u_int8_t *)&datas[1], 4, orig_crc);
	}	
	count+=2*sizeof(u_int32_t);
   }      
   
   printf("%d", orig_crc);
}

int rekcrc(int fd, u_int32_t orig_crc)
{
   u_int32_t rev_crc;   
   u_int32_t header[3], datas[2];
   u_int32_t new_crc, count;
   u_int32_t offset = 1024*768-4; // The last 4 bytes of kernel space     
      	              	        
   //lseek (fd, 0, SEEK_SET);
   //read(fd, header, 3*sizeof(u_int32_t));   
   //printf("RECRC: %x %x %x\n", header[0], header[1], header[2]);
   
   // header[0] : magic
   // header[1] : length
   // header[2] : crc
   // header[3] : flash version
  
   
   // Get new crc
   lseek (fd, 3*sizeof(u_int32_t), SEEK_SET);   
   new_crc = 0xffffffff;
                    
   count=3*sizeof(u_int32_t);   
   while(count<1024*768-4)
   {   	   	
   	read(fd, datas, 2*sizeof(u_int32_t));
	new_crc = crc32((u_int8_t *)&datas[0], 4, new_crc);
	new_crc = crc32((u_int8_t *)&datas[1], 4, new_crc);
	count+=2*sizeof(u_int32_t);
   }         
         
   // Caculate crc32 reverse
   rev_crc = crc32_reverse(new_crc, orig_crc);
   
   //printf("CRC: %x %x %x\n", orig_crc, new_crc, rev_crc);
   //printf("REV CRC: %x\n", crc32(&rev_crc, 4, new_crc));
      
   // Write crc32 reverse into the last 4 bytes of kernel space
   if (offset != lseek (fd, offset, SEEK_SET))
   {
	perror ("lseek()");
	return (1);
   }   
   
   write(fd, &rev_crc, sizeof(u_int32_t));
}
#endif

int file_to_flash_ex(int fd, u_int32_t offset, const char *filename)
{
   u_int8_t buf[FBLOCKSIZE];
   FILE *fp;
   int err;
   u_int32_t *filelen;
   u_int32_t i, count;
   
   /* Erase Block for File */  
   /* err = erase_flash (fd, offset, FBLOCKSIZE); */

   if (offset != lseek (fd, offset, SEEK_SET))
   {
	perror ("lseek()");
	return (1);
   }
         
   memset(buf, 0, FBLOCKSIZE);
   filelen = (u_int32_t *)&buf[0];
      
   if ((fp = fopen (filename,"r")) != NULL)
   {   
   	/* One 4k block as a file */
   	/* 4 byte file length     */
   	/* 4 byte check sum	  */
   	/* Others will be content */
              
   	/* Read file to buf */
   	*filelen = fread(&buf[4], 1, FBLOCKSIZE-4, fp);
      	 	   	
   	fclose(fp);
   }	
                
   err = write(fd, buf, FBLOCKSIZE);
	         
   //printf("Write to flash : %x %x\n", *filelen);
   
err:
   /*printf ("Copied %d bytes from %s to address 0x%.8x in flash\n",len,filename,offset);*/
   return (0);
}


int showinfo (int fd)
{
   int i,err,n;
   struct mtd_info_user mtd;
   static struct region_info_user region[1024];

   err = getmeminfo (fd,&mtd);
   if (err < 0)
	 {
		perror ("MEMGETINFO");
		return (1);
	 }

   err = getregions (fd,region,&n);
   if (err < 0)
	 {
		perror ("MEMGETREGIONCOUNT");
		return (1);
	 }

   printf ("mtd.type = ");
   switch (mtd.type)
	 {
	  case MTD_ABSENT:
		printf ("MTD_ABSENT");
		break;
	  case MTD_RAM:
		printf ("MTD_RAM");
		break;
	  case MTD_ROM:
		printf ("MTD_ROM");
		break;
	  case MTD_NORFLASH:
		printf ("MTD_NORFLASH");
		break;
	  case MTD_NANDFLASH:
		printf ("MTD_NANDFLASH");
		break;
	  case MTD_PEROM:
		printf ("MTD_PEROM");
		break;
	  case MTD_OTHER:
		printf ("MTD_OTHER");
		break;
	  case MTD_UNKNOWN:
		printf ("MTD_UNKNOWN");
		break;
	  default:
		printf ("(unknown type - new MTD API maybe?)");
	 }

   printf ("\nmtd.flags = ");
   if (mtd.flags == MTD_CAP_ROM)
	 printf ("MTD_CAP_ROM");
   else if (mtd.flags == MTD_CAP_RAM)
	 printf ("MTD_CAP_RAM");
   else if (mtd.flags == MTD_CAP_NORFLASH)
	 printf ("MTD_CAP_NORFLASH");
   else if (mtd.flags == MTD_CAP_NANDFLASH)
	 printf ("MTD_CAP_NANDFLASH");
   else if (mtd.flags == MTD_WRITEABLE)
	 printf ("MTD_WRITEABLE");
   else
	 {
		int i,first = 1;
		static struct
		  {
			 const char *name;
			 int value;
		  } flags[] =
		  {
			 { "MTD_CLEAR_BITS", MTD_CLEAR_BITS },
			 { "MTD_SET_BITS", MTD_SET_BITS },
			 { "MTD_ERASEABLE", MTD_ERASEABLE },
			 { "MTD_WRITEB_WRITEABLE", MTD_WRITEB_WRITEABLE },
			 { "MTD_VOLATILE", MTD_VOLATILE },
			 { "MTD_XIP", MTD_XIP },
			 { "MTD_OOB", MTD_OOB },
			 { "MTD_ECC", MTD_ECC },
			 { NULL, -1 }
		  };
		for (i = 0; flags[i].name != NULL; i++)
		  if (mtd.flags & flags[i].value)
			{
			   if (first)
				 {
					printf (flags[i].name);
					first = 0;
				 }
			   else printf (" | %s",flags[i].name);
			}
	 }

   printf ("\nmtd.size = ");
   printsize (mtd.size);

   printf ("\nmtd.erasesize = ");
   printsize (mtd.erasesize);

   printf ("\nmtd.oobblock = ");
   printsize (mtd.oobblock);

   printf ("\nmtd.oobsize = ");
   printsize (mtd.oobsize);

   printf ("\nmtd.ecctype = ");
   switch (mtd.ecctype)
	 {
	  case MTD_ECC_NONE:
		printf ("MTD_ECC_NONE");
		break;
	  case MTD_ECC_RS_DiskOnChip:
		printf ("MTD_ECC_RS_DiskOnChip");
		break;
	  case MTD_ECC_SW:
		printf ("MTD_ECC_SW");
		break;
	  default:
		printf ("(unknown ECC type - new MTD API maybe?)");
	 }

   printf ("\n"
		   "regions = %d\n"
		   "\n",
		   n);

   for (i = 0; i < n; i++)
	 {
		printf ("region[%d].offset = 0x%.8x\n"
				"region[%d].erasesize = ",
				i,region[i].offset,i);
		printsize (region[i].erasesize);
		printf ("\nregion[%d].numblocks = %d\n"
				"region[%d].regionindex = %d\n",
				i,region[i].numblocks,
				i,region[i].regionindex);
	 }
   return (0);
}

unsigned long getsize (int fd)
{
   int err;
   struct mtd_info_user mtd;   

   err = getmeminfo (fd,&mtd);
   if (err < 0)
   {
	perror ("MEMGETINFO");
	return (0);
   }

   return(mtd.size);
}

void showusage (const char *progname)
{
   fprintf (stderr,
			"usage: %s info <device>\n"
			"       %s read <device> <offset> <dest-filename>\n"
			"       %s write <device> <offset> <source-filename>\n"
			"       %s erase <device> <offset> <len>\n"
			"       %s upgrade <device> <image-filename>\n"
#ifdef FLASH_RW			
			"	%s kcrc <device>\n"
			"	%s rekcrc <device>\n"
#endif			
			,
			
			progname,
			progname,
			progname,
			progname,
			progname,
			progname,
			progname);
   exit (1);
}

#define OPT_INFO	1
#define OPT_READ	2
#define OPT_WRITE	3
#define OPT_ERASE	4
#define OPT_UPGRADE	5
#define OPT_RECRC	6
#define OPT_CRC		7

#ifdef ASUS
int mtdutil(int argc, char *argv[])
#else
int main (int argc,char *argv[])
#endif
{
   const char *progname;
   int err = 0,fd,option = OPT_INFO;
   int open_flag;
   unsigned long tsize;
   (progname = strrchr (argv[0],'/')) ? progname++ : (progname = argv[0]);

   /* parse command-line options */
   if (argc == 3 && !strcmp (argv[1],"info"))
	 option = OPT_INFO;
   else if (argc == 5 && !strcmp (argv[1],"read"))
	 option = OPT_READ;
   else if (argc == 5 && !strcmp (argv[1],"write"))
	 option = OPT_WRITE;
   else if (argc == 5 && !strcmp (argv[1],"erase"))
	 option = OPT_ERASE;
   else if (argc==4 && !strcmp (argv[1], "upgrade"))
         option = OPT_UPGRADE;
   else if (argc==4 && !strcmp (argv[1], "rekcrc"))
   	 option = OPT_RECRC;   
   else if (argc==3 && !strcmp (argv[1], "kcrc"))
   	 option = OPT_CRC;       	    
   else
	 showusage (progname);

   /* open device */
   open_flag = (option==OPT_INFO || option==OPT_READ) ? O_RDONLY : O_RDWR;
   
   if ((fd = open (argv[2],O_SYNC | open_flag)) < 0)
   {
		perror ("open()");
		exit (1);
	 }

   switch (option)
	 {
	  case OPT_INFO:
		showinfo (fd);
		break;
	  case OPT_READ:
		err = flash_to_file_ex(fd, strtol(argv[3],NULL,0), argv[4]);
		break;
	  case OPT_WRITE:
		err = file_to_flash_ex (fd, strtol(argv[3],NULL,0), argv[4]);
		break;
	  case OPT_ERASE:
		err = erase_flash (fd,strtol (argv[3],NULL,0),strtol (argv[4],NULL,0));
		break;
	  case OPT_UPGRADE:
	        tsize = getsize(fd);
	        /*printf("Write image(%d bytes) from %s to %s\n", tsize, argv[3], argv[2]);*/
	  	/*printf("Erase Flash\n");*/
	  	err = erase_flash (fd, 0, tsize);
	  	/*printf("Write Finish from %s\n", argv[3]);*/
	  	err = file_to_flash (fd, 0, tsize, argv[3]);
	  	/*printf("Upgrade Finish\n");*/
	  	/*system("reboot");*/
	  	break;		
#ifdef FLASH_RW	  	
	  case OPT_RECRC:
	  	rekcrc(fd, strtol(argv[3], NULL, 0));
	  	break;	
	  case OPT_CRC:
	  	kcrc(fd);
	  	break;		
#endif	  	
	 }

   /* close device */
   if (close (fd) < 0)
	 perror ("close()");

   exit (err);
}

/* CVS edit 2 */
