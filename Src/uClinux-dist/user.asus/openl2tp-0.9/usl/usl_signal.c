/*****************************************************************************
 * Copyright (C) 2004 Katalix Systems Ltd
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA 
 *
 *****************************************************************************/

/*
 * Signal notification.
 * Applications register callbacks to be called when specific signal is
 * generated.
 */

#include "usl.h"
#include <sys/socket.h>		
#include <netinet/in.h>		
#include <arpa/inet.h>		
#include <unistd.h>			
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include "../l2tp_rpc.h"	
#include "../l2tp_private.h"
#include <bcmnvram.h>
#include <time.h>

struct usl_notifier {
	struct usl_list_head		list;
	usl_notify_fn_t			func;
	void				*arg;
};

static					USL_LIST_HEAD(usl_sig_notifier_list);
static usl_signal_data_t		usl_sigdata;

void (*usl_signal_terminate_hook)(void) = NULL;

#ifdef WX54G
int break_usl_fd_pool = 0;
#endif

#ifdef DEBUG
/* Useful for test harnesses to run code for a few events then return */
int usl_main_loop_max_count = 0;
#endif

static void usl_signal_notify(int val)
{
	struct usl_list_head *walk;
	struct usl_list_head *tmp;
	struct usl_notifier *notifier;

	usl_list_for_each(walk, tmp, &usl_sig_notifier_list) {
		notifier = usl_list_entry(walk, struct usl_notifier, list);
		(*notifier->func)(notifier->arg, val);
	}
}

static void usl_signal_procgroup(int sig)
{
	struct sigaction act, oldact;

	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	killpg(0, sig);
	sigaction(sig, &act, &oldact);
	sigaction(sig, &oldact, NULL);
}

static void usl_sighup(int sig)
{
	usl_sigdata.sighup = 1;
	if (usl_sigdata.running) {
		usl_signal_procgroup(sig);
	}
	usl_signal_notify(sig);
	if (usl_sigdata.waiting) {
#ifdef WX54G
		// If we set break_usl_fd_pool, openl2tpd will crash when it receive SIGHUP.
//		break_usl_fd_pool = 1;
#else
		siglongjmp(usl_sigdata.sigjmp, 1);
#endif
	}
}

static void usl_sigterm(int sig)
{
	usl_sigdata.sigterm = 1;
	if (usl_sigdata.running) {
		usl_signal_procgroup(sig);
	}
	usl_signal_notify(sig);
	if (usl_sigdata.waiting) {
#ifdef WX54G
		break_usl_fd_pool = 1;
#else
		siglongjmp(usl_sigdata.sigjmp, 1);
#endif
	}
}

static void usl_sigchld(int sig)
{
	/* Child processes are reaped in the main process loop */
	usl_sigdata.sigchld = 1;
	usl_signal_notify(sig);
}

static void usl_sigalrm(int sig)
{
	usl_timer_tick();
}

static void usl_sigusr1(int sig)
{
	usl_signal_notify(sig);
}

static void usl_sigusr2(int sig)
{
	usl_signal_notify(sig);
}

static void usl_sigbad(int sig)
{
	static sig_atomic_t crashed = 0;

	if (crashed) {
		_exit(127);
	}
	crashed = 1;
	USL_SYSLOG(LOG_ERR, "Fatal signal %d", sig);
	if (usl_sigdata.running) {
		usl_signal_procgroup(SIGTERM);
	}
	usl_signal_notify(sig);
	exit(127);
}

/* Allow an application to jump back into the main loop.
 */
void usl_signal_continue(void)
{
#ifdef WX54G
	break_usl_fd_pool = 1;
#else
	siglongjmp(usl_sigdata.sigjmp, 1);
#endif
	/* NOTREACHED */
}

