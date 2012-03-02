/*
 * Layer Two Tunnelling Protocol Daemon
 * Copyright (C) 1998 Adtran, Inc.
 * Copyright (C) 2002 Jeff McAdams
 *
 * Mark Spencer
 *
 * This software is distributed under the terms
 * of the GPL, which you should have received
 * along with this source.
 *
 * Network routines for UDP handling
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "l2tp.h"

char hostname[256];
unsigned int listen_addy = INADDR_ANY;	/* Address to listen on */
struct sockaddr_in server, from;	/* Server and transmitter structs */
int server_socket;				/* Server socket */
#ifdef USE_KERNEL
int kernel_support;				/* Kernel Support there or not? */
#endif

/*
 * Debugging info
 */
int debug_tunnel = 1;
int debug_network = 0;			/* Debug networking? */
int packet_dump = 0;			/* Dump packets? */
int debug_avp = 1;				/* Debug AVP negotiations? */
int debug_state = 0;			/* Debug state machine? */

int init_network(void)
{
	long arg;
	int length = sizeof(server);
	int debug_ret = 0;

	gethostname(hostname, sizeof(hostname));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(listen_addy);
	server.sin_port = htons(gconfig.port);
	if ((server_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		log(LOG_CRIT, "%s: Unable to allocate socket. Terminating.\n", __FUNCTION__);
		return -EINVAL;
	};
	/* L2TP/IPSec: Set up SA for listening port here?  NTB 20011015
	 */
	//SJ_Yen
	debug_ret = bind(server_socket, (struct sockaddr *) &server, sizeof(server));

//    if (bind (server_socket, (struct sockaddr *) &server, sizeof (server)))
	if (debug_ret != 0)
	{
		close(server_socket);
		log(LOG_CRIT, "%s: Unable to bind socket. Terminating.\n", __FUNCTION__);
		return -EINVAL;
	};
	if (getsockname(server_socket, (struct sockaddr *) &server, &length))
	{
		log(LOG_CRIT, "%s: Unable to read socket name.Terminating.\n", __FUNCTION__);
		return -EINVAL;
	}
#ifdef USE_KERNEL
	if (gconfig.forceuserspace)
	{
		log(LOG_LOG, "Not looking for kernel support.\n");
		kernel_support = 0;
	} else
	{
		if (ioctl(server_socket, SIOCSETL2TP, NULL) < 0)
		{
			log(LOG_LOG, "L2TP kernel support not detected.\n");
			kernel_support = 0;
		} else
		{
			log(LOG_LOG, "Using l2tp kernel support.\n");
			kernel_support = -1;
		}
	}
#else
	log(LOG_LOG, "This binary does not support kernel L2TP.\n");
#endif
	arg = fcntl(server_socket, F_GETFL);
	arg |= O_NONBLOCK;
	fcntl(server_socket, F_SETFL, arg);
	gconfig.port = ntohs(server.sin_port);
	return 0;
}

inline void extract(void *buf, int *tunnel, int *call)
{
	/*
	 * Extract the tunnel and call #'s, and fix the order of the 
	 * version
	 */

	struct payload_hdr *p = (struct payload_hdr *) buf;
	if (PLBIT(p->ver))
	{
		*tunnel = p->tid;
		*call = p->cid;
	} else
	{
		*tunnel = p->length;
		*call = p->tid;
	}
}

inline void fix_hdr(void *buf)
{
	/*
	 * Fix the byte order of the header
	 */

	struct payload_hdr *p = (struct payload_hdr *) buf;
	_u16 ver = ntohs(p->ver);
	if (CTBIT(p->ver))
	{
		/*
		 * Control headers are always
		 * exactly 12 bytes big.
		 */
		swaps(buf, 12);
	} else
	{
		int len = 6;
		if (PSBIT(ver))
			len += 4;
		if (PLBIT(ver))
			len += 2;
		if (PFBIT(ver))
			len += 4;
		swaps(buf, len);
	}
}

void dethrottle(void *call)
{
/*	struct call *c = (struct call *)call; */
/*	if (c->throttle) {
#ifdef DEBUG_FLOW
		log(LOG_DEBUG, "%s: dethrottling call %d, and setting R-bit\n",__FUNCTION__,c->ourcid); 
#endif 		c->rbit = RBIT;
		c->throttle = 0;
	} else {
		log(LOG_DEBUG, "%s:  call %d already dethrottled?\n",__FUNCTION__,c->ourcid); 	
	} */
}

void control_xmit(void *b)
{
	struct buffer *buf = (struct buffer *) b;
	struct tunnel *t;
	struct timeval tv;
	int ns;
	if (!buf)
	{
		log(LOG_WARN, "%s: called on NULL buffer!\n", __FUNCTION__);
		return;
	}

	buf->retries++;
	t = buf->tunnel;
	ns = ntohs(((struct control_hdr *) (buf->start))->Ns);
	if (t)
	{
		if (ns < t->cLr)
		{
#ifdef DEBUG_CONTROL_XMIT
			log(LOG_DEBUG, "%s: Tossing packet %d\n", __FUNCTION__, ns);
#endif
			/* Okay, it's been received.  Let's toss it now */
			toss(buf);
			return;
		}
	}
	if (buf->retries > DEFAULT_MAX_RETRIES)
	{
		/*
		 * Too many retries.  Either kill the tunnel, or
		 * if there is no tunnel, just stop retransmitting.
		 */
		if (t)
		{
			if (t->self->needclose)
			{
				log(LOG_DEBUG, "%s: Unable to deliver closing message for tunnel %d. Destroying anyway.\n", __FUNCTION__, t->ourtid);
				t->self->needclose = 0;
				t->self->closing = -1;
			} else
			{
				log(LOG_DEBUG, "%s: Maximum retries exceeded for tunnel %d.  Closing.\n", __FUNCTION__, t->ourtid);
				strcpy(t->self->errormsg, "Timeout");
				t->self->needclose = -1;
			}
		}
	} else
	{
		/*
		 * FIXME:  How about adaptive timeouts?
		 */
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		schedule(tv, control_xmit, buf);
#ifdef DEBUG_CONTROL_XMIT
		log(LOG_DEBUG, "%s: Scheduling and transmitting packet %d\n", __FUNCTION__, ns);
#endif
		udp_xmit(buf);
	}
}

void udp_xmit(struct buffer *buf)
{
	int xxx = 0;
	/*
	 * Just send it all out.
	 */
//  printf("packet dump for UDP_XMIT\n");
//  do_packet_dump(buf);


#if 0
	struct sockaddr_in to;
	to.sin_family = AF_INET;
	to.sin_port = buf->port;
	/* if (buf->retry>-1) buf->retry++; */
	bcopy(&buf->addr, &to.sin_addr, sizeof(buf->addr));
#endif

//  printf("peer.sin_family=%x, peer.sin_addr.s_addr=%x, peer.sin_port=%x \n", buf->peer.sin_family, buf->peer.sin_addr.s_addr, buf->peer.sin_port);
//  printf("socket.start.size=%x,socket.len.size=%x,socket.peer.size=%x\n",sizeof(buf->start),sizeof(buf->len),sizeof(buf->peer));

//  do_packet_dump(&buf->start);
//  do_packet_dump(&buf->peer);
	xxx = sendto(server_socket, buf->start, buf->len, 0, (struct sockaddr *) &buf->peer, sizeof(buf->peer));
#if 0							//SJ_Yen
	xxx = sendto(server_socket, buf->start, buf->len, 0, (struct sockaddr *) &buf->peer, sizeof(buf->peer));

	xxx = sendto(server_socket, buf->start, buf->len, 0, (struct sockaddr *) &buf->peer, sizeof(buf->peer));

	xxx = sendto(server_socket, buf->start, buf->len, 0, (struct sockaddr *) &buf->peer, sizeof(buf->peer));
#endif
//  log (LOG_DEBUG, "server_socket=%x\n", server_socket);
//  log (LOG_DEBUG, "sendto=%x, errno=%x\n", xxx, errno);
//  log (LOG_DEBUG, "sockaddr=%x\n", (struct sockaddr *) &buf->peer);
//  log_asus (LOG_DEBUG, "peer.sin_family=%x, peer.sin_addr.s_addr=%x, peer.sin_port=%x \n", buf->peer.sin_family, buf->peer.sin_addr.s_addr, buf->peer.sin_port);

}

void network_thread()
{
	/*
	 * We loop forever waiting on either data from the ppp drivers or from
	 * our network socket.  Control handling is no longer done here.
	 */
	int fromlen;				/* Length of the address */
	int tunnel, call;			/* Tunnel and call */
	int recvsize;				/* Length of data received */
	struct buffer *buf;			/* Payload buffer */
	struct call *c, *sc;		/* Call to send this off to */
	struct tunnel *st;			/* Tunnel */
	fd_set readfds;				/* Descriptors to watch for reading */
	int max;					/* Highest fd */
	struct timeval tv;			/* Timeout for select */
	int debug_num = 0;

#if 0
	char *sendtomsg1 = "11111TestSendtoFunc";
	char *sendtomsg2 = "22222TestSendtoFunc";
	char *sendtomsg3 = "33333TestSendtoFunc";
	int lenofmsg1 = sizeof(sendtomsg1) + sizeof(buf->peer);
	int lenofmsg2 = sizeof(sendtomsg2) + sizeof(buf->peer);
	int lenofmsg3 = sizeof(sendtomsg3) + sizeof(buf->peer);
#endif
	/* This one buffer can be recycled for everything except control packets */
	buf = new_buf(MAX_RECV_SIZE);
	for (;;)
	{
		/*
		 * First, let's send out any outgoing packets that are waiting on us.
		 * xmit_udp should only
		 * contain control packets in the unthreaded version!
		 */
		max = 0;

		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before FD_ZERO(&XXXX)@network.c\n");
		}

		FD_ZERO(&readfds);

		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
			//              log (LOG_DEBUG, "Debug msg before st=tunnels.head@network.c\n");
		}

		st = tunnels.head;

		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before while(st)@network.c\n");
		}

		while (st)
		{
			if (st->self->needclose ^ st->self->closing)
			{
				if (debug_tunnel)
					log(LOG_DEBUG, "%S: closing down tunnel %d\n", __FUNCTION__, st->ourtid);
				call_close(st->self);
				/* Reset the while loop
				   and check for NULL */
				st = tunnels.head;
				if (!st)
				{
					if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
					{
//                  log (LOG_DEBUG, "Debug msg before break at while(st)@network.c\n");
					}
					break;
				}
				continue;
			}
			if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
			{
//                log (LOG_DEBUG, "Debug msg before sc = st->call_head@network.c\n");
			}
			sc = st->call_head;

			if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
			{
//                log (LOG_DEBUG, "Debug msg before while(sc)@network.c\n");
			}

			while (sc)
			{
				if (sc->needclose ^ sc->closing)
				{
					call_close(sc);
					sc = st->call_head;
					if (!sc)
					{
						if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
						{
//                  log (LOG_DEBUG, "Debug msg before break at while(sc)@network.c\n");
						}
						break;
					}
					continue;
				}
				if (sc->fd > -1)
				{
/*					if (!sc->throttle && !sc->needclose && !sc->closing) */
					if (!sc->needclose && !sc->closing)
					{
						if (sc->fd > max)
							max = sc->fd;
						FD_SET(sc->fd, &readfds);
					}
				}
				if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
				{
//                log (LOG_DEBUG, "Debug msg before sc = sc->next@network.c\n");
				}
				sc = sc->next;
			}
			if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
			{
//                log (LOG_DEBUG, "Debug msg before st = st->next@network.c\n");
			}
			st = st->next;
		}

		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before FD_SET(server_socket...)@network.c\n");
		}
		FD_SET(server_socket, &readfds);
		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before max = server_socket@network.c\n");
		}
		if (server_socket > max)
			max = server_socket;
		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before FD_SET(control_fd...)@network.c\n");
		}
		FD_SET(control_fd, &readfds);
		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before max = control_fd@network.c\n");
		}
		if (control_fd > max)
			max = control_fd;
		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before tv.tv_sec@network.c\n");
		}
		tv.tv_sec = 1;
		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before tv.tv_usec@network.c\n");
		}
		tv.tv_usec = 0;
		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before schedule_unlock@network.c\n");
		}
