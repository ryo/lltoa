#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#ifndef timespecsub
#define timespecsub(tsp, usp, vsp)					\
	do {								\
		(vsp)->tv_sec = (tsp)->tv_sec - (usp)->tv_sec;		\
		(vsp)->tv_nsec = (tsp)->tv_nsec - (usp)->tv_nsec;	\
		if ((vsp)->tv_nsec < 0) {				\
			(vsp)->tv_sec--;				\
			(vsp)->tv_nsec += 1000000000L;			\
		}							\
	} while (/* CONSTCOND */ 0)
#endif

char *
sprintf_wrap(char *dst, uint64_t n)
{
	snprintf(dst, 22, "%llu", (unsigned long long)n);
	return NULL;
}

char *
str_uint64_div(char *dst, uint64_t n)
{
	int l;
	char *p, *q;

	p = q = dst + 20;	/* 20 = maximum length of decimal UINT64_MAX */

	do {
		*--p = (n % 10) + '0';
		n /= 10;
	} while (n != 0);
	l = q - p;
	memmove(dst, p, l);
	dst[l] = '\0';
	return &dst[l];
}

inline int
my_log10(uint64_t n)
{
	if (n < 10000000000000000ULL) {
		if (n < 100000000) {
			if (n < 10000) {
				if (n < 100) {
					if (n < 10) {
						return 1;
					} else {
						return 2;
					}
				} else {
					if (n < 1000) {
						return 3;
					} else {
						return 4;
					}
				}
			} else {
				if (n < 1000000) {
					if (n < 100000) {
						return 5;
					} else {
						return 6;
					}
				} else {
					if (n < 10000000) {
						return 7;
					} else {
						return 8;
					}
				}
			}
		} else {
			if (n < 1000000000000ULL) {
				if (n < 10000000000ULL) {
					if (n < 1000000000ULL) {
						return 9;
					} else {
						return 10;
					}
				} else {
					if (n < 100000000000ULL) {
						return 11;
					} else {
						return 12;
					}
				}
			} else {
				if (n < 100000000000000ULL) {
					if (n < 10000000000000ULL) {
						return 13;
					} else {
						return 14;
					}
				} else {
					if (n < 1000000000000000ULL) {
						return 15;
					} else {
						return 16;
					}
				}
			}
		}
	}

	if (n < 1000000000000000000ULL) {
		if (n < 100000000000000000ULL)
			return 17;
		return 18;
	}

	if (n < 10000000000000000000ULL)
		return 19;
	return 20;
}

char *
str_uint64_div_nomemcpy(char *dst, uint64_t n)
{
	int l;
	char *p;

	l = my_log10(n);

	dst += l;
	p = dst;
	*dst-- = '\0';
	for (; l > 0; l--) {
		*dst-- = (n % 10) + '0';
		n /= 10;
	}

	return p;
}

static inline uint64_t
lldiv10(uint64_t x)
{
#define MAGIC10_H	0xcccccccc
#define MAGIC10_L	0xcccccccd
#define MAGIC10_SHIFT	3

	uint64_t xh = x >> 32;
	uint64_t xl = x & 0xffffffff;

	uint64_t a = xh * MAGIC10_L;
	uint64_t ah = a >> 32;
	uint64_t al = a & 0xffffffff;
	uint64_t b = xl * MAGIC10_H;;
	uint64_t bh = b >> 32;
	uint64_t bl = b & 0xffffffff;

	return (((((xl * MAGIC10_L) >> 32) + al + bl) >> 32) + ah + bh + xh * MAGIC10_H) >> MAGIC10_SHIFT;
}

char *
str_uint64_mulshift(char *dst, uint64_t n)
{
	uint64_t q;
	int l;
	char *p, *p0;

	p = p0 = dst + 20;	/* 20 = maximum length of decimal UINT64_MAX */

	do {
		q = lldiv10(n);
		*--p = (n - (q * 10)) + '0';
		n = q;
	} while (n != 0);
	l = p0 - p;
	memmove(dst, p, l);
	dst[l] = '\0';
	return &dst[l];

}

