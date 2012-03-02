
# -*- sh -*-

# $Id: Makefile,v 1.34 2003/06/20 13:58:46 dwmw2 Exp $

#
# These flags are used for build native versions of the tools for the
# host system so it can generate images etc
#

CFLAGS += $(INCZ) -D__USE_BSD -I$(ROOTDIR)/lib/zlib 
LDFLAGS += -L$(ROOTDIR)/lib/zlib

TARGETS = ftl_format erase eraseall  nanddump doc_loadbios \
	mkfs.jffs ftl_check mkfs.jffs2 lock unlock \
	einfo mtd_debug fcp nandwrite jffs2dump \
	#nftldump nftl_format jffs2reader

SYMLINKS = crc32.h crc32.c compr_rtime.c

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS) $(SYMLINKS)

romfs:
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_ERASE        /bin/erase
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_ERASEALL     /bin/eraseall
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_FTL_CHECK    /bin/ftl_check
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_FTL_FORMAT   /bin/ftl_format
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_MKFSJFFS     /bin/mkfs.jffs
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_MKFSJFFS2    /bin/mkfs.jffs2
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NFTLDUMP     /bin/nftldump
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NFTL_FORMAT  /bin/nftl_format
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDDUMP     /bin/nanddump
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDTEST     /bin/nandtest
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDWRITE    /bin/nandwrite
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_DOC_LOADBIOS /bin/doc_loadbios
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_DOC_LOADIPL  /bin/doc_loadipl


mkfs.jffs2.o eraseall.o crc32.o: crc32.h

$(SYMLINKS):
	ln -sf $(ROOTDIR)/$(LINUXDIR)/fs/jffs2/$@ $@

mkfs.jffs2: crc32.o compr_rtime.o mkfs.jffs2.o compr_zlib.o
	$(CC) $(LDFLAGS) -o $@ $^ -lz

eraseall: crc32.o eraseall.o
	$(CC) $(LDFLAGS) -o $@ $^

jffs2reader: jffs2reader.o
	$(CC) $(LDFLAGS) -o $@ $^ -lz 

jffs2dump: jffs2dump.o crc32.o
	$(CC) $(LDFLAGS) -o $@ $^

