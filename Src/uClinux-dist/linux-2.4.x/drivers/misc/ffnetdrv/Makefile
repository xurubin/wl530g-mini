#
# Makefile for the qd driver loadable module
#
O_TARGET := unimac.o
obj-y:=	qd_drv/Dev/qdInit.o \
	qd_drv/Dev/qdModule.o \
	qd_drv/Dev/qdIoctl.o \
	qd_drv/src/driver/gtDrvConfig.o \
	qd_drv/src/driver/gtDrvEvents.o \
	qd_drv/src/driver/gtHwCntl.o \
	qd_drv/src/platform/gtDebug.o \
	qd_drv/src/platform/gtMiiSmiIf.o \
	qd_drv/src/platform/gtSem.o \
	qd_drv/src/msapi/gtSysConfig.o \
	qd_drv/src/msapi/gtBrgFdb.o \
	qd_drv/src/msapi/gtBrgStp.o \
	qd_drv/src/msapi/gtBrgVlan.o \
	qd_drv/src/msapi/gtBrgVtu.o \
	qd_drv/src/msapi/gtEvents.o \
	qd_drv/src/msapi/gtPhyCtrl.o \
	qd_drv/src/msapi/gtPhyInt.o \
	qd_drv/src/msapi/gtPortCtrl.o \
	qd_drv/src/msapi/gtPortPav.o \
	qd_drv/src/msapi/gtPortRateCtrl.o \
	qd_drv/src/msapi/gtPortRmon.o \
	qd_drv/src/msapi/gtPortStat.o \
	qd_drv/src/msapi/gtPortStatus.o \
	qd_drv/src/msapi/gtQosMap.o \
	qd_drv/src/msapi/gtSysCtrl.o \
	qd_drv/src/msapi/gtSysStatus.o \
	qd_drv/src/msapi/gtUtils.o \
	qd_drv/src/msapi/gtVct.o \
	qd_drv/src/msapi/gtVersion.o \
	unimac/mvFF_eth.o \
	unimac/mvFF_addrtbl.o \
	unimac/mv_unimac_mgr.o \
	unimac/mvFF_netconf.o \
	qd_drv/Dev/qdFFmii.o


EXTRA_CFLAGS := -I./qd_drv/include -I./unimac -I./include
EXTRA_CFLAGS += -DCONFIG_ARCH_LIBERTAS -D__KERNEL__ -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -DLINUX
#CFLAGS += -DDEBUG_QD -DMV_DEBUG
EXTRA_CFLAGS += -DNDEBUG


include $(TOPDIR)/Rules.make

