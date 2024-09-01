#include <sys/time.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOOPS_PER_ROUND 1048576

/* set once the end is reached, reset when setting an alarm */
static volatile int stop_now;

/* These are the functions to call for different pattern walk tests.
 * They are expected to return the word size when called with a NULL
 * area for indexed walks, or the same + 256 for pointer accesses.
 * The 3rd word (bits 16 to 24), indicates how many extra parallel
 * words are read at once.
 */
static unsigned int (*run[10])(void *area);
static const char *name[10];

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

static inline uint64_t rbit64(uint64_t x)
{
#ifdef __aarch64__
	asm volatile("rbit %0, %1\n" : "=r"(x) : "r"(x));
#else
#if defined(__x86_64__)
	__asm__("bswap %0" : "=r"(x) : "0"(x));
#else
	x = ((x & 0xffffffff00000000) >> 32) | ((x & 0x00000000ffffffff) << 32);
	x = ((x & 0xffff0000ffff0000) >> 16) | ((x & 0x0000ffff0000ffff) << 16);
	x = ((x & 0xff00ff00ff00ff00) >>  8) | ((x & 0x00ff00ff00ff00ff) <<  8);
#endif
	x = ((x & 0xf0f0f0f0f0f0f0f0) >>  4) | ((x & 0x0f0f0f0f0f0f0f0f) <<  4);
	x = ((x & 0xcccccccccccccccc) >>  2) | ((x & 0x3333333333333333) <<  2);
	x = ((x & 0xaaaaaaaaaaaaaaaa) >>  1) | ((x & 0x5555555555555555) <<  1);
#endif
	return x;
}

static void fill_area_32(void *area, size_t size)
{
	uint32_t *base = (uint32_t *)area;
	uint32_t ofs, addr;

	ofs = size / 2;
	for (addr = 0; addr < size / sizeof(uint32_t); addr++) {
		base[addr] = ofs;
		ofs += sizeof(*base);
		if (!(ofs & (ofs - 1))) {
			/* reached next power of two, restart from the lower one.
			 * E.g, for 4kB, 0x800->0xfff, 0x400->0x7ff, 0x200->0x3ff, ...
			 */
			ofs = (ofs / 4) & -sizeof(*base);
		}
	}
}

static void fill_area_64(void *area, size_t size)
{
	uint64_t *base = (uint64_t *)area;
	uint64_t ofs, addr;

	ofs = size / 2;
	for (addr = 0; addr < size / sizeof(*base); addr++) {
		base[addr] = ofs;
		ofs += sizeof(*base);
		if (!(ofs & (ofs - 1))) {
			/* reached next power of two, restart from the lower one.
			 * E.g, for 4kB, 0x800->0xfff, 0x400->0x7ff, 0x200->0x3ff, ...
			 */
			ofs = (ofs / 4) & ~sizeof(*base);
		}
	}
}

static void fill_area_ptr(void *area, size_t size)
{
	int shift;

	for (shift = 0; size >> shift > 1; shift++)
		;

	if (sizeof(void*) == 4) {
		uint32_t *base = (uint32_t *)area;
		uint32_t ofs, addr;

		ofs = size / 2;
		for (addr = 0; addr < size / sizeof(*base); addr++) {
			base[addr] = (long)base + ofs;
			ofs += sizeof(*base);
			if (!(ofs & (ofs - 1))) {
				/* reached next power of two, restart from the lower one.
				 * E.g, for 4kB, 0x800->0xfff, 0x400->0x7ff, 0x200->0x3ff, ...
				 */
				ofs = (ofs / 4) & -sizeof(*base);
			}
		}
	} else {
		uint64_t *base = (uint64_t *)area;
		uint64_t ofs, addr;

		ofs = size / 2;
		for (addr = 0; addr < size / sizeof(*base); addr++) {
			base[addr] = (long)base + ofs;
			ofs += sizeof(*base);
			if (!(ofs & (ofs - 1))) {
				/* reached next power of two, restart from the lower one.
				 * E.g, for 4kB, 0x800->0xfff, 0x400->0x7ff, 0x200->0x3ff, ...
				 */
				ofs = (ofs / 4) & -sizeof(*base);
			}
		}
	}
}