char *
str_uint64_nodiv(char *dst, uint64_t n)
{
	int q;
	char *p;
	int pad;

	pad = 0;
	p = dst;

	/*
	 * max uint64 is 18446744073709551615,
	 * most significant digit is 1 or 0.
	 */
	if (n >= 10000000000000000000ULL) {
		*p++ = '1';
		n -= 10000000000000000000ULL;
		pad = 1;
	}

#define QOUTPUT(X)				\
	if (n >= X) {				\
		q = 0;				\
		do {				\
			q++;			\
			n -= X;			\
		} while (n >= X);		\
		*p++ = q + '0';			\
		pad = 1;			\
	} else if (pad) {			\
		*p++ = '0';			\
	}

	QOUTPUT(1000000000000000000ULL);
	QOUTPUT(100000000000000000ULL);
	QOUTPUT(10000000000000000ULL);
	QOUTPUT(1000000000000000ULL);
	QOUTPUT(100000000000000ULL);
	QOUTPUT(10000000000000ULL);
	QOUTPUT(1000000000000ULL);
	QOUTPUT(100000000000ULL);
	QOUTPUT(10000000000ULL);
	QOUTPUT(1000000000ULL);
	QOUTPUT(100000000ULL);
	QOUTPUT(10000000ULL);
	QOUTPUT(1000000ULL);
	QOUTPUT(100000ULL);
	QOUTPUT(10000ULL);
	QOUTPUT(1000ULL);
	QOUTPUT(100ULL);
	QOUTPUT(10ULL);

	*p++ = n + '0';
	*p = '\0';
	return p;
}

char *
str_uint64_nodiv_noloop(char *dst, uint64_t n)
{
	char *p;
	int pad;

	pad = 0;
	p = dst;

	/*
	 * max uint64 is 18446744073709551615,
	 * most significant digit is 1 or 0.
	 */
	if (n >= 10000000000000000000ULL) {
		*p++ = '1';
		n -= 10000000000000000000ULL;
		pad = 1;
	}

#define QXOUTPUT(X)						\
	if (n < ((X) * 8)) {					\
		if (n < ((X) * 4)) {				\
			if (n < ((X) * 2)) {			\
				if (n < (X)) {			\
					if (pad)		\
						*p++ = '0';	\
				} else {			\
					*p++ = '1';		\
					n -= (X);		\
					pad = 1;		\
				}				\
			} else {				\
				pad = 1;			\
				if (n < ((X) * 3)) {		\
					*p++ = '2';		\
					n -= (X) * 2;		\
				} else {			\
					*p++ = '3';		\
					n -= (X) * 3;		\
				}				\
			}					\
		} else {					\
			pad = 1;				\
			if (n < ((X) * 6)) {			\
				if (n < ((X) * 5)) {		\
					*p++ = '4';		\
					n -= (X) * 4;		\
				} else {			\
					*p++ = '5';		\
					n -= (X) * 5;		\
				}				\
			} else {				\
				if (n < ((X) * 7)) {		\
					*p++ = '6';		\
					n -= (X) * 6;		\
				} else {			\
					*p++ = '7';		\
					n -= (X) * 7;		\
				}				\
			}					\
		}						\
	} else {						\
		pad = 1;					\
		if (n < ((X) * 9)) {				\
			*p++ = '8';				\
			n -= (X) * 8;				\
		} else {					\
			*p++ = '9';				\
			n -= (X) * 9;				\
		}						\
	}

	QXOUTPUT(1000000000000000000ULL);
	QXOUTPUT(100000000000000000ULL);
	QXOUTPUT(10000000000000000ULL);
	QXOUTPUT(1000000000000000ULL);
	QXOUTPUT(100000000000000ULL);
	QXOUTPUT(10000000000000ULL);
	QXOUTPUT(1000000000000ULL);
	QXOUTPUT(100000000000ULL);
	QXOUTPUT(10000000000ULL);
	QXOUTPUT(1000000000ULL);
	QXOUTPUT(100000000ULL);
	QXOUTPUT(10000000ULL);
	QXOUTPUT(1000000ULL);
	QXOUTPUT(100000ULL);
	QXOUTPUT(10000ULL);
	QXOUTPUT(1000ULL);
	QXOUTPUT(100ULL);
	QXOUTPUT(10ULL);

	*p++ = n + '0';
	*p = '\0';
	return p;
}

char buf[1024];

struct {
	const char *description;
	char *(*targetfunc)(char *, uint64_t);
} functable[] = {
	{	"sprintf",			sprintf_wrap		},
	{	"str_uint64_div_nomemcopy",	str_uint64_div_nomemcpy	},
	{	"str_uint64_div",		str_uint64_div		},
	{	"str_uint64_mulshift",		str_uint64_mulshift	},
	{	"str_uint64_nodiv",		str_uint64_nodiv	},
	{	"str_uint64_nodiv_noloop",	str_uint64_nodiv_noloop	},
};

struct {
	uint64_t n;
	const char *ok;
} testpattern[] = {
	{	1234567890ULL,			"1234567890"		},
	{	12345678901234567890ULL,	"12345678901234567890"	},
	{	987654321098765ULL,		"987654321098765"	},
	{	101010101010101010ULL,		"101010101010101010"	},
	{	0ULL,				"0"			},
	{	18446744073709551615ULL,	"18446744073709551615"	},
};


