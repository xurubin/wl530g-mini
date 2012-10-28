
void
do_cp(argc, argv)
	char	**argv;
{
	BOOL	dirflag;
	char	*srcname;
	char	*destname;
	char	*lastarg;
	struct	stat	statbuf;
	
	BOOL     skipexisting = (argc > 1)  && (!strcmp(argv[1], "-se"));
	if (skipexisting) {
	    argc--;
		argv++;
	}
	lastarg = argv[argc - 1];

	dirflag = isadir(lastarg);

	if ((argc > 3) && !dirflag) {
		fprintf(stderr, "%s: not a directory\n", lastarg);
		return;
	}

	while (argc-- > 2) {
		destname = lastarg;
		if (dirflag)
			destname = buildname(destname, srcname);
			
		if ( (!skipexisting) || (stat(destname, &statbuf) < 0) )
			(void) copyfile(*++argv, destname, FALSE);
	}
}
