#
# Makefile for kernel loadable module for Marvell Libertas Wireless Driver.
#

BUILD_SUBDIRS = mvwlan
ROMFS_SUBDIRS = mvwlan

ifdef CONFIG_USER_MRVL_AP_DAEMON
BUILD_SUBDIRS+= mvapd
ROMFS_SUBDIRS+= mvapd
endif

all:
	for x in $(BUILD_SUBDIRS); do (cd $$x && $(MAKE) ); done

clean:
	for x in $(BUILD_SUBDIRS); do (cd $$x && $(MAKE) clean ); done

distclean:	clean

convert:
	for x in $(BUILD_SUBDIRS); do (cd $$x && $(MAKE) convert ); done
	
romfs:
	for x in $(ROMFS_SUBDIRS); do (cd $$x && $(MAKE) romfs ); done

