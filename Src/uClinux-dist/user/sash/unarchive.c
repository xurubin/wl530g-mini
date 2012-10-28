/*
 *  Copyright (C) 2000 by Glenn McGrath
 *  Copyright (C) 2001 by Laurence Anderson
 *	
 *  Based on previous work by busybox developers and others.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include "lzfx.h"
#include "../uhttpd/nvram.h"

const char* TEMPLATE_SUFFIX = ".tpl";
off_t archive_offset;

typedef struct tar_entrys_s {
        char *name;
        char *link_name;
        off_t size;
        uid_t uid;
        gid_t gid;
        mode_t mode;
        time_t mtime;
        dev_t device;
} tar_entry_t;

typedef struct {
	char* buf;
	int size;
	int ptr;
} MEMFILE;

void perror_msg(const char *s, ...)
{
        va_list p;

        va_start(p, s);
        vprintf(s, p);
		printf("\n");
        va_end(p);
}
void error_msg(const char* s) {
	printf("%s\n", s);
}
size_t memfread ( void * ptr, size_t size, size_t count, MEMFILE * stream ) {
	int s = size * count;
	if (stream->ptr + s >= stream->size)
		s = stream->size - stream->ptr;
	memcpy(ptr, stream->buf + stream->ptr, s);
	stream->ptr += s;
	return s;
}
int copy_file_chunk(MEMFILE *src_file, FILE *dst_file, unsigned long long chunksize)
{
        size_t nread, nwritten, size;
        char buffer[BUFSIZ];

        while (chunksize != 0) {
                if (chunksize > BUFSIZ)
                        size = BUFSIZ;
                else
                        size = chunksize;

                nread = memfread (buffer, 1, size, src_file);

                if (nread != size ) {
                        perror_msg ("read");
                        return -1;
                } else if (nread == 0) {
                        if (chunksize != -1) {
                                error_msg ("Unable to read all data");
                                return -1;
                        }

                        return 0;
                }

                nwritten = fwrite (buffer, 1, nread, dst_file);

                if (nwritten != nread) {
                        if (ferror (dst_file))
                                perror_msg ("write");
                        else
                                error_msg ("Unable to write all data");
                        return -1;
                }

                if (chunksize != -1)
                        chunksize -= nwritten;
        }

        return 0;
}

void expand_variable(const char* var_name, FILE* dst_file) {
	char var_content[128];
	if (nvram_get_variable(var_name, var_content, sizeof(var_content))) {
		fwrite(var_content, 1, strlen(var_content), dst_file);
	}
}
int expand_template(MEMFILE *src_file, FILE *dst_file, unsigned long long chunksize) {
	char * buf = src_file->buf + src_file->ptr;
	if (chunksize > src_file->size - src_file->ptr)
		chunksize = src_file->size - src_file->ptr;
		
	int p = 0;
	char config_var[32];
	int config_var_len = 0;
	int parsing_var = 0;
	while (p < chunksize) {
		if (parsing_var) {
			// found end tag
			if (p + 1 < chunksize && buf[p] == '?' && buf[p+1] == '>') {
				//substitute config_var[config_var_len]
				config_var[config_var_len] = '\0';
				expand_variable(config_var, dst_file);
				p += 2;
				config_var_len = 0;
				parsing_var = 0;
			} else { // part of variable name
				// variable name too long, treat it as plain text
				if (config_var_len >= sizeof(config_var) - 1) {
					fwrite("<?", 1, 2, dst_file);
					fwrite(config_var, 1, config_var_len, dst_file);
					config_var_len = 0;
					parsing_var = 0;
				} else {
					config_var[config_var_len++] = buf[p];
				}
				p++;
			}
		} else {
			if (p + 1 < chunksize && buf[p] == '<' && buf[p+1] == '?') {
				p += 2;
				config_var_len = 0;
				parsing_var = 1;
			} else { 
				fwrite(buf + p, 1, 1, dst_file);
				p++;
			}
		}
	}
	if (parsing_var) { // Got a unclosed variable at the end of file
		fwrite("<?", 1, 2, dst_file);
		fwrite(config_var, 1, config_var_len, dst_file);
	}
	src_file->ptr += chunksize;
}

void seek_sub_file(MEMFILE *src_stream, const int count)
{
	archive_offset += count;
	src_stream->ptr += count;
}


/* Extract the data postioned at src_stream to either filesystem, stdout or 
 * buffer depending on the value of 'function' which is defined in libbb.h 
 *
 * prefix doesnt have to be just a directory, it may prefix the filename as well.
 *
 * e.g. '/var/lib/dpkg/info/dpkg.' will extract all files to the base bath 
 * '/var/lib/dpkg/info/' and all files/dirs created in that dir will have 
 * 'dpkg.' as their prefix
 *
 * For this reason if prefix does point to a dir then it must end with a
 * trailing '/' or else the last dir will be assumed to be the file prefix 
 */
