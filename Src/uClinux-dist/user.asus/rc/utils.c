#include<stdio.h>

#ifdef REMOVE
#ifndef ASUS
int nvram_get(char *n)
{
}

int nvram_set(char *n, char *v)
{
}


int nvram_commit(void)
{
}

#endif
int nvram_getall(char *b)
{
}

int nvram_unset(char *n)
{
}
#endif
 
unsigned char*
ether_etoa(char *ea, char *buf)
{
	sprintf(buf,"%x:%x:%x:%x:%x:%x",
 		ea[0]&0xff, ea[1]&0xff, ea[2]&0xff, ea[3]&0xff, ea[4]&0xff, ea[5]&0xff);
 	return (buf);
}

unsigned char*
ether_ntoa(char *ea, char *buf)
{
	sprintf(buf,"%x:%x:%x:%x:%x:%x",
 		ea[0]&0xff, ea[1]&0xff, ea[2]&0xff, ea[3]&0xff, ea[4]&0xff, ea[5]&0xff);
 	return (buf);
}
 
/* parse a xx:xx:xx:xx:xx:xx format ethernet address */
void
ether_atoe(char *p, char *ea)
{
 	int i;
 
 	for (i = 0; i < 6; i++) {
 		ea[i] = (char) strtoul(p, &p, 16);
 		if (*p == '\0')	/* just bail on error */
 			break;
 		p++;
 	}
}


void gen_log_file(void)
{
	nvram_set_f("general.log", mtd_dump_ver("product"));
	nvram_set_f("general.log", mtd_dump_ver("fw"));
}

