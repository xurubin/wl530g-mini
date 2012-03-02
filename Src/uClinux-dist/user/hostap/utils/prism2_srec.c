/*
 * Firmware image downloader for Host AP driver
 * (for Intersil Prism2/2.5/3 cards)
 *
 * Copyright (c) 2002-2003, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>

#include "wireless_copy.h"
#include "util.h"


static int verbose = 0;
static int ignore_incompatible_interface = 0;

#define verbose_printf(a...) do { if (verbose) printf(a); } while (0)
#define vverbose_printf(a...) do { if (verbose > 1) printf(a); } while (0)


struct s3_record {
	unsigned int addr;
	int len;
	unsigned char *data;
};

struct s3_info_platform {
	unsigned int platform, variant, major, minor;
};

struct s3_info_component {
	unsigned int component, variant, major, minor;
};

struct s3_info_compatibility {
	unsigned int role, iface_id, variant, bottom, top;
};

struct s3_crc16 {
	unsigned int start, len, prog;
};

struct s3_plug_record {
	unsigned int pdr, plug_addr, plug_len;
};

#define S3_SPECIAL_MASK		0xff000000
#define S3_PLUG_ADDR		0xff000000
#define S3_CRC16_ADDR		0xff100000
#define S3_FW_INFO_ADDR_MASK	0xffff0000
#define S3_FW_INFO_ADDR		0xff200000
#define S3_PRI_INFO_ADDR	0xff010000
#define S3_ENG_ADDR		0xffff0000


struct srec_data {
	char *name;
	char *name_rec;

	struct s3_info_component component;
	int component_set;

	int start_addr_set;
	unsigned int start_addr;

	struct s3_record *s3_records;
	int s3_used, s3_entries;

	struct s3_info_platform *platforms;
	int platform_count;

	struct s3_info_compatibility *compatibilities;
	int compatibility_count;

	struct s3_crc16 *crc16;
	int crc16_count;

	struct s3_plug_record *plugs;
	int plug_count;

	unsigned int s3_fw_info_addr; /* expected next S3 f/w info address */
	size_t s3_fw_info_buf_len;
	char *s3_fw_info_buf;
};


#define PRISM2_PDA_SIZE 1024

struct pdr {
	unsigned int pdr, len;
	unsigned char *data;
};

struct wlan_info {
	struct s3_info_platform nicid, priid, staid;
	struct s3_info_compatibility mfi_pri_sup, cfi_pri_sup,
		pri_sup, sta_sup,
		mfi_sta_act, cfi_pri_act, cfi_sta_act;

	char pda_buf[PRISM2_PDA_SIZE];
	struct pdr *pdrs;
	int pdr_count;
};


const u16 crc16_table[256] =
{
	0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
	0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
	0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
	0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
	0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
	0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
	0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
	0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
	0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
	0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
	0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
	0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
	0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
	0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
	0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
	0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
	0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
	0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
	0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
	0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
	0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
	0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
	0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
	0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
	0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
	0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
	0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
	0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
	0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
	0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
	0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
	0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};


int crc16(u8 *buf, int len)
{
	u16 crc;
	int i;

	crc = 0;
	for (i = 0; i < len; i++)
		crc = (crc >> 8) ^ crc16_table[(crc & 0xff) ^ *buf++];
	return crc;
}


static int s3_comp(const void *a, const void *b)
{
	const struct s3_record *aa = a, *bb = b;

	return aa->addr - bb->addr;
}

static int hex2int(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);

	printf("{hex2int failure '%c'}", c);
	return -1;
}

int get_hex8(char *txt)
{
	int val, val2;

	val = hex2int(txt[0]);
	val2 = hex2int(txt[1]);
	if (val < 0 || val2 < 0)
		return -1;

	return (val * 16 + val2) & 0xff;
}

unsigned int get_hex16_le(char *txt)
{
	unsigned int val = 0;
	int i;
	for (i = 1; i >= 0; i--)
		val = (val << 8) | get_hex8(&txt[i * 2]);
	return val;
}

unsigned int get_hex32(char *txt)
{
	unsigned int val = 0;
	int i;
	for (i = 0; i < 4; i++)
		val = (val << 8) | get_hex8(&txt[i * 2]);
	return val;
}

unsigned int get_hex32_le(char *txt)
{
	unsigned int val = 0;
	int i;
	for (i = 3; i >= 0; i--)
		val = (val << 8) | get_hex8(&txt[i * 2]);
	return val;
}


int verify_checksum(char *data, int len)
{
	int i;
	unsigned char c, c2;

	c = 0;
	for (i = 0; i < len; i++)
		c += get_hex8(&data[i * 2]);
	c = 0xff - c;
	c2 = get_hex8(&data[len * 2]);
	if (c != c2)
		return 1;

	return 0;
}

void enlarge_s3_buf(struct srec_data *srec)
{
	srec->s3_records = (struct s3_record *)
		realloc(srec->s3_records,
			2 * srec->s3_entries * sizeof(struct s3_record));
	if (srec->s3_records == NULL) {
		printf("Could not allocate memory for S3 records.\n");
		exit(1);
	}
	memset(&srec->s3_records[srec->s3_entries], 0,
	       srec->s3_entries * sizeof(struct s3_record));
	srec->s3_entries *= 2;
}

