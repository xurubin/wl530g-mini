############################################################################

#
# Makefile -- Top level uClinux makefile.
#
# Copyright (c) 2001-2002, SnapGear (www.snapgear.com)
# Copyright (c) 2001, Lineo
#

VERSIONPKG = 1.3.4
VERSIONSTR = $(CONFIG_VENDOR)/$(CONFIG_PRODUCT) Version $(VERSIONPKG)

############################################################################
#
# Lets work out what the user wants, and if they have configured us yet
#

ifeq (.config,$(wildcard .config))
include .config

all: subdirs romfs modules modules_install image
else
all: config_error
endif

############################################################################
#
# Get the core stuff worked out
#

LINUXDIR = $(CONFIG_LINUXDIR)
LIBCDIR  = $(CONFIG_LIBCDIR)
ROOTDIR  = $(shell pwd)
PATH	 := $(PATH):$(ROOTDIR)/tools
HOSTCC   = unset GCC_EXEC_PREFIX; cc
IMAGEDIR = $(ROOTDIR)/images
RELDIR = $(ROOTDIR)/release
ROMFSDIR = $(ROOTDIR)/romfs
ROMFSINST= romfs-inst.sh
SCRIPTSDIR = $(ROOTDIR)/config/scripts
TFTPDIR    = /tftpboot


LINUX_CONFIG  = $(ROOTDIR)/$(LINUXDIR)/.config
CONFIG_CONFIG = $(ROOTDIR)/config/.config
MODULES_CONFIG = $(ROOTDIR)/modules/.config


CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

ifeq (config.arch,$(wildcard config.arch))
include config.arch
ARCH_CONFIG = $(ROOTDIR)/config.arch
export ARCH_CONFIG
endif

ifneq ($(SUBARCH),)
# Using UML, so make the kernel and non-kernel with different ARCHs
MAKEARCH = $(MAKE) ARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKE) ARCH=$(ARCH) SUBARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
else
MAKEARCH = $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKEARCH)
endif

DIRS    = $(VENDOR_TOPDIRS) lib user
VENDDIR = $(ROOTDIR)/vendors/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.

export VENDOR PRODUCT ROOTDIR LINUXDIR HOSTCC CONFIG_SHELL
export CONFIG_CONFIG LINUX_CONFIG ROMFSDIR SCRIPTSDIR
export VERSIONPKG VERSIONSTR ROMFSINST PATH IMAGEDIR RELDIR RELFILES TFTPDIR

############################################################################

#
# Config stuff,  we recall ourselves to load the new config.arch before
# running the kernel and other config scripts
#

WEBS=web_WL530g_EN web_SnapAP_EN
KVER=1.9
FVER=4.9

.PHONY: config.tk config.in

WL530g:
	rm -rf target/www
	cp -rf ../user.asus/rc/rc target/sbin/.
	cp -rf ../user.asus/httpd/httpd target/sbin/.
	cp -rf ./wlan.v.2.1/mvwlan.o target/lib/modules/.
	cp -rf ./wlan.v.2.1/mvapd target/bin/.
	cp -rf ./wlan.v.2.1/mvmfgd target/bin/.
	cp -rf ./wlan.v.2.1/mvwlcfg target/bin/.
	cp -rf ../user.asus/www/asus/web_WL530g target/www
	./mkcramfs target target.cramfs
	./addver target.cramfs $(KVER) $(FVER) WL530g 0 0 0 0 0 0 0 0 zImage
	trx -o WL530g_$(KVER).$(FVER)_$(LANGUAGE).bin -b 32 zImage -b 655360 target.cramfs

WL520g:
	rm -rf target/www
	cp -rf ../user.asus/rc/rc target/sbin/.
	cp -rf ../user.asus/www/asus/web_WL520g target/www	
	./mkcramfs target target.cramfs
	./addver target.cramfs $(KVER) $(FVER) WL520g 0 0 0 0 0 0 0 0 zImage
	trx -o WL520g_$(KVER).$(FVER)_$(LANGUAGE).bin -b 32 zImage -b 655360 target.cramfs