int usl_signal_notifier_add(usl_notify_fn_t func, void *arg)
{
	struct usl_notifier *notifier;

	notifier = (struct usl_notifier *) calloc(1, sizeof(struct usl_notifier));
	if (notifier == NULL) {
		USL_DEBUG("%s: out of memory", __FUNCTION__);
		return -ENOMEM;
	}
	USL_LIST_HEAD_INIT(&notifier->list);
	notifier->func = func;
	notifier->arg = arg;
	usl_list_add(&notifier->list, &usl_sig_notifier_list);

	return 0;
}

void usl_signal_notifier_remove(usl_notify_fn_t func, void *arg)
{
	struct usl_list_head *walk;
	struct usl_list_head *tmp;
	struct usl_notifier *notifier;

	usl_list_for_each(walk, tmp, &usl_sig_notifier_list) {
		notifier = usl_list_entry(walk, struct usl_notifier, list);
		if (notifier->func == func && notifier->arg == arg) {
			usl_list_del(&notifier->list);
			USL_POISON_MEMORY(notifier, 0xf3, sizeof(*notifier));
			free(notifier);
			return;
		}
	}
}

#ifdef WX54G
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)
#define ST_START	0
#define ST_CTUNL	1
#define ST_CSESS	2
#define ST_SESSOK	3
#define ST_SESSOK1	4
static int state = ST_START;


static int
route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask)
{
    int s;
    struct rtentry rt;

    /* Open a raw socket to the kernel */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        goto err;

    /* Fill in rtentry */
    memset(&rt, 0, sizeof(rt));
    if (dst)
        inet_aton(dst, &sin_addr(&rt.rt_dst));
    if (gateway)
        inet_aton(gateway, &sin_addr(&rt.rt_gateway));
    if (genmask)
        inet_aton(genmask, &sin_addr(&rt.rt_genmask));
    rt.rt_metric = metric;
    rt.rt_flags = RTF_UP;
    if (sin_addr(&rt.rt_gateway).s_addr)
        rt.rt_flags |= RTF_GATEWAY;
    if (sin_addr(&rt.rt_genmask).s_addr == INADDR_BROADCAST)
        rt.rt_flags |= RTF_HOST;
    rt.rt_dev = name;

    /* Force address family to AF_INET */     rt.rt_dst.sa_family = AF_INET;
    rt.rt_gateway.sa_family = AF_INET;     rt.rt_genmask.sa_family = AF_INET;      if (ioctl(s, cmd, &rt) < 0)
        goto err;

    close(s);
    return 0;

 err:
    close(s);
    perror(name);
    return errno;
}

static int
route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
    return route_manip(SIOCADDRT, name, metric, dst, gateway, genmask);
}


#if 0 //eric--
static void
modify_system (void)
{
	struct l2tp_api_system_msg_data smsg;
	int result;

	memset (&smsg, 0, sizeof (smsg));

	// change pendto timeout 
	// (openl2tpd will reconnect tunnel and session after it was breaker for tunnel_persist_pend_timeout seconds)
	smsg.config.tunnel_persist_pend_timeout = 120;	// minium value is 120 seconds. refer to l2tp_tunnel_globals_modify()
	smsg.config.flags |= L2TP_API_CONFIG_FLAG_TUNNEL_PERSIST_PEND_TIMEOUT;

	l2tp_system_modify_1_svc(smsg, &result, NULL);				// always return TRUE
}
#endif // #if 0


