#ifdef __SSE4_1__
#include <x86intrin.h>
#endif

#include <sys/time.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
#define HAS_MANY_REGISTERS 0
#else
#define HAS_MANY_REGISTERS 1
#endif

#define LOOPS_PER_ROUND 65536

/* set once the end is reached, reset when setting an alarm */
static volatile int stop_now;

/* These are the functions to call for word sizes of 2^0 to 2^6 */
unsigned int (*run[7])(void *area, size_t mask);


/*****************************************************************************
 *                              8-bit accesses                               *
 *****************************************************************************/

static inline void read8(const char *addr, const unsigned long ofs)
{
	asm volatile("" : : "r" (*(uint8_t *)(addr + ofs)));
}

static inline void read8_dual(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint8_t *)(addr + ofs1)), "r" (*(uint8_t *)(addr + ofs2)));
	}
	else {
		asm volatile("" : : "r" (*(uint8_t *)(addr + ofs1)));
		asm volatile("" : : "r" (*(uint8_t *)(addr + ofs2)));
	}
}

/* runs the 8-bit test, returns the number of rounds */
unsigned int run8_generic(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read8_dual(addr + 0000,   0, 512 +   0);
			read8_dual(addr + 0000, 256, 512 + 256);
			read8_dual(addr + 0000, 128, 512 + 128);
			read8_dual(addr + 0000, 384, 512 + 384);
			read8_dual(addr + 0000, 320, 512 + 320);
			read8_dual(addr + 0000,  64, 512 +  64);
			read8_dual(addr + 0000, 192, 512 + 192);
			read8_dual(addr + 0000, 448, 512 + 448);

			read8_dual(addr + 1024,   0, 512 +   0);
			read8_dual(addr + 1024, 256, 512 + 256);
			read8_dual(addr + 1024, 128, 512 + 128);
			read8_dual(addr + 1024, 384, 512 + 384);
			read8_dual(addr + 1024, 320, 512 + 320);
			read8_dual(addr + 1024,  64, 512 +  64);
			read8_dual(addr + 1024, 192, 512 + 192);
			read8_dual(addr + 1024, 448, 512 + 448);

			read8_dual(addr + 2048,   0, 512 +   0);
			read8_dual(addr + 2048, 256, 512 + 256);
			read8_dual(addr + 2048, 128, 512 + 128);
			read8_dual(addr + 2048, 384, 512 + 384);
			read8_dual(addr + 2048, 320, 512 + 320);
			read8_dual(addr + 2048,  64, 512 +  64);
			read8_dual(addr + 2048, 192, 512 + 192);
			read8_dual(addr + 2048, 448, 512 + 448);

			read8_dual(addr + 3072,   0, 512 +   0);
			read8_dual(addr + 3072, 256, 512 + 256);
			read8_dual(addr + 3072, 128, 512 + 128);
			read8_dual(addr + 3072, 384, 512 + 384);
			read8_dual(addr + 3072, 320, 512 + 320);
			read8_dual(addr + 3072,  64, 512 +  64);
			read8_dual(addr + 3072, 192, 512 + 192);
			read8_dual(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}


/*****************************************************************************
 *                              16-bit accesses                              *
 *****************************************************************************/

static inline void read16(const char *addr, const unsigned long ofs)
{
	asm volatile("" : : "r" (*(uint16_t *)(addr + ofs)));
}

static inline void read16_dual(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint16_t *)(addr + ofs1)), "r" (*(uint16_t *)(addr + ofs2)));
	}
	else {
		asm volatile("" : : "r" (*(uint16_t *)(addr + ofs1)));
		asm volatile("" : : "r" (*(uint16_t *)(addr + ofs2)));
	}
}

/* runs the 16-bit test, returns the number of rounds */
unsigned int run16_generic(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read16_dual(addr + 0000,   0, 512 +   0);
			read16_dual(addr + 0000, 256, 512 + 256);
			read16_dual(addr + 0000, 128, 512 + 128);
			read16_dual(addr + 0000, 384, 512 + 384);
			read16_dual(addr + 0000, 320, 512 + 320);
			read16_dual(addr + 0000,  64, 512 +  64);
			read16_dual(addr + 0000, 192, 512 + 192);
			read16_dual(addr + 0000, 448, 512 + 448);

			read16_dual(addr + 1024,   0, 512 +   0);
			read16_dual(addr + 1024, 256, 512 + 256);
			read16_dual(addr + 1024, 128, 512 + 128);
			read16_dual(addr + 1024, 384, 512 + 384);
			read16_dual(addr + 1024, 320, 512 + 320);
			read16_dual(addr + 1024,  64, 512 +  64);
			read16_dual(addr + 1024, 192, 512 + 192);
			read16_dual(addr + 1024, 448, 512 + 448);

			read16_dual(addr + 2048,   0, 512 +   0);
			read16_dual(addr + 2048, 256, 512 + 256);
			read16_dual(addr + 2048, 128, 512 + 128);
			read16_dual(addr + 2048, 384, 512 + 384);
			read16_dual(addr + 2048, 320, 512 + 320);
			read16_dual(addr + 2048,  64, 512 +  64);
			read16_dual(addr + 2048, 192, 512 + 192);
			read16_dual(addr + 2048, 448, 512 + 448);

			read16_dual(addr + 3072,   0, 512 +   0);
			read16_dual(addr + 3072, 256, 512 + 256);
			read16_dual(addr + 3072, 128, 512 + 128);
			read16_dual(addr + 3072, 384, 512 + 384);
			read16_dual(addr + 3072, 320, 512 + 320);
			read16_dual(addr + 3072,  64, 512 +  64);
			read16_dual(addr + 3072, 192, 512 + 192);
			read16_dual(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}


/*****************************************************************************
 *                              32-bit accesses                              *
 *****************************************************************************/

static inline void read32(const char *addr, const unsigned long ofs)
{
	asm volatile("" : : "r" (*(uint32_t *)(addr + ofs)));
}

static inline void read32_dual(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint32_t *)(addr + ofs1)), "r" (*(uint32_t *)(addr + ofs2)));
	}
	else {
		asm volatile("" : : "r" (*(uint32_t *)(addr + ofs1)));
		asm volatile("" : : "r" (*(uint32_t *)(addr + ofs2)));
	}
}

/* runs the 32-bit test, returns the number of rounds */
unsigned int run32_generic(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read32_dual(addr + 0000,   0, 512 +   0);
			read32_dual(addr + 0000, 256, 512 + 256);
			read32_dual(addr + 0000, 128, 512 + 128);
			read32_dual(addr + 0000, 384, 512 + 384);
			read32_dual(addr + 0000, 320, 512 + 320);
			read32_dual(addr + 0000,  64, 512 +  64);
			read32_dual(addr + 0000, 192, 512 + 192);
			read32_dual(addr + 0000, 448, 512 + 448);

			read32_dual(addr + 1024,   0, 512 +   0);
			read32_dual(addr + 1024, 256, 512 + 256);
			read32_dual(addr + 1024, 128, 512 + 128);
			read32_dual(addr + 1024, 384, 512 + 384);
			read32_dual(addr + 1024, 320, 512 + 320);
			read32_dual(addr + 1024,  64, 512 +  64);
			read32_dual(addr + 1024, 192, 512 + 192);
			read32_dual(addr + 1024, 448, 512 + 448);

			read32_dual(addr + 2048,   0, 512 +   0);
			read32_dual(addr + 2048, 256, 512 + 256);
			read32_dual(addr + 2048, 128, 512 + 128);
			read32_dual(addr + 2048, 384, 512 + 384);
			read32_dual(addr + 2048, 320, 512 + 320);
			read32_dual(addr + 2048,  64, 512 +  64);
			read32_dual(addr + 2048, 192, 512 + 192);
			read32_dual(addr + 2048, 448, 512 + 448);

			read32_dual(addr + 3072,   0, 512 +   0);
			read32_dual(addr + 3072, 256, 512 + 256);
			read32_dual(addr + 3072, 128, 512 + 128);
			read32_dual(addr + 3072, 384, 512 + 384);
			read32_dual(addr + 3072, 320, 512 + 320);
			read32_dual(addr + 3072,  64, 512 +  64);
			read32_dual(addr + 3072, 192, 512 + 192);
			read32_dual(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}


/*****************************************************************************
 *                              64-bit accesses                              *
 *****************************************************************************/

static inline void read64(const char *addr, const unsigned long ofs)
{
	asm volatile("" : : "r" (*(uint64_t *)(addr + ofs)));
}

static inline void read64_dual(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1)), "r" (*(uint64_t *)(addr + ofs2)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2)));
	}
}

