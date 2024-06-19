#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define ENTRIES (1U << 28)
#define SHIFT   (32 - 28)
#define MASK    (~0U >> SHIFT)

static inline uint32_t rbit32(uint32_t x)
{
#ifdef __aarch64__
	asm volatile("rbit %w0, %w1\n" : "=r"(x) : "r"(x));
#else
#if defined(__x86_64__) || defined(__i486__)
	__asm__("bswap %0" : "=r"(x) : "0"(x));
#else
	x = ((x & 0xffff0000) >> 16) | ((x & 0x0000ffff) << 16);
	x = ((x & 0xff00ff00) >>  8) | ((x & 0x00ff00ff) <<  8);
#endif
	x = ((x & 0xf0f0f0f0) >>  4) | ((x & 0x0f0f0f0f) <<  4);
	x = ((x & 0xcccccccc) >>  2) | ((x & 0x33333333) <<  2);
	x = ((x & 0xaaaaaaaa) >>  1) | ((x & 0x55555555) <<  1);
#endif
	return x;
}

static void fill_area(uint32_t *area)
{
	uint32_t addr;

	for (addr = 0; addr < ENTRIES; addr++) {
		area[addr] = rbit32((rbit32(addr << SHIFT) + 1) << SHIFT);
	}
}

static void scan_area(const uint32_t *area, int rounds)
{
	uint32_t next, ent;

	while (rounds--) {
		next = 0;
		for (ent = next = 0; ent < ENTRIES; ent++)
			next = area[next];
		asm("" :: "r"(next));
	}
}

int main(int argc, char **argv)
{
	int rounds = 1;
	void *area;

	if (argc > 1)
		rounds = atoi(argv[1]);

	printf("malloc 1 GB...\n");
	area = calloc(1, ENTRIES * 4);
	if (!area)
		return 1;
	printf("fill...\n");
	fill_area(area);
	printf("scan %d times...\n", rounds);
	scan_area(area, rounds);
	return 0;
}