static void
modify_ppp_profile (void)
{
	struct l2tp_api_ppp_profile_msg_data pmsg;
	int result;
	bool_t bRes;
	int lcp_echo_failure_count = 3;
	char *p;

	// modify ppp profile
	/******************************** modify ppp profile *************************/
	memset (&pmsg, 0, sizeof (pmsg));

	// profile name, essential parameter
	pmsg.profile_name = "default";
	
	// MTU
	pmsg.mtu = 1464;								// this value will not greater than mtu of tunnel
	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_MTU;

	// MRU
	pmsg.mru = 1464;								// this value will not greater than mru of tunnel
	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_MRU;

	// SYNC Mode
	// L2TP_API_PPP_SYNCMODE_SYNC, L2TP_API_PPP_SYNCMODE_ASYNC, L2TP_API_PPP_SYNCMODE_SYNC_ASYNC
	pmsg.sync_mode = L2TP_API_PPP_SYNCMODE_SYNC;
	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_SYNC_MODE;

	// PAP Auth
	// enable : pmsg.auth_flags &= ~L2TP_API_PPP_AUTH_REFUSE_PAP
	// disable: pmsg.auth_flags |=  L2TP_API_PPP_AUTH_REFUSE_PAP
//	pmsg.auth_flags &= ~L2TP_API_PPP_AUTH_REFUSE_PAP;
//	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_AUTH_FLAGS;

	// Auth. none
	pmsg.auth_flags |= L2TP_API_PPP_AUTH_NONE;
//	pmsg.auth_flags &= ~L2TP_API_PPP_AUTH_NONE;
	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_AUTH_FLAGS;

	// Idle timeout
	pmsg.idle_timeout = 1800; // unit: seconds
	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_IDLE_TIMEOUT;

	// LCP echo fail count
	lcp_echo_failure_count = 3;
	p = nvram_safe_get ("lcp_echo_failure_count");
	if (strlen (p) != 0 && atoi (p) > 0)	{
		lcp_echo_failure_count = atoi (p);
		l2tp_log (LOG_INFO, "!!!override LCP Echo Failure Count: %d", lcp_echo_failure_count);
	}
	pmsg.lcp_echo_failure_count = lcp_echo_failure_count;
	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_LCP_ECHO_FAILURE_COUNT;

	// LCP echo interval
	pmsg.lcp_echo_interval = 10;
	pmsg.flags |= L2TP_API_PPP_PROFILE_FLAG_LCP_ECHO_INTERVAL;

	// Default route
//	pmsg.use_as_default_route = L2TP_API_PPP_PROFILE_DEFAULT_USE_AS_DEFAULT_ROUTE;
//	pmsg.flags2 |= L2TP_API_PPP_PROFILE_FLAG_USE_AS_DEFAULT_ROUTE;
	pmsg.flags2 &= ~(L2TP_API_PPP_PROFILE_FLAG_USE_AS_DEFAULT_ROUTE);

	// Apply settings
	bRes = l2tp_ppp_profile_modify_1_svc (pmsg, &result, NULL);
//	l2tp_log (LOG_INFO, "******l2tp_ppp_profile_modify_1_svc() return %d result %d\n", bRes, result);
}


