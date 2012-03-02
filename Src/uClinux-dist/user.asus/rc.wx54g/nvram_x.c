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
#include <syslog.h>
#include <bcmnvram.h>
#include <sys/mman.h>
#include <net/if.h>
#ifdef REMOVE_WL600
#include <sbsdram.h>
#include <bcmendian.h>
#include <flash.h>
#include <flashutl.h>
#endif

#define MAX_LINE_SIZE 512
#define MAX_FILE_NAME 64
#define MAX_HEADER_SIZE 32
#define MAX_SID	17

#define IMAGE_KERNEL_LEN 786432

#define IMAGE_HEADER 	"HDR0"
#define PROFILE_HEADER 	"HDR1"

char *sidMap[MAX_SID];
char *workingDir="/etc/linuxigd";

/*
 * NOTE : The mutex must be initialized in the OS previous to the point at
 *           which multiple entry points to the nvram code are enabled 
 *
 */
#define MAX_NVRAM_SIZE 4096
#define EPI_VERSION_STR "2.4.20"

char envHeader[8];
char envBuf[1024*32];
int envBufOff;

#ifdef REMOVE
int nvram_init(void *b)
{
	envBufOff=0;
}

int nvram_set(const char *name, const char *value)
{
#ifndef REMOVE_NVRAM
	FILE *fp;
	char buf[1024];
	
	fp=fopen("/dev/nvram","a+");
	if (fp==NULL) printf("fd error\n");
	else
	{
		if (value==NULL || value[0]=='\0')
	 		sprintf(buf, "%s", name);
		else sprintf(buf, "%s=%s", name, value);
		//printf("set: %s %d\n", buf, strlen(buf));
		fwrite(buf, 1, strlen(buf)+1, fp);
		fclose(fp);
	}
#else
	int fd;

	sprintf(envBuf + envBufOff, "%s=%s", name, value);
	envBufOff+=strlen(name)+strlen(value)+1;
	*(envBuf+envBufOff) = 0x0;
	envBufOff++;

	fd = open("/dev/nvram", 0);
	if (fd<0) printf("fd error\n");
	{
		write(fd, envBuf+envBufOff, strlen(name)+strlen(value)+1);
	}	
	close(fd);
#endif
}

char *nvram_get(const char *value)
{
#ifndef NVRAM_REMOVE
	int fd;
	char buf[64], *nvram_buf;
	unsigned long *off;

	fd = open("/dev/nvram", 0);

	if (fd<0) printf("fd error\n");
	{
		nvram_buf = mmap(0, 0x8000, PROT_READ, MAP_SHARED, fd, 0);
		strcpy(buf, value);
		read(fd, buf, sizeof(buf));
		close(fd);
		if (strcmp(buf, value))
		{
			off = (unsigned long *)buf;
			//printf("get: %s %x\n", nvram_buf+*(off), strlen(nvram_buf+*(off)));
			if (*(nvram_buf+*(off))=='\0')
				return NULL;
			else return(nvram_buf+*(off));
		}
	}
	return NULL;
#else
	char *ptr;
	char cmpstr[64];

	sprintf(cmpstr, "%s=", value);

	if (envBufOff>0)
	{
		ptr = envBuf;
		for(ptr=envBuf;ptr!=NULL&&(ptr-envBuf)<envBufOff;ptr+=strlen(ptr)+1)
		{
			if (strncmp(ptr, cmpstr, strlen(cmpstr))==0)
			{
				//printf("nvram_get: %s=%s\n", value, ptr);
				return(ptr+strlen(cmpstr));
			}
		}
	}
	return NULL;
#endif
}

