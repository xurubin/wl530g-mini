/*
 * Resizable simple ram filesystem for Linux.
 *
 * Copyright (C) 2000 Linus Torvalds.
 *               2000 Transmeta Corp.
 *
 * Usage limits added by David Gibson, Linuxcare Australia.
 * Some 2.4-VM compatibility fixes, and a few improvements
 * by Willy Tarreau, EXOSEC - 2005/11/20, 2006/01/16.
 * This file is released under the GPL.
 */

/*
 * NOTE! This filesystem is probably most useful
 * not as a real filesystem, but as an example of
 * how virtual filesystems can be written.
 *
 * It doesn't get much simpler than this. Consider
 * that this file implements the full semantics of
 * a POSIX-compliant read-write filesystem.
 *
 * Note in particular how the filesystem does not
 * need to implement any data structures of its own
 * to keep track of the virtual data: using the VFS
 * caches is sufficient.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/locks.h>
#include <linux/highmem.h>
#include <linux/slab.h>

#include <asm/uaccess.h>
#include <linux/spinlock.h>

#if PAGE_CACHE_SIZE % 1024
#error Oh no, PAGE_CACHE_SIZE is not divisible by 1k! I cannot cope.
#endif

#define IBLOCKS_PER_PAGE  (PAGE_CACHE_SIZE / 512)
#define K_PER_PAGE (PAGE_CACHE_SIZE / 1024)

/* some random number */
#define RAMFS_MAGIC	0x858458f6

static struct super_operations ramfs_ops;
static struct address_space_operations ramfs_aops;
static struct file_operations ramfs_file_operations;
static struct inode_operations ramfs_dir_inode_operations;

/*
 * ramfs super-block data in memory
 */
struct ramfs_sb_info {
	/* Prevent races accessing the used block
	 * counts. Conceptually, this could probably be a semaphore,
	 * but the only thing we do while holding the lock is
	 * arithmetic, so there's no point */
	spinlock_t ramfs_lock;

	/* It is important that at least the free counts below be
	   signed.  free_XXX may become negative if a limit is changed
	   downwards (by a remount) below the current usage. */	  

	/* maximum number of pages in a file */
	long max_file_pages;

	/* max total number of data pages */
	long max_pages;
	/* free_pages = max_pages - total number of pages currently in use */
	long free_pages;
	
	/* max number of inodes */
	long max_inodes;
	/* free_inodes = max_inodes - total number of inodes currently in use */
	long free_inodes;

	/* max number of dentries */
	long max_dentries;
	/* free_dentries = max_dentries - total number of dentries in use */
	long free_dentries;
};

#define RAMFS_SB(sb) ((struct ramfs_sb_info *)((sb)->u.generic_sbp))

/*
 * Resource limit helper functions
 */

static inline void lock_rsb(struct ramfs_sb_info *rsb)
{
	spin_lock(&(rsb->ramfs_lock));
}

static inline void unlock_rsb(struct ramfs_sb_info *rsb)
{
	spin_unlock(&(rsb->ramfs_lock));
}

/* Decrements the free inode count and returns true, or returns false
 * if there are no free inodes */
static int ramfs_alloc_inode(struct super_block *sb)
{
	struct ramfs_sb_info *rsb = RAMFS_SB(sb);
	int ret = 1;

	lock_rsb(rsb);
	if (!rsb->max_inodes || rsb->free_inodes > 0)
		rsb->free_inodes--;
	else
		ret = 0;
	unlock_rsb(rsb);
	
	return ret;
}

/* Increments the free inode count */
static void ramfs_dealloc_inode(struct super_block *sb)
{
	struct ramfs_sb_info *rsb = RAMFS_SB(sb);
	
	lock_rsb(rsb);
	rsb->free_inodes++;
	unlock_rsb(rsb);
}

/* Decrements the free dentry count and returns true, or returns false
 * if there are no free dentries */
static int ramfs_alloc_dentry(struct super_block *sb)
{
	struct ramfs_sb_info *rsb = RAMFS_SB(sb);
	int ret = 1;

	lock_rsb(rsb);
	if (!rsb->max_dentries || rsb->free_dentries > 0)
		rsb->free_dentries--;
	else
		ret = 0;
	unlock_rsb(rsb);
	
	return ret;
}

/* Increments the free dentry count */
static void ramfs_dealloc_dentry(struct super_block *sb)
{
	struct ramfs_sb_info *rsb = RAMFS_SB(sb);
	
	lock_rsb(rsb);
	rsb->free_dentries++;
	unlock_rsb(rsb);
}

