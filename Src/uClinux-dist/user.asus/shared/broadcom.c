/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id$
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <httpd.h>
#endif /* WEBS */


#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <netconf.h>
#include <nvparse.h>
#include <wlutils.h>

static char * rfctime(const time_t *timep);
static char * reltime(unsigned int seconds);

#define wan_prefix(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)

/*
 * Country names and abbreviations from ISO 3166
 */
typedef struct {
	char *name;     /* Long name */
	char *abbrev;   /* Abbreviation */
} country_name_t;
country_name_t country_names[];     /* At end of this file */

char ibuf[WLC_IOCTL_MAXLEN];

#if defined(linux)

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if_arp.h>

#define sys_restart() kill(1, SIGHUP)
#define sys_reboot() kill(1, SIGTERM)
#define sys_stats(url) eval("stats", (url))

#ifndef WEBS

#define MIN_BUF_SIZE	4096

/* Upgrade from remote server or socket stream */
static int
sys_upgrade(char *url, FILE *stream, int *total)
{
	char upload_fifo[] = "/tmp/uploadXXXXXX";
	FILE *fifo = NULL;
	char *write_argv[] = { "write", upload_fifo, "linux", NULL };
	pid_t pid;
	char *buf = NULL;
	int count, ret = 0;
	long flags = -1;
	int size = BUFSIZ;

	if (url)
		return eval("write", url, "linux");

	/* Feed write from a temporary FIFO */
	if (!mktemp(upload_fifo) ||
	    mkfifo(upload_fifo, S_IRWXU) < 0||
	    (ret = _eval(write_argv, NULL, 0, &pid)) ||
	    !(fifo = fopen(upload_fifo, "w"))) {
		if (!ret)
			ret = errno;
		goto err;
	}

	/* Set nonblock on the socket so we can timeout */
	if ((flags = fcntl(fileno(stream), F_GETFL)) < 0 ||
	    fcntl(fileno(stream), F_SETFL, flags | O_NONBLOCK) < 0) {
		ret = errno;
		goto err;
	}

	/*
	* The buffer must be at least as big as what the stream file is
	* using so that it can read all the data that has been buffered 
	* in the stream file. Otherwise it would be out of sync with fn
	* select specially at the end of the data stream in which case
	* the select tells there is no more data available but there in 
	* fact is data buffered in the stream file's buffer. Since no
	* one has changed the default stream file's buffer size, let's
	* use the constant BUFSIZ until someone changes it.
	*/
	if (size < MIN_BUF_SIZE)
		size = MIN_BUF_SIZE;
	if ((buf = malloc(size)) == NULL) {
		ret = ENOMEM;
		goto err;
	}
	
	/* Pipe the rest to the FIFO */
	cprintf("Upgrading");
	while (total && *total) {
		if (waitfor(fileno(stream), 5) <= 0)
			break;
		count = safe_fread(buf, 1, size, stream);
		if (!count && (ferror(stream) || feof(stream)))
			break;
		*total -= count;
		safe_fwrite(buf, 1, count, fifo);
		cprintf(".");
	}
	fclose(fifo);
	fifo = NULL;

	/* Wait for write to terminate */
	waitpid(pid, &ret, 0);
	cprintf("done\n");

	/* Reset nonblock on the socket */
	if (fcntl(fileno(stream), F_SETFL, flags) < 0) {
		ret = errno;
		goto err;
	}

 err:
 	if (buf)
		free(buf);
	if (fifo)
		fclose(fifo);
	unlink(upload_fifo);
	return ret;
}

#endif

/* Dump firewall log */
static int
ej_dumplog(int eid, webs_t wp, int argc, char_t **argv)
{
	char buf[4096], *line, *next, *s;
	int len, ret = 0;

	time_t tm;
	char *verdict, *src, *dst, *proto, *spt, *dpt;

	if (klogctl(3, buf, 4096) < 0) {
		websError(wp, 400, "Insufficient memory\n");
		return -1;
	}

	for (next = buf; (line = strsep(&next, "\n"));) {
		if (!strncmp(line, "<4>DROP", 7))
			verdict = "denied";
		else if (!strncmp(line, "<4>ACCEPT", 9))
			verdict = "accepted";
		else
			continue;

		/* Parse into tokens */
		s = line;
		len = strlen(s);
		while (strsep(&s, " "));

		/* Initialize token values */
		time(&tm);
		src = dst = proto = spt = dpt = "n/a";

		/* Set token values */
		for (s = line; s < &line[len] && *s; s += strlen(s) + 1) {
			if (!strncmp(s, "TIME=", 5))
				tm = strtoul(&s[5], NULL, 10);
			else if (!strncmp(s, "SRC=", 4))
				src = &s[4];
			else if (!strncmp(s, "DST=", 4))
				dst = &s[4];
			else if (!strncmp(s, "PROTO=", 6))
				proto = &s[6];
			else if (!strncmp(s, "SPT=", 4))
				spt = &s[4];
			else if (!strncmp(s, "DPT=", 4))
				dpt = &s[4];
		}

		ret += websWrite(wp, "%s %s connection %s to %s:%s from %s:%s\n",
				 rfctime(&tm), proto, verdict, dst, dpt, src, spt);
		ret += websWrite(wp, "<br>");
	}

	return ret;
}

struct lease_t {
	unsigned char chaddr[16];
	u_int32_t yiaddr;
	u_int32_t expires;
	char hostname[64];
};

/* Dump leases in <tr><td>hostname</td><td>MAC</td><td>IP</td><td>expires</td></tr> format */
static int
ej_lan_leases(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp = NULL;
	struct lease_t lease;
	int i;
	struct in_addr addr;
	unsigned long expires = 0;
	char sigusr1[] = "-XX";
	int ret = 0;

	/* Write out leases file */
	sprintf(sigusr1, "-%d", SIGUSR1);
	eval("killall", sigusr1, "udhcpd");

	if (!(fp = fopen("/tmp/udhcpd.leases", "r")))
		return 0;

	while (fread(&lease, sizeof(lease), 1, fp)) {
		/* Do not display reserved leases */
		if (ETHER_ISNULLADDR(lease.chaddr))
			continue;
		ret += websWrite(wp, "<tr><td>%s</td><td>", lease.hostname);
		for (i = 0; i < 6; i++) {
			ret += websWrite(wp, "%02X", lease.chaddr[i]);
			if (i != 5) ret += websWrite(wp, ":");
		}
		addr.s_addr = lease.yiaddr;
		ret += websWrite(wp, "</td><td>%s</td><td>", inet_ntoa(addr));
		expires = ntohl(lease.expires);
		if (!expires)
			ret += websWrite(wp, "Expired");
		else
			ret += websWrite(wp, "%s", reltime(expires));
		ret += websWrite(wp, "</td></tr>");
	}

	fclose(fp);

	return ret;
}

/* Renew lease */
static int
sys_renew(void)
{
	int unit;
	char tmp[100];
	char *str;
	int pid;

	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	
	snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", unit);
	if ((str = file2str(tmp))) {
		pid = atoi(str);
		free(str);
		return kill(pid, SIGUSR1);
	}
	
	return -1;
}

/* Release lease */
static int
sys_release(void)
{
	int unit;
	char tmp[100];
	char *str;
	int pid;

	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	
	snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", unit);
	if ((str = file2str(tmp))) {
		pid = atoi(str);
		free(str);
		return kill(pid, SIGUSR2);
	}
	
	return -1;
}

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

/* Return WAN link state */
static int
ej_wan_link(int eid, webs_t wp, int argc, char_t **argv)
{
	char *wan_ifname;
	int s;
	struct ifreq ifr;
	struct ethtool_cmd ecmd;
	FILE *fp;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	wan_prefix(unit, prefix);

	/* non-exist and disabled */
	if (nvram_match(strcat_r(prefix, "proto", tmp), "") ||
	    nvram_match(strcat_r(prefix, "proto", tmp), "disabled")) {
		return websWrite(wp, "N/A");
	}
	/* PPPoE connection status */
	else if (nvram_match(strcat_r(prefix, "proto", tmp), "pppoe")) {
		wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
		if ((fp = fopen(strcat_r("/tmp/ppp/link.", wan_ifname, tmp), "r"))) {
			fclose(fp);
			return websWrite(wp, "Connected");
		} else
			return websWrite(wp, "Disconnected");
	}
	/* Get real interface name */
	else
		wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	/* Open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return websWrite(wp, "N/A");

	/* Check for hardware link */
	strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
	ifr.ifr_data = (void *) &ecmd;
	ecmd.cmd = ETHTOOL_GSET;
	if (ioctl(s, SIOCETHTOOL, &ifr) < 0) {
		close(s);
		return websWrite(wp, "Unknown");
	}
	if (!ecmd.speed) {
		close(s);
		return websWrite(wp, "Disconnected");
	}

	/* Check for valid IP address */
	strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		close(s);
		return websWrite(wp, "Connecting");
	}

	/* Otherwise we are probably configured */
	close(s);
	return websWrite(wp, "Connected");
}

/* Display IP Address lease */
static int
ej_wan_lease(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned long expires = 0;
	int ret = 0;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	wan_prefix(unit, prefix);
	
	if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp")) {
		char *str;
		time_t now;

		snprintf(tmp, sizeof(tmp), "/tmp/udhcpc%d.expires", unit); 
		if ((str = file2str(tmp))) {
			expires = atoi(str);
			free(str);
		}
		time(&now);
		if (expires <= now)
			ret += websWrite(wp, "Expired");
		else
			ret += websWrite(wp, "%s", reltime(expires - now));
	} else
		ret += websWrite(wp, "N/A");

	return ret;
}

/* Report sys up time */
static int
ej_sysuptime(int eid, webs_t wp, int argc, char_t **argv)
{
	char *str = file2str("/proc/uptime");
	if (str) {
		unsigned int up = atoi(str);
		free(str);
		return websWrite(wp, reltime(up));
	}
	return websWrite(wp, "N/A");
}

/* Return a list of wan interfaces (eth0/eth1/eth2/eth3) */
static int
ej_wan_iflist(int eid, webs_t wp, int argc, char_t **argv)
{
	char name[IFNAMSIZ], *next;
	int ret = 0;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char ea[64];
	int s;
	struct ifreq ifr;

	/* current unit # */
	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	wan_prefix(unit, prefix);
	
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return errno;
	
	/* build wan interface name list */
	foreach(name, nvram_safe_get("wan_ifnames"), next) {
		strncpy(ifr.ifr_name, name, IFNAMSIZ);
		if (ioctl(s, SIOCGIFHWADDR, &ifr))
			continue;
		ret += websWrite(wp, "<option value=\"%s\" %s>%s (%s)</option>", name,
				 nvram_match(strcat_r(prefix, "ifname", tmp), name) ? "selected" : "",
				 name, ether_etoa(ifr.ifr_hwaddr.sa_data, ea));
	}

	close(s);

	return ret;
}


#endif

static char *
rfctime(const time_t *timep)
{
	static char s[201];
	struct tm tm;

#if defined(linux)
	setenv("TZ", nvram_safe_get("time_zone"), 1);
#endif
	memcpy(&tm, localtime(timep), sizeof(struct tm));
	strftime(s, 200, "%a, %d %b %Y %H:%M:%S %z", &tm);
	return s;
}

static char *
reltime(unsigned int seconds)
{
	static char s[] = "XXXXX days, XX hours, XX minutes, XX seconds";
	char *c = s;

	if (seconds > 60*60*24) {
		c += sprintf(c, "%d days, ", seconds / (60*60*24));
		seconds %= 60*60*24;
	}
	if (seconds > 60*60) {
		c += sprintf(c, "%d hours, ", seconds / (60*60));
		seconds %= 60*60;
	}
	if (seconds > 60) {
		c += sprintf(c, "%d minutes, ", seconds / 60);
		seconds %= 60;
	}
	c += sprintf(c, "%d seconds", seconds);

	return s;
}

/* Report time in RFC-822 format */
static int
ej_localtime(int eid, webs_t wp, int argc, char_t **argv)
{
	time_t tm;

	time(&tm);
	return websWrite(wp, rfctime(&tm));
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1
 * <% nvram_get("lan_ipaddr"); %> produces "192.168.1.1"
 * <% nvram_get("undefined"); %> produces ""
 */
static int
ej_nvram_get(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *c;
	int ret = 0;

	if (ejArgs(argc, argv, "%s", &name) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (c = nvram_safe_get(name); *c; c++) {
		if (isprint((int) *c) &&
		    *c != '"' && *c != '&' && *c != '<' && *c != '>')
			ret += websWrite(wp, "%c", *c);
		else
			ret += websWrite(wp, "&#%d", *c);
	}

	return ret;
}

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
static int
ej_nvram_match(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *match, *output;

	if (ejArgs(argc, argv, "%s %s %s", &name, &match, &output) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (nvram_match(name, match))
		return websWrite(wp, output);

	return 0;
}	

static int
ej_wl_channel_list(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit, unit2, i, channel;
	char name[IFNAMSIZ], *next;
	char ifnames[256];
	char *phytype, *country, *channelstr;

	phytype = websGetVar(wp, "wl_phytype", NULL);
	if (phytype == NULL)
		phytype = nvram_safe_get("wl_phytype");

	country = websGetVar(wp, "wl_country_code", NULL);
	if (country == NULL)	
		country = nvram_safe_get("wl_country_code");

	channelstr = websGetVar(wp, "wl_channel", NULL);
	if (channelstr == NULL)
		channelstr = nvram_safe_get("wl_channel");

	channel = atoi(channelstr);

	if ((phytype == NULL) || (country == NULL) || (channel < 0))
		return -1;

	if ((unit2 = atoi(nvram_safe_get("wl_unit"))) < 0)
		return -1;

	snprintf(ifnames, sizeof(ifnames), "%s %s", 
		nvram_safe_get("lan_ifnames"),
		nvram_safe_get("wan_ifnames"));

	foreach(name, ifnames, next) {
		if (wl_probe(name) == 0 &&
		    wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit)) == 0 &&
		    unit == unit2) {
			wl_channels_in_country_t *cic = (wl_channels_in_country_t *)ibuf;

			cic->buflen = sizeof(ibuf);;
			strcpy(cic->country_abbrev, country);
			if (!strcmp(phytype, "a"))
				cic->band = WLC_BAND_A;
			else if ((!strcmp(phytype, "b")) || (!strcmp(phytype, "g")))
				cic->band = WLC_BAND_B;
			else
				return -1;

			if (wl_ioctl(name, WLC_GET_CHANNELS_IN_COUNTRY, cic, cic->buflen) == 0) {
				websWrite(wp, "<option value=\"0\" %s>Auto</option>\n",
					channel == 0 ? "selected" : "");
				for(i = 0; i < cic->count; i++)
					websWrite(wp, "<option value=\"%d\" %s>%d</option>\n",
				 		cic->channel[i], channel == cic->channel[i] ? "selected" : "", 
						cic->channel[i]);
			}

			return 0;
		}
	}

	return -1;			
}

static int
ej_wl_country_list(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit, unit2, i;
	char name[IFNAMSIZ], *next;
	char ifnames[256];
	char *phytype, *country;

	phytype = websGetVar(wp, "wl_phytype", NULL);
	if (phytype == NULL)
		phytype = nvram_safe_get("wl_phytype");

	country = websGetVar(wp, "wl_country_code", NULL);
	if (country == NULL)	
		country = nvram_safe_get("wl_country_code");

	if ((phytype == NULL) || (country == NULL))
		return -1;

	if ((unit2 = atoi(nvram_safe_get("wl_unit"))) < 0)
		return -1;

	snprintf(ifnames, sizeof(ifnames), "%s %s", 
		nvram_safe_get("lan_ifnames"),
		nvram_safe_get("wan_ifnames"));

	foreach(name, ifnames, next) {
		if (wl_probe(name) == 0 &&
		    wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit)) == 0 &&
		    unit == unit2) {
			wl_country_list_t *cl = (wl_country_list_t *)ibuf;
			country_name_t *cntry;
			char *abbrev;

			cl->buflen = sizeof(ibuf);;
			cl->band_set = TRUE;

			if (!strcmp(phytype, "a"))
				cl->band = WLC_BAND_A;
			else if ((!strcmp(phytype, "b")) || (!strcmp(phytype, "g")))
				cl->band = WLC_BAND_B;
			else
				return -1;

			if (wl_ioctl(name, WLC_GET_COUNTRY_LIST, cl, cl->buflen) == 0) {
				for(i = 0; i < cl->count; i++) {
					abbrev = &cl->country_abbrev[i*WLC_CNTRY_BUF_SZ];
					for(cntry = country_names;
						cntry->name && strcmp(abbrev, cntry->abbrev);
						cntry++); 
					websWrite(wp, "<option value=\"%s\" %s>%s</option>\n",
				 		abbrev, strcmp(abbrev, country) ? "" : "selected",
						cntry->name ? cntry->name : abbrev);
				}
			}

			return 0;
		}
	}

	return -1;
}

