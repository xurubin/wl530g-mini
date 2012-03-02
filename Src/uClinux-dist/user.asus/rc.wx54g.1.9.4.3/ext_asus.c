#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<signal.h>
#include<bcmnvram.h>
#include<shutils.h>
#include<netconf.h>

#ifdef REMOVE
long sync_interval=1;

int timesync(void)
{
	time_t now;
	struct tm tm;
	
	if (nvram_match("router_disable", "1")) return 0;
	
	if (sync_interval!=-1)
	{
		//printf("Sync Time: %d\n", sync_interval);	
        	sync_interval--;

	    	if (sync_interval==0)
		{
           		time(&now);
		   	memcpy(&tm, localtime(&now), sizeof(struct tm));
		   	if (tm.tm_year>100) // More than 2000 
		   	{	 
		      		sync_interval=86400/60;
			  	openlog("NTP client", 0, 0);
			  	syslog(0, "time is synchronized to %s", nvram_safe_get("9_x_NTPServer"));
			  	closelog();
		   	}	
		  	else sync_interval=1;
			
			stop_ntpc();
			start_ntpc();
		   	//printf("Sync time %d %d\n", sync_interval, tm.tm_year);
		}
	}	
}

enum 
{
	URLACTIVE=0,
	LANWANACTIVE,
	WANLANACTIVE,
	ACTIVEITEMS
} ACTIVE;
int CurrentActive[ACTIVEITEMS] = {0, 0, 0};

int timecheck_item(char *enable, char *datestr, char *timestr)
{
	char *activeDate, *activeTime;
	int current, active, activeTimeStart, activeTimeEnd, i;
	time_t now;
	struct tm *tm;

	time(&now);
	tm = localtime(&now);
	current = tm->tm_hour*60 + tm->tm_min;

	active=0;

	// Check if filter is enabled
	if (nvram_match(enable, "1"))
	{	
		activeDate=nvram_safe_get(datestr);
		activeTime=nvram_safe_get(timestr);

		activeTimeStart=((activeTime[0]-'0')*10+(activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
		
		activeTimeEnd=((activeTime[4]-'0')*10+(activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');
		
		//printf("Active Date: %s %d\n", activeDate, tm->tm_wday);
		//printf("Active Time: %d (%d-%d)\n", current, activeTimeStart, activeTimeEnd);
		if (activeTimeEnd<activeTimeStart) 
		{
			i = activeTimeEnd;
			activeTimeEnd = activeTimeStart;
			activeTimeStart = i;
		}
		
		if (activeDate[tm->tm_wday] == '1')
		{
			if (activeTimeStart<=activeTimeEnd &&
			     current>=activeTimeStart &&
			     current<=activeTimeEnd)
			{	
				active = 1;
			}	
			else 
			{
				active = 0;
			}			
		}		
	}
	return(active);
}

int timecheck(void)
{
	int activeNow, activeFlag;

	if (nvram_match("router_disable", "1")) return 0;
	
	activeFlag = 0;
	activeNow = timecheck_item("7_UrlFilterEnable", "7_URLActiveDate", "7_URLActiveTime");

	if (CurrentActive[URLACTIVE]!=activeNow)
	{
		CurrentActive[URLACTIVE] = activeNow;
		activeFlag = 1;
	}	


#ifdef REMOVE	
	activeNow = timecheck_item("7_WanLanFirewallEnable", "7_LanWanActiveDate", "7_LanWanActiveTime");

	if (CurrentActive[LANWANACTIVE]!=activeNow)
	{
		CurrentActive[LANWANACTIVE] = activeNow;
		activeFlag = 1;
	}	
	activeNow = timecheck_item("7_WanLanFirewallEnable", "7_WanLanActiveDate", "7_WanLanActiveTime");

	if (CurrentActive[WANLANACTIVE]!=activeNow)
	{
		CurrentActive[WANLANACTIVE] = activeNow;
		activeFlag = 1;
	}	
#endif

	if (activeFlag)
	{	
#ifdef REMOVE
		start_firewall_ex(nvram_safe_get("wan_ifname"), ""
				"br0", "", 
				CurrentActive[URLACTIVE],
				CurrentActive[LANWANACTIVE],
				CurrentActive[WANLANACTIVE]);
#endif
		//printf("Change dproxy setting\n");
		stop_dns();
		start_dns();
	}
	return 0;
}	
	
static char *http_cmd="http://127.0.0.1/";
static char *http_run="/init/httpserver_rerun";
char buf[256];

/* Sometimes, httpd becomes inaccessible, try to re-run it */
int httpcheck(void)
{
	if (!http_check(http_cmd, buf, sizeof(buf), 0))
	{
		stop_httpd();
		start_httpd();
	}
	return 0;
}
#endif

/* LED/Reset to Default process */
/* Insert module 		*/
/* mknod /dev/led0 		*/
/* mknod /dev/led3 		*/
/* Above job is done by preprocess */
/* Send PID of rc to LED module */

int 
set_pid(int pid)	
{	
	FILE *fp;

	if ((fp=fopen("/dev/led4", "a+")))
	{
		fprintf(fp, "%d ", getpid());
		fclose(fp);
		return 0;
	}		
	else 
	{
		return -1;
	}	
}

int
control_led(int led, int flag)
{
	FILE *fp;
	char ledname[24], buf[3];

	sprintf(ledname, "/dev/led%d", led);
	if ((fp=fopen(ledname, "w+"))==NULL) return -1;
	fprintf(fp, "%d", flag);
	fclose(fp);
}

int
start_led(void)
{
	FILE *fp;
	
	//printf("Send PID:%d to LED\n", getpid());
	set_pid(getpid());

	control_led(0, 1);
	control_led(1, 1);
	control_led(2, 1);
}	

void 
handle_led(int sig)
{
	if (sig==SIGUSR1)
	{
		eval("erase", "/dev/mtd2");
		//printf("Send PID:%d to LED again\n", getpid());
		set_pid(getpid());
	}	
}
