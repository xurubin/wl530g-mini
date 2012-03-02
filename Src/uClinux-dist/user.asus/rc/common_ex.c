
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
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<signal.h>
#include<bcmnvram.h>
#include<shutils.h>
#include<netconf.h>
#include<wlioctl.h>
#include<sys/time.h>
#include<syslog.h>
#include <stdarg.h>

#define XSTR(s) STR(s)
#define STR(s) #s

static char list[1500];

/* remove space in the end of string */
char *trim_r(char *str)
{
	int i;

	i=strlen(str)-1;

	while(i>0&&(*(str+i)==' ' || *(str+i)=='\r' || *(str+i)=='n'))
	{
		*(str+i)=0x0;
		i--;
	}
	return(str);
}

/* convert mac address format from XXXXXXXXXXXX to XX:XX:XX:XX:XX:XX */
char *mac_conv(char *mac_name, int idx, char *buf)
{
	char *mac, name[32];
	int i, j;

	if (idx!=-1)	
		sprintf(name, "%s%d", mac_name, idx);
	else sprintf(name, "%s", mac_name);

	mac = nvram_safe_get(name);

	if (strlen(mac)==0) 
	{
		buf[0] = 0;
	}
	else
	{
		j=0;	
		for(i=0; i<12; i++)
		{		
			if (i!=0&&i%2==0) buf[j++] = ':';
			buf[j++] = mac[i];
		}
	}
	buf[j] = 0;

	dprintf("mac: %s\n", buf);

	return(buf);
}

void getsyspara(void)
{
	FILE *fp;
	char buf[1];
	unsigned long *imagelen;
	char dataPtr[4];
	char verPtr[64];
	char productid[13];
	char fwver[12];
	int i;

	strcpy(productid, "WLHDD");
	strcpy(fwver, "0.1.0.1");

	if ((fp = fopen("/dev/mtd0", "rb"))!=NULL)
	{
		if (fseek(fp, 4, SEEK_SET)!=0) goto write_ver;
		fread(dataPtr, 1, 4, fp);
		imagelen = (unsigned long *)dataPtr;

		dprintf("image len %x\n", *imagelen);
		if (fseek(fp, *imagelen - 64, SEEK_SET)!=0) goto write_ver;
		dprintf("seek\n");
		if (!fread(verPtr, 1, 64, fp)) goto write_ver;
		dprintf("ver %x %x", verPtr[0], verPtr[1]);
		strncpy(productid, verPtr + 4, 12);
		productid[12] = 0;
		sprintf(fwver, "%d.%d.%d.%d", verPtr[0], verPtr[1], verPtr[2], verPtr[3]);

		dprintf("get fw ver: %s\n", productid);
		fclose(fp);
	}

write_ver:
	nvram_set("productid", productid);
	nvram_set("firmver", fwver);

	
	if (strstr(productid, "WL530"))
	{
		// only WL530 have auto mode
	}
	else
	{	
		// ugly solution
		if (nvram_match("wan_mode_x", "2"))
			nvram_set("wan_mode_x", "1");
	}
}

