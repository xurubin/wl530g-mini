 
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <stdarg.h>
#include <wlioctl.h>

#if 0 
#define DEBUG printf
#else
#define DEBUG(format, args...) 
#endif

#define BCM47XX_SOFTWARE_RESET  0x40		/* GPIO 6 */
#define RESET_WAIT		3		/* seconds */
#define RESET_WAIT_COUNT	RESET_WAIT * 10 /* 10 times a second */

#define NORMAL_PERIOD		1		/* second */
#define URGENT_PERIOD		100 * 1000	/* microsecond */	
						/* 1/10 second */
#define STACHECK_PERIOD_CONNECT 60/5		/* 30 seconds */
#define STACHECK_PERIOD_DISCONNECT 5/5		/* 5 seconds */

#define GPIO0 0x0001
#define GPIO1 0x0002
#define GPIO2 0x0004
#define GPIO3 0x0008
#define GPIO4 0x0010
#define GPIO5 0x0020
#define GPIO6 0x0040
#define GPIO7 0x0080
#define GPIO8 0x0100
#define GPIO9 0x0200
#define GPIO10 0x0400
#define GPIO11 0x0800
#define GPIO12 0x1000
#define GPIONONE 0x0
#define GPIOMASK 0xffff

#define LED_ON 	0
#define LED_OFF 1
#define LED_BLINK 2

unsigned int LED_POWER=GPIO12;
unsigned int LED_WLAN=GPIO0;
unsigned int LED_WAN=GPIO1;
unsigned int BTN_RESET=GPIO10;

struct itimerval itv; 
int watchdog_period=0;
static int btn_pressed=0;
static int btn_count = 0;
long sync_interval=-1; // every 30 seconds a unit
int sync_flag=0;
long timestamp_g=0;
int stacheck_interval=-1;
int blinkflag=0;
int wanstate=0;
int cablestate=0;

void gpio_write(char *dev, unsigned int gpio, unsigned int flag)
{
	unsigned int val;
	FILE *fp;	

	if (gpio==GPIONONE) return;

	val = gpio_read(dev, GPIOMASK);

	fp=fopen(dev, "w");
	if (fp!=NULL)
	{
		if (flag) val|=gpio;
		else val&=~gpio;

		fwrite(&val, 4, 1, fp);
		fclose(fp);
	}	
}

int gpio_read(char *dev, unsigned int gpio)
{
	unsigned int val;
	FILE *fp;

	if (gpio==GPIONONE) return 0;

	fp=fopen(dev, "r");

	if (fp!=NULL)
	{
		fread(&val, 4, 1, fp);
		fclose(fp);
		return(val&gpio);
	}
	return 0;
}


/* Functions used to control led and button */
gpio_init()
{
	unsigned int outen;
	char *productid;

	// set different GPIO for SnapAP
	productid = nvram_safe_get("productid");

	if (strstr(productid, "WL530"))
	{
		// WL530's GPIO 
		LED_POWER=GPIO1;
		LED_WLAN=GPIO0;
		LED_WAN=GPIO12;
		BTN_RESET=GPIO10;
	}
	else if (strstr(productid, "WL520"))
	{
		LED_POWER=GPIO6;
		LED_WLAN=GPIO0;
		LED_WAN=GPIONONE;
		BTN_RESET=GPIO10;
	}
	else
	{	
		LED_POWER=GPIO1;
		LED_WLAN=GPIO0;
		LED_WAN=GPIONONE;
		BTN_RESET=GPIO10;
	}
	gpio_write("/dev/gpio/outen", GPIOMASK, 1);
	gpio_write("/dev/gpio/outen", LED_POWER, 0);
	gpio_write("/dev/gpio/outen", LED_WLAN, 0);
	gpio_write("/dev/gpio/outen", LED_WAN, 0);
	gpio_write("/dev/gpio/outen", BTN_RESET, 1);
}


