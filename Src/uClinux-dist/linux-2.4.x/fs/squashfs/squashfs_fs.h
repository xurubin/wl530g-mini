#ifndef SQUASHFS_FS
#define SQUASHFS_FS
/*
 * Squashfs
 *
 * Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008
 * Phillip Lougher <phillip@lougher.demon.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * squashfs_fs.h
 */
 
#define SQUASHFS_MAGIC          0x73717368

#define SQUASHFS_CACHED_FRAGMENTS	CONFIG_SQUASHFS_FRAGMENT_CACHE_SIZE
#define SQUASHFS_MAJOR			4
#define SQUASHFS_MINOR			0
#define SQUASHFS_START			0

/* size of metadata (inode and directory) blocks */
#define SQUASHFS_METADATA_SIZE		8192
#define SQUASHFS_METADATA_LOG		13

/* default size of data blocks */
#define SQUASHFS_FILE_SIZE		131072
#define SQUASHFS_FILE_LOG		17

#define SQUASHFS_FILE_MAX_SIZE		1048576
#define SQUASHFS_FILE_MAX_LOG		20

/* Max number of uids and gids */
#define SQUASHFS_IDS			65536

/* Max length of filename (not 255) */
#define SQUASHFS_NAME_LEN		256

#define SQUASHFS_INVALID_FRAG		(0xffffffffU)
#define SQUASHFS_INVALID_XATTR		(0xffffffffU)
#define SQUASHFS_INVALID_BLK		(-1LL)

/* Filesystem flags */
#define SQUASHFS_NOI			0
#define SQUASHFS_NOD			1
#define SQUASHFS_NOF			3
#define SQUASHFS_NO_FRAG		4
#define SQUASHFS_ALWAYS_FRAG		5
#define SQUASHFS_DUPLICATE		6
#define SQUASHFS_EXPORT			7
#define SQUASHFS_COMP_OPT		10

#define SQUASHFS_BIT(flag, bit)		((flag >> bit) & 1)

#define SQUASHFS_UNCOMPRESSED_INODES(flags)	SQUASHFS_BIT(flags, \
						SQUASHFS_NOI)

#define SQUASHFS_UNCOMPRESSED_DATA(flags)	SQUASHFS_BIT(flags, \
						SQUASHFS_NOD)

#define SQUASHFS_UNCOMPRESSED_FRAGMENTS(flags)	SQUASHFS_BIT(flags, \
						SQUASHFS_NOF)

#define SQUASHFS_NO_FRAGMENTS(flags)		SQUASHFS_BIT(flags, \
						SQUASHFS_NO_FRAG)

#define SQUASHFS_ALWAYS_FRAGMENTS(flags)	SQUASHFS_BIT(flags, \
						SQUASHFS_ALWAYS_FRAG)

#define SQUASHFS_DUPLICATES(flags)		SQUASHFS_BIT(flags, \
						SQUASHFS_DUPLICATE)

#define SQUASHFS_EXPORTABLE(flags)		SQUASHFS_BIT(flags, \
						SQUASHFS_EXPORT)

#define SQUASHFS_COMP_OPTS(flags)		SQUASHFS_BIT(flags, \
						SQUASHFS_COMP_OPT)

/* Max number of types and file types */
#define SQUASHFS_DIR_TYPE		1
#define SQUASHFS_REG_TYPE		2
#define SQUASHFS_SYMLINK_TYPE		3
#define SQUASHFS_BLKDEV_TYPE		4
#define SQUASHFS_CHRDEV_TYPE		5
#define SQUASHFS_FIFO_TYPE		6
#define SQUASHFS_SOCKET_TYPE		7
#define SQUASHFS_LDIR_TYPE		8
#define SQUASHFS_LREG_TYPE		9
#define SQUASHFS_LSYMLINK_TYPE		10
#define SQUASHFS_LBLKDEV_TYPE		11
#define SQUASHFS_LCHRDEV_TYPE		12
#define SQUASHFS_LFIFO_TYPE		13
#define SQUASHFS_LSOCKET_TYPE		14

/* Xattr types */
#define SQUASHFS_XATTR_USER             0
#define SQUASHFS_XATTR_TRUSTED          1
#define SQUASHFS_XATTR_SECURITY         2
#define SQUASHFS_XATTR_VALUE_OOL        256
#define SQUASHFS_XATTR_PREFIX_MASK      0xff

/* Flag whether block is compressed or uncompressed, bit is set if block is
 * uncompressed */
