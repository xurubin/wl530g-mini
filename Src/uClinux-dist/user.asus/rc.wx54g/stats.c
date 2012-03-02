/*
 * Support for router statistics gathering
 *
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id: stats.c,v 1.5 2003/02/19 02:41:17 mhuang Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <shutils.h>

extern int http_post(const char *server, char *buf, size_t count);

#define BUFSPACE 50000

int
http_stats(const char *url)
{
	char *buf, *s;
	char **cur;
	char *secrets[] = { "os_server", "stats_server", "http_passwd", NULL };
	char *files[] = { "/proc/version", "/proc/meminfo", "/proc/cpuinfo", "/proc/interrupts",
			  "/proc/net/dev", "/proc/net/pppoe", "/proc/net/snmp", NULL };
	char *contents;

	if (!(buf = malloc(BUFSPACE)))
		return errno;

	/* Get NVRAM variables */
	nvram_getall(buf, NVRAM_SPACE);
	for (s = buf; *s; s++) {
		for (cur = secrets; *cur; cur++) {
			if (!strncmp(s, *cur, strlen(*cur))) {
				s += strlen(*cur) + 1;
				while (*s)
					*s++ = '*';
				break;
			}
		}
		*(s += strlen(s)) = '&';
	}

	/* Dump interesting /proc entries */
	for (cur = files; *cur; cur++) {
		if ((contents = file2str(*cur))) {
			s += snprintf(s, buf + BUFSPACE - s, "%s=%s&", *cur, contents);
			free(contents);
		}
	}

	/* Report uptime */
	s += snprintf(s, buf + BUFSPACE - s, "uptime=%lu&", (unsigned long) time(NULL));

	/* Save */
	s += snprintf(s, buf + BUFSPACE - s, "action=save");
	buf[BUFSPACE-1] = '\0';

	/* Post to server */
	http_post(url ? : nvram_safe_get("stats_server"), buf, BUFSPACE);

	free(buf);
	return 0;
}