/*****************************************************************************
 *                            pointer accesses                               *
 *****************************************************************************/

/* runs the single pointer test, returns the number of rounds */
unsigned int run_1ptr_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	void **ofs0;

	if (!area)
		return 256 + sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = &((void**)area)[0];
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory reads
			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;

			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;

			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;

			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;
			ofs0 = *ofs0;
		}
		asm("" :: "r"(ofs0));
	}
	return rounds;
}

/* runs the dual pointer test, returns the number of rounds */
unsigned int run_2ptr_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	void **ofs0;
	void **ofs1;

	if (!area)
		return (1 << 16) + 256 + sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = (void**)area + 0;
		ofs1 = (void**)area + 64;
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory dual-reads
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;

			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;

			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;

			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
			ofs0 = *ofs0; ofs1 = *ofs1;
		}
		asm("" :: "r"(ofs0));
		asm("" :: "r"(ofs1));
	}
	return rounds;
}

/* runs the quad pointer test, returns the number of rounds */
unsigned int run_4ptr_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	void **ofs0;
	void **ofs1;
	void **ofs2;
	void **ofs3;

	if (!area)
		return (3 << 16) + 256 + sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = (void**)area + 0;
		ofs1 = (void**)area + 64;
		ofs2 = (void**)area + 128;
		ofs3 = (void**)area + 192;
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory quad-reads
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;

			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;

			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;

			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
		}
		asm("" :: "r"(ofs0));
		asm("" :: "r"(ofs1));
		asm("" :: "r"(ofs2));
		asm("" :: "r"(ofs3));
	}
	return rounds;
}

/* runs the 8-pointer test, returns the number of rounds */
unsigned int run_8ptr_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	void **ofs0;
	void **ofs1;
	void **ofs2;
	void **ofs3;
	void **ofs4;
	void **ofs5;
	void **ofs6;
	void **ofs7;

	if (!area)
		return (7 << 16) + 256 + sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = (void**)area + 0;
		ofs1 = (void**)area + 64;
		ofs2 = (void**)area + 128;
		ofs3 = (void**)area + 192;
		ofs4 = (void**)area + 256;
		ofs5 = (void**)area + 320;
		ofs6 = (void**)area + 384;
		ofs7 = (void**)area + 448;
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory octa-reads
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;

			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;

			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;

			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
			ofs0 = *ofs0; ofs1 = *ofs1; ofs2 = *ofs2; ofs3 = *ofs3;
			ofs4 = *ofs4; ofs5 = *ofs5; ofs6 = *ofs6; ofs7 = *ofs7;
		}
		asm("" :: "r"(ofs0));
		asm("" :: "r"(ofs1));
		asm("" :: "r"(ofs2));
		asm("" :: "r"(ofs3));
		asm("" :: "r"(ofs4));
		asm("" :: "r"(ofs5));
		asm("" :: "r"(ofs6));
		asm("" :: "r"(ofs7));
	}
	return rounds;
}

/*****************************************************************************
 *                              32-bit accesses                              *
 *****************************************************************************/

/* runs the single 32-bit test, returns the number of rounds */
unsigned int run_1w32_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	uint32_t ofs0;

	if (!area)
		return sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = 0;
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory reads
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);

			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);

			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);

			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs0 = *(uint32_t *)(area + ofs0);
		}
		asm("" :: "r"(ofs0));
	}
	return rounds;
}

/* runs the dual 32-bit test, returns the number of rounds */
unsigned int run_2w32_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	uint32_t ofs0;
	uint32_t ofs1;

	if (!area)
		return (1 << 16) + sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = 0;
		ofs1 = 64;
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory dual reads
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);

			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);

			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);

			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
			ofs0 = *(uint32_t *)(area + ofs0);
			ofs1 = *(uint32_t *)(area + ofs1);
		}
		asm("" :: "r"(ofs0));
		asm("" :: "r"(ofs1));
	}
	return rounds;
}