char *extract_archive(MEMFILE *src_stream, const tar_entry_t *file_entry, const char *prefix)
{
	FILE *dst_stream = NULL;
	char *full_name = NULL;
	char *buffer = NULL;
	struct utimbuf t;
	int is_template = 0;
	
	/* prefix doesnt have to be a proper path it may prepend 
	 * the filename as well */
	if (prefix != NULL) {
		/* strip leading '/' in filename to extract as prefix may not be dir */
		/* Cant use concat_path_file here as prefix might not be a directory */
		char *path = file_entry->name;
		if (strncmp("./", path, 2) == 0) {
			path += 2;
			if (strlen(path) == 0) {
				return(NULL);
			}
		}
		full_name = malloc(strlen(prefix) + strlen(path) + 1);
		strcpy(full_name, prefix);
		strcat(full_name, path);
	} else {
		full_name = file_entry->name;
	}

	/* Check if the file is a template */
	int suffix_i = strlen(full_name) - strlen(TEMPLATE_SUFFIX);
	if (suffix_i && (!strcmp(full_name + suffix_i,
				TEMPLATE_SUFFIX)) ) {
		is_template = 1;
		full_name[suffix_i] = '\0'; // Remove the template suffix.
	}
	/* Ignore this for now: The file already exists */
	struct stat oldfile;
	int stat_res;
	stat_res = lstat (full_name, &oldfile);
	/* Ignore this for now: Create leading directories with default umask */
	switch(file_entry->mode & S_IFMT) {
		case S_IFREG:
			if (file_entry->link_name) { /* Found a cpio hard link */
				if (link(file_entry->link_name, full_name) != 0) {
					perror_msg("Cannot link from %s to '%s'",
							file_entry->name, file_entry->link_name);
				}
			} else {
				if ((dst_stream = fopen(full_name, "w")) == NULL) {
					seek_sub_file(src_stream, file_entry->size);
					return NULL;
				}
				archive_offset += file_entry->size;
				if (!is_template)
					copy_file_chunk(src_stream, dst_stream, file_entry->size);			
				else
					expand_template(src_stream, dst_stream, file_entry->size);			
				fclose(dst_stream);
			}
			break;
		case S_IFDIR:
			if (stat_res != 0) {
				if (mkdir(full_name, file_entry->mode) < 0) {
					perror_msg("extract_archive: %s", full_name);
				}
			}
			break;
		case S_IFLNK:
			if (symlink(file_entry->link_name, full_name) < 0) {
				perror_msg("Cannot create symlink from %s to '%s'", file_entry->name, file_entry->link_name);
				return NULL;
			}
			break;
		case S_IFSOCK:
		case S_IFBLK:
		case S_IFCHR:
		case S_IFIFO:
			if (mknod(full_name, file_entry->mode, file_entry->device) == -1) {
				perror_msg("Cannot create node %s", file_entry->name);
				return NULL;
			}
			break;
	}

	/* Changing a symlink's properties normally changes the properties of the 
	 * file pointed to, so dont try and change the date or mode, lchown does
	 * does the right thing, but isnt available in older versions of libc */
	if (S_ISLNK(file_entry->mode)) {
#if (__GLIBC__ > 2) && (__GLIBC_MINOR__ > 1)
		lchown(full_name, file_entry->uid, file_entry->gid);
#endif
	} else {
		// Preserve time
		t.actime = file_entry->mtime;
		t.modtime = file_entry->mtime;
		utime(full_name, &t);
		chmod(full_name, file_entry->mode);
		chown(full_name, file_entry->uid, file_entry->gid);
	}

	free(full_name);

	return(NULL); /* Maybe we should say if failed */
}