static int
ej_wl_ioctl(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit, val;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";
	char *op, *type, *var;
	char *name;

	if (ejArgs(argc, argv, "%s %s %s", &op, &type, &var) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if ((unit = atoi(nvram_safe_get("wl_unit"))) < 0)
		return -1;

	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	name = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (strcmp(op, "get") == 0) {
		if (strcmp(type, "int") == 0)
			return websWrite(wp, "%u", wl_get_int(name, var, &val) == 0 ? val : 0);
	}
	return -1;
}

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_invmatch("wan_proto", "dhcp", "disabled"); %> does not produce
 * <% nvram_invmatch("wan_proto", "static", "disabled"); %> produces "disabled"
 */
static int
ej_nvram_invmatch(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name, *invmatch, *output;

	if (ejArgs(argc, argv, "%s %s %s", &name, &invmatch, &output) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (nvram_invmatch(name, invmatch))
		return websWrite(wp, output);

	return 0;
}	

/*
 * Example: 
 * filter_maclist=00:12:34:56:78:00 00:87:65:43:21:00
 * <% nvram_list("filter_maclist", 1); %> produces "00:87:65:43:21:00"
 * <% nvram_list("filter_maclist", 100); %> produces ""
 */
static int
ej_nvram_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	int which;
	char word[256], *next;

	if (ejArgs(argc, argv, "%s %d", &name, &which) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	foreach(word, nvram_safe_get(name), next) {
		if (which-- == 0)
			return websWrite(wp, word);
	}

	return 0;
}

static int
ej_options(int eid, webs_t wp, int argc, char_t **argv)
{
	char *name;
	int cur, min, max, step = 1, ret = 0;

	if (ejArgs(argc, argv, "%s %d %d %d", &name, &min, &max, &step) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (cur = atoi(nvram_safe_get(name)); min <= max; min += step) {
		ret += websWrite(wp, "<option value=\"%d\" %s>%d</option>\n",
				 min, min == cur ? "selected" : "", min);
	}

	return ret;
}


/*
 * Example: 
 * <% filter_client(1, 10); %> produces a table of the first 10 client filter entries
 */
static int
ej_filter_client(int eid, webs_t wp, int argc, char_t **argv)
{
	int i, n, j, ret = 0;
	netconf_filter_t start, end;
	bool valid;
	char port[] = "XXXXX";
	char *days[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	char *hours[] = {
		"12:00 AM", "1:00 AM", "2:00 AM", "3:00 AM", "4:00 AM", "5:00 AM",
		"6:00 AM", "7:00 AM", "8:00 AM", "9:00 AM", "10:00 AM", "11:00 AM",
		"12:00 PM", "1:00 PM", "2:00 PM", "3:00 PM", "4:00 PM", "5:00 PM",
		"6:00 PM", "7:00 PM", "8:00 PM", "9:00 PM", "10:00 PM", "11:00 PM"
	};

	if (ejArgs(argc, argv, "%d %d", &i, &n) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (; i <= n; i++) {
		valid = get_filter_client(i, &start, &end);

		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<td></td>");

		/* Print address range */
		ret += websWrite(wp, "<td><input name=\"filter_client_from_start%d\" value=\"%s\" size=\"15\" maxlength=\"15\"></td>",
				 i, valid ? inet_ntoa(start.match.src.ipaddr) : "");
		ret += websWrite(wp, "<td>-</td>");
		ret += websWrite(wp, "<td><input name=\"filter_client_from_end%d\" value=\"%s\" size=\"15\" maxlength=\"15\"></td>",
				 i, valid ? inet_ntoa(end.match.src.ipaddr) : "");
		ret += websWrite(wp, "<td></td>");

		/* Print protocol */
		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"filter_client_proto%d\">", i);
		ret += websWrite(wp, "<option value=\"tcp\" %s>TCP</option>",
				 valid && start.match.ipproto == IPPROTO_TCP ? "selected" : "");
		ret += websWrite(wp, "<option value=\"udp\" %s>UDP</option>",
				 valid && start.match.ipproto == IPPROTO_UDP ? "selected" : "");
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td></td>");

		/* Print port range */
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(start.match.dst.ports[0]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"filter_client_to_start%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td>-</td>");
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(start.match.dst.ports[1]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"filter_client_to_end%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td></td>");

		/* Print day range */
		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"filter_client_from_day%d\">", i);
		for (j = 0; j < ARRAYSIZE(days); j++)
			ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>",
					 j, valid && start.match.days[0] == j ? "selected" : "", days[j]);
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td>-</td>");
		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"filter_client_to_day%d\">", i);
		for (j = 0; j < ARRAYSIZE(days); j++)
			ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>",
					 j, valid && start.match.days[1] == j ? "selected" : "", days[j]);
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td></td>");

		/* Print time range */
		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"filter_client_from_sec%d\">", i);
		for (j = 0; j < ARRAYSIZE(hours); j++)
			ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>",
					 j * 3600, valid && start.match.secs[0] == (j * 3600) ? "selected" : "", hours[j]);
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td>-</td>");

		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"filter_client_to_sec%d\">", i);
		for (j = 0; j < ARRAYSIZE(hours); j++)
			ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>",
					 j * 3600, valid && start.match.secs[1] == (j * 3600) ? "selected" : "", hours[j]);
		/* Special case for 11:59:59 PM */
		ret += websWrite(wp, "<option value=\"%d\" %s>12:00 AM</option>",
				 24 * 3600 - 1, valid && start.match.secs[1] == (24 * 3600 - 1) ? "selected" : "");
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td></td>");

		/* Print enable */
		ret += websWrite(wp, "<td><input type=\"checkbox\" name=\"filter_client_enable%d\" %s></td>",
				 i, valid && !(start.match.flags & NETCONF_DISABLED) ? "checked" : "");

		ret += websWrite(wp, "</tr>");
	}

	return ret;
}

/*
 * Example: 
 * <% forward_port(1, 10); %> produces a table of the first 10 port forward entries
 */
static int
ej_forward_port(int eid, webs_t wp, int argc, char_t **argv)
{
	int i, n, ret = 0;
	netconf_nat_t nat;
	bool valid;
	char port[] = "XXXXX";

	if (ejArgs(argc, argv, "%d %d", &i, &n) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (; i <= n; i++) {
		valid = get_forward_port(i, &nat);

		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<td></td>");

		/* Print protocol */
		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"forward_port_proto%d\">", i);
		ret += websWrite(wp, "<option value=\"tcp\" %s>TCP</option>",
				 valid && nat.match.ipproto == IPPROTO_TCP ? "selected" : "");
		ret += websWrite(wp, "<option value=\"udp\" %s>UDP</option>",
				 valid && nat.match.ipproto == IPPROTO_UDP ? "selected" : "");
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td></td>");

		/* Print WAN destination port range */
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(nat.match.dst.ports[0]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"forward_port_from_start%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td>-</td>");
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(nat.match.dst.ports[1]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"forward_port_from_end%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td>></td>");

		/* Print address range */
		ret += websWrite(wp, "<td><input name=\"forward_port_to_ip%d\" value=\"%s\" size=\"15\" maxlength=\"15\"></td>",
				 i, valid ? inet_ntoa(nat.ipaddr) : "");
		ret += websWrite(wp, "<td>:</td>");

		/* Print LAN destination port range */
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(nat.ports[0]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"forward_port_to_start%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td>-</td>");
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(nat.ports[1]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"forward_port_to_end%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td></td>");

		/* Print enable */
		ret += websWrite(wp, "<td><input type=\"checkbox\" name=\"forward_port_enable%d\" %s></td>",
				 i, valid && !(nat.match.flags & NETCONF_DISABLED) ? "checked" : "");

		ret += websWrite(wp, "</tr>");
	}

	return ret;
}

static int
ej_autofw_port(int eid, webs_t wp, int argc, char_t **argv)
{
	int i, n, ret = 0;
	netconf_app_t app;
	bool valid;
	char port[] = "XXXXX";

	if (ejArgs(argc, argv, "%d %d", &i, &n) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (; i <= n; i++) {
		valid = get_autofw_port(i, &app);

		/* Parse out_proto:out_port,in_proto:in_start-in_end>to_start-to_end,enable,desc */
		ret += websWrite(wp, "<tr>");
		ret += websWrite(wp, "<td></td>");

		/* Print outbound protocol */
		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"autofw_port_out_proto%d\">", i);
		ret += websWrite(wp, "<option value=\"tcp\" %s>TCP</option>",
				 valid && app.match.ipproto == IPPROTO_TCP ? "selected" : "");
		ret += websWrite(wp, "<option value=\"udp\" %s>UDP</option>",
				 valid && app.match.ipproto == IPPROTO_UDP ? "selected" : "");
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td></td>");

		/* Print outbound port */
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(app.match.dst.ports[0]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"autofw_port_out_start%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td>-</td>");
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(app.match.dst.ports[1]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"autofw_port_out_end%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td></td>");

		/* Print related protocol */
		ret += websWrite(wp, "<td>");
		ret += websWrite(wp, "<select name=\"autofw_port_in_proto%d\">", i);
		ret += websWrite(wp, "<option value=\"tcp\" %s>TCP</option>",
				 valid && app.proto == IPPROTO_TCP ? "selected" : "");
		ret += websWrite(wp, "<option value=\"udp\" %s>UDP</option>",
				 valid && app.proto == IPPROTO_UDP ? "selected" : "");
		ret += websWrite(wp, "</select>");
		ret += websWrite(wp, "</td>");
		ret += websWrite(wp, "<td></td>");

		/* Print related destination port range */
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(app.dport[0]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"autofw_port_in_start%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td>-</td>");
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(app.dport[1]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"autofw_port_in_end%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td></td>");

		/* Print mapped destination port range */
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(app.to[0]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"autofw_port_to_start%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td>-</td>");
		if (valid)
			snprintf(port, sizeof(port), "%d", ntohs(app.to[1]));
		else
			*port = '\0';
		ret += websWrite(wp, "<td><input name=\"autofw_port_to_end%d\" value=\"%s\" size=\"5\" maxlength=\"5\"></td>",
				 i, port);
		ret += websWrite(wp, "<td></td>");

		/* Print enable */
		ret += websWrite(wp, "<td><input type=\"checkbox\" name=\"autofw_port_enable%d\" %s></td>",
				 i, valid && !(app.match.flags & NETCONF_DISABLED) ? "checked" : "");

		ret += websWrite(wp, "</tr>");
	}

	return ret;
}

/*
 * Example: 
 * lan_route=192.168.2.0:255.255.255.0:192.168.2.1:1
 * <% lan_route("ipaddr", 0); %> produces "192.168.2.0"
 */
static int
ej_lan_route(int eid, webs_t wp, int argc, char_t **argv)
{
	char *arg;
	int which;
	char word[256], *next;
	char *ipaddr, *netmask, *gateway, *metric;

	if (ejArgs(argc, argv, "%s %d", &arg, &which) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	foreach(word, nvram_safe_get("lan_route"), next) {
		if (which-- == 0) {
			netmask = word;
			ipaddr = strsep(&netmask, ":");
			if (!ipaddr || !netmask)
				continue;
			gateway = netmask;
			netmask = strsep(&gateway, ":");
			if (!netmask || !gateway)
				continue;
			metric = gateway;
			gateway = strsep(&metric, ":");
			if (!gateway || !metric)
				continue;
			if (!strcmp(arg, "ipaddr"))
				return websWrite(wp, ipaddr);
			else if (!strcmp(arg, "netmask"))
				return websWrite(wp, netmask);
			else if (!strcmp(arg, "gateway"))
				return websWrite(wp, gateway);
			else if (!strcmp(arg, "metric"))
				return websWrite(wp, metric);
		}
	}

	return 0;
}

/*
 * Example: 
 * wan_route=192.168.10.0:255.255.255.0:192.168.10.1:1
 * <% wan_route("ipaddr", 0); %> produces "192.168.10.0"
 */
static int
ej_wan_route(int eid, webs_t wp, int argc, char_t **argv)
{
	char *arg;
	int which;
	char word[256], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";


	if (ejArgs(argc, argv, "%s %d", &arg, &which) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if ((unit = atoi(nvram_safe_get("wan_unit"))) < 0)
		unit = 0;
	wan_prefix(unit, prefix);
	
	foreach(word, nvram_safe_get(strcat_r(prefix, "route", tmp)), next) {
		if (which-- == 0) {
			netmask = word;
			ipaddr = strsep(&netmask, ":");
			if (!ipaddr || !netmask)
				continue;
			gateway = netmask;
			netmask = strsep(&gateway, ":");
			if (!netmask || !gateway)
				continue;
			metric = gateway;
			gateway = strsep(&metric, ":");
			if (!gateway || !metric)
				continue;
			if (!strcmp(arg, "ipaddr"))
				return websWrite(wp, ipaddr);
			else if (!strcmp(arg, "netmask"))
				return websWrite(wp, netmask);
			else if (!strcmp(arg, "gateway"))
				return websWrite(wp, gateway);
			else if (!strcmp(arg, "metric"))
				return websWrite(wp, metric);
		}
	}

	return 0;
}

/* Return a list of the currently present wireless interfaces */
static int
ej_wl_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char name[IFNAMSIZ], *next;
	int unit, ret = 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";
	char *hwaddr, *ssid;
	char ifnames[256];

	snprintf(ifnames, sizeof(ifnames), "%s %s", 
		nvram_safe_get("lan_ifnames"),
		nvram_safe_get("wan_ifnames"));
	foreach(name, ifnames, next) {
		/* Probe for wl interfaces */
		if (wl_probe(name) ||
		    wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
			continue;

		/* Get configured SSID */
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		hwaddr = nvram_get(strcat_r(prefix, "hwaddr", tmp));
		ssid = nvram_get(strcat_r(prefix, "ssid", tmp));
		if (!hwaddr || !*hwaddr || !ssid || !*ssid)
			continue;

		ret += websWrite(wp, "<option value=\"%d\" %s>%s (%s)</option>", unit,
				 unit == atoi(nvram_safe_get("wl_unit")) ? "selected" : "",
				 ssid, hwaddr);
	}

	if (!ret)
		ret += websWrite(wp, "<option value=\"-1\" selected>None</option>");

	return ret;
}

/* Return a list of the supported bands on the currently selected wireless interface */
static int
ej_wl_phytypes(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit, ret = 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";
	char *phytype;
	char *phylist;
	int i;

	if ((unit = atoi(nvram_safe_get("wl_unit"))) < 0)
		return websWrite(wp, "None");

	/* Get available phy types of the currently selected wireless interface */
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	phylist = nvram_safe_get(strcat_r(prefix, "phytypes", tmp));

	/* Get configured phy type */
	phytype = websGetVar(wp, "wl_phytype", NULL);
	if (phytype == NULL)
		phytype = nvram_safe_get("wl_phytype");

	for (i = 0; i < strlen(phylist); i++) {
		ret += websWrite(wp, "<option value=\"%c\" %s>802.11%c (%s GHz)</option>",
				 phylist[i], phylist[i] == *phytype ? "selected" : "", phylist[i], 
				 phylist[i] == 'a' ? "5" : "2.4");
	}

	return ret;
}

/* Return a radio ID given a phy type */
static int
ej_wl_radioid(int eid, webs_t wp, int argc, char_t **argv)
{
	char *phytype, var[100], *next;
	int unit;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";
	int which;

	if (ejArgs(argc, argv, "%s", &phytype) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if ((unit = atoi(nvram_safe_get("wl_unit"))) < 0)
		return websWrite(wp, "None");
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	which = strcspn(nvram_safe_get(strcat_r(prefix, "phytypes", tmp)), phytype);
	foreach(var, nvram_safe_get(strcat_r(prefix, "radioids", tmp)), next) {
		if (which == 0)
			return websWrite(wp, var);
		which--;
	}

	return websWrite(wp, "None");
}

/* Return current core revision */
static int
ej_wl_corerev(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";

	if ((unit = atoi(nvram_safe_get("wl_unit"))) < 0)
		return websWrite(wp, "None");
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	return websWrite(wp, nvram_safe_get(strcat_r(prefix, "corerev", tmp)));
}

/* Return current wireless channel */
static int
ej_wl_cur_channel(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit, unit2;
	char name[IFNAMSIZ], *next;
	char ifnames[256];

	if ((unit2 = atoi(nvram_safe_get("wl_unit"))) < 0)
		return -1;

	snprintf(ifnames, sizeof(ifnames), "%s %s", 
		nvram_safe_get("lan_ifnames"),
		nvram_safe_get("wan_ifnames"));
	foreach(name, ifnames, next) {
		if (wl_probe(name) == 0 &&
		    wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit)) == 0 &&
		    unit == unit2) {
			channel_info_t ci;
			wl_ioctl(name, WLC_GET_CHANNEL, &ci, sizeof(ci));
			return websWrite(wp, "Current: %d", ci.target_channel);
		}
	}
	return -1;
}

/* Return current country */
static int
ej_wl_cur_country(int eid, webs_t wp, int argc, char_t **argv)
{
	int unit, unit2;
	char name[IFNAMSIZ], *next;
	char ifnames[256];

	if ((unit2 = atoi(nvram_safe_get("wl_unit"))) < 0)
		return -1;

	snprintf(ifnames, sizeof(ifnames), "%s %s", 
		nvram_safe_get("lan_ifnames"),
		nvram_safe_get("wan_ifnames"));
	foreach(name, ifnames, next) {
		if (wl_probe(name) == 0 &&
		    wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit)) == 0 &&
		    unit == unit2) {
			char buf[WLC_CNTRY_BUF_SZ];
			wl_ioctl(name, WLC_GET_COUNTRY, buf, sizeof(buf));
			return websWrite(wp, "Current: %s", buf);
		}
	}
	return -1;
}

