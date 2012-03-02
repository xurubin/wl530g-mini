/*
 *
 *  State Transaction:
 *  0. STOP
 *  1. CONNECTING_PROFILE
 * 	a. scanning
 *  2. CONNECTING_ONE
 *  3. SCANNING
 *  4. CONNECTED
 *  5. BEING_AP
 * 
 *  CONNECTING_PROFILE --> CONNECTED --> SCANNING ---|
 *                     \-> BEING_AP -/               |                 
 *                                                   |
 *                     CONNECTED <- CONNECTING_ONE <--
 *  CONNECTED <- CONNECT_PROFILE <-/
 *  BEING_AP <-/
 *
 *  Environment
 *  1. Power on and connect to existed LAN
 *  2. Power on and connect to existed WLAN
 *  3. Power on and no LAN/WLAN is connected to 
 *
 */

/* 
 *
 *  State Transaction:
 *  0. STOP
 *  1. CONNECTING_PROFILE
 *  2. CONNECTING_ONE
 *  3. SCANNING
 *  4. CONNECTED
 *  5. BEING_AP
 * 
 *  CONNECTING_PROFILE --> CONNECTED --> SCANNING ---|
 *                     \-> BEING_AP -/               |                 
 *                                                   |
 *                     CONNECTED <- CONNECTING_ONE <--
 *  CONNECTED <- CONNECT_PROFILE <-/
 *  BEING_AP <-/
 *
 *  Environment
 *  1. Power on and connect to existed LAN
 *  2. Power on and connect to existed WLAN
 *  3. Power on and no LAN/WLAN is connected to 
 *
 */
                                                
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlutils.h>
#include "ethtool-util.h"
#include "iboxcom.h"

#define SIOCETHTOOL 0x8946
#define MAX_PROFILE_NUMBER 32
#define MAX_SITE_NUMBER 24

enum 
{
	STA_STATE_STOP = 0,
	STA_STATE_CONNECTING_PROFILE,
	STA_STATE_CONNECTING_ONE,
	STA_STATE_SCANNING_PROFILE,
	STA_STATE_SCANNING,
	STA_STATE_CONNECTED,
	STA_STATE_BEING_AP
} STA_STATE;

#define STA_ISTIMEOUT(t) ((unsigned long)(now-sta_timer)>=t)
#define STA_STATE_TIMEOUT_CONNECTING 10 // 10 sec
#define STA_STATE_TIMEOUT_SCANNING 30 //30 sec
#define STA_STATE_TIMEOUT_SCANNING_PROFILE 1 //1 sec

int sta_state = STA_STATE_STOP;
int sta_profile = 0;
int sta_scan = 0;
time_t sta_timer = 0;
time_t now;
	
char pdubuf_res[INFO_PDU_LENGTH];
unsigned char mac[6] = { 0x00, 0x0c, 0x6e, 0xbd, 0xf3, 0xc5};

PKT_GET_INFO_STA stainfo_g;
int sites_g_count=0;
SITES sites_g[MAX_SITE_NUMBER];
int profiles_g_count=0;
PROFILES profiles_g[MAX_PROFILE_NUMBER];
int scan_g_type;
int scan_g_mode;

#ifdef WCLIENT
int is_lan_connected(char *lan_if)
{
	int fd, err, ret;
	struct ifreq ifr;
	struct ethtool_cmd ecmd;

	/* check if udhcpc get IP from DHCP server */
	printf("check lan\n");	
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, lan_if);
	fd=socket(AF_INET, SOCK_DGRAM, 0);
	if (fd<0) return 0;
	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err==0)
	{
		printf("lan connect: %x\n", ecmd.speed);
		ret = ecmd.speed;
	}
	else ret = 0;

	close(fd);
	return ret;
}