WBR14G:
	rm -rf target.smc/www
	cp -rf ../user.asus/www/SMC target.smc/www	
	./mkcramfs target.smc target.cramfs
	./addver target.cramfs $(KVER) $(FVER) WBR14G 0 0 0 0 0 0 0 0 zImage
	trx -o WBR14G_$(KVER).$(FVER).bin -b 32 zImage -b 655360 target.cramfs

WX54G:
	rm -rf target/www
	cp -rf ../user.asus/rc.wx54g/rc target/sbin/.
	cp -rf ../user.asus/httpd.wx54g/httpd target/sbin/.
	cp -rf ./wlan.v.2.1/mvwlan.o target/lib/modules/.
	cp -rf ./wlan.v.2.1/mvapd target/bin/.
	cp -rf ./wlan.v.2.1/mvmfgd target/bin/.
	cp -rf ./wlan.v.2.1/mvwlcfg target/bin/.
	cp -rf ../user.asus/www/terayon/web_WX54G target/www
	./mkcramfs target target.cramfs
# original version number
#	./addver target.cramfs $(KVER) $(FVER) WX54G 0 0 0 0 0 0 0 0 zImage
# use month and day as version number
#	./addver target.cramfs $(shell date +%m%d|sed -e 's,\([0-9]\)\([0-9]\)\([0-9]\)\([0-9]\),\1.\2 \3.\4,') WX54G 0 0 0 0 0 0 0 0 zImage
# use hour and minute as version number
	./addver target.cramfs $(shell date +%k%M|sed -e 's,\([0-9]\)\([0-9]\)\([0-9]\)\([0-9]\),\1.\2 \3.\4,') WX54G 0 0 0 0 0 0 0 0 zImage
	trx -o WX54G_$(KVER).$(FVER).bin -b 32 zImage -b 655360 target.cramfs
	ls -l WX54G_$(KVER).$(FVER).bin target/sbin/openl2tpd

image-%:
	cd ../user.asus/www/asus && ./mkweb $* $(LANGUAGE) $* && cd ../../../filesystem
	make $*

#For WL520g/WL530g Only
images-%:
	make image-$* LANGUAGE="EN"
	make image-$* LANGUAGE="KR"
	make image-$* LANGUAGE="TW"
	make image-$* LANGUAGE="CN"
	make image-$* LANGUAGE="JP"

config.in:
	config/mkconfig > config.in

config.tk: config.in
	$(MAKE) -C $(SCRIPTSDIR) tkparse
	ARCH=dummy $(SCRIPTSDIR)/tkparse < config.in > config.tmp
	@if [ -f /usr/local/bin/wish ];	then \
		echo '#!'"/usr/local/bin/wish -f" > config.tk; \
	else \
		echo '#!'"/usr/bin/wish -f" > config.tk; \
	fi
	cat $(SCRIPTSDIR)/header.tk >> ./config.tk
	cat config.tmp >> config.tk
	rm -f config.tmp
	echo "set defaults \"/dev/null\"" >> config.tk
	echo "set help_file \"config/Configure.help\"" >> config.tk
	cat $(SCRIPTSDIR)/tail.tk >> config.tk
	chmod 755 config.tk

.PHONY: xconfig
xconfig: config.tk
	@wish -f config.tk
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	 fi
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_xconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_xconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_xconfig; \
	 fi
	@config/setconfig final

.PHONY: config
config: config.in
	@HELP_FILE=config/Configure.help \
		$(CONFIG_SHELL) $(SCRIPTSDIR)/Configure config.in
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_config; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_config; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_config; \
	 fi
	@config/setconfig final