/* runs the 64-bit test, returns the number of rounds */
unsigned int run64_generic(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read64_dual(addr + 0000,   0, 512 +   0);
			read64_dual(addr + 0000, 256, 512 + 256);
			read64_dual(addr + 0000, 128, 512 + 128);
			read64_dual(addr + 0000, 384, 512 + 384);
			read64_dual(addr + 0000, 320, 512 + 320);
			read64_dual(addr + 0000,  64, 512 +  64);
			read64_dual(addr + 0000, 192, 512 + 192);
			read64_dual(addr + 0000, 448, 512 + 448);

			read64_dual(addr + 1024,   0, 512 +   0);
			read64_dual(addr + 1024, 256, 512 + 256);
			read64_dual(addr + 1024, 128, 512 + 128);
			read64_dual(addr + 1024, 384, 512 + 384);
			read64_dual(addr + 1024, 320, 512 + 320);
			read64_dual(addr + 1024,  64, 512 +  64);
			read64_dual(addr + 1024, 192, 512 + 192);
			read64_dual(addr + 1024, 448, 512 + 448);

			read64_dual(addr + 2048,   0, 512 +   0);
			read64_dual(addr + 2048, 256, 512 + 256);
			read64_dual(addr + 2048, 128, 512 + 128);
			read64_dual(addr + 2048, 384, 512 + 384);
			read64_dual(addr + 2048, 320, 512 + 320);
			read64_dual(addr + 2048,  64, 512 +  64);
			read64_dual(addr + 2048, 192, 512 + 192);
			read64_dual(addr + 2048, 448, 512 + 448);

			read64_dual(addr + 3072,   0, 512 +   0);
			read64_dual(addr + 3072, 256, 512 + 256);
			read64_dual(addr + 3072, 128, 512 + 128);
			read64_dual(addr + 3072, 384, 512 + 384);
			read64_dual(addr + 3072, 320, 512 + 320);
			read64_dual(addr + 3072,  64, 512 +  64);
			read64_dual(addr + 3072, 192, 512 + 192);
			read64_dual(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}


#ifdef __SSE4_1__
/* same with two addresses at once */
static inline void read64_dual_sse(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	__m128i xmm0, xmm1;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1) :
	             "0" (_mm_loadl_epi64((void *)(addr + ofs1))),
	             "1" (_mm_loadl_epi64((void *)(addr + ofs2))));
}

/* runs the 64-bit test using SSE optimizations, returns the number of rounds */
unsigned int run64_sse(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read64_dual_sse(addr + 0000,   0, 512 +   0);
			read64_dual_sse(addr + 0000, 256, 512 + 256);
			read64_dual_sse(addr + 0000, 128, 512 + 128);
			read64_dual_sse(addr + 0000, 384, 512 + 384);
			read64_dual_sse(addr + 0000, 320, 512 + 320);
			read64_dual_sse(addr + 0000,  64, 512 +  64);
			read64_dual_sse(addr + 0000, 192, 512 + 192);
			read64_dual_sse(addr + 0000, 448, 512 + 448);
			read64_dual_sse(addr + 1024,   0, 512 +   0);
			read64_dual_sse(addr + 1024, 256, 512 + 256);
			read64_dual_sse(addr + 1024, 128, 512 + 128);
			read64_dual_sse(addr + 1024, 384, 512 + 384);
			read64_dual_sse(addr + 1024, 320, 512 + 320);
			read64_dual_sse(addr + 1024,  64, 512 +  64);
			read64_dual_sse(addr + 1024, 192, 512 + 192);
			read64_dual_sse(addr + 1024, 448, 512 + 448);
			read64_dual_sse(addr + 2048,   0, 512 +   0);
			read64_dual_sse(addr + 2048, 256, 512 + 256);
			read64_dual_sse(addr + 2048, 128, 512 + 128);
			read64_dual_sse(addr + 2048, 384, 512 + 384);
			read64_dual_sse(addr + 2048, 320, 512 + 320);
			read64_dual_sse(addr + 2048,  64, 512 +  64);
			read64_dual_sse(addr + 2048, 192, 512 + 192);
			read64_dual_sse(addr + 2048, 448, 512 + 448);
			read64_dual_sse(addr + 3072,   0, 512 +   0);
			read64_dual_sse(addr + 3072, 256, 512 + 256);
			read64_dual_sse(addr + 3072, 128, 512 + 128);
			read64_dual_sse(addr + 3072, 384, 512 + 384);
			read64_dual_sse(addr + 3072, 320, 512 + 320);
			read64_dual_sse(addr + 3072,  64, 512 +  64);
			read64_dual_sse(addr + 3072, 192, 512 + 192);
			read64_dual_sse(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}
#endif


#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
/* same with two addresses at once */
static inline void read64_dual_vfp(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	asm volatile("vldr %%d4, [%0,%1]\n\t"
	             "vldr %%d5, [%0,%2]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs1), "I" (ofs2)
	             : "%d4", "%d5");
}

/* runs the 64-bit test using VFP optimizations, returns the number of rounds */
unsigned int run64_vfp(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read64_dual_vfp(addr + 0000,   0, 512 +   0);
			read64_dual_vfp(addr + 0000, 256, 512 + 256);
			read64_dual_vfp(addr + 0000, 128, 512 + 128);
			read64_dual_vfp(addr + 0000, 384, 512 + 384);
			read64_dual_vfp(addr + 0000, 320, 512 + 320);
			read64_dual_vfp(addr + 0000,  64, 512 +  64);
			read64_dual_vfp(addr + 0000, 192, 512 + 192);
			read64_dual_vfp(addr + 0000, 448, 512 + 448);
			read64_dual_vfp(addr + 1024,   0, 512 +   0);
			read64_dual_vfp(addr + 1024, 256, 512 + 256);
			read64_dual_vfp(addr + 1024, 128, 512 + 128);
			read64_dual_vfp(addr + 1024, 384, 512 + 384);
			read64_dual_vfp(addr + 1024, 320, 512 + 320);
			read64_dual_vfp(addr + 1024,  64, 512 +  64);
			read64_dual_vfp(addr + 1024, 192, 512 + 192);
			read64_dual_vfp(addr + 1024, 448, 512 + 448);
			read64_dual_vfp(addr + 2048,   0, 512 +   0);
			read64_dual_vfp(addr + 2048, 256, 512 + 256);
			read64_dual_vfp(addr + 2048, 128, 512 + 128);
			read64_dual_vfp(addr + 2048, 384, 512 + 384);
			read64_dual_vfp(addr + 2048, 320, 512 + 320);
			read64_dual_vfp(addr + 2048,  64, 512 +  64);
			read64_dual_vfp(addr + 2048, 192, 512 + 192);
			read64_dual_vfp(addr + 2048, 448, 512 + 448);
			read64_dual_vfp(addr + 3072,   0, 512 +   0);
			read64_dual_vfp(addr + 3072, 256, 512 + 256);
			read64_dual_vfp(addr + 3072, 128, 512 + 128);
			read64_dual_vfp(addr + 3072, 384, 512 + 384);
			read64_dual_vfp(addr + 3072, 320, 512 + 320);
			read64_dual_vfp(addr + 3072,  64, 512 +  64);
			read64_dual_vfp(addr + 3072, 192, 512 + 192);
			read64_dual_vfp(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}
#endif


#if defined(__ARM_ARCH_7A__)
/* same with two addresses at once */
static inline void read64_dual_armv7(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	uint32_t r0, r1;
	asm volatile("ldrd %0, %1, [%2,%3]" : "=r" (r0), "=r" (r1) : "r" (addr), "I" (ofs1));
	asm volatile("ldrd %0, %1, [%2,%3]" : "=r" (r0), "=r" (r1) : "r" (addr), "I" (ofs2));
}

/* runs the 64-bit test using ARMv7 optimizations, returns the number of rounds */
unsigned int run64_armv7(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read64_dual_armv7(addr + 0000,   0, 512 +   0);
			read64_dual_armv7(addr + 0000, 256, 512 + 256);
			read64_dual_armv7(addr + 0000, 128, 512 + 128);
			read64_dual_armv7(addr + 0000, 384, 512 + 384);
			read64_dual_armv7(addr + 0000, 320, 512 + 320);
			read64_dual_armv7(addr + 0000,  64, 512 +  64);
			read64_dual_armv7(addr + 0000, 192, 512 + 192);
			read64_dual_armv7(addr + 0000, 448, 512 + 448);
			read64_dual_armv7(addr + 1024,   0, 512 +   0);
			read64_dual_armv7(addr + 1024, 256, 512 + 256);
			read64_dual_armv7(addr + 1024, 128, 512 + 128);
			read64_dual_armv7(addr + 1024, 384, 512 + 384);
			read64_dual_armv7(addr + 1024, 320, 512 + 320);
			read64_dual_armv7(addr + 1024,  64, 512 +  64);
			read64_dual_armv7(addr + 1024, 192, 512 + 192);
			read64_dual_armv7(addr + 1024, 448, 512 + 448);
			read64_dual_armv7(addr + 2048,   0, 512 +   0);
			read64_dual_armv7(addr + 2048, 256, 512 + 256);
			read64_dual_armv7(addr + 2048, 128, 512 + 128);
			read64_dual_armv7(addr + 2048, 384, 512 + 384);
			read64_dual_armv7(addr + 2048, 320, 512 + 320);
			read64_dual_armv7(addr + 2048,  64, 512 +  64);
			read64_dual_armv7(addr + 2048, 192, 512 + 192);
			read64_dual_armv7(addr + 2048, 448, 512 + 448);
			read64_dual_armv7(addr + 3072,   0, 512 +   0);
			read64_dual_armv7(addr + 3072, 256, 512 + 256);
			read64_dual_armv7(addr + 3072, 128, 512 + 128);
			read64_dual_armv7(addr + 3072, 384, 512 + 384);
			read64_dual_armv7(addr + 3072, 320, 512 + 320);
			read64_dual_armv7(addr + 3072,  64, 512 +  64);
			read64_dual_armv7(addr + 3072, 192, 512 + 192);
			read64_dual_armv7(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}
#endif


/*****************************************************************************
 *                             128-bit accesses                              *
 *****************************************************************************/

static inline void read128(const char *addr, const unsigned long ofs)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : :
		             "r" (*(uint64_t *)(addr + ofs + 0)),
		             "r" (*(uint64_t *)(addr + ofs + 8)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 0)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 8)));
	}
}