void sta_info_init(void)
{
	int i;

     	stainfo_g.mode = STATION_MODE_INFRA;
	stainfo_g.chan = 1;
	strcpy(stainfo_g.ssid, "WirelessHD");
	stainfo_g.rate = 0;
	stainfo_g.wep = 0;
	stainfo_g.wepkeylen = 0;
	stainfo_g.wepkeyactive = 0;
	stainfo_g.sharedkeyauth = 0;
	stainfo_g.brgmacclone = 0;
	stainfo_g.preamble = 0;	
	stainfo_g.profileCount = 0;

	// initial sites info
	wl_read_profile();
	stainfo_g.profileCount = profiles_g_count;

	/* Check if client mode is disabled */
	if (nvram_invmatch("wlp_clientmode_x", "0") && profiles_g_count!=0)
	{	
		//int pref = atoi(nvram_safe_get("wlp_pref_x"));
		//if (pref < profiles_g_count)
		//	sta_start_connecting_one(&profiles_g[pref]);
		//else sta_start_connecting_profile(0);
		if (nvram_match("wlp_clientmode_x", "2")) 
			nvram_set("wl0_mode", "wet");
		else nvram_set("wl0_mode", "sta");
		eval("wlconfig", "eth2");
		sta_start_scanning_profile(0);
	}
	else if (nvram_match("wlp_beap_x", "1")) sta_start_being_ap();
	else sta_stop();
}	

void sta_connect(PROFILES *profile)
{
	int i;

	printf("ssid: %s\n", profile->ssid);
     	stainfo_g.mode = profile->mode;
	stainfo_g.chan = profile->chan;
	strncpy(stainfo_g.ssid, profile->ssid, 32);
	stainfo_g.rate = profile->rate;
	stainfo_g.wep = profile->wep;
	stainfo_g.wepkeylen = profile->wepkeylen;
	stainfo_g.wepkeyactive = profile->wepkeyactive;
	stainfo_g.sharedkeyauth = profile->sharedkeyauth;
	stainfo_g.brgmacclone = profile->brgmacclone;
	stainfo_g.preamble = profile->preamble;	

	wl_connect(profile);
}

void sta_stop()
{
	eval("wl", "radio", "off");
	sta_state = STA_STATE_STOP;
}

void sta_start_scanning()
{
	
	if (sta_state==STA_STATE_BEING_AP)
	{			
		stop_dhcpd();
		if (nvram_match("wlp_clientmode_x", "2")) 
			nvram_set("wl0_mode", "wet");
		else nvram_set("wl0_mode", "sta");
		eval("wlconfig", "eth2");
	}

	wl_scan();
	time(&sta_timer);
	sta_state = STA_STATE_SCANNING;
	
	sta_status_report(sta_state, 0);
}


void sta_start_connecting_profile(int is_next)
{	
	if (is_next) sta_profile++;
	else sta_profile = 0;

	if (profiles_g_count == 0 || sta_profile>=profiles_g_count)
	{
		if (nvram_match("wlp_beap_x", "1"))
		{
			sta_start_being_ap();
			return;			
		}
		sta_profile=0;
	}
	
	printf("connect to profile : %d\n", sta_profile);
	sta_connect(&profiles_g[sta_profile]);
	time(&sta_timer);
	sta_state = STA_STATE_CONNECTING_PROFILE;

	sta_status_report(sta_state, 0);
}


void sta_start_scanning_profile(int is_next)
{
	int i, j, cand=-1, cand_rssi=-248, pref;
	char ssid[33], ssids[33];
	
	ssid[32] = ssids[32] = 0;

	if (is_next) sta_scan++;
	else sta_scan = 0;

	pref = atoi(nvram_safe_get("wlp_pref_x"));
		
	// scan each entry in profiles actively
	for(i=0;i<profiles_g_count;i++)
	{
		memcpy(ssid, profiles_g[i].ssid, 32);
		ssid[32] = 0;
		printf("find: %s %d %d\n", ssid, profiles_g[i].wep, profiles_g[i].wepkeylen);
		eval("wl", "scan", ssid);
		sleep(1);
		wl_scan_results();

		for(j=0;j<sites_g_count;j++)
		{
			memcpy(ssids, sites_g[j].SSID, 32);

			printf("scaned: %s %d\n", ssids, sites_g[j].RSSI);
			
			if (!strcmp(ssid, ssids) && 
				sites_g[j].RSSI>cand_rssi &&
				cand != pref)
			{
				cand = i;
				cand_rssi = sites_g[j].RSSI;
				break;
			}
		}
	}	

	if (cand==-1)
	{
		for(i=0;i<profiles_g_count;i++)
		{
			if (profiles_g[i].mode == STATION_MODE_ADHOC)
			{
				cand = i;
				break;
			}	
		}
	}

	if (cand!=-1)
	{	
		printf("connect to one : %d\n", cand);	
		sta_start_connecting_one(&profiles_g[cand]);
	}
	else if (sta_scan<3) // totally, 3 chances to try
	{
		printf("try : %d\n", sta_scan);	
		time(&sta_timer);
		sta_state = STA_STATE_SCANNING_PROFILE;
		sta_status_report(sta_state, 0);
	}
	else if (nvram_match("wlp_beap_x", "1")) sta_start_being_ap();
	else sta_start_scanning();
}



