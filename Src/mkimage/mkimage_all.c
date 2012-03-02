#include <stdio.h>

int
main(int argc, char *argv[])
{
	FILE *in, *out;
	char buf[1024];
	long count, klen, ilen, mlen, i;

	if (argc<4)
	{
		printf("mkimage <image> <linux> <mfg_data> <bootloader>\n");
		exit(0);
	}


	in=fopen(argv[2], "r");

	if (in==NULL) 
	{
		printf("can't open kernel file\n");
		exit(0);
	}

	out=fopen(argv[1], "w");
	
	if (out==NULL)
	{
		printf("can't open image file\n");
		exit(0);
	}

	/* 0xFFC00000 - 0xFFFCFFFF */
	ilen=0x3d0000;

	/* 0xFFFD0000 - 0xFFFF0000 */
	mlen=0x020000;

	/* read image */
	while((count=fread(buf, 1, sizeof(buf), in)))
	{
		fwrite(buf, 1, count, out);
		ilen-=count;
	}	

	fclose(in);

	printf("image file length: %d %d\n", 0x3d0000-ilen, argc);

	if (ilen>0)
	{
		buf[0] = 0xff;

		for(i=0;i<ilen;i++)
		{
			fwrite(buf, 1, 1, out);
		}
	}

	in=fopen(argv[3], "r");
	
	if (in==NULL) 
	{
		printf("can't open mfg_data\n");
		exit(0);
	}

	/* append filesystem */
	while((count=fread(buf, 1, sizeof(buf), in)))
	{
		fwrite(buf, 1, count, out);
		mlen-=count;
	}

	fclose(in);

	if (mlen>0)
	{
		buf[0] = 0xff;

		for(i=0;i<mlen;i++)
		{
			fwrite(buf, 1, 1, out);
		}
	}

	printf("Write for bootloader\n");

	in=fopen(argv[4], "r");
	if (in==NULL)
	{
		printf("can't open boot loader\n");
		exit(0);
	}

	while((count=fread(buf, 1, sizeof(buf), in)))
	{
		fwrite(buf, 1, count, out);
	}
	fclose(in);
	fclose(out);
}