int s3_combine(struct srec_data *srec)
{
	int from, to, removed;

	from = to = removed = 0;
	while (from < srec->s3_used) {
		struct s3_record *_to, *_to_1, *_from;
		_to = &srec->s3_records[to];
		_to_1 = to > 0 ? &srec->s3_records[to - 1] : NULL;
		_from = &srec->s3_records[from];

		if (to != 0 && _to_1->addr + _to_1->len == _from->addr) {
			_to_1->data = (unsigned char *)
				realloc(_to_1->data, _to_1->len + _from->len);
			if (_to_1->data == NULL) {
				printf("realloc() failed\n");
				return 1;
			}
			memcpy(_to_1->data + _to_1->len, _from->data,
			       _from->len);
			free(_from->data);
			_from->data = NULL;
			_to_1->len += _from->len;
			removed++;
		} else {
			if (from != to)
				memcpy(_to, _from, sizeof(struct s3_record));
			to++;
		}
		from++;
	}

	srec->s3_used -= removed;

	if (srec->s3_entries > srec->s3_used) {
		srec->s3_entries = srec->s3_used;
		srec->s3_records = (struct s3_record *)
			realloc(srec->s3_records,
				srec->s3_entries * sizeof(struct s3_record));
		if (srec->s3_records == NULL) {
			printf("realloc() failed\n");
			return 1;
		}
	}

	return 0;
}


int process_s3_plug(struct srec_data *srec, char *data, int len,
		    unsigned int addr)
{
	struct s3_plug_record *p;

	if (len != 3 * 4) {
		printf("Unknown S3 plug record length %d\n", len);
		return 1;
	}

	srec->plugs = (struct s3_plug_record *)
		realloc(srec->plugs, (srec->plug_count + 1) *
			sizeof(struct s3_plug_record));
	assert(srec->plugs != NULL);

	p = &srec->plugs[srec->plug_count];
	srec->plug_count++;
	p->pdr = get_hex32_le(data);
	p->plug_addr = get_hex32_le(data + 8);
	p->plug_len = get_hex32_le(data + 16);
	verbose_printf("S3 plug record: PDR=0x%04x plug: addr=0x%08X len=%i\n",
		       p->pdr, p->plug_addr, p->plug_len);

	return 0;
}


int process_s3_crc16(struct srec_data *srec, char *data, int len,
		     unsigned int addr)
{
	struct s3_crc16 *c;

	if (len != 3 * 4) {
		printf("Unknown S3 CRC-16 generation record length %d\n", len);
		return 1;
	}

	srec->crc16 = (struct s3_crc16 *)
		realloc(srec->crc16,
			(srec->crc16_count + 1) * sizeof(struct s3_crc16));
	assert(srec->crc16 != NULL);
	c = &srec->crc16[srec->crc16_count];
	srec->crc16_count++;

	c->start = get_hex32_le(data);
	c->len = get_hex32_le(data + 8);
	c->prog = get_hex32_le(data + 16);
	verbose_printf("S3 CRC-16 generation record: start=0x%08X "
		       "len=%d prog=%d\n",
		       c->start, c->len, c->prog);

	return 0;
}


static int process_s3_fw_info_name(struct srec_data *srec, char *data, int len,
				   unsigned short words)
{
	int i;

	srec->name_rec = (char *) malloc(len + 1);
	assert(srec->name_rec != NULL);
	memset(srec->name_rec, 0, len + 1);

	for (i = 0; i < len; i++) {
		int c;
		c = get_hex8(data + 2 * i);
		if (c == 0)
			break;
		if (c >= 32 && c < 127)
			srec->name_rec[i] = c;
		else
			srec->name_rec[i] = '_';
	}
	verbose_printf("Special S3 firmware info record - name '%s'\n",
		       srec->name_rec);
	return 0;
}


static int process_s3_fw_info_ver(struct srec_data *srec, char *data,
				  int words)
{
	if (words != 4) {
		printf("Invalid S3 f/w info: version info (words=%i)\n",
		       words);
		return 1;
	}
	if (srec->component_set) {
		printf("More than one S3 f/w info component?\n");
		return 1;
	}
	srec->component_set++;
	srec->component.component = get_hex16_le(data);
	srec->component.variant = get_hex16_le(data + 4);
	srec->component.major = get_hex16_le(data + 8);
	srec->component.minor = get_hex16_le(data + 12);
	verbose_printf("S3 f/w info: version: component=0x%04x %i.%i.%i\n",
		       srec->component.component, srec->component.major,
		       srec->component.minor, srec->component.variant);
	return 0;
}


static int process_s3_fw_info_comp(struct srec_data *srec, char *data,
				   int words)
{
	struct s3_info_compatibility *r;
	int role, iface_id;

	if (words < 5 || (words - 2) % 3) {
		printf("Invalid S3 firmware info: compatibility info "
		       "(words=%i)\n", words);
		return 1;
	}

	role = get_hex16_le(data);
	iface_id = get_hex16_le(data + 4);
	data += 2 * 4;
	words -= 2;

	while (words >= 3) {
		srec->compatibilities = (struct s3_info_compatibility *)
			realloc(srec->compatibilities,
				(srec->compatibility_count + 1) *
				sizeof(struct s3_info_compatibility));
		assert(srec->compatibilities != NULL);

		r = &srec->compatibilities[srec->compatibility_count];
		srec->compatibility_count++;
		r->role = role;
		r->iface_id = iface_id;
		r->variant = get_hex16_le(data);
		r->bottom = get_hex16_le(data + 4);
		r->top = get_hex16_le(data + 8);
		data += 3 * 4;
		words -= 3;
		verbose_printf("S3 f/w info: compatibility: role=0x%04x "
			       "iface_id=0x%04x variant=%i range=%i-%i\n",
			       r->role, r->iface_id, r->variant,
			       r->bottom, r->top);
	}

	return 0;
}


static int process_s3_fw_info_seq(struct srec_data *srec, char *data,
				  int words)
{
	unsigned short seq;

	if (words != 1) {
		printf("Invalid S3 firmware info: "
		       "build sequence words: %i\n", words);
		return 1;
	}
	seq = get_hex16_le(data);
	verbose_printf("S3 f/w info: build seq#: %i\n", seq);
	return 0;
}


