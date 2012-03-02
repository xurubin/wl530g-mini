/*
 * NVRAM variable manipulation (direct mapped flash)
 *
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id: nvram_vx.c,v 1.8 2003/05/10 01:59:07 mhuang Exp $
 */

#include <typedefs.h>
//#include <osl.h>
#include <bcmnvram.h>
//#include <bcmendian.h>
#include <bcmutils.h>
//#include <flash.h>
#include <flashutl.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <sbutils.h>

struct nvram_tuple * _nvram_realloc(struct nvram_tuple *t, const char *name, const char *value);
void _nvram_free(struct nvram_tuple *t);
int _nvram_read(void *buf);

extern char * _nvram_get(const char *name);
extern int _nvram_set(const char *name, const char *value);
extern int _nvram_unset(const char *name);
extern int _nvram_getall(char *buf, int count);
extern int _nvram_commit(struct nvram_header *header);
extern int _nvram_init(void);
extern void _nvram_exit(void);

static struct nvram_header *nvram_header = NULL;

#ifdef vxworks
#include <vxWorks.h>
#include <vxLib.h>
#include <taskLib.h>
static SEM_ID nvram_lock = 0;
#define NVRAM_LOCK()	do { if (nvram_lock) semTake(nvram_lock, WAIT_FOREVER); } while (0)
#define NVRAM_UNLOCK()	do { if (nvram_lock) semGive(nvram_lock); } while (0)
#else
#define NVRAM_LOCK()	do {} while (0)
#define NVRAM_UNLOCK()	do {} while (0)
#endif

/* Convenience */
#define KB * 1024
#define MB * 1024 * 1024

char *
nvram_get(const char *name)
{
	char *value;

	NVRAM_LOCK();
	value = _nvram_get(name);
	NVRAM_UNLOCK();

	return value;
}

int
nvram_getall(char *buf, int count)
{
	int ret;

	NVRAM_LOCK();
	ret = _nvram_getall(buf, count);
	NVRAM_UNLOCK();

	return ret;
}

int
nvram_set(const char *name, const char *value)
{
	int ret;

	NVRAM_LOCK();
	ret = _nvram_set(name, value);
	NVRAM_UNLOCK();

	return ret;
}

int
nvram_unset(const char *name)
{
	int ret;

	NVRAM_LOCK();
	ret = _nvram_unset(name);
	NVRAM_UNLOCK();

	return ret;
}

static bool
nvram_reset(void *sbh)
{
	chipcregs_t *cc;
	char *value;
	uint32 watchdog = 0, gpio;
	uint idx, msec;

	idx = sb_coreidx(sbh);

	/* Check if we were soft reset */
	if ((cc = sb_setcore(sbh, SB_CC, 0))) {
		watchdog = R_REG(&cc->intstatus) & 0x80000000;
		sb_setcoreidx(sbh, idx);
	}
	if (watchdog)
		return FALSE;

	value = nvram_get("reset_gpio");
	if (!value)
		return FALSE;

	gpio = (uint32) bcm_atoi(value);
	if (gpio > 7)
		return FALSE;

	/* Setup GPIO input */
	sb_gpioouten(sbh, (1 << gpio), 0);

	/* GPIO reset is asserted low */
	for (msec = 0; msec < 5000; msec++) {
		if (sb_gpioin(sbh) & (1 << gpio))
			return FALSE;
		OSL_DELAY(1000);
	}

	return TRUE;
}
	
extern unsigned char embedded_nvram[];

