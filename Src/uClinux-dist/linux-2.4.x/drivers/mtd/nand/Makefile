#
# linux/drivers/nand/Makefile
#
# $Id: Makefile.common,v 1.4 2003/11/04 22:59:11 ahennessy Exp $

ifeq ($(PATCHLEVEL),4)
O_TARGET			:= nandlink.o
export-objs			:= nand.o nand_ecc.o nand_ids.o
endif

obj-$(CONFIG_MTD_NAND)		+= nand.o nand_ecc.o
obj-$(CONFIG_MTD_NAND_SPIA)	+= spia.o
obj-$(CONFIG_MTD_NAND_TOTO)	+= toto.o
obj-$(CONFIG_MTD_NAND_AUTCPU12)	+= autcpu12.o
obj-$(CONFIG_MTD_NAND_EDB7312)  += edb7312.o
obj-$(CONFIG_MTD_NAND_TX4925NDFMC)  += tx4925ndfmc.o
obj-$(CONFIG_MTD_NAND_TX4938NDFMC)  += tx4938ndfmc.o
obj-$(CONFIG_MTD_NAND_IDS)	+= nand_ids.o

-include $(TOPDIR)/Rules.make
