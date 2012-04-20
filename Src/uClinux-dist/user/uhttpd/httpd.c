/* milli_httpd - pretty small HTTP server
 ** A combination of
 ** micro_httpd - really small HTTP server
 ** and
 ** mini_httpd - small HTTP server
 **
 ** Copyright � 1999,2000 by Jef Poskanzer <jef@acme.com>.
 ** All rights reserved.
 **
 ** Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions
 ** are met:
 ** 1. Redistributions of source code must retain the above copyright
 **    notice, this list of conditions and the following disclaimer.
 ** 2. Redistributions in binary form must reproduce the above copyright
 **    notice, this list of conditions and the following disclaimer in the
 **    documentation and/or other materials provided with the distribution.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 ** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 ** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 ** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 ** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 ** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 ** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 ** SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "httpd.h"
#include "handlers.h"
#include "nvram.h"

#include <error.h>
#include <sys/signal.h>

// Added by Joey for ethtool
#include <net/if.h>
#ifndef SIOCETHTOOL
#define SIOCETHTOOL 0x8946
#endif

#define SERVER_NAME "httpd"
#define SERVER_PORT 80
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

/* A multi-family sockaddr. */
typedef union {
	struct sockaddr sa;
	struct sockaddr_in sa_in;
} usockaddr;

/* Globals. */
static FILE *conn_fp;
static char auth_userid[AUTH_MAX];
static char auth_passwd[AUTH_MAX];
static char auth_realm[AUTH_MAX];

/* Forwards. */
static int initialize_listen_socket(usockaddr* usaP);
static int auth_check(char* dirname, char* authorization);
static void send_authenticate(char* realm);
static void send_error(int status, char* title, char* extra_header, char* text);
static void send_headers(int status, char* title, char* extra_header,
		char* mime_type);
static int b64_decode(const char* str, char* space, int size);
static int match(const char* pattern, const char* string);
static int match_one(const char* pattern, int patternlen, const char* string);
static void handle_request(void);

/* added by Joey */
int redirect = 1;
char wan_if[16];
int http_port = SERVER_PORT;

/* Added by Joey for handle one people at the same time */
unsigned int login_ip = 0;
time_t login_timestamp = 0;
unsigned int login_ip_tmp = 0;
unsigned int login_try = 0;

static int initialize_listen_socket(usockaddr* usaP) {
	int listen_fd;
	int i;

	memset(usaP, 0, sizeof(usockaddr));
	usaP->sa.sa_family = AF_INET;
	usaP->sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
	usaP->sa_in.sin_port = htons(http_port);

	listen_fd = socket(usaP->sa.sa_family, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("socket");
		return -1;
	}
	(void) fcntl(listen_fd, F_SETFD, 1);
	i = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &i, sizeof(i))
			< 0) {
		perror("setsockopt");
		return -1;
	}
	if (bind(listen_fd, &usaP->sa, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return -1;
	}
	if (listen(listen_fd, 1024) < 0) {
		perror("listen");
		return -1;
	}
	return listen_fd;
}


void http_login_cache(usockaddr *u) {
	login_ip_tmp = (unsigned int) (u->sa_in.sin_addr.s_addr);
	//printf("client :%x\n", login_ip_tmp);
}

void http_login(unsigned int ip) {
	char tmpstr[32];

	if (http_port != SERVER_PORT || ip == 0x100007f)
		return;

	login_ip = ip;
	login_try = 0;
	time(&login_timestamp);
	sprintf(tmpstr, "%d", ip);
	//nvram_set("login_ip", tmpstr);
}

void http_logout(unsigned int ip) {
	//fprintf(stderr, "ip : %x %x %x\n", ip, login_ip, login_try);

	if (ip == login_ip) {
		login_try = login_ip;
		login_ip = 0;
		login_timestamp = 0;
		//nvram_set("login_ip", "0");
	}
}

void http_login_timeout(unsigned int ip) {
	time_t now;

	time(&now);

	//printf("login : %x %x\n", now, login_timestamp);

	if (login_ip != ip && (unsigned long) (now - login_timestamp) > 60) //one minitues
			{
		http_logout(login_ip);
	}
}

int http_login_check(void) {
	if (http_port != SERVER_PORT || login_ip_tmp == 0x100007f)
		return 1;

	http_login_timeout(login_ip_tmp);

	if (login_ip != 0) {
		if (login_ip != login_ip_tmp) {
			return 0;
		}
	}
	return 1;
}