void wan_netmask_check()
{
	unsigned int ip, gw, nm, lip, lnm;

	if (nvram_match("wan0_proto", "static") ||
	    nvram_match("wan0_proto", "pptp"))
	{
		ip = inet_addr(nvram_safe_get("wan_ipaddr"));
		gw = inet_addr(nvram_safe_get("wan_gateway"));
		nm = inet_addr(nvram_safe_get("wan_netmask"));

		dprintf("ip : %x %x %x %x %x\n", ip, gw, nm, (ip&nm), (gw&nm));

		lip = inet_addr(nvram_safe_get("lan_ipaddr"));
		lnm = inet_addr(nvram_safe_get("lan_netmask"));

		if (ip==0x0 || (ip&lnm)==(lip&lnm))
		{
			nvram_set("wan_ipaddr", "1.1.1.1");
			nvram_set("wan_netmask", "255.0.0.0");	
		}

		// check netmask here
		if (gw==0 || gw==0xffffffff || (ip&nm)==(gw&nm))
		{

			nvram_set("wan0_netmask", nvram_safe_get("wan_netmask"));
		}
		else
		{
	
			dprintf("ip1 : %x %x %x %x %x\n", ip, gw, nm, (ip&nm), (gw&nm));	
			for(nm=0xffffffff;nm!=0;nm=(nm>>8))
			{
				dprintf("nm: %x\n", nm);

				if ((ip&nm)==(gw&nm)) 
				{
					break;
				}	
			}
			if (nm==0xffffffff) 
			{
				nvram_set("wan0_netmask", "255.255.255.255");
			}
			else if (nm==0xffffff) 
			{
				nvram_set("wan0_netmask", "255.255.255.0");
			}
			else if (nm==0xffff) 
			{
				nvram_set("wan0_netmask", "255.255.0.0");
			}
			else if (nm==0xff) 
			{
				nvram_set("wan0_netmask", "255.0.0.0");
			}
			else 
			{
				nvram_set("wan0_netmask", "0.0.0.0");
			}
		}
		nvram_set("wanx_ipaddr", nvram_safe_get("wan0_ipaddr"));
		nvram_set("wanx_netmask", nvram_safe_get("wan0_netmask"));
		nvram_set("wanx_gateway", nvram_safe_get("wan0_gateway"));
	}
}


