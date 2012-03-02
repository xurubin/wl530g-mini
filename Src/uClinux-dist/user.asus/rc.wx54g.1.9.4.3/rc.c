/*
 * Router rc control script
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
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <dirent.h>

#include <epivers.h>
#include <bcmnvram.h>
#include <mtd.h>
#include <shutils.h>
#include <rc.h>
#include <netconf.h>
#include <nvparse.h>
#include <bcmdevs.h>
#include <wlutils.h>

#define STANDALONE 1
#include <rgcfg.h>

#define RC1_START() 
#define RC1_STOP()  
#define RC7_START()
#define RC7_STOP()
#define LINUX_OVERRIDES() 
extern struct nvram_tuple router_defaults[];
static void restore_defaults(void);
static void sysinit(void);
static void rc_signal(int sig);

#ifdef ASUS_EXTENSION
#define MFG_DEV "/dev/mtd3"
//#define MFG_DEV "/etc/mfg_data"
//#define HAVE_MFG_DATA (mfg_data.magic!=0xffff)
#define HAVE_MFG_DATA 1
int control_led(int led, int flag);
struct mvwlan_mfg_param mfg_data;
pid_t mvapd_pid=0;	
#endif

int 
read_mfg_data(void)
{
	FILE *fp;
	char domain[12];

	fp=fopen(MFG_DEV, "rb");

	if (fp==NULL) return 0;
	else
	{
		fread(&mfg_data, 1, sizeof(struct mvwlan_mfg_param), fp);
		fclose(fp);

		sprintf(domain, "0x%02x", mfg_data.domain);
		nvram_set("regulation_domain", domain);

		printf("MFG: magic %x %x\n", mfg_data.magic, mfg_data.domain);
		
		if (mfg_data.magic==0xffff) 
		{
			return 0;		
		}
	}
	return 1;
}

void
create_unimac_conf(void)
{
	FILE *fp;
	char tmpmac[32], *mac;
	int i,j;

	// create unimac.conf
	fp=fopen("/tmp/unimac.conf","w");
	if (fp==NULL) return;	

	if (nvram_invmatch("wan_hwaddr_x",""))
	{
		mac = nvram_safe_get("wan_hwaddr_x");

		j=0;
		for(i=0;i<12;i++)
		{
			if (i!=0&&i%2==0) tmpmac[j++]=':';
			tmpmac[j++] = mac[i];
		}
		tmpmac[j]=0;
	}
	else tmpmac[0]=0;

	if (HAVE_MFG_DATA) 
		fprintf(fp, "MFG_FILE=%s\n", MFG_DEV);
	else fprintf(fp, "MFG_FILE=/etc/mfg_data\n");


	if (nvram_match("wan_mode_x", "0"))
	{	
		fprintf(fp, "VLANS=1\n");
		fprintf(fp, "VLAN=LAN\n");
		fprintf(fp, "PORTS=0,1,2,3,4\n");
		//fprintf(fp, "PORTS=0,1\n");
		fclose(fp);
	}
	else
	{
		fprintf(fp, "VLANS=2\n");
		fprintf(fp, "VLAN=LAN\n");	
		fprintf(fp, "PORTS=1,2,3,4\n");
		//fprintf(fp, "PORTS=1\n");

		fprintf(fp, "VLAN=WAN\n");
		fprintf(fp, "PORTS=0\n");
		if (strlen(tmpmac)>0)
			fprintf(fp, "MAC=%s\n", tmpmac);
	}
	fclose(fp);
}

void
create_hostapd_conf(void)
{
#ifdef ASUS
	FILE *fp;
	char *tmpstr;
	int mode;

	// create hostapd.conf
	fp=fopen("/tmp/hostapd.conf","w");
	if (fp==NULL)
	{
		return;	
	}

	if (HAVE_MFG_DATA) fprintf(fp, "manuf_file=%s\n", MFG_DEV);
	else fprintf(fp, "manuf_file=/etc/mfg_data\n");
	fprintf(fp, "interface=wlan0\n");
	fprintf(fp, "logger_syslog=-1\n");
	fprintf(fp, "logger_syslog_level=4\n");	
	fprintf(fp, "logger_stdout=-1\n");	
	fprintf(fp, "logger_stdout_level=4\n");	
	fprintf(fp, "debug=0\n");
	fprintf(fp, "dump_file=/tmp/hostapd.dump\n");	
	fprintf(fp, "daemonize=1\n");

#ifdef WPA2 // Those setting starts from V2.0
	fprintf(fp, "wireless_enable=1\n");
	fprintf(fp, "auto_link=0\n");
	fprintf(fp, "ssid_patch=0\n");
	fprintf(fp, "speed_booster=0\n");
#endif

	if (nvram_match("wl_gmode", "1")) //Auto
	{
		//fprintf(fp, "ap_mode=2\n");
		mode = 2;

		if (nvram_match("wl_rateset", "12"))
			fprintf(fp,"basic_rate=1000,2000\n");
		else if (nvram_match("wl_rateset", "default"))	
			fprintf(fp,"basic_rate=1000,2000,5500,11000\n");	
		else	fprintf(fp,"basic_rate=1000,2000,5500,11000,6000,9000,12000,24000,36000,48000,54000\n");		
		fprintf(fp,"oper_rate=1000,2000,5500,11000,6000,9000,12000,18000,24000,36000,48000,54000\n");		
	}
	else if (nvram_match("wl_gmode", "4")) //802.11g
	{
		//fprintf(fp, "ap_mode=1\n");
		mode = 1;
	
		if (nvram_match("wl_rateset", "12"))
			fprintf(fp,"basic_rate=1000,2000\n");
		else if (nvram_match("wl_rateset", "default"))	
			fprintf(fp,"basic_rate=1000,2000,5500,11000,6000,9000,12000,24000\n");	
		else	fprintf(fp,"basic_rate=1000,2000,5500,11000,6000,9000,12000,24000,36000,48000,54000\n");	
		fprintf(fp,"oper_rate=1000,2000,5500,11000,6000,9000,12000,18000,24000,36000,48000,54000\n");	
	}
	else //802.11b
	{
		//fprintf(fp, "ap_mode=0\n");
		mode = 0;
	
		if (nvram_match("wl_rateset", "12"))
			fprintf(fp,"basic_rate=1000,2000\n");
		else if (nvram_match("wl_rateset", "default"))	
			fprintf(fp,"basic_rate=1000,2000,5500,11000\n");	
		else	
			fprintf(fp,"basic_rate=1000,2000,5500,11000\n");

		fprintf(fp,"oper_rate=1000,2000,5500,11000\n");	
	
	}

	if (nvram_match("wl_gmode_protection", "1")) //802.11g Protection
	{
		fprintf(fp, "g_protect=1\n");
	}
	else
	{
		fprintf(fp, "g_protect=0\n");
	}

	fprintf(fp, "iw_mode=3\n");

	if (nvram_match("wl_rate", "0"))
	{
		fprintf(fp, "fixed_tx_data_rate=%s\n", nvram_safe_get("wl_rate"));
	}
	else 
	{
		char tmprate[16];
		int len;

		strcpy(tmprate, nvram_safe_get("wl_rate"));
		len = strlen(tmprate);
		tmprate[len-3] = 0;

		if (strcmp(tmprate, "1000") == 0 ||
			strcmp(tmprate, "2000") == 0 ||
			strcmp(tmprate, "5500") == 0 ||
			strcmp(tmprate, "11000") == 0)
		{

			if (mode==1) // gonly mode
			{
				fprintf(fp, "fixed_tx_data_rate=0\n");
			}
			else
			{
				fprintf(fp, "fixed_tx_data_rate=1\n");
				fprintf(fp, "fixed_tx_b_rate=%s\n", tmprate);
				fprintf(fp, "fixed_tx_g_rate=54000\n");
				if (mode==2) mode = 0;
			}
		}
		else
		{
			if (mode==0) // b only mode
			{
				fprintf(fp, "fixed_tx_data_rate=0\n");
			}
			else
			{		
				fprintf(fp, "fixed_tx_data_rate=1\n");
				fprintf(fp, "fixed_tx_b_rate=11000\n");
				fprintf(fp, "fixed_tx_g_rate=%s\n", tmprate);
				if (mode==2) mode = 1;
			}
		}
	}

	fprintf(fp, "ap_mode=%d\n", mode);

	fprintf(fp, "ssid=%s\n", nvram_safe_get("wl_ssid")); 	
	fprintf(fp, "beacon_interval=%s\n", nvram_safe_get("wl_bcn"));
	fprintf(fp, "dtim_period=%s\n", nvram_safe_get("wl_dtim")); 	
	fprintf(fp, "rts_threshold=%s\n", nvram_safe_get("wl_rts")); 		
	fprintf(fp, "fragment_threshold=%s\n", nvram_safe_get("wl_frag")); 	
	fprintf(fp, "short_retry_limit=7\n"); 		
	fprintf(fp, "long_retry_limit=4\n");
	fprintf(fp, "current_tx_power_level=1\n");	
	fprintf(fp, "short_preamble=1\n"); 			
	fprintf(fp, "channel=%s\n", nvram_safe_get("wl_channel")); 		
	fprintf(fp, "hide_ssid=%s\n", nvram_safe_get("wl_closed")); 	
	fprintf(fp, "minimal_eap=0\n");
		
	if (nvram_match("wl_auth_mode","shared"))
	{
		fprintf(fp, "auth_algs=2\n"); 		
	}
	else if (nvram_match("wl_auth_mode","psk"))
	{
		fprintf(fp, "auth_algs=1\n"); 		
	}
	else 
	{
		fprintf(fp, "auth_algs=3\n");
	}

	if (nvram_match("wl_auth_mode","open") || nvram_match("wl_auth_mode", "shared"))
	{
		if (nvram_match("wl_wep_x","0"))
		{	
			fprintf(fp, "privacy_invoked=0\n");
			fprintf(fp, "exclude_unencrypted=0\n");
		}
		else if (nvram_match("wl_wep_x","1"))
		{	
			fprintf(fp, "privacy_invoked=1\n");
			fprintf(fp, "exclude_unencrypted=0\r\n");			

			fprintf(fp, "wep_key_type=hex\n");		
	
			fprintf(fp, "wep_key_len=5\n");		
	
			fprintf(fp, "wep_default_key1=%s\n", nvram_safe_get("wl_key1")); 	

			fprintf(fp, "wep_default_key2=%s\n", nvram_safe_get("wl_key2")); 	

			fprintf(fp, "wep_default_key3=%s\n", nvram_safe_get("wl_key3")); 	

			fprintf(fp, "wep_default_key4=%s\n", nvram_safe_get("wl_key4"));
	
			fprintf(fp, "wep_default_key=%s\n", nvram_safe_get("wl_key")); 	

		}
		else if (nvram_match("wl_wep_x","2"))
		{	
			fprintf(fp, "privacy_invoked=1\n");
			fprintf(fp, "exclude_unencrypted=0\n");

			fprintf(fp, "wep_key_type=hex\n");		
	
			fprintf(fp, "wep_key_len=13\n");		
			fprintf(fp, "wep_default_key1=%s\n", nvram_safe_get("wl_key1")); 	

			fprintf(fp, "wep_default_key2=%s\n", nvram_safe_get("wl_key2")); 	

			fprintf(fp, "wep_default_key3=%s\n", nvram_safe_get("wl_key3")); 	

			fprintf(fp, "wep_default_key4=%s\n", nvram_safe_get("wl_key4")); 
	
			fprintf(fp, "wep_default_key=%s\n", nvram_safe_get("wl_key")); 	
		}


		fprintf(fp, "wpa_mode=0\n");

#ifdef WPA2
		fprintf(fp, "wpa2_mode=0\n");
#endif

	}
	else // wpa-psk
	{
#ifdef WPA2
		if (nvram_match("wl_crypto", "tkip"))
		{	
			fprintf(fp, "wpa_mode=1\n");
			fprintf(fp, "wpa_encry=2\n");
			fprintf(fp, "wpa_passphrase=%s\n", nvram_safe_get("wl_wpa_psk"));
			fprintf(fp, "wpa2_mode=0\n");
			fprintf(fp, "wpa2_encry=4\n");
			fprintf(fp, "wpa2_passphrase=%s\n", nvram_safe_get("wl_wpa_psk"));
		}
		else if (nvram_match("wl_crypto", "aes"))
		{
			fprintf(fp, "wpa_mode=0\n");
			fprintf(fp, "wpa_encry=2\n");
			fprintf(fp, "wpa_passphrase=%s\n", nvram_safe_get("wl_wpa_psk"));
			fprintf(fp, "wpa2_mode=1\n");
			fprintf(fp, "wpa2_encry=4\n");
			fprintf(fp, "wpa2_passphrase=%s\n", nvram_safe_get("wl_wpa_psk"));
		}
		else // Both
		{

			fprintf(fp, "wpa_mode=1\n");
			fprintf(fp, "wpa_encry=2\n");
			fprintf(fp, "wpa_passphrase=%s\n", nvram_safe_get("wl_wpa_psk"));

			fprintf(fp, "wpa2_mode=1\n");
			fprintf(fp, "wpa2_encry=4\n");
			fprintf(fp, "wpa2_passphrase=%s\n", nvram_safe_get("wl_wpa_psk"));
		}

		fprintf(fp, "group_rekey_time=%s\n", nvram_safe_get("wl_wpa_gtk_rekey"));
 
#else
		fprintf(fp, "wpa_mode=1\n");
		fprintf(fp, "wpa_encry=2\n");
		fprintf(fp, "wpa_passphrase=%s\n", nvram_safe_get("wl_wpa_psk"));
		fprintf(fp, "wpa_group_rekey_time=%s\n", nvram_safe_get("wl_wpa_gtk_rekey")); 
#endif
	}

	if (nvram_match("wl_macmode", "allow"))
	{	
		fprintf(fp, "macaddr_acl=1\n");
		//1, deny unless in accept list 		
		fprintf(fp, "accept_mac_file=/tmp/hostapd.accept\n"); 
		write_acl_list("/tmp/hostapd.accept");	
	}
	else if (nvram_match("wl_macmode", "deny"))
	{	
		fprintf(fp, "macaddr_acl=0\n"); 		
		//0, accept unless in deny list
		fprintf(fp, "deny_mac_file=/tmp/hostapd.deny\n");
		write_acl_list("/tmp/hostapd.deny");
	}

	if (strstr(nvram_safe_get("productid"), "WL520"))
	{	
		fprintf(fp, "wlan_tx_gpio=1\n");
		fprintf(fp, "wlan_rx_gpio=1\n");
	}
	else
	{
		fprintf(fp, "wlan_tx_gpio=0\n");
		fprintf(fp, "wlan_rx_gpio=0\n");
	}
	fprintf(fp, "antenna=1\n");
#ifdef WPA2
	fprintf(fp, "watchdog_timer=1\n");
#endif

#ifdef WDS
	if (nvram_match("wl_mode_x", "1"))
	{
		fprintf(fp, "op_mode=1\n");
	}
	else
	{
		fprintf(fp, "op_mode=0\n");
	}
	if (nvram_match("wl_ap_isolate", "1"))
		fprintf(fp, "sta_bridge=0\n");
	else fprintf(fp, "sta_bridge=1\n");
#endif

	fclose(fp);
#endif
}

void
start_devices(void)
{
	char *mvapd_argv[] = { "mvapd", "/tmp/hostapd.conf", NULL };
	char pid[16];

	create_unimac_conf();
	eval("rgcfg","-i");
	
	create_hostapd_conf();
	_eval(mvapd_argv, NULL, 0, &mvapd_pid);
	//printf("mvapd pid : %x\n", mvapd_pid);
	sleep(2);
	sprintf(pid, "%s", mvapd_pid);
	eval("kill", "-1", pid);

#ifdef WDS
	if (nvram_invmatch("wl_wdsapply_x", "0"))
	{
		// write WDS list here
		write_wds_list();	
	}
#endif
}

/* 					
 * WAN state/LED description 		
 * WAN disabled		: OFF		
 * Cable not attached	: BLINK 	
 * Disconnected		: BLINK(PPTP/PPPoE,DHCP)
 * Connected		: ON		
 */
 
