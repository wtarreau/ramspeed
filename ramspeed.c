#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct test_fct {
	const char *name;
	unsigned int (*f)(unsigned int, unsigned int);
};

static inline unsigned long long rdtsc()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

unsigned int bench_memcpy(unsigned int loop, unsigned int size)
{
	unsigned long long before, after;
	unsigned int i;
	char *src, *dst;

	src = malloc(size);
	dst = malloc(size);

	/* ensure the pages are allocated */
	memset(src, 0, size);
	memset(dst, 0, size);

	before = rdtsc();
	for (i = 0; i < loop; i++)
		memcpy(dst, src, size);
	after = rdtsc();

	free(src);
	free(dst);
	return after - before;
}

#ifdef __arm__
unsigned int bench_memcpy2(unsigned int loop, unsigned int size)
{
	unsigned long long before, after;
	unsigned int i;
	char *src, *dst;
	char *s, *d;

	src = malloc(size);
	dst = malloc(size);

	/* ensure the pages are allocated */
	memset(src, 0, size);
	memset(dst, 0, size);

	before = rdtsc();
	for (i = 0; i < loop; i++) {
		char *end = dst + size;
		d = dst; s = src;
		for (d = dst; d < end; /*ptr += 32*/)
			asm("ldmia %0!, { r4-r11 }\n\t"
			    "stmia %1!, { r4-r11 }\n\t"
			    : "=r" (s), "=r" (d) : "0" (d), "1" (s) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	}
	after = rdtsc();

	free(src);
	free(dst);
	return after - before;
}
#endif

unsigned int bench_memchr(unsigned int loop, unsigned int size)
{
	unsigned long long before, after;
	unsigned int i;
	char *dst;

	dst = malloc(size);

	/* ensure the pages are allocated */
	memset(dst, 0, size);

	before = rdtsc();
	for (i = 0; i < loop; i++)
		if (memchr(dst, i|1, size))
			return 0;
	after = rdtsc();

	free(dst);
	return after - before;
}

unsigned int bench_memset(unsigned int loop, unsigned int size)
{
	unsigned long long before, after;
	unsigned int i;
	char *dst;

	dst = malloc(size);

	/* ensure the pages are allocated */
	memset(dst, 0, size);

	before = rdtsc();
	for (i = 0; i < loop; i++)
		memset(dst, i, size);
	after = rdtsc();

	free(dst);
	return after - before;
}

#ifdef __arm__
unsigned int bench_write32(unsigned int loop, unsigned int size)
{
	unsigned long long before, after;
	unsigned int i;
	char *dst, *ptr, *end;

	dst = malloc(size);

	/* ensure the pages are allocated */
	memset(dst, 0, size);

	before = rdtsc();
	for (i = 0; i < loop; i++) {
		end = dst + size;
		for (ptr = dst; ptr < end; /*ptr += 32*/)
			asm("stmia %0!, { r4-r11 }" : "=r" (ptr) : "0" (ptr) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	}
	after = rdtsc();

	free(dst);
	return after - before;
}

unsigned int bench_memchr_arm(unsigned int loop, unsigned int size)
{
	unsigned long long before, after;
	unsigned int i;
	char *dst, *ptr, *end;

	dst = malloc(size);

	/* ensure the pages are allocated */
	memset(dst, 0, size);

	before = rdtsc();
	for (i = 0; i < loop; i++) {
		end = dst + size;
		for (ptr = dst; ptr < end; ptr += 32)
			asm("ldmia %0, { r4-r11 }" :: "r" (ptr) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
		//for (ptr = dst; ptr < end; ptr += 16)
		//	asm("ldmia %0, { r4-r7 }" :: "r" (ptr) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
		//for (ptr = dst; ptr < end; ptr += 8)
		//	asm("ldmia %0, { r4-r5 }" :: "r" (ptr) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
		//for (ptr = dst; ptr < end; ptr += 4)
		//	asm("ldm %0, { r4 }" :: "r" (ptr) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	}
	after = rdtsc();

	free(dst);
	return after - before;
}

unsigned int bench_prefetch(unsigned int loop, unsigned int size)
{
	unsigned long long before, after;
	unsigned int i;
	char *dst, *ptr, *end;

	dst = malloc(size);

	/* ensure the pages are allocated */
	memset(dst, 0, size);

	before = rdtsc();
	for (i = 0; i < loop; i++) {
		/* only prefetch the first word of each 32-byte cacheline. The
		 * code has almost no effect on the speed, only the bandwidth
		 * counts. Fetching 128 bytes at once instead of 32 doesn't help
		 * for instance.
		 */
		end = dst + size;
		for (ptr = dst; ptr < end; ptr += 32) {
			asm("pld [%0]\n\t" :: "r" (ptr));
		}
	}
	after = rdtsc();

	free(dst);
	return after - before;
}
#endif

static struct test_fct test_fcts[] = {
#ifdef __arm__
	{ "bench_prefetch", bench_prefetch },
	{ "bench_memchr_arm", bench_memchr_arm },
	{ "bench_write32", bench_write32 },
#endif
	{ "bench_memchr", bench_memchr },
	{ "bench_memset", bench_memset },
	{ "bench_memcpy", bench_memcpy },
#ifdef __arm__
	//{ "bench_memcpy2", bench_memcpy2 },
#endif
	{ NULL, NULL }
};

void run_bench(unsigned int loop, unsigned int size)
{
	unsigned int usecs;
	struct test_fct *t = test_fcts;

	printf("Running tests with %d loops of %d bytes\n", loop, size);

	while (t->name && t->f) {
		usecs = t->f(loop, size);
		printf("%s: %d us for %d loops of %d bytes = %lld kB/s\n",
		       t->name,
		       usecs, loop, size, (unsigned long long)1024ULL * loop * size / usecs);
		t++;
	}
}

main(int argc, char **argv)
{
	unsigned int size;
	unsigned int loop;

	loop = 10;
	size = 16777216;

	if (argc > 1)
		loop = atoi(argv[1]);

	if (argc > 2)
		size = atoi(argv[2]);

	run_bench(loop, size);
	exit(0);
}