/* If the given page can be added to the give inode for ramfs, return
 * true and update the filesystem's free page count and the inode's
 * i_blocks field. Always returns true if the page is already used by
 * ramfs (ie. PG_fs_1 is set). We use the PG_fs_1 flag here because
 * it's the only one which will always remain till ramfs_removepage() */
int ramfs_alloc_page(struct inode *inode, struct page *page)
{
	int ret = 1;

	/* do we already own this page ? */
	if (!test_bit(PG_fs_1, &page->flags)) {
		struct ramfs_sb_info *rsb = RAMFS_SB(inode->i_sb);
		lock_rsb(rsb);
		if ( (rsb->free_pages > 0) &&
		     ( !rsb->max_file_pages ||
		       (inode->i_data.nrpages <= rsb->max_file_pages) ) ) {
			inode->i_blocks += IBLOCKS_PER_PAGE;
			rsb->free_pages--;
	        	set_bit(PG_fs_1, &page->flags); /* we own it now */
		} else {
			ret = 0;	/* will become ENOSPC */
		}
		unlock_rsb(rsb);
	}

	return ret;
}

static int ramfs_statfs(struct super_block *sb, struct statfs *buf)
{
	struct ramfs_sb_info *rsb = RAMFS_SB(sb);

	lock_rsb(rsb);
	buf->f_blocks = rsb->max_pages;
	buf->f_files = rsb->max_inodes;

	buf->f_bfree = rsb->free_pages;
	buf->f_bavail = buf->f_bfree;
	buf->f_ffree = rsb->free_inodes;
	unlock_rsb(rsb);

	buf->f_type = RAMFS_MAGIC;
	buf->f_bsize = PAGE_CACHE_SIZE;
	buf->f_namelen = 255;
	return 0;
}

/*
 * Lookup the data. This is trivial - if the dentry didn't already
 * exist, we know it is negative.
 */
static struct dentry * ramfs_lookup(struct inode *dir, struct dentry *dentry)
{
	d_add(dentry, NULL);
	return NULL;
}

/*
 * Read a page. Again trivial. If it didn't already exist
 * in the page cache, it is zero-filled.
 */
static int ramfs_readpage(struct file *file, struct page * page)
{
	if (!Page_Uptodate(page)) {
		memset(kmap(page), 0, PAGE_CACHE_SIZE);
		kunmap(page);
		flush_dcache_page(page);
		SetPageUptodate(page);
	}
	UnlockPage(page);
	return 0;
}

static int ramfs_writepage(struct page *page)
{
	struct inode *inode = (struct inode *)page->mapping->host;

	if (PageLaunder(page)) {
		activate_page(page);
		SetPageReferenced(page);
	}

	if (! ramfs_alloc_page(inode, page)) {
		UnlockPage(page);
		return -ENOSPC;
	}

	/* Set the page dirty again, unlock */
	SetPageDirty(page);
	UnlockPage(page);
	return 0;
}

static int ramfs_prepare_write(struct file *file, struct page *page, unsigned offset, unsigned to)
{
	struct inode *inode = (struct inode *)page->mapping->host;
	void *addr;

	if (! ramfs_alloc_page(inode, page)) {
		ClearPageUptodate(page);
		return -ENOSPC;
	}

	addr = (void *) kmap(page);
	if (!Page_Uptodate(page)) {
		memset(addr, 0, PAGE_CACHE_SIZE);
		flush_dcache_page(page);
		SetPageUptodate(page);
	}
	SetPageDirty(page);
	return 0;
}

static int ramfs_commit_write(struct file *file, struct page *page, unsigned offset, unsigned to)
{
	struct inode *inode = page->mapping->host;
	loff_t pos = ((loff_t)page->index << PAGE_CACHE_SHIFT) + to;

	kunmap(page);
	if (pos > inode->i_size)
		inode->i_size = pos;
	return 0;
}

static void ramfs_removepage(struct page *page)
{
	struct inode *inode = (struct inode *)page->mapping->host;

	/* Did we own this page ? */
	if (test_bit(PG_fs_1, &page->flags)) {
		struct ramfs_sb_info *rsb = RAMFS_SB(inode->i_sb);
		lock_rsb(rsb);
		if (rsb->free_pages >= rsb->max_pages) {
			printk(KERN_ERR "ramfs: Error in page deallocation, free_pages (%ld) > max_pages (%ld)\n", rsb->free_pages, rsb->max_pages);
		} else {
			rsb->free_pages++;
			inode->i_blocks -= IBLOCKS_PER_PAGE;
		}
		unlock_rsb(rsb);
	        clear_bit(PG_fs_1, &page->flags);
	}
}