static int
create_tunnel (struct l2tp_api_tunnel_msg_data *tmsg, char *username, char *secret)
{
	int tunl_id, ret_val;
	bool_t bRes;

	/******************************** build a tunnel *****************************/
	ret_val = 0;
	tunl_id = 0;

	tmsg->flags |= L2TP_API_TUNNEL_FLAG_PEER_ADDR;	// eric, essential parameters

	// profile name
	OPTSTRING(tmsg->tunnel_profile_name) = "default";
	tmsg->tunnel_profile_name.valid = 1;
	tmsg->flags |= L2TP_API_TUNNEL_FLAG_PROFILE_NAME;

	// framing type
//	tmsg->framing_cap_sync = TRUE;
//	tmsg->framing_cap_async = FALSE;
//	tmsg->flags |= L2TP_API_TUNNEL_FLAG_FRAMING_CAP;

	// L2TP authentication mode
	// L2TP_API_TUNNEL_AUTH_MODE_NONE, L2TP_API_TUNNEL_AUTH_MODE_SIMPLE, L2TP_API_TUNNEL_AUTH_MODE_CHALLENGE
	tmsg->auth_mode = L2TP_API_TUNNEL_AUTH_MODE_NONE; 
	tmsg->flags |= L2TP_API_TUNNEL_FLAG_AUTH_MODE;

	// secret
	if (secret != NULL || strlen (secret) != 0)	{
//		l2tp_log (LOG_INFO, "secret: %s", secret);
		OPTSTRING(tmsg->secret) = secret;
		tmsg->secret.valid = 1;
		tmsg->flags |= L2TP_API_TUNNEL_FLAG_SECRET;

		// Request LNS send Challenge Response AVP in SCCRQ
//		tmsg->auth_mode = L2TP_API_TUNNEL_AUTH_MODE_CHALLENGE; 
	}

	// tunnel name
//	OPTSTRING(tmsg->tunnel_name) = "wx54g_tunnel";
//	tmsg->tunnel_name.valid = 1;
//	tmsg->flags |= L2TP_API_TUNNEL_FLAG_TUNNEL_NAME;

	// persist
	tmsg->persist = 1;
	tmsg->flags |= L2TP_API_TUNNEL_FLAG_PERSIST;

	// hostname
	if (username != NULL && strlen (username) != 0)	{
//		l2tp_log (LOG_INFO, "hostname: %s\n", username);
		OPTSTRING(tmsg->host_name) = username;
		tmsg->host_name.valid = 1;
		tmsg->flags |= L2TP_API_TUNNEL_FLAG_HOST_NAME;
	}

	// UDP port
	tmsg->our_udp_port = 1701;
	tmsg->flags |= L2TP_API_TUNNEL_FLAG_OUR_UDP_PORT;
	tmsg->our_addr.s_addr = INADDR_ANY;
	tmsg->peer_udp_port = 1701;
	tmsg->flags |= L2TP_API_TUNNEL_FLAG_PEER_UDP_PORT;

	bRes = l2tp_tunnel_create_1_svc (*tmsg, &tunl_id, NULL);

	if (tunl_id > 0)	{
		l2tp_log (LOG_INFO, "tunnel id: %d. Waiting for reply...", tunl_id);
		ret_val = tunl_id;
	}

	return ret_val;
}


static int
create_session (struct l2tp_api_session_msg_data *smsg, int tunl_id, char *username, char *password)
{
	int sess_id, ret_val;
	bool_t bRes;

	if (tunl_id == 0)	{
		return 0;
	}
	ret_val = 0;

	// tunnel id
	smsg->tunnel_id = tunl_id;

	// session type
	// L2TP_API_SESSION_TYPE_LAIC, L2TP_API_SESSION_TYPE_LAOC, L2TP_API_SESSION_TYPE_LNIC, L2TP_API_SESSION_TYPE_LNOC
	smsg->session_type = L2TP_API_SESSION_TYPE_LAIC;
	smsg->flags |= L2TP_API_SESSION_FLAG_SESSION_TYPE;

	// ppp user name
	if (username != NULL && strlen (username) != 0)	{
//		l2tp_log (LOG_INFO, "wan_pppoe_username: (%s)\n", username);
		OPTSTRING(smsg->user_name) = username;
		smsg->user_name.valid = 1;
		smsg->flags |= L2TP_API_SESSION_FLAG_USER_NAME;
	}
	
	// ppp user password
	if (password != NULL && strlen (password) != 0)	{
//		l2tp_log (LOG_INFO, "wan_pppoe_passwd: (%s)\n", password);
		OPTSTRING(smsg->user_password) = password;
		smsg->user_password.valid = 1;
		smsg->flags |= L2TP_API_SESSION_FLAG_USER_PASSWORD;
	}

	// framing type (sync)
	smsg->framing_type_sync = TRUE;
	smsg->framing_type_async = FALSE;
	smsg->flags |= L2TP_API_SESSION_FLAG_FRAMING_TYPE;

	// connect speed
	smsg->rx_connect_speed = 10000000;
	smsg->tx_connect_speed = 10000000;
	smsg->flags |= L2TP_API_SESSION_FLAG_CONNECT_SPEED;

	// ppp profile name
	OPTSTRING(smsg->ppp_profile_name) = "default";
	smsg->ppp_profile_name.valid = 1;
	smsg->flags |= L2TP_API_SESSION_FLAG_PPP_PROFILE_NAME;

	// calling number
	OPTSTRING(smsg->calling_number) = "1";
	smsg->calling_number.valid = 1;
	smsg->flags |= L2TP_API_SESSION_FLAG_CALLING_NUMBER;

	bRes = l2tp_session_create_1_svc(*smsg, &sess_id, NULL);
//	l2tp_log (LOG_INFO, "******l2tp_session_create_1_svc() return %d sess_id %d tunnel_id %d\n", bRes, sess_id, smsg->tunnel_id);

	if (sess_id > 0)	{
		l2tp_log (LOG_INFO, "tunnel/session id: %d/%d. Waiting for reply...", smsg->tunnel_id, sess_id);
		ret_val = sess_id;
	}

	return ret_val;
}
#endif