/* Return current phytype */
static int
ej_wl_cur_phytype(int eid, webs_t wp, int argc, char_t **argv)
{
	char *value;

	/* Get configured phy type */
	value = nvram_safe_get("wl_phytype");

	return websWrite(wp, "Current: 802.11%s", value);
}

static char * 
wan_name(int unit, char *prefix, char *name, int len)
{
	char tmp[100], *desc;
	desc = nvram_safe_get(strcat_r(prefix, "desc", tmp));
	snprintf(tmp, sizeof(tmp), "Connection %d", unit + 1);
	snprintf(name, len, "%s", !strcmp(desc, "") ? tmp : desc);
	return name;
}

/* Return a list of wan connections (Connection <N>/<Connection Name>) */
static int
ej_wan_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit, ret = 0;

	/* build wan connection name list */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		wan_prefix(unit, prefix);
		if (!nvram_get(strcat_r(prefix, "unit", tmp)))
			continue;
		ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>", unit,
				unit == atoi(nvram_safe_get("wan_unit")) ? "selected" : "",
				wan_name(unit, prefix, tmp, sizeof(tmp)));
	}

	return ret;
}

struct variable {
	char *name;
	char *longname;
	void (*validate)(webs_t wp, char *value, struct variable *v);
	char **argv;
	int nullok;
};

#define ARGV(args...) ((char *[]) { args, NULL })
#define XSTR(s) STR(s)
#define STR(s) #s

static void
validate_list(webs_t wp, char *value, struct variable *v,
	      int (*valid)(webs_t, char *, struct variable *))
{
	int n, i;
	char name[100];
	char buf[1000] = "", *cur = buf;

	n = atoi(value);

	for (i = 0; i < n; i++) {
		snprintf(name, sizeof(name), "%s%d", v->name, i);
		if (!(value = websGetVar(wp, name, NULL)))
			return;
		if (!*value && v->nullok)
			continue;
		if (!valid(wp, value, v))
			continue;
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s",
				cur == buf ? "" : " ", value);
	}

	nvram_set(v->name, buf);
}	

static int
valid_ipaddr(webs_t wp, char *value, struct variable *v)
{
	unsigned int buf[4];
	struct in_addr ipaddr, netaddr, broadaddr, netmask;

	if (sscanf(value, "%d.%d.%d.%d", &buf[0], &buf[1], &buf[2], &buf[3]) != 4) {
		websWrite(wp, "Invalid <b>%s</b> %s: not an IP address<br>",
			  v->longname, value);
		return FALSE;
	}

	ipaddr.s_addr = htonl((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);

	if (v->argv) {
		(void) inet_aton(nvram_safe_get(v->argv[0]), &netaddr);
		(void) inet_aton(nvram_safe_get(v->argv[1]), &netmask);
		netaddr.s_addr &= netmask.s_addr;
		broadaddr.s_addr = netaddr.s_addr | ~netmask.s_addr;
		if (netaddr.s_addr != (ipaddr.s_addr & netmask.s_addr)) {
			websWrite(wp, "Invalid <b>%s</b> %s: not in the %s/",
				  v->longname, value, inet_ntoa(netaddr));
			websWrite(wp, "%s network<br>", inet_ntoa(netmask));
			return FALSE;
		}
		if (ipaddr.s_addr == netaddr.s_addr) {
			websWrite(wp, "Invalid <b>%s</b> %s: cannot be the network address<br>",
				  v->longname, value);
			return FALSE;
		}
		if (ipaddr.s_addr == broadaddr.s_addr) {
			websWrite(wp, "Invalid <b>%s</b> %s: cannot be the broadcast address<br>",
				  v->longname, value);
			return FALSE;
		}
	}

	return TRUE;
}

static void
validate_ipaddr(webs_t wp, char *value, struct variable *v)
{
	if (valid_ipaddr(wp, value, v))
		nvram_set(v->name, value);
}

static void
validate_ipaddrs(webs_t wp, char *value, struct variable *v)
{
	validate_list(wp, value, v, valid_ipaddr);
}

static int
valid_choice(webs_t wp, char *value, struct variable *v)
{
	char **choice;

	for (choice = v->argv; *choice; choice++) {
		if (!strcmp(value, *choice))
			return TRUE;
	}

	websWrite(wp, "Invalid <b>%s</b> %s: not one of ", v->longname, value);
	for (choice = v->argv; *choice; choice++)
		websWrite(wp, "%s%s", choice == v->argv ? "" : "/", *choice);
	websWrite(wp, "<br>");
	return FALSE;
}

static void
validate_choice(webs_t wp, char *value, struct variable *v)
{
	if (valid_choice(wp, value, v))
		nvram_set(v->name, value);
}

static int
valid_range(webs_t wp, char *value, struct variable *v)
{
	int n, start, end;

	n = atoi(value);
	start = atoi(v->argv[0]);
	end = atoi(v->argv[1]);

	if (n < start || n > end) {
		websWrite(wp, "Invalid <b>%s</b> %s: out of range %d-%d<br>",
			  v->longname, value, start, end);
		return FALSE;
	}

	return TRUE;
}

static void
validate_range(webs_t wp, char *value, struct variable *v)
{
	if (valid_range(wp, value, v))
		nvram_set(v->name, value);
}

static int
valid_name(webs_t wp, char *value, struct variable *v)
{
	int n, min, max;

	n = strlen(value);
	min = atoi(v->argv[0]);
	max = atoi(v->argv[1]);

	if (n > max) {
		websWrite(wp, "Invalid <b>%s</b> %s: longer than %d characters<br>",
			  v->longname, value, max);
		return FALSE;
	}
	else if (n < min) {
		websWrite(wp, "Invalid <b>%s</b> %s: shorter than %d characters<br>",
			  v->longname, value, min);
		return FALSE;
	}

	return TRUE;
}

static void
validate_name(webs_t wp, char *value, struct variable *v)
{
	if (valid_name(wp, value, v))
		nvram_set(v->name, value);
}

static int
valid_hwaddr(webs_t wp, char *value, struct variable *v)
{
	unsigned char hwaddr[6];

	/* Make exception for "NOT IMPLELEMENTED" string */
	if (!strcmp(value,"NOT_IMPLEMENTED")) 
		return(TRUE);

	/* Check for bad, multicast, broadcast, or null address */
	if (!ether_atoe(value, hwaddr) ||
	    (hwaddr[0] & 1) ||
	    (hwaddr[0] & hwaddr[1] & hwaddr[2] & hwaddr[3] & hwaddr[4] & hwaddr[5]) == 0xff ||
	    (hwaddr[0] | hwaddr[1] | hwaddr[2] | hwaddr[3] | hwaddr[4] | hwaddr[5]) == 0x00) {
		websWrite(wp, "Invalid <b>%s</b> %s: not a MAC address<br>",
			  v->longname, value);
		return FALSE;
	}

	return TRUE;
}

static void
validate_hwaddr(webs_t wp, char *value, struct variable *v)
{
	if (valid_hwaddr(wp, value, v))
		nvram_set(v->name, value);
}

static void
validate_hwaddrs(webs_t wp, char *value, struct variable *v)
{
	validate_list(wp, value, v, valid_hwaddr);
}

static void
validate_country(webs_t wp, char *value, struct variable *v)
{
	country_name_t *country;
	for(country = country_names; country->name; country++)
		if (!strcmp(value, country->abbrev))
			nvram_set(v->name, value);
}

static void
validate_dhcp(webs_t wp, char *value, struct variable *v)
{
	struct variable dhcp_variables[] = {
		{ longname: "DHCP Server Starting LAN IP Address", argv: ARGV("lan_ipaddr", "lan_netmask") },
		{ longname: "DHCP Server Ending LAN IP Address", argv: ARGV("lan_ipaddr", "lan_netmask") },
	};
	char *start, *end;

	if (!(start = websGetVar(wp, "dhcp_start", NULL)) ||
	    !(end = websGetVar(wp, "dhcp_end", NULL)))
		return;
	if (!*start) start = end;
	if (!*end) end = start;
	if (!*start && !*end && !strcmp(nvram_safe_get("lan_proto"), "dhcp")) {
		websWrite(wp, "Invalid <b>%s</b>: must specify a range<br>", v->longname);
		return;
	}
	if (!valid_ipaddr(wp, start, &dhcp_variables[0]) ||
	    !valid_ipaddr(wp, end, &dhcp_variables[1]))
		return;
	if (ntohl(inet_addr(start)) > ntohl(inet_addr(end))) {
		websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
			  dhcp_variables[0].longname, start, dhcp_variables[1].longname, end);
		return;
	}

	nvram_set("dhcp_start", start);
	nvram_set("dhcp_end", end);
}

static void
validate_lan_ipaddr(webs_t wp, char *value, struct variable *v)
{
	struct variable fields[] = {
		{ name: "lan_ipaddr", longname: "LAN IP Address" },
		{ name: "lan_netmask", longname: "LAN Subnet Mask" },
	};
	char *lan_ipaddr, *lan_netmask;
	struct in_addr ipaddr, netmask, netaddr, broadaddr;
	char *lan_ipaddrs[] = { "dhcp_start", "dhcp_end", "dmz_ipaddr" };
	netconf_filter_t start, end;
	netconf_nat_t nat;
	bool valid;
	int i;

	/* Basic validation */
	if (!(lan_ipaddr = websGetVar(wp, fields[0].name, NULL)) ||
	    !(lan_netmask = websGetVar(wp, fields[1].name, NULL)) ||
	    !valid_ipaddr(wp, lan_ipaddr, &fields[0]) ||
	    !valid_ipaddr(wp, lan_netmask, &fields[1]))
		return;

	/* Check for broadcast or network address */
	(void) inet_aton(lan_ipaddr, &ipaddr);
	(void) inet_aton(lan_netmask, &netmask);
	netaddr.s_addr = ipaddr.s_addr & netmask.s_addr;
	broadaddr.s_addr = netaddr.s_addr | ~netmask.s_addr;
	if (ipaddr.s_addr == netaddr.s_addr) {
		websWrite(wp, "Invalid <b>%s</b> %s: cannot be the network address<br>",
			  fields[0].longname, lan_ipaddr);
		return;
	}
	if (ipaddr.s_addr == broadaddr.s_addr) {
		websWrite(wp, "Invalid <b>%s</b> %s: cannot be the broadcast address<br>",
			  fields[0].longname, lan_ipaddr);
		return;
	}

	nvram_set("lan_ipaddr", lan_ipaddr);
	nvram_set("lan_netmask", lan_netmask);

	/* Fix up LAN IP addresses */
	for (i = 0; i < ARRAYSIZE(lan_ipaddrs); i++) {
		value = nvram_get(lan_ipaddrs[i]);
		if (value && *value) {
			(void) inet_aton(value, &ipaddr);
			ipaddr.s_addr &= ~netmask.s_addr;
			ipaddr.s_addr |= netaddr.s_addr;
			nvram_set(lan_ipaddrs[i], inet_ntoa(ipaddr));
		}
	}				

	/* Fix up client filters and port forwards */
	for (i = 0; i < MAX_NVPARSE; i++) {
		if (get_filter_client(i, &start, &end)) {
			start.match.src.ipaddr.s_addr &= ~netmask.s_addr;
			start.match.src.ipaddr.s_addr |= netaddr.s_addr;
			end.match.src.ipaddr.s_addr &= ~netmask.s_addr;
			end.match.src.ipaddr.s_addr |= netaddr.s_addr;
			valid = set_filter_client(i, &start, &end);
			a_assert(valid);
		}
		if (get_forward_port(i, &nat)) {
			nat.ipaddr.s_addr &= ~netmask.s_addr;
			nat.ipaddr.s_addr |= netaddr.s_addr;
			valid = set_forward_port(i, &nat);
			a_assert(valid);
		}
	}
}	