struct inode *ramfs_get_inode(struct super_block *sb, int mode, int dev)
{
	struct inode * inode;

	if (! ramfs_alloc_inode(sb))
		return NULL;

	inode = new_inode(sb);

	if (inode) {
		inode->i_mode = mode;
		inode->i_uid = current->fsuid;
		inode->i_gid = current->fsgid;
		inode->i_blksize = PAGE_CACHE_SIZE;
		inode->i_blocks = 0;
		inode->i_rdev = NODEV;
		inode->i_mapping->a_ops = &ramfs_aops;
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
		switch (mode & S_IFMT) {
		default:
			init_special_inode(inode, mode, dev);
			break;
		case S_IFREG:
			inode->i_fop = &ramfs_file_operations;
			break;
		case S_IFDIR:
			inode->i_op = &ramfs_dir_inode_operations;
			inode->i_fop = &dcache_dir_ops;
			break;
		case S_IFLNK:
			inode->i_op = &page_symlink_inode_operations;
			break;
		}
	} else
		ramfs_dealloc_inode(sb);

	return inode;
}

/*
 * File creation. Allocate an inode, update free inode and dentry counts
 * and we're done..
 */
static int ramfs_mknod(struct inode *dir, struct dentry *dentry, int mode, int dev)
{
	struct super_block *sb = dir->i_sb;
	struct inode * inode;
	int error = -ENOSPC;

	if (! ramfs_alloc_dentry(sb))
		return error;

	inode = ramfs_get_inode(dir->i_sb, mode, dev);

	if (inode) {
		d_instantiate(dentry, inode);
		dget(dentry);		/* Extra count - pin the dentry in core */
		error = 0;
	} else {
		ramfs_dealloc_dentry(sb);
	}
	return error;
}

static int ramfs_mkdir(struct inode * dir, struct dentry * dentry, int mode)
{
	return ramfs_mknod(dir, dentry, mode | S_IFDIR, 0);
}

static int ramfs_create(struct inode *dir, struct dentry *dentry, int mode)
{
	return ramfs_mknod(dir, dentry, mode | S_IFREG, 0);
}

/*
 * Link a file..
 */
static int ramfs_link(struct dentry *old_dentry, struct inode * dir, struct dentry * dentry)
{
	struct super_block *sb = dir->i_sb;
	struct inode *inode = old_dentry->d_inode;

	if (S_ISDIR(inode->i_mode))
		return -EPERM;

	if (! ramfs_alloc_dentry(sb))
		return -ENOSPC;

	inode->i_nlink++;
	atomic_inc(&inode->i_count);	/* New dentry reference */
	dget(dentry);		/* Extra pinning count for the created dentry */
	d_instantiate(dentry, inode);
	return 0;
}

static inline int ramfs_positive(struct dentry *dentry)
{
	return dentry->d_inode && !d_unhashed(dentry);
}

/*
 * Check that a directory is empty (this works
 * for regular files too, they'll just always be
 * considered empty..).
 *
 * Note that an empty directory can still have
 * children, they just all have to be negative..
 */
static int ramfs_empty(struct dentry *dentry)
{
	struct list_head *list;

	spin_lock(&dcache_lock);
	list = dentry->d_subdirs.next;

	while (list != &dentry->d_subdirs) {
		struct dentry *de = list_entry(list, struct dentry, d_child);

		if (ramfs_positive(de)) {
			spin_unlock(&dcache_lock);
			return 0;
		}
		list = list->next;
	}
	spin_unlock(&dcache_lock);
	return 1;
}

/*
 * This works for both directories and regular files.
 * (non-directories will always have empty subdirs)
 */
static int ramfs_unlink(struct inode * dir, struct dentry *dentry)
{
	struct super_block *sb = dir->i_sb;
	int retval = -ENOTEMPTY;

	if (ramfs_empty(dentry)) {
		struct inode *inode = dentry->d_inode;

		inode->i_nlink--;
		dput(dentry);			/* Undo the count from "create" - this does all the work */

		ramfs_dealloc_dentry(sb);

		retval = 0;
	}
	return retval;
}

#define ramfs_rmdir ramfs_unlink

/*
 * The VFS layer already does all the dentry stuff for rename,
 * we just have to decrement the usage count for the target if
 * it exists so that the VFS layer correctly free's it when it
 * gets overwritten.
 */
static int ramfs_rename(struct inode * old_dir, struct dentry *old_dentry, struct inode * new_dir,struct dentry *new_dentry)
{
	struct super_block *sb = new_dir->i_sb;

	int error = -ENOTEMPTY;

	if (ramfs_empty(new_dentry)) {
		struct inode *inode = new_dentry->d_inode;
		if (inode) {
			inode->i_nlink--;
			dput(new_dentry);
			ramfs_dealloc_dentry(sb);
		}
		error = 0;
	}
	return error;
}