void sta_start_connecting_one(PROFILES *profile)
{
	sta_connect(profile);
	time(&sta_timer);
	sta_state = STA_STATE_CONNECTING_ONE;


	sta_status_report(sta_state, 0);
}

void sta_start_connected()
{
	// DHCP renew;
	FILE *fp;
	int pid;

	fp = fopen("/var/run/udhcpc.pid", "r");

	if (fp!=NULL)
	{
		fscanf(fp, "%d", &pid);
		printf("release to %d\n", pid);
		kill(SIGUSR2, pid);
		sleep(1);
		printf("renew to %d\n", pid);
		kill(SIGUSR1, pid);
		fclose(fp);
	}
	sta_state = STA_STATE_CONNECTED;

	sta_status_report(sta_state, 0);
}

void sta_start_being_ap()
{
	PROFILES profile;
	pid_t pid;
	char *nas_cmd[]={"nas", "/tmp/nas.conf", NULL};

	//printf("set as a AP");
	/* Become an AP with SSID = Shared Name */
	memset(&profile, 0, sizeof(profile));
	profile.mode = STATION_MODE_AP;
	strcpy(profile.ssid, nvram_safe_get("wl0_ssid")); 

	//printf("ssid: %s\n", profile->ssid);
     	stainfo_g.mode = profile.mode;
	stainfo_g.chan = profile.chan;
	strncpy(stainfo_g.ssid, profile.ssid, 32);
	stainfo_g.rate = profile.rate;
	stainfo_g.wep = profile.wep;
	stainfo_g.wepkeylen = profile.wepkeylen;
	stainfo_g.wepkeyactive = profile.wepkeyactive;
	stainfo_g.sharedkeyauth = profile.sharedkeyauth;
	stainfo_g.brgmacclone = profile.brgmacclone;
	stainfo_g.preamble = profile.preamble;	

	if (nvram_match("wl_wdsmode_x", "1")) nvram_set("wl0_mode", "wds");
	else nvram_set("wl0_mode", "ap");
	//eval("wlconf", "eth2", "up");
	eval("wlconfig", "eth2");
	eval("killall", "nas");
	_eval(nas_cmd, NULL, 0, &pid);

	stop_dhcpd();

	if (!is_dhcpd_exist()) 
	{
		printf("start your own dhcp server\n");
		start_dhcpd();
	}
	sta_state = STA_STATE_BEING_AP;
	sta_status_report(sta_state, 0);
}

int sta_status()
{
	if (sta_state==STA_STATE_BEING_AP) return 0;
	else return(wl_status());
}

void sta_status_check()
{
	time(&now);
	
	if (sta_state==STA_STATE_STOP)
	{
		// do nothing
	}
	else if(sta_state==STA_STATE_CONNECTING_PROFILE)
	{
		if (wl_status()) // have connected to one AP
		{
			sta_start_connected();
		}
		else if (STA_ISTIMEOUT(STA_STATE_TIMEOUT_CONNECTING))
		{
			sta_start_connecting_profile(1);
		}
	}
	else if (sta_state==STA_STATE_CONNECTING_ONE)
	{
		if (wl_status())
		{
			sta_start_connected();
		}
		else if (STA_ISTIMEOUT(STA_STATE_TIMEOUT_CONNECTING))
		{
			if (nvram_match("wlp_beap_x", "1"))
			{
				sta_start_being_ap();
				return;			
			}
			else sta_start_scanning_profile(0);
		}
	}
	else if (sta_state==STA_STATE_SCANNING)
	{
		if (STA_ISTIMEOUT(STA_STATE_TIMEOUT_SCANNING))
		{
			sta_start_scanning_profile(0);
		}
	}
	else if (sta_state==STA_STATE_SCANNING_PROFILE)
	{
		if (STA_ISTIMEOUT(STA_STATE_TIMEOUT_SCANNING))
		{
			sta_start_scanning_profile(1); //Try again
		}
	}
	else if (sta_state==STA_STATE_CONNECTED)
	{
		if (!wl_status())
		{
			sta_start_scanning_profile(0);
		}
	}
	else if (sta_state==STA_STATE_BEING_AP)
	{
		// do nothing now
	}
}

