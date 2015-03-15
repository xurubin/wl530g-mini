 A fork of ASUS's uClinux-based GPL firmware 1.9.4.9 for WL-530g, targeting at similar devices (Marvell 88W8510-BAN + 88E6060, Libertas AP32/AP22) but aiming at much smaller footprint: 1MB Nor flash and 8MB RAM.

## Functionalities include:
* SquashFS 4.0 (with LZMA2) backported to 2.4 kernel, supporting both kernel zImage and root filesystem.
* Patched binary wireless driver to support 8M RAM.
* Updated dnsmasq as DHCP and DNS server.
* Statically linked binary-blob kernel module. See how it's done.
* And more to come.