static void
validate_filter_client(webs_t wp, char *value, struct variable *v)
{
	int n, i, j;
	bool valid;
	struct variable fields[] = {
		{ name: "filter_client_from_start%d", longname: "LAN Client Filter Starting IP Address", argv: ARGV("lan_ipaddr", "lan_netmask") },
		{ name: "filter_client_from_end%d", longname: "LAN Client Filter Ending IP Address", argv: ARGV("lan_ipaddr", "lan_netmask") },
		{ name: "filter_client_proto%d", longname: "LAN Client Filter Protocol", argv: ARGV("tcp", "udp") },
		{ name: "filter_client_to_start%d", longname: "LAN Client Filter Starting Destination Port", argv: ARGV("0", "65535") },
		{ name: "filter_client_to_end%d", longname: "LAN Client Filter Ending Destination Port", argv: ARGV("0", "65535") },
		{ name: "filter_client_from_day%d", longname: "LAN Client Filter Starting Day", argv: ARGV("0", "6") },
		{ name: "filter_client_to_day%d", longname: "LAN Client Filter Ending Day", argv: ARGV("0", "6") },
		{ name: "filter_client_from_sec%d", longname: "LAN Client Filter Starting Second", argv: ARGV("0", "86400") },
		{ name: "filter_client_to_sec%d", longname: "LAN Client Filter Ending Second", argv: ARGV("0", "86400") },
	};
	char *from_start, *from_end, *proto, *to_start, *to_end, *from_day, *to_day, *from_sec, *to_sec, *enable;
	char **locals[] = { &from_start, &from_end, &proto, &to_start, &to_end, &from_day, &to_day, &from_sec, &to_sec };
	char name[1000];
	netconf_filter_t start, end;

	/* filter_client indicates how many to expect */
	if (!valid_range(wp, value, v))
		return;
	n = atoi(value);

	for (i = 0; i <= n; i++) {
		/* Set up field names */
		for (j = 0; j < ARRAYSIZE(fields); j++) {
			snprintf(name, sizeof(name), fields[j].name, i);
			if (!(*locals[j] = websGetVar(wp, name, NULL)))
				break;
		}
		/* Incomplete web page */
		if (j < ARRAYSIZE(fields))
			continue;
		/* Enable is a checkbox */
		snprintf(name, sizeof(name), "filter_client_enable%d", i);
		if (websGetVar(wp, name, NULL))
			enable = "on";
		else
			enable = "off";
		/* Delete entry if all fields are blank */
		if (!*from_start && !*from_end && !*to_start && !*to_end) {
			del_filter_client(i);
			continue;
		}
		/* Fill in empty fields with default values */
		if (!*from_start) from_start = from_end;
		if (!*from_end) from_end = from_start;
		if (!*to_start) to_start = to_end;
		if (!*to_end) to_end = to_start;
		if (!*from_start || !*from_end) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a LAN IP Address Range<br>", v->longname);
			continue;
		}
		if (!*to_start || !*to_end) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a Destination Port Range<br>", v->longname);
			continue;
		}
		/* Check individual fields */
		if (!valid_ipaddr(wp, from_start, &fields[0]) ||
		    !valid_ipaddr(wp, from_end, &fields[1]) ||
		    !valid_choice(wp, proto, &fields[2]) ||
		    !valid_range(wp, to_start, &fields[3]) ||
		    !valid_range(wp, to_end, &fields[4]) ||
		    !valid_range(wp, from_day, &fields[5]) ||
		    !valid_range(wp, to_day, &fields[6]) ||
		    !valid_range(wp, from_sec, &fields[7]) ||
		    !valid_range(wp, to_sec, &fields[8]))
			continue;
		/* Check dependencies between fields */
		if (ntohl(inet_addr(from_start)) > ntohl(inet_addr(from_end))) {
			websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
				  fields[0].longname, from_start, fields[1].longname, from_end);
			continue;
		}
		if (atoi(to_start) > atoi(to_end)) {
			websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
				  fields[3].longname, to_start, fields[4].longname, to_end);
			continue;
		}

		/* Set up parameters */
		memset(&start, 0, sizeof(netconf_filter_t));
		if (!strcmp(proto, "tcp"))
			start.match.ipproto = IPPROTO_TCP;
		else if (!strcmp(proto, "udp"))
			start.match.ipproto = IPPROTO_UDP;
		(void) inet_aton(from_start, &start.match.src.ipaddr);
		start.match.src.netmask.s_addr = htonl(0xffffffff);
		start.match.dst.ports[0] = htons(atoi(to_start));
		start.match.dst.ports[1] = htons(atoi(to_end));
		start.match.days[0] = atoi(from_day);
		start.match.days[1] = atoi(to_day);
		start.match.secs[0] = atoi(from_sec);
		start.match.secs[1] = atoi(to_sec);
		if (!strcmp(enable, "off"))
			start.match.flags |= NETCONF_DISABLED;
		memcpy(&end, &start, sizeof(netconf_filter_t));
		(void) inet_aton(from_end, &end.match.src.ipaddr);

		/* Do it */
		valid = set_filter_client(i, &start, &end);
		a_assert(valid);
	}
}

static void
validate_forward_port(webs_t wp, char *value, struct variable *v)
{
	int n, i, j;
	bool valid;
	struct variable fields[] = {
		{ name: "forward_port_proto%d", longname: "Port Forward Protocol", argv: ARGV("tcp", "udp") },
		{ name: "forward_port_from_start%d", longname: "Port Forward Starting WAN Port", argv: ARGV("0", "65535") },
		{ name: "forward_port_from_end%d", longname: "Port Forward Ending WAN Port", argv: ARGV("0", "65535") },
		{ name: "forward_port_to_ip%d", longname: "Port Forward LAN IP Address", argv: ARGV("lan_ipaddr", "lan_netmask") },
		{ name: "forward_port_to_start%d", longname: "Port Forward Starting LAN Port", argv: ARGV("0", "65535") },
		{ name: "forward_port_to_end%d", longname: "Port Forward Ending LAN Port", argv: ARGV("0", "65535") },
	};
	char *proto, *from_start, *from_end, *to_ip, *to_start, *to_end, *enable;
	char **locals[] = { &proto, &from_start, &from_end, &to_ip, &to_start, &to_end };
	char name[1000];
	netconf_nat_t nat;

	/* forward_port indicates how many to expect */
	if (!valid_range(wp, value, v))
		return;
	n = atoi(value);

	for (i = 0; i <= n; i++) {
		/* Set up field names */
		for (j = 0; j < ARRAYSIZE(fields); j++) {
			snprintf(name, sizeof(name), fields[j].name, i);
			if (!(*locals[j] = websGetVar(wp, name, NULL)))
				break;
		}
		/* Incomplete web page */
		if (j < ARRAYSIZE(fields))
			continue;
		/* Enable is a checkbox */
		snprintf(name, sizeof(name), "forward_port_enable%d", i);
		if (websGetVar(wp, name, NULL))
			enable = "on";
		else
			enable = "off";
		/* Delete entry if all fields are blank */
		if (!*from_start && !*from_end && !*to_ip && !*to_start && !*to_end) {
			del_forward_port(i);
			continue;
		}
		/* Fill in empty fields with default values */
		if (!*from_start) from_start = from_end;
		if (!*from_end) from_end = from_start;
		if (!*to_start && !*to_end)
			to_start = from_start;
		if (!*to_start) to_start = to_end;
		if (!*to_end) to_end = to_start;
		if (!*from_start || !*from_end) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a LAN IP Address Range<br>", v->longname);
			continue;
		}
		if (!*to_ip) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a LAN IP Address<br>", v->longname);
			continue;
		}
		if (!*to_start || !*to_end) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a Destination Port Range<br>", v->longname);
			continue;
		}
		/* Check individual fields */
		if (!valid_choice(wp, proto, &fields[0]) ||
		    !valid_range(wp, from_start, &fields[1]) ||
		    !valid_range(wp, from_end, &fields[2]) ||
		    !valid_ipaddr(wp, to_ip, &fields[3]) ||
		    !valid_range(wp, to_start, &fields[4]) ||
		    !valid_range(wp, to_end, &fields[5]))
			continue;
		if (atoi(from_start) > atoi(from_end)) {
			websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
				  fields[1].longname, from_start, fields[2].longname, from_end);
			continue;
		}
		if (atoi(to_start) > atoi(to_end)) {
			websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
				  fields[4].longname, to_start, fields[5].longname, to_end);
			continue;
		}
		if ((atoi(from_end) - atoi(from_start)) != (atoi(to_end) - atoi(to_start))) {
			websWrite(wp, "Invalid <b>%s</b>: WAN Port Range and LAN Port Range must be the same size<br>", v->longname);
			continue;
		}

		/* Set up parameters */
		memset(&nat, 0, sizeof(netconf_nat_t));
		if (!strcmp(proto, "tcp"))
			nat.match.ipproto = IPPROTO_TCP;
		else if (!strcmp(proto, "udp"))
			nat.match.ipproto = IPPROTO_UDP;
		nat.match.dst.ports[0] = htons(atoi(from_start));
		nat.match.dst.ports[1] = htons(atoi(from_end));
		(void) inet_aton(to_ip, &nat.ipaddr);
		nat.ports[0] = htons(atoi(to_start));
		nat.ports[1] = htons(atoi(to_end));
		if (!strcmp(enable, "off"))
			nat.match.flags |= NETCONF_DISABLED;

		/* Do it */
		valid = set_forward_port(i, &nat);
		a_assert(valid);
	}
}

static void
validate_autofw_port(webs_t wp, char *value, struct variable *v)
{
	int n, i, j;
	bool valid;
	struct variable fields[] = {
		{ name: "autofw_port_out_proto%d", longname: "Outbound Protocol", argv: ARGV("tcp", "udp") },
		{ name: "autofw_port_out_start%d", longname: "Outbound Port Start", argv: ARGV("0", "65535") },
		{ name: "autofw_port_out_end%d", longname: "Outbound Port End", argv: ARGV("0", "65535") },
		{ name: "autofw_port_in_proto%d", longname: "Inbound Protocol", argv: ARGV("tcp", "udp") },
		{ name: "autofw_port_in_start%d", longname: "Inbound Port Start", argv: ARGV("0", "65535") },
		{ name: "autofw_port_in_end%d", longname: "Inbound Port End", argv: ARGV("0", "65535") },
 		{ name: "autofw_port_to_start%d", longname: "To Port Start", argv: ARGV("0", "65535") },
 		{ name: "autofw_port_to_end%d", longname: "To Port End", argv: ARGV("0", "65535") },
	};
	char *out_proto, *out_start, *out_end, *in_proto, *in_start, *in_end, *to_start, *to_end, *enable;
	char **locals[] = { &out_proto, &out_start, &out_end, &in_proto, &in_start, &in_end, &to_start, &to_end };
	char name[1000];
	netconf_app_t app;

	/* autofw_port indicates how many to expect */
	if (!valid_range(wp, value, v))
		return;
	n = atoi(value);

	for (i = 0; i <= n; i++) {
		/* Set up field names */
		for (j = 0; j < ARRAYSIZE(fields); j++) {
			snprintf(name, sizeof(name), fields[j].name, i);
			if (!(*locals[j] = websGetVar(wp, name, NULL)))
				break;
		}
		/* Incomplete web page */
		if (j < ARRAYSIZE(fields))
			continue;
		/* Enable is a checkbox */
		snprintf(name, sizeof(name), "autofw_port_enable%d", i);
		if (websGetVar(wp, name, NULL))
			enable = "on";
		else
			enable = "off";
		/* Delete entry if all fields are blank */
		if (!*out_start && !*out_end && !*in_start && !*in_end && !*to_start && !*to_end) {
			del_autofw_port(i);
			continue;
		}
		/* Fill in empty fields with default values */
		if (!*out_start) out_start = out_end;
		if (!*out_end) out_end = out_start;
		if (!*in_start) in_start = in_end;
		if (!*in_end) in_end = in_start;
		if (!*to_start && !*to_end)
			to_start = in_start;
		if (!*to_start) to_start = to_end;
		if (!*to_end) to_end = to_start;
		if (!*out_start || !*out_end) {
			websWrite(wp, "Invalid <b>%s</b>: must specify an Outbound Port Range<br>", v->longname);
			continue;
		}
		if (!*in_start || !*in_end) {
			websWrite(wp, "Invalid <b>%s</b>: must specify an Inbound Port Range<br>", v->longname);
			continue;
		}
		if (!*to_start || !*to_end) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a To Port Range<br>", v->longname);
			continue;
		}
		/* Check individual fields */
		if (!valid_choice(wp, out_proto, &fields[0]) ||
		    !valid_range(wp, out_start, &fields[1]) ||
		    !valid_range(wp, out_end, &fields[2]) ||
		    !valid_choice(wp, in_proto, &fields[3]) ||
		    !valid_range(wp, in_start, &fields[4]) ||
		    !valid_range(wp, in_end, &fields[5]) ||
		    !valid_range(wp, to_start, &fields[6]) ||
		    !valid_range(wp, to_end, &fields[7]))
			continue;
		/* Check dependencies between fields */
		if (atoi(out_start) > atoi(out_end)) {
			websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
				  fields[1].longname, out_start, fields[2].longname, out_end);
			continue;
		}
		if (atoi(in_start) > atoi(in_end)) {
			websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
				  fields[4].longname, in_start, fields[5].longname, in_end);
			continue;
		}
		if (atoi(to_start) > atoi(to_end)) {
			websWrite(wp, "Invalid <b>%s</b> %s: greater than <b>%s</b> %s<br>",
				  fields[6].longname, in_start, fields[7].longname, in_end);
			continue;
		}
		if ((atoi(in_end) - atoi(in_start)) != (atoi(to_end) - atoi(to_start))) {
			websWrite(wp, "Invalid <b>%s</b>: Inbound Port Range and To Port Range must be the same size<br>", v->longname);
			continue;
		}

		/* Set up parameters */
		memset(&app, 0, sizeof(netconf_app_t));
		if (!strcmp(out_proto, "tcp"))
			app.match.ipproto = IPPROTO_TCP;
		else if (!strcmp(out_proto, "udp"))
			app.match.ipproto = IPPROTO_UDP;
		app.match.dst.ports[0] = htons(atoi(out_start));
		app.match.dst.ports[1] = htons(atoi(out_end));
		if (!strcmp(in_proto, "tcp"))
			app.proto = IPPROTO_TCP;
		else if (!strcmp(in_proto, "udp"))
			app.proto = IPPROTO_UDP;
		app.dport[0] = htons(atoi(in_start));
		app.dport[1] = htons(atoi(in_end));
		app.to[0] = htons(atoi(to_start));
		app.to[1] = htons(atoi(to_end));
		if (!strcmp(enable, "off"))
			app.match.flags |= NETCONF_DISABLED;

		/* Do it */
		valid = set_autofw_port(i, &app);
		a_assert(valid);
	}
}

static void
validate_lan_route(webs_t wp, char *value, struct variable *v)
{
	int n, i;
	char buf[1000] = "", *cur = buf;
	struct variable lan_route_variables[] = {
		{ longname: "Route IP Address", argv: NULL },
		{ longname: "Route Subnet Mask", argv: NULL },
		{ longname: "Route Gateway", argv: NULL },
		{ longname: "Route Metric", argv: ARGV("0", "15") },
	};

	n = atoi(value);

	for (i = 0; i < n; i++) {
		char lan_route_ipaddr[] = "lan_route_ipaddrXXX";
		char lan_route_netmask[] = "lan_route_netmaskXXX";
		char lan_route_gateway[] = "lan_route_gatewayXXX";
		char lan_route_metric[] = "lan_route_metricXXX";
		char *ipaddr, *netmask, *gateway, *metric;

 		snprintf(lan_route_ipaddr, sizeof(lan_route_ipaddr), "%s_ipaddr%d", v->name, i);
		snprintf(lan_route_netmask, sizeof(lan_route_netmask), "%s_netmask%d", v->name, i);
 		snprintf(lan_route_gateway, sizeof(lan_route_gateway), "%s_gateway%d", v->name, i);
 		snprintf(lan_route_metric, sizeof(lan_route_metric), "%s_metric%d", v->name, i);
		if (!(ipaddr = websGetVar(wp, lan_route_ipaddr, NULL)) ||
		    !(netmask = websGetVar(wp, lan_route_netmask, NULL)) ||
		    !(gateway = websGetVar(wp, lan_route_gateway, NULL)) ||
		    !(metric = websGetVar(wp, lan_route_metric, NULL)))
			return;
		if (!*ipaddr && !*netmask && !*gateway && !*metric)
			continue;
		if (!*ipaddr && !*netmask && *gateway) {
			ipaddr = "0.0.0.0";
			netmask = "0.0.0.0";
		}
		if (!*gateway)
			gateway = "0.0.0.0";
		if (!*metric)
			metric = "0";
		if (!*ipaddr) {
			websWrite(wp, "Invalid <b>%s</b>: must specify an IP Address<br>", v->longname);
			continue;
		}
		if (!*netmask) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a Subnet Mask<br>", v->longname);
			continue;
		}
		lan_route_variables[2].argv = ARGV("lan_ipaddr", "lan_netmask");
		if (!valid_ipaddr(wp, ipaddr, &lan_route_variables[0]) ||
		    !valid_ipaddr(wp, netmask, &lan_route_variables[1]) ||
		    !valid_ipaddr(wp, gateway, &lan_route_variables[2]) ||
		    !valid_range(wp, metric, &lan_route_variables[3]))
			continue;
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s:%s:%s:%s",
				cur == buf ? "" : " ", ipaddr, netmask, gateway, metric);
	}

	nvram_set(v->name, buf);
}