int nvram_commit(void)
{
#ifndef REMOVE_NVRAM
	int fd;
	struct ifreq ifr;

	fd=open("/dev/nvram", 0);
	if (fd>=0)
	{
		ioctl(fd, 0x48534c46, &ifr);
		close(fd);
	}	
#else
#ifndef ASUS
	unsigned long count, *filelen;

	memcpy(envHeader, PROFILE_HEADER, 4);
	filelen = (unsigned long *)(envHeader+4);
	*filelen = envBufOff;
	mtd_write_raw(envHeader, envBufOff+sizeof(envHeader), MTD_DEV_NVRAM);
#else	
   	FILE *fp;
   	char header[8];
   	unsigned long count, *filelen;
	char *mtdutil_argv[]={"mtdutil","upgrade","/dev/mtd2","/etc/linuxigd/flash",NULL};
	int fd;

   	fp = fopen("/etc/linuxigd/flash", "w");
	strncpy(header, PROFILE_HEADER, 4);
	filelen = (unsigned long *)(header + 4);
	*filelen = envBufOff;

	printf("Write : %d to flash\n", envBufOff);
    	   
   	if (fp!=NULL)
   	{   		
   		count = fwrite(header, 1, 8, fp);
		count = fwrite(envBuf, 1, *filelen, fp);	   
   		fclose(fp);	
	}

	/* write to flash as well */
	mtdutil(4, mtdutil_argv);


	fd = open("/dev/nvram", 0);
	if (fd<0) printf("fd error");
	{
		ioctl(fd, 0x48534c46, NULL);
	}	
	close(fd);
#endif
#endif
	return 0;
}
#endif
#ifndef ASUS
/* Remove CR/LF/Space/'"' in the end of string 
 * 
 */
 
char *strtrim(char *str)
{
   int i;
         
   if (*str == '"')
     str++;
     
   i=strlen(str)-1;  
   	
   while(i>=0)
   {   
      if (*(str+i)==13 || *(str+i)==10 || *(str+i)==' ')
      {
   	*(str+i)=0x0; 
      }
      else if ( *(str+i)=='"')
      {
	*(str+i)=0x0;
 	break;	
      }      
      else break;
      i--;
   }   
   return (str);
}
#endif


void nvram_read_sid(char *envBuf)
{
   char *p, *v;
   int i, sid=0;
      
   for(i=0; i<MAX_SID; i++)
      sidMap[sid] = NULL;
   
   p  = envBuf;
         
   
   while(*p!=0x0)
   {
   	/*printf("Read SID: %s\n", p);*/
   	
   	if (strncmp(p, "sid_", 4)==0)
   	{
   	    v = strchr(p, '=');	
   	    v++;   	    
   	    sid = 0;
   	    
   	    if (p[4]>='0'&&p[4]<='9')
   	       sid = p[4]-'0';   	       
   	    if (p[5]>='0'&&p[5]<='9')
   	       sid = sid*10 + p[5]-'0';   
   	       
   	    if (sid<MAX_SID)      	       
   	       sidMap[sid] = v;  
   	       
   	    /*printf("Sid %d: %s\n", sid, v);*/
   	}   	
   	p = p + strlen(p) + 1;
   }	
}

void nvram_read_file(char *file, int sid)
{
   FILE *fl;
   char filename[MAX_FILE_NAME];
   char buf[MAX_LINE_SIZE];
   char item[MAX_LINE_SIZE];
   unsigned char *v, *sp;

   sprintf(filename, "%s/%s", workingDir, file);
   
   if ((fl=fopen(filename, "r+"))==NULL) 
   {
      /*printf("Open fail: %s\n", filename);*/
      return;
   }     
        
   while(fgets(buf, MAX_LINE_SIZE, fl)!=NULL)
   {    	         
      v = strchr(buf, '=');
      if (v != NULL && ((sp = strchr(buf, ' ')) == NULL || (sp > v))) 
      {
            /* change the "name=val" string to "set name val" */
            *v++ = '\0';
#ifdef REMOVE_WL600            
            tail = strchr(v, ';');            
            if (tail != NULL)
               *tail='\0';                     
#endif      
            if (*v=='"')
               v++;
               
            if (sid!=-1)
            {        
                sprintf(item, "%d_%s", sid, buf);                          
                nvram_set(item, strtrim(v));
                /*printf("Set: %s %s\n", item, v);*/
            }
            else
            {
                nvram_set(buf, strtrim(v));
                /*printf("Set: %s %s\n", buf, v);*/
            }   
      }     
   }     
   fclose(fl);				
}