void usl_main_loop(void)
{
	sigset_t mask;
#ifdef DEBUG
	int loop_count;
#endif

#ifdef WX54G	// try to build a tunnel
//	int flags;
	int tunl_id, sess_id, cnt, result;
	struct hostent *h;
	optstring tunl_name, sess_name;
	unsigned char *p, *pstate, *pstate1, pu[72], pp[72];
	struct l2tp_api_tunnel_msg_data tmsg;
	struct l2tp_api_session_msg_data smsg;
	time_t t1, t2;

	memset (&tmsg, 0, sizeof (tmsg));
	memset (&smsg, 0, sizeof (smsg));


	// If you have any question about following configuration, reference l2tp_config.c
//4	modify_system ();
	modify_ppp_profile ();

	// resolve name of L2TP Server
	cnt = 0;
	p = nvram_safe_get ("wan_heartbeat_x");
	if (strlen (p) == 0)	{
		l2tp_log (LOG_INFO, "Invalid L2TP server; Exiting...");
		exit (1);
	}
	do	{
		h = (struct hostent *)gethostbyname(p);
		if (h != NULL && h->h_addrtype == AF_INET)	{
			break;
		}
		sleep (1);
	} while (cnt++ < 50);
	if (h == NULL)	{
		l2tp_log (LOG_INFO, "Resolve (%s) failure. Exiting...\n", p);
		exit (1);
	}

    memcpy(&tmsg.peer_addr.s_addr, h->h_addr, sizeof(tmsg.peer_addr.s_addr));
	l2tp_log (LOG_INFO, "L2TP Server: %s/0x%X", p, tmsg.peer_addr.s_addr);

	// add a route to L2TP Server through default gateway (for Terayon)
	route_add("WAN", 0, inet_ntoa(*(struct in_addr *) &tmsg.peer_addr.s_addr), nvram_safe_get("wan0_gateway"), "255.255.255.255");

	memset (pu, 0, sizeof (pu));
	strncpy (pu, nvram_safe_get ("wan_pppoe_username"), sizeof (pu) - 1);
	memset (pp, 0, sizeof (pp));
	strncpy (pp, nvram_safe_get ("wan_pppoe_passwd"), sizeof (pp) - 1);

	tunl_id = sess_id = 0;
#endif // WX54G

#ifdef DEBUG
	loop_count = 0;
#endif

	for (;;) {
#ifdef WX54G
	switch (state)	{
		case ST_SESSOK1:
			break;

		case ST_SESSOK:
			pstate = l2tp_tunnel_get_state_name (tunl_id);
			pstate1 = l2tp_session_get_state_name (tunl_id, sess_id);
			if (strcmp (pstate, "ESTABLISHED") != 0)	{
				l2tp_log (LOG_INFO, "tunnel %d state: %s", tunl_id, pstate);
				state = ST_START;
				l2tp_log (LOG_INFO, "ST_SESSOK -> ST_START");
			} else if (strcmp (pstate1, "ESTABLISHED") != 0)	{
				l2tp_log (LOG_INFO, "tunnel/session %d/%d state: %s/%s", tunl_id, sess_id, pstate, pstate1);

				// delete unavailable session
				tunl_name.valid = 0;
				OPTSTRING (tunl_name) = NULL;
				sess_name.valid = 0;
				OPTSTRING (sess_name) = NULL;
				l2tp_session_delete_1_svc(tunl_id, tunl_name, sess_id, sess_name, &result, NULL);
				if (result == 0)	{
//					l2tp_log (LOG_INFO, "Destroy tunnel/session %d/%d", tunl_id, sess_id);
					sess_id = 0;
				}
				
				state = ST_CSESS;
				l2tp_log (LOG_INFO, "ST_SESSOK -> ST_CSESS");
			}
			time (&t2);
			if (t2 - t1 > 60)	{
				state = ST_SESSOK1;
				l2tp_log (LOG_INFO, "ST_SESSOK -> ST_SESSOK1");
				break;
			}
			break;

		case ST_START:
			// Start
			tunl_id = 0;
			sess_id = 0;
			state = ST_CTUNL;
			// fall through
			
		case ST_CTUNL:
			// Create tunnel
			if (tunl_id == 0)	{
				l2tp_log (LOG_INFO, "Try to establish tunnel with %s (%d)...", p, l2tp_tunnel_get_num_tunnels ());
				tunl_id = create_tunnel (&tmsg, pu, pp);
			}

			pstate = l2tp_tunnel_get_state_name (tunl_id);
			if (strcmp (pstate, "ESTABLISHED") == 0)	{
				l2tp_log (LOG_INFO, "ST_CTUNL -> ST_CSESS");
				sess_id = 0;
				state = ST_CSESS;
			} else if (strcmp (pstate, "CLOSING") == 0)	{
				l2tp_log (LOG_INFO, "tunnel %d state: %s", tunl_id, pstate);

				tunl_name.valid = 0;
				OPTSTRING (tunl_name) = NULL;
				l2tp_tunnel_delete_1_svc(tunl_id, tunl_name, &result, NULL);
				if (result == 0)	{
//					l2tp_log (LOG_INFO, "Destroy tunnel %d", tunl_id);
					tunl_id = 0;
				}
			}
			break;

		case ST_CSESS:
			if (tunl_id == 0 || (tunl_id != 0 && strcmp (l2tp_tunnel_get_state_name (tunl_id), "ESTABLISHED") != 0))	{
				state = ST_CTUNL;
				break;
			} else if (tunl_id != 0 && sess_id == 0)	{
				l2tp_log (LOG_INFO, "tunnel %d state: %s", tunl_id, l2tp_tunnel_get_state_name (tunl_id));
				sess_id = create_session (&smsg, tunl_id, pu, pp);
			}

			pstate = l2tp_session_get_state_name (tunl_id, sess_id);
			pstate1 = l2tp_tunnel_get_state_name (tunl_id);
			if (strcmp (pstate, "ESTABLISHED") == 0)	{
//				l2tp_log (LOG_INFO, "tunnel/session %d/%d", tunl_id, sess_id);
				l2tp_log (LOG_INFO, "ST_CSESS -> ST_SESSOK", tunl_id, sess_id);
				time (&t1);
				state = ST_SESSOK;
			} else if (strcmp (pstate, "IDLE") == 0)	{
				l2tp_log (LOG_INFO, "tunnel/session %d/%d state: %s/%s", tunl_id, sess_id, pstate1, pstate);
				sess_id = 0;
			}
			break;
		default:
			l2tp_log (LOG_INFO, "Unknown state %d", state);
	}
#endif 

		
#ifdef DEBUG
		loop_count++;
		if ((usl_main_loop_max_count > 0) && (loop_count > usl_main_loop_max_count)) {
			loop_count = 0;
			usl_main_loop_max_count = 0;
			break;
		}
#endif
//	l2tp_log (LOG_INFO, "%s %s() %5d sigsetjmp()\n", __FILE__, __FUNCTION__, __LINE__); //eric++
#ifdef WX54G
		// nothing to do
#else
		if (sigsetjmp(usl_sigdata.sigjmp, 1) == 0) 
#endif
		{
//	l2tp_log (LOG_INFO, "%s %s() %5d sigsetjmp() OK\n", __FILE__, __FUNCTION__, __LINE__); //eric++
			sigprocmask(SIG_BLOCK, &mask, NULL);
			if (usl_sigdata.sighup || usl_sigdata.sigterm || usl_sigdata.sigchld) {
				usl_sigdata.waiting = 0;
				sigprocmask(SIG_UNBLOCK, &mask, NULL);
			} else {
				usl_sigdata.waiting = 1;
				sigprocmask(SIG_UNBLOCK, &mask, NULL);
				usl_fd_poll();
				continue;
			}
		}
		if (usl_sigdata.sighup) {
			/* We don't use this signal any more */
			usl_sigdata.sighup = 0;
		}
		if (usl_sigdata.sigterm) {
			if (usl_signal_terminate_hook != NULL) {
				(*usl_signal_terminate_hook)();
			}
			exit(1);
			/* NOTREACHED */
		}
		if (usl_sigdata.sigchld) {
			usl_sigdata.sigchld = 0;
			usl_pid_reap_children(0);     /* Don't leave dead kids lying around */
		}

	}
}