void LED_CONTROL(led,flag)
{ 
	gpio_write("/dev/gpio/out", led, flag);
}

void LED_BLINK_CONTROL(led, flag)
{
	unsigned int val;

	if (flag) val=(0x03<<(led*2));
	else val=(0x10<<(led*2));
	gpio_write("/dev/gpio/control", val, flag);
}	

void wanledctrl(int flag)
{
	if (flag<2) 
	{
		LED_BLINK_CONTROL(LED_WAN, 0);
		LED_CONTROL(LED_WAN, !flag);
	}
	else 
	{
		LED_BLINK_CONTROL(LED_WAN, 1);
	}

	//printf("WAN LED: %d %d\n", flag, blinkflag);
}

static void
alarmtimer(unsigned long sec,unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
	itv.it_value.tv_usec = usec;
	itv.it_interval = itv.it_value;
	setitimer(ITIMER_REAL, &itv, NULL);
}

void btn_check(void)
{
	//printf("btn :  %d %d\n", btn_pressed, btn_count);	
	if (!gpio_read("/dev/gpio/in", BTN_RESET))
	{
		if (!btn_pressed)
		{
			btn_pressed=1;
			btn_count=0;
			alarmtimer(0, URGENT_PERIOD);
		}
		else {	/* Whenever it is pushed steady */
			if( ++btn_count > RESET_WAIT_COUNT )
			{
				btn_pressed=2;
			}

			if (btn_pressed==2)
			{
				/* 0123456789 */
				/* 0011100111 */
				if ((btn_count%10)<1 ||
				    ((btn_count%10)>4 && (btn_count%10)<7)) LED_CONTROL(LED_POWER, LED_OFF);
				else LED_CONTROL(LED_POWER, LED_ON);


			}
		}
	}
	else
	{
		if(btn_pressed==1)
		{
			btn_count = 0;
			btn_pressed = 0;			
			LED_CONTROL(LED_POWER, LED_ON);
			alarmtimer(NORMAL_PERIOD, 0);
		}
		else if(btn_pressed==2)
		{
			alarmtimer(0, 0);
			eval("erase", "/dev/mtd2");
			kill(1, SIGTERM);
		}
	}

	if (!btn_pressed && gpio_read("/dev/gpio/out", LED_POWER))
	{
		LED_CONTROL(LED_POWER, LED_ON);
	}		
}

void refresh_ntpc(void)
{
	eval("killall","ntpclient");
	kill_pidfile_s("/var/run/ntp.pid", SIGUSR1);	
	//printf("Sync time %dn", sync_interval);
}

int ntp_timesync(void)
{
	time_t now;
	struct tm tm;	
	struct tm gm, local;
	struct timezone tz;

	//if (nvram_match("router_disable", "1")) return 0;
	
	if (sync_interval!=-1)
	{
        	sync_interval--;

	    	if (sync_interval==0)
		{
			/* Update kernel timezone */
			setenv("TZ", nvram_safe_get("time_zone_x"), 1);
			time(&now);
			gmtime_r(&now, &gm);
			localtime_r(&now, &local);
			tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;
			settimeofday(NULL, &tz);
		   	memcpy(&tm, localtime(&now), sizeof(struct tm));

		   	if (tm.tm_year>100) // More than 2000 
		   	{	 
		      		sync_interval=60*60/5;

			  	logmessage("ntp client", "time is synchronized to %s", nvram_safe_get("ntp_servers"));
				stop_upnp();
				sleep(1);
				start_upnp();
		   	}	
		  	else sync_interval=1;
	
			refresh_ntpc();	
		}
	}	
}

enum 
{
	URLACTIVE=0,
	WEBACTIVE,
	RADIOACTIVE,
	ACTIVEITEMS
} ACTIVE;

int svcStatus[ACTIVEITEMS] = { -1, -1, -1};
int extStatus[ACTIVEITEMS] = { 0, 0, 0};
char svcDate[ACTIVEITEMS][10];
char *svcTime[ACTIVEITEMS][20];

