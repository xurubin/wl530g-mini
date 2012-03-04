

#
# Makefile for the linux squashfs routines.
#

## EXTRA_CFLAGS += -DSQUASHFS_TRACE

obj-y += block.o cache.o dir.o file.o fragment.o id.o squashfs_inode.o export.o
obj-y += namei.o super.o symlink.o zlib_wrapper.o decompressor.o
obj-$(CONFIG_SQUASHFS_XATTR) += xattr.o xattr_id.o
obj-$(CONFIG_SQUASHFS_LZO) += lzo_wrapper.o
obj-$(CONFIG_SQUASHFS_XZ) += xz_wrapper.o

O_TARGET := squashfs.o
obj-m := $(O_TARGET)

include $(TOPDIR)/Rules.make