static int process_s3_fw_info_platform(struct srec_data *srec, char *data,
				       int words)
{
	struct s3_info_platform *p;

	if (words != 4) {
		printf("Invalid S3 firmware info: "
		       "platform info words: %i\n", words);
		return 1;
	}

	srec->platforms = (struct s3_info_platform *)
		realloc(srec->platforms, (srec->platform_count + 1) *
			sizeof(struct s3_info_platform));
	assert(srec->platforms != NULL);

	p = &srec->platforms[srec->platform_count];
	srec->platform_count++;
	p->platform = get_hex16_le(data);
	p->variant = get_hex16_le(data + 4);
	p->major = get_hex16_le(data + 8);
	p->minor = get_hex16_le(data + 12);
	verbose_printf("S3 f/w info: platform: id=0x%04x %u.%u.%u\n",
		       p->platform, p->major, p->minor, p->variant);

	return 0;
}


int process_s3_fw_info(struct srec_data *srec, char *data, int len,
		       unsigned int addr)
{
	unsigned short words, type;
	int ret = 0;

	if (addr != srec->s3_fw_info_addr) {
		printf("Invalid S3 firmware info start addr 0x%x (expected "
		       "0x%x)\n", addr, srec->s3_fw_info_addr);
		srec->s3_fw_info_addr = S3_FW_INFO_ADDR;
		return 1;
	}

	if (addr == S3_FW_INFO_ADDR) {
		if (srec->s3_fw_info_buf) {
			printf("Unfinished S3 firmware info\n");
			return 1;
		}
		if (len < 4) {
			printf("Too short S3 firmware info (len %i)\n", len);
			printf("S3 line: %s\n", data);
			return 1;
		}
		words = get_hex16_le(data);
		type = get_hex16_le(data + 4);
	} else {
		if (srec->s3_fw_info_buf == NULL) {
			printf("Did not see first fragment of S3 firmware "
			       "info record\n");
			return -1;
		}
		words = get_hex16_le(srec->s3_fw_info_buf);
		type = get_hex16_le(srec->s3_fw_info_buf + 4);
	}

	if (addr != S3_FW_INFO_ADDR ||
	    len + srec->s3_fw_info_buf_len < words * 2) {
		/* S3 f/w info record spanning multiple S3 records */
		char *newbuf;
		newbuf = realloc(srec->s3_fw_info_buf,
				 (srec->s3_fw_info_buf_len + len) * 2);
		if (newbuf == NULL) {
			printf("Failed to allocate memory for S3 f/w info "
			       "record\n");
			return 1;
		}
		memcpy(newbuf + srec->s3_fw_info_buf_len * 2, data, len * 2);
		srec->s3_fw_info_buf = newbuf;
		srec->s3_fw_info_buf_len += len;
		srec->s3_fw_info_addr = addr + len;
		if (srec->s3_fw_info_buf_len < words * 2)
			return 0; /* continued in the next S3 record */

		/* Full S3 f/w info record has now been reassembled */
		data = srec->s3_fw_info_buf;
		len = srec->s3_fw_info_buf_len;
	}

	/* remove words and type fields */
	data += 8;
	len -= 4;

	/* silly, but this special record has different logic in words field */
	if (type != 0x8001)
		words--;

	if (words * 2 != len) {
		printf("Invalid S3 firmware info len: len=%i words=%i "
		       "type=0x%x\n", len, words, type);
		printf("S3 line: %s\n", data);
		return 1;
	}

	/* Reset next S3 f/w info address into default value */
	srec->s3_fw_info_addr = S3_FW_INFO_ADDR;

	switch (type) {
	case 1:
		ret = process_s3_fw_info_ver(srec, data, words);
		break;
	case 2:
		ret = process_s3_fw_info_comp(srec, data, words);
		break;
	case 3:
		ret = process_s3_fw_info_seq(srec, data, words);
		break;
	case 4:
		ret = process_s3_fw_info_platform(srec, data, words);
		break;
	case 0x8001:
		ret = process_s3_fw_info_name(srec, data, len, words);
		break;
	default:
		printf("Unknown S3 firmware info type %i\n", type);
		ret = 1;
		break;
	}

	free(srec->s3_fw_info_buf);
	srec->s3_fw_info_buf = NULL;
	srec->s3_fw_info_buf_len = 0;
	return ret;
}


#define MAX_S3_LEN 128

int process_s3_data(struct srec_data *srec, char *data, int len,
		    unsigned int addr)
{
	unsigned char buf[MAX_S3_LEN];
	int i;

	for (i = 0; i < len; i++)
		buf[i] = get_hex8(data + i * 2);

	if (srec->s3_used > 0 &&
	    srec->s3_records[srec->s3_used - 1].addr +
	    srec->s3_records[srec->s3_used - 1].len == addr) {
		/* combine with previous S3 record */
		srec->s3_records[srec->s3_used - 1].data = (unsigned char *)
			realloc(srec->s3_records[srec->s3_used - 1].data,
				srec->s3_records[srec->s3_used - 1].len + len);
		if (srec->s3_records[srec->s3_used - 1].data == NULL) {
			printf("realloc() failed\n");
			return 1;
		}
		memcpy(srec->s3_records[srec->s3_used - 1].data +
		       srec->s3_records[srec->s3_used - 1].len, buf, len);
		srec->s3_records[srec->s3_used - 1].len += len;
	} else {
		if (srec->s3_used + 1 == srec->s3_entries)
			enlarge_s3_buf(srec);
		srec->s3_records[srec->s3_used].addr = addr;
		srec->s3_records[srec->s3_used].len = len;
		srec->s3_records[srec->s3_used].data = (unsigned char *)
			malloc(len);
		if (srec->s3_records[srec->s3_used].data == NULL) {
			printf("malloc() failed\n");
			return 1;
		}
		memcpy(srec->s3_records[srec->s3_used].data, buf, len);
		srec->s3_used++;
	}

	return 0;
}


