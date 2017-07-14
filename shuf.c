/*
 * Copyright (c) 2017 Brian Callahan <bcallah@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef LIBBSD
#include <bsd/stdlib.h>
#include <bsd/unistd.h>
#endif

static FILE    *ofile;
static char 	delimiter = '\n';
static int 	most = -1, rflag;

/*
 * shuf -- output a random permutation of input lines
 */

static void
printshuf(int argn, char *args[], int ind, size_t len)
{

	if (argn > 1)
		len = strlen(args[ind]);
	fwrite(args[ind], len, 1, ofile);
	fwrite(&delimiter, 1, 1, ofile);
}

static void
randomshuf(int argn, char *args[], size_t len)
{
	int 	i;

	while ((most == -1 ? 1 : most-- > 0)) {
		i = arc4random_uniform(argn);
		printshuf(argn, args, i, len);
	}
}

static void
shuf(int argn, char *args[], size_t len)
{
	char   *argt;
	int 	i, j;

	for (i = argn - 1; i > 0; i--) {
		j = arc4random_uniform(i + 1);
		argt = args[j];
		args[j] = args[i];
		args[i] = argt;
	}

	most = (most == -1 ? argn : most > argn ? argn : most);

	for (i = 0; i < most; i++)
		printshuf(argn, args, i, len);
}

static void
shufecho(int argn, char *args[])
{

	if (rflag)
		randomshuf(argn, args, strlen(args[0]));
	else
		shuf(argn, args, strlen(args[0]));
}

static void
shuffile(const char *input, int argn, size_t inputlen)
{
	const char     *s;
	char	      **args = NULL, *argt;
	int 		i = 0;
	size_t 		len, savedinputlen;

	if ((args = reallocarray(args, argn, sizeof(char *))) == NULL)
		errx(1, "memory exhausted");

	savedinputlen = inputlen;

	while (i < argn) {
		for (s = input; *s != delimiter; s++) {
			if (inputlen-- == 0)
				break;
		}
		len = s - input;

		if ((args[i] = malloc(len + 1)) == NULL)
			err(1, "malloc");
		argt = args[i++];

		while (len-- > 0)
			*argt++ = *input++;
		*argt = '\0';

		input++;
		if (inputlen > 0) {
			if (--inputlen == 0)
				break;
		}
	}

	most = (most < argn ? most : argn);

	if (rflag)
		randomshuf(argn, args, savedinputlen);
	else
		shuf(argn, args, savedinputlen);

	for (i = 0; i < argn; i++) {
		free(args[i]);
		args[i] = NULL;
	}

	free(args);
	args = NULL;
}

static void
shufintegers(int range, int lo)
{
	int    *args = NULL;
	int 	argt, i, j;

	if ((args = reallocarray(args, range, sizeof(int))) == NULL)
		errx(1, "range size will exhaust memory");

	if (rflag) {
		while ((most == -1 ? 1 : most-- > 0))
			fprintf(ofile, "%d%c",
				arc4random_uniform(range) + lo, delimiter);
	} else {
		for (i = 0; i < range; i++)
			args[i] = lo + i;

		for (i = range - 1; i > 0; i--) {
			j = arc4random_uniform(i + 1);
			argt = args[j];
			args[j] = args[i];
			args[i] = argt;
		}

		most = (most == -1 ? range : most > range ? range : most);

		for (i = 0; i < most; i++)
			fprintf(ofile, "%d%c", args[i], delimiter);
	}

	free(args);
	args = NULL;
}

static void
repledge(int oflag)
{
#ifdef __OpenBSD__
	const char     *new;

	if (oflag)
		new = "stdio wpath";
	else
		new = "stdio";

	if (pledge(new, NULL) == -1)
		errx(1, "pledge");
#endif
}

static void
usage(void)
{
	const char *name;

	name = getprogname();

	fprintf(stderr, "usage: %s [-hv] [-n count] [-o file] [-rz] [file]\n"
		"       %s [-hv] -e [-n count] [-o file] [-rz] [args ...]\n"
		"       %s [-hv] -i lo-hi [-n count] [-o file] [-rz]\n",
		name, name, name);

	exit(1);
}