#define SQUASHFS_COMPRESSED_BIT		(1 << 15)

#define SQUASHFS_COMPRESSED_SIZE(B)	(((B) & ~SQUASHFS_COMPRESSED_BIT) ? \
		(B) & ~SQUASHFS_COMPRESSED_BIT :  SQUASHFS_COMPRESSED_BIT)

#define SQUASHFS_COMPRESSED(B)		(!((B) & SQUASHFS_COMPRESSED_BIT))

#define SQUASHFS_COMPRESSED_BIT_BLOCK	(1 << 24)

#define SQUASHFS_COMPRESSED_SIZE_BLOCK(B)	((B) & \
						~SQUASHFS_COMPRESSED_BIT_BLOCK)

#define SQUASHFS_COMPRESSED_BLOCK(B)	(!((B) & SQUASHFS_COMPRESSED_BIT_BLOCK))

/*
 * Inode number ops.  Inodes consist of a compressed block number, and an
 * uncompressed offset within that block
 */
#define SQUASHFS_INODE_BLK(A)		((unsigned int) ((A) >> 16))

#define SQUASHFS_INODE_OFFSET(A)	((unsigned int) ((A) & 0xffff))

#define SQUASHFS_MKINODE(A, B)		((long long)(((long long) (A)\
					<< 16) + (B)))

/* Translate between VFS mode and squashfs mode */
#define SQUASHFS_MODE(A)		((A) & 0xfff)

/* fragment and fragment table defines */
#define SQUASHFS_FRAGMENT_BYTES(A)	\
				((A) * sizeof(struct squashfs_fragment_entry))

#define SQUASHFS_FRAGMENT_INDEX(A)	(SQUASHFS_FRAGMENT_BYTES(A) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_FRAGMENT_INDEX_OFFSET(A)	(SQUASHFS_FRAGMENT_BYTES(A) % \
						SQUASHFS_METADATA_SIZE)

#define SQUASHFS_FRAGMENT_INDEXES(A)	((SQUASHFS_FRAGMENT_BYTES(A) + \
					SQUASHFS_METADATA_SIZE - 1) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_FRAGMENT_INDEX_BYTES(A)	(SQUASHFS_FRAGMENT_INDEXES(A) *\
						sizeof(u64))

/* inode lookup table defines */
#define SQUASHFS_LOOKUP_BYTES(A)	((A) * sizeof(u64))

#define SQUASHFS_LOOKUP_BLOCK(A)	(SQUASHFS_LOOKUP_BYTES(A) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_LOOKUP_BLOCK_OFFSET(A)	(SQUASHFS_LOOKUP_BYTES(A) % \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_LOOKUP_BLOCKS(A)	((SQUASHFS_LOOKUP_BYTES(A) + \
					SQUASHFS_METADATA_SIZE - 1) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_LOOKUP_BLOCK_BYTES(A)	(SQUASHFS_LOOKUP_BLOCKS(A) *\
					sizeof(u64))

/* uid/gid lookup table defines */
#define SQUASHFS_ID_BYTES(A)		((A) * sizeof(unsigned int))

#define SQUASHFS_ID_BLOCK(A)		(SQUASHFS_ID_BYTES(A) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_ID_BLOCK_OFFSET(A)	(SQUASHFS_ID_BYTES(A) % \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_ID_BLOCKS(A)		((SQUASHFS_ID_BYTES(A) + \
					SQUASHFS_METADATA_SIZE - 1) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_ID_BLOCK_BYTES(A)	(SQUASHFS_ID_BLOCKS(A) *\
					sizeof(u64))
/* xattr id lookup table defines */
#define SQUASHFS_XATTR_BYTES(A)		((A) * sizeof(struct squashfs_xattr_id))

#define SQUASHFS_XATTR_BLOCK(A)		(SQUASHFS_XATTR_BYTES(A) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_XATTR_BLOCK_OFFSET(A)	(SQUASHFS_XATTR_BYTES(A) % \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_XATTR_BLOCKS(A)	((SQUASHFS_XATTR_BYTES(A) + \
					SQUASHFS_METADATA_SIZE - 1) / \
					SQUASHFS_METADATA_SIZE)

#define SQUASHFS_XATTR_BLOCK_BYTES(A)	(SQUASHFS_XATTR_BLOCKS(A) *\
					sizeof(u64))
#define SQUASHFS_XATTR_BLK(A)		((unsigned int) ((A) >> 16))

#define SQUASHFS_XATTR_OFFSET(A)	((unsigned int) ((A) & 0xffff))