check_wanmode(void)
{
	#define WANPHY 		0
	#define WANLINKOFF 	5
	unsigned int link=0;

	ReadSwitchReg_r(WANPHY, WANLINKOFF, &link);

	if (link) return 1;
	else return 0;
}

void
change_wanmode(void)
{
	#define WANPHY 		0
	#define WANLINKOFF 	5
	unsigned int link=0;

	if (nvram_match("wan_mode_x", "0"))
		nvram_set("router_disable", "1");
	else if (nvram_match("wan_mode_x", "1") ||
		 nvram_match("wan_mode_x", "2"))
		nvram_set("router_disable", "0");

	//printf("wan mode %s\n", nvram_safe_get("router_disable"));
}

static void
restore_defaults(void)
{
	extern struct nvram_tuple router_defaults[];
	
	struct nvram_tuple linux_overrides[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "LAN wlan0", 0 },
		{ "wan_ifname", "WAN", 0 },
		{ "wan_ifnames", "WAN", 0 },
		{ 0, 0, 0 }
	};
	struct nvram_tuple *t, *u;

	int restore_defaults, i;

	/* Restore defaults if told to or OS has changed */
	restore_defaults = !nvram_match("restore_defaults", "0") || nvram_invmatch("os_name", "linux");
	if (restore_defaults)
		cprintf("Restoring defaults...");