int process_s3(struct srec_data *srec, char *data, int len)
{
	unsigned int addr;

	if (len < 4) {
		printf("Too short S3 len=%i\n", len);
		return 1;
	}

	if (len - 4 > MAX_S3_LEN) {
		printf("Too long S3 len=%i\n", len);
		return 1;
	}

	addr = get_hex32(data);
	len -= 4;
	data += 8;

	if (addr & S3_SPECIAL_MASK) {
		/* special S3 records */
		if (addr == S3_PLUG_ADDR) {
			if (process_s3_plug(srec, data, len, addr))
				return 1;
		} else if (addr == S3_CRC16_ADDR) {
			if (process_s3_crc16(srec, data, len, addr))
				return 1;
		} else if ((addr & S3_FW_INFO_ADDR_MASK) == S3_FW_INFO_ADDR) {
			if (process_s3_fw_info(srec, data, len, addr))
				return 1;
		} else {
			printf("Unknown special S3 record: '%s'\n", data);
			return 1;
		}
	} else {
		/* normal S3 record */
		if (process_s3_data(srec, data, len, addr))
			return 1;
	}

	return 0;
}

int process_s7(struct srec_data *srec, char *data, int len)
{
	int i;
	if (len != 4) {
		printf("Invalid S7 length %i (expected 4)\n", len);
		return 1;
	}

	srec->start_addr = 0;
	for (i = 0; i < 4; i++) {
		srec->start_addr = (srec->start_addr << 8) |
			(hex2int(data[i * 2]) << 4) |
			hex2int(data[1 + i * 2]);
	}
	srec->start_addr_set = 1;
	verbose_printf("Start address 0x%08x\n", srec->start_addr);

	return 0;
}


void free_srec(struct srec_data *srec)
{
	int i;

	if (srec == NULL)
		return;

	if (srec->s3_records != NULL) {
		for (i = 0; i < srec->s3_used; i++)
			free(srec->s3_records[i].data);
		free(srec->s3_records);
	}

	free(srec->platforms);
	free(srec->compatibilities);
	free(srec->crc16);
	free(srec->plugs);
	free(srec->name);
	free(srec->name_rec);
	free(srec->s3_fw_info_buf);
	free(srec);
}

struct srec_data * read_srec(const char *fname)
{
	FILE *f;
	char buf[1024];
	const char *pos;
	int i, len, slen, total_len;
	int errors = 0, line = 0;
	struct srec_data *srec;

	f = fopen(fname, "r");
	if (f == NULL) {
		printf("'%s' not readable.\n", fname);
		return NULL;
	}

	srec = (struct srec_data *) malloc(sizeof(struct srec_data));
	if (srec == NULL) {
		fclose(f);
		return NULL;
	}
	memset(srec, 0, sizeof(struct srec_data));
	srec->s3_fw_info_addr = S3_FW_INFO_ADDR;

	pos = strrchr(fname, '/');
	if (pos == NULL)
		pos = fname;
	else
		pos++;
	srec->name = strdup(pos);

	srec->s3_used = 0;
	srec->s3_entries = 1024;
	srec->s3_records = (struct s3_record *)
		malloc(srec->s3_entries * sizeof(struct s3_record));
	if (srec->s3_records == NULL) {
		printf("Could not allocate memory for S3 records.\n");
		fclose(f);
		free(srec);
		return NULL;
	}
	memset(srec->s3_records, 0,
	       srec->s3_entries * sizeof(struct s3_record));

	while (fgets(buf, sizeof(buf), f) != NULL) {
		line++;
		buf[sizeof(buf) - 1] = '\0';
		for (i = 0; i < sizeof(buf); i++) {
			if (buf[i] == '\r' || buf[i] == '\n') {
				buf[i] = '\0';
			}
			if (buf[i] == '\0')
				break;
		}
		if (buf[0] == '\0')
			continue;

		if (buf[0] != 'S' || buf[1] == '\0') {
			fprintf(stderr, "Skipped unrecognized line %i: '%s'\n",
				line, buf);
			errors++;
			continue;
		}

		len = get_hex8(buf + 2);
		slen = strlen(buf + 2);
		if (len < 1 || slen & 1 || len + 1 != slen / 2) {
			printf("Invalid line %i length (len=%d slen=%d) "
			       "'%s'\n", line, len, slen, buf);
		}

		if (verify_checksum(buf + 2, len)) {
			printf("line %i: checksum failure\n", line);
			errors++;
			continue;
		}

		buf[slen] = '\0';

		switch (buf[1]) {
		case '3':
			if (process_s3(srec, buf + 4, len - 1)) {
				printf("S3 parse error; line=%d\n", line);
				errors++;
			}
			break;
		case '7':
			if (process_s7(srec, buf + 4, len - 1)) {
				printf("S7 parse error; line=%d\n", line);
				errors++;
			}
			break;
		default:
			fprintf(stderr, "Skipped unrecognized S-line %i: "
				"'%s'\n", line, buf);
			errors++;
			break;
		}
	}

	fclose(f);

	if (srec->s3_fw_info_buf) {
		fprintf(stderr, "Unfinished S3 firmware info\n");
		errors++;
	}

	verbose_printf("Before S3 sort\n");
	verbose_printf("S3 area count: %i\n", srec->s3_used);
	for (i = 0; i < srec->s3_used; i++)
		verbose_printf("  addr=0x%08X..0x%08X (len=%i)\n",
			       srec->s3_records[i].addr,
			       srec->s3_records[i].addr +
			       srec->s3_records[i].len - 1,
			       srec->s3_records[i].len);

	qsort(srec->s3_records, srec->s3_used, sizeof(struct s3_record),
	      s3_comp);
	if (s3_combine(srec))
		errors++;