tar_entry_t *get_header_tar(MEMFILE *tar_stream)
{
	union {
		unsigned char raw[512];
		struct {
			char name[100];		/*   0-99 */
			char mode[8];		/* 100-107 */
			char uid[8];		/* 108-115 */
			char gid[8];		/* 116-123 */
			char size[12];		/* 124-135 */
			char mtime[12];		/* 136-147 */
			char chksum[8];		/* 148-155 */
			char typeflag;		/* 156-156 */
			char linkname[100];	/* 157-256 */
			char magic[6];		/* 257-262 */
			char version[2];	/* 263-264 */
			char uname[32];		/* 265-296 */
			char gname[32];		/* 297-328 */
			char devmajor[8];	/* 329-336 */
			char devminor[8];	/* 337-344 */
			char prefix[155];	/* 345-499 */
			char padding[12];	/* 500-512 */
		} formated;
	} tar;
	tar_entry_t *tar_entry = NULL;
	long i;
	long sum = 0;

	if (archive_offset % 512 != 0) {
		seek_sub_file(tar_stream, 512 - (archive_offset % 512));
	}

	if (memfread(tar.raw, 1, 512, tar_stream) != 512) {
		/* Unfortunatly its common for tar files to have all sorts of
		 * trailing garbage, fail silently */
//		error_msg("Couldnt read header");
		return(NULL);
	}
	archive_offset += 512;

	/* Check header has valid magic, unfortunately some tar files
	 * have empty (0'ed) tar entries at the end, which will
	 * cause this to fail, so fail silently for now
	 */
	if (strncmp(tar.formated.magic, "ustar", 5) != 0) {
		return(NULL);
	}

	/* Do checksum on headers */
	for (i =  0; i < 148 ; i++) {
		sum += tar.raw[i];
	}
	sum += ' ' * 8;
	for (i =  156; i < 512 ; i++) {
		sum += tar.raw[i];
	}
	if (sum != strtol(tar.formated.chksum, NULL, 8)) {
		error_msg("Invalid tar header checksum");
		return(NULL);
	}

	/* convert to type'ed variables */
	tar_entry = calloc(1, sizeof(tar_entry_t));
	tar_entry->name = strdup(tar.formated.name);

	tar_entry->uid   = strtol(tar.formated.uid, NULL, 8);
	tar_entry->gid   = strtol(tar.formated.gid, NULL, 8);
	tar_entry->size  = strtol(tar.formated.size, NULL, 8);
	tar_entry->mtime = strtol(tar.formated.mtime, NULL, 8);
	tar_entry->link_name  = strlen(tar.formated.linkname) ? 
	    strdup(tar.formated.linkname) : NULL;
	tar_entry->device = (strtol(tar.formated.devmajor, NULL, 8) << 8) +
		strtol(tar.formated.devminor, NULL, 8);
	/* Set bits 0-11 of the files mode */
	tar_entry->mode = 07777 & strtol(tar.formated.mode, NULL, 8);

	/* Set bits 12-15 of the files mode */
	switch (tar.formated.typeflag) {
	/* busybox identifies hard links as being regular files with 0 size and a link name */
	case '1':
		tar_entry->mode |= S_IFREG;
		break;
	case '7':
		/* Reserved for high performance files, treat as normal file */
	case 0:
	case '0':
		tar_entry->mode |= S_IFREG;
		break;
	case '2':
		tar_entry->mode |= S_IFLNK;
		break;
	case '3':
		tar_entry->mode |= S_IFCHR;
		break;
	case '4':
		tar_entry->mode |= S_IFBLK;
		break;
	case '5':
		tar_entry->mode |= S_IFDIR;
		break;
	case '6':
		tar_entry->mode |= S_IFIFO;
		break;
	default:
		perror_msg("Unknown typeflag: 0x%x", tar.formated.typeflag);
	}
	return(tar_entry);
}

char *unarchive(MEMFILE *src_stream, const char *prefix)
{
	tar_entry_t *file_entry;
	int extract_flag;
	int i;
	char *buffer = NULL;

	archive_offset = 0;
	while ((file_entry = get_header_tar(src_stream)) != NULL) {
		buffer = extract_archive(src_stream, file_entry, prefix);

		free(file_entry->name); /* may be null, but doesn't matter */
		free(file_entry->link_name);
		free(file_entry);
	}
	return(buffer);
}


void unarchive_config() {
	int bufsize = 0;
	const char * databuf = readNVRAM(NVRAM_TEMPLATE, 0, &bufsize);
	if ((!databuf) || (!bufsize)) {
		printf("Cannot load config block.");
		return;
	}
	int kind = (databuf[4]<<8) | databuf[5];
    int len = (databuf[6]<<24) | (databuf[7]<<16) | (databuf[8]<<8) | databuf[9];
    if(databuf[0]!='L' || databuf[1]!='Z' || databuf[2]!='F' || databuf[3]!='X' ||
	   kind != 1 || len > bufsize){
        printf("Bad LZFX config file format.\n");
        return;
    }
	databuf += 10;
	int olen = (databuf[0]<<24) | (databuf[1]<<16) | (databuf[2]<<8) | databuf[3];
	bufsize = len - 4;
	databuf += 4;
	
	char* tarbuf = malloc(olen);
	int rc = lzfx_decompress(databuf, bufsize, tarbuf, &olen);
	if (rc < 0) {
		printf("Cannot decompress: %d\n", rc);
		return;
	}
	MEMFILE tarfile;
	tarfile.buf = tarbuf;
	tarfile.size = olen;
	tarfile.ptr = 0;
	unarchive(&tarfile, "/tmp/");
	free(tarbuf);
}
