#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <sys/wait.h>

typedef struct {
	int		sighup:1;
	int		sigterm:1;
	int		sigchld:1;
	int		sigusr1:1;
	int		sigusr2:1;
	int		running:1;
	sig_atomic_t	waiting;
	sigjmp_buf 	sigjmp;
} usl_signal_data_t;


struct u1_bits_s{
	unsigned char X;
	unsigned char Y;
} __attribute__ ((packed));
typedef struct u1_bits_s u1_bits_t;

union u1_u {
	u1_bits_t bits;
	unsigned short int flags_ver;
} __attribute__ ((packed));
typedef union u1_u u1_t;

struct u2_bits_s{
	unsigned short int  	length;
	unsigned short int  	tunnel_id;
	unsigned short int  	session_id;
	unsigned short int  	ns;
	unsigned short int  	nr;
} __attribute__ ((packed));
typedef struct u2_bits_s u2_bits_t;

union u2_u {
	u2_bits_t bits;
	unsigned char			data[0];
} __attribute__ ((packed));
typedef union u2_u u2_t;

struct s1_s {
	u1_t u1;
	u2_t u2;
};
typedef struct s1_s s1_t;

struct s2 {
	union {
		struct {
			unsigned char X;
			unsigned char Y;
		}bits1; //eric**
		unsigned short int flags_ver;
	}u1; //eric**
	union {
		struct {
			unsigned short int  	length;
			unsigned short int  	tunnel_id;
			unsigned short int  	session_id;
			unsigned short int  	ns;
			unsigned short int  	nr;
		}bits2; //eric**
		unsigned char			data[0];
	}u2; //eric**
};

struct t8 {
	unsigned char X[1] __attribute__ ((packed));
};

struct mystruct_s {
	unsigned char X;
} __attribute__ ((packed));

typedef struct mystruct_s mystruct;

struct t16 {
	unsigned int Y __attribute__ ((packed));
};

struct uc1 {
	unsigned char a,b,c,d,e;
};

struct uc2 {
	unsigned char a[5];
};

usl_signal_data_t		usl_sigdata;
//sigjmp_buf sj;

#define SIGNAL(s, handler)	do { \
		sa.sa_handler = handler; \
		if (sigaction(s, &sa, NULL) < 0) { \
			printf ("Couldn't establish signal handler (%d): %m", s); \
			exit(1); \
		} \
	} while (0)

#if 0
static void usl_sigbad(int sig)
{
	printf ("%s() sig %d\n", __FUNCTION__, sig);
	exit(127);
}

static void dump_val (unsigned char *p)
{
	int i;
	for (i = 0; i < 5; ++i)
		printf ("%02X ", p[i]);
	printf ("\n");
}
#endif

int main(void)
{
#if 0
	int v = 0x55;
	unsigned char *pc;
	unsigned short int *p16;
	struct uc1 u1;
	struct uc2 u2;
///	int iRes;
//	struct sigaction sa;
//	sigset_t mask;
//	sigjmp_buf sj;

	printf ("test program.\n");

//	memset(&usl_sigdata, 0, sizeof(usl_sigdata));
//	sigemptyset(&mask);
//	sigaddset(&mask, SIGILL);
	
//	sa.sa_mask = mask;
//	sa.sa_flags = 0;
	
//	SIGNAL(SIGILL, usl_sigbad);
//	signal(SIGILL, usl_sigbad);
//	printf ("sigsetjmp()\n");
//	iRes = sigsetjmp(usl_sigdata.sigjmp, 1);
//	iRes = sigsetjmp(sj, 1);
//	printf ("sigsetjmp() OK return %d\n", iRes);
//	printf ("%ld %ld %ld %ld %ld\n", sizeof (struct s1), sizeof (struct s2), sizeof (struct t8), sizeof (struct t16), sizeof (mystruct));
	printf ("align of (char %d int %d)\n", __alignof__ (unsigned char), __alignof__ (int));

	pc = (unsigned char *) &u1;
	dump_val (pc);
	u1.a = v++;
	dump_val (pc);
	u1.b = v++;
	dump_val (pc);
	u1.c = v++;
	dump_val (pc);
	u1.e = v++;
	dump_val (pc);
	u1.d = v++;
	dump_val (pc);
	printf ("%02X\n", u1.a);
	printf ("%02X\n", u1.b);
	printf ("%02X\n", u1.c);
	printf ("%02X\n", u1.d);
	printf ("%02X\n", u1.e);

	printf ("\n");
	pc = (unsigned char *) &u2;
	dump_val (pc);
	u2.a[0] = v++;
	dump_val (pc);
	u2.a[1] = v++;
	dump_val (pc);
	u2.a[2] = v++;
	dump_val (pc);
	u2.a[3] = v++;
	dump_val (pc);
	u2.a[4] = v++;
	dump_val (pc);
	printf ("%02X\n", u2.a[0]);
	printf ("%02X\n", u2.a[1]);
	printf ("%02X\n", u2.a[2]);
	printf ("%02X\n", u2.a[3]);
	printf ("%02X\n", u2.a[4]);
	printf ("version 2\n");
	
	p16 = (unsigned short int) 0x00010000;
	printf ("%p %p %p\n", p16, p16+1, p16+2);

	printf ("u1_bits_t %d u1_t %d s1_s %d s1_t %d\n", sizeof (u1_bits_t), sizeof (u1_t), sizeof (struct s1_s), sizeof (s1_t));
#endif

#if 0
	int result = 0;
	char *argv[100];
	int argc = 0;

	memset (argv, 0, sizeof (argv));
	argv[argc++] = "pppd";
	argv[argc++] = "--version";
	argv[argc++] = NULL;
	result = execv("/bin/pppd", argv);
	printf ("result %d errno %d %s\n", result, errno, strerror (errno));
#endif

	pid_t pid;
	char *argv[100];
	int argc;

	pid = vfork();
	if (pid < 0)	{
		perror ("vfork failure");
	} else if (pid == 0)	{
		// child process
		printf ("start child process\n");
		argc = 0;
		memset (argv, 0, sizeof (argv));
		argv[argc++] = "pppd";
		argv[argc++] = "--version";
		execv ("/bin/pppd", argv);
		printf ("child process: execv() closed\n");
	} else {
		// parent process
		printf ("parent process. pid of child process is %d\n", pid);
	}
	
	return 0;
}