	verbose_printf("\nAfter S3 sort\n");
	verbose_printf("S3 area count: %i\n", srec->s3_used);
	total_len = 0;
	for (i = 0; i < srec->s3_used; i++) {
		verbose_printf("  addr=0x%08X..0x%08X (len=%i)\n",
			       srec->s3_records[i].addr,
			       srec->s3_records[i].addr +
			       srec->s3_records[i].len - 1,
			       srec->s3_records[i].len);
		total_len += srec->s3_records[i].len;
	}
	verbose_printf("Total data length: %i\n", total_len);

	if (errors > 0) {
		printf("Errors found - file cannot be used\n");
		free_srec(srec);
		return NULL;
	}

	return srec;
}

enum { ROLE_SUPPLIER = 0, ROLE_ACTOR = 1 };
enum { IFACE_MF = 1, IFACE_CF = 2, IFACE_PRI = 3, IFACE_STA = 4 };

void show_compatibility(struct s3_info_compatibility *r, const char *prefix)
{
	const char *iface_id_str;
	switch (r->iface_id) {
	case IFACE_MF: iface_id_str = "Modem-Firmware"; break;
	case IFACE_CF: iface_id_str = "Controller-Firmware"; break;
	case IFACE_PRI: iface_id_str = "Primary Firmware-Driver"; break;
	case IFACE_STA: iface_id_str = "Station Firmware-Driver"; break;
	default: iface_id_str = "??"; break;
	}
	printf("  %srole=%s variant=%d range=%d-%d iface=%s (%d)\n", prefix,
	       r->role == ROLE_SUPPLIER ? "Supplier" : "Actor   ",
	       r->variant, r->bottom, r->top,
	       iface_id_str, r->iface_id);
}


void show_srec(struct srec_data *srec)
{
	int i, total_len;

	printf("srec summary for %s\n", srec->name);

	if (srec->name_rec)
		printf("Included file name: %s\n", srec->name_rec);

	printf("Component: 0x%04x %i.%i.%i",
	       srec->component.component,
	       srec->component.major,
	       srec->component.minor,
	       srec->component.variant);
	switch (srec->component.component) {
	case HFA384X_COMP_ID_PRI:
		printf(" (primary firmware)");
		break;
	case HFA384X_COMP_ID_STA:
		printf(" (station firmware)");
		break;
	case HFA384X_COMP_ID_FW_AP:
		printf(" (tertiary firmware)");
		break;
	}
	printf("\n");

	printf("Supported platforms:\n");
	for (i = 0; i < srec->platform_count; i++) {
		struct s3_info_platform *p = &srec->platforms[i];
		printf("  0x%04x %u.%u.%u", p->platform, p->major, p->minor,
		       p->variant);
		if (i == srec->platform_count - 1 || i % 5 == 4)
			printf("\n");
		else
			printf(",");
	}

	if (verbose) {
		printf("Interface compatibility information:\n");
		for (i = 0; i < srec->compatibility_count; i++)
			show_compatibility(&srec->compatibilities[i], "");
	}

	printf("Separate S3 data areas:\n");
	total_len = 0;
	for (i = 0; i < srec->s3_used; i++) {
		printf("  addr 0x%08X..0x%08X (len %i)\n",
		       srec->s3_records[i].addr,
		       srec->s3_records[i].addr + srec->s3_records[i].len - 1,
		       srec->s3_records[i].len);
		total_len += srec->s3_records[i].len;
	}
	printf("Total data length: %i bytes\n", total_len);
	if (srec->start_addr_set)
		printf("Start address 0x%08x\n", srec->start_addr);
}


static int get_compid(const char *dev, u16 rid, struct s3_info_platform *p)
{
	char buf[PRISM2_HOSTAPD_MAX_BUF_SIZE];
	struct prism2_hostapd_param *param;
	struct hfa384x_comp_ident *comp;

	param = (struct prism2_hostapd_param *) buf;
	comp = (struct hfa384x_comp_ident *) param->u.rid.data;

	if (hostapd_get_rid(dev, param, rid, 1) ||
	    param->u.rid.len != sizeof(*comp))
		return -1;

	p->platform = le_to_host16(comp->id);
	p->major = le_to_host16(comp->major);
	p->minor = le_to_host16(comp->minor);
	p->variant = le_to_host16(comp->variant);

	return 0;
}


static int get_range(const char *dev, u16 rid, struct s3_info_compatibility *r)
{
	char buf[PRISM2_HOSTAPD_MAX_BUF_SIZE];
	struct prism2_hostapd_param *param;
	struct hfa384x_sup_range *range;

	param = (struct prism2_hostapd_param *) buf;
	range = (struct hfa384x_sup_range *) param->u.rid.data;

	if (hostapd_get_rid(dev, param, rid, 1) ||
	    param->u.rid.len != sizeof(*range))
		return -1;

	r->role = le_to_host16(range->role);
	r->iface_id = le_to_host16(range->id);
	r->variant = le_to_host16(range->variant);
	r->bottom = le_to_host16(range->bottom);
	r->top = le_to_host16(range->top);

	return 0;
}


int read_wlan_rids(const char *dev, struct wlan_info *wlan)
{
	if (get_compid(dev, HFA384X_RID_NICID, &wlan->nicid) ||
	    get_compid(dev, HFA384X_RID_PRIID, &wlan->priid) ||
	    get_compid(dev, HFA384X_RID_STAID, &wlan->staid)) {
		printf("Missing wlan component info\n");
		return 1;
	}

	get_range(dev, HFA384X_RID_PRISUPRANGE, &wlan->pri_sup);
	get_range(dev, HFA384X_RID_CFIACTRANGES, &wlan->cfi_pri_act);
	get_range(dev, HFA384X_RID_MFISUPRANGE, &wlan->mfi_pri_sup);
	get_range(dev, HFA384X_RID_CFISUPRANGE, &wlan->cfi_pri_sup);
	get_range(dev, HFA384X_RID_STASUPRANGE, &wlan->sta_sup);
	get_range(dev, HFA384X_RID_MFIACTRANGES, &wlan->mfi_sta_act);
	get_range(dev, HFA384X_RID_CFIACTRANGES, &wlan->cfi_sta_act);

	return 0;
}


