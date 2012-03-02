#include <stdio.h>

int
main(int argc, char *argv[])
{
	FILE *in, *out;
	char buf[1024];
	long count, klen, ilen, i;

	if (argc<4)
	{
		printf("mkimage <image> <kernel> <filesystem> <bootloader>\n");
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

	/* 0xFFC00000 - 0xFFCBFFFF */
	klen=0xa0000;

	/* 0xFFC00000 - 0xFFFEFFFF */
	ilen=0x3f0000-klen;

	/* read kernel */
	while((count=fread(buf, 1, sizeof(buf), in)))
	{
		fwrite(buf, 1, count, out);
		klen-=count;
	}	

	fclose(in);

	printf("kernel file length: %d %d\n", 655350-klen, argc);

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
		printf("can't open filesystem\n");
		exit(0);
	}

	/* append filesystem */
	while((count=fread(buf, 1, sizeof(buf), in)))
	{
		fwrite(buf, 1, count, out);
		ilen-=count;
	}

if(argc>=5)
{
	printf("Write for bootloader\n");

	if (ilen>0)
	{
		buf[0] = 0xff;

		for(i=0; i< ilen; i++)
			fwrite(buf, 1, 1, out);
	}
}
	fclose(in);

if(argc>=5)
{
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
}

	fclose(out);
}