static inline void read128_dual(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : :
		             "r" (*(uint64_t *)(addr + ofs1 + 0)),
		             "r" (*(uint64_t *)(addr + ofs1 + 8)),
		             "r" (*(uint64_t *)(addr + ofs2 + 0)),
		             "r" (*(uint64_t *)(addr + ofs2 + 8)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 + 0)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 + 0)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 + 8)));
	}
}

/* runs the 128-bit test, returns the number of rounds */
unsigned int run128_generic(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read128_dual(addr + 0000,   0, 512 +   0);
			read128_dual(addr + 0000, 256, 512 + 256);
			read128_dual(addr + 0000, 128, 512 + 128);
			read128_dual(addr + 0000, 384, 512 + 384);
			read128_dual(addr + 0000, 320, 512 + 320);
			read128_dual(addr + 0000,  64, 512 +  64);
			read128_dual(addr + 0000, 192, 512 + 192);
			read128_dual(addr + 0000, 448, 512 + 448);

			read128_dual(addr + 1024,   0, 512 +   0);
			read128_dual(addr + 1024, 256, 512 + 256);
			read128_dual(addr + 1024, 128, 512 + 128);
			read128_dual(addr + 1024, 384, 512 + 384);
			read128_dual(addr + 1024, 320, 512 + 320);
			read128_dual(addr + 1024,  64, 512 +  64);
			read128_dual(addr + 1024, 192, 512 + 192);
			read128_dual(addr + 1024, 448, 512 + 448);

			read128_dual(addr + 2048,   0, 512 +   0);
			read128_dual(addr + 2048, 256, 512 + 256);
			read128_dual(addr + 2048, 128, 512 + 128);
			read128_dual(addr + 2048, 384, 512 + 384);
			read128_dual(addr + 2048, 320, 512 + 320);
			read128_dual(addr + 2048,  64, 512 +  64);
			read128_dual(addr + 2048, 192, 512 + 192);
			read128_dual(addr + 2048, 448, 512 + 448);

			read128_dual(addr + 3072,   0, 512 +   0);
			read128_dual(addr + 3072, 256, 512 + 256);
			read128_dual(addr + 3072, 128, 512 + 128);
			read128_dual(addr + 3072, 384, 512 + 384);
			read128_dual(addr + 3072, 320, 512 + 320);
			read128_dual(addr + 3072,  64, 512 +  64);
			read128_dual(addr + 3072, 192, 512 + 192);
			read128_dual(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}


#ifdef __SSE4_1__
static inline void read128_sse(const char *addr, const unsigned long ofs)
{
	__m128i xmm0;
	asm volatile("" : "=xm" (xmm0) : "0" (_mm_load_si128((void *)(addr + ofs))));
}

static inline void read128_dual_sse(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	__m128i xmm0, xmm1;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1) :
	             "0" (_mm_load_si128((void *)(addr + ofs1))),
	             "1" (_mm_load_si128((void *)(addr + ofs2))));
}

