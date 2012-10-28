user=root
group=root
interface=br0

# pppd with 'usepeerdns' will update /tmp/ppp/resolv.conf, which dnsmasq should monitor
resolv-file=/tmp/ppp/resolv.conf

dhcp-leasefile=/tmp/dnsmasq.leases

# dhcp settings
dhcp-range=<?subnet_ip_prefix?>.100,<?subnet_ip_prefix?>.254
# static dhcp clients
dhcp-host=<?dhcp0_mac?>, <?subnet_ip_prefix?>.<?dhcp0_ip?>, <?dhcp0_name?>, infinite
dhcp-host=<?dhcp1_mac?>, <?subnet_ip_prefix?>.<?dhcp1_ip?>, <?dhcp1_name?>, infinite
#dhcp-host=00:1b:fc:03:43:a9, 192.168.222.4, a8js, infinite