static int auth_check(char* dirname, char* authorization) {
	char authinfo[500];
	char* authpass;
	int l;

	/* Is this directory unprotected? */
	if (!strlen(auth_passwd))
		/* Yes, let the request go through. */
		return 1;

	/* Basic authorization info? */
	if (!authorization || strncmp(authorization, "Basic ", 6) != 0) {
		send_authenticate(dirname);
		http_logout(login_ip_tmp);
		return 0;
	}

	/* Decode it. */
	l = b64_decode(&(authorization[6]), authinfo, sizeof(authinfo));
	authinfo[l] = '\0';
	/* Split into user and password. */
	authpass = strchr(authinfo, ':');
	if (authpass == (char*) 0) {
		/* No colon?  Bogus auth info. */
		send_authenticate(dirname);
		http_logout(login_ip_tmp);
		return 0;
	}
	*authpass++ = '\0';

	/* Is this the right user and password? */
	if (strcmp(auth_userid, authinfo) == 0
			&& strcmp(auth_passwd, authpass) == 0) {
		//fprintf(stderr, "login check : %x %x\n", login_ip, login_try);
		/* Is this is the first login after logout */
		if (login_ip == 0 && login_try == login_ip_tmp) {
			send_authenticate(dirname);
			login_try = 0;
			return 0;
		}
		return 1;
	}

	send_authenticate(dirname);
	http_logout(login_ip_tmp);
	return 0;
}

static void send_authenticate(char* realm) {
	char header[10000];

	(void) snprintf(header, sizeof(header),
			"WWW-Authenticate: Basic realm=\"%s\"", realm);
	send_error(401, "Unauthorized", header, "Authorization required.");
}

static void send_error(int status, char* title, char* extra_header, char* text) {
	send_headers(status, title, extra_header, "text/html");
	(void) fprintf(conn_fp,
			"<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n",
			status, title, status, title);
	(void) fprintf(conn_fp, "%s\n", text);
	(void) fprintf(conn_fp, "</BODY></HTML>\n");
	(void) fflush(conn_fp);
}

static void send_headers(int status, char* title, char* extra_header,
		char* mime_type) {
	time_t now;
	char timebuf[100];

	(void) fprintf(conn_fp, "%s %d %s\r\n", PROTOCOL, status, title);
	(void) fprintf(conn_fp, "Server: %s\r\n", SERVER_NAME);
	now = time((time_t*) 0);
	(void) strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	(void) fprintf(conn_fp, "Date: %s\r\n", timebuf);
	if (extra_header != (char*) 0)
		(void) fprintf(conn_fp, "%s\r\n", extra_header);
	if (mime_type != (char*) 0)
		(void) fprintf(conn_fp, "Content-Type: %s\r\n", mime_type);
	(void) fprintf(conn_fp, "Connection: close\r\n");
	(void) fprintf(conn_fp, "\r\n");
}

/* Base-64 decoding.  This represents binary data as printable ASCII
 ** characters.  Three 8-bit binary bytes are turned into four 6-bit
 ** values, like so:
 **
 **   [11111111]  [22222222]  [33333333]
 **
 **   [111111] [112222] [222233] [333333]
 **
 ** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
 */

static int b64_decode_table[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, /* 00-0F */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 10-1F */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /* 20-2F */
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, /* 30-3F */
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, /* 40-4F */
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /* 50-5F */
		-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 60-6F */
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /* 70-7F */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 80-8F */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 90-9F */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* A0-AF */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* B0-BF */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* C0-CF */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* D0-DF */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* E0-EF */
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 /* F0-FF */
};

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
 ** Return the actual number of bytes generated.  The decoded size will
 ** be at most 3/4 the size of the encoded, and may be smaller if there
 ** are padding characters (blanks, newlines).
 */
