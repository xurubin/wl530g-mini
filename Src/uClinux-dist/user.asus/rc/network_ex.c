/*
 * Copyright 2004, ASUSTek Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>															
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <signal.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <nvparse.h>
#include <rc.h>
#include <bcmutils.h>

#ifdef REMOVE
int start_pppoe(void)
{

	int timeout = 5;
	char pppunit[] = "XXXXXXXXXXXX";

	/* Add optional arguments */
	for (arg = pppoe_argv; *arg; arg++);
	if (nvram_invmatch(strcat_r(prefix, "pppoe_service", tmp), "")) {
		*arg++ = "-s";
				*arg++ = nvram_safe_get(strcat_r(prefix, "pppoe_service", tmp));
			}
			if (nvram_invmatch(strcat_r(prefix, "pppoe_ac", tmp), "")) {
				*arg++ = "-a";
				*arg++ = nvram_safe_get(strcat_r(prefix, "pppoe_ac", tmp));
			}
			if (nvram_match(strcat_r(prefix, "pppoe_demand", tmp), "1") || 
			    nvram_match(strcat_r(prefix, "pppoe_keepalive", tmp), "1"))
				*arg++ = "-k";
			snprintf(pppunit, sizeof(pppunit), "%d", unit);
			*arg++ = "-U";
			*arg++ = pppunit;

			/* launch pppoe client daemon */


			/* ppp interface name is referenced from this point on */
			wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			
			/* Pretend that the WAN interface is up */
			if (nvram_match(strcat_r(prefix, "pppoe_demand", tmp), "1")) {
				/* Wait for pppx to be created */
				while (ifconfig(wan_ifname, IFUP, NULL, NULL) && timeout--)
					sleep(1);

				/* Retrieve IP info */
				if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
					continue;
				strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);

				/* Set temporary IP address */
				if (ioctl(s, SIOCGIFADDR, &ifr))
					perror(wan_ifname);
				nvram_set(strcat_r(prefix, "ipaddr", tmp), inet_ntoa(sin_addr(&ifr.ifr_addr)));
				nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.255");

				/* Set temporary P-t-P address */
				if (ioctl(s, SIOCGIFDSTADDR, &ifr))
					perror(wan_ifname);
				nvram_set(strcat_r(prefix, "gateway", tmp), inet_ntoa(sin_addr(&ifr.ifr_dstaddr)));

				close(s);

				/* 
				* Preset routes so that traffic can be sent to proper pppx even before 
				* the link is brought up.
				*/
				preset_wan_routes(wan_ifname);
}
#endif