#ifdef REMOVE
	/* Delete dynamically generated variables */
	if (restore_defaults) {
		char tmp[100], prefix[] = "wlXXXXXXXXXX_";
		for (i = 0; i < MAX_NVPARSE; i++) {
			del_filter_client(i);
			del_forward_port(i);
			del_autofw_port(i);
			snprintf(prefix, sizeof(prefix), "wl%d_", i);
			nvram_unset(strcat_r(prefix, "ifname", tmp));
		}
	}
#endif
	/* Restore defaults */
	for (t = router_defaults; t->name; t++) {
		if (restore_defaults || !nvram_get(t->name)) {
			for (u = linux_overrides; u && u->name; u++) {
				if (!strcmp(t->name, u->name)) {
					nvram_set(u->name, u->value);
					break;
				}
			}
			if (!u || !u->name)
				nvram_set(t->name, t->value);
		}
	}

	/* Always set OS defaults */
	nvram_set("os_name", "linux");
	nvram_set("os_version", EPI_VERSION_STR);

	/* Commit values */
	if (restore_defaults) {
		nvram_commit();
		cprintf("done\n");
	}
}

#ifdef REMOVE
static int
build_ifnames(char *type, char *names, int *size)
{
	char name[32], *next;
	int len = 0;
	int s;

	/* open a raw scoket for ioctl */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
       		return -1;

	/*
	 * go thru all device names (wl<N> il<N> et<N> vlan<N>) and interfaces to 
	 * build an interface name list in which each i/f name coresponds to a device
	 * name in device name list. Interface/device name matching rule is device
	 * type dependant:
	 *
	 *	wl:	by unit # provided by the driver, for example, if eth1 is wireless
	 *		i/f and its unit # is 0, then it will be in the i/f name list if
	 *		wl0 is in the device name list.
	 *	il/et:	by mac address, for example, if et0's mac address is identical to
	 *		that of eth2's, then eth2 will be in the i/f name list if et0 is 
	 *		in the device name list.
	 *	vlan:	by name, for example, vlan0 will be in the i/f name list if vlan0
	 *		is in the device name list.
	 */
	foreach (name, type, next) {
		struct ifreq ifr;
		int i, unit;
		char var[32], *mac, ea[ETHER_ADDR_LEN];
		
		/* ifindex could be sparsely, so try each of them and 16 max is ok for now */
		for (i = 1; i <= 16; i ++) {
			/* ignore i/f that is not ethernet */
			ifr.ifr_ifindex = i;
			if (ioctl(s, SIOCGIFNAME, &ifr))
				continue;
			if (ioctl(s, SIOCGIFHWADDR, &ifr))
				continue;
			if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
				continue;
			
			/* wl: use unit # to identify wl */
			if (!strncmp(name, "wl", 2)) {
				if (wl_probe(ifr.ifr_name) ||
				    wl_ioctl(ifr.ifr_name, WLC_GET_INSTANCE, &unit, sizeof(unit)) ||
				    unit != atoi(&name[2]))
					continue;
			}
			/* et/il: use mac addr to identify et/il */
			else if (!strncmp(name, "et", 2) || !strncmp(name, "il", 2)) {
				snprintf(var, sizeof(var), "%smacaddr", name);
				if (!(mac = nvram_get(var)) || 
				    !ether_atoe(mac, ea) ||
				    bcmp(ea, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
					continue;
			}
			/* vlan: use name to identify vlan */
			else if (!strncmp(name, "vlan", 4)) {
				if (strcmp(name, ifr.ifr_name))
					continue;
			}
			/* mac address: compare value */
			else if (ether_atoe(name, ea) && !bcmp(ea, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN))
				;
			/* others: ignore */
			else
				continue;

			/* append interface name to list */
			len += snprintf(&names[len], *size - len, "%s ", ifr.ifr_name);
		}
	}
	
	close(s);

	*size = len;
	return 0;
}	

static void
restore_defaults(void)
{
	struct nvram_tuple generic[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "eth0 eth2 eth3 eth4", 0 },
		{ "wan_ifname", "eth1", 0 },
		{ "wan_ifnames", "eth1", 0 },
		{ 0, 0, 0 }
	};
	struct nvram_tuple vlan[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "vlan0 eth1 eth2 eth3", 0 },
		{ "wan_ifname", "vlan1", 0 },
		{ "wan_ifnames", "vlan1", 0 },
		{ 0, 0, 0 }
	};
	struct nvram_tuple dyna[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "", 0 },
		{ "wan_ifname", "", 0 },
		{ "wan_ifnames", "", 0 },
		{ 0, 0, 0 }
	};