//  if (strlen(lns_server_name) == 0){
		schedule_unlock();
		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before select(max + 1 ...)@network.c\n");
			debug_num = select(max + 1, &readfds, NULL, NULL, NULL);
//      log (LOG_DEBUG, "Debug msg after select, the ret = %d @network.c\n", debug_num);
		} else
			select(max + 1, &readfds, NULL, NULL, NULL);

		if (strlen(lns_server_name) != 0)	// Cheni & SJ_Yen
		{
//                log (LOG_DEBUG, "Debug msg before schedule_unlock2..@network.c\n");
		}
		schedule_lock();
//  }

		if ((FD_ISSET(control_fd, &readfds)) || (strlen(lns_server_name) != 0))
		{
//            log (LOG_DEBUG, "Debug MSG::: The lns_server_name is '%s' at network.c before do_control!!\n", lns_server_name);
//      log (LOG_DEBUG, "Debug MSG::: The length of lns_server_name is '%d' at network.c before do_control!!\n", strlen(lns_server_name));
			do_control();
		}
		if (FD_ISSET(server_socket, &readfds))
		{
			/*
			 * Okay, now we're ready for reading and processing new data.
			 */
			recycle_buf(buf);
			/* Reserve space for expanding payload packet headers */
			buf->start += PAYLOAD_BUF;
			buf->len -= PAYLOAD_BUF;
			fromlen = sizeof(from);
			recvsize = recvfrom(server_socket, buf->start, buf->len, 0, (struct sockaddr *) &from, &fromlen);
			if (recvsize < MIN_PAYLOAD_HDR_LEN)
			{
				if (recvsize < 0)
				{
					if (errno != EAGAIN)
						log(LOG_WARN, "%s: recvfrom returned error %d (%s)\n", __FUNCTION__, errno, strerror(errno));
				} else
				{
					log(LOG_WARN, "%s: received too small a packet\n", __FUNCTION__);
				}
			} else
			{
				buf->len = recvsize;
				if (packet_dump)
				{
					do_packet_dump(buf);
				}
				fix_hdr(buf->start);
				extract(buf->start, &tunnel, &call);
//				dbg("%s() extract tunnel %d call %d\n", __FUNCTION__, tunnel, call); //eric++
				if (debug_network)
				{
					log(LOG_DEBUG, "%s: recv packet from %s, size = %d,
tunnel = %d, call = %d\n", __FUNCTION__, inet_ntoa(from.sin_addr), recvsize, tunnel, call);
				}
				if (packet_dump)
				{
					do_packet_dump(buf);
				}
				if (!(c = get_call(tunnel, call, from.sin_addr.s_addr, from.sin_port)))
				{
					if ((c = get_tunnel(tunnel, from.sin_addr.s_addr, from.sin_port)))
					{
						/*
						 * It is theoretically possible that we could be sent
						 * a control message (say a StopCCN) on a call that we
						 * have already closed or some such nonsense.  To prevent
						 * this from closing the tunnel, if we get a call on a valid
						 * tunnel, but not with a valid CID, we'll just send a ZLB
						 * to ack receiving the packet.
						 */
						if (debug_tunnel)
							log(LOG_DEBUG, "%s: no such call %d on tunnel %d.  Sending special ZLB\n", __FUNCTION__);
						handle_special(buf, c, call);
					} else
						log(LOG_DEBUG, "%s: unable to find call or tunnel to handle packet.  call = %d, tunnel = %d Dumping.\n", __FUNCTION__, call, tunnel);

				} else
				{
					buf->peer = from;
					/* Handle the packet */
					c->container->chal_us.vector = NULL;
					if (handle_packet(buf, c->container, c))
					{
						if (debug_tunnel)
							log(LOG_DEBUG, "%s: bad packet\n", __FUNCTION__);
					};
					if (c->cnu)
					{
						/* Send Zero Byte Packet */
						control_zlb(buf, c->container, c);
						c->cnu = 0;
					}
				}
			}
//			dbg ("if (recvsize >= MIN_PAYLOAD_HDR_LEN) end\n"); //eric++
		};

		st = tunnels.head;
//		dbg ("while (st) %p\n", st); //eric++
		while (st)
		{
//      printf("start st\n");
			sc = st->call_head;
			while (sc)
			{
//      printf("start sc\n");
				if ((sc->fd >= 0) && FD_ISSET(sc->fd, &readfds))
				{
					/* Got some payload to send */
					//printf("Got some payload to send\n");
					int result;
					//  printf("Got some payload to send\n");
					recycle_payload(buf, sc->container->peer);
#ifdef DEBUG_FLOW
					log(LOG_DEBUG, "%s: rws = %d, pSs = %d, pLr = %d\n", __FUNCTION__, sc->rws, sc->pSs, sc->pLr);
#endif
/*					if ((sc->rws>0) && (sc->pSs > sc->pLr + sc->rws) && !sc->rbit) {
#ifdef DEBUG_FLOW
						log(LOG_DEBUG, "%s: throttling payload (call = %d, tunnel = %d, Lr = %d, Ss = %d, rws = %d)!\n",__FUNCTION__,
								 sc->cid, sc->container->tid, sc->pLr, sc->pSs, sc->rws); 
#endif
						sc->throttle = -1;
						We unthrottle in handle_packet if we get a payload packet, 
						valid or ZLB, but we also schedule a dethrottle in which
						case the R-bit will be set
						FIXME: Rate Adaptive timeout? 						
						tv.tv_sec = 2;
						tv.tv_usec = 0;
						sc->dethrottle = schedule(tv, dethrottle, sc); 					
					} else */
/*					while ((result=read_packet(buf,sc->fd,sc->frame & SYNC_FRAMING))>0) */
					//  sendto (server_socket, sendtomsg1, lenofmsg1, 0,
					//      (struct sockaddr *) &buf->peer, sizeof (buf->peer));

					while ((result = read_packet(buf, sc->fd, SYNC_FRAMING)) > 0)
					{
//          do_packet_dump(buf);
						//  printf("read packet (buf, sc->fd)...\n");
						add_payload_hdr(sc->container, sc, buf);
						if (packet_dump)
						{
							do_packet_dump(buf);
						}


						sc->prx = sc->data_rec_seq_num;
						if (sc->zlb_xmit)
						{
							//      printf("sc->zlb_xmit\n");
							deschedule(sc->zlb_xmit);
							sc->zlb_xmit = NULL;
						}
						//  printf("start sc->tx_bytes += buf->len\n");
						sc->tx_bytes += buf->len;
						//  printf("start sc->tx_pkts++\n");
						sc->tx_pkts++;
						//  printf("start udp_xmit\n");
						//  sendto (server_socket, sendtomsg2, lenofmsg2, 0,
						//  (struct sockaddr *) &buf->peer, sizeof (buf->peer));
						udp_xmit(buf);
						//  sendto (server_socket, sendtomsg3, lenofmsg3, 0,
						//        (struct sockaddr *) &buf->peer, sizeof (buf->peer));
						//  printf("start recycle_payload\n");
						recycle_payload(buf, sc->container->peer);
					}
					if (result != 0)
					{
						log(LOG_WARN, "%s: tossing read packet, error = %s (%d).  Closing call.\n", __FUNCTION__, strerror(-result), -result);
						strcpy(sc->errormsg, strerror(-result));
						sc->needclose = -1;
					}
				}
//      printf("end before sc\n");
				sc = sc->next;
			}
			st = st->next;
		}
//		dbg ("while (st) %p end\n", st); //eric++
	}

}