int read_wlan_pda(const char *fname, struct wlan_info *wlan)
{
	FILE *f;
	int pos;
	u16 *pda, len, pdr;

	f = fopen(fname, "r");
	if (f == NULL)
		return 1;

	if (fread(wlan->pda_buf, 1, PRISM2_PDA_SIZE, f) != PRISM2_PDA_SIZE) {
		fclose(f);
		return 1;
	}

	fclose(f);


	pda = (u16 *) wlan->pda_buf;
	pos = 0;
	while (pos + 1 < PRISM2_PDA_SIZE / 2) {
		len = le_to_host16(pda[pos]);
		pdr = le_to_host16(pda[pos + 1]);
		if (len == 0 || pos + len > PRISM2_PDA_SIZE / 2)
			return 1;

		wlan->pdrs = (struct pdr *)
			realloc(wlan->pdrs,
				(wlan->pdr_count + 1) * sizeof(struct pdr));
		assert(wlan->pdrs != NULL);
		wlan->pdrs[wlan->pdr_count].pdr = pdr;
		wlan->pdrs[wlan->pdr_count].len = (len - 1) * 2;
		wlan->pdrs[wlan->pdr_count].data =
			(unsigned char *) (&pda[pos + 2]);
		wlan->pdr_count++;

		if (pdr == 0x0000 && len == 2) {
			/* PDA end found */
			if (crc16(wlan->pda_buf, (pos + 3) * 2) != 0)
			{
				printf("PDA checksum incorrect.\n");
				return 1;
			}
			return 0;
		}

		pos += len + 1;
	}

	return 1;
}


void show_wlan(struct wlan_info *wlan)
{
	printf("Wireless LAN card information:\n");
	printf("Components:\n");
	printf("  NICID: 0x%04x v%u.%u.%u\n",
	       wlan->nicid.platform, wlan->nicid.major,
	       wlan->nicid.minor, wlan->nicid.variant);
	printf("  PRIID: 0x%04x v%u.%u.%u\n",
	       wlan->priid.platform, wlan->priid.major,
	       wlan->priid.minor, wlan->priid.variant);
	printf("  STAID: 0x%04x v%u.%u.%u\n",
	       wlan->staid.platform, wlan->staid.major,
	       wlan->staid.minor, wlan->staid.variant);
	if (verbose) {
		printf("Interface compatibility information:\n");
		show_compatibility(&wlan->mfi_pri_sup, "PRI ");
		show_compatibility(&wlan->cfi_pri_sup, "PRI ");
		show_compatibility(&wlan->pri_sup, "PRI ");
		show_compatibility(&wlan->sta_sup, "STA ");
		show_compatibility(&wlan->cfi_pri_act, "PRI ");
		show_compatibility(&wlan->cfi_sta_act, "STA ");
		show_compatibility(&wlan->mfi_sta_act, "STA ");
	}

	if (verbose > 1) {
		int i, j;
		for (i = 0; i < wlan->pdr_count; i++) {
			printf("PDR 0x%04x len=%i:",
			       wlan->pdrs[i].pdr, wlan->pdrs[i].len);
			for (j = 0; j < wlan->pdrs[i].len; j++)
				printf(" %02x", wlan->pdrs[i].data[j]);
			printf("\n");
		}
	}
}


void free_wlan(struct wlan_info *wlan)
{
	if (wlan->pdrs)
		free(wlan->pdrs);
}


unsigned char *get_s3_data_pos(struct srec_data *srec, unsigned int addr,
			       unsigned int len)
{
	int i;

	for (i = 0; i < srec->s3_used; i++) {
		struct s3_record *s = &srec->s3_records[i];

		if (s->addr <= addr && s->addr + s->len > addr) {
			if (s->len - (addr - s->addr) < len) {
				printf("Plug record (addr=0x%08x, len=%u) not "
				       "within data area\n",
				       addr, len);
				return NULL;
			}
			return s->data + (addr - s->addr);
		}
	}

	return NULL;
}


static int supported_platform(struct s3_info_platform *nicid,
			      struct srec_data *srec, int nv_dl)
{
	int i;

	for (i = 0; i < srec->platform_count; i++) {
		struct s3_info_platform *p = &srec->platforms[i];

		if (p->platform == nicid->platform &&
		    p->major == nicid->major &&
		    p->minor == nicid->minor &&
		    p->variant == nicid->variant)
			return 1;
	}

	/* NICID was not found - reported compatability, if it is known to work
	 * and non-volatile download is not used */
	if (nv_dl)
		return 0;

	for (i = 0; i < srec->platform_count; i++) {
		struct s3_info_platform *p = &srec->platforms[i];

		/* Known to work:
		 * NICID 0x8008 v1.0.1 (D-Link DWL-650) with supported platform
		 * 0x8008 v1.0.0 */

		if (nicid->platform == 0x8008 && nicid->major == 1 &&
		    nicid->minor == 0 && nicid->variant == 1 &&
		    p->platform == 0x8008 && p->major == 1 &&
		    p->minor == 0 && p->variant == 0) {
			printf("Exact NICID was not found from the list of "
			       "supported platforms, but an\nalternative that "
			       "has been reported to work was found.\n");
			return 1;
		}
	}

	return 0;
}