/* runs the 128-bit test, returns the number of rounds */
unsigned int run128_sse(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read128_dual_sse(addr + 0000,   0, 512 +   0);
			read128_dual_sse(addr + 0000, 256, 512 + 256);
			read128_dual_sse(addr + 0000, 128, 512 + 128);
			read128_dual_sse(addr + 0000, 384, 512 + 384);
			read128_dual_sse(addr + 0000, 320, 512 + 320);
			read128_dual_sse(addr + 0000,  64, 512 +  64);
			read128_dual_sse(addr + 0000, 192, 512 + 192);
			read128_dual_sse(addr + 0000, 448, 512 + 448);
			read128_dual_sse(addr + 1024,   0, 512 +   0);
			read128_dual_sse(addr + 1024, 256, 512 + 256);
			read128_dual_sse(addr + 1024, 128, 512 + 128);
			read128_dual_sse(addr + 1024, 384, 512 + 384);
			read128_dual_sse(addr + 1024, 320, 512 + 320);
			read128_dual_sse(addr + 1024,  64, 512 +  64);
			read128_dual_sse(addr + 1024, 192, 512 + 192);
			read128_dual_sse(addr + 1024, 448, 512 + 448);
			read128_dual_sse(addr + 2048,   0, 512 +   0);
			read128_dual_sse(addr + 2048, 256, 512 + 256);
			read128_dual_sse(addr + 2048, 128, 512 + 128);
			read128_dual_sse(addr + 2048, 384, 512 + 384);
			read128_dual_sse(addr + 2048, 320, 512 + 320);
			read128_dual_sse(addr + 2048,  64, 512 +  64);
			read128_dual_sse(addr + 2048, 192, 512 + 192);
			read128_dual_sse(addr + 2048, 448, 512 + 448);
			read128_dual_sse(addr + 3072,   0, 512 +   0);
			read128_dual_sse(addr + 3072, 256, 512 + 256);
			read128_dual_sse(addr + 3072, 128, 512 + 128);
			read128_dual_sse(addr + 3072, 384, 512 + 384);
			read128_dual_sse(addr + 3072, 320, 512 + 320);
			read128_dual_sse(addr + 3072,  64, 512 +  64);
			read128_dual_sse(addr + 3072, 192, 512 + 192);
			read128_dual_sse(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}
#endif

#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
static inline void read128_vfp(const char *addr, const unsigned long ofs)
{
	/* Here the only way to get it done properly is to do it by hand :-( */
	asm volatile("vldr %%d4, [%0,%1+0]\n\t"
	             "vldr %%d5, [%0,%1+8]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs)
	             : "%d4", "%d5");
}

static inline void read128_dual_vfp(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	/* Here the only way to get it done properly is to do it by hand :-( */
	asm volatile("vldr %%d4, [%0,%1]\n\t"
	             "vldr %%d5, [%0,%1+8]\n\t"
	             "vldr %%d6, [%0,%2]\n\t"
	             "vldr %%d7, [%0,%2+8]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs1), "I" (ofs2)
	             : "%d4", "%d5", "%d6", "%d7");
}

/* runs the 128-bit test, returns the number of rounds */
unsigned int run128_vfp(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read128_dual_vfp(addr + 0000,   0, 512 +   0);
			read128_dual_vfp(addr + 0000, 256, 512 + 256);
			read128_dual_vfp(addr + 0000, 128, 512 + 128);
			read128_dual_vfp(addr + 0000, 384, 512 + 384);
			read128_dual_vfp(addr + 0000, 320, 512 + 320);
			read128_dual_vfp(addr + 0000,  64, 512 +  64);
			read128_dual_vfp(addr + 0000, 192, 512 + 192);
			read128_dual_vfp(addr + 0000, 448, 512 + 448);
			read128_dual_vfp(addr + 1024,   0, 512 +   0);
			read128_dual_vfp(addr + 1024, 256, 512 + 256);
			read128_dual_vfp(addr + 1024, 128, 512 + 128);
			read128_dual_vfp(addr + 1024, 384, 512 + 384);
			read128_dual_vfp(addr + 1024, 320, 512 + 320);
			read128_dual_vfp(addr + 1024,  64, 512 +  64);
			read128_dual_vfp(addr + 1024, 192, 512 + 192);
			read128_dual_vfp(addr + 1024, 448, 512 + 448);
			read128_dual_vfp(addr + 2048,   0, 512 +   0);
			read128_dual_vfp(addr + 2048, 256, 512 + 256);
			read128_dual_vfp(addr + 2048, 128, 512 + 128);
			read128_dual_vfp(addr + 2048, 384, 512 + 384);
			read128_dual_vfp(addr + 2048, 320, 512 + 320);
			read128_dual_vfp(addr + 2048,  64, 512 +  64);
			read128_dual_vfp(addr + 2048, 192, 512 + 192);
			read128_dual_vfp(addr + 2048, 448, 512 + 448);
			read128_dual_vfp(addr + 3072,   0, 512 +   0);
			read128_dual_vfp(addr + 3072, 256, 512 + 256);
			read128_dual_vfp(addr + 3072, 128, 512 + 128);
			read128_dual_vfp(addr + 3072, 384, 512 + 384);
			read128_dual_vfp(addr + 3072, 320, 512 + 320);
			read128_dual_vfp(addr + 3072,  64, 512 +  64);
			read128_dual_vfp(addr + 3072, 192, 512 + 192);
			read128_dual_vfp(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}
#endif


/*****************************************************************************
 *                             256-bit accesses                              *
 *****************************************************************************/

static inline void read256(const char *addr, const unsigned long ofs)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs +  0)), "r" (*(uint64_t *)(addr + ofs +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 16)), "r" (*(uint64_t *)(addr + ofs + 24)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs +  0)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 24)));
	}
}

static inline void read256_dual(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 +  0)), "r" (*(uint64_t *)(addr + ofs1 +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 + 16)), "r" (*(uint64_t *)(addr + ofs1 + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 +  0)), "r" (*(uint64_t *)(addr + ofs2 +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 + 16)), "r" (*(uint64_t *)(addr + ofs2 + 24)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 +  0)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs1 + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 +  0)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs2 + 24)));
	}
}

/* runs the 256-bit test, returns the number of rounds */
unsigned int run256_generic(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read256_dual(addr + 0000,   0, 512 +   0);
			read256_dual(addr + 0000, 256, 512 + 256);
			read256_dual(addr + 0000, 128, 512 + 128);
			read256_dual(addr + 0000, 384, 512 + 384);
			read256_dual(addr + 0000, 320, 512 + 320);
			read256_dual(addr + 0000,  64, 512 +  64);
			read256_dual(addr + 0000, 192, 512 + 192);
			read256_dual(addr + 0000, 448, 512 + 448);

			read256_dual(addr + 1024,   0, 512 +   0);
			read256_dual(addr + 1024, 256, 512 + 256);
			read256_dual(addr + 1024, 128, 512 + 128);
			read256_dual(addr + 1024, 384, 512 + 384);
			read256_dual(addr + 1024, 320, 512 + 320);
			read256_dual(addr + 1024,  64, 512 +  64);
			read256_dual(addr + 1024, 192, 512 + 192);
			read256_dual(addr + 1024, 448, 512 + 448);

			read256_dual(addr + 2048,   0, 512 +   0);
			read256_dual(addr + 2048, 256, 512 + 256);
			read256_dual(addr + 2048, 128, 512 + 128);
			read256_dual(addr + 2048, 384, 512 + 384);
			read256_dual(addr + 2048, 320, 512 + 320);
			read256_dual(addr + 2048,  64, 512 +  64);
			read256_dual(addr + 2048, 192, 512 + 192);
			read256_dual(addr + 2048, 448, 512 + 448);

			read256_dual(addr + 3072,   0, 512 +   0);
			read256_dual(addr + 3072, 256, 512 + 256);
			read256_dual(addr + 3072, 128, 512 + 128);
			read256_dual(addr + 3072, 384, 512 + 384);
			read256_dual(addr + 3072, 320, 512 + 320);
			read256_dual(addr + 3072,  64, 512 +  64);
			read256_dual(addr + 3072, 192, 512 + 192);
			read256_dual(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}

#ifdef __SSE4_1__
static inline void read256_sse(const char *addr, const unsigned long ofs)
{
	__m128i xmm0, xmm1;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1) :
	             "0" (_mm_load_si128((void *)(addr + ofs +  0))),
	             "1" (_mm_load_si128((void *)(addr + ofs + 16))));
}

