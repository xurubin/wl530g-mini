O_TARGET := xz.o

export-objs := xz_dec_syms.o

obj-y := xz_dec_syms.o xz_dec_stream.o xz_dec_lzma2.o
obj-$(CONFIG_XZ_DEC_BCJ) += xz_dec_bcj.o
obj-$(CONFIG_XZ_DEC_TEST) += xz_dec_test.o
obj-m := $(O_TARGET)

EXTRA_CFLAGS += -I $(TOPDIR)/lib/xz

include $(TOPDIR)/Rules.make

