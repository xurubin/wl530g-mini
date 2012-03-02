#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/mman.h>
#include <sys/errno.h>

main(int argc, char *argv[])
{
	FILE *fd;
	int fdno;
	char buf[100], *nvram_buf;
	int len, i;
	unsigned long *off;

	if (argc>=3 && strcmp(argv[1],"get")==0)
	{
		fdno = open("/dev/nvram", 0);
		if (fdno<0) printf("fd error\n");
		else
		{
			nvram_buf = 0;
			nvram_buf = mmap(0, 0x8000, PROT_READ, MAP_SHARED, fdno, 0);
			strcpy(buf, argv[2]);
			len = read(fdno, buf, sizeof(buf));
			if (strcmp(buf, argv[2])==0)
			{
				//printf("[none]\n");
			}
			else
			{
				off = (unsigned long *)buf;	
				printf("%s\n", nvram_buf+(*off));
			}
			close(fdno);
		}
	}
	else if (argc>=3 && strcmp(argv[1],"set")==0)
	{
		fd = fopen("/dev/nvram", "a+");
		if (fd==NULL) printf("fd error\n");
		else
		{
			strcpy(buf, argv[2]);
			fwrite(buf, 1, sizeof(buf), fd);
			//printf("set: %s\n", buf);
			fclose(fd);
		}
	}
	else if (argc==2 && strcmp(argv[1],"commit")==0)
	{
		struct ifreq ifr;
		unsigned int cmd;
	
		fdno = open("/dev/nvram", 0);
		if (fdno<0) printf("fd error\n");
		else
		{
			cmd = 0x48534c46;
			ifr.ifr_data = cmd;
			ioctl(fdno, cmd, &ifr);
			close(fdno);
		}
		 		
	}

	if (argc==1)
	{
		printf("usage: nvram get [name]		\n");
		printf("       nvram set [name] [value] \n");
		printf("       nvram commit		\n");
	}
}