/*****************************************************************************
 *                              64-bit accesses                              *
 *****************************************************************************/

/* runs the single 64-bit test, returns the number of rounds */
unsigned int run_1w64_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	uint64_t ofs0;

	if (!area)
		return sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = 0;
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory reads
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);

			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);

			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);

			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs0 = *(uint64_t *)(area + ofs0);
		}
		asm("" :: "r"(ofs0));
	}
	return rounds;
}

/* runs the dual 64-bit test, returns the number of rounds */
unsigned int run_2w64_generic(void *area)
{
	unsigned int rounds;
	unsigned int loop;
	uint64_t ofs0;
	uint64_t ofs1;

	if (!area)
		return (1 << 16) + sizeof(ofs0);

	for (rounds = 0; !stop_now; rounds++) {
		ofs0 = 0;
		ofs1 = 64;
		for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
			// 16 memory dual reads
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);

			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);

			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);

			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
			ofs0 = *(uint64_t *)(area + ofs0);
			ofs1 = *(uint64_t *)(area + ofs1);
		}
		asm("" :: "r"(ofs0));
		asm("" :: "r"(ofs1));
	}
	return rounds;
}


/*****************************************************************************
 *                                 measurements                              *
 *****************************************************************************/

/* returns a timestamp in microseconds */
static inline uint64_t rdtsc()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

/* just marks the alarm as received */
void alarm_handler(int sig)
{
	stop_now = 1;
}