void nvram_write_files(char *envBuf, char *sid)
{
   FILE *fl;
   char filename[MAX_FILE_NAME];
   char buf[MAX_LINE_SIZE];
   char header[MAX_HEADER_SIZE];
   char name[MAX_HEADER_SIZE];
   char *p, *v;
   int i;
   
   for(i=0; i<MAX_SID; i++)
   {
	//printf("SID: %d %s\n", i, sidMap[i]);

   	if (sidMap[i]!=NULL && (sid==NULL || strcmp(sid, sidMap[i])==0))
   	{   	   
   	     sprintf(header, "%d_", i);	 
   	     sprintf(filename, "%s/%s", workingDir, sidMap[i]);  
   
   	     if ((fl=fopen(filename, "w+"))==NULL) return;
   	 
   	     //printf("Open: %s\n", filename);
   	     
   	     p = envBuf;       	           
   
   	     while(*p!=0x0)
   	     {         	     	        	     	
       		if (strncmp(p, header, strlen(header))==0)	
       		{       		   
       		   v = strchr(p, '=');		
       		   strncpy(name, p+strlen(header), v-p-strlen(header));
       		   name[v-p-strlen(header)] = 0x0;       		   		       		   
       		   v++;
       		   sprintf(buf, "%s=\"%s\"\n", name, v);       		   
       		   fputs(buf, fl);                	       		          		         		         		   
       		}   
       		p = p + strlen(p) + 1;
   	     }
   	     
   	     if (fl!=NULL) fclose(fl);        	       	       	    	
   	}
   }
  
}

void nvram_write_to_files(char *envBuf, char *sid, unsigned long maximum, char *filename)
{
   FILE *fl;
   char buf[MAX_LINE_SIZE];
   char header[MAX_HEADER_SIZE];
   char name[MAX_HEADER_SIZE];
   char *p, *v;
   int i;
   unsigned long count, filelen;
   
   if ((fl=fopen(filename, "w+"))==NULL) return;
      
#ifdef REMOVE_WL600 
   for(i=0; i<MAX_SID; i++)
   {
   	if (sidMap[i]!=NULL && (sid==NULL || strcmp(sid, sidMap[i])==0))
   	{   	   
   	     sprintf(buf, "sid_%d=%s\0", i, sidMap[i]);
   	     fwrite(buf, 1, strlen(buf)+1, fl);
   	     //sprintf(filename, "%s/%s", workingDir, sidMap[i]);
   	     //if ((fl=fopen(filename, "w+"))==NULL) return;
   	     
   	     p = envBuf;       	           
   	     sprintf(header, "%d_", i);
   
   	     while(*p!=0x0)
   	     {       	      	     	
       		if (strncmp(p, header, strlen(header))==0)	
       		{       		   
       		   v = strchr(p, '=');		
       		   strncpy(name, p+strlen(header), v-p-strlen(header));
       		   name[v-p-strlen(header)] = 0x0;       		   		       		   
       		   v++;
       		   sprintf(buf, "%d_%s=%s\0", i, name, v);
       		   fwrite(buf, 1, strlen(buf) + 1, fl);
       		      	       		          		         		         		   
       		}   
       		p = p + strlen(p) + 1;
   	     }
   	     
   	     //if (fl!=NULL) fclose(fl);        	       	       	    	
   	}
   }
#endif   
   
   if (fl!=NULL) 
   {   	   
        p = envBuf; 
        count = 0;
   	
   	while(*p!=0x0)
   	{   
   	  count = count + strlen(p) + 1;
       	  p = p + strlen(p) + 1;
   	}   	
   	
   	filelen = count + (1024 - count%1024);
   	   	
   	fwrite(PROFILE_HEADER, 1, 4, fl);
   	fwrite(&filelen, 1, 4, fl);
   	fwrite(envBuf, 1, count, fl);
   	for(i=count;i<filelen;i++) fwrite(p, 1, 1, fl);   	
   	fclose(fl);
   }	
  
}

void nvram_read_files(char *sid)
{   
   int i;
   
   for(i=0; i<MAX_SID; i++)
   {
   	if (sidMap[i]!=NULL && (sid==NULL || strcmp(sid, sidMap[i])==0))
   	{
   	     nvram_read_file(sidMap[i], i);
   	}	
   }
}

#define MIN(a, b) ((a<b)?a:b)

