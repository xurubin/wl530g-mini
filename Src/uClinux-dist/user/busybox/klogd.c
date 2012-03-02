/* vi: set sw=4 ts=4: */
/*
 * Mini klogd implementation for busybox
 *
 * Copyright (C) 2001 by Gennady Feldman <gfeldman@cachier.com>.
 * Changes: Made this a standalone busybox module which uses standalone
 * 					syslog() client interface.
 *
 * Copyright (C) 1999,2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 1999,2000,2001 by Erik Andersen <andersee@debian.org>
 *
 * Copyright (C) 2000 by Karl M. Hegbloom <karlheg@debian.org>
 *
 * "circular buffer" Copyright (C) 2000 by Gennady Feldman <gfeldman@mail.com>
 *
 * Maintainer: Gennady Feldman <gena01@cachier.com> as of Mar 12, 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h> /* for our signal() handlers */
#include <string.h> /* strncpy() */
#include <errno.h>  /* errno and friends */
#include <unistd.h>
#include <ctype.h>
#include <sys/syslog.h>

#if __GNU_LIBRARY__ < 5
# ifdef __alpha__
#   define klogctl syslog
# endif
#else
# include <sys/klog.h>
#endif

#include "busybox.h"

#define ASUS 1
#ifdef ASUS
/* Add by Joey */
int logAll = FALSE;
#endif

static void klogd_signal(int sig)
{
	klogctl(7, NULL, 0);
	klogctl(0, 0, 0);
	//logMessage(0, "Kernel log daemon exiting.");
	syslog_msg(LOG_SYSLOG, LOG_NOTICE, "Kernel log daemon exiting.");
	exit(TRUE);
}

static void doKlogd (void) __attribute__ ((noreturn));
static void doKlogd (void)
{
	int priority = LOG_INFO;
	char log_buffer[4096];
	int i, n, lastc;
	char *start;

	/* Set up sig handlers */
	signal(SIGINT, klogd_signal);
	signal(SIGKILL, klogd_signal);
	signal(SIGTERM, klogd_signal);
	signal(SIGHUP, SIG_IGN);

	/* "Open the log. Currently a NOP." */
	klogctl(1, NULL, 0);

#ifdef ASUS // Joey	
    	//if (!logAll)
	//   syslog(6, "started", 0); /* Disable printk to console */
#else
	syslog_msg(LOG_SYSLOG, LOG_NOTICE, "klogd started: " BB_BANNER);
#endif

	while (1) {
		/* Use kernel syscalls */
		memset(log_buffer, '\0', sizeof(log_buffer));
		n = klogctl(2, log_buffer, sizeof(log_buffer));
		if (n < 0) {
			char message[80];

			if (errno == EINTR)
				continue;
			snprintf(message, 79, "klogd: Error return from sys_sycall: %d - %s.\n", 
												errno, strerror(errno));
			syslog_msg(LOG_SYSLOG, LOG_ERR, message);
			exit(1);
		}

		/* klogctl buffer parsing modelled after code in dmesg.c */
		start=&log_buffer[0];
		lastc='\0';
		for (i=0; i<n; i++) {
			if (lastc == '\0' && log_buffer[i] == '<') {
				priority = 0;
				i++;
				while (isdigit(log_buffer[i])) {
					priority = priority*10+(log_buffer[i]-'0');
					i++;
				}
				if (log_buffer[i] == '>') i++;
				start = &log_buffer[i];
			}
			if (log_buffer[i] == '\n') {
				log_buffer[i] = '\0';  /* zero terminate this message */
#ifdef ASUS
				if (!syslog_msg_packet(LOG_DAEMON, LOG_KERN|priority, start))
				{
				   //only report packets information from kernel currently
                   if (logAll)
				      syslog_msg(LOG_DAEMON, LOG_KERN | priority, start);
				}
#else
				syslog_msg(LOG_KERN, priority, start);
#endif
				start = &log_buffer[i+1];
				priority = LOG_INFO;
			}
			lastc = log_buffer[i];
		}
	}
}

extern int klogd_main(int argc, char **argv)
{
	/* no options, no getopt */
	int opt;
#ifndef __uClinux__ /* fork() not available in uClinux */
	int doFork = TRUE;
#endif  /* __uClinux__ */

	/* do normal option parsing */
	while ((opt = getopt(argc, argv, "nd")) > 0) {
		switch (opt) {
			case 'n':
#ifndef __uClinux__	/* fork() not available in uClinux */
				doFork = FALSE;
#endif  /* __uClinux__ */
				break;
#ifdef ASUS
 			case 'd': /* Add by Joey */
				logAll = TRUE;
                		break;
#endif
			default:
				show_usage();
		}
	}

#ifndef __uClinux__	/* fork() not available in uClinux */
	if (doFork == TRUE) {
		if (daemon(0, 1) < 0)
			perror_msg_and_die("daemon");
	}
#endif  /* __uClinux__ */

	doKlogd();
	
	return EXIT_SUCCESS;
}

/*
Local Variables
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/

#ifdef ASUS
/* Joey : process packet related log message */ 
int syslog_msg_packet(int facility, int priority, char *buf)
{

	#define MAX_LOG_BUF 1024
	char newbuf[MAX_LOG_BUF], *line;
	char *next, *s;
	int len, ret = 0, alarm=0;
	long filelen;
	char *verdict, *src, *dst, *proto, *spt, *dpt, *tm;
	  
	line = buf;

	if (!strncmp(line, "DROP", 4))
		verdict = "denied";
	else if (!strncmp(line, "ACCEPT", 6))
		verdict = "accepted";
	else if (!strncmp(line, "ALERT",5))
	{
		verdict = "access";
		alarm = 1;
	}
	else return 0;

	/* Parse into tokens */
	s = line;
	len = strlen(s);
	while (strsep(&s, " "));

	/* Initialize token values */
	src = dst = proto = spt = dpt = "n/a";

	/* Set token values */
	for(s = line; s < &line[len] && *s; s += strlen(s) + 1) 
	{
			if (!strncmp(s, "TIME=", 5))
				tm = &s[0];
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
	sprintf(newbuf, "%s connection %s to %s:%s from %s:%s\n",
			proto, verdict, dst, dpt, src, spt);
		
	if (!alarm)
	{
		openlog("filter", 0, facility);
		syslog(priority, "%s", newbuf);
		closelog();
	}
	else
	{
		openlog("alarm", 0, facility);
		syslog(priority, "%s", newbuf);
		closelog();
	}
    	return 1;
}
#endif