#ifdef ASUS_EXT
	struct nvram_tuple wl300g[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "eth2", 0 },
		{ "wan_ifname", "eth0", 0 },
		{ "wan_ifnames", "eth0", 0 },
		{ "wan_nat_x", "0", 0},
		{ "wan_route_x", "0", 0},
		{ 0, 0, 0 }
	};
	struct nvram_tuple wl300g2[] = {
		{ "lan_ifname", "br0", 0 },
		{ "lan_ifnames", "eth0 eth2", 0 },
		{ "wan_ifname", "eth1", 0 },
		{ "wan_ifnames", "eth1", 0 },
		{ "wan_nat_x", "0", 0},
		{ "wan_route_x", "0", 0},
		{ 0, 0, 0 }
	};
#endif

#ifdef CONFIG_SENTRY5
#include "rcs5.h"
#else
#define RC1_START() 
#define RC1_STOP()  
#define RC7_START()
#define RC7_STOP()
#define LINUX_OVERRIDES() 
#define EXTRA_RESTORE_DEFAULTS() 
#endif

	struct nvram_tuple *linux_overrides;
	struct nvram_tuple *t, *u;
	int restore_defaults, i;
	uint boardflags;
	char *landevs, *wandevs;
	char lan_ifnames[128], wan_ifnames[128];
	char wan_ifname[32], *next;
	int len;
	int ap = 0;

	/* Restore defaults if told to or OS has changed */
	restore_defaults = !nvram_match("restore_defaults", "0") || nvram_invmatch("os_name", "linux");
	if (restore_defaults)
		cprintf("Restoring defaults...");

	/* Delete dynamically generated variables */
	if (restore_defaults) {
		char tmp[100], prefix[] = "wlXXXXXXXXXX_";
		for (i = 0; i < MAX_NVPARSE; i++) {
			del_filter_client(i);
			del_forward_port(i);
			del_autofw_port(i);
			snprintf(prefix, sizeof(prefix), "wl%d_", i);
			for (t = router_defaults; t->name; t ++) {
				if (!strncmp(t->name, "wl_", 3))
					nvram_unset(strcat_r(prefix, &t->name[3], tmp));
			}
			snprintf(prefix, sizeof(prefix), "wan%d_", i);
			for (t = router_defaults; t->name; t ++) {
				if (!strncmp(t->name, "wan_", 4))
					nvram_unset(strcat_r(prefix, &t->name[4], tmp));
			}
		}
	}