static inline void read256_dual_sse(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	__m128i xmm0, xmm1, xmm2, xmm3;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1), "=xm" (xmm2), "=xm" (xmm3) :
	             "0" (_mm_load_si128((void *)(addr + ofs1 +  0))),
	             "1" (_mm_load_si128((void *)(addr + ofs1 + 16))),
	             "2" (_mm_load_si128((void *)(addr + ofs2 +  0))),
	             "3" (_mm_load_si128((void *)(addr + ofs2 + 16))));
}

/* runs the 256-bit test, returns the number of rounds */
unsigned int run256_sse(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read256_dual_sse(addr + 0000,   0, 512 +   0);
			read256_dual_sse(addr + 0000, 256, 512 + 256);
			read256_dual_sse(addr + 0000, 128, 512 + 128);
			read256_dual_sse(addr + 0000, 384, 512 + 384);
			read256_dual_sse(addr + 0000, 320, 512 + 320);
			read256_dual_sse(addr + 0000,  64, 512 +  64);
			read256_dual_sse(addr + 0000, 192, 512 + 192);
			read256_dual_sse(addr + 0000, 448, 512 + 448);
			read256_dual_sse(addr + 1024,   0, 512 +   0);
			read256_dual_sse(addr + 1024, 256, 512 + 256);
			read256_dual_sse(addr + 1024, 128, 512 + 128);
			read256_dual_sse(addr + 1024, 384, 512 + 384);
			read256_dual_sse(addr + 1024, 320, 512 + 320);
			read256_dual_sse(addr + 1024,  64, 512 +  64);
			read256_dual_sse(addr + 1024, 192, 512 + 192);
			read256_dual_sse(addr + 1024, 448, 512 + 448);
			read256_dual_sse(addr + 2048,   0, 512 +   0);
			read256_dual_sse(addr + 2048, 256, 512 + 256);
			read256_dual_sse(addr + 2048, 128, 512 + 128);
			read256_dual_sse(addr + 2048, 384, 512 + 384);
			read256_dual_sse(addr + 2048, 320, 512 + 320);
			read256_dual_sse(addr + 2048,  64, 512 +  64);
			read256_dual_sse(addr + 2048, 192, 512 + 192);
			read256_dual_sse(addr + 2048, 448, 512 + 448);
			read256_dual_sse(addr + 3072,   0, 512 +   0);
			read256_dual_sse(addr + 3072, 256, 512 + 256);
			read256_dual_sse(addr + 3072, 128, 512 + 128);
			read256_dual_sse(addr + 3072, 384, 512 + 384);
			read256_dual_sse(addr + 3072, 320, 512 + 320);
			read256_dual_sse(addr + 3072,  64, 512 +  64);
			read256_dual_sse(addr + 3072, 192, 512 + 192);
			read256_dual_sse(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}
#endif

#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
static inline void read256_dual_vfp(const char *addr, const unsigned long ofs)
{
	asm volatile("vldr %%d4, [%0,%1]\n\t"
	             "vldr %%d5, [%0,%1+8]\n\t"
	             "vldr %%d6, [%0,%1+16]\n\t"
	             "vldr %%d7, [%0,%1+24]\n\t"
	             "vldr %%d4, [%0,%1+512]\n\t"
	             "vldr %%d5, [%0,%1+512+8]\n\t"
	             "vldr %%d6, [%0,%1+512+16]\n\t"
	             "vldr %%d7, [%0,%1+512+24]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs)
	             : "%d4", "%d5", "%d6", "%d7");
}

/* runs the 256-bit test, returns the number of rounds */
unsigned int run256_vfp(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read256_dual_vfp(addr,   0);
			read256_dual_vfp(addr, 256);
			read256_dual_vfp(addr, 128);
			read256_dual_vfp(addr, 384);
			read256_dual_vfp(addr, 320);
			read256_dual_vfp(addr,  64);
			read256_dual_vfp(addr, 192);
			read256_dual_vfp(addr, 448);

			addr += 1024;

			read256_dual_vfp(addr,   0);
			read256_dual_vfp(addr, 256);
			read256_dual_vfp(addr, 128);
			read256_dual_vfp(addr, 384);
			read256_dual_vfp(addr, 320);
			read256_dual_vfp(addr,  64);
			read256_dual_vfp(addr, 192);
			read256_dual_vfp(addr, 448);

			addr += 1024;

			read256_dual_vfp(addr,   0);
			read256_dual_vfp(addr, 256);
			read256_dual_vfp(addr, 128);
			read256_dual_vfp(addr, 384);
			read256_dual_vfp(addr, 320);
			read256_dual_vfp(addr,  64);
			read256_dual_vfp(addr, 192);
			read256_dual_vfp(addr, 448);

			addr += 1024;

			read256_dual_vfp(addr,   0);
			read256_dual_vfp(addr, 256);
			read256_dual_vfp(addr, 128);
			read256_dual_vfp(addr, 384);
			read256_dual_vfp(addr, 320);
			read256_dual_vfp(addr,  64);
			read256_dual_vfp(addr, 192);
			read256_dual_vfp(addr, 448);
		}
	}
	return rounds;
}
#endif

#if defined(__ARM_ARCH_7A__)
static inline void read256_armv7(const char *addr, const unsigned long ofs)
{
	asm volatile("ldmia %0, { r4-r11 }" :: "r" (addr + ofs +  0) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
}

static inline void read256_dual_armv7(const char *addr, const unsigned long ofs1, const unsigned long ofs2)
{
	asm volatile("ldmia %0, { r4-r11 }" :: "r" (addr + ofs1 +  0) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	asm volatile("ldmia %0, { r4-r11 }" :: "r" (addr + ofs2 +  0) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
}

/* runs the 256-bit test, returns the number of rounds */
unsigned int run256_armv7(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read256_dual_armv7(addr + 0000,   0, 512 +   0);
			read256_dual_armv7(addr + 0000, 256, 512 + 256);
			read256_dual_armv7(addr + 0000, 128, 512 + 128);
			read256_dual_armv7(addr + 0000, 384, 512 + 384);
			read256_dual_armv7(addr + 0000, 320, 512 + 320);
			read256_dual_armv7(addr + 0000,  64, 512 +  64);
			read256_dual_armv7(addr + 0000, 192, 512 + 192);
			read256_dual_armv7(addr + 0000, 448, 512 + 448);
			read256_dual_armv7(addr + 1024,   0, 512 +   0);
			read256_dual_armv7(addr + 1024, 256, 512 + 256);
			read256_dual_armv7(addr + 1024, 128, 512 + 128);
			read256_dual_armv7(addr + 1024, 384, 512 + 384);
			read256_dual_armv7(addr + 1024, 320, 512 + 320);
			read256_dual_armv7(addr + 1024,  64, 512 +  64);
			read256_dual_armv7(addr + 1024, 192, 512 + 192);
			read256_dual_armv7(addr + 1024, 448, 512 + 448);
			read256_dual_armv7(addr + 2048,   0, 512 +   0);
			read256_dual_armv7(addr + 2048, 256, 512 + 256);
			read256_dual_armv7(addr + 2048, 128, 512 + 128);
			read256_dual_armv7(addr + 2048, 384, 512 + 384);
			read256_dual_armv7(addr + 2048, 320, 512 + 320);
			read256_dual_armv7(addr + 2048,  64, 512 +  64);
			read256_dual_armv7(addr + 2048, 192, 512 + 192);
			read256_dual_armv7(addr + 2048, 448, 512 + 448);
			read256_dual_armv7(addr + 3072,   0, 512 +   0);
			read256_dual_armv7(addr + 3072, 256, 512 + 256);
			read256_dual_armv7(addr + 3072, 128, 512 + 128);
			read256_dual_armv7(addr + 3072, 384, 512 + 384);
			read256_dual_armv7(addr + 3072, 320, 512 + 320);
			read256_dual_armv7(addr + 3072,  64, 512 +  64);
			read256_dual_armv7(addr + 3072, 192, 512 + 192);
			read256_dual_armv7(addr + 3072, 448, 512 + 448);
		}
	}
	return rounds;
}
#endif


/*****************************************************************************
 *                             512-bit accesses                              *
 *****************************************************************************/

static inline void read512(const char *addr, const unsigned long ofs)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs +  0)), "r" (*(uint64_t *)(addr + ofs +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 16)), "r" (*(uint64_t *)(addr + ofs + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 32)), "r" (*(uint64_t *)(addr + ofs + 40)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 48)), "r" (*(uint64_t *)(addr + ofs + 56)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs +  0)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs +  8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 32)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 40)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 48)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + ofs + 56)));
	}
}