static int ramfs_symlink(struct inode * dir, struct dentry *dentry, const char * symname)
{
	int error;

	error = ramfs_mknod(dir, dentry, S_IFLNK | S_IRWXUGO, 0);
	if (!error) {
		int l = strlen(symname)+1;
		struct inode *inode = dentry->d_inode;
		error = block_symlink(inode, symname, l);
	}
	return error;
}

static int ramfs_sync_file(struct file * file, struct dentry *dentry, int datasync)
{
	return 0;
}

static void ramfs_delete_inode(struct inode *inode)
{
	ramfs_dealloc_inode(inode->i_sb);

	clear_inode(inode);
}

static void ramfs_put_super(struct super_block *sb)
{
	kfree(sb->u.generic_sbp);
}

struct ramfs_params {
	long pages;
	long filepages;
	long inodes;
	long dentries;
};

static int parse_options(char * options, struct ramfs_params *p)
{
	char save = 0, *savep = NULL, *optname, *value;

	p->pages = -1;
	p->filepages = -1;
	p->inodes = -1;
	p->dentries = -1;

	for (optname = strtok(options,","); optname;
		     optname = strtok(NULL,",")) {
	if ((value = strchr(optname,'=')) != NULL) {
			save = *value;
			savep = value;
			*value++ = 0;
		}

		if (!strcmp(optname, "maxfilesize") && value) {
			p->filepages = memparse(value, &value)
				/ K_PER_PAGE;
			if (*value)
				return -EINVAL;
		} else if (!strcmp(optname, "maxsize") && value) {
			p->pages = memparse(value, &value) // size in blocks
				/ K_PER_PAGE;
			if (*value)
				return -EINVAL;
		/* we also accept tmpfs-like 'size' option */
		} else if (!strcmp(optname, "size") && value) {
			unsigned long long size;
			size = memparse(value, &value);
			if (*value == '%') {
				struct sysinfo si;
				si_meminfo(&si);
				size <<= PAGE_SHIFT;
				size *= si.totalram;
				size /= 100;
				value++;
			}
			if (*value)
				return -EINVAL;
			p->pages = size >> PAGE_SHIFT;
		} else if ((!strcmp(optname, "nr_inodes") ||
			!strcmp(optname, "maxinodes")) && value) {
			p->inodes = memparse(value, &value);
			if (*value)
				return -EINVAL;
 		} else if (!strcmp(optname, "maxdentries") && value) {
			p->dentries = memparse(value, &value);
			if (*value)
				return -EINVAL;
		}

		if (optname != options)
			*(optname-1) = ',';
		if (value)
			*savep = save;
	}

	return 0;
}

static void init_limits(struct ramfs_sb_info *rsb, struct ramfs_params *p)
{
	struct sysinfo si;

	si_meminfo(&si);

	/* By default we set the limits to be:
	       - Allow this ramfs to take up to 25% of all available RAM
	       - No limit on filesize (except no file may be bigger that
	         the total max size, obviously)
	       - dentries limited to one per 4k of data space
	       - No limit to the number of inodes (except that there
	         are never more inodes than dentries).
	*/
	rsb->max_pages = (si.totalram / 4);

	if (p->pages >= 0)
		rsb->max_pages = p->pages;

	rsb->max_file_pages = 0;
	if (p->filepages >= 0)
 		rsb->max_file_pages = p->filepages;

	rsb->max_dentries = rsb->max_pages * K_PER_PAGE / 4;
 	if (p->dentries >= 0)
		rsb->max_dentries = p->dentries;

	rsb->max_inodes = 0;
 	if (p->inodes >= 0)
		rsb->max_inodes = p->inodes;

	rsb->free_pages = rsb->max_pages;
	rsb->free_inodes = rsb->max_inodes;
	rsb->free_dentries = rsb->max_dentries;

	return;
}

/* reset_limits is called during a remount to change the usage limits.

   This will suceed, even if the new limits are lower than current
   usage. This is the intended behaviour - new allocations will fail
   until usage falls below the new limit */
