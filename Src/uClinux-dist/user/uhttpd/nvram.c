//#define MOCK_NVRAM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#ifndef MOCK_NVRAM
#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <linux/mtd/mtd.h>
#endif
#include "nvram.h"

#define MAX_NVRAM_BLOCK_SIZE 0x2000
typedef struct {
	char * devicename;
	int size;
	int need_refresh;
	char cache[MAX_NVRAM_BLOCK_SIZE];
} NVRAM;
NVRAM flash[2] =
#ifdef MOCK_NVRAM
{{"config.nvram", 128, 1}, {"template.nvram", 0x2000, 1}};
#else
{{"/dev/mtdblock2", 0x2000, 1}, {"/dev/mtdblock3", 0x2000, 1}};
#endif

int reloadNVRAM(int nvram)
{
	int f = open(flash[nvram].devicename, O_RDONLY);
	if (f)
	{
		int read_size = read(f, flash[nvram].cache, flash[nvram].size);
		close(f);
		if (read_size == flash[nvram].size)
		{
			flash[nvram].need_refresh = 0;
			return 1;
		}
	}
	printf("Failed to reload NVRAM %s.\n", flash[nvram].devicename);
	return 0;
}

void reloadNVRAMs()
{
	reloadNVRAM(NVRAM_CONFIG);
	reloadNVRAM(NVRAM_TEMPLATE);
}

const char* readNVRAM(int nvram, int start, int* size)
{
	if (flash[nvram].need_refresh)
		reloadNVRAM(nvram);
	if (start >= flash[nvram].size)
	{
		if (size) *size = 0;
		return 0;
	}
	else
	{
		if (size) *size = flash[nvram].size - start;
		return flash[nvram].cache + start;
	}
}

int getNVRAMUsedBytes(int nvram)
{
	int i = flash[nvram].size;
	while (i && (flash[nvram].cache[i-1] == (char)0xFF))
		i--;
	return i;
}

int getNVRAMSize(int nvram)
{
	return flash[nvram].size;
}

int writeNVRAM(int nvram, const char* data, int start, int size)
{
        int i, f;
	if (start + size >= flash[nvram].size)
		return 0;
	for(i=0; i<size; i++)
	{
		// Need to flip a bit from 0 to 1, which is impossible without an erase
		if ((char)((~flash[nvram].cache[start + i]) & data[i]))
			return 0;
	}
	f = open(flash[nvram].devicename, O_WRONLY);
	lseek(f, start,  SEEK_SET);
	for(i=0; i<size; i++)
	{
		flash[nvram].cache[start + i] = data[i];
		if (!write(f, &data[i], 1))
		{
			printf("Write to %s at %d failed.\n", flash[nvram].devicename, start + i);
			close(f);
			return 0;
		}
	}
	close(f);
	return 1;
}

int eraseNVRAM(int nvram)
{
	int fd;
	int i;
#ifndef MOCK_NVRAM
	struct mtd_info_user mtdInfo;
	struct erase_info_user mtdEraseInfo;
#else
	int data;
#endif

#ifdef MOCK_NVRAM
	fd = open(flash[nvram].devicename, O_RDWR | O_CREAT | O_SYNC);
#else
	fd = open(flash[nvram].devicename, O_RDWR | O_SYNC);
#endif
	if(fd < 0) {
		printf("Could not erase flash: %s\n", flash[nvram].devicename);
		return 0;
	}

#ifndef MOCK_NVRAM
	if(ioctl(fd, MEMGETINFO, &mtdInfo)) {
		printf(stderr, "Could not get MTD device info from %s\n", flash[nvram].devicename);
		close(fd);
		return 0;
	}

	if (flash[nvram].size != mtdInfo.size)
	{
                printf("Flash geometry mismatch: %s %d %d.\n", flash[nvram].devicename, flash[nvram].size, mtdInfo.size);
		exit(1);
		return 0;
	}
	mtdEraseInfo.length = mtdInfo.erasesize;

	for (mtdEraseInfo.start = 0;
		 mtdEraseInfo.start < mtdInfo.size;
		 mtdEraseInfo.start += mtdInfo.erasesize) {

		ioctl(fd, MEMUNLOCK, &mtdEraseInfo);
		if(ioctl(fd, MEMERASE, &mtdEraseInfo)) {
			fprintf(stderr, "Could not erase MTD device: %s\n", flash[nvram].devicename);
			close(fd);
			exit(1);
		}
	}
#else
	data = 0xFF;
	for(i=0; i < getNVRAMSize(nvram); i++)
		write(fd, &data, 1);
#endif
	close(fd);

	if (reloadNVRAM(nvram))
	{
		for(i=0; i<flash[nvram].size; i++)
			if (flash[nvram].cache[i] != (char)0xFF)
				return 0;
		return 1;
	}
	else
		return 0;
}

