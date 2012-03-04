#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <zlib.h>

void Decompress(const char* fname)
{
	FILE* fin = fopen(fname, "rb");
	int len;
	
	fseek(fin, 0, SEEK_END);
	len = ftell(fin);

	char* buf = (char*)malloc(len+2);
	fseek(fin, 0, SEEK_SET);
	buf[0] = 0x78;
	buf[1] = 0x9C;
	fread(buf+2, len, 1, fin);
	fclose(fin);
	
	unsigned outlen = 2*1024*1024;
	unsigned char* outbuf = (unsigned char*) malloc(outlen);
	int r = uncompress(outbuf, &outlen, buf, len+2);

	FILE* fout = fopen("out.bin", "wb");
	fwrite(outbuf, outlen, 1, fout);
	fclose(fout);
}
unsigned char* DeflateToBuf(const char* fname, unsigned* blob_len)
{
	FILE* fin = fopen(fname, "rb");
	int len;
	
	fseek(fin, 0, SEEK_END);
	len = ftell(fin);

	char* buf = (char*)malloc(len);
	fseek(fin, 0, SEEK_SET);
	fread(buf, len, 1, fin);
	fclose(fin);
	
	unsigned outlen = compressBound(len);
	unsigned char* outbuf = (unsigned char*) malloc(outlen);
	int r = compress2(outbuf, &outlen, buf, len, Z_BEST_COMPRESSION);
	free(buf);
	if (r != Z_OK)
	{
		printf("Deflating %s failed.\n", fname);
		return NULL;
	}
	*blob_len = outlen;
	return outbuf;
}
void FixChecksum(FILE* fout)
{
	unsigned checksum, len, data;

	fseek(fout, 0, SEEK_END);
	len = ftell(fout);
	
	if (len % 4 != 0)
		printf("File is not aligned to word boundary!\n");
		
	fseek(fout, 0, SEEK_SET);
	checksum = 0;
	while(!feof(fout))
	{
		if (fread(&data, 4, 1, fout))
			checksum += data;
	}
	
	checksum = ~checksum;
	fseek(fout, 0, SEEK_END);
	fwrite(&checksum, 4, 1, fout);
}

int main(int argc, char* argv[])
{
	if (argc == 2)
	{
		Decompress(argv[1]);
		return 0;
	}
	else if (argc != 3)
	{
		printf("Usage: build <layout file> <code.bin>\n");
		return 1;
	}
	
	char buf[256];
	int i;
	
	FILE* fout = fopen(argv[2], "w+b");
	// file header
	i = 0x48343132;
	fwrite(&i, 4, 1, fout);
	
	// dummy placeholder for data len
	i = 0;
	fwrite(&i, 4, 1, fout);
	
	memset(buf, 0, sizeof(buf));
	fwrite(buf, 0x74, 1, fout);
	
	FILE* flayout = fopen(argv[1], "rt");
	int total_data_len = 0;
	
	int eip = -1;
	while(!feof(flayout))
	{
		unsigned mem_addr = (unsigned) -1;
		char fname[255];
		if (fscanf(flayout, "%x %s", &mem_addr, fname) == 2)
		{
			if (!strcmp("eip", fname))
			{
				eip = mem_addr;
				continue;
			}
			int data_len = 0;
			unsigned char* data_ptr = DeflateToBuf(fname, &data_len);
			if ( (data_ptr[0] != 0x78) || ((data_ptr[1] != 0x9C) && (data_ptr[1] != 0xDA)) )
			{
				printf("Deflating %s failed %.2x %.2x.\n", fname, data_ptr[0], data_ptr[1]);
				return 1;
			}
			
			// Write data to output file
			total_data_len += fwrite(&mem_addr, 1, 4, fout);
			
			i = 3; // Flag for compressed data
			total_data_len += fwrite(&i, 1, 4, fout);

			data_len -= 2; // Remove the zlib header
			data_len -= 4; // Strip the last 4 bytes (CRC??)
			if (data_len %4 != 0)
				data_len = (data_len + 3) & (~3);
				
			total_data_len += fwrite(data_ptr + 2, 1, data_len, fout);
			
			printf("%s @ %.8X: %d bytes.\n", fname, mem_addr, data_len);
			free(data_ptr);
		}
		else
		{
			printf("layout file format error.\n");
			return 1;
		}
	}
	fclose(flayout);
		
	if (eip == -1)
		printf("Did you forget to specify EIP?\n");
	// EIP field
	total_data_len += fwrite(&eip, 1, 4, fout);
	i = 4; // Flag for EIP
	total_data_len += fwrite(&i, 1, 4, fout);
	
	// Patch up the data len field
	fseek(fout, 4, SEEK_SET);
	fwrite(&total_data_len, 1, 4, fout);
//	fclose(fout);
//	fout = fopen("test.bin", "r+b");
	// Finally checksum
	FixChecksum(fout);
	fclose(fout);
	return 0;
}