static void reset_limits(struct ramfs_sb_info *rsb, struct ramfs_params *p)
{
	lock_rsb(rsb);

	if (p->pages >= 0) {
		int used_pages = rsb->max_pages - rsb->free_pages;

		rsb->max_pages = p->pages;
		rsb->free_pages = rsb->max_pages - used_pages;
	}

	if (p->filepages >= 0) {
		rsb->max_file_pages = p->filepages;
 	}
	

	if (p->dentries >= 0) {
		int used_dentries = rsb->max_dentries - rsb->free_dentries;

		rsb->max_dentries = p->dentries;
		rsb->free_dentries = rsb->max_dentries - used_dentries;
	}

	if (p->inodes >= 0) {
		int used_inodes = rsb->max_inodes - rsb->free_inodes;

		rsb->max_inodes = p->inodes;
		rsb->free_inodes = rsb->max_inodes - used_inodes;
	}

	unlock_rsb(rsb);
}

static int ramfs_remount(struct super_block * sb, int * flags, char * data)
{
	struct ramfs_params params;
	struct ramfs_sb_info * rsb = RAMFS_SB(sb);

	if (parse_options((char *)data, &params) != 0)
		return -EINVAL;

	reset_limits(rsb, &params);

#if CONFIG_DEBUG_RAMFS
 	printk(KERN_DEBUG "ramfs: remounted with options: %s\n", 
	       data ? (char *)data : "<defaults>" );
	printk(KERN_DEBUG "ramfs: max_pages=%ld max_file_pages=%ld "
		"max_inodes=%ld max_dentries=%ld\n",
	       rsb->max_pages, rsb->max_file_pages,
	       rsb->max_inodes, rsb->max_dentries);
#endif CONFIG_DEBUG_RAMFS

	return 0;
}
 
static struct address_space_operations ramfs_aops = {
	readpage:	ramfs_readpage,
	writepage:	ramfs_writepage,
	prepare_write:	ramfs_prepare_write,
	commit_write:	ramfs_commit_write,
	removepage:	ramfs_removepage,
};

static struct file_operations ramfs_file_operations = {
	read:		generic_file_read,
	write:		generic_file_write,
	mmap:		generic_file_mmap,
	fsync:		ramfs_sync_file,
};

static struct inode_operations ramfs_dir_inode_operations = {
	create:		ramfs_create,
	lookup:		ramfs_lookup,
	link:		ramfs_link,
	unlink:		ramfs_unlink,
	symlink:	ramfs_symlink,
	mkdir:		ramfs_mkdir,
	rmdir:		ramfs_rmdir,
	mknod:		ramfs_mknod,
	rename:		ramfs_rename,
};

static struct super_operations ramfs_ops = {
	statfs:		ramfs_statfs,
	put_inode:	force_delete,
	delete_inode:	ramfs_delete_inode,
	put_super:      ramfs_put_super,
	remount_fs:     ramfs_remount,
};

/*
 * Initialisation
 */

static struct super_block *ramfs_read_super(struct super_block * sb, void * data, int silent)
{
	struct inode * inode;
	struct dentry * root;
	struct ramfs_sb_info * rsb;
	struct ramfs_params params;

	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = RAMFS_MAGIC;
	sb->s_op = &ramfs_ops;

	sb->u.generic_sbp = kmalloc(sizeof(struct ramfs_sb_info), GFP_KERNEL);
	rsb = RAMFS_SB(sb);

	spin_lock_init(&rsb->ramfs_lock);

	if (parse_options((char *)data, &params) != 0)
		return NULL;

	init_limits(rsb, &params);

	inode = ramfs_get_inode(sb, S_IFDIR | 0755, 0);
	if (!inode)
		return NULL;

	root = d_alloc_root(inode);
	if (!root) {
		iput(inode);
		return NULL;
	}
	sb->s_root = root;

#if CONFIG_DEBUG_RAMFS
	printk(KERN_DEBUG "ramfs: mounted with options: %s\n", 
	       data ? (char *)data : "<defaults>" );
	printk(KERN_DEBUG "ramfs: max_pages=%ld max_file_pages=%ld "
		"max_inodes=%ld max_dentries=%ld\n",
	       rsb->max_pages, rsb->max_file_pages,
	       rsb->max_inodes, rsb->max_dentries);
#endif CONFIG_DEBUG_RAMFS
	return sb;
}

static DECLARE_FSTYPE(ramfs_fs_type, "ramfs", ramfs_read_super, FS_LITTER);
static DECLARE_FSTYPE(rootfs_fs_type, "rootfs", ramfs_read_super, FS_NOMOUNT|FS_LITTER);

static int __init init_ramfs_fs(void)
{
	return register_filesystem(&ramfs_fs_type);
}

static void __exit exit_ramfs_fs(void)
{
	unregister_filesystem(&ramfs_fs_type);
}

module_init(init_ramfs_fs)
module_exit(exit_ramfs_fs)

int __init init_rootfs(void)
{
	return register_filesystem(&rootfs_fs_type);
}

MODULE_LICENSE("GPL");