/* runs the 512-bit test, returns the number of rounds */
unsigned int run512_generic(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read512(addr + 0000,   0); read512(addr + 0000, 512 +   0);
			read512(addr + 0000, 256); read512(addr + 0000, 512 + 256);
			read512(addr + 0000, 128); read512(addr + 0000, 512 + 128);
			read512(addr + 0000, 384); read512(addr + 0000, 512 + 384);
			read512(addr + 0000, 320); read512(addr + 0000, 512 + 320);
			read512(addr + 0000,  64); read512(addr + 0000, 512 +  64);
			read512(addr + 0000, 192); read512(addr + 0000, 512 + 192);
			read512(addr + 0000, 448); read512(addr + 0000, 512 + 448);

			read512(addr + 1024,   0); read512(addr + 1024, 512 +   0);
			read512(addr + 1024, 256); read512(addr + 1024, 512 + 256);
			read512(addr + 1024, 128); read512(addr + 1024, 512 + 128);
			read512(addr + 1024, 384); read512(addr + 1024, 512 + 384);
			read512(addr + 1024, 320); read512(addr + 1024, 512 + 320);
			read512(addr + 1024,  64); read512(addr + 1024, 512 +  64);
			read512(addr + 1024, 192); read512(addr + 1024, 512 + 192);
			read512(addr + 1024, 448); read512(addr + 1024, 512 + 448);

			read512(addr + 2048,   0); read512(addr + 2048, 512 +   0);
			read512(addr + 2048, 256); read512(addr + 2048, 512 + 256);
			read512(addr + 2048, 128); read512(addr + 2048, 512 + 128);
			read512(addr + 2048, 384); read512(addr + 2048, 512 + 384);
			read512(addr + 2048, 320); read512(addr + 2048, 512 + 320);
			read512(addr + 2048,  64); read512(addr + 2048, 512 +  64);
			read512(addr + 2048, 192); read512(addr + 2048, 512 + 192);
			read512(addr + 2048, 448); read512(addr + 2048, 512 + 448);

			read512(addr + 3072,   0); read512(addr + 3072, 512 +   0);
			read512(addr + 3072, 256); read512(addr + 3072, 512 + 256);
			read512(addr + 3072, 128); read512(addr + 3072, 512 + 128);
			read512(addr + 3072, 384); read512(addr + 3072, 512 + 384);
			read512(addr + 3072, 320); read512(addr + 3072, 512 + 320);
			read512(addr + 3072,  64); read512(addr + 3072, 512 +  64);
			read512(addr + 3072, 192); read512(addr + 3072, 512 + 192);
			read512(addr + 3072, 448); read512(addr + 3072, 512 + 448);
		}
	}
	return rounds;
}

#ifdef __SSE4_1__
static inline void read512_sse(const char *addr, const unsigned long ofs)
{
	__m128i xmm0, xmm1, xmm2, xmm3;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1), "=xm" (xmm2), "=xm" (xmm3) :
	             "0" (_mm_load_si128((void *)(addr + ofs +  0))),
	             "1" (_mm_load_si128((void *)(addr + ofs + 16))),
	             "2" (_mm_load_si128((void *)(addr + ofs + 32))),
	             "3" (_mm_load_si128((void *)(addr + ofs + 48))));
}

/* runs the 512-bit test, returns the number of rounds */
unsigned int run512_sse(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read512_sse(addr + 0000,   0); read512_sse(addr + 0000, 512 +   0);
			read512_sse(addr + 0000, 256); read512_sse(addr + 0000, 512 + 256);
			read512_sse(addr + 0000, 128); read512_sse(addr + 0000, 512 + 128);
			read512_sse(addr + 0000, 384); read512_sse(addr + 0000, 512 + 384);
			read512_sse(addr + 0000, 320); read512_sse(addr + 0000, 512 + 320);
			read512_sse(addr + 0000,  64); read512_sse(addr + 0000, 512 +  64);
			read512_sse(addr + 0000, 192); read512_sse(addr + 0000, 512 + 192);
			read512_sse(addr + 0000, 448); read512_sse(addr + 0000, 512 + 448);
			read512_sse(addr + 1024,   0); read512_sse(addr + 1024, 512 +   0);
			read512_sse(addr + 1024, 256); read512_sse(addr + 1024, 512 + 256);
			read512_sse(addr + 1024, 128); read512_sse(addr + 1024, 512 + 128);
			read512_sse(addr + 1024, 384); read512_sse(addr + 1024, 512 + 384);
			read512_sse(addr + 1024, 320); read512_sse(addr + 1024, 512 + 320);
			read512_sse(addr + 1024,  64); read512_sse(addr + 1024, 512 +  64);
			read512_sse(addr + 1024, 192); read512_sse(addr + 1024, 512 + 192);
			read512_sse(addr + 1024, 448); read512_sse(addr + 1024, 512 + 448);
			read512_sse(addr + 2048,   0); read512_sse(addr + 2048, 512 +   0);
			read512_sse(addr + 2048, 256); read512_sse(addr + 2048, 512 + 256);
			read512_sse(addr + 2048, 128); read512_sse(addr + 2048, 512 + 128);
			read512_sse(addr + 2048, 384); read512_sse(addr + 2048, 512 + 384);
			read512_sse(addr + 2048, 320); read512_sse(addr + 2048, 512 + 320);
			read512_sse(addr + 2048,  64); read512_sse(addr + 2048, 512 +  64);
			read512_sse(addr + 2048, 192); read512_sse(addr + 2048, 512 + 192);
			read512_sse(addr + 2048, 448); read512_sse(addr + 2048, 512 + 448);
			read512_sse(addr + 3072,   0); read512_sse(addr + 3072, 512 +   0);
			read512_sse(addr + 3072, 256); read512_sse(addr + 3072, 512 + 256);
			read512_sse(addr + 3072, 128); read512_sse(addr + 3072, 512 + 128);
			read512_sse(addr + 3072, 384); read512_sse(addr + 3072, 512 + 384);
			read512_sse(addr + 3072, 320); read512_sse(addr + 3072, 512 + 320);
			read512_sse(addr + 3072,  64); read512_sse(addr + 3072, 512 +  64);
			read512_sse(addr + 3072, 192); read512_sse(addr + 3072, 512 + 192);
			read512_sse(addr + 3072, 448); read512_sse(addr + 3072, 512 + 448);
		}
	}
	return rounds;
}
#endif