static void
version(void)
{

	fputs("shuf 1.2\n"
"Copyright (c) 2017 Brian Callahan <bcallah@openbsd.org>\n\n"
"Permission to use, copy, modify, and distribute this software for any\n"
"purpose with or without fee is hereby granted, provided that the above\n"
"copyright notice and this permission notice appear in all copies.\n\n"
"THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES\n"
"WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF\n", stderr);
	fputs(
"MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR\n"
"ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES\n"
"WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN\n"
"ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF\n"
"OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\n", stderr);

	exit(1);
}

int
main(int argc, char *argv[])
{
	FILE	       *ifile;
	const char     *errstr;
	char	       *argp, *buf;
	int 		argn = 1, ch, hi = 0, lo = 0, prev = 1;
	int 		eflag = 0, iflag = 0, oflag = 0;
	size_t 		buflen = 0, bufsize = 1024;

#ifdef __OpenBSD__
	if (pledge("stdio rpath wpath cpath", NULL) == -1)
		errx(1, "pledge");
#endif

	while ((ch = getopt(argc, argv, "ehi:n:o:rvz")) != -1) {
		switch (ch) {
		case 'e':
			if (iflag)
				errx(1, "cannot combine -e with -i");
			eflag = 1;
			break;
		case 'h':
			usage();
		case 'i':
			if (eflag)
				errx(1, "cannot combine -i with -e");
			if (iflag++)
				errx(1, "cannot have multiple -i");

			argp = strchr(optarg, '-');
			*argp = '\0';

			lo = strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr != NULL)
				errx(1, "invalid or out of range lo");

			hi = strtonum(argp + 1, 0, INT_MAX, &errstr);
			if (errstr != NULL)
				errx(1, "invalid or out of range hi");

			if (lo >= hi)
				errx(1, "lo is greater than or equal to hi");
			if (hi == INT_MAX && lo == 0)
				errx(1, "lo-hi range too large");
			break;
		case 'n':
			most = strtonum(optarg, 0, INT_MAX, &errstr);
			if (errstr != NULL)
				errx(1,
				     "-n count must be from 0 to %d, not %s",
				     INT_MAX, optarg);
			break;
		case 'o':
			if (oflag++)
				errx(1, "cannot have multiple -o");

			if ((ofile = fopen(optarg, "w")) == NULL)
				err(1, "could not open %s", optarg);

			break;
		case 'r':
			rflag = 1;
			break;
		case 'v':
			version();
		case 'z':
			delimiter = '\0';
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (eflag == 0 && argc > 1)
		errx(1, "extra operand '%s'", *++argv);

	if (oflag == 0)
		ofile = stdout;

	if (eflag) {
		repledge(oflag);
		shufecho(argc, argv);
	} else if (iflag) {
		repledge(oflag);
		shufintegers(hi - lo + 1, lo);
	} else {
		if (argc == 0 || (argv[0][0] == '-' && argv[0][1] == '\0')) {
			ifile = stdin;
		} else {
			if ((ifile = fopen(*argv, "r")) == NULL)
				err(1, "could not open %s", argv[1]);
		}

		repledge(oflag);

		if ((buf = malloc(bufsize)) == NULL)
			err(1, "malloc failed");

		while ((ch = fgetc(ifile)) != EOF) {
			buf[buflen++] = ch;

			if (buflen == bufsize) {
				bufsize <<= 1;
				if ((buf = realloc(buf, bufsize)) == NULL)
					err(1, "realloc failed");
			}
			if (prev == delimiter)
				++argn;
			prev = ch;
		}
		buf[buflen] = '\0';

		if (ifile != stdin)
			fclose(ifile);

		shuffile(buf, argn, buflen);

		free(buf);
		buf = NULL;
	}

	if (oflag)
		fclose(ofile);

	return 0;
}