#ifdef REMOVE
	/* 
	 * Build bridged i/f name list and wan i/f name list from lan device name list
	 * and wan device name list. Both lan device list "landevs" and wan device list
	 * "wandevs" must exist in order to preceed.
	 */
	if ((landevs = nvram_get("landevs")) && (wandevs = nvram_get("wandevs"))) {
		/* build bridged i/f list based on nvram variable "landevs" */
		len = sizeof(lan_ifnames);
		if (!build_ifnames(landevs, lan_ifnames, &len) && len)
			dyna[1].value = lan_ifnames;
		else
			goto canned_config;
		/* build wan i/f list based on nvram variable "wandevs" */
		len = sizeof(wan_ifnames);
		if (!build_ifnames(wandevs, wan_ifnames, &len) && len) {
			dyna[3].value = wan_ifnames;
			foreach (wan_ifname, wan_ifnames, next) {
				dyna[2].value = wan_ifname;
				break;
			}
		}
		else
			ap = 1;
		linux_overrides = dyna;
	}
	/* override lan i/f name list and wan i/f name list with default values */
	else 
#endif

	{
canned_config:
		boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
		if (boardflags & BFL_ENETVLAN)
			linux_overrides = vlan;
		else
			linux_overrides = generic;

		/* override the above linux_overrides with a different table */
		LINUX_OVERRIDES();
	}

	/* Restore defaults */
	for (t = router_defaults; t->name; t++) {
		if (restore_defaults || !nvram_get(t->name)) {
			for (u = linux_overrides; u && u->name; u++) {
				if (!strcmp(t->name, u->name)) {
					nvram_set(u->name, u->value);
					break;
				}
			}
			if (!u || !u->name)
				nvram_set(t->name, t->value);
		}
	}

	/* Force to AP */
	if (ap)
		nvram_set("router_disable", "1");

