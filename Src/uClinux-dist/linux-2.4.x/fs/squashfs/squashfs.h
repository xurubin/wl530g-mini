/*
 * Squashfs - a compressed read only filesystem for Linux
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
 * squashfs.h
 */
#ifndef __SQUASHFS_H__
#define __SQUASHFS_H__
//////////////////////////////////////////////////////////////////////////////

#define WARNING(s, args...)	printk(KERN_WARNING "SQUASHFS: "s, ## args)

#ifdef SQUASHFS_TRACE
#define TRACE(s, args...)	printk(KERN_NOTICE "SQUASHFS: "s, ## args)
#else
#define TRACE(s, args...)	{}
#endif

#define ERROR(s, args...)	printk(KERN_ERR "SQUASHFS error: "s, ## args)

#define WARNING(s, args...)	printk(KERN_WARNING "SQUASHFS: "s, ## args)

extern void *kmalloc(size_t, int);
static inline void *kzalloc(size_t size, unsigned int  flags)
{
	void * ptr =  kmalloc(size, flags );
	if (ptr)
	{
		memset(ptr, 0, size);
	}
	return ptr;
}
static inline void * kcalloc  (size_t n, size_t size, unsigned int flags)
{
	return kzalloc(n * size, flags);
}
#define i_size_read(INO)           (INO->i_size)


static inline u32 new_encode_dev(dev_t dev)
{
        unsigned major = MAJOR(dev);
        unsigned minor = MINOR(dev);
        return (minor & 0xff) | (major << 8) | ((minor & ~0xff) << 12);
}
static inline dev_t new_decode_dev(u32 dev)
{
        unsigned major = (dev & 0xfff00) >> 8;
        unsigned minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);
        return MKDEV(major, minor);
}
static inline u64 huge_encode_dev(dev_t dev)
{
        return new_encode_dev(dev);
}

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type
//////////////////////////////////////////////////////////////////////////////

/* block.c */
extern int squashfs_read_data(struct super_block *, void **, u64, int, u64 *,
				int, int);

/* cache.c */
extern struct squashfs_cache *squashfs_cache_init(char *, int, int);
extern void squashfs_cache_delete(struct squashfs_cache *);
extern struct squashfs_cache_entry *squashfs_cache_get(struct super_block *,
				struct squashfs_cache *, u64, int);
extern void squashfs_cache_put(struct squashfs_cache_entry *);
extern int squashfs_copy_data(void *, struct squashfs_cache_entry *, int, int);
extern int squashfs_read_metadata(struct super_block *, void *, u64 *,
				int *, int);
extern struct squashfs_cache_entry *squashfs_get_fragment(struct super_block *,
				u64, int);
extern struct squashfs_cache_entry *squashfs_get_datablock(struct super_block *,
				u64, int);
extern int squashfs_read_table(struct super_block *, void *, u64, int);

/* decompressor.c */
extern const struct squashfs_decompressor *squashfs_lookup_decompressor(int);
extern void *squashfs_decompressor_init(struct super_block *, unsigned short);

/* export.c */
extern u64 *squashfs_read_inode_lookup_table(struct super_block *, u64,
				unsigned int);

/* fragment.c */
extern int squashfs_frag_lookup(struct super_block *, unsigned int, u64 *);
extern u64 *squashfs_read_fragment_index_table(struct super_block *,
				u64, unsigned int);

/* id.c */
extern int squashfs_get_id(struct super_block *, unsigned int, unsigned int *);
extern u64 *squashfs_read_id_index_table(struct super_block *, u64,
				unsigned short);

/* inode.c */
extern struct inode *squashfs_iget(struct super_block *, long long,
				unsigned int);
extern int squashfs_read_inode(struct inode *, long long);

/* xattr.c */
extern ssize_t squashfs_listxattr(struct dentry *, char *, size_t);

/*
 * Inodes, files,  decompressor and xattr operations
 */

/* dir.c */
extern const struct file_operations squashfs_dir_ops;

/* export.c */
extern const struct export_operations squashfs_export_ops;

/* file.c */
extern const struct address_space_operations squashfs_aops;

/* inode.c */
extern const struct inode_operations squashfs_inode_ops;

/* namei.c */
extern const struct inode_operations squashfs_dir_inode_ops;

/* symlink.c */
extern const struct address_space_operations squashfs_symlink_aops;
extern const struct inode_operations squashfs_symlink_inode_ops;

/* xattr.c */
extern const struct xattr_handler *squashfs_xattr_handlers[];

/* zlib_wrapper.c */
extern const struct squashfs_decompressor squashfs_zlib_comp_ops;
#endif