int test(void);
void benchmark(int);
void show_result(const char *, int, struct timespec *, struct timespec *);

#define	NLOOP_DEFAULT	10000000

int
main(int argc, char *argv[])
{
	int ch;
	int nflag, tflag;
	extern char *optarg;

	tflag = 0;
	nflag = NLOOP_DEFAULT;

	while ((ch = getopt(argc, argv, "n:t")) != -1) {
		switch (ch) {
		case 'n':
			nflag = atoi(optarg);
			break;
		case 't':
			tflag = 1;
			break;
		default:
			fprintf(stderr, "usage: %s [options]\n"
			    "\t-t	test only\n"
			    "\t-n <#>	specify loop count\n",
			    argv[0]);
			return 1;
		}
	}


	if (test() != 0)
		return 2;
	if (tflag)
		return 0;

	benchmark(nflag);

	return 0;
}


int
test(void)
{
	int funcno, i;

	for (funcno = 0; funcno < (sizeof(functable) / sizeof(functable[0]));
	    funcno++) {

		for (i = 0; i < (sizeof(testpattern) / sizeof(testpattern[0]));
		    i++) {

			functable[funcno].targetfunc(buf, testpattern[i].n);

			if (strcmp(testpattern[i].ok, buf) != 0) {
				fprintf(stderr,
				    "%s: implementation error: %llu -> \"%s\"\n",
				    functable[funcno].description,
				    (unsigned long long)testpattern[i].n, buf);
				return 1;
			}
		}

	}
	return 0;
}

void
benchmark(int nloop)
{
	struct timespec begin, end;
#define	BENCHMARK_BEGIN		clock_gettime(CLOCK_MONOTONIC, &begin)
#define	BENCHMARK_END		clock_gettime(CLOCK_MONOTONIC, &end)
#define	BENCHMARK_RESULT(t)	show_result(t, nloop, &begin, &end)
	int funcno;

	int i;

	printf("try %d loops for each functions\n", nloop);
	printf("-------------------------------\n");

	for (funcno = 0; funcno < (sizeof(functable) / sizeof(functable[0]));
	    funcno++) {

		BENCHMARK_BEGIN;

		for (i = 0; i < nloop; i++) {
			/* TEST CODE BEGIN */

			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);
			functable[funcno].targetfunc(buf, 9999999999999999999ULL);

			functable[funcno].targetfunc(buf, 12345678901234567890ULL);
			functable[funcno].targetfunc(buf, 1234567890123456789ULL);
			functable[funcno].targetfunc(buf, 123456789012345678ULL);
			functable[funcno].targetfunc(buf, 12345678901234567ULL);
			functable[funcno].targetfunc(buf, 1234567890123456ULL);
			functable[funcno].targetfunc(buf, 123456789012345ULL);
			functable[funcno].targetfunc(buf, 12345678901234ULL);
			functable[funcno].targetfunc(buf, 1234567890123ULL);
			functable[funcno].targetfunc(buf, 123456789012ULL);
			functable[funcno].targetfunc(buf, 12345678901ULL);
			functable[funcno].targetfunc(buf, 1234567890ULL);
			functable[funcno].targetfunc(buf, 123456789ULL);
			functable[funcno].targetfunc(buf, 12345678ULL);
			functable[funcno].targetfunc(buf, 1234567ULL);
			functable[funcno].targetfunc(buf, 123456ULL);
			functable[funcno].targetfunc(buf, 12345ULL);
			functable[funcno].targetfunc(buf, 1234ULL);
			functable[funcno].targetfunc(buf, 123ULL);
			functable[funcno].targetfunc(buf, 12ULL);
			functable[funcno].targetfunc(buf, 1ULL);

			/* TEST CODE END */
		}

		BENCHMARK_END;
		BENCHMARK_RESULT(functable[funcno].description);
	}
}

void
show_result(const char *title, int nloop, struct timespec *begin, struct timespec *end)
{
	struct timespec elapsed;
	double elapsed_f;
	static double ntimes = 0;

	timespecsub(end, begin, &elapsed);
	elapsed_f = elapsed.tv_sec + elapsed.tv_nsec / 1000000000.0;
	printf("%-32s %4llu.%09lu sec, ",
	    title, (unsigned long long)elapsed.tv_sec, elapsed.tv_nsec);
	printf(" %15.05f times/sec,  ",
	    nloop / elapsed_f);

	if (ntimes == 0) {
		ntimes = nloop / elapsed_f;
		printf("100.00%% (*standard)\n");
	} else {
		printf("%6.02f%%\n", 100.0 * (nloop / elapsed_f) / ntimes);
	}
}