int
nvram_init(void *sbh)
{
#ifdef ASUS
#else
	ulong offsets[] = {
		FLASH_BASE + 4 MB - NVRAM_SPACE,
		FLASH_BASE + 3 MB - NVRAM_SPACE,
		FLASH_BASE + 2 MB - NVRAM_SPACE,
		FLASH_BASE + 1 MB - NVRAM_SPACE,
		FLASH_BASE + 4 KB,
		FLASH_BASE + 1 KB,
#ifdef _CFE_
		(ulong) embedded_nvram,
#endif
	};
	int i, ret;

#ifdef vxworks
	/* Initialize private semaphore */
	if ((nvram_lock = semBCreate(SEM_Q_FIFO, SEM_FULL)) == NULL) {
		printf("nvram_init: semBCreate failed\n");
		return ERROR;
	}
#endif

	if (sbh) {
		for (i = 4; i < ARRAYSIZE(offsets); i++) {
			nvram_header = (struct nvram_header *) offsets[i];
			if (nvram_header->magic == NVRAM_MAGIC)
				break;
			nvram_header = NULL;
		}
		/* Temporarily initialize with embedded NVRAM */
		ret = _nvram_init();
		if (ret == 0) {
			/* Restore defaults if button held down */
			if (nvram_reset(sbh))
				return 1;
			_nvram_exit();
		}
	}

	/* Find NVRAM */
	for (i = 0; i < ARRAYSIZE(offsets); i++) {
		nvram_header = (struct nvram_header *) offsets[i];
		if (nvram_header->magic == NVRAM_MAGIC)
			break;
		nvram_header = NULL;
	}

	ret = _nvram_init();
	if (ret == 0) {
		/* Restore defaults if embedded NVRAM used */
		if (nvram_header && i >= 4)
			ret = 1;
	}

	return ret;
#endif
}

void
nvram_exit(void)
{
	_nvram_exit();
}

int
_nvram_read(void *buf)
{
	uint32 *src, *dst;
	uint i;

	if (!nvram_header)
		return -19; /* -ENODEV */

	src = (uint32 *) nvram_header;
	dst = (uint32 *) buf;

	for (i = 0; i < sizeof(struct nvram_header); i += 4)
		*dst++ = *src++;

	for (; i < nvram_header->len && i < NVRAM_SPACE; i += 4)
		*dst++ = ltoh32(*src++);

	return 0;
}

struct nvram_tuple *
_nvram_realloc(struct nvram_tuple *t, const char *name, const char *value)
{
	if (!(t = MALLOC(sizeof(struct nvram_tuple) + strlen(name) + 1 + strlen(value) + 1)))
		return NULL;

	/* Copy name */
	t->name = (char *) &t[1];
	strcpy(t->name, name);

	/* Copy value */
	t->value = t->name + strlen(name) + 1;
	strcpy(t->value, value);

	return t;
}

void
_nvram_free(struct nvram_tuple *t)
{
	if (t)
		MFREE(t, sizeof(struct nvram_tuple) + strlen(t->name) + 1 + strlen(t->value) + 1);
}

int
nvram_commit(void)
{
	struct nvram_header *header;
	int ret;
	uint32 *src, *dst;
	uint i;

	if (!(header = (struct nvram_header *) MALLOC(NVRAM_SPACE))) {
		printf("nvram_commit: out of memory\n");
		return -12; /* -ENOMEM */
	}

	NVRAM_LOCK();

	/* Regenerate NVRAM */
	ret = _nvram_commit(header);
	if (ret)
		goto done;
	
	src = (uint32 *) &header[1];
	dst = src;

	for (i = sizeof(struct nvram_header); i < header->len && i < NVRAM_SPACE; i += 4)
		*dst++ = htol32(*src++);
#ifdef ASUS
#else
#ifdef _CFE_
	if ((ret = cfe_open("flash0.nvram")) >= 0) {
		cfe_writeblk(ret, 0, (unsigned char *) header, NVRAM_SPACE);
		cfe_close(ret);
	}
#else
	if (flash_init((void *) FLASH_BASE, NULL) == 0)
		nvWrite((unsigned short *) header, NVRAM_SPACE);
#endif
#endif

 done:
	NVRAM_UNLOCK();
	MFREE(header, NVRAM_SPACE);
	return ret;
}