/* This function is used to map nvram value from asus to Broadcom */
void convert_asus_values()
{	
	char tmpstr[32], tmpstr1[32], macbuf[36];
	char servers[64];
	char ifnames[36];
	char *ptr;
	int i, num;

	getsyspara();

	/* convert country code from regulation_domain */
	convert_country();

	dprintf("read from nvram\n");

	/* Wireless Section */
	/* Country Code */
	nvram_set("wl0_country_code", nvram_safe_get("wl_country_code"));

	/* GMODE */
	nvram_set("wl0_gmode", nvram_safe_get("wl_gmode"));

	if (nvram_match("wl_gmode_protection_x", "1"))
		nvram_set("wl0_gmode_protection", "auto");
	else
		nvram_set("wl0_gmode_protection", "off");

	if (nvram_match("wl_wep_x", "0"))
		nvram_set("wl0_wep", "disabled");
	else nvram_set("wl0_wep", "enabled");

	if (nvram_match("wl_auth_mode", "shared"))
		nvram_set("wl0_auth", "1");
	else nvram_set("wl0_auth", "0");

	nvram_set("wl0_auth_mode", nvram_safe_get("wl_auth_mode"));
	nvram_set("wl0_ssid", nvram_safe_get("wl_ssid"));

	if (nvram_match("wl_channel", "0"))
	{
		sprintf(tmpstr, "%d", atoi(nvram_safe_get("wl_chan_list")));	
		nvram_set("wl_channel", tmpstr);
	}

	nvram_set("wl0_channel", nvram_safe_get("wl_channel"));
	nvram_set("wl0_country_code", nvram_safe_get("wl_country_code"));
	nvram_set("wl0_rate", nvram_safe_get("wl_rate"));
	nvram_set("wl0_rateset", nvram_safe_get("wl_rateset"));
	nvram_set("wl0_frag", nvram_safe_get("wl_frag"));
	nvram_set("wl0_rts", nvram_safe_get("wl_rts"));
	nvram_set("wl0_dtim", nvram_safe_get("wl_dtim"));
	nvram_set("wl0_bcn", nvram_safe_get("wl_bcn"));
	nvram_set("wl0_plcphdr", nvram_safe_get("wl_plcphdr"));
	nvram_set("wl0_crypto", nvram_safe_get("wl_crypto"));
	nvram_set("wl0_wpa_psk", nvram_safe_get("wl_wpa_psk"));
	nvram_set("wl0_key", nvram_safe_get("wl_key"));
	nvram_set("wl0_key1", nvram_safe_get("wl_key1"));
	nvram_set("wl0_key2", nvram_safe_get("wl_key2"));
	nvram_set("wl0_key3", nvram_safe_get("wl_key3"));
	nvram_set("wl0_key4", nvram_safe_get("wl_key4"));
	nvram_set("wl0_closed", nvram_safe_get("wl_closed"));
	nvram_set("wl0_frameburst", nvram_safe_get("wl_frameburst"));
	nvram_set("wl0_afterburner", nvram_safe_get("wl_afterburner"));
	nvram_set("wl0_ap_isolate", nvram_safe_get("wl_ap_isolate"));
	nvram_set("wl0_radio", nvram_safe_get("wl_radio_x"));
	nvram_set("wl0_radius_ipaddr", nvram_safe_get("wl_radius_ipaddr"));
	nvram_set("wl0_radius_port", nvram_safe_get("wl_radius_port"));
	nvram_set("wl0_radius_secret", nvram_safe_get("wl_radius_secret"));

	if (nvram_invmatch("wl_mode_ex", "ap"))
	{
		/* Station or Ethernet Bridge Mode */
		nvram_set("wl0_mode", nvram_safe_get("wl_mode_ex"));
	}
	else
	{
		/* WDS control */
		if (nvram_match("wl_mode_x", "1")) 
			nvram_set("wl0_mode", "wds");
		else nvram_set("wl0_mode", "ap");

		nvram_set("wl0_lazywds", nvram_safe_get("wl_lazywds"));
	}

	if (nvram_match("wl_wdsapply_x", "1"))
	{
		num = atoi(nvram_safe_get("wl_wdsnum_x"));
		list[0]=0;

		for(i=0;i<num;i++)
		{
			sprintf(list, "%s %s", list, mac_conv("wl_wdslist_x", i, macbuf));
		}

		dprintf("wds list %s %x\n", list, num);

		nvram_set("wl0_wds", list);
	}
	else nvram_set("wl0_wds", "");

	/* Mac filter */
	nvram_set("wl0_macmode", nvram_safe_get("wl_macmode"));

	/* Direct copy value */
	/* LAN Section */

	/* LAN Section */
	if (nvram_match("dhcp_enable_x", "1"))
		nvram_set("lan_proto", "dhcp");
	else nvram_set("lan_proto", "static");

	nvram_set("wan0_proto", nvram_safe_get("wan_proto"));
	nvram_set("wan0_ipaddr", nvram_safe_get("wan_ipaddr"));
	nvram_set("wan0_gateway", nvram_safe_get("wan_gateway"));

#ifndef REMOVE_QTR
	if (nvram_invmatch("wan0_proto", "pptp"))
	{
		eval("insmod", "ip_conntrack_proto_gre.o");
		eval("insmod", "ip_conntrack_pptp.o");
		eval("insmod", "ip_nat_proto_gre.o");
		eval("insmod", "ip_nat_pptp.o");	
	}
#endif

	nvram_set("wan_ipaddr_t", "");
	nvram_set("wan_netmask_t", "");
	nvram_set("wan_gateway_t", "");
	nvram_set("wan_dns_t", "");
	nvram_set("wan_status_t", "Disconnected");

	wan_netmask_check();

	if (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pptp"))
	{
		nvram_set("wan0_pppoe_ifname", "ppp0");
		nvram_set("wan0_pppoe_username", nvram_safe_get("wan_pppoe_username"));
		nvram_set("wan0_pppoe_passwd", nvram_safe_get("wan_pppoe_passwd"));
		nvram_set("wan0_pppoe_idletime", nvram_safe_get("wan_pppoe_idletime"));
		nvram_set("wan0_pppoe_txonly_x", nvram_safe_get("wan_pppoe_txonly_x"));
		nvram_set("wan0_pppoe_mtu", nvram_safe_get("wan_pppoe_mtu"));
		nvram_set("wan0_pppoe_mru", nvram_safe_get("wan_pppoe_mru"));
		nvram_set("wan0_pppoe_service", nvram_safe_get("wan_pppoe_service"));
		nvram_set("wan0_pppoe_ac", nvram_safe_get("wan_pppoe_ac"));
		nvram_set("wan0_pppoe_demand", "1");
		nvram_set("wan0_pppoe_ipaddr", nvram_safe_get("wan_ipaddr"));
		nvram_set("wan0_pppoe_netmask", nvram_safe_get("wan_netmask"));
		nvram_set("wan0_pppoe_gateway", nvram_safe_get("wan_gateway"));
	}
	nvram_set("wan0_hostname", nvram_safe_get("wan_hostname"));
	nvram_set("wan0_hwaddr", mac_conv("wan_hwaddr_x", -1, macbuf));

	convert_routes();

	memset(servers, 0, sizeof(servers));

	if (nvram_invmatch("ntp_server0", ""))
		sprintf(servers, "%s%s ", servers, nvram_safe_get("ntp_server0"));
	if (nvram_invmatch("ntp_server1", ""))
		sprintf(servers, "%s%s ", servers, nvram_safe_get("ntp_server1"));

	nvram_set("ntp_servers", servers);

	if (nvram_match("wan_nat_x", "0") && nvram_match("wan_route_x", "IP_Bridged"))
	{
		sprintf(ifnames, "%s", nvram_safe_get("lan_ifnames"));
		sprintf(ifnames, "%s %s", ifnames, nvram_safe_get("wan_ifnames"));
		nvram_set("lan_ifnames_t", ifnames);
		nvram_set("router_disable", "1");
		nvram_set("vlan_enable", "0");
	}
	else 
	{ 
		nvram_set("lan_ifnames_t", nvram_safe_get("lan_ifnames"));
		nvram_set("router_disable", "0");
		nvram_set("vlan_enable", "1");
	}

	// clean some temp variables
	nvram_set("usb_web_device", "");
	nvram_set("usb_web_flag", "");
	nvram_set("login_ip", "");

	if (nvram_match("misc_http_x", "1"))
	{
		if (nvram_invmatch("misc_httpport_x", ""))
			nvram_set("http_wanport", nvram_safe_get("misc_httpport_x"));
		else nvram_set("http_wanport", "8080");
	}
	else nvram_set("http_wanport", "");

	if (nvram_invmatch("fw_enable_x", "0"))
	{
		nvram_set("fw_disable", "0");
	}
	else
	{
		nvram_set("fw_disable", "1");
	}

	num = 0;	
	if (nvram_match("fw_log_x", "accept") ||
		nvram_match("fw_log_x", "both"))
		num |= 2;
		
	if (nvram_match("fw_log_x", "drop") ||
		nvram_match("fw_log_x", "both"))
		num |= 1;
	
	sprintf(tmpstr, "%d", num);
	nvram_set("log_level", tmpstr);	

	dprintf("end map\n");
	
	if(nvram_invmatch("sp_battle_ips", "0"))
	{
		eval("insmod", "ip_nat_starcraft.o");
		eval("insmod", "ipt_NETMAP.o");
	}


	strcpy(tmpstr, nvram_safe_get("time_zone"));
	/* replace . with : */
	if ((ptr=strchr(tmpstr, '.'))!=NULL) *ptr = ':';
	/* remove *_? */
	if ((ptr=strchr(tmpstr, '_'))!=NULL) *ptr = 0x0;
	nvram_set("time_zone_x", tmpstr);
	
	dprintf("end map\n");
}


char *findpattern(char *target, char *pattern)
{
	char *find;
	int len;

	//printf("find : %s %s\n", target, pattern);

	if ((find=strstr(target, pattern)))
	{
		len = strlen(pattern);
		if (find[len]==';' || find[len]==0)
		{
			return find;
		}
	}
	return NULL;
}

void update_lan_status(int isup)
{
	char *proto;
		
	if (isup)
	{
		nvram_set("lan_ipaddr_t", nvram_safe_get("lan_ipaddr"));
		nvram_set("lan_netmask_t", nvram_safe_get("lan_netmask"));

		if (nvram_match("wan_route_x", "IP_Routed"))
		{
			if (nvram_match("lan_proto", "dhcp"))
			{
				if(nvram_invmatch("dhcp_gateway_x", ""))
					nvram_set("lan_gateway_t", nvram_safe_get("dhcp_gateway_x"));
				else nvram_set("lan_gateway_t", nvram_safe_get("lan_ipaddr"));
			}
			else nvram_set("lan_gateway_t", nvram_safe_get("lan_ipaddr"));
		}
		else
		{
			nvram_set("lan_gateway_t", nvram_safe_get("lan_gateway"));
		}
	}
}


void update_wan_status(int isup)
{
	char *proto;
	char dns_str[36];

	proto = nvram_safe_get("wan_proto");

	if (!strcmp(proto, "static")) nvram_set("wan_proto_t", "Static");
	else if (!strcmp(proto, "dhcp")) nvram_set("wan_proto_t", "Automatic IP");
	else if (!strcmp(proto, "pppoe")) nvram_set("wan_proto_t", "PPPoE");
	else if (!strcmp(proto, "pptp")) nvram_set("wan_proto_t", "PPTP");
	else if (!strcmp(proto, "bigpond")) nvram_set("wan_proto_t", "BigPond");


		
	if (!isup)
	{
		nvram_set("wan_ipaddr_t", "");
		nvram_set("wan_netmask_t", "");
		nvram_set("wan_gateway_t", "");
		nvram_set("wan_dns_t", "");
		nvram_set("wan_status_t", "Disconnected");
	}	
	else
	{
		nvram_set("wan_ipaddr_t", nvram_safe_get("wan0_ipaddr"));
		nvram_set("wan_netmask_t", nvram_safe_get("wan0_netmask"));
		nvram_set("wan_gateway_t", nvram_safe_get("wan0_gateway"));


		if (nvram_invmatch("wan_dnsenable_x", "1"))	
		{		
			if (nvram_invmatch("wan_dns1_x",""))
				sprintf(dns_str, "%s", nvram_safe_get("wan_dns1_x"));		
			if (nvram_invmatch("wan_dns1_x",""))
				sprintf(dns_str, " %s", nvram_safe_get("wan_dns1_x"));		
			nvram_set("wan_dns_t", dns_str);
		}
		else nvram_set("wan_dns_t", nvram_safe_get("wan0_dns"));
		nvram_set("wan_status_t", "Connected");
	}

	if (isup) wanledctrl(1); // wan port on
	else wanledctrl(0);
}

/*
 * logmessage
 *
 */
void logmessage(char *logheader, char *fmt, ...)
{
  va_list args;
  char buf[512];

  va_start(args, fmt);

  vsnprintf(buf, sizeof(buf), fmt, args);
  openlog(logheader, 0, 0);
  syslog(0, buf);
  closelog();
  va_end(args);
}


/*
 * wanmessage
 *
 */
void wanmessage(char *fmt, ...)
{
  va_list args;
  char buf[512];

  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  nvram_set("wan_reason_t", buf);
  va_end(args);
}

/* 
 * pppstatus
 * 
 */
char *pppstatus(char *buf)
{
   FILE *fp;
   char sline[128], *p;

   if ((fp=fopen("/tmp/wanstatus.log", "r")) && fgets(sline, sizeof(sline), fp))
   {
	p = strstr(sline, ",");
	strcpy(buf, p+1);
   }
   else
   {
	strcpy(buf, "unknown reason");
   }	
}


/* 
 * Kills process whose PID is stored in plaintext in pidfile
 * @param	pidfile	PID file, signal
 * @return	0 on success and errno on failure
 */
int
kill_pidfile_s(char *pidfile, int sig)
{
	FILE *fp = fopen(pidfile, "r");
	char buf[256];
	extern errno;

	if (fp && fgets(buf, sizeof(buf), fp)) {
		pid_t pid = strtoul(buf, NULL, 0);
		fclose(fp);
		return kill(pid, sig);
  	} else
		return errno;
}