/*****************************************************************************
 * Init and cleanup
 *****************************************************************************/

void usl_signal_init(void)
{
	struct sigaction sa;
	sigset_t mask;
	static int initialized = 0;

	if (initialized) {
		return;
	}
	initialized = 1;

	memset(&usl_sigdata, 0, sizeof(usl_sigdata));

	/* Mask out any uninteresting signals. */
	sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGALRM);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);

#define SIGNAL(s, handler)	do { \
		sa.sa_handler = handler; \
		if (sigaction(s, &sa, NULL) < 0) { \
			USL_SYSLOG(LOG_ERR, "Couldn't establish signal handler (%d): %m", s); \
			exit(1); \
		} \
	} while (0)

	sa.sa_mask = mask;
	sa.sa_flags = 0;
	signal(SIGPIPE, SIG_IGN);

	/* Install handlers for interesting signals */
	SIGNAL(SIGHUP, usl_sighup);
	SIGNAL(SIGINT, usl_sigterm);
	SIGNAL(SIGTERM, usl_sigterm);
	SIGNAL(SIGCHLD, usl_sigchld);
	SIGNAL(SIGUSR1, usl_sigusr1);
	SIGNAL(SIGUSR2, usl_sigusr2);
	SIGNAL(SIGALRM, usl_sigalrm);
	SIGNAL(SIGSYS, usl_sigbad);
#ifndef DEBUG /* Don't catch app failures when debugging */
	SIGNAL(SIGABRT, usl_sigbad);
	SIGNAL(SIGFPE, usl_sigbad);
	SIGNAL(SIGILL, usl_sigbad);
	SIGNAL(SIGQUIT, usl_sigbad);
	SIGNAL(SIGSEGV, usl_sigbad);
	SIGNAL(SIGBUS, usl_sigbad);
#endif /* DEBUG */
	SIGNAL(SIGPOLL, usl_sigbad);
	SIGNAL(SIGPROF, usl_sigbad);
	SIGNAL(SIGTRAP, usl_sigbad);
	SIGNAL(SIGVTALRM, usl_sigbad);
	SIGNAL(SIGXCPU, usl_sigbad);
	SIGNAL(SIGXFSZ, usl_sigbad);
}

void usl_signal_cleanup(void)
{
}