void mtd_dump_image(char *dev, unsigned long offset, unsigned long len)
{
   FILE *fdev=NULL;
   unsigned long count;
   char buf[4096];
   int i;
     
   if ((fdev = fopen(dev, "rb"))==NULL) goto err;        
   /*printf("Image open ok!\n");*/
   if (fseek(fdev, offset, SEEK_SET)!=0) goto err;
   /*printf("Image seek ok!\n");*/   
      
   printf("Dump: ");   
   
   while (len>0) 
   {
	count = fread(buf, 1, MIN(len, sizeof(buf)), fdev);
	
	if (count==0) break; 
	
	len -= count;
	for(i=0; i<count; i++)
	   printf("%2x ", (int )buf[i]);	   
   }   
   printf("\n");
   
err:   
   if (fdev) fclose(fdev);
}

void mtd_dump_ver(char *type)
{   
   FILE *fdev=NULL;
   char buf[1];
   unsigned long *imagelen;
   char dataPtr[4]; 
   char verPtr[64];
   char productid[13];
   int i;
     
   if ((fdev = fopen(MTD_DEV_KERNEL, "rb"))==NULL) return;
   /*printf("Image open ok!\n");*/
   if (fseek(fdev, 4, SEEK_SET)!=0) goto err;
   /*printf("Image seek ok!\n");*/   
             
   fread(dataPtr, 1, 4, fdev);
	        
   imagelen = dataPtr;
   //printf("Image len: %x %x %x %x\n" ,dataPtr[0], dataPtr[1], dataPtr[2], dataPtr[3]);
   
   if (fseek(fdev, *imagelen-64, SEEK_SET)!=0) goto err;
   fread(verPtr, 1, 64, fdev);
   
   if (strcmp(type, "hw")==0)
   {
      i=0; 
      while(verPtr[i]!=0)
      {	
         printf("%d.%d ",verPtr[16+i], verPtr[17+i]); 
         i = i+2;
      }   
   }   
   else if (strcmp(type, "fw")==0)
   {
      printf("%d.%d.%d.%d",verPtr[0], verPtr[1], verPtr[2], verPtr[3]); 
   }   
   else if (strcmp(type, "product")==0)
   {
      strncpy(productid, verPtr+4, 12);
      productid[12]=0;	
      printf("%s",productid); 
   }      
err:   
   fclose(fdev);
}


void nvram_getall_from_file(char *buf, int maximum, char *file)
{
   FILE *fp;
   char header[8];
   unsigned long count, *filelen;
      
   fp = fopen(file, "r+");
    	   
   if (fp!=NULL)
   {   
   	count = fread(header, 1, 8, fp);
   	if (count>=8 && strncmp(header, PROFILE_HEADER, 4)==0)
   	{  
	   filelen = header + 4;
   	   count = fread(buf, 1, *filelen, fp);	
	   envBufOff = *filelen;
   	}   
   	fclose(fp);	
   }
}


int nvram_getall_from_flash(char *buf)
{
   FILE *fp;
   char header[8];
   unsigned long count, *filelen;
   int ret=1;
   
#ifndef ASUS   
   fp = fopen("/etc/linuxigd/flash","r+");	
#else
   fp = fopen(MTD_DEV_NVRAM, "r+");
#endif
    	   
   if (fp!=NULL)
   {   
   	count = fread(header, 1, 8, fp);
   	if (count>=8 && strncmp(header, PROFILE_HEADER, 4)==0)
   	{  
	   filelen = header + 4;
   	   count = fread(buf, 1, *filelen, fp);
	   envBufOff = *filelen;
	   ret = 0;
   	}   	
   	fclose(fp);	
   }
   return(ret);
}


void nvram_getall_default(char *buf)
{
   FILE *fl;
   char filename[MAX_FILE_NAME];
   char buf[MAX_LINE_SIZE];
   char item[MAX_LINE_SIZE];
   unsigned char *v, *sp;

   sprintf(filename, "%s/flash.default", workingDir);
   
   if ((fl=fopen(filename, "r+"))==NULL) 
   {
      printf("Open fail: %s\n", filename);
      return;
   }     
        
   while(fgets(buf, MAX_LINE_SIZE, fl)!=NULL)
   {    	         
      v = strchr(buf, '=');
      if (v != NULL && ((sp = strchr(buf, ' ')) == NULL || (sp > v))) 
      {
#ifdef REMOVE
{
	FILE *fp;

	fp = fopen("/dev/nvram", "a+");
	if (fp==NULL) printf("fd error");
	else
	{
		strtrim(buf);
		fwrite(buf, 1, strlen(buf)+1, fp);
		fclose(fp);
	}	
}
#endif

            /* change the "name=val" string to "set name val" */
            *v++ = '\0';
	    
            if (*v=='"')
               v++;
               
            if (nvram_get(buf)==NULL)
	    {
		nvram_set(buf, strtrim(v));
	    }	
      }     
   }     
   fclose(fl);				
}