int timecheck_item(char *activeDate, char *activeTime)
{
	#define DAYSTART (0)
	#define DAYEND (60*60*23+60*59+59) //86399
	int current, active, activeTimeStart, activeTimeEnd, i;
	time_t now;
	struct tm *tm;

	time(&now);
	tm = localtime(&now);
	current = tm->tm_hour*60 + tm->tm_min;

	active=0;

	//printf("active: %s %s\n", activeDate, activeTime);

	activeTimeStart=((activeTime[0]-'0')*10+(activeTime[1]-'0'))*60 + (activeTime[2]-'0')*10 + (activeTime[3]-'0');
		
	activeTimeEnd=((activeTime[4]-'0')*10+(activeTime[5]-'0'))*60 + (activeTime[6]-'0')*10 + (activeTime[7]-'0');
				
	if (activeDate[tm->tm_wday] == '1')
	{
		if (activeTimeEnd<activeTimeStart)
		{
			if ((current>=activeTimeStart && current<=DAYEND) ||
			   (current>=DAYSTART && current<=activeTimeEnd))
			{
				active = 1;
			}
			else
			{
				active = 0;
			}
		}
		else
		{
			if (current>=activeTimeStart &&
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

int svc_timecheck(void)
{
	int activeFlag, activeNow;

	activeFlag = 0;

	/* Initialize */
	if (svcStatus[URLACTIVE]==-1 && nvram_invmatch("url_enable_x", "0"))
	{
		strcpy(svcDate[URLACTIVE], nvram_safe_get("url_date_x"));
		strcpy(svcTime[URLACTIVE], nvram_safe_get("url_time_x"));
		svcStatus[URLACTIVE] = -2;
	}	

	if (svcStatus[URLACTIVE]!=-1)
	{
		activeNow = timecheck_item(svcDate[URLACTIVE], svcTime[URLACTIVE]);	
		if (activeNow!=svcStatus[URLACTIVE])
		{
			//printf("url time change: %d\n", activeNow);
			svcStatus[URLACTIVE] = activeNow;
			stop_dns();
			start_dns();
		}
	}

	if (svcStatus[WEBACTIVE]==-1 && 
		nvram_invmatch("usb_webenable_x", "0") &&
		nvram_invmatch("usb_websecurity_x", "0"))
	{	
		strcpy(svcDate[WEBACTIVE], nvram_safe_get("usb_websecurity_date_x"));
		strcpy(svcTime[WEBACTIVE], nvram_safe_get("usb_websecurity_time_x"));
		svcStatus[WEBACTIVE] = -2;
	}

	if (svcStatus[WEBACTIVE]!=-1)
	{
		activeNow = timecheck_item(svcDate[WEBACTIVE], svcTime[WEBACTIVE]);
		if (activeNow!=svcStatus[WEBACTIVE])
		{
			svcStatus[WEBACTIVE] = activeNow;

			if (!notice_rcamd(svcStatus[WEBACTIVE])) svcStatus[WEBACTIVE]=-1;
		}	
	}

	if (svcStatus[RADIOACTIVE]==-1 && nvram_invmatch("wl_radio_x", "0"))
	{	
		strcpy(svcDate[RADIOACTIVE], nvram_safe_get("wl_radio_date_x"));
		strcpy(svcTime[RADIOACTIVE], nvram_safe_get("wl_radio_time_x"));
		svcStatus[RADIOACTIVE] = -2;
	}


	if (svcStatus[RADIOACTIVE]!=-1)
	{
		activeNow = timecheck_item(svcDate[RADIOACTIVE], svcTime[RADIOACTIVE]);
		if (activeNow!=svcStatus[RADIOACTIVE])
		{
			svcStatus[RADIOACTIVE] = activeNow;
#ifdef BRCM
			if (activeNow) eval("wl", "radio", "on");
			else eval("wl", "radio", "off");
#endif
		}
	}

	//printf("svc : %d %d %d\n", svcStatus[0], svcStatus[1], svcStatus[2]);
	return 0;
}	
	
/* Sometimes, httpd becomes inaccessible, try to re-run it */
int http_processcheck(void)
{
	static char *http_cmd="http://127.0.0.1/";
	char buf[256];
	char piddir[64], pidstatus[512];
	int pidflag=0;
	FILE *fp;
	//printf("http check\n");

	if (!http_check(http_cmd, buf, sizeof(buf), 0))
	{
		dprintf("http rerun\n");
		kill_pidfile("/var/run/httpd.pid");
		stop_httpd();
		start_httpd();
	}

	// check upnp process
	sprintf(piddir, "/proc/%d/status", atoi(nvram_safe_get("upnp_pid")));
	if((fp=fopen(piddir, "r+"))!=NULL)
	{
		while(fgets(pidstatus, sizeof(pidstatus), fp))
		{
			if(strstr(pidstatus, "State:"))
			{
				if(strstr(pidstatus, "zombie")) pidflag=0;
				else pidflag=1;
				break;
			}
		}
		fclose(fp);
	}
	else pidflag=0;

	if(!pidflag)
	{
		stop_upnp();
		sleep(1);
		start_upnp();
	}
	
	return 0;
}


int notice_rcamd(int flag)
{
	 int rcamdpid=-1;
	 //printf("Send signal : %d %d\n", rcamdpid, flag);
	 if (rcamdpid==-1)
	 {
			FILE *fp;

			if ((fp=fopen("/var/run/rcamd.pid","r"))!=NULL)
			{
				fscanf(fp,"%d", &rcamdpid);
				fclose(fp);
			}			
	 }
	 if (rcamdpid!=-1) 
	 {
		 if (flag)
			kill(rcamdpid, SIGUSR1);
		 else
			kill(rcamdpid, SIGUSR2);	

		 return 1;
	 }
	 return 0;
}


static void catch_sig(int sig)
{
	if (sig == SIGUSR1)
	{
		dprintf("Catch Reset to Default Signal\n");
	}
	else if (sig == SIGUSR2)
	{
		FILE *fp;
		char command[256], *cmd_ptr;

		dprintf("Get Signal: %d %d %d\n", svcStatus[WEBACTIVE], extStatus[WEBACTIVE], sig);

		if (!svcStatus[WEBACTIVE]) return;

		if (extStatus[WEBACTIVE]==0)
		{
			fp = fopen("/var/tmp/runshell", "r+");
			if (fp!=NULL)
			{
				cmd_ptr = fgets(command, 256, fp);

				if (cmd_ptr!=NULL) system(command);
			}
			fclose(fp);
			unlink("/tmp/runshell");
			notice_rcamd(0);
			extStatus[WEBACTIVE]=1;
		}
		else if (svcStatus[WEBACTIVE]==1)
		{
			notice_rcamd(1);
			extStatus[WEBACTIVE] = 0;
		}
	}	
}

void sta_check(void)
{
#ifdef REMOVE
	int ret, i;
	char *wl_ifname=nvram_safe_get("wl0_ifname");
	char bssid[32];
	int val;

	if (stacheck_interval==-1)
	{
		if (nvram_invmatch("wl0_mode", "sta") && 
            		nvram_invmatch("wl0_mode", "wet")) return;

		stacheck_interval=STACHECK_PERIOD_DISCONNECT;
	}
	
	stacheck_interval--;

	if (stacheck_interval) return;

	// Get bssid
	ret=wl_ioctl(wl_ifname, WLC_GET_BSSID, bssid, sizeof(bssid));

	if (ret==0 && !(bssid[0]==0&&bssid[1]==0&&bssid[2]==0
		&&bssid[3]==0&&bssid[4]==0&&bssid[5]==0)) 	
	{
		dprintf("connected\n");
		stacheck_interval=STACHECK_PERIOD_CONNECT;
	}
	else 
	{
		dprintf("disconnected\n");
		stacheck_interval=STACHECK_PERIOD_DISCONNECT;
		eval("wl", "join", nvram_safe_get("wl0_ssid"));
	}
#endif
	return;
}

/* wathchdog is runned in NORMAL_PERIOD, 1 seconds
 * check in each NORMAL_PERIOD
 *	1. button
 *
 * check in each NORAML_PERIOD*5
 *
 *      1. ntptime, 
 *      2. time-dependent service
 *      3. http-process
 *      4. usb hotplug status
 */

void watchdog(void)
{
	time_t now;
	int flag;

	/* handle button */
	btn_check();

	/* if timer is set to less than 1 sec, then bypass the following */
	if (itv.it_value.tv_sec==0) return;

#ifndef REMOVE_QTR
	// check only when wan mode is not disabled
	if (nvram_invmatch("router_disable", "1"))
	{
		// if cable is not attached or wan status is disconnected
		// LED blinking
		if (!(flag=check_wanmode()) || nvram_match("wan_status_t", "Disconnected"))
		{
			wanstate = 0;
			blinkflag = (blinkflag+1)%2;

			LED_CONTROL(LED_WAN, blinkflag);
		}
		else
		{
			if (!wanstate)
			{
				LED_CONTROL(LED_WAN, LED_ON);
			}
			wanstate = 1;
		}	

		// cable state changed
		if (flag!=cablestate)
		{
			// state changed, 
			// cable is not attached 
			// wan mode is auto
			if (flag && 
				nvram_match("wan_mode_x", "2") && 
				!dhcpd_exist())
 			{
				stop_dhcpd();
				if (flag) start_dhcpd();
			}
			cablestate = flag;
		}
	}
#endif
	watchdog_period = (watchdog_period+1)%10;
	if (watchdog_period) return;

	time(&now);
	//printf("now: %d %d\n", (long )now, blinkflag);

	/* sync time to ntp server if necessary */
	ntp_timesync();

	/* check for time-dependent services */
	svc_timecheck();

	/* http server check */
	http_processcheck();

#ifdef USB_SUPPORT
	/* web cam process */
	if (nvram_invmatch("usb_web_device", ""))
	{						
		hotplug_usb_webcam(nvram_safe_get("usb_web_device"), 
				   atoi(nvram_safe_get("usb_web_flag")));		

		nvram_set("usb_web_device", "");
		nvram_set("usb_web_flag", "");

		// reset WEBCAM status	
		svcStatus[WEBACTIVE] = -1;
	}
#endif
	/* station or ethernet bridge handler */
	sta_check();
}

int 
watchdog_main(int argc, char *argv[])
{
	FILE *fp;

#ifdef REMOVE
	/* Run it under background */
	switch (fork()) {
	case -1:
		exit(0);
		break;
	case 0:	
		// start in a new session	
		(void) setsid();
		break;
	default:	
		/* parent process should just die */
		_exit(0);
	}
#endif

	/* write pid */
	if ((fp=fopen("/var/run/watchdog.pid", "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set the signal handler */
	signal(SIGUSR1, catch_sig);
	signal(SIGUSR2, catch_sig);
	signal(SIGALRM, watchdog);

	/* set timer */
	alarmtimer(NORMAL_PERIOD, 0);

	/* Start GPIO function */
	gpio_init();

	/* Start POWER LED */
	LED_CONTROL(LED_POWER, LED_ON);
	LED_CONTROL(LED_WLAN, LED_ON);
	LED_CONTROL(LED_WAN, LED_OFF);

	/* Start sync time */
	sync_interval=1;
	start_ntpc();

	/* Most of time it goes to sleep */
	while(1)
	{
		pause();
	}
	
	return 0;
}