static void
validate_wan_route(webs_t wp, char *value, struct variable *v)
{
	int n, i;
	char buf[1000] = "", *cur = buf;
	struct variable wan_route_variables[] = {
		{ longname: "Route IP Address", argv: NULL },
		{ longname: "Route Subnet Mask", argv: NULL },
		{ longname: "Route Gateway", argv: NULL },
		{ longname: "Route Metric", argv: ARGV("0", "15") },
	};

	n = atoi(value);

	for (i = 0; i < n; i++) {
		char wan_route_ipaddr[] = "wan_route_ipaddrXXX";
		char wan_route_netmask[] = "wan_route_netmaskXXX";
		char wan_route_gateway[] = "wan_route_gatewayXXX";
		char wan_route_metric[] = "wan_route_metricXXX";
		char *ipaddr, *netmask, *gateway, *metric;

 		snprintf(wan_route_ipaddr, sizeof(wan_route_ipaddr), "%s_ipaddr%d", v->name, i);
		snprintf(wan_route_netmask, sizeof(wan_route_netmask), "%s_netmask%d", v->name, i);
 		snprintf(wan_route_gateway, sizeof(wan_route_gateway), "%s_gateway%d", v->name, i);
 		snprintf(wan_route_metric, sizeof(wan_route_metric), "%s_metric%d", v->name, i);
		if (!(ipaddr = websGetVar(wp, wan_route_ipaddr, NULL)) ||
		    !(netmask = websGetVar(wp, wan_route_netmask, NULL)) ||
		    !(gateway = websGetVar(wp, wan_route_gateway, NULL)) ||
		    !(metric = websGetVar(wp, wan_route_metric, NULL)))
			continue;
		if (!*ipaddr && !*netmask && !*gateway && !*metric)
			continue;
		if (!*ipaddr && !*netmask && *gateway) {
			ipaddr = "0.0.0.0";
			netmask = "0.0.0.0";
		}
		if (!*gateway)
			gateway = "0.0.0.0";
		if (!*metric)
			metric = "0";
		if (!*ipaddr) {
			websWrite(wp, "Invalid <b>%s</b>: must specify an IP Address<br>", v->longname);
			continue;
		}
		if (!*netmask) {
			websWrite(wp, "Invalid <b>%s</b>: must specify a Subnet Mask<br>", v->longname);
			continue;
		}
		if (!valid_ipaddr(wp, ipaddr, &wan_route_variables[0]) ||
		    !valid_ipaddr(wp, netmask, &wan_route_variables[1]) ||
		    !valid_ipaddr(wp, gateway, &wan_route_variables[2]) ||
		    !valid_range(wp, metric, &wan_route_variables[3]))
			continue;
		cur += snprintf(cur, buf + sizeof(buf) - cur, "%s%s:%s:%s:%s",
				cur == buf ? "" : " ", ipaddr, netmask, gateway, metric);
	}

	nvram_set(v->name, buf);
}

static void
validate_wl_auth_mode(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;

	if (strcmp(value, "disabled") && atoi(nvram_safe_get("wl_gmode")) == GMODE_AFTERBURNER) {
		websWrite(wp, "<br>Invalid <b>%s</b>: must be set to <b>Disabled</b> when 54g Mode is AfterBurner.", v->longname);
		return;
	}

	if (!strcmp(value, "radius") || !strcmp(value, "wpa")) {
		if (nvram_match("wl_radius_ipaddr", "")) {
			websWrite(wp, "Invalid <b>%s</b>: must first specify a valid <b>RADIUS Server</b><br>", v->longname);
			return;
		}
	}

	if (!strcmp(value, "psk")) {
		if (nvram_match("wl_wpa_psk", "")) {
			websWrite(wp, "Invalid <b>%s</b>: must first specify a valid <b>WPA Pre-Shared Key</b><br>", v->longname);
			return;
		}
	} 

	if (!strcmp(value, "wpa") || !strcmp(value, "psk")) {
		if (!nvram_match("wl_wep", "tkip") && !nvram_match("wl_wep", "aes") &&
		    !nvram_match("wl_wep", "tkip+aes")) {
			websWrite(wp, "Invalid <b>%s</b>: <b>Data Encryption</b> mode must be TKIP or AES or TKIP+AES<br>", v->longname);
			return;
		}
		nvram_set("wl_auth", "0");
	} else {
		if (!nvram_match("wl_wep", "off") && !nvram_match("wl_wep", "wep")) {
			websWrite(wp, "Invalid <b>%s</b>: <b>Data Encryption</b> mode must be Off or WEP<br>", v->longname);
			return;
		}
	}			

	nvram_set(v->name, value);
}

static void
validate_wl_wpa_psk(webs_t wp, char *value, struct variable *v)
{
	int len = strlen(value);
	char *c;

	if (len == 64) {
		for (c = value; *c; c++) {
			if (!isxdigit((int) *c)) {
				websWrite(wp, "Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>", v->longname, *c);
				return;
			}
		}
	} else if (len < 8 || len > 63) {
		websWrite(wp, "Invalid <b>%s</b>: must be between 8 and 63 ASCII characters or 64 hexadecimal digits<br>", v->longname);
		return;
	}

	nvram_set(v->name, value);
}

static void
validate_wl_key(webs_t wp, char *value, struct variable *v)
{
	char *c;

	switch (strlen(value)) {
	case 5:
	case 13:			
		break;
	case 10:
	case 26:
		for (c = value; *c; c++) {
			if (!isxdigit((int) *c)) {
				websWrite(wp, "Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>", v->longname, *c);
				return;
			}
		}
		break;
	default:
		websWrite(wp, "Invalid <b>%s</b>: must be 5 or 13 ASCII characters or 10 or 26 hexadecimal digits<br>", v->longname);
		return;
	}

	nvram_set(v->name, value);
}

static void
validate_wl_wep(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;

#ifdef ABURN_WSEC_CHECK
	if (strcmp(value, "off") && atoi(nvram_safe_get("wl_gmode")) == GMODE_AFTERBURNER) {
		websWrite(wp, "<br>Invalid <b>%s</b>: must be set to <b>Off</b> when 54g Mode is AfterBurner.", v->longname);
		return;
	}
#endif

	if (!strcmp(value, "wep") || !strcmp(value, "on") || !strcmp(value, "restricted")) {
		char wl_key[] = "wl_keyXXX";

		snprintf(wl_key, sizeof(wl_key), "wl_key%s", nvram_safe_get("wl_key"));
		if (!strlen(nvram_safe_get(wl_key))) {
			websWrite(wp, "Invalid <b>%s</b>: must first specify a valid <b>Network Key</b><br>", v->longname);
			return;
		}
	}

	nvram_set(v->name, value);
}

static void
validate_wl_auth(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;

	if (atoi(value) == 1) {
		char wl_key[] = "wl_keyXXX";

		snprintf(wl_key, sizeof(wl_key), "wl_key%s", nvram_safe_get("wl_key"));
		if (!strlen(nvram_safe_get(wl_key))) {
			websWrite(wp, "Invalid <b>%s</b>: must first specify a valid <b>Network Key</b><br>", v->longname);
			return;
		}
	}

	nvram_set(v->name, value);
}

static void
validate_wan_ifname(webs_t wp, char *value, struct variable *v)
{
	char ifname[64], *next;
	foreach (ifname, nvram_safe_get("wan_ifnames"), next)
		if (!strcmp(ifname, value)) {
			nvram_set(v->name, value);
			return;
		}
	websWrite(wp, "Invalid <b>%s</b>: must be one of <b>%s</b><br>", v->longname, nvram_safe_get("wan_ifnames"));
}

static void
validate_wl_gmode(webs_t wp, char *value, struct variable *v)
{
	if (!valid_choice(wp, value, v))
		return;

	/* force certain wireless variables to fixed values */
	if (atoi(value) == GMODE_AFTERBURNER) {
		if (
		    nvram_invmatch("wl_auth_mode", "disabled") ||
#ifdef ABURN_WSEC_CHECK
		    nvram_invmatch("wl_wep", "off") ||
#endif
		    nvram_invmatch("wl_mode", "ap") ||
		    nvram_invmatch("wl_lazywds", "0") ||
		    nvram_invmatch("wl_wds", "")) {
			/* notify the user */
#ifdef ABURN_WSEC_CHECK
			websWrite(wp, "Invalid <b>%s</b>: AfterBurner mode requires:"
				"<br><b>Network Authentication</b> set to <b>Disabled</b>"
				"<br><b>Data Encryption</b> set to <b>Off</b>"
				"<br><b>Mode</b> set to <b>Access Point</b>"
				"<br><b>Bridge Restrict</b> set to <b>Enabled</b>"
				"<br><b>Remote Bridges</b> set to empty."
				"<br>", v->longname);
#else
			websWrite(wp, "Invalid <b>%s</b>: AfterBurner mode requires:"
				"<br><b>Network Authentication</b> set to <b>Disabled</b>"
				"<br><b>Mode</b> set to <b>Access Point</b>"
				"<br><b>Bridge Restrict</b> set to <b>Enabled</b>"
				"<br><b>Remote Bridges</b> set to empty."
				"<br>", v->longname);
#endif
			return;
		}
	}

	nvram_set(v->name, value);
}

static void
validate_wl_lazywds(webs_t wp, char *value, struct variable *v)
{
	char *gmode;
	if (strcmp(value, "0") &&
	    (gmode = websGetVar(wp, "wl_gmode", NULL)) && atoi(gmode) == GMODE_AFTERBURNER) {
		websWrite(wp, "Invalid <b>%s</b>: must be set to <b>Enabled</b> when 54g Mode is AfterBurner.<br>", v->longname);
		return;
	}
	validate_choice(wp, value, v);
}
	
static void
validate_wl_wds_hwaddrs(webs_t wp, char *value, struct variable *v)
{
	char *gmode;
	if ((gmode = websGetVar(wp, "wl_gmode", NULL)) && atoi(gmode) == GMODE_AFTERBURNER) {
		int n, i;
		char name[100], *mac;

		n = atoi(value);

		for (i = 0; i < n; i++) {
			snprintf(name, sizeof(name), "%s%d", v->name, i);
			if (!(mac = websGetVar(wp, name, NULL)) || strlen(mac) == 0)
				continue;
			websWrite(wp, "Invalid <b>%s</b>: must be empty when 54g Mode is AfterBurner.<br>", v->longname);
			return;
		}
	}
	validate_list(wp, value, v, valid_hwaddr);
}

static void
validate_wl_mode(webs_t wp, char *value, struct variable *v)
{
	char *gmode;
	if (strcmp(value, "ap") &&
	    (gmode = websGetVar(wp, "wl_gmode", NULL)) && atoi(gmode) == GMODE_AFTERBURNER) {
		websWrite(wp, "Invalid <b>%s</b>: must be set to <b>Access Point</b> when 54g Mode is AfterBurner.<br>", v->longname);
		return;
	}
	validate_choice(wp, value, v);
}
	
struct variable variables[];
extern struct nvram_tuple router_defaults[];

/* Hook to write wl_* default set through to wl%d_* variable set */
static void
wl_unit(webs_t wp, char *value, struct variable *v)
{
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";

	/* Do not write through if no interfaces are present */
	if (atoi(value) < 0)
		return;

	/* Set prefix */
	snprintf(prefix, sizeof(prefix), "wl%d_", atoi(value));

	/* Write through to selected variable set */
	for (; v >= variables && !strncmp(v->name, "wl_", 3); v--)
		nvram_set(strcat_r(prefix, &v->name[3], tmp), nvram_safe_get(v->name));
}

static void
wan_primary(webs_t wp)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int i;
	for (i = 0; i < MAX_NVPARSE; i ++) {
		/* skip non-exist and disabled connection */
		wan_prefix(i, prefix);
		if (!nvram_get(strcat_r(prefix, "unit", tmp))||
		    nvram_match(strcat_r(prefix, "proto", tmp), "disabled"))
			continue;
		/* make connection <i> primary */
		nvram_set(strcat_r(prefix, "primary", tmp), "1");
		/* notify the user */
		websWrite(wp, "<br><b>%s</b> is set to primary.",
			wan_name(i, prefix, tmp, sizeof(tmp)));
		break;
	}
}