typedef struct {
	char * variable_name;
	char * variable_value;
} MOCK_VARIABLE;
MOCK_VARIABLE mock_variables[100];

MOCK_VARIABLE* lookup_mock_variable(const char* var_name)
{
	MOCK_VARIABLE* v;
	for(v = mock_variables; v->variable_name; v++)
		if (!strcmp(v->variable_name, var_name))
			return v;
	return 0;
}

#define NextStr(buf, index, len) while((index < len) && buf[index++]) {}

int nvram_get_variable(const char* variable_name, char* content, int content_len)
{
//	MOCK_VARIABLE* v = lookup_mock_variable(variable_name);
//	if (v)
//	{
//		strncpy(content, v->variable_value, content_len);
//		return 1;
//	}
//	else
//		return 0;
	int len;
	const char* data = readNVRAM(NVRAM_CONFIG, 0, &len);
	const char* var_content = 0;
	int name_idx;
	int idx = 0;
	// The last variable content counts
	while (idx < len)
	{
		name_idx = idx;
		NextStr(data, idx, len);
		if (idx >= len) break;
		if (!strcmp(variable_name, data + name_idx))
			var_content = data + idx;
		NextStr(data, idx, len);
	}
	if (var_content)
	{
		strncpy(content, var_content, content_len);
		return 1;
	}
	return 0;
}

int nvram_append_variable(const char* variable_name, const char* content)
{
	int used = getNVRAMUsedBytes(NVRAM_CONFIG);
	int free = getNVRAMSize(NVRAM_CONFIG) - used;
	int name_len = strlen(variable_name);
	int content_len = strlen(content);
	if (free >= name_len + content_len + 2)
	{
		writeNVRAM(NVRAM_CONFIG, variable_name, used, name_len + 1);
		used += name_len + 1;
		writeNVRAM(NVRAM_CONFIG, content, used, content_len + 1);
		return 1;
	}
	else
		return 0;
}

int nvram_set_variable(const char* variable_name, const char* content)
{
//	int i;
//	MOCK_VARIABLE* v = lookup_mock_variable(variable_name);
//	if (v)
//	{
//		free(v->variable_value);
//		v->variable_value = strdup(content);
//		return 1;
//	}
//	for(i=0; i<sizeof(mock_variables)/sizeof(mock_variables[0]) && mock_variables[i].variable_name; i++) {}
//	if (sizeof(mock_variables)/sizeof(mock_variables[0]))
//	{
//		mock_variables[i].variable_name = strdup(variable_name);
//		mock_variables[i].variable_value = strdup(content);
//		return 1;
//	}
//	return 0;
	int existing_variables[256];
	int i, var_count = 0;

	// Append the variable, if fails then need to erase the entire block and consolidate variables.
	if (!nvram_append_variable(variable_name, content))
	{
		int used = getNVRAMUsedBytes(NVRAM_CONFIG);
		char data[MAX_NVRAM_BLOCK_SIZE];
		int var_idx = 0;
		int seen;
		int cur_content;
		int written = 0;
		const char *cur_content_ptr;
		memcpy(data, readNVRAM(NVRAM_CONFIG, 0, 0), used);

		while(var_idx < used)
		{
			// Later variable overwrite existing ones.
			seen = 0;
			for(i=0; i<var_count; i++)
				if (!strcmp(data+existing_variables[i], data+var_idx))
				{
					existing_variables[i] = var_idx;
					seen = 1;
				}
			if (!seen)
				existing_variables[var_count++] = var_idx;
			// Skip to next variable
			NextStr(data, var_idx, used);
			NextStr(data, var_idx, used);
		}

		// erase NVRAM and write back existing variables
		eraseNVRAM(NVRAM_CONFIG);
		for(i=0; i<var_count; i++)
		{
			cur_content = existing_variables[i];
			NextStr(data, cur_content, used);
			if (!strcmp(data + existing_variables[i], variable_name))
			{
				cur_content_ptr = content;
				written = 1;
			}
			else
				cur_content_ptr = data + cur_content;

			if (!nvram_append_variable(data + existing_variables[i], cur_content_ptr))
			{
				printf("Failed to consolidate variables. %s %s\n", data + existing_variables[i], data + cur_content);
				exit(1);
			}
		}
		// Try to append the variable again if we haven't done so, and if it fails again then we have truly ran out of flash space
		if ((!written) && (!nvram_append_variable(variable_name, content)))
			return 0;
	}
	return 1;
}