int start_pppd(char *prefix)
{
	int ret;
	FILE *fp;
	char options[80];
	char *pptp_argv[] = { "pppd", "file", options, NULL};
 	char tmp[100];
	mode_t mask;
	pid_t pid;

#ifdef RPPPPOE	
	sprintf(options, "/tmp/ppp/options.wan%s", 
		nvram_safe_get(strcat_r(prefix, "unit", tmp)));
#else
	sprintf(options, "/tmp/ppp/options.WAN");
#endif
 
	mask = umask(066);
	
 	/* Generate options file */
	if (!(fp = fopen(options, "w"))) 
	{
       	        perror(options);
		umask(mask);
        	return -1;
 	}
 	
	umask(mask);

#ifdef NOMPPE
	fprintf(fp, "noauth\n");
#else
	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth refuse-eap\n");
#endif
	fprintf(fp, "user '%s'\n", 
       		nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp))); 
	fprintf(fp, "password '%s'\n", 
		nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp)));
       	       	
       	if (nvram_match(strcat_r(prefix, "proto", tmp), "pptp")) 
       	{
		fprintf(fp, "connect true\n");    
		fprintf(fp, "pty '/usr/sbin/pptp %s --nolaunchpppd'\n", 
   			nvram_invmatch("wan_heartbeat_x", "") ?
   			nvram_safe_get("wan_heartbeat_x") : 
   			nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp))); 
		fprintf(fp, "lock\n");
 	}
	else if (nvram_match(strcat_r(prefix, "proto", tmp), "pppoe")) 
	{
#ifdef RPPPPOE
		fprintf(fp, "plugin rp-pppoe.so nic-%s\n", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));

		fprintf(fp, "mru %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_mru", tmp)));
		fprintf(fp, "mtu %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp)));

		if (nvram_invmatch(strcat_r(prefix, "pppoe_service", tmp), ""))			{
			fprintf(fp, "rp_pppoe_service '%s'\n", 
			nvram_safe_get(strcat_r(prefix, "pppoe_service", tmp)));
		}	
		if (nvram_invmatch(strcat_r(prefix, "pppoe_ac", tmp), "")) {
			fprintf(fp, "rp_pppoe_ac '%s'\n", 
			nvram_safe_get(strcat_r(prefix, "pppoe_ac", tmp)));
		}
#else
		fprintf(fp, "plugin pppoe %s\n", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		fprintf(fp, "pppoe_debug 1\n");

		fprintf(fp, "mru %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_mru", tmp)));
		fprintf(fp, "mtu %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp)));

		if (nvram_invmatch(strcat_r(prefix, "pppoe_service", tmp), ""))			{
			fprintf(fp, "pppoe_srv_name '%s'\n", 
			nvram_safe_get(strcat_r(prefix, "pppoe_service", tmp)));
		}	
		if (nvram_invmatch(strcat_r(prefix, "pppoe_ac", tmp), "")) {
			fprintf(fp, "pppoe_ac_name '%s'\n", 
			nvram_safe_get(strcat_r(prefix, "pppoe_ac", tmp)));
		}
#endif
	}
	
	//fprintf(fp, "debug\n");

	if (atoi(nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp))) &&
	     nvram_match(strcat_r(prefix, "pppoe_demand", tmp), "1"))
	{ 
		fprintf(fp, "idle %s ", nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp)));
		if (nvram_invmatch(strcat_r(prefix, "pppoe_txonly_x", tmp), "0")) 
		{
			fprintf(fp, "tx_only ");
		}
		fprintf(fp, "demand\n");
	}
	
      	if (nvram_invmatch(strcat_r(prefix, "dnsenable_x", tmp), "0"))
		fprintf(fp, "usepeerdns\n");  
 
	fprintf(fp, "defaultroute\n");
       	fprintf(fp, "persist\n");
	fprintf(fp, "ipcp-accept-remote ipcp-accept-local noipdefault\n");
#ifdef RPPPPOE
	fprintf(fp, "ktune\n");
	fprintf(fp, "unit %s\n", 
		nvram_get(strcat_r(prefix, "unit", tmp)) ? : "0");
#endif
       	/* pppoe set these options automatically */
       	/* looks like pptp also likes them */
#ifdef RPPPPOE
    	fprintf(fp, "default-asyncmap nopcomp noaccomp\n");
	/* pppoe disables "vj bsdcomp deflate" automagically */
	/* ccp should still be enabled - mppe/mppc requires this */
       	fprintf(fp, "novj nobsdcomp nodeflate\n");       	
#else
#endif
	fprintf(fp, "holdoff 0\n");

       	/* echo failures */
 	fprintf(fp, "lcp-echo-interval 10\n");
	fprintf(fp, "lcp-echo-failure 6\n");	
       	fclose(fp);
	
	ret = _eval(pptp_argv, NULL, 0, &pid);
}

void start_pppoe_relay(char *wan_if)
{
	if (nvram_match("wan_pppoe_relay_x", "1"))
	{
		char *pppoerelay_argv[] = {"pppoe-relay", "-C", "br0", "-S", wan_if, "-F", NULL};
		int ret;
		pid_t pid;

		ret = _eval(pppoerelay_argv, NULL, 0, &pid);
	}
}

void setup_ethernet(char *wan_if)
{
	if (nvram_invmatch("wan_etherspeed_x", "auto"))
	{
		eval("et", "-i", wan_if, "speed", nvram_safe_get("wan_etherspeed_x"));	
	}	
} 
