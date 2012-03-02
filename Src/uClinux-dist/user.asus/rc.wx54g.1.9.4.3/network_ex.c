/*
 * Network services
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
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

//Add by SJ_Yen 2005/04/21

int start_l2tp(char *prefix)
{
	int ret;
	FILE *fp;
	char username[32],passwd[32];
	char *l2tp_argv[] = {"/bin/l2tpd","-D","-c","/tmp/l2tpd.conf", NULL};
	char tmp[100];
	pid_t pid;
/*	
	strcpy(username, nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp)));
	strcpy(passwd, nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp)));
*/
	strcpy(username, nvram_safe_get("wan_pppoe_username"));
	strcpy(passwd, nvram_safe_get("wan_pppoe_passwd"));

	/*Generate config file*/
        if (!(fp = fopen("/tmp/l2tpd.conf", "w"))) {
                perror("/tmp/l2tpd.conf");
                return -1;
        }
	
	fprintf(fp, "[global]\n");
	fprintf(fp, "port = 1701\n");
	fprintf(fp, "auth file = /tmp/l2tp/l2tp-secrets\n");
	fprintf(fp, "[lac L2TP]\n");
	fprintf(fp, "redial = yes\n");
	fprintf(fp, "redial timeout = 15\n");
	fprintf(fp, "max redials = 5\n");
        fprintf(fp, "hidden bit = no\n");
        fprintf(fp, "length bit = yes\n");
        fprintf(fp, "refuse authentication = no\n");
        fprintf(fp, "require authentication = yes\n");
        fprintf(fp, "ppp debug = yes\n");
        fprintf(fp, "pppoptfile = /tmp/l2tp/options.l2tpd\n");
        fprintf(fp, "server = t %s\n", nvram_safe_get("wan_heartbeat_x"));
        
	fclose(fp);
        chmod("/tmp/l2tp/options.l2tpd", 0600);
	

	/*Generate options file*/
	if (!(fp = fopen("/tmp/l2tp/options.l2tpd", "w"))) {
		perror("/tmp/l2tp/options.l2tpd");
		return -1;
	}

	fprintf(fp,"ipap-accept-local\n");
	fprintf(fp,"ipap-accept-remote\n");
	fprintf(fp,"noccp\n");
	fprintf(fp,"auth\n");
	fprintf(fp,"remote_name %s\n", username);
	fprintf(fp,"crtscts\n");
	fprintf(fp,"idle 1800\n");
	fprintf(fp,"mtu 1410\n");
	fprintf(fp,"mru 1410\n");
	fprintf(fp,"defaultroute\n");
	fprintf(fp,"debug\n");
	fprintf(fp,"lock\n");
	fprintf(fp,"proxyarp\n");
	fprintf(fp,"connect-relay 5000\n");

	fclose(fp);
	chmod("/tmp/l2tp/options.l2tpd", 0600);

	/*Generate Authenticate files*/

	/* Generate pap-secrets file */
	if (!(fp = fopen("/tmp/ppp/pap-secrets", "w"))) {
		perror("/tmp/ppp/pap-secrets");
		return -1;
	}
	fprintf(fp, "\"%s\" * \"%s\" *\n",
		username,
		passwd);
	fclose(fp);
	chmod("/tmp/ppp/pap-secrets", 0600);

	/* Generate L2TP secrets file */
	if (!(fp = fopen("/tmp/l2tp/l2tp-secrets", "w"))) {
		perror("/tmp/l2tp/l2tp-secrets");
		return -1;
	}
	fprintf(fp, "\"%s\" * \"%s\" *\n",
		username,
		passwd);
	fclose(fp);
	chmod("/tmp/l2tp/l2tp-secrets", 0600);


	/*Run L2TPD*/
	eval("hostname",username);
	ret = _eval(l2tp_argv, NULL, 0, &pid);
	sleep(3);
	if (!(fp=fopen("/tmp/l2tp/l2tp-control", "w"))){
		perror("/tmp/l2tp/l2tp-control");
		return -1;
	}
	fprintf(fp,"s");
	fclose(fp);
	eval("cp", "/tmp/l2tp/l2tp-control","/var/run/l2tp-control");

}

int
stop_l2tp(void)
{
        int ret;
                                                                                                                             
        ret = eval("killall", "l2tpd");
        ret += eval("killall", "l2tpd");
        dprintf("done\n");
                                                                                                                             
        return ret;
}