/* cached data constants for filesystem */
#define SQUASHFS_CACHED_BLKS		8

#define SQUASHFS_MAX_FILE_SIZE_LOG	64

#define SQUASHFS_MAX_FILE_SIZE		(1LL << \
					(SQUASHFS_MAX_FILE_SIZE_LOG - 2))

/* meta index cache */
#define SQUASHFS_META_INDEXES	(SQUASHFS_METADATA_SIZE / sizeof(unsigned int))
#define SQUASHFS_META_ENTRIES	127
#define SQUASHFS_META_SLOTS	8

struct meta_entry {
	u64			data_block;
	unsigned int		index_block;
	unsigned short		offset;
	unsigned short		pad;
};

struct meta_index {
	unsigned int		inode_number;
	unsigned int		offset;
	unsigned short		entries;
	unsigned short		skip;
	unsigned short		locked;
	unsigned short		pad;
	struct meta_entry	meta_entry[SQUASHFS_META_ENTRIES];
};


/*
 * definitions for structures on disk
 */
#define ZLIB_COMPRESSION	1
#define LZMA_COMPRESSION	2
#define LZO_COMPRESSION		3
#define XZ_COMPRESSION		4

struct squashfs_super_block {
	u32			s_magic;
	u32			inodes;
	u32			mkfs_time;
	u32			block_size;
	u32			fragments;
	u16			compression;
	u16			block_log;
	u16			flags;
	u16			no_ids;
	u16			s_major;
	u16			s_minor;
	u64			root_inode;
	u64			bytes_used;
	u64			id_table_start;
	u64			xattr_id_table_start;
	u64			inode_table_start;
	u64			directory_table_start;
	u64			fragment_table_start;
	u64			lookup_table_start;
};

struct squashfs_dir_index {
	u32			index;
	u32			start_block;
	u32			size;
	unsigned char		name[0];
};

struct squashfs_base_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
};

struct squashfs_ipc_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			nlink;
};

struct squashfs_lipc_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			nlink;
	u32			xattr;
};

struct squashfs_dev_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			nlink;
	u32			rdev;
};

struct squashfs_ldev_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			nlink;
	u32			rdev;
	u32			xattr;
};

struct squashfs_symlink_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			nlink;
	u32			symlink_size;
	char			symlink[0];
};

struct squashfs_reg_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			start_block;
	u32			fragment;
	u32			offset;
	u32			file_size;
	u16			block_list[0];
};

struct squashfs_lreg_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u64			start_block;
	u64			file_size;
	u64			sparse;
	u32			nlink;
	u32			fragment;
	u32			offset;
	u32			xattr;
	u16			block_list[0];
};

struct squashfs_dir_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			start_block;
	u32			nlink;
	u16			file_size;
	u16			offset;
	u32			parent_inode;
};

struct squashfs_ldir_inode {
	u16			inode_type;
	u16			mode;
	u16			uid;
	u16			guid;
	u32			mtime;
	u32			inode_number;
	u32			nlink;
	u32			file_size;
	u32			start_block;
	u32			parent_inode;
	u16			i_count;
	u16			offset;
	u32			xattr;
	struct squashfs_dir_index	index[0];
};

union squashfs_inode {
	struct squashfs_base_inode		base;
	struct squashfs_dev_inode		dev;
	struct squashfs_ldev_inode		ldev;
	struct squashfs_symlink_inode		symlink;
	struct squashfs_reg_inode		reg;
	struct squashfs_lreg_inode		lreg;
	struct squashfs_dir_inode		dir;
	struct squashfs_ldir_inode		ldir;
	struct squashfs_ipc_inode		ipc;
	struct squashfs_lipc_inode		lipc;
};

struct squashfs_dir_entry {
	u16			offset;
	u16			inode_number;
	u16			type;
	u16			size;
	char			name[0];
};

struct squashfs_dir_header {
	u32			count;
	u32			start_block;
	u32			inode_number;
};

struct squashfs_fragment_entry {
	u64			start_block;
	u32			size;
	unsigned int		unused;
};

struct squashfs_xattr_entry {
	u16			type;
	u16			size;
	char			data[0];
};

struct squashfs_xattr_val {
	u32			vsize;
	char			value[0];
};

struct squashfs_xattr_id {
	u64			xattr;
	u32			count;
	u32			size;
};

struct squashfs_xattr_id_table {
	u64			xattr_table_start;
	u32			xattr_ids;
	u32			unused;
};

#endif