#ifdef REMOVE
	/* Always set OS defaults */
	nvram_set("os_name", "linux");
	nvram_set("os_version", EPI_ROUTER_VERSION_STR);
	nvram_set("os_date", __DATE__);
#endif

	nvram_set("is_modified", "0");

	/* Commit values */
	if (restore_defaults) {
		EXTRA_RESTORE_DEFAULTS();
		nvram_commit();
		cprintf("done\n");
	}
}
#endif

static void
set_wan0_vars(void)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	
	/* check if there are any connections configured */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_get(strcat_r(prefix, "unit", tmp)))
			break;
	}
	/* automatically configure wan0_ if no connections found */
	if (unit >= MAX_NVPARSE) {
		struct nvram_tuple *t;
		char *v;

		/* Write through to wan0_ variable set */
		snprintf(prefix, sizeof(prefix), "wan%d_", 0);
		for (t = router_defaults; t->name; t ++) {
			if (!strncmp(t->name, "wan_", 4)) {
				if (nvram_get(strcat_r(prefix, &t->name[4], tmp)))
					continue;
				v = nvram_get(t->name);
				nvram_set(tmp, v ? v : t->value);
			}
		}
		nvram_set(strcat_r(prefix, "unit", tmp), "0");
		nvram_set(strcat_r(prefix, "desc", tmp), "Default Connection");
		nvram_set(strcat_r(prefix, "primary", tmp), "1");
	}
}

static int noconsole = 0;