/* Hook to write wan_* default set through to wan%d_* variable set */
static void
wan_unit(webs_t wp, char *value, struct variable *v)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char pppx[] = "pppXXXXXXXXXXX";
	int unit, i;
	char *wan_ifname;
	int wan_disabled = 0;
	int wan_prim = 0;
	int wan_wildcard = 0;
	char *wan_pppoe_service;
	char *wan_pppoe_ac;
	char wan_tmp[100];
	int wildcard;
	char *pppoe_service;
	char *pppoe_ac;
	char ea[ETHER_ADDR_LEN], wan_ea[ETHER_ADDR_LEN];

	/* Do not write through if no connections are present */
	if ((unit = atoi(value)) < 0)
		return;

	/* override wan_pppoe_ifname */
	if (nvram_match("wan_proto", "pppoe")) {
		snprintf(pppx, sizeof(pppx), "ppp%d", unit);
		nvram_set("wan_pppoe_ifname", pppx);
	}

	/*
	* Need to make sure this connection can co-exist with others.
	* Disable others if it can't (assuming this is the wanted one).
	* Disabled connection is for sure no problem to co-exist with
	* other connections.
	*/
	if (nvram_match("wan_proto", "disabled")) {
		/* Non primary always go with disabled connection. */
		nvram_set("wan_primary", "0");
		wan_disabled = 1;
	}
	/*
	* PPPoE connection is for sure no problem to co-exist with
	* other PPPoE connections even when they share the same 
	* ethernet interface, but we need to make sure certain 
	* PPPoE parameters are reasonablely different from eatch other 
	* if they share the same ethernet interface.
	*/
	else if (nvram_match("wan_proto", "pppoe")) {
		/* must disable others if this connection is wildcard (any service any ac) */
		wan_pppoe_service = nvram_get("wan_pppoe_service");
		wan_pppoe_ac = nvram_get("wan_pppoe_ac");
		wan_wildcard = (wan_pppoe_service == NULL || *wan_pppoe_service == 0) &&
			(wan_pppoe_ac == NULL || *wan_pppoe_ac == 0);
		wan_ifname = nvram_safe_get("wan_ifname");
		wan_name(unit, "wan_", wan_tmp, sizeof(wan_tmp));
		/* check all PPPoE connections that share the same interface */
		for (i = 0; i < MAX_NVPARSE; i ++) {
			/* skip the current connection */
			if (i == unit)
				continue;
			/* skip non-exist and connection that does not share the same i/f */
			wan_prefix(i, prefix);
			if (!nvram_get(strcat_r(prefix, "unit", tmp)) ||
			    nvram_match(strcat_r(prefix, "proto", tmp), "disabled") ||
			    nvram_invmatch(strcat_r(prefix, "ifname", tmp), wan_ifname))
				continue;
			/* PPPoE can share the same i/f, but none can be wildcard */
			if (nvram_match(strcat_r(prefix, "proto", tmp), "pppoe")) {
				if (wan_wildcard) {
					/* disable connection <i> */
					nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
					nvram_set(strcat_r(prefix, "primary", tmp), "0");
					/* notify the user */
					websWrite(wp, "<br><b>%s</b> is <b>disabled</b> because both "
						"<b>PPPoE Service Name</b> and <b>PPPoE Access Concentrator</b> "
						"in <b>%s</b> are empty.",
						wan_name(i, prefix, tmp, sizeof(tmp)), wan_tmp);
				}
				else {
					pppoe_service = nvram_get(strcat_r(prefix, "pppoe_service", tmp));
					pppoe_ac = nvram_get(strcat_r(prefix, "pppoe_ac", tmp));
					wildcard = (pppoe_service == NULL || *pppoe_service == 0) &&
						(pppoe_ac == NULL || *pppoe_ac == 0);
					/* allow connection <i> if certain pppoe parameters are not all same */
					if (!wildcard &&
					    (nvram_invmatch(strcat_r(prefix, "pppoe_service", tmp), nvram_safe_get("wan_pppoe_service")) ||
					         nvram_invmatch(strcat_r(prefix, "pppoe_ac", tmp), nvram_safe_get("wan_pppoe_ac"))))
						continue;
					/* disable connection <i> */
					nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
					nvram_set(strcat_r(prefix, "primary", tmp), "0");
					/* notify the user */
					websWrite(wp, "<br><b>%s</b> is <b>disabled</b> because both its "
						"<b>PPPoE Service Name</b> and <b>PPPoE Access Concentrator</b> "
						"are empty.",
						wan_name(i, prefix, tmp, sizeof(tmp)));
				}
			}
			/* other types can't (?) share the same i/f with PPPoE */
			else {
				/* disable connection <i> */
				nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
				nvram_set(strcat_r(prefix, "primary", tmp), "0");
				/* notify the user */
				websWrite(wp, "<br><b>%s</b> is <b>disabled</b> because it can't  "
					"share the same interface with <b>%s</b>.",
					wan_name(i, prefix, tmp, sizeof(tmp)), wan_tmp);
			}
		}
	}
	/* 
	* All other types (now DHCP, Static) can't co-exist with 
	* other connections if they use the same ethernet i/f.
	*/
	else {
		wan_ifname = nvram_safe_get("wan_ifname");
		wan_name(unit, "wan_", wan_tmp, sizeof(wan_tmp));
		/* check all connections that share the same interface */
		for (i = 0; i < MAX_NVPARSE; i ++) {
			/* skip the current connection */
			if (i == unit)
				continue;
			/* check if connection <i> exists and share the same i/f*/
			wan_prefix(i, prefix);
			if (!nvram_get(strcat_r(prefix, "unit", tmp)) ||
			    nvram_match(strcat_r(prefix, "proto", tmp), "disabled") ||
			    nvram_invmatch(strcat_r(prefix, "ifname", tmp), wan_ifname))
				continue;
			/* disable connection <i> */
			nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
			nvram_set(strcat_r(prefix, "primary", tmp), "0");
			/* notify the user */
			websWrite(wp, "<br><b>%s</b> is disabled because it can't share "
				"the ethernet interface with <b>%s</b>.",
				wan_name(i, prefix, tmp, sizeof(tmp)), wan_tmp);
		}
	}

	/*
	* Check if MAC address has been changed. Need to sync it to all connections
	* that share the same i/f if it is changed.
	*/
	wan_prefix(unit, prefix);
	ether_atoe(nvram_safe_get("wan_hwaddr"), wan_ea);
	ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), ea);
	if (memcmp(ea, wan_ea, ETHER_ADDR_LEN)) {
		wan_ifname = nvram_safe_get("wan_ifname");
		wan_name(unit, "wan_", wan_tmp, sizeof(wan_tmp));
		/* sync all connections that share the same interface */
		for (i = 0; i < MAX_NVPARSE; i ++) {
			/* skip the current connection */
			if (i == unit)
				continue;
			/* check if connection <i> exists and share the same i/f*/
			wan_prefix(i, prefix);
			if (!nvram_get(strcat_r(prefix, "unit", tmp)) ||
			    nvram_invmatch(strcat_r(prefix, "ifname", tmp), wan_ifname))
				continue;
			/* check if connection <i>'s hardware address is different */
			if (ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), ea) &&
			    !memcmp(ea, wan_ea, ETHER_ADDR_LEN))
			    continue;
			/* change connection <i>'s hardware address */
			nvram_set(strcat_r(prefix, "hwaddr", tmp), nvram_safe_get("wan_hwaddr"));
			/* notify the user */
			websWrite(wp, "<br><b>MAC Address</b> in <b>%s</b> is changed to "
				"<b>%s</b> because it shares the ethernet interface with <b>%s</b>.",
				wan_name(i, prefix, tmp, sizeof(tmp)), nvram_safe_get("wan_hwaddr"),
				wan_tmp);
		}
	}

	/* Set prefix */
	wan_prefix(unit, prefix);

	/* Write through to selected variable set */
	for (; v >= variables && !strncmp(v->name, "wan_", 4); v--)
		nvram_set(strcat_r(prefix, &v->name[4], tmp), nvram_safe_get(v->name));

	/*
	* There must be one and only one primary connection among all 
	* enabled connections so that traffic can be routed by default 
	* through the primary connection unless they are targetted to
	* a specific connection by means of static routes. (Primary ~=
	* Default Gateway).
	*/
	/* the current connection is primary, set others to non-primary */
	if (!wan_disabled && nvram_match(strcat_r(prefix, "primary", tmp), "1")) {
		/* set other connections to non-primary */
		for (i = 0; i < MAX_NVPARSE; i ++) {
			/* skip the current connection */
			if (i == unit)
				continue;
			/* skip non-exist and disabled connection */
			wan_prefix(i, prefix);
			if (!nvram_get(strcat_r(prefix, "unit", tmp)) ||
			    nvram_match(strcat_r(prefix, "proto", tmp), "disabled"))
				continue;
			/* skip non-primary connection */
			if (nvram_invmatch(strcat_r(prefix, "primary", tmp), "1"))
				continue;
			/* force primary to non-primary */
			nvram_set(strcat_r(prefix, "primary", tmp), "0");
			/* notify the user */
			websWrite(wp, "<br><b>%s</b> is set to non-primary.",
				wan_name(i, prefix, tmp, sizeof(tmp)));
		}
		wan_prim = 1;
	}
	/* the current connection is not parimary, check if there is any primary */
	else {
		/* check other connections to see if there is any primary */
		for (i = 0; i < MAX_NVPARSE; i ++) {
			/* skip the current connection */
			if (i == unit)
				continue;
			/* primary connection exists, honor it */
			wan_prefix(i, prefix);
			if (nvram_match(strcat_r(prefix, "primary", tmp), "1")) {
				wan_prim = 1;
				break;
			}
		}
	}
	/* no one is primary, pick the first enabled one as primary */
	if (!wan_prim)
		wan_primary(wp);
}

/* 
 * Variables are set in order (put dependent variables later). Set
 * nullok to TRUE to ignore zero-length values of the variable itself.
 * For more complicated validation that cannot be done in one pass or
 * depends on additional form components or can throw an error in a
 * unique painful way, write your own validation routine and assign it
 * to a hidden variable (e.g. filter_ip).
 */
struct variable variables[] = {
	/* basic settings */
	{ "http_username", "Router Username", validate_name, ARGV("0", "63"), TRUE },
	{ "http_passwd", "Router Password", validate_name, ARGV("0", "63"), TRUE },
	{ "http_wanport", "Router WAN Port", validate_range, ARGV("0", "65535"), TRUE },
	{ "router_disable", "Router Mode", validate_choice, ARGV("0", "1"), FALSE },
	{ "fw_disable", "Firewall", validate_choice, ARGV("0", "1"), FALSE },
	{ "time_zone", "Time Zone", validate_choice, ARGV("PST8PDT", "MST7MDT", "CST6CDT", "EST5EDT"), FALSE },
	{ "upnp_enable", "UPnP", validate_choice, ARGV("0", "1"), FALSE },
	{ "ntp_server", "NTP Servers", validate_ipaddrs, NULL, TRUE },
	{ "log_level", "Connection Logging", validate_range, ARGV("0", "3"), FALSE },
	{ "log_ipaddr", "Log LAN IP Address", validate_ipaddr, ARGV("lan_ipaddr", "lan_netmask"), TRUE },
	/* LAN settings */
	{ "lan_ipaddr", "IP Address", validate_lan_ipaddr, NULL, FALSE },
	{ "lan_proto", "Protocol", validate_choice, ARGV("dhcp", "static"), FALSE },
	{ "lan_stp", "Spanning Tree Protocol", validate_choice, ARGV("0", "1"), FALSE },
	{ "lan_lease", "DHCP Server Lease Time", validate_range, ARGV("1", "604800"), FALSE },
	{ "dhcp_start", "DHCP Server LAN IP Address Range", validate_dhcp, NULL, FALSE },
	{ "lan_route", "Static Routes", validate_lan_route, NULL, FALSE },
	/* ALL wan_XXXX variables below till wan_uint variable are per-interface */
	{ "wan_desc", "Description", validate_name, ARGV("0", "255"), TRUE },
	{ "wan_proto", "Protocol", validate_choice, ARGV("dhcp", "static", "pppoe", "disabled"), FALSE },
	{ "wan_hostname", "Host Name", validate_name, ARGV("0", "255"), TRUE },
	{ "wan_domain", "Domain Name", validate_name, ARGV("0", "255"), TRUE },
	{ "wan_ifname", "Interface Name", validate_wan_ifname, NULL, TRUE },
	{ "wan_hwaddr", "MAC Address", validate_hwaddr, NULL, TRUE },
	{ "wan_ipaddr", "IP Address", validate_ipaddr, NULL, FALSE },
	{ "wan_netmask", "Subnet Mask", validate_ipaddr, NULL, FALSE },
	{ "wan_gateway", "Default Gateway", validate_ipaddr, NULL, TRUE },
	{ "wan_dns", "DNS Servers", validate_ipaddrs, NULL, TRUE },
	{ "wan_wins", "WINS Servers", validate_ipaddrs, NULL, TRUE },
	{ "wan_pppoe_ifname", "PPPoE Interface Name", NULL, NULL, TRUE },
	{ "wan_pppoe_username", "PPPoE Username", validate_name, ARGV("0", "255"), TRUE },
	{ "wan_pppoe_passwd", "PPPoE Password", validate_name, ARGV("0", "255"), TRUE },
	{ "wan_pppoe_service", "PPPoE Service Name", validate_name, ARGV("0", "255"), TRUE },
	{ "wan_pppoe_ac", "PPPoE Access Concentrator", validate_name, ARGV("0", "255"), TRUE },
	{ "wan_pppoe_keepalive", "PPPoE Keep Alive", validate_choice, ARGV("0", "1"), FALSE },
	{ "wan_pppoe_demand", "PPPoE Connect on Demand", validate_choice, ARGV("0", "1"), FALSE },
	{ "wan_pppoe_idletime", "PPPoE Max Idle Time", validate_range, ARGV("1", "3600"), TRUE },
	{ "wan_pppoe_mru", "PPPoE MRU", validate_range, ARGV("128", "16384"), FALSE },
	{ "wan_pppoe_mtu", "PPPoE MTU", validate_range, ARGV("128", "16384"), FALSE },
	{ "wan_primary", "Primary Interface", validate_choice, ARGV("0", "1"), FALSE },
	{ "wan_route", "Static Routes", validate_wan_route, NULL, FALSE },
	/* MUST leave this entry here after all wl_XXXX per-interface variables */
	{ "wan_unit", "WAN Instance", wan_unit, NULL, TRUE },
	/* filter settings */
	{ "filter_macmode", "MAC Filter Mode", validate_choice, ARGV("disabled", "allow", "deny"), FALSE },
	{ "filter_maclist", "MAC Filter", validate_hwaddrs, NULL, TRUE },
	{ "filter_client", "LAN Client Filter", validate_filter_client, ARGV("0", XSTR(MAX_NVPARSE - 1)), FALSE },
	/* routing settings */
	{ "forward_port", "Port Forward", validate_forward_port, ARGV("0", XSTR(MAX_NVPARSE - 1)), FALSE },
	{ "autofw_port", "Application Specific Port Forward", validate_autofw_port, ARGV("0", XSTR(MAX_NVPARSE - 1)), FALSE },
	{ "dmz_ipaddr", "DMZ LAN IP Address", validate_ipaddr, ARGV("lan_ipaddr", "lan_netmask"), TRUE },
	/* ALL wl_XXXX variables are per-interface */
	{ "wl_ssid", "Network Name (SSID)", validate_name, ARGV("1", "32"), FALSE },
	{ "wl_closed", "Network Type", validate_choice, ARGV("0", "1"), FALSE },
	{ "wl_country_code", "Country Code", validate_country, NULL, FALSE },
	{ "wl_mode", "Mode", validate_wl_mode, ARGV("ap", "wds", "sta", "wet"), FALSE },
	{ "wl_infra", "Network", validate_choice, ARGV("0", "1"), FALSE },
	{ "wl_lazywds", "Bridge Restrict", validate_wl_lazywds, ARGV("0", "1"), FALSE },
	{ "wl_wds", "Remote Bridges", validate_wl_wds_hwaddrs, NULL, TRUE },
	/* Order and index matters for WEP keys so enumerate them separately */
	{ "wl_key", "Network Key Index", validate_range, ARGV("1", "4"), FALSE },
	{ "wl_key1", "Network Key 1", validate_wl_key, NULL, TRUE },
	{ "wl_key2", "Network Key 2", validate_wl_key, NULL, TRUE },
	{ "wl_key3", "Network Key 3", validate_wl_key, NULL, TRUE },
	{ "wl_key4", "Network Key 4", validate_wl_key, NULL, TRUE },
	{ "wl_wep", "Data Encryption", validate_wl_wep, ARGV("off", "wep", "tkip", "aes", "tkip+aes"), FALSE },
	{ "wl_auth", "Shared Key Authentication", validate_wl_auth, ARGV("0", "1"), FALSE },
	{ "wl_macmode", "MAC Restrict Mode", validate_choice, ARGV("disabled", "allow", "deny"), FALSE },
	{ "wl_maclist", "Allowed MAC Address", validate_hwaddrs, NULL, TRUE },
	{ "wl_radio", "Radio Enable", validate_choice, ARGV("0", "1"), FALSE },
	{ "wl_phytype", "Radio Band", validate_choice, ARGV("a", "b", "g"), TRUE },
	{ "wl_antdiv", "Antenna Diversity", validate_choice, ARGV("-1", "0", "1", "3"), FALSE },
	/* Channel and rate are fixed in wlconf() if incorrect */
	{ "wl_channel", "Channel", validate_range, ARGV("0", "216"), FALSE },
	{ "wl_rate", "Rate", validate_range, ARGV("0", "54000000"), FALSE },
	{ "wl_rateset", "Supported Rates", validate_choice, ARGV("all", "default", "12"), FALSE },
	{ "wl_frag", "Fragmentation Threshold", validate_range, ARGV("256", "2346"), FALSE },
	{ "wl_rts", "RTS Threshold", validate_range, ARGV("0", "2347"), FALSE },
	{ "wl_dtim", "DTIM Period", validate_range, ARGV("1", "255"), FALSE },
	{ "wl_bcn", "Beacon Interval", validate_range, ARGV("1", "65535"), FALSE },
	{ "wl_plcphdr", "Preamble Type", validate_choice, ARGV("long", "short"), FALSE },
	{ "wl_gmode", "54g Mode", validate_wl_gmode, ARGV(XSTR(GMODE_AUTO), XSTR(GMODE_ONLY), XSTR(GMODE_PERFORMANCE), XSTR(GMODE_LRS), XSTR(GMODE_LEGACY_B), XSTR(GMODE_AFTERBURNER)), FALSE },
	{ "wl_gmode_protection", "54g Protection", validate_choice, ARGV("off", "auto"), FALSE },
	{ "wl_frameburst", "Frame Bursting", validate_choice, ARGV("off", "on"), FALSE },
	/* security parameters */
	{ "wl_wpa_psk", "WPA Pre-Shared Key", validate_wl_wpa_psk, ARGV("64"), TRUE },
	{ "wl_wpa_gtk_rekey", "WPA GTK Rekey Timer", NULL, NULL, TRUE },
	{ "wl_radius_ipaddr", "RADIUS Server", validate_ipaddr, NULL, TRUE },
	{ "wl_radius_port", "RADIUS Port", validate_range, ARGV("0", "65535"), FALSE },
	{ "wl_radius_key", "RADIUS Shared Secret", validate_name, ARGV("0", "255"), TRUE },
	{ "wl_auth_mode", "Network Authentication", validate_wl_auth_mode, ARGV("disabled", "radius", "wpa", "psk"), FALSE },
	/* MUST leave this entry here after all wl_XXXX variables */
	{ "wl_unit", "802.11 Instance", wl_unit, NULL, TRUE },
	/* Internal variables */
	{ "os_server", "OS Server", NULL, NULL, TRUE },
	{ "stats_server", "Stats Server", NULL, NULL, TRUE },
	{ "timer_interval", "Timer Interval", NULL, NULL, TRUE },
};