.PHONY: menuconfig
menuconfig: config.in
	$(MAKE) -C $(SCRIPTSDIR)/lxdialog all
	@HELP_FILE=config/Configure.help \
		$(CONFIG_SHELL) $(SCRIPTSDIR)/Menuconfig config.in
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	 fi
	@config/setconfig defaults
	@if egrep "^CONFIG_DEFAULTS_KERNEL=y" .config > /dev/null; then \
		$(MAKE) linux_menuconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_MODULES=y" .config > /dev/null; then \
		$(MAKE) modules_menuconfig; \
	 fi
	@if egrep "^CONFIG_DEFAULTS_VENDOR=y" .config > /dev/null; then \
		$(MAKE) config_menuconfig; \
	 fi
	@config/setconfig final

.PHONY: oldconfig
oldconfig:
	@$(MAKE) oldconfig_linux
	@$(MAKE) oldconfig_modules
	@$(MAKE) oldconfig_config
	@$(MAKE) oldconfig_uClibc
	@config/setconfig final

.PHONY: modules
modules:
	. $(LINUXDIR)/.config; if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(LINUXDIR)/modules ] || mkdir $(LINUXDIR)/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) modules; \
	fi

.PHONY: modules_install
modules_install:
	. $(LINUXDIR)/.config; if [ "$$CONFIG_MODULES" = "y" ]; then \
		[ -d $(ROMFSDIR)/lib/modules ] || mkdir -p $(ROMFSDIR)/lib/modules; \
		$(MAKEARCH_KERNEL) -C $(LINUXDIR) INSTALL_MOD_PATH=$(ROMFSDIR) DEPMOD=true modules_install; \
		rm -f $(ROMFSDIR)/lib/modules/*/build; \
		find $(ROMFSDIR)/lib/modules -type f | xargs -r $(STRIP) -g; \
	fi

linux_xconfig:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) xconfig
linux_menuconfig:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) menuconfig
linux_config:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) config
modules_xconfig:
	[ ! -d modules ] || $(MAKEARCH) -C modules xconfig
modules_menuconfig:
	[ ! -d modules ] || $(MAKEARCH) -C modules menuconfig
modules_config:
	[ ! -d modules ] || $(MAKEARCH) -C modules config
modules_clean:
	-[ ! -d modules ] || $(MAKEARCH) -C modules clean
config_xconfig:
	$(MAKEARCH) -C config xconfig
config_menuconfig:
	$(MAKEARCH) -C config menuconfig
config_config:
	$(MAKEARCH) -C config config
oldconfig_config:
	$(MAKEARCH) -C config oldconfig
oldconfig_modules:
	[ ! -d modules ] || $(MAKEARCH) -C modules oldconfig
oldconfig_linux:
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) oldconfig
oldconfig_uClibc:
	[ -z "$(findstring uClibc,$(LIBCDIR))" ] || $(MAKEARCH) -C $(LIBCDIR) oldconfig

############################################################################
#
# normal make targets
#

.PHONY: romfs
romfs:
	for dir in $(DIRS) ; do $(MAKEARCH) -C $$dir romfs || exit 1 ; done
	-find $(ROMFSDIR)/. -name CVS | xargs -r rm -rf

.PHONY: image
image:
	[ -d $(IMAGEDIR) ] || mkdir $(IMAGEDIR)
	$(MAKEARCH) -C $(VENDDIR) image

.PHONY: netflash
netflash:
	make -C prop/mstools CONFIG_PROP_MSTOOLS_NETFLASH_NETFLASH=y

.PHONY: release
release:
	make dep all
	[ -d $(RELDIR) ] || mkdir $(RELDIR)
	for i in $(RELFILES); do \
		a=`expr "$$i" : "\(.*\),.*"` ; \
		b=`expr "$$i" : "[^,]*,\(.*\)"` ; \
		[ "$$a" ] || a=$$i; \
		[ "$$b" ] || b=_`basename $$i`; \
		cp $$a $(RELDIR)/$(CONFIG_PRODUCT)_v$(VERSIONPKG)_`date +%Y%m%d`$$b; \
	done

%_fullrelease:
	make clean
	@rm -f .oldconfig
	@if [ -e vendors/$(@:_fullrelease=)/config.device ] ; then \
		cp vendors/$(@:_fullrelease=)/config.device .config ; \
	else \
		echo ***;echo;echo ; \
		echo $(@:_fullrelease=) must exist to use this target ; \
		exit 1;\
	fi
	new_values=`yes n | config/setconfig defaults | grep '(NEW)'` ; \
	if [ "$$new_values" ] ; then \
		echo $$new_values ; \
		echo Incomplete config, exiting ; \
		exit 1 ; \
	fi
	@config/setconfig final
	make netflash
	make release
	
#
# fancy target that allows a vendor to have other top level
# make targets,  for example "make vendor_flash" will run the
# vendor_flash target in the vendors directory
#

vendor_%:
	$(MAKEARCH) -C $(VENDDIR) $@

.PHONY: linux
linux linux%_only:
	@if [ $(LINUXDIR) != linux-2.5.x -a $(LINUXDIR) != linux-2.6.x -a ! -f $(LINUXDIR)/.depend ] ; then \
		echo "ERROR: you need to do a 'make dep' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) $(LINUXTARGET) || exit 1
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) zImage || exit 1
	if [ -f $(LINUXDIR)/vmlinux ]; then \
		ln -f $(LINUXDIR)/vmlinux $(LINUXDIR)/linux ; \
	fi

.PHONY: subdirs
subdirs: linux
	for dir in $(DIRS) ; do $(MAKEARCH_KERNEL) -C $$dir || exit 1 ; done

dep:
	@if [ ! -f $(LINUXDIR)/.config ] ; then \
		echo "ERROR: you need to do a 'make config' first" ; \
		exit 1 ; \
	fi
	$(MAKEARCH_KERNEL) -C $(LINUXDIR) dep

# This one removes all executables from the tree and forces their relinking
.PHONY: relink
relink:
	find user -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f
	find prop -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f
	find $(VENDDIR) -name '*.gdb' | sed 's/^\(.*\)\.gdb/\1 \1.gdb/' | xargs rm -f

clean: modules_clean
	for dir in $(LINUXDIR) $(DIRS); do $(MAKEARCH) -C $$dir clean ; done
	rm -rf $(ROMFSDIR)/*
	rm -f $(IMAGEDIR)/*
	rm -f config.tk
	rm -f $(LINUXDIR)/linux
	rm -rf $(LINUXDIR)/net/ipsec/alg/libaes $(LINUXDIR)/net/ipsec/alg/perlasm

real_clean mrproper: clean
	-$(MAKEARCH_KERNEL) -C $(LINUXDIR) mrproper
	-$(MAKEARCH) -C config clean
	-$(MAKEARCH) -C uClibc distclean
	rm -rf romfs config.in config.arch config.tk images
	rm -f modules/config.tk
	rm -rf .config .config.old .oldconfig autoconf.h
	rm -f $(RELDIR)

distclean: mrproper
	-$(MAKEARCH_KERNEL) -C $(LINUXDIR) distclean
	-rm -f user/tinylogin/applet_source_list user/tinylogin/config.h

%_only:
	[ ! -d "$(@:_only=)" ] || $(MAKEARCH) -C $(@:_only=)

%_clean:
	[ ! -d "$(@:_clean=)" ] || $(MAKEARCH) -C $(@:_clean=) clean

config_error:
	@echo "*************************************************"
	@echo "You have not run make config."
	@echo "The build sequence for this source tree is:"
	@echo "1. 'make config' or 'make xconfig'"
	@echo "2. 'make dep'"
	@echo "3. 'make'"
	@echo "*************************************************"
	@exit 1

prune:
	$(MAKE) -C user prune

dist-prep:
	-find $(ROOTDIR) -name 'Makefile*.bin' | while read t; do \
		$(MAKEARCH) -C `dirname $$t` -f `basename $$t` $@; \
	 done

############################################################################