static void
sysinit(void)
{
	char buf[PATH_MAX];
	struct utsname name;
	struct stat tmp_stat;
	time_t tm = 0;

	/* /proc */
	mount("proc", "/proc", "proc", MS_MGC_VAL, NULL);

	/* /tmp */
	mount("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);

	/* /var */
	mkdir("/tmp/var", 0777);
	mkdir("/var/lock", 0777);
	mkdir("/var/log", 0777);
	mkdir("/var/run", 0777);
	mkdir("/var/tmp", 0777);

	/* Setup console */
	if (console_init())
		noconsole = 1;
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));

	/* Modules */
	uname(&name);
	snprintf(buf, sizeof(buf), "/lib/modules/%s", name.release);
	if (stat("/proc/modules", &tmp_stat) == 0 &&
	    stat(buf, &tmp_stat) == 0) {
		char module[80], *modules, *next;
		modules = nvram_get("kernel_mods") ? : "et robo il wl";
		foreach(module, modules, next)
			eval("insmod", module);
	}

#ifdef ASUS_EXT
	//eval("insmod", "/lib/modules/2.4.20/splink_led.o");
	eval("insmod", "/lib/modules/unimac.o");
	eval("insmod", "/lib/modules/mvwlan.o");
	eval("insmod", "/lib/modules/nvram_mod.o");
	eval("insmod", "/lib/modules/gpio.o");
	symlink("/sbin/rc", "/tmp/ddnsclient");
#endif

	/* Set a sane date */
	stime(&tm);

	dprintf("done\n");
}

/* States */
enum {
	RESTART,
	STOP,
	START,
	TIMER,
	IDLE,
	SERVICE
};
static int state = START;
static int signalled = -1;


/* Signal handling */
static void
rc_signal(int sig)
{
	if (state == IDLE) {	
		if (sig == SIGHUP) {
			dprintf("signalling RESTART\n");
			signalled = RESTART;
		}
		else if (sig == SIGUSR2) {
			dprintf("signalling START\n");
			signalled = START;
		}
		else if (sig == SIGINT) {
			dprintf("signalling STOP\n");
			signalled = STOP;
		}
		else if (sig == SIGALRM) {
			dprintf("signalling TIMER\n");
			signalled = TIMER;
		}
		else if (sig == SIGUSR1) {
			dprintf("signalling USR1\n");
			signalled = SERVICE;
		}
	}
}

/* Timer procedure */
int
do_timer(void)
{
	int interval = atoi(nvram_safe_get("timer_interval"));
	time_t now;
	struct tm gm, local;
	struct timezone tz;

	dprintf("%d\n", interval);

#ifdef ASUS_EXT
	/* Update kernel timezone */
	setenv("TZ", nvram_safe_get("time_zone"), 1);
	time(&now);
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;
	settimeofday(NULL, &tz);
	return 0;
#endif
	if (interval == 0)
		return 0;

	/* Report stats */
	if (nvram_invmatch("stats_server", "")) {
		char *stats_argv[] = { "stats", nvram_get("stats_server"), NULL };
		_eval(stats_argv, NULL, 5, NULL);
	}

	/* Sync time */
	start_ntpc();

	/* Update kernel timezone */
	setenv("TZ", nvram_safe_get("time_zone"), 1);
	time(&now);
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;
	settimeofday(NULL, &tz);
	alarm(interval);
	return 0;
}

/* Main loop */
static void
main_loop(void)
{
	sigset_t sigset;
	pid_t shell_pid = 0;
	uint boardflags;
	
	/* Basic initialization */
	sysinit();

	/* Setup signal handlers */
	signal_init();
	signal(SIGHUP, rc_signal);
	signal(SIGUSR2, rc_signal);
	signal(SIGINT, rc_signal);
	signal(SIGALRM, rc_signal);
#ifdef ASUS_EXTENSION
	signal(SIGUSR1, rc_signal);
#endif

	sigemptyset(&sigset);

	/* Give user a chance to run a shell before bringing up the rest of the system */
	if (!noconsole)
		run_shell(1, 0);

#ifdef REMOVE
	/* Add vlan */
	boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
	if (boardflags & BFL_ENETVLAN)
		config_vlan();
#endif

	/* Add loopback */
	config_loopback();

	/* Convert deprecated variables */
	convert_deprecated();

	/* Restore defaults if necessary */
	restore_defaults();

#ifdef ASUS_EXT
	read_mfg_data();	
	convert_asus_values();
	start_devices();
	change_wanmode();
#endif
	/* Setup wan0 variables if necessary */
	set_wan0_vars();

	/* Loop forever */
	for (;;) {
		switch (state) {
		case SERVICE:
			dprintf("SERVICE\n");
			service_handle();
			state = IDLE;
			break;
		case RESTART:
			dprintf("RESTART\n");
			/* Fall through */
		case STOP:
			dprintf("STOP\n");
			RC7_STOP();
			stop_services();
			stop_wan();
			stop_lan();
			RC1_STOP();
			if (state == STOP) {
				state = IDLE;
				break;
			}
			/* Fall through */
		case START:
			dprintf("START\n");
			RC1_START();
			start_lan();
			start_services();
			start_wan();
			//start_nas("wan");
			RC7_START();
#ifdef ASUS_EXT
			start_misc();
#endif

			/* Fall through */
		case TIMER:
			dprintf("TIMER\n");
			do_timer();
			/* Fall through */
		case IDLE:
			dprintf("IDLE\n");
			state = IDLE;
			/* Wait for user input or state change */
			while (signalled == -1) {
				if (!noconsole && (!shell_pid || kill(shell_pid, 0) != 0))
					shell_pid = run_shell(0, 1);
				else
					sigsuspend(&sigset);
			}
			state = signalled;
			signalled = -1;
			break;
		default:
			dprintf("UNKNOWN\n");
			return;
		}
	}
}