static int b64_decode(const char* str, char* space, int size) {
	const char* cp;
	int space_idx, phase;
	int d, prev_d = 0;
	unsigned char c;

	space_idx = 0;
	phase = 0;
	for (cp = str; *cp != '\0'; ++cp) {
		d = b64_decode_table[(int) *cp];
		if (d != -1) {
			switch (phase) {
			case 0:
				++phase;
				break;
			case 1:
				c = ((prev_d << 2) | ((d & 0x30) >> 4));
				if (space_idx < size)
					space[space_idx++] = c;
				++phase;
				break;
			case 2:
				c = (((prev_d & 0xf) << 4) | ((d & 0x3c) >> 2));
				if (space_idx < size)
					space[space_idx++] = c;
				++phase;
				break;
			case 3:
				c = (((prev_d & 0x03) << 6) | d);
				if (space_idx < size)
					space[space_idx++] = c;
				phase = 0;
				break;
			}
			prev_d = d;
		}
	}
	return space_idx;
}

/* Simple shell-style filename matcher.  Only does ? * and **, and multiple
 ** patterns separated by |.  Returns 1 or 0.
 */
int match(const char* pattern, const char* string) {
	const char* or;

	for (;;) {
		or = strchr(pattern, '|');
		if (or == (char*) 0)
			return match_one(pattern, strlen(pattern), string);
		if (match_one(pattern, or - pattern, string))
			return 1;
		pattern = or + 1;
	}
	return 0;
}

static int match_one(const char* pattern, int patternlen, const char* string) {
	const char* p;

	for (p = pattern; p - pattern < patternlen; ++p, ++string) {
		if (*p == '?' && *string != '\0')
			continue;
		if (*p == '*') {
			int i, pl;
			++p;
			if (*p == '*') {
				/* Double-wildcard matches anything. */
				++p;
				i = strlen(string);
			} else
				/* Single-wildcard matches anything but slash. */
				i = strcspn(string, "/");
			pl = patternlen - (p - pattern);
			for (; i >= 0; --i)
				if (match_one(p, pl, &(string[i])))
					return 1;
			return 0;
		}
		if (*p != *string)
			return 0;
	}
	if (*string == '\0')
		return 1;
	return 0;
}

static void handle_request(void) {
	char line[10000], *cur;
	char *method, *path, *protocol, *authorization, *boundary;
	char *cp;
	char *file;
	int len;
	struct mime_handler *handler;
	int cl = 0, flags;

	/* Initialize the request variables. */
	authorization = boundary = NULL;
	bzero(line, sizeof line);

	/* Parse the first line of the request. */
	if (fgets(line, sizeof(line), conn_fp) == (char*) 0) {
		send_error(400, "Bad Request", (char*) 0, "No request found.");
		return;
	}

	method = path = line;
	strsep(&path, " ");
	while (*path == ' ')
		path++;
	protocol = path;
	strsep(&protocol, " ");
	while (*protocol == ' ')
		protocol++;
	cp = protocol;
	strsep(&cp, " ");
	if (!method || !path || !protocol) {
		send_error(400, "Bad Request", (char*) 0, "Can't parse request.");
		return;
	}
	cur = protocol + strlen(protocol) + 1;

	/* Parse the rest of the request headers. */
	while (fgets(cur, line + sizeof(line) - cur, conn_fp) != (char*) 0) {

		if (strcmp(cur, "\n") == 0 || strcmp(cur, "\r\n") == 0) {
			break;
		} else if (strncasecmp(cur, "Authorization:", 14) == 0) {
			cp = &cur[14];
			cp += strspn(cp, " \t");
			authorization = cp;
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Content-Length:", 15) == 0) {
			cp = &cur[15];
			cp += strspn(cp, " \t");
			cl = strtoul(cp, NULL, 0);
		} else if ((cp = strstr(cur, "boundary="))) {
			boundary = &cp[9];
			for (cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++)
				;
			*cp = '\0';
			cur = ++cp;
		}

	}

	if (strcasecmp(method, "get") != 0 && strcasecmp(method, "post") != 0) {
		send_error(501, "Not Implemented", (char*) 0,
				"That method is not implemented.");
		return;
	}
	if (path[0] != '/') {
		send_error(400, "Bad Request", (char*) 0, "Bad filename.");
		return;
	}
	file = &(path[1]);
	len = strlen(file);
	if (file[0] == '/' || strcmp(file, "..") == 0
			|| strncmp(file, "../", 3) == 0 || strstr(file, "/../") != (char*) 0
			|| strcmp(&(file[len - 3]), "/..") == 0) {
		send_error(400, "Bad Request", (char*) 0, "Illegal filename.");
		return;
	}

	redirect = 0;

	//printf("File: %s\n", file);

	if (http_port == SERVER_PORT && !http_login_check()) {
		sprintf(line,
				"Please log out user, %.8x, first or wait for session timeout(60 seconds).",
				ntohl(login_ip));

		printf("Response: %s \n", line);
		send_error(200, "Request is rejected", (char*) 0, line);
		return;
	}

	if ((file[0] == '\0' || file[len - 1] == '/')) {
		{
			file = "index.html";
		}
	}

	for (handler = &mime_handlers[0]; handler->pattern; handler++) {
		if (match(handler->pattern, file)) {
			if (handler->auth) {
				handler->auth(auth_userid, auth_passwd, auth_realm);
				if (!auth_check(auth_realm, authorization))
					return;
			}

			if (!redirect)
				http_login(login_ip_tmp);

			if (strcasecmp(method, "post") == 0 && !handler->input) {
				send_error(501, "Not Implemented", NULL,
						"That method is not implemented.");
				return;
			}
			if (handler->input) {
				handler->input(file, conn_fp, cl, boundary);
				if ((flags = fcntl(fileno(conn_fp), F_GETFL)) != -1
						&& fcntl(fileno(conn_fp), F_SETFL, flags | O_NONBLOCK)
								!= -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp) != EOF)
						(void) fgetc(conn_fp);
					fcntl(fileno(conn_fp), F_SETFL, flags);
				}
			}
			send_headers(200, "OK", handler->extra_header, handler->mime_type);
			if (handler->output)
				handler->output(file, conn_fp);
//			if ((handler->output) && (!handler->output(file, conn_fp)))
//				send_error(404, "Not Found", (char*) 0, "File not found.");
//			else
//				send_headers(200, "OK", handler->extra_header, handler->mime_type);
			break;
		}
	}

	if (!handler->pattern)
		send_error(404, "Not Found", (char*) 0, "File not found.");

	if (strcmp(file, "Logout") == 0) {
		http_logout(login_ip_tmp);
	}
}

