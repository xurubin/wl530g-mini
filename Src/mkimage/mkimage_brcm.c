#include <stdio.h>

int
main(int argc, char *argv[])
{
	FILE *in, *out;
	char buf[1024];
	long count, klen, ilen, i;

	if (argc<4)
	{
		printf("mkimage <image> <bootloader> <linux image>\n");
		exit(0);
	}


	in=fopen(argv[2], "r");

	if (in==NULL) 
	{
		printf("can't open bootloader file\n");
		exit(0);
	}

	out=fopen(argv[1], "w");
	
	if (out==NULL)
	{
		printf("can't open image file\n");
		exit(0);
	}

	/* 0xFFC00000 - 0xFFCBFFFF */
	//klen=0x12c000;
	klen=0x40000;

	/* 0xFFC00000 - 0xFFFEFFFF */

	/* read bootloder */
	while((count=fread(buf, 1, sizeof(buf), in)))
	{
		fwrite(buf, 1, count, out);
		klen-=count;
	}	

	fclose(in);

	printf("kernel file length: %d %d\n", 0x40000-klen, argc);

	if (klen>0)
	{
		buf[0] = 0xff;

		for(i=0;i<klen;i++)
		{
			fwrite(buf, 1, 1, out);
		}
	}

	in=fopen(argv[3], "r");
	
	if (in==NULL) 
	{
		printf("can't open linux image\n");
		exit(0);
	}

	/* append filesystem */
	while((count=fread(buf, 1, sizeof(buf), in)))
	{
		fwrite(buf, 1, count, out);
		ilen-=count;
	}

	fclose(in);
	fclose(out);
}