int
main(int argc, char **argv)
{
	char *base = strrchr(argv[0], '/');
	
	base = base ? base + 1 : argv[0];

	/* init */
	if (strstr(base, "init")) {
		main_loop();
		return 0;
	}

	/* Set TZ for all rc programs */
	setenv("TZ", nvram_safe_get("time_zone"), 1);

	/* ppp */
	if (strstr(base, "ip-up"))
		return ipup_main(argc, argv);
	else if (strstr(base, "ip-down"))
		return ipdown_main(argc, argv);

	/* udhcpc [ deconfig bound renew ] */
	else if (strstr(base, "udhcpc"))
	{
		return udhcpc_main(argc, argv);
	}

	/* stats [ url ] */
	else if (strstr(base, "stats"))
		return http_stats(argv[1] ? : nvram_safe_get("stats_server"));

	/* erase [device] */
	else if (strstr(base, "erase")) {
		if (argv[1])
			return mtd_erase(argv[1]);
		else {
			fprintf(stderr, "usage: erase [device]\n");
			return EINVAL;
		}
	}

	/* write [path] [device] */
	else if (strstr(base, "write")) {
		if (argc >= 3)
			return mtd_write(argv[1], argv[2]);
		else {
			fprintf(stderr, "usage: write [path] [device]\n");
			return EINVAL;
		}
	}

	/* hotplug [event] */
	else if (strstr(base, "hotplug")) {
		if (argc >= 2) {
			if (!strcmp(argv[1], "net"))
				return hotplug_net();
#ifdef ASUS_EXT
			else if(!strcmp(argv[1], "usb"))
				return hotplug_usb();
#endif
		} else {
			fprintf(stderr, "usage: hotplug [event]\n");
			return EINVAL;
		}
	}

#ifdef ASUS_EXT
	/* stop all service */
	else if (strstr(base, "stopservice")) {
		return stop_service_main();
	}
	/* ddns update ok */
	else if (strstr(base, "ddns_updated")) {
		return ddns_updated_main();
	}
	/* ddns update manually */
	else if (strstr(base, "ddnsclient")) {
		return start_ddns();
	}
	/* rgcfg */
	else if (strstr(base, "rgcfg")) {
		return rgcfg_main(argc, argv);
	}
#ifdef REMOVE
	/* send alarm */
	else if (strstr(base, "sendalarm")) {
		if (argc >= 1)
			return sendalarm_main(argc, argv);
		else {
			fprintf(stderr, "usage: sendalarm\n");
			return EINVAL;
		}
	}
#endif
	/* invoke watchdog */
	else if (strstr(base, "watchdog")) {
		return(watchdog_main());
	}
#ifdef REMOVE
	/* remove webcam module */
	else if (strstr(base, "rmwebcam")) {
		if (argc >= 2)
			return (remove_webcam_main(atoi(argv[1])));
		else return EINVAL;
	}
	/* remove usbstorage module */
	else if (strstr(base, "rmstorage")) {
		return (remove_storage_main());
	}
#endif
	/* run ntp client */
	else if (strstr(base, "ntp")) {
		return (ntp_main());
	}
	/* write srom */
	else if (strstr(base, "wsrom")) 
	{
		do_timer();
		if (argc >= 4) 
			return wsrom_main(argv[1], atoi(argv[2]), atoi(argv[3]));
		else {
			fprintf(stderr, "usage: wsrom [dev] [position] [value in 4 bytge]\n");
			return EINVAL;
		}
	}
	/* udhcpc_ex [ deconfig bound renew ], for lan only */
	else if (strstr(base, "landhcpc"))
		return udhcpc_ex_main(argc, argv);
#endif

	/* rc [stop|start|restart ] */
	else if (strstr(base, "rc")) {
		if (argv[1]) {
			if (strncmp(argv[1], "start", 5) == 0)
				return kill(1, SIGUSR2);
			else if (strncmp(argv[1], "stop", 4) == 0)
				return kill(1, SIGINT);
			else if (strncmp(argv[1], "restart", 7) == 0)
				return kill(1, SIGHUP);
		} else {
			fprintf(stderr, "usage: rc [start|stop|restart]\n");
			return EINVAL;
		}
	}

	return EINVAL;
}