int combine_info(struct wlan_info *wlan, struct srec_data *srec,
		 int nv_dl, int v_dl)
{
	int i, j, found;

	/* verify that the platform is supported */
	if (!supported_platform(&wlan->nicid, srec, nv_dl)) {
		printf("NICID was not found from the list of supported "
		       "platforms.\n");
		return 1;
	}

	for (i = 0; i < srec->compatibility_count; i++) {
		struct s3_info_compatibility *sr, *wr;
		sr = &srec->compatibilities[i];
		if (sr->role == ROLE_SUPPLIER)
			continue;
		if (sr->role != ROLE_ACTOR) {
			printf("Unknown interface compatibility role %d\n",
			       sr->role);
			return 1;
		}

		switch (sr->iface_id) {
		case IFACE_MF: wr = &wlan->mfi_pri_sup; break;
		case IFACE_CF: wr = &wlan->cfi_pri_sup; break;
		case IFACE_PRI: wr = &wlan->pri_sup; break;
		case IFACE_STA: wr = &wlan->sta_sup; break;
		default:
			printf("Unknown interface compatibility id %d\n",
			       sr->iface_id);
			return 1;
		}

		if (sr->variant != wr->variant ||
		    sr->top < wr->bottom || sr->bottom > wr->top) {
			printf("Incompatible interfaces:\n");
			show_compatibility(sr, "SREC: ");
			show_compatibility(wr, "card: ");
			if (ignore_incompatible_interface)
				printf("Ignoring incompatibility\n");
			else
				return 1;
		}
	}

	for (i = 0; i < srec->plug_count; i++) {
		struct s3_plug_record *p = &srec->plugs[i];
		unsigned char *pos;
		verbose_printf("Plugging PDR 0x%04x at 0x%08x (len=%d)\n",
			       p->pdr, p->plug_addr, p->plug_len);
		pos = get_s3_data_pos(srec, p->plug_addr, p->plug_len);
		if (pos == NULL)
			return 1;

		if (p->pdr == 0xffffffff) {
			/* Special PDR reserved for an ASCIIZ string
			 * (available from RID FFFF); like file name for
			 * upgrade packet, etc. */
			int len;
			memset(pos, 0, p->plug_len);
			len = strlen(srec->name);
			if (p->plug_len > 0)
				memcpy(pos, srec->name,
				       len > p->plug_len - 1 ? p->plug_len - 1
				       : len);
			continue;
		}

		found = 0;
		for (j = 0; j < wlan->pdr_count; j++) {
			if (wlan->pdrs[j].pdr == p->pdr) {
				if (wlan->pdrs[j].len != p->plug_len) {
					printf("Plug record length mismatch "
					       "(PDR=0x%04x): %i != %i\n",
					       p->pdr, wlan->pdrs[j].len,
					       p->plug_len);
					if (ignore_incompatible_interface) {
						printf("==> use default\n");
						break;
					} else if (p->pdr == 1 &&
						   wlan->pdrs[j].len <
						   p->plug_len) {
						/* PDR 0x0001 - 'manufacturing
						 * part number' seems to be
						 * smaller on some PDAs; just
						 * enlarge it using default */
						printf("==> extend from "
						       "default\n");
					} else
						return 1;
				}
				memcpy(pos, wlan->pdrs[j].data,
				       wlan->pdrs[j].len);
				found = 1;
				break;
			}
		}
		if (!found) {
			int j;
			printf("PDR 0x%04x not found from wlan card PDA. "
			       "Using default data.\n  len=%i:", p->pdr,
			       p->plug_len);
			for (j = 0; j < p->plug_len; j++)
				printf(" %02x", pos[j]);
			printf("\n");
		}
	}

	for (i = 0; i < srec->crc16_count; i++) {
		struct s3_crc16 *c = &srec->crc16[i];
		if (c->prog) {
			struct s3_record *s = NULL;
			verbose_printf("Generating CRC-16 (start=0x%08x, "
				       "len=%d) at 0x%08x\n", c->start, c->len,
				       c->start - 2);
			/* Note! CRC-16 support is not implemented, but primary
			 * firmware expects that secondary firmware has
			 * signature stamp 0xC0DE at the address location of
			 * CRC-16 value (just before c->start). */
			found = 0;
			for (j = 0; j < srec->s3_used; j++) {
				s = &srec->s3_records[j];
				if (s->addr == c->start) {
					found = 1;
					break;
				}
			}
			if (!found) {
				printf("Could not found proper place for "
				       "CRC-16\n");
				return 1;
			}

			s->data = realloc(s->data, s->len + 2);
			assert(s->data != NULL);
			memmove(s->data + 2, s->data, s->len);
			s->addr -= 2;
			s->len += 2;
			s->data[0] = 0xDE;
			s->data[1] = 0xC0;
		}
	}

	if (v_dl && srec->start_addr == 0) {
		printf("\nThis image is not meant to be downloaded to "
		       "volatile memory.\n");
		return 1;
	}

	if (nv_dl && srec->start_addr != 0) {
		printf("\nThis image is not meant to be downloaded to "
		       "non-volatile memory.\n");
		return 1;
	}

	return 0;
}


void dump_s3_data(struct srec_data *srec, const char *fname)
{
	FILE *f;
	int i;

	f = fopen(fname, "w");
	if (f == NULL)
		return;

	printf("Writing image dump into '%s'\n", fname);

	for (i = 0; i < srec->s3_used; i++) {
		fseek(f, srec->s3_records[i].addr, SEEK_SET);
		fwrite(srec->s3_records[i].data, srec->s3_records[i].len, 1,
		       f);
	}

	fclose(f);
}


