/*
	Layer 2 Tunneling Protocol Control file
	Written by Yen@asus Apl. 14, 2005
*/

#include<stdio.h>

void main()
{
	FILE *fp;
	char *ip_addr="192.168.123.31";
	fp=fopen("/var/run/l2tp-control", "w");
//	printf("%s\n", ip_addr);
	fprintf(fp,"t %s\n", ip_addr);
	fclose(fp);
}