void sta_status_report(int stastate, int refresh)
{

	#define MAXBUF 8192
	FILE *fp;	
	int		result, i;
	int		cmd;
	unsigned char	buf[MAXBUF];

	//printf("report\n");

	if (refresh)
	{
		if(sta_state==STA_STATE_BEING_AP)
			stastate=STA_STATE_BEING_AP;
		else return;
	}

	fp=fopen("/tmp/wlan11b.log", "w+");

	if (fp==NULL) 
	{
		printf("can not open file\n");
		return;
	}

	if (stastate==STA_STATE_STOP)
	{
		fprintf(fp, "Wireless Function is disabled\n");
	}
	else if (stastate==STA_STATE_BEING_AP)
	{
		char *auth[] = {"Open System or Shared Key", 
				"Shared Key",
				"WPA-PSK",
				"WPA",
				"Radius"};
		char *wep[] = {"None", "WEP-64", "WEP-128"};
		char *wpa[] = {"TKIP", "AES"};
		int authidx, encidx;


		fprintf(fp, "Mode: AP\n");
		fprintf(fp, "SSID: %s\n", nvram_safe_get("wl_ssid"));

		authidx = atoi(nvram_safe_get("wl_authmode_x"));
		encidx = atoi(nvram_safe_get("wl_weptype_x"));

		if (authidx>=5) authidx=0;
		if (encidx>=3) encidx=0;

		fprintf(fp, "Authentication: %s\n", auth[authidx]);

		if (authidx==1)
		{	
			if (authidx<0||encidx>1) encidx=0;
			fprintf(fp, "Encryption: %s\n", wep[encidx+1]);
		}	
		else if (authidx==2)
		{
			if (authidx<0||encidx>1) encidx=0;
			fprintf(fp, "Encryption: %s\n", wpa[encidx]);
		}
		else if (authidx==3) 
		{
			if (authidx<0||encidx>2) encidx=0;
			fprintf(fp, "Encryption: %s\n", wep[encidx]);
		}
		else
		{
			if (authidx<0||encidx>2) encidx=0;
			fprintf(fp, "Encryption: %s\n", wep[encidx]);
		}

		fprintf(fp, "Association List\n");
		fprintf(fp, "------------------------------------\n");

		cmd = WLC_GET_ASSOCLIST;
		*(unsigned int *)buf = 1000;		//query 1000 MAC address

		if ( (result = wl_ioctl("eth2", cmd, buf, MAXBUF)) != 0 ){
			printf("Fail: cmd=0x%02x (%d)\n", cmd, result);
		}
		else 
		{
			unsigned int	count = *(unsigned int *)buf;
			unsigned int 	j;

			j=0;

			for ( i=0; i< count*6; i++ ) 
			{
				if ((i%6)==0)
				{
					fprintf(fp, "STA%2x:", j++);
				}

				fprintf(fp, "%02x", i, buf[sizeof(unsigned int)+i]);
				if ( (i % 6)==5 )
					fprintf(fp, "\n");
				else
					fprintf(fp, ":");
			}
		}

	}
	else
	{
		if (stastate==STA_STATE_CONNECTED || 
			stastate==STA_STATE_CONNECTING_ONE ||
			stastate==STA_STATE_CONNECTING_PROFILE)
		{
			if (stainfo_g.mode==0)
			{	
				fprintf(fp, "Mode: Infra Structure(STA)\n");
			}
			else
			{
				fprintf(fp, "Mode: Adhoc(STA)\n");
			}

			if (stastate == STA_STATE_CONNECTED)
			 	fprintf(fp, "Status: Associated\n");
			else fprintf(fp, "Status: Connecting\n");
			
			fprintf(fp, "SSID  : %s\n", stainfo_g.ssid); 

			if (stainfo_g.sharedkeyauth)
			{
				fprintf(fp, "Authenication: Shared Key\n");
			}
			else
			{
				fprintf(fp, "Authenication: Open System\n");
			}

			if (stainfo_g.wep == STA_ENCRYPTION_ENABLE)
			{
				if (stainfo_g.wepkeylen==STA_ENCRYPTION_TYPE_WEP128)
				{	
					fprintf(fp, "Encryption: WEP 128 bits\n");
				}
				else
				{
					fprintf(fp, "Encryption: WEP 64 bits\n");
				}
			}
			else
			{	
				fprintf(fp, "Encryption: None\n");
			}
		}
		else if (sta_state==STA_STATE_SCANNING ||
			  sta_state==STA_STATE_SCANNING_PROFILE)
		{
			fprintf(fp, "Mode: Station\n");
			fprintf(fp, "Status: Scanning\n");
		}
	}
	fclose(fp);
}
#endif
extern char ssid_g[];
extern char netmask_g[];
extern char productid_g[];
extern char firmver_g[];