static void
validate_cgi(webs_t wp)
{
	struct variable *v;
	char *value;

	/* Validate and set variables in table order */
	for (v = variables; v < &variables[sizeof(variables)/sizeof(variables[0])]; v++) {
		if (!(value = websGetVar(wp, v->name, NULL)))
			continue;
		if ((!*value && v->nullok) || !v->validate)
			nvram_set(v->name, value);
		else
			v->validate(wp, value, v);
	}
}

static const char * const apply_header =
"<head>"
"<title>Broadcom Home Gateway Reference Design: Apply</title>"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
"<style type=\"text/css\">"
"body { background: white; color: black; font-family: arial, sans-serif; font-size: 9pt }"
".title	{ font-family: arial, sans-serif; font-size: 13pt; font-weight: bold }"
".subtitle { font-family: arial, sans-serif; font-size: 11pt }"
".label { color: #306498; font-family: arial, sans-serif; font-size: 7pt }"
"</style>"
"</head>"
"<body>"
"<p>"
"<span class=\"title\">APPLY</span><br>"
"<span class=\"subtitle\">This screen notifies you of any errors "
"that were detected while changing the router's settings.</span>"
"<form method=\"get\" action=\"apply.cgi\">"
"<p>"
;

static const char * const apply_footer =
"<p>"
"<input type=\"button\" name=\"action\" value=\"Continue\" OnClick=\"document.location.href='%s';\">"
"</form>"
"<p class=\"label\">&#169;2001-2003 Broadcom Corporation. All rights reserved.</p>"
"</body>"
;

enum {
	NOTHING,
	REBOOT,
	RESTART,
};

static int
apply_cgi(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	  char_t *url, char_t *path, char_t *query)
{
	int action = NOTHING;
	char *value;

	websHeader(wp);
	websWrite(wp, (char_t *) apply_header);

	value = websGetVar(wp, "action", "");

	/* Apply values */
	if (!strcmp(value, "Apply")) {
		websWrite(wp, "Validating values...");
		validate_cgi(wp);
		websWrite(wp, "done<br>");
		websWrite(wp, "Committing values...");
		nvram_commit();
		websWrite(wp, "done<br>");
		action = RESTART;
	}

	/* Restore defaults */
	else if (!strncmp(value, "Restore", 7)) {
		websWrite(wp, "Restoring defaults...");
		nvram_set("sdram_ncdl", "0");
		nvram_set("restore_defaults", "1");
		nvram_commit();
		websWrite(wp, "done<br>");
		action = REBOOT;
	}
	
	/* Release lease */
	else if (!strcmp(value, "Release")) {
		websWrite(wp, "Releasing lease...");
		if (sys_release())
			websWrite(wp, "error<br>");
		else
			websWrite(wp, "done<br>");
		action = NOTHING;
	}

	/* Renew lease */
	else if (!strcmp(value, "Renew")) {
		websWrite(wp, "Renewing lease...");
		if (sys_renew())
			websWrite(wp, "error<br>");
		else
			websWrite(wp, "done<br>");
		action = NOTHING;
	}

	/* Reboot router */
	else if (!strcmp(value, "Reboot")) {
		websWrite(wp, "Rebooting...");
		action = REBOOT;
	}

	/* Upgrade image */
	else if (!strcmp(value, "Upgrade")) {
		char *os_name = nvram_safe_get("os_name");
		char *os_server = websGetVar(wp, "os_server", nvram_safe_get("os_server"));
		char *os_version = websGetVar(wp, "os_version", "current");
		char url[PATH_MAX];
		if (!*os_version)
			os_version = "current";
		snprintf(url, sizeof(url), "%s/%s/%s/%s.trx",
			 os_server, os_name, os_version, os_name);
		websWrite(wp, "Retrieving %s...", url);
		if (sys_upgrade(url, NULL, NULL)) {
			websWrite(wp, "error<br>");
			goto footer;
		} else {
			websWrite(wp, "done<br>");
			action = REBOOT;
		}
	}

	/* Report stats */
	else if (!strcmp(value, "Stats")) {
		char *server = websGetVar(wp, "stats_server", nvram_safe_get("stats_server"));
		websWrite(wp, "Contacting %s...", server);
		if (sys_stats(server)) {
			websWrite(wp, "error<br>");
			goto footer;
		} else {
			websWrite(wp, "done<br>");
			nvram_set("stats_server", server);
		}
	}

	/* Delete connection */
	else if (!strcmp(value, "Delete")) {
		int unit;
		if (!(value = websGetVar(wp, "wan_unit", NULL)) || 
		    (unit = atoi(value)) < 0 || unit >= MAX_NVPARSE) {
			websWrite(wp, "Unable to delete connection, index error.");
			action = NOTHING;
		}
		else {
			struct nvram_tuple *t;
			char tmp[100], prefix[] = "wanXXXXXXXXXX_";
			int unit2, units = 0;
			/*
			* We cann't delete the last connection since we can't differentiate
			* the cases where user does not want any connection (user deletes 
			* the last one) vs. the router is booted the first time when there
			* is no connection at all (where a default one is created anyway).
			*/
			for (unit2 = 0; unit2 < MAX_NVPARSE; unit2 ++) {
				wan_prefix(unit2, prefix);
				if (nvram_get(strcat_r(prefix, "unit", tmp)) && units++ > 0)
					break;
			}
			if (units < 2) {
				websWrite(wp, "Can not delete the last connection.");
				action = NOTHING;
			}
			else {
				/* set prefix */
				wan_prefix(unit, prefix);
				/* remove selected wan%d_ set */
				websWrite(wp, "Deleting connection...");
				for (t = router_defaults; t->name; t ++) {
					if (!strncmp(t->name, "wan_", 4))
						nvram_unset(strcat_r(prefix, &t->name[4], tmp));
				}
				/* fix unit number */
				unit2 = unit;
				for (; unit < MAX_NVPARSE; unit ++) {
					wan_prefix(unit, prefix);
					if (nvram_get(strcat_r(prefix, "unit", tmp)))
						break;
				}
				if (unit >= MAX_NVPARSE) {
					unit = unit2 - 1;
					for (; unit >= 0; unit --) {
						wan_prefix(unit, prefix);
						if (nvram_get(strcat_r(prefix, "unit", tmp)))
							break;
					}
				}
				snprintf(tmp, sizeof(tmp), "%d", unit);
				nvram_set("wan_unit", tmp);
				/* check if there is any primary connection - see comment in wan_unit() */
				for (unit = 0; unit < MAX_NVPARSE; unit ++) {
					wan_prefix(unit, prefix);
					if (!nvram_get(strcat_r(prefix, "unit", tmp)))
						continue;
					if (nvram_invmatch(strcat_r(prefix, "proto", tmp), "disabled") &&
					    nvram_match(strcat_r(prefix, "primary", tmp), "1"))
						break;
				}
				/* no one is primary, pick the first enabled one as primary */
				if (unit >= MAX_NVPARSE)
					wan_primary(wp);
				/* save the change */
				nvram_commit();
				websWrite(wp, "done<br>");
				action = RESTART;
			}
		}
	}
	
	/* Invalid action */
	else
		websWrite(wp, "Invalid action %s<br>", value);

 footer:
	websWrite(wp, (char_t *) apply_footer, websGetVar(wp, "page", ""));
	websFooter(wp);
	websDone(wp, 200);

	if (action == RESTART)
		sys_restart();
	else if (action == REBOOT)
		sys_reboot();

	return 1;
}

static int
wireless_asp(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	     char_t *url, char_t *path, char_t *query)
{
	struct variable *v;
	char *value;
	char name[IFNAMSIZ], *next;
	int unit = 0, oldunit, reset = FALSE;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";

	/* Can enter this function through GET or POST */
	if ((value = websGetVar(wp, "action", NULL))) {
		if (strcmp(value, "Select"))
			return apply_cgi(wp, urlPrefix, webDir, arg, url, path, query);
	}

	/* copy wl%d_XXXXXXXX to wl_XXXXXXXX */
	if ((value = websGetVar(wp, "wl_unit", NULL))) {
		unit = atoi(value);
		oldunit = atoi(nvram_get("wl_unit"));
		if (unit != oldunit) {
			reset = TRUE;
		}
	} else {
		char ifnames[256];
		snprintf(ifnames, sizeof(ifnames), "%s %s", 
			nvram_safe_get("lan_ifnames"),
			nvram_safe_get("wan_ifnames"));
		/* Probe for first wl interface */
		foreach(name, ifnames, next) {
			if (wl_probe(name) == 0 &&
			    wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit)) == 0)
				break;
		}
		if (!*name)
			unit = -1;
	}
	if (unit >= 0) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		for (v = variables; v < &variables[sizeof(variables)/sizeof(variables[0])]; v++) {
			if (!strncmp(v->name, "wl_", 3))
				nvram_set(v->name, nvram_safe_get(strcat_r(prefix, &v->name[3], tmp)));
			if (!strncmp(v->name, "wl_unit", 7))
				break;
		}
	}

	/* Set currently selected unit */
	snprintf(tmp, sizeof(tmp), "%d", unit);
	nvram_set("wl_unit", tmp);

	/* reset the vars to configured ones if unit has changed */
	if (reset) {
		websSetVar(wp, "wl_country_code", nvram_get("wl_country_code"));
		websSetVar(wp, "wl_channel", nvram_get("wl_channel"));
		websSetVar(wp, "wl_phytype", nvram_get("wl_phytype"));
	}

	/* Display the page */
	return websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
}

static int
security_asp(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	     char_t *url, char_t *path, char_t *query)
{
	struct variable *v;
	char *value;
	char name[IFNAMSIZ], *next;
	int unit = 0;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";

	/* Can enter this function through GET or POST */
	if ((value = websGetVar(wp, "action", NULL))) {
		if (strcmp(value, "Select"))
			return apply_cgi(wp, urlPrefix, webDir, arg, url, path, query);
	}

	/* copy wl%d_XXXXXXXX to wl_XXXXXXXX */
	if ((value = websGetVar(wp, "wl_unit", NULL))) {
		unit = atoi(value);
	} else {
		char ifnames[256];
		snprintf(ifnames, sizeof(ifnames), "%s %s", 
			nvram_safe_get("lan_ifnames"),
			nvram_safe_get("wan_ifnames"));
		/* Probe for first wl interface */
		foreach(name, ifnames, next) {
			if (wl_probe(name) == 0 &&
			    wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit)) == 0)
				break;
		}
		if (!*name)
			unit = -1;
	}
	if (unit >= 0) {
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);
		for (v = variables; v < &variables[sizeof(variables)/sizeof(variables[0])]; v++) {
			if (!strncmp(v->name, "wl_", 3))
				nvram_set(v->name, nvram_safe_get(strcat_r(prefix, &v->name[3], tmp)));
			if (!strncmp(v->name, "wl_unit", 7))
				break;
		}
	}

	/* Set currently selected unit */
	snprintf(tmp, sizeof(tmp), "%d", unit);
	nvram_set("wl_unit", tmp);

	/* Display the page */
	return websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
}

static int
wan_asp(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	     char_t *url, char_t *path, char_t *query)
{
	struct variable *v;
	char *value;
	int unit = 0;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char ustr[16];
	struct nvram_tuple *t;

	/* Can enter this function through GET or POST */
	if ((value = websGetVar(wp, "action", NULL))) {
		if (!strcmp(value, "New")) {
			/* pick one that 'does not' exist */
			for (unit = 0; unit < MAX_NVPARSE; unit ++) {
				wan_prefix(unit, prefix);
				if (!nvram_get(strcat_r(prefix, "unit", tmp)))
					break;
			}
			if (unit >= MAX_NVPARSE) {
				websHeader(wp);
				websWrite(wp, (char_t *) apply_header);
				websWrite(wp, "Unable to create new connection. Maximum %d.", MAX_NVPARSE);
				websWrite(wp, (char_t *) apply_footer, websGetVar(wp, "page", ""));
				websFooter(wp);
				websDone(wp, 200);
				return 1;
			}
			/* copy default to to newly created wan%d_ set */
			for (t = router_defaults; t->name; t ++) {
				if (!strncmp(t->name, "wan_", 4))
					nvram_set(strcat_r(prefix, &t->name[4], tmp), t->value);
			}
			/* the following variables must be overwritten */
			snprintf(ustr, sizeof(ustr), "%d", unit);
			nvram_set(strcat_r(prefix, "unit", tmp), ustr);
			nvram_set(strcat_r(prefix, "proto", tmp), "disabled");
			nvram_set(strcat_r(prefix, "ifname", tmp), nvram_safe_get("wan_ifname"));
			nvram_set(strcat_r(prefix, "hwaddr", tmp), nvram_safe_get("wan_hwaddr"));
			nvram_set(strcat_r(prefix, "ifnames", tmp), nvram_safe_get("wan_ifnames"));
			/* commit change */
			nvram_commit();
		}
		else if (!strcmp(value, "Select")) {
			if ((value = websGetVar(wp, "wan_unit", NULL)))
				unit = atoi(value);
			else
				unit = -1;
		}
		else
			return apply_cgi(wp, urlPrefix, webDir, arg, url, path, query);
	}
	else if ((value = websGetVar(wp, "wan_unit", NULL)))
		unit = atoi(value);
	else
		unit = atoi(nvram_safe_get("wan_unit"));
	if (unit < 0 || unit >= MAX_NVPARSE)
		unit = 0;

	/* Set prefix */
	wan_prefix(unit, prefix);

	/* copy wan%d_ set to wan_ set */
	for (v = variables; v < &variables[sizeof(variables)/sizeof(variables[0])]; v++) {
		if (strncmp(v->name, "wan_", 4))
			continue;
		value = nvram_get(strcat_r(prefix, &v->name[4], tmp));
		if (value)
			nvram_set(v->name, value);
		if (!strncmp(v->name, "wan_unit", 8))
			break;
	}

	/* Set currently selected unit */
	snprintf(tmp, sizeof(tmp), "%d", unit);
	nvram_set("wan_unit", tmp);

	/* Display the page */
	return websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query);
}

#ifdef WEBS

void
initHandlers(void)
{
	websAspDefine("nvram_get", ej_nvram_get);
	websAspDefine("nvram_match", ej_nvram_match);
	websAspDefine("nvram_invmatch", ej_nvram_invmatch);
	websAspDefine("nvram_list", ej_nvram_list);
	websAspDefine("filter_client", ej_filter_client);
	websAspDefine("forward_port", ej_forward_port);
	websAspDefine("static_route", ej_static_route);
	websAspDefine("localtime", ej_localtime);
	websAspDefine("dumplog", ej_dumplog);
	websAspDefine("dumpleases", ej_dumpleases);
	websAspDefine("link", ej_link);

	websUrlHandlerDefine("/apply.cgi", NULL, 0, apply_cgi, 0);
	websUrlHandlerDefine("/wireless.asp", NULL, 0, wireless_asp, 0);

	websSetPassword(nvram_safe_get("http_passwd"));

	websSetRealm("Broadcom Home Gateway Reference Design");
}

#else /* !WEBS */

static void
do_auth(char *userid, char *passwd, char *realm)
{
	strncpy(userid, nvram_safe_get("http_username"), AUTH_MAX);
	strncpy(passwd, nvram_safe_get("http_passwd"), AUTH_MAX);
	strncpy(realm, "Broadcom Home Gateway Reference Design", AUTH_MAX);
}

static char post_buf[10000];

static void
do_apply_post(char *url, FILE *stream, int len, char *boundary)
{
	/* Get query */
	if (!fgets(post_buf, MIN(len + 1, sizeof(post_buf)), stream))
		return;
	len -= strlen(post_buf);

	/* Initialize CGI */
	init_cgi(post_buf);

	/* Slurp anything remaining in the request */
	while (len--)
		(void) fgetc(stream);
}