void nvram_read_default(char *file)
{
   FILE *fl;
   char filename[MAX_FILE_NAME];
   char buf[MAX_LINE_SIZE];
   char item[MAX_LINE_SIZE];
   unsigned char *v, *sp;

   sprintf(filename, "%s/%s", workingDir, file);
   
   if ((fl=fopen(filename, "r+"))==NULL) 
   {
      /*printf("Open fail: %s\n", filename);*/
      return;
   }     
        
   while(fgets(buf, MAX_LINE_SIZE, fl)!=NULL)
   {    	         
      v = strchr(buf, '=');
      if (v != NULL && ((sp = strchr(buf, ' ')) == NULL || (sp > v))) 
      {
            /* change the "name=val" string to "set name val" */
            *v++ = '\0';
	    
            if (*v=='"')
               v++;
               
            if (nvram_get(buf)==NULL)
	    {
		dprintf("Lost in previous version!!!\r\n");	    
		nvram_set(buf, strtrim(v));
	    }	
      }     
   }     
   fclose(fl);				
}

// Write parameters from flash to files and keep one copy in envBuf
void nvram_x_get(char *sid)
{
	printf("nvram_x_get1:%s\n", NULL);
	nvram_init(NULL);
	nvram_getall_from_flash(envBuf);
	printf("nvram_x_get2:%d\n", envBufOff);
	nvram_getall_default(envBuf);
	printf("nvram_x_get3:%d\n", envBufOff);	
    	nvram_read_sid(envBuf);
	printf("nvram_x_get4:%d\n", envBufOff);	
    	nvram_write_files(envBuf, sid);
}

void nvram_x_set(char *sid)
{
	printf("nvram_x_set1:%s %d\n", sid, envBufOff);	
	nvram_init(NULL);
	printf("nvram_x_set2:%d\n", envBufOff);	
	nvram_getall_default(envBuf);	
	printf("nvram_x_set3:%d\n", envBufOff);	
    	nvram_read_sid(envBuf);
	printf("nvram_x_set4:%d\n", envBufOff);	
	nvram_init(NULL);
	printf("nvram_x_set5:%d\n", envBufOff);	
    	nvram_read_files(sid);
	printf("nvram_x_set6:%d\n", envBufOff);	
    	nvram_commit(); 
}

void nvram_x_fget(char *file)
{
	// suppport read from flash to file
	printf("nvram_x_fget1:%d\n", envBufOff);	
	nvram_init(NULL);
	printf("nvram_x_fget2:%d\n", envBufOff);	
	nvram_getall_default(envBuf);	
	printf("nvram_x_fget3:%d\n", envBufOff);	
    	nvram_read_sid(envBuf);
	printf("nvram_x_fget4:%d\n", envBufOff);	
	nvram_init(NULL);
	printf("nvram_x_fget5:%d\n", envBufOff);	
    	nvram_read_files(NULL);
	printf("nvram_x_fget6:%d\n", envBufOff);
    	nvram_commit(); 
	printf("nvram_x_fget7:%d\n", envBufOff);
    	nvram_write_to_files(envBuf, NULL, sizeof(envBuf), file);
}

void nvram_x_fset(char *file)
{
	// support write to all once only
	// from file to flash
	nvram_init(NULL);
        nvram_getall_from_file(envBuf, sizeof(envBuf), file);
	nvram_getall_default(envBuf);	
    	nvram_commit();    
}