#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
static inline void read512_vfp(const char *addr, const unsigned long ofs)
{
	asm volatile("vldr %%d4, [%0,%1]\n\t"
	             "vldr %%d5, [%0,%1+8]\n\t"
	             "vldr %%d6, [%0,%1+16]\n\t"
	             "vldr %%d7, [%0,%1+24]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs)
	             : "%d4", "%d5", "%d6", "%d7");
	asm volatile("vldr %%d4, [%0,%1+32]\n\t"
	             "vldr %%d5, [%0,%1+40]\n\t"
	             "vldr %%d6, [%0,%1+48]\n\t"
	             "vldr %%d7, [%0,%1+56]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs)
	             : "%d4", "%d5", "%d6", "%d7");
}

/* runs the 512-bit test, returns the number of rounds */
unsigned int run512_vfp(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read512_vfp(addr + 0000,   0); read512_vfp(addr + 0000, 512 +   0);
			read512_vfp(addr + 0000, 256); read512_vfp(addr + 0000, 512 + 256);
			read512_vfp(addr + 0000, 128); read512_vfp(addr + 0000, 512 + 128);
			read512_vfp(addr + 0000, 384); read512_vfp(addr + 0000, 512 + 384);
			read512_vfp(addr + 0000, 320); read512_vfp(addr + 0000, 512 + 320);
			read512_vfp(addr + 0000,  64); read512_vfp(addr + 0000, 512 +  64);
			read512_vfp(addr + 0000, 192); read512_vfp(addr + 0000, 512 + 192);
			read512_vfp(addr + 0000, 448); read512_vfp(addr + 0000, 512 + 448);
			read512_vfp(addr + 1024,   0); read512_vfp(addr + 1024, 512 +   0);
			read512_vfp(addr + 1024, 256); read512_vfp(addr + 1024, 512 + 256);
			read512_vfp(addr + 1024, 128); read512_vfp(addr + 1024, 512 + 128);
			read512_vfp(addr + 1024, 384); read512_vfp(addr + 1024, 512 + 384);
			read512_vfp(addr + 1024, 320); read512_vfp(addr + 1024, 512 + 320);
			read512_vfp(addr + 1024,  64); read512_vfp(addr + 1024, 512 +  64);
			read512_vfp(addr + 1024, 192); read512_vfp(addr + 1024, 512 + 192);
			read512_vfp(addr + 1024, 448); read512_vfp(addr + 1024, 512 + 448);
			read512_vfp(addr + 2048,   0); read512_vfp(addr + 2048, 512 +   0);
			read512_vfp(addr + 2048, 256); read512_vfp(addr + 2048, 512 + 256);
			read512_vfp(addr + 2048, 128); read512_vfp(addr + 2048, 512 + 128);
			read512_vfp(addr + 2048, 384); read512_vfp(addr + 2048, 512 + 384);
			read512_vfp(addr + 2048, 320); read512_vfp(addr + 2048, 512 + 320);
			read512_vfp(addr + 2048,  64); read512_vfp(addr + 2048, 512 +  64);
			read512_vfp(addr + 2048, 192); read512_vfp(addr + 2048, 512 + 192);
			read512_vfp(addr + 2048, 448); read512_vfp(addr + 2048, 512 + 448);
			read512_vfp(addr + 3072,   0); read512_vfp(addr + 3072, 512 +   0);
			read512_vfp(addr + 3072, 256); read512_vfp(addr + 3072, 512 + 256);
			read512_vfp(addr + 3072, 128); read512_vfp(addr + 3072, 512 + 128);
			read512_vfp(addr + 3072, 384); read512_vfp(addr + 3072, 512 + 384);
			read512_vfp(addr + 3072, 320); read512_vfp(addr + 3072, 512 + 320);
			read512_vfp(addr + 3072,  64); read512_vfp(addr + 3072, 512 +  64);
			read512_vfp(addr + 3072, 192); read512_vfp(addr + 3072, 512 + 192);
			read512_vfp(addr + 3072, 448); read512_vfp(addr + 3072, 512 + 448);
		}
	}
	return rounds;
}
#endif