static void
do_apply_cgi(char *url, FILE *stream)
{
	char *path, *query;

	/* Parse path */
	query = url;
	path = strsep(&query, "?") ? : url;

	apply_cgi(stream, NULL, NULL, 0, url, path, query);

	/* Reset CGI */
	init_cgi(NULL);
}

static int upgrade_ret;

static void
do_upgrade_post(char *url, FILE *stream, int len, char *boundary)
{
	char buf[1024];

	upgrade_ret = EINVAL;

	/* Look for our part */
	while (len > 0) {
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream))
			return;
		len -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20) &&
		    strstr(buf, "name=\"file\""))
			break;
	}

	/* Skip boundary and headers */
	while (len > 0) {
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream))
			return;
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	upgrade_ret = sys_upgrade(NULL, stream, &len);

	/* Slurp anything remaining in the request */
	while (len--)
		(void) fgetc(stream);
}

static void
do_upgrade_cgi(char *url, FILE *stream)
{
	websHeader(stream);
	websWrite(stream, (char_t *) apply_header);

	/* We could probably be more informative here... */
	if (upgrade_ret)
		websWrite(stream, "Error during upgrade<br>");
	else
		websWrite(stream, "Upgrade complete. Rebooting...<br>");

	websWrite(stream, (char_t *) apply_footer, "firmware.asp");
	websFooter(stream);
	websDone(stream, 200);

	/* Reboot if successful */
	if (upgrade_ret == 0)
		sys_reboot();
}

static void
do_wireless_asp(char *url, FILE *stream)
{
	char *path, *query;

	/* Parse path */
	query = url;
	path = strsep(&query, "?") ? : url;

	wireless_asp(stream, NULL, NULL, 0, url, path, query);

	/* Reset CGI */
	init_cgi(NULL);
}

static void
do_security_asp(char *url, FILE *stream)
{
	char *path, *query;

	/* Parse path */
	query = url;
	path = strsep(&query, "?") ? : url;

	security_asp(stream, NULL, NULL, 0, url, path, query);

	/* Reset CGI */
	init_cgi(NULL);
}

static void
do_wan_asp(char *url, FILE *stream)
{
	char *path, *query;

	/* Parse path */
	query = url;
	path = strsep(&query, "?") ? : url;

	wan_asp(stream, NULL, NULL, 0, url, path, query);

	/* Reset CGI */
	init_cgi(NULL);
}

static char no_cache[] =
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Expires: 0"
;

struct mime_handler mime_handlers[] = {
	{ "wan.asp", "text/html", no_cache, do_apply_post, do_wan_asp, do_auth },
	{ "wireless.asp", "text/html", no_cache, do_apply_post, do_wireless_asp, do_auth },
	{ "security.asp", "text/html", no_cache, do_apply_post, do_security_asp, do_auth },
	{ "**.asp", "text/html", no_cache, NULL, do_ej, do_auth },
	{ "**.css", "text/css", NULL, NULL, do_file, do_auth },
	{ "**.gif", "image/gif", NULL, NULL, do_file, do_auth },
	{ "**.jpg", "image/jpeg", NULL, NULL, do_file, do_auth },
	{ "**.js", "text/javascript", NULL, NULL, do_file, do_auth },
	{ "apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, do_auth },
	{ "upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi, do_auth },
	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

struct ej_handler ej_handlers[] = {
	{ "nvram_get", ej_nvram_get },
	{ "nvram_match", ej_nvram_match },
	{ "nvram_invmatch", ej_nvram_invmatch },
	{ "nvram_list", ej_nvram_list },
	{ "options", ej_options },
	{ "filter_client", ej_filter_client },
	{ "forward_port", ej_forward_port },
	{ "autofw_port", ej_autofw_port },
	{ "localtime", ej_localtime },
	{ "sysuptime", ej_sysuptime },
	{ "dumplog", ej_dumplog },
	{ "wl_list", ej_wl_list },
	{ "wl_phytypes", ej_wl_phytypes },
	{ "wl_radioid", ej_wl_radioid },
	{ "wl_corerev", ej_wl_corerev },
	{ "wl_cur_channel", ej_wl_cur_channel },
	{ "lan_route", ej_lan_route },
	{ "lan_leases", ej_lan_leases },
	{ "wan_list", ej_wan_list },
	{ "wan_iflist", ej_wan_iflist },
	{ "wan_route", ej_wan_route },
	{ "wan_link", ej_wan_link },
	{ "wan_lease", ej_wan_lease },
	{ "wl_channel_list", ej_wl_channel_list },
	{ "wl_cur_phytype", ej_wl_cur_phytype },
	{ "wl_cur_country", ej_wl_cur_country },
	{ "wl_country_list", ej_wl_country_list },
	{ "wl_ioctl", ej_wl_ioctl },
	{ NULL, NULL }
};

#endif /* !WEBS */

/*
 * Country names and abbreviations from ISO 3166
 */
country_name_t country_names[] = {

{"AFGHANISTAN",		 "AF"},
{"ALBANIA",		 "AL"},
{"ALGERIA",		 "DZ"},
{"AMERICAN SAMOA", 	 "AS"},
{"ANDORRA",		 "AD"},
{"ANGOLA",		 "AO"},
{"ANGUILLA",		 "AI"},
{"ANTARCTICA",		 "AQ"},
{"ANTIGUA AND BARBUDA",	 "AG"},
{"ARGENTINA",		 "AR"},
{"ARMENIA",		 "AM"},
{"ARUBA",		 "AW"},
{"AUSTRALIA",		 "AU"},
{"AUSTRIA",		 "AT"},
{"AZERBAIJAN",		 "AZ"},
{"BAHAMAS",		 "BS"},
{"BAHRAIN",		 "BH"},
{"BANGLADESH",		 "BD"},
{"BARBADOS",		 "BB"},
{"BELARUS",		 "BY"},
{"BELGIUM",		 "BE"},
{"BELIZE",		 "BZ"},
{"BENIN",		 "BJ"},
{"BERMUDA",		 "BM"},
{"BHUTAN",		 "BT"},
{"BOLIVIA",		 "BO"},
{"BOSNIA AND HERZEGOVINA","BA"},
{"BOTSWANA",		 "BW"},
{"BOUVET ISLAND",	 "BV"},
{"BRAZIL",		 "BR"},
{"BRITISH INDIAN OCEAN TERRITORY", 	"IO"},
{"BRUNEI DARUSSALAM",	 "BN"},
{"BULGARIA",		 "BG"},
{"BURKINA FASO",	 "BF"},
{"BURUNDI",		 "BI"},
{"CAMBODIA",		 "KH"},
{"CAMEROON",		 "CM"},
{"CANADA",		 "CA"},
{"CAPE VERDE",		 "CV"},
{"CAYMAN ISLANDS",	 "KY"},
{"CENTRAL AFRICAN REPUBLIC","CF"},
{"CHAD",		 "TD"},
{"CHILE",		 "CL"},
{"CHINA",		 "CN"},
{"CHRISTMAS ISLAND",	 "CX"},
{"COCOS (KEELING) ISLANDS","CC"},
{"COLOMBIA",		 "CO"},
{"COMOROS",		 "KM"},
{"CONGO",		 "CG"},
{"CONGO, THE DEMOCRATIC REPUBLIC OF THE", "CD"},
{"COOK ISLANDS",	 "CK"},
{"COSTA RICA",		 "CR"},
{"COTE D'IVOIRE",	 "CI"},
{"CROATIA",		 "HR"},
{"CUBA",		 "CU"},
{"CYPRUS",		 "CY"},
{"CZECH REPUBLIC",	 "CZ"},
{"DENMARK",		 "DK"},
{"DJIBOUTI",		 "DJ"},
{"DOMINICA",		 "DM"},
{"DOMINICAN REPUBLIC", 	 "DO"},
{"ECUADOR",		 "EC"},
{"EGYPT",		 "EG"},
{"EL SALVADOR",		 "SV"},
{"EQUATORIAL GUINEA",	 "GQ"},
{"ERITREA",		 "ER"},
{"ESTONIA",		 "EE"},
{"ETHIOPIA",		 "ET"},
{"FALKLAND ISLANDS (MALVINAS)",	"FK"},
{"FAROE ISLANDS",	 "FO"},
{"FIJI",		 "FJ"},
{"FINLAND",		 "FI"},
{"FRANCE",		 "FR"},
{"FRENCH GUIANA",	 "GF"},
{"FRENCH POLYNESIA",	 "PF"},
{"FRENCH SOUTHERN TERRITORIES",	 "TF"},
{"GABON",		 "GA"},
{"GAMBIA",		 "GM"},
{"GEORGIA",		 "GE"},
{"GERMANY",		 "DE"},
{"GHANA",		 "GH"},
{"GIBRALTAR",		 "GI"},
{"GREECE",		 "GR"},
{"GREENLAND",		 "GL"},
{"GRENADA",		 "GD"},
{"GUADELOUPE",		 "GP"},
{"GUAM",		 "GU"},
{"GUATEMALA",		 "GT"},
{"GUINEA",		 "GN"},
{"GUINEA-BISSAU",	 "GW"},
{"GUYANA",		 "GY"},
{"HAITI",		 "HT"},
{"HEARD ISLAND AND MCDONALD ISLANDS",	"HM"},
{"HOLY SEE (VATICAN CITY STATE)", 	"VA"},
{"HONDURAS",		 "HN"},
{"HONG KONG",		 "HK"},
{"HUNGARY",		 "HU"},
{"ICELAND",		 "IS"},
{"INDIA",		 "IN"},
{"INDONESIA",		 "ID"},
{"IRAN, ISLAMIC REPUBLIC OF",		"IR"},
{"IRAQ",		 "IQ"},
{"IRELAND",		 "IE"},
{"ISRAEL",		 "IL"},
{"ITALY",		 "IT"},
{"JAMAICA",		 "JM"},
{"JAPAN",		 "JP"},
{"JORDAN",		 "JO"},
{"KAZAKHSTAN",		 "KZ"},
{"KENYA",		 "KE"},
{"KIRIBATI",		 "KI"},
{"KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF", "KP"},
{"KOREA, REPUBLIC OF",	 "KR"},
{"KUWAIT",		 "KW"},
{"KYRGYZSTAN",		 "KG"},
{"LAO PEOPLE'S DEMOCRATIC REPUBLIC", 	"LA"},
{"LATVIA",		 "LV"},
{"LEBANON",		 "LB"},
{"LESOTHO",		 "LS"},
{"LIBERIA",		 "LR"},
{"LIBYAN ARAB JAMAHIRIYA","LY"},
{"LIECHTENSTEIN",	 "LI"},
{"LITHUANIA",		 "LT"},
{"LUXEMBOURG",		 "LU"},
{"MACAO",		 "MO"},
{"MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF",	 "MK"},
{"MADAGASCAR",		 "MG"},
{"MALAWI",		 "MW"},
{"MALAYSIA",		 "MY"},
{"MALDIVES",		 "MV"},
{"MALI",		 "ML"},
{"MALTA",		 "MT"},
{"MARSHALL ISLANDS",	 "MH"},
{"MARTINIQUE",		 "MQ"},
{"MAURITANIA",		 "MR"},
{"MAURITIUS",		 "MU"},
{"MAYOTTE",		 "YT"},
{"MEXICO",		 "MX"},
{"MICRONESIA, FEDERATED STATES OF", 	"FM"},
{"MOLDOVA, REPUBLIC OF", "MD"},
{"MONACO",		 "MC"},
{"MONGOLIA",		 "MN"},
{"MONTSERRAT",		 "MS"},
{"MOROCCO",		 "MA"},
{"MOZAMBIQUE",		 "MZ"},
{"MYANMAR",		 "MM"},
{"NAMIBIA",		 "NA"},
{"NAURU",		 "NR"},
{"NEPAL",		 "NP"},
{"NETHERLANDS",		 "NL"},
{"NETHERLANDS ANTILLES", "AN"},
{"NEW CALEDONIA",	 "NC"},
{"NEW ZEALAND",		 "NZ"},
{"NICARAGUA",		 "NI"},
{"NIGER",		 "NE"},
{"NIGERIA",		 "NG"},
{"NIUE",		 "NU"},
{"NORFOLK ISLAND",	 "NF"},
{"NORTHERN MARIANA ISLANDS","MP"},
{"NORWAY",		 "NO"},
{"OMAN",		 "OM"},
{"PAKISTAN",		 "PK"},
{"PALAU",		 "PW"},
{"PALESTINIAN TERRITORY, OCCUPIED", 	"PS"},
{"PANAMA",		 "PA"},
{"PAPUA NEW GUINEA",	 "PG"},
{"PARAGUAY",		 "PY"},
{"PERU",		 "PE"},
{"PHILIPPINES",		 "PH"},
{"PITCAIRN",		 "PN"},
{"POLAND",		 "PL"},
{"PORTUGAL",		 "PT"},
{"PUERTO RICO",		 "PR"},
{"QATAR",		 "QA"},
{"REUNION",		 "RE"},
{"ROMANIA",		 "RO"},
{"RUSSIAN FEDERATION",	 "RU"},
{"RWANDA",		 "RW"},
{"SAINT HELENA",	 "SH"},
{"SAINT KITTS AND NEVIS","KN"},
{"SAINT LUCIA",		 "LC"},
{"SAINT PIERRE AND MIQUELON",	 	"PM"},
{"SAINT VINCENT AND THE GRENADINES", 	"VC"},
{"SAMOA",		 "WS"},
{"SAN MARINO",		 "SM"},
{"SAO TOME AND PRINCIPE","ST"},
{"SAUDI ARABIA",	 "SA"},
{"SENEGAL",		 "SN"},
{"SEYCHELLES",		 "SC"},
{"SIERRA LEONE",	 "SL"},
{"SINGAPORE",		 "SG"},
{"SLOVAKIA",		 "SK"},
{"SLOVENIA",		 "SI"},
{"SOLOMON ISLANDS",	 "SB"},
{"SOMALIA",		 "SO"},
{"SOUTH AFRICA",	 "ZA"},
{"SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS", "GS"},
{"SPAIN",		 "ES"},
{"SRI LANKA",		 "LK"},
{"SUDAN",		 "SD"},
{"SURINAME",		 "SR"},
{"SVALBARD AND JAN MAYEN","SJ"},
{"SWAZILAND",		 "SZ"},
{"SWEDEN",		 "SE"},
{"SWITZERLAND",		 "CH"},
{"SYRIAN ARAB REPUBLIC", "SY"},
{"TAIWAN, PROVINCE OF CHINA", 		"TW"},
{"TAJIKISTAN",		 "TJ"},
{"TANZANIA, UNITED REPUBLIC OF",	"TZ"},
{"THAILAND",		 "TH"},
{"TIMOR-LESTE",		 "TL"},
{"TOGO",		 "TG"},
{"TOKELAU",		 "TK"},
{"TONGA",		 "TO"},
{"TRINIDAD AND TOBAGO",	 "TT"},
{"TUNISIA",		 "TN"},
{"TURKEY",		 "TR"},
{"TURKMENISTAN",	 "TM"},
{"TURKS AND CAICOS ISLANDS",		"TC"},
{"TUVALU",		 "TV"},
{"UGANDA",		 "UG"},
{"UKRAINE",		 "UA"},
{"UNITED ARAB EMIRATES", "AE"},
{"UNITED KINGDOM",	 "GB"},
{"UNITED STATES",	 "US"},
{"UNITED STATES MINOR OUTLYING ISLANDS","UM"},
{"URUGUAY",		 "UY"},
{"UZBEKISTAN",		 "UZ"},
{"VANUATU",		 "VU"},
{"VENEZUELA",		 "VE"},
{"VIET NAM",		 "VN"},
{"VIRGIN ISLANDS, BRITISH", "VG"},
{"VIRGIN ISLANDS, U.S.", "VI"},
{"WALLIS AND FUTUNA",	 "WF"},
{"WESTERN SAHARA", 	 "EH"},
{"YEMEN",		 "YE"},
{"YUGOSLAVIA",		 "YU"},
{"ZAMBIA",		 "ZM"},
{"ZIMBABWE",		 "ZW"},
{NULL, 			 NULL}
};
