/* NETMAP - static NAT mapping of IP network addresses (1:1).
   The mapping can be applied to source (POSTROUTING),
   destination (PREROUTING), or both (with separate rules).

   Author: Svenning Soerensen <svenning@post5.tele.dk>
*/

#include <linux/config.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <net/udp.h>
#include <asm/uaccess.h>
#include <asm/checksum.h>


#define MODULENAME "NETMAP"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Svenning Soerensen <svenning@post5.tele.dk>");
MODULE_DESCRIPTION("iptables 1:1 NAT mapping of IP networks target");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

static int
check(const char *tablename,
      const struct ipt_entry *e,
      void *targinfo,
      unsigned int targinfosize,
      unsigned int hook_mask)
{
	const struct ip_nat_multi_range *mr = targinfo;

	if (strcmp(tablename, "nat") != 0) {
		DEBUGP(MODULENAME":check: bad table `%s'.\n", tablename);
		return 0;
	}
	if (targinfosize != IPT_ALIGN(sizeof(*mr))) {
		DEBUGP(MODULENAME":check: size %u.\n", targinfosize);
		return 0;
	}
	if (hook_mask & ~((1 << NF_IP_PRE_ROUTING) | (1 << NF_IP_POST_ROUTING))) {
		DEBUGP(MODULENAME":check: bad hooks %x.\n", hook_mask);
		return 0;
	}
	if (!(mr->range[0].flags & IP_NAT_RANGE_MAP_IPS)) {
		DEBUGP(MODULENAME":check: bad MAP_IPS.\n");
		return 0;
	}
	if (mr->rangesize != 1) {
		DEBUGP(MODULENAME":check: bad rangesize %u.\n", mr->rangesize);
		return 0;
	}
	return 1;
}

static unsigned int
target(struct sk_buff **pskb,
       unsigned int hooknum,
       const struct net_device *in,
       const struct net_device *out,
       const void *targinfo,
       void *userinfo)
{
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;
	u_int32_t new_ip, netmask;
	u_int32_t new_port, base_port;
	const struct ip_nat_multi_range *mr = targinfo;
	struct ip_nat_multi_range newrange;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *udph = (struct udphdr *)((u_int32_t *)iph + iph->ihl);

	IP_NF_ASSERT(hooknum == NF_IP_PRE_ROUTING
		     || hooknum == NF_IP_POST_ROUTING);

	ct = ip_conntrack_get(*pskb, &ctinfo);

	//netmask = ~(mr->range[0].min_ip ^ mr->range[0].max_ip);
	netmask = 0x00ffffff;

	//printk("netmask : %x %x %x %x %x %x %x\n", netmask, (unsigned int)udph->dest, (unsigned int)udph->source, (unsigned int)mr->range[0].min_ip, (unsigned int)mr->range[0].max_ip, (unsigned int)mr->range[0].min.udp.port, (unsigned int)mr->range[0].max.udp.port);

	if (hooknum == NF_IP_PRE_ROUTING)
	{	
		base_port = ntohs(udph->source);
		new_ip = (mr->range[0].min_ip&netmask)+ ((ntohs(udph->dest) - 10000)<<24);
		new_port = base_port;
		mr->range[0].min.udp.port = htons(new_port);
		mr->range[0].max.udp.port = htons(new_port);
		//printk("pre1: %x %x\n", new_ip, new_port);
	}
	else
	{
		base_port = ntohs(udph->dest);
		new_ip = mr->range[0].min_ip;
		new_port = 10000 + ((iph->saddr&~netmask)>>24);
		//printk("pos1: %x %x\n", new_ip, new_port);
		mr->range[0].min.udp.port = htons(new_port);
		mr->range[0].max.udp.port = htons(new_port);
	}

	newrange = ((struct ip_nat_multi_range)
	{ 1, { { mr->range[0].flags | IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED,
		 new_ip, new_ip,
		 mr->range[0].min, mr->range[0].max } } });

	/* Hand modified range to generic setup. */
	return ip_nat_setup_info(ct, &newrange, hooknum);
}

static struct ipt_target target_module = { 
	.name = MODULENAME,
	.target = target,
	.checkentry = check,
	.me = THIS_MODULE
};

static int __init init(void)
{
	return ipt_register_target(&target_module);
}

static void __exit fini(void)
{
	ipt_unregister_target(&target_module);
}

module_init(init);
module_exit(fini);