int main(int argc, char **argv) {
	usockaddr usa;
	int listen_fd;
	int conn_fd;
	int init_ip = 0;
	socklen_t sz = sizeof(usa);
	char pidfile[32];

	reloadNVRAMs();

	// Added by Joey for handling WAN Interface
	// usage: httpd [wan interface] [port]
	if (argc > 2)
		http_port = atoi(argv[2]);
	if (argc > 1)
		strcpy(wan_if, argv[1]);
	else
		strcpy(wan_if, "");

	//websSetVer();
	//init_ip = atoi(nvram_safe_get("login_ip"));
	//if (init_ip!=0)
	//       http_login(init_ip);

	/* Ignore broken pipes */
	signal(SIGPIPE, SIG_IGN);

	/* Initialize listen socket */
	if ((listen_fd = initialize_listen_socket(&usa)) < 0) {
		fprintf(stderr, "can't bind to any address\n");
		exit(errno);
	}

#if !defined(DEBUG) && !defined(vxworks)
	{
		FILE *pid_fp;
		/* Daemonize and log PID */
		//if (http_port==SERVER_PORT)
		//{
#ifdef REMOVE
		if (daemon(1, 1) == -1)
		{
			perror("daemon");
			exit(errno);
		}
#endif
		//}
		if (http_port==SERVER_PORT)
		strcpy(pidfile, "/var/run/httpd.pid");
		else sprintf(pidfile, "/var/run/httpd-%d.pid", http_port);

		if (!(pid_fp = fopen(pidfile, "w"))) {
			perror(pidfile);
			return errno;
		}
		fprintf(pid_fp, "%d", getpid());
		fclose(pid_fp);
	}
#endif

	/* Loop forever handling requests */
	for (;;) {
		if ((conn_fd = accept(listen_fd, &usa.sa, &sz)) < 0) {
			perror("accept");
			return errno;
		}
		if (!(conn_fp = fdopen(conn_fd, "r+"))) {
			perror("fdopen");
			return errno;
		}
		http_login_cache(&usa);
		handle_request();
		fflush(conn_fp);
		fclose(conn_fp);
		close(conn_fd);
	}

	shutdown(listen_fd, 2);
	close(listen_fd);

	return 0;
}