char *processPacket(int sockfd, char *pdubuf)
{
    IBOX_COMM_PKT_HDR	*phdr;
    IBOX_COMM_PKT_HDR_EX *phdr_ex;
    IBOX_COMM_PKT_RES_EX *phdr_res;
    PKT_GET_INFO *ginfo;
    PKT_GET_INFO_STA *stainfo;
    PKT_GET_INFO_EX1 *cmd;					
    PKT_GET_INFO_EX1 *res;
    PKT_GET_INFO_SITES *res_sites;
    PKT_GET_INFO_PROFILE *cmd_profiles, *res_profiles;

    int i;

    phdr = (IBOX_COMM_PKT_HDR *)pdubuf;  
    phdr_res = (IBOX_COMM_PKT_RES_EX *)pdubuf_res;
    
    //printf("Get: %x %x %x\n", phdr->ServiceID, phdr->PacketType, phdr->OpCode);
    
    if (phdr->ServiceID==NET_SERVICE_ID_IBOX_INFO &&
        phdr->PacketType==NET_PACKET_TYPE_CMD)
    {	    
	if (phdr->OpCode!=NET_CMD_ID_GETINFO &&
	    phdr_res->OpCode==phdr->OpCode &&
	    phdr_res->Info==phdr->Info)
	{	
		// if transaction id is equal to the transaction id of the last response message, just re-send message again;
		return pdubuf_res;
	}	
	
	phdr_res->ServiceID=NET_SERVICE_ID_IBOX_INFO;
	phdr_res->PacketType=NET_PACKET_TYPE_RES;
	phdr_res->OpCode=phdr->OpCode;

	if (phdr->OpCode!=NET_CMD_ID_GETINFO)
	{
		phdr_ex = (IBOX_COMM_PKT_HDR_EX *)pdubuf;	
		
		// Check Mac Address
		if (memcpy(phdr_ex->MacAddress, mac, 6)==0)
		{
			//printf("Mac Error %2x%2x%2x%2x%2x%2x\n",
			//	(unsigned char)phdr_ex->MacAddress[0],
			//	(unsigned char)phdr_ex->MacAddress[1],
			//	(unsigned char)phdr_ex->MacAddress[2],
			//	(unsigned char)phdr_ex->MacAddress[3],
			//	(unsigned char)phdr_ex->MacAddress[4],
			//	(unsigned char)phdr_ex->MacAddress[5]
			//	);
			return NULL;
		}
		
		// Check Password
		//if (strcmp(phdr_ex->Password, "admin")!=0)
		//{
		//	phdr_res->OpCode = phdr->OpCode | NET_RES_ERR_PASSWORD;
		//	printf("Password Error %s\n", phdr_ex->Password);	
		//	return NULL;
		//}
		phdr_res->Info = phdr_ex->Info;
		memcpy(phdr_res->MacAddress, phdr_ex->MacAddress, 6);
	}

	switch(phdr->OpCode)
	{
		case NET_CMD_ID_GETINFO:		 		  	
		     ginfo=(PKT_GET_INFO *)(pdubuf_res+sizeof(IBOX_COMM_PKT_RES));
		     memset(ginfo, 0, sizeof(ginfo));
#ifdef PRNINFO
    		     readPrnID(ginfo->PrinterInfo);
#else
		     memset(ginfo->PrinterInfo, 0, sizeof(ginfo->PrinterInfo));
#endif	
   		     strcpy(ginfo->SSID, ssid_g);
		     strcpy(ginfo->NetMask, netmask_g);
		     strcpy(ginfo->ProductID, productid_g);
		     strcpy(ginfo->FirmwareVersion, firmver_g);
		     memcpy(ginfo->MacAddress, mac, 6);
#ifdef WCLIENT
		     ginfo->OperationMode = OPERATION_MODE_WB;
		     ginfo->Regulation = 0xff;
#endif

		     sendInfo(sockfd, pdubuf_res);
		     return pdubuf_res;		     	
#ifdef WCLIENT	
		case NET_CMD_ID_GETINFO_EX:
		     cmd = (PKT_GET_INFO_EX1 *)(pdubuf+sizeof(IBOX_COMM_PKT_HDR_EX));	
		     res = (PKT_GET_INFO_EX1 *)(pdubuf_res+sizeof(IBOX_COMM_PKT_RES_EX));
		     stainfo = (PKT_GET_INFO_STA *)(pdubuf_res+sizeof(IBOX_COMM_PKT_RES_EX)+sizeof(PKT_GET_INFO_EX1));
				
		     if (cmd->FieldID!=FIELD_GENERAL_CURRENT_STA) return NULL;
		     res->FieldCount=1;
		     res->FieldID=FIELD_GENERAL_CURRENT_STA;

		     memcpy(stainfo, &stainfo_g, sizeof(stainfo_g));
		     stainfo->connectionStatus = sta_status();
		     //printf("GetSTA: %s\n", stainfo->ssid);
			
		     sendInfo(sockfd, pdubuf_res);
		     return pdubuf_res;
		case NET_CMD_ID_SETINFO:				
		     cmd=(PKT_GET_INFO_EX1 *)(pdubuf+sizeof(IBOX_COMM_PKT_HDR_EX));			
		     stainfo = (PKT_GET_INFO_STA *)(pdubuf+sizeof(IBOX_COMM_PKT_HDR_EX)+sizeof(PKT_GET_INFO_EX1));
		     //printf("SSID: %s\n", stainfo->ssid);
		     sendInfo(sockfd, pdubuf_res);
		     return pdubuf_res;
		case NET_CMD_ID_GETINFO_SITES:				
		     cmd=(PKT_GET_INFO_EX1 *)(pdubuf+sizeof(IBOX_COMM_PKT_HDR_EX));	
		     res_sites = (PKT_GET_INFO_SITES *)(pdubuf_res+sizeof(IBOX_COMM_PKT_RES_EX));
		     sta_start_scanning();
		     //sta_count_reset();
		     sleep(2);	
		     wl_scan_results();

		     printf("Get INFO %d\n", sites_g_count);

		     res_sites->Count = sites_g_count;
	
		     for(i=0;i<res_sites->Count&&i<MAX_SITE_NUMBER;i++)
		     {
			 memcpy(&res_sites->Sites[i%8], &sites_g[i], sizeof(SITES));				 
			 if (i%8==0&&i!=0)
			 {
			 	res_sites->Index = i/8;
		     		sendInfo(sockfd, pdubuf_res);
			 } 	
		     }
		     if (i%8!=0)
		     {
			 res_sites->Index = i/8;
		     	 sendInfo(sockfd, pdubuf_res);
			 printf("Send:%d %d\n", res_sites->Index, res_sites->Count);
		     }	
		     return pdubuf_res;
		case NET_CMD_ID_GETINFO_PROF:		     
		     cmd_profiles = (PKT_GET_INFO_PROFILE *)(pdubuf+sizeof(IBOX_COMM_PKT_HDR_EX));	
		     res_profiles = (PKT_GET_INFO_PROFILE *)(pdubuf_res+sizeof(IBOX_COMM_PKT_RES_EX));
		     res_profiles->StartIndex = cmd_profiles->StartIndex;
		     res_profiles->Count = cmd_profiles->Count;
		     for(i=0;i<cmd_profiles->Count;i++)
		     {
			 memcpy(&res_profiles->p.Profiles[i], &profiles_g[cmd_profiles->StartIndex+i], sizeof(PROFILES));
		     }
		     sendInfo(sockfd, pdubuf_res);
		     return pdubuf_res;
		case NET_CMD_ID_SETINFO_PROF:
		     cmd_profiles =(PKT_GET_INFO_PROFILE *)(pdubuf+sizeof(IBOX_COMM_PKT_HDR_EX));	
		     res_profiles = (PKT_GET_INFO_SITES *)(pdubuf_res+sizeof(IBOX_COMM_PKT_RES_EX));

		     if (cmd_profiles->Count==0)
		     {
			 if (cmd_profiles->StartIndex == 0xff) // Save to file
			 {
			 }
			 else 
			 {
				 scan_g_type = cmd_profiles->p.ProfileControl.ButtonType;
				 scan_g_mode = cmd_profiles->p.ProfileControl.ButtonMode;
				 stainfo_g.profileCount = cmd_profiles->p.ProfileControl.ProfileCount;

				 sta_start_connecting_one(&profiles_g[cmd_profiles->StartIndex]);
			         //printf("set: %d %d\n", cmd_profiles->StartIndex, stainfo_g.profileCount);
				
				 profiles_g_count = stainfo_g.profileCount;
				 wl_write_profile();				 
			 }
		     }		
		     else
		     {	 	
		     	 for(i=0;i<cmd_profiles->Count;i++)
		     	 {
				memcpy(&profiles_g[cmd_profiles->StartIndex + i], &cmd_profiles->p.Profiles[i], sizeof(PROFILES));
		     	 }
		     }	 	
		     sendInfo(sockfd, pdubuf_res);
		     return pdubuf_res;
#endif
		case NET_CMD_ID_MANU_CMD:
		{
		     #define MAXSYSCMD 64
		     #define MAXPARA 6
		     #define SIZEOFHDR 46
		     #define SIZEOFRES 14
		     char cmdstr[MAXSYSCMD];
		     char *cmdargv[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL};
		     char *cmdptr;
		     int cmdcnt;		 
		     PKT_SYSCMD *syscmd;
		     PKT_SYSCMD_RES *syscmd_res;
		     FILE *fp;

		     syscmd = (PKT_SYSCMD *)(pdubuf+SIZEOFHDR);
		     syscmd_res = (PKT_SYSCMD_RES *)(pdubuf_res+SIZEOFRES);

		     if (syscmd->len>=MAXSYSCMD) syscmd->len=MAXSYSCMD;
		     syscmd->cmd[syscmd->len]=0;
		     syscmd->len=strlen(syscmd->cmd);

		     //printf("system cmd: %d %d %d %d %s\n", sizeof(IBOX_COMM_PKT_RES_EX), sizeof(unsigned short), sizeof(IBOX_COMM_PKT_HDR_EX) + syscmd->len, syscmd->cmd);
		      strcpy(cmdstr, syscmd->cmd);	
		      cmdptr = cmdstr;
		      while (strsep(&cmdptr, " "));

		      cmdcnt=0;
		      /* Set token values */
		      for (cmdptr = cmdstr; cmdptr < &cmdstr[syscmd->len] && *cmdptr && cmdcnt<MAXPARA; cmdptr += strlen(cmdptr) + 1) 
		      {
		           cmdargv[cmdcnt++] = cmdptr;
		      }	 	

		      if (cmdcnt==0) syscmd_res->len=0;
		      else
		      {				
		      	_eval(cmdargv, ">/tmp/syscmd.out", 0, NULL);

		      	fp = fopen("/tmp/syscmd.out", "r");

		      	if (fp!=NULL)
		      	{
			 	syscmd_res->len = fread(syscmd_res->res, 1, sizeof(syscmd_res->res), fp);
			 	fclose(fp);
		      	}
		      	else syscmd_res->len=0;
		      } 	

		      //printf("res : %d %s\n", syscmd_res->len, syscmd_res->res);
		      sendInfo(sockfd, pdubuf_res);
	 	      return pdubuf_res;
		}
		default:
			return NULL;	
	}
    }
    return NULL;
}