#if defined(__ARM_ARCH_7A__)
static inline void read512_armv7(const char *addr, const unsigned long ofs)
{
	asm volatile("ldmia %0, { r4-r11 }" :: "r" (addr + ofs +  0) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	asm volatile("ldmia %0, { r4-r11 }" :: "r" (addr + ofs + 32) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
}

/* runs the 512-bit test, returns the number of rounds */
unsigned int run512_armv7(void *area, size_t mask)
{
	unsigned int rounds;
	const char *addr;
	size_t rnd;

	for (rounds = 0; !stop_now; rounds++) {
		for (rnd = (size_t)((LOOPS_PER_ROUND / 64 + 1) * 257 * 4096UL); rnd -= 257 * 4096;) {
			/* Walk following a pseudo-random pattern and limit redundancy.
			 * A 4096-byte address space is crossed following pseudo-random
			 * moves within 64 byte locations and for each we test both the
			 * position and a next one 512 bytes apart. This guarantees to
			 * perform non-contiguous accesses that prevent any streaming
			 * operation from being performed.
			 */
			addr = area + (rnd & mask);

			read512_armv7(addr + 0000,   0); read512_armv7(addr + 0000, 512 +   0);
			read512_armv7(addr + 0000, 256); read512_armv7(addr + 0000, 512 + 256);
			read512_armv7(addr + 0000, 128); read512_armv7(addr + 0000, 512 + 128);
			read512_armv7(addr + 0000, 384); read512_armv7(addr + 0000, 512 + 384);
			read512_armv7(addr + 0000, 320); read512_armv7(addr + 0000, 512 + 320);
			read512_armv7(addr + 0000,  64); read512_armv7(addr + 0000, 512 +  64);
			read512_armv7(addr + 0000, 192); read512_armv7(addr + 0000, 512 + 192);
			read512_armv7(addr + 0000, 448); read512_armv7(addr + 0000, 512 + 448);
			read512_armv7(addr + 1024,   0); read512_armv7(addr + 1024, 512 +   0);
			read512_armv7(addr + 1024, 256); read512_armv7(addr + 1024, 512 + 256);
			read512_armv7(addr + 1024, 128); read512_armv7(addr + 1024, 512 + 128);
			read512_armv7(addr + 1024, 384); read512_armv7(addr + 1024, 512 + 384);
			read512_armv7(addr + 1024, 320); read512_armv7(addr + 1024, 512 + 320);
			read512_armv7(addr + 1024,  64); read512_armv7(addr + 1024, 512 +  64);
			read512_armv7(addr + 1024, 192); read512_armv7(addr + 1024, 512 + 192);
			read512_armv7(addr + 1024, 448); read512_armv7(addr + 1024, 512 + 448);
			read512_armv7(addr + 2048,   0); read512_armv7(addr + 2048, 512 +   0);
			read512_armv7(addr + 2048, 256); read512_armv7(addr + 2048, 512 + 256);
			read512_armv7(addr + 2048, 128); read512_armv7(addr + 2048, 512 + 128);
			read512_armv7(addr + 2048, 384); read512_armv7(addr + 2048, 512 + 384);
			read512_armv7(addr + 2048, 320); read512_armv7(addr + 2048, 512 + 320);
			read512_armv7(addr + 2048,  64); read512_armv7(addr + 2048, 512 +  64);
			read512_armv7(addr + 2048, 192); read512_armv7(addr + 2048, 512 + 192);
			read512_armv7(addr + 2048, 448); read512_armv7(addr + 2048, 512 + 448);
			read512_armv7(addr + 3072,   0); read512_armv7(addr + 3072, 512 +   0);
			read512_armv7(addr + 3072, 256); read512_armv7(addr + 3072, 512 + 256);
			read512_armv7(addr + 3072, 128); read512_armv7(addr + 3072, 512 + 128);
			read512_armv7(addr + 3072, 384); read512_armv7(addr + 3072, 512 + 384);
			read512_armv7(addr + 3072, 320); read512_armv7(addr + 3072, 512 + 320);
			read512_armv7(addr + 3072,  64); read512_armv7(addr + 3072, 512 +  64);
			read512_armv7(addr + 3072, 192); read512_armv7(addr + 3072, 512 + 192);
			read512_armv7(addr + 3072, 448); read512_armv7(addr + 3072, 512 + 448);
		}
	}
	return rounds;
}
#endif


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

/* returns a mask to cover the nearest lower power of two for <size> */
static size_t mask_rounded_down(size_t size)
{
	size_t mask = size;
	unsigned int shift = 1;

	while (shift < 8 * sizeof(mask) && (size = mask >> shift)) {
		mask |= size;
		shift <<= 1;
	}
	return mask >> 1;
}

/* Randomly accesses aligned words of size <word> bytes over <size> bytes of
 * area <area> for about <usec> microseconds, then returns the number of words
 * read per microsecond. Note: size is rounded down to the lower power of two,
 * and must be at least 4kB. It is mandatory that <word> is a power of two.
 */
unsigned int random_read_over_area(void *area, unsigned int usec, size_t size, size_t word)
{
	size_t mask;
	unsigned int rounds;
	uint64_t before, after;
	int fct;

	mask = mask_rounded_down(size);
	mask &= -(size_t)4096;

	memset(area, 0, size);
	rounds = 0;

	/* find what function to use based on the word size */
	for (fct = 0; word >>= 1; fct++);

	if (fct >= sizeof(run) / sizeof(*run))
		return 0;

	if (!run[fct])
		return 0;

	set_alarm(usec);
	before = rdtsc();

	rounds = run[fct](area, mask);

	after = rdtsc();
	set_alarm(0);

	/* speed = transactions per microsecond. Use 64-bit computations to avoid
	 * overflows. The caller can turn this into bytes per second by multiplying
	 * by <word>.
	 */
	usec = after - before;
	if (usec < 1)
		usec = 1;
	rounds *= LOOPS_PER_ROUND;
	return rounds / usec;
}

#define USE_GENERIC 0
#define USE_SSE     1
#define USE_VFP     2
#define USE_ARMV7   4

int main(int argc, char **argv)
{
	unsigned int usec;
	size_t size, size_max;
	void *area;
	unsigned int ret, word;
	int ptr_only = 0;
	int quiet = 0;
	int slowstart = 0;
	int bw = 0;
	int implementation;

	/* set default implementation bits */
	implementation = USE_GENERIC;
#ifdef __SSE4_1__
	implementation |= USE_SSE;
#endif
#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
	implementation |= USE_VFP;
#endif
#if defined(__ARM_ARCH_7A__)
	implementation |= USE_ARMV7;
#endif

	usec = 100000;
	size_max = 16 * 1048576;

	while (argc > 1 && *argv[1] == '-') {
		if (strcmp(argv[1], "-p") == 0) {
			ptr_only = 1;
		}
		else if (strcmp(argv[1], "-q") == 0) {
			quiet = 1;
		}
		else if (strcmp(argv[1], "-s") == 0) {
			slowstart = 1;
		}
		else if (strcmp(argv[1], "-b") == 0) {
			bw = 1;
		}
		else if (strcmp(argv[1], "-G") == 0) {
			implementation = USE_GENERIC;
		}
#ifdef __SSE4_1__
		else if (strcmp(argv[1], "-S") == 0) {
			implementation = USE_SSE;
		}
#endif
#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
		else if (strcmp(argv[1], "-V") == 0) {
			implementation = USE_VFP;
		}
#endif
#if defined(__ARM_ARCH_7A__)
		else if (strcmp(argv[1], "-7") == 0) {
			implementation = USE_ARMV7;
		}
#endif
		else {
			fprintf(stderr,
				"Usage: prog [options]* <time> <area>\n"
				"  -b : report equivalent bandwidth in MB/s\n"
				"  -p : only report performance on a word the size of a pointer\n"
				"  -s : slowstart : pre-heat for 500ms to let cpufreq adapt\n"
				"  -q : quiet : don't show column headers\n"
				"  -h : show this help\n"
				"  -G : use generic code only\n"
#ifdef __SSE4_1__
				"  -S : use SSE\n"
#endif
#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
				"  -V : use VFP\n"
#endif
#if defined(__ARM_ARCH_7A__)
				"  -7 : use ARMv7\n"
#endif
				"");
			exit(!!strcmp(argv[1], "-h"));
		}
		argc--;
		argv++;
	}

	if (argc > 1)
		usec = atoi(argv[1]) * 1000;

	if (argc > 2)
		size_max = atol(argv[2]) * 1024;

	run[0] = run8_generic;
	run[1] = run16_generic;
	run[2] = run32_generic;
	run[3] = run64_generic;
	run[4] = run128_generic;
	run[5] = run256_generic;
	run[6] = run512_generic;

#ifdef __SSE4_1__
	if (implementation & USE_SSE) {
		run[3] = run64_sse;
		run[4] = run128_sse;
		run[5] = run256_sse;
		run[6] = run512_sse;
	}
#endif
#if defined(__ARM_ARCH_7A__)
	if (implementation & USE_ARMV7) {
		run[3] = run64_armv7;
		run[5] = run256_armv7;
		run[6] = run512_armv7;
	}
#endif
#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
	if (implementation & USE_VFP) {
		run[3] = run64_vfp;
		run[4] = run128_vfp;
		run[5] = run256_vfp;
		run[6] = run512_vfp;
	}
#endif

	area = malloc(size_max + 4096);
	if ((off_t)area & 4095)
		area = (void *)(((off_t)area | 4095) + 1);

	if (slowstart) {
		set_alarm(500000);
		memset(area, 0, size_max);
		while (!stop_now);
		set_alarm(0);
	}

	if (ptr_only) {
		if (!quiet)
			printf("   size:  void*(%d bits)\n", (int)sizeof(void*) * 8);

		for (size = 1024; size <= size_max; size *= 2) {
			printf(quiet ? "%6u " : "%6uk: ", (unsigned int)(size >> 10U));
			ret = random_read_over_area(area, usec, size, sizeof(void *));
			printf("%6u\n", bw ? ret * (int)sizeof(void *) : ret);
		}
	}
	else {
		if (!quiet)
			printf("   size:     4B%c    8B%c   16B    32B    64B\n",
			       (sizeof(void *) == 4) ? '*' : ' ',
			       (sizeof(void *) == 8) ? '*' : ' ');

		for (size = 4096; size <= size_max; size *= 2) {
			printf(quiet ? "%6u " : "%6uk: ", (unsigned int)(size >> 10U));
			for (word = 4; word <= 64; word *= 2) {
				ret = random_read_over_area(area, usec, size, word);
				printf("%6u ", bw ? ret * word : ret);
			}
			printf("\n");
		}
	}
	exit(0);
}