int start_pptp(char *prefix)
{
	int ret;
	FILE *fp;
	char *pptp_argv[] = { "pppd", NULL};
	char username[32],passwd[32];
	char tmp[100];
	pid_t pid;

	strcpy(username, nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp)));
	strcpy(passwd, nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp)));

	/* Generate options file */
	if (!(fp = fopen("/tmp/ppp/options", "w"))) {
       	        perror("/tmp/ppp/options");
       	        return -1;
	}
	
	if (nvram_invmatch(strcat_r(prefix, "pppoe_idletime", tmp), ""))
		fprintf(fp, "idle %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp)));
	
       	fprintf(fp, "noauth\n");	
       	fprintf(fp, "user '%s'\n",username); 
	fprintf(fp, "demand\n");
	fprintf(fp, "connect true\n");    
       	fprintf(fp, "pty '/bin/pptp %s --nolaunchpppd'\n",nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp))); 

	fprintf(fp, "defaultroute\n"); 

	//if (nvram_invmatch(strcat_r(prefix, "pppoe_mtu", tmp), ""))	
       	//	fprintf(fp, "mtu %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp)));

      	if (nvram_invmatch(strcat_r(prefix, "dnsenable_x", tmp), "0"))
		fprintf(fp, "usepeerdns\n");   

       	fprintf(fp, "persist\n");        
       	fprintf(fp, "ipcp-accept-remote\n");  
       	fprintf(fp, "ipcp-accept-local\n");        
       	fprintf(fp, "noipdefault\n");  
       	fprintf(fp, "ktune\n");      
    	fprintf(fp, "default-asyncmap\n"); 
	fprintf(fp, "nopcomp\n");	
	fprintf(fp, "noaccomp\n");	
       	fprintf(fp, "noccp\n");  
       	fprintf(fp, "novj\n");       
       	fprintf(fp, "nobsdcomp\n");   
       	fprintf(fp, "nodeflate\n");     
	fprintf(fp, "lcp-echo-interval 10\n");
	fprintf(fp, "lcp-echo-failure 3\n");	
       	fprintf(fp, "lock\n");

       	fclose(fp);

       	/* Generate pap-secrets file */
       	if (!(fp = fopen("/tmp/ppp/pap-secrets", "w"))) {
       	        perror("/tmp/ppp/pap-secrets");
        	return -1;
       	}
       	fprintf(fp, "\"%s\" * \"%s\" *\n",
		username,
		passwd);
       	fclose(fp);
	chmod("/tmp/ppp/pap-secrets", 0600);

       	/* Generate chap-secrets file */
       	if (!(fp = fopen("/tmp/ppp/chap-secrets", "w"))) {
        	perror("/tmp/ppp/chap-secrets");
               	return -1;
       	}

       	fprintf(fp, "\"%s\" * \"%s\" *\n",
		username,
		passwd);
       	fclose(fp);

	chmod("/tmp/ppp/chap-secrets", 0600);

	/* Enable Forwarding */
	if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "r+"))) {
		fputc('1', fp);
		fclose(fp);
	} else
	{	
		perror("/proc/sys/net/ipv4/ip_forward");
	}
	
	/* Bring up  WAN interface */
	ifconfig(nvram_safe_get(strcat_r(prefix, "ifname", tmp)), 
		 IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	ret = _eval(pptp_argv, NULL, 0, &pid);
}
#if 0
int start_l2tp(char *prefix)
{
	int ret;
	FILE *fp;
	char *pptp_argv[] = { "pppd", NULL};
	char username[32],passwd[32];
	char tmp[100];
	pid_t pid;

	strcpy(username, nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp)));
	strcpy(passwd, nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp)));

	/* Generate options file */
	if (!(fp = fopen("/tmp/ppp/options", "w"))) {
       	        perror("/tmp/ppp/options");
       	        return -1;
	}
	
	if (nvram_invmatch(strcat_r(prefix, "pppoe_idletime", tmp), ""))
		fprintf(fp, "idle %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp)));
	
       	fprintf(fp, "noauth\n");	
       	fprintf(fp, "user '%s'\n",username); 
	fprintf(fp, "demand\n");
	fprintf(fp, "connect true\n");    
       	fprintf(fp, "pty '/bin/pptp %s --nolaunchpppd'\n",nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp))); 

	fprintf(fp, "defaultroute\n"); 

	//if (nvram_invmatch(strcat_r(prefix, "pppoe_mtu", tmp), ""))	
       	//	fprintf(fp, "mtu %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp)));

      	if (nvram_invmatch(strcat_r(prefix, "dnsenable_x", tmp), "0"))
		fprintf(fp, "usepeerdns\n");   

       	fprintf(fp, "persist\n");        
       	fprintf(fp, "ipcp-accept-remote\n");  
       	fprintf(fp, "ipcp-accept-local\n");        
       	fprintf(fp, "noipdefault\n");  
       	fprintf(fp, "ktune\n");      
    	fprintf(fp, "default-asyncmap\n"); 
	fprintf(fp, "nopcomp\n");	
	fprintf(fp, "noaccomp\n");	
       	fprintf(fp, "noccp\n");  
       	fprintf(fp, "novj\n");       
       	fprintf(fp, "nobsdcomp\n");   
       	fprintf(fp, "nodeflate\n");     
	fprintf(fp, "lcp-echo-interval 10\n");
	fprintf(fp, "lcp-echo-failure 3\n");	
       	fprintf(fp, "lock\n");

       	fclose(fp);

       	/* Generate pap-secrets file */
       	if (!(fp = fopen("/tmp/ppp/pap-secrets", "w"))) {
       	        perror("/tmp/ppp/pap-secrets");
        	return -1;
       	}
       	fprintf(fp, "\"%s\" * \"%s\" *\n",
		username,
		passwd);
       	fclose(fp);
	chmod("/tmp/ppp/pap-secrets", 0600);

       	/* Generate chap-secrets file */
       	if (!(fp = fopen("/tmp/ppp/chap-secrets", "w"))) {
        	perror("/tmp/ppp/chap-secrets");
               	return -1;
       	}

       	fprintf(fp, "\"%s\" * \"%s\" *\n",
		username,
		passwd);
       	fclose(fp);

	chmod("/tmp/ppp/chap-secrets", 0600);

	/* Enable Forwarding */
	if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "r+"))) {
		fputc('1', fp);
		fclose(fp);
	} else
	{	
		perror("/proc/sys/net/ipv4/ip_forward");
	}
	
	/* Bring up  WAN interface */
	ifconfig(nvram_safe_get(strcat_r(prefix, "ifname", tmp)), 
		 IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	ret = _eval(pptp_argv, NULL, 0, &pid);
}
#endif




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