int download_srec(const char *iface, struct srec_data *srec, int non_volatile)
{
	struct prism2_download_param *param;
	int plen, i, ret = 0, s = -1;
	unsigned int total_len;
	struct iwreq iwr;

	plen = sizeof(struct prism2_download_param) +
		srec->s3_used * sizeof(struct prism2_download_area);
	param = (struct prism2_download_param *) malloc(plen);
	assert(param != NULL);
	memset(param, 0, plen);

	param->dl_cmd = non_volatile ? PRISM2_DOWNLOAD_NON_VOLATILE :
		PRISM2_DOWNLOAD_VOLATILE;
	param->start_addr = srec->start_addr;
	param->num_areas = srec->s3_used;

	total_len = 0;
	for (i = 0; i < srec->s3_used; i++) {
		if (srec->s3_records[i].len > PRISM2_MAX_DOWNLOAD_AREA_LEN) {
			printf("Too large data area.\n");
			ret = 1;
			goto out;
		}
		param->data[i].addr = srec->s3_records[i].addr;
		param->data[i].len = srec->s3_records[i].len;
		param->data[i].ptr = srec->s3_records[i].data;
		total_len += srec->s3_records[i].len;
	}
	if (total_len > PRISM2_MAX_DOWNLOAD_LEN) {
		printf("Too large total download length.\n");
		ret = 1;
		goto out;
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		ret = 1;
		goto out;
	}

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, iface, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) param;
	iwr.u.data.length = plen;

	if (ioctl(s, PRISM2_IOCTL_DOWNLOAD, &iwr) < 0) {
		if (errno == EOPNOTSUPP) {
			/* PDA read should have already failed if
			 * PRISM2_DOWNLOAD_SUPPORT is not defined */
			printf("Odd.. Download request for the kernel driver "
			       "failed.\n"
			       "Are you sure you have compiled (and loaded "
			       "the correct version of)\n"
			       "hostap.o module with "
			       "PRISM2_DOWNLOAD_SUPPORT definition in\n"
			       "driver/module/hostap_config.h?\n");
		}
		perror("ioctl[PRISM2_IOCTL_DOWNLOAD]");
		ret = 1;
		goto out;
	}

 out:
	if (s >= 0)
		close(s);
	free(param);
	return ret;
}


void usage(void)
{
	printf("Usage: prism2_srec [-vvrfd] <interface> <srec file name>\n"
	       "Options:\n"
	       "  -v   verbose (add another for more verbosity\n"
	       "  -r   download SREC file into RAM (volatile)\n"
	       "  -f   download SREC file into flash (non-volatile)\n"
	       "  -d   dump SREC image into prism2_srec.dump\n"
	       "  -i   ignore incompatible interfaces errors\n"
	       "       Warning! This can result in failed upgrade!\n\n"
	       "Options -r and -f cannot be used together.\n"
	       "If -r or -f is not specified, image summary is shown and\n"
	       "compatibility with WLAN card is verified without downloading\n"
	       "anything.\n");

	exit(1);
}


int main(int argc, char *argv[])
{
	struct srec_data *srec;
	struct wlan_info wlan;
	int opt, ret;
	const char *iface, *srec_fname;
	char fname[256];
	int dump_image_data = 0;
	int volatile_download = 0;
	int non_volatile_download = 0;
	int show_after = 0;

	if (argc < 3)
		usage();

	for (;;) {
		opt = getopt(argc, argv, "vrfdi");
		if (opt < 0)
			break;
		switch (opt) {
		case 'v': verbose++; break;
		case 'r': volatile_download++; break;
		case 'f': non_volatile_download++; break;
		case 'd': dump_image_data++; break;
		case 'i': ignore_incompatible_interface++; break;
		default:
			usage();
			break;
		}
	}
	if (non_volatile_download && ignore_incompatible_interface)
		ignore_incompatible_interface = 0;
	if (volatile_download && non_volatile_download)
		usage();
	if (argc - optind != 2)
		usage();
	iface = argv[optind++];
	srec_fname = argv[optind++];

	srec = read_srec(srec_fname);
	if (srec == NULL) {
		printf("Parsing '%s' failed.\n", srec_fname);
		exit(1);
	}

	show_srec(srec);

	printf("\n");
	memset(&wlan, 0, sizeof(wlan));

	if (read_wlan_rids(iface, &wlan)) {
		printf("Could not read wlan RIDs\n");
		exit(1);
	}

	snprintf(fname, sizeof(fname), "/proc/net/hostap/%s/pda", iface);
	if (read_wlan_pda(fname, &wlan)) {
		printf("Could not read wlan PDA. This requires "
		       "PRISM2_DOWNLOAD_SUPPORT definition in\n"
		       "driver/module/hostap_config.h.\n");
		exit(1);
	}

	show_wlan(&wlan);

	printf("\nVerifying update compatibility and combining data:\n");
	if (combine_info(&wlan, srec, non_volatile_download,
			 volatile_download)) {
		printf("Incompatible updata data.\n");
		exit(1);
	}
	printf("OK.\n");

	ret = 0;
	if (volatile_download) {
		printf("\nDownloading to volatile memory (RAM).\n");
		if (download_srec(iface, srec, 0)) {
			printf("\nDownload failed!\n");
			ret = 1;
		} else {
			printf("OK.\n");
			show_after = 1;
		}
	}

	if (non_volatile_download) {
		printf("\nDownloading to non-volatile memory (flash).\n"
		       "Note! This can take about 30 seconds. "
		       "Do _not_ remove card during download.\n");
		if (download_srec(iface, srec, 1)) {
			printf("\nDownload failed!\n");
			ret = 1;
		} else {
			printf("OK.\n");
			show_after = 1;
		}
	}

	if (dump_image_data)
		dump_s3_data(srec, "prism2_srec.dump");

	if (show_after) {
		struct s3_info_platform id;
		printf("Components after download:\n");
		if (get_compid(iface, HFA384X_RID_NICID, &id) == 0)
			printf("  NICID: 0x%04x v%u.%u.%u\n",
			       id.platform, id.major, id.minor,
			       id.variant);
		if (get_compid(iface, HFA384X_RID_PRIID, &id) == 0)
			printf("  PRIID: 0x%04x v%u.%u.%u\n",
			       id.platform, id.major, id.minor,
			       id.variant);
		if (get_compid(iface, HFA384X_RID_STAID, &id) == 0)
			printf("  STAID: 0x%04x v%u.%u.%u\n",
			       id.platform, id.major, id.minor,
			       id.variant);
	}

	free_srec(srec);
	free_wlan(&wlan);

	return ret;
}