/* sets an alarm to trigger after <usec> microseconds. 0 disables it */
void set_alarm(unsigned int usec)
{
	struct itimerval timer = {
		.it_value.tv_usec = usec % 1000000,
		.it_value.tv_sec  = usec / 1000000,
	};

	if (usec) {
		stop_now = 0;
		signal(SIGALRM, alarm_handler);
		signal(SIGVTALRM, alarm_handler);
	}
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

/* Randomly accesses aligned words using function #<fct> over <size> bytes of
 * area <area> for about <usec> microseconds, then returns the number of words
 * read per microsecond. Note: size is rounded down to the lower power of two,
 * and must be at least 4kB.
 */
unsigned int random_read_over_area(void *area, unsigned int usec, size_t size, int fct)
{
	uint64_t rounds;
	uint64_t before, after;
	unsigned int word;

	if (fct >= sizeof(run) / sizeof(*run))
		return 0;

	if (!run[fct])
		return 0;

	word = run[fct](NULL);

	if (word & 256)
		fill_area_ptr(area, size);
	else {
		if ((word & 255) == 4)
			fill_area_32(area, size);
		else if ((word & 255) == 8)
			fill_area_64(area, size);
		else
			abort();
	}

	set_alarm(usec);
	after = rdtsc();
	before = rdtsc();
	before += before - after; // compensate for the syscall time

	rounds = run[fct](area);

	after = rdtsc();
	set_alarm(0);

	/* speed = transactions per millisecond. Use 64-bit computations to avoid
	 * overflows. The caller can turn this into bytes per second by multiplying
	 * by <word>.
	 */
	usec = after - before;
	if (usec < 1)
		usec = 1;
	rounds *= LOOPS_PER_ROUND;
	return rounds * 1000ULL / usec;
}

int main(int argc, char **argv)
{
	unsigned int usec;
	size_t size, size_max;
	void *area;
	unsigned int cols = 0;
	unsigned int wins = 0;
	unsigned int ret, word;
	int quiet = 0;
	int slowstart = 0;
	int fmt = 0;
	int fct;

	usec = 100000;
	size_max = 0;

	while (argc > 1 && *argv[1] == '-') {
		if (strcmp(argv[1], "-q") == 0) {
			quiet = 1;
		}
		else if (strcmp(argv[1], "-s") == 0) {
			slowstart = 1;
		}
		else if (strcmp(argv[1], "-b") == 0) {
			fmt = 1;
		}
		else if (strcmp(argv[1], "-n") == 0) {
			fmt = 2;
		}
		else if (argc > 1 && strcmp(argv[1], "-c") == 0) {
			/* -c col[,...] */
			char *next = argv[2];
			char *end;
			int col;

			while (*next) {
				col = strtol(next, &end, 0);
				if (!col || (*end != '\0' && *end != ','))
					break;
				cols |= 1 << (col - 1);
				if (*end == ',')
					end++;
				next = end;
			}
			argc--; argv++;
		}
		else if (argc > 1 && strcmp(argv[1], "-w") == 0) {
			/* -w size[,...] */
			char *next = argv[2];
			char *end;
			int win;

			while (*next) {
				win = strtol(next, &end, 0);
				if ((win < 12 || win > 31) || (*end != '\0' && *end != ','))
					break;
				if ((1U << win) > size_max)
					size_max = 1U << win;
				wins |= 1U << win;
				if (*end == ',')
					end++;
				next = end;
			}
			argc--; argv++;
		}
		else {
			fprintf(stderr,
				"Usage: prog [options]* <time_ms> <area_kB>\n"
				"  -b          report equivalent bandwidth in MB/s\n"
				"  -c <cols>   only emit these columns (1..N, ...)\n"
				"  -n          report output in nanosecond per access\n"
				"  -s          slowstart : pre-heat for 500ms to let cpufreq adapt\n"
				"  -w <sizes>  only test at these power of 2 sizes (12..31, ...)\n"
				"  -q          quiet : don't show column headers\n"
				"  -h          show this help\n"
				"");
			exit(!!strcmp(argv[1], "-h"));
		}
		argc--;
		argv++;
	}

	if (argc > 1)
		usec = atoi(argv[1]) * 1000;

	if (!size_max)
		size_max = 16 * 1048576;

	if (argc > 2)
		size_max = atol(argv[2]) * 1024;

	run[0] = run_1w32_generic;  name[0] = "1x32";
	run[1] = run_2w32_generic;  name[1] = "2x32";
	run[2] = run_1w64_generic;  name[2] = "1x64";
	run[3] = run_2w64_generic;  name[3] = "2x64";
	run[4] = run_1ptr_generic;  name[4] = "1xPTR";
	run[5] = run_2ptr_generic;  name[5] = "2xPTR";
	run[6] = run_4ptr_generic;  name[6] = "4xPTR";
	run[7] = run_8ptr_generic;  name[7] = "8xPTR";

	if (posix_memalign(&area, size_max / 4, size_max) != 0 || !area) {
		printf("Failed to allocate memory\n");
		exit(1);
	}

	if (slowstart) {
		set_alarm(500000);
		memset(area, 0, size_max);
		while (!stop_now);
		set_alarm(0);
	}

	if (!quiet) {
		int field;

		printf("   size:");
		for (field = 0; name[field]; field++) {
			if (cols && !(cols & (1 << field)))
				continue;
			if (fmt == 1 || fmt == 2)
				printf("%6s", name[field]);
			else
				printf("%8s", name[field]);
		}

		printf("\n");
	}

	for (size = 4096; size <= size_max || size <= wins; size *= 2) {
		if (wins && !(wins & size))
			continue;
		printf(quiet ? "%6u " : "%6uk: ", (unsigned int)(size >> 10U));
		for (fct = 0; run[fct]; fct++) {
			if (cols && !(cols & (1 << fct)))
				continue;
			ret = random_read_over_area(area, usec, size, fct);
			if (fmt == 1) {
				/* bandwidth in MB/s */
				word = run[fct](NULL);
				word = (word & 255) * (((word >> 16) & 255) + 1);
				printf("%5u ", ret * word / 1024U);
			} else if (fmt == 2) {
				/* nanoseconds per access */
				double lat = 1000000.0 / ret;
				if (lat < 10.0)
					printf("%1.3f ", lat);
				else if (lat < 100.0)
					printf("%2.2f ", lat);
				else if (lat < 1000.0)
					printf("%3.1f ", lat);
				else
					printf("%4.0f ", lat);
			} else {
				/* accesses per millisecond */
				printf("%7u ", ret);
			}
		}
		printf("\n");
	}

	exit(0);
}
