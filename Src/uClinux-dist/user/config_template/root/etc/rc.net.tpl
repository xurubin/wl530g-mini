# Bridge LAN & WLAN
/bin/ifconfig LAN up
/bin/ifconfig wlan0 up
/bin/brctl addbr br0
/bin/brctl addif br0 LAN
/bin/brctl addif br0 wlan0
/bin/brctl setfd br0 1
/bin/ifconfig br0 <?subnet_ip_prefix?>.<?router_ipaddr?> up

/bin/ifconfig lo 127.0.0.1
/bin/route add 127.0.0.0 lo

/bin/ifconfig WAN up

# DNS settings
/bin/dnsmasq -d -C /etc/config/dnsmasq.conf &

# FW + NAT settings
/bin/iptables -t nat -F
/bin/iptables -t nat -A POSTROUTING -o ppp0 -j MASQUERADE

# Disable WAN pinging
/bin/iptables -A INPUT -i ppp0 -p icmp --icmp-type 8 -j DROP

# HTTP Server redirection
#/bin/iptables -t nat -A PREROUTING -p tcp --dport 81 -j DNAT --to 192.168.222.3:80
#/bin/iptables -t nat -A POSTROUTING -d 192.168.222.3 -p tcp --dport 80 -j SNAT --to 192.168.222.1 

# STM32 Ports
#/bin/iptables -t nat -A PREROUTING -p udp -m udp --dport 6000 -j DNAT --to 192.168.222.6
#/bin/iptables -t nat -A POSTROUTING -d 192.168.222.6 -p udp --dport 6000 -j SNAT --to 192.168.222.1 
#/bin/iptables -t nat -A PREROUTING -p udp -m udp --dport 6001 -j DNAT --to 192.168.222.6
#/bin/iptables -t nat -A POSTROUTING -d 192.168.222.6 -p udp --dport 6001 -j SNAT --to 192.168.222.1 
#/bin/iptables -t nat -A PREROUTING -p udp -m udp --dport 6002 -j DNAT --to 192.168.222.6
#/bin/iptables -t nat -A POSTROUTING -d 192.168.222.6 -p udp --dport 6002 -j SNAT --to 192.168.222.1 
#/bin/iptables -t nat -A PREROUTING -p udp -m udp --dport 6003 -j DNAT --to 192.168.222.6
#/bin/iptables -t nat -A POSTROUTING -d 192.168.222.6 -p udp --dport 6003 -j SNAT --to 192.168.222.1 

## UPNP Server
# adding the MINIUPNPD chain for nat
iptables -t nat -N MINIUPNPD
#adding the rule to MINIUPNPD
iptables -t nat -A PREROUTING -i ppp0 -j MINIUPNPD
#adding the MINIUPNPD chain for filter
iptables -t filter -N MINIUPNPD
#adding the rule to MINIUPNPD
iptables -t filter -A FORWARD -i WAN ! -o EAN -j MINIUPNPD

miniupnpd -f /etc/config/miniupnpd.conf -d &

pppd plugin pppoe nic-WAN mru 1458 usepeerdns defaultroute persist maxfail 0 &
# HTTP server
/bin/httpd br0&

echo 1 > /proc/sys/net/ipv4/ip_dynaddr
echo 1 > /proc/sys/net/ipv4/ip_forward