#ifndef ASUS
int
main(int argc, char **argv)
{
    int restore_default;
    char *s;
        	
    if (argc==1)
    {
    	printf(" nvram [command] [service] 						\n");
    	printf(" [command]								\n");
    	printf("   get 				: get all variables from flash		\n");
    	printf("   get [service] 		: get variables of service from flash   \n");
    	printf("   set 				: get all variables to flash		\n");
    	printf("   set [service] 		: set variables of service to flash	\n");    
        printf("   fget [file]			: save all current setting into a file  \n");
    	printf("   fset [file]			: restore all setting from a file to flash \n");
    	printf("   default		        : set defalut variables to flash	\n");     	
    	printf("   dump [dev] [offset] [len]    : dump flash content 			\n"); 	
    	printf("   verinfo [type]     		: dump version information		\n"); 			
    	return 0;
    }	
    
  
    if (strcmp(argv[1], "dump") == 0)
    {
    	if (argc==5)
    	{      	    	  	    	   
    	    mtd_dump_image(argv[2], atol(argv[3]), atol(argv[4]));
    	}    	
	else 
    	{
    	    printf("   dump [dev] [offset] [len]    : dump flash content 			\n"); 		
    	}
    	return 0;
    }
    else if (strcmp(argv[1], "verinfo") == 0)
    {
    	if (argc>=3)
    	{      	     	    
    	    s = argv[3];
    	    
    	    if (argc>3 && 
    	         strncmp(s, "WL600",5)!=0 &&
    	         strncmp(s, "WL500",5)!=0 &&
    	         strncmp(s, "WL300",5)!=0 &&
		 strncmp(s, "WL520",5)!=0) 
    	    {
    	    	printf("0.0");    	    
    	    }	  
    	    else if (strcmp(argv[2],"pidstr")==0)
    	    {    	       	    	    	    	    	    	    	    	
    	    	s[5]=0;	
    	    	printf("%s", s);	    	    
    	    }  	    		
    	    else if (strcmp(argv[2],"blstr")==0)
    	    {    	       	    	    	    	
    	    	s+=6;
    	    	s[2]=0;
    	    	s[5]=0;	
    	    	printf("%d.%d", atoi(s), atoi(s+3));	    	    
    	    }
    	    else if (strcmp(argv[2],"hwstr")==0)
    	    {    	    	
    	    	s+=12;
    	    	s[2]=0;
    	    	s[5]=0;	
    	    	printf("%d.%d", atoi(s), atoi(s+3));    	    
    	    }	   	  	    	       	   
    	    else mtd_dump_ver(argv[2]);
    	}    	
	else 
    	{
    	    printf("   verinfo [type]     	     : dump version information\n"); 		
    	}
    	return 0;
    }
           

    nvram_init(NULL);

    if (strcmp(argv[1], "get") == 0)
    {
	restore_default = nvram_getall_from_flash(envBuf);
	nvram_getall_default(envBuf);		
    	nvram_read_sid(envBuf);
    
    	if (argc==2) /* Get all variables */
    	{    	    
    	    nvram_write_files(envBuf, NULL);   
    	    //system("/web/script/bcm_set check");
	}
	else if (argc==3)
	{
	    nvram_write_files(envBuf, argv[2]);	
	}	
    }
    else if (strcmp(argv[1], "set") == 0)
    {
	nvram_getall_default(envBuf);		
    	nvram_read_sid(envBuf);
	nvram_init(NULL);

    	if (argc==2)
    	{
    	   nvram_read_files(NULL);	
    	   nvram_commit();    
    	   /* For broadcom compatible only */
    	   //printf("Run compatible!\n");
    	   //system("/web/script/bcm_set");
	}
	else if (argc==3)
    	{
    	   nvram_read_files(argv[2]);
    	   nvram_commit();
    	}   
    }  
    else if (strcmp(argv[1], "fget") == 0)
    {
    	/* 1. Save all setting to nvram */
    	/* 2. Read from nvram again 	*/
    	/* 3. Write to file 		*/
    	if (argc>=3) /* Get all variables */
    	{    	    
    	    //nvram_read_files(NULL);
    	    //nvram_commit();
    	    nvram_write_to_files(envBuf, NULL, sizeof(envBuf), argv[2]);
    	    //system("/web/script/bcm_set check");
	}
    }   
    else if (strcmp(argv[1], "fset") == 0)
    {          	
        nvram_getall_from_file(envBuf, sizeof(envBuf), argv[2]);

    	if (argc>=3)
    	{
    	   nvram_write_files(envBuf, NULL);
    	   nvram_read_files(NULL);
    	   nvram_commit();    
    	   /* For broadcom compatible only */
    	   //printf("Run compatible!\n");
    	   //system("/web/script/bcm_set");
	}	
    }         
    else if (strcmp(argv[1], "default") == 0)
    {
	nvram_getall_default(envBuf);		
    	nvram_commit();
    }
    return 0;
}
#endif
