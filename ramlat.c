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

static inline uint64_t rdtsc()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

/* reads 8 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read8(const char *addr)
{
	asm volatile("" : : "r" (*(uint8_t *)addr));
}

/* same with two addresses at once */
static inline void read8_dual(const char *addr1, const char *addr2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint8_t *)addr1), "r" (*(uint8_t *)addr2));
	}
	else {
		asm volatile("" : : "r" (*(uint8_t *)addr1));
		asm volatile("" : : "r" (*(uint8_t *)addr2));
	}
}

/* reads 16 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read16(const char *addr)
{
	asm volatile("" : : "r" (*(uint16_t *)addr));
}

/* same with two addresses at once */
static inline void read16_dual(const char *addr1, const char *addr2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint16_t *)addr1), "r" (*(uint16_t *)addr2));
	}
	else {
		asm volatile("" : : "r" (*(uint16_t *)addr1));
		asm volatile("" : : "r" (*(uint16_t *)addr2));
	}
}

/* reads 32 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read32(const char *addr)
{
	asm volatile("" : : "r" (*(uint32_t *)addr));
}

/* same with two addresses at once */
static inline void read32_dual(const char *addr1, const char *addr2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint32_t *)addr1), "r" (*(uint32_t *)addr2));
	}
	else {
		asm volatile("" : : "r" (*(uint32_t *)addr1));
		asm volatile("" : : "r" (*(uint32_t *)addr2));
	}
}

/* reads 64 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read64(const char *addr)
{
	asm volatile("" : : "r" (*(uint64_t *)addr));
}

/* same with two addresses at once */
static inline void read64_dual(const char *addr1, const char *addr2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)addr1), "r" (*(uint64_t *)addr2));
		//asm volatile("" : : "r" (*(uint64_t *)addr1));
		//asm volatile("" : : "r" (*(uint64_t *)addr2));

		// +5.2% on cortex a9, -37% on MIPS, -28% on ARMv5, -2.3% on A53, -36% on Armada370
		//asm volatile("" : : "r" (*(uint32_t *)addr1), "r" (*(uint32_t *)(addr1 + 4)));
		//asm volatile("" : : "r" (*(uint32_t *)addr2), "r" (*(uint32_t *)(addr2 + 4)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)addr1));
		asm volatile("" : : "r" (*(uint64_t *)addr2));
	}
}

/* reads 128 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read128(const char *addr)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)addr), "r" (*(uint64_t *)(addr + 8)));
		// +1.5% on cortex a9, -22% on MIPS, -33% on ARMv5, -21% on A53, -36% on Armada370, -50% on x86_64
		//asm volatile("" : : "r" (*(uint32_t *)addr), "r" (*(uint32_t *)(addr + 4)));
		//asm volatile("" : : "r" (*(uint32_t *)(addr + 8)), "r" (*(uint32_t *)(addr + 12)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)addr));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 8)));
	}
}

/* reads 256 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read256(const char *addr)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)addr),        "r" (*(uint64_t *)(addr + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 16)), "r" (*(uint64_t *)(addr + 24)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)addr));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 24)));
	}
}

/* reads 512 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read512(const char *addr)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)addr),        "r" (*(uint64_t *)(addr + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 16)), "r" (*(uint64_t *)(addr + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 32)), "r" (*(uint64_t *)(addr + 40)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 48)), "r" (*(uint64_t *)(addr + 56)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)addr));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 32)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 40)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 48)));
		asm volatile("" : : "r" (*(uint64_t *)(addr + 56)));
	}
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
	unsigned int rounds, loop;
	uint64_t before, after;
	size_t rnd = 0;
	const char *addr;

	mask = mask_rounded_down(size);
	mask &= -(size_t)4096;

	memset(area, 0, size);//mask + 1);
	rounds = 0;

	set_alarm(usec);
	before = rdtsc();

	switch (word) {
	case 1:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 64) {
				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 4096-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and a next one 512 bytes apart. This guarantees to
				 * perform non-contiguous accesses that prevent any streaming
				 * operation from being performed.
				 */
				addr = area + (rnd & mask);
				rnd += 257 * 4096; // 257 is prime, will cover all addresses

				read8_dual(addr + 0000 + 0,   addr + 0000 + 512 + 0);
				read8_dual(addr + 0000 + 256, addr + 0000 + 512 + 256);
				read8_dual(addr + 0000 + 128, addr + 0000 + 512 + 128);
				read8_dual(addr + 0000 + 384, addr + 0000 + 512 + 384);
				read8_dual(addr + 0000 + 320, addr + 0000 + 512 + 320);
				read8_dual(addr + 0000 + 64,  addr + 0000 + 512 + 64);
				read8_dual(addr + 0000 + 192, addr + 0000 + 512 + 192);
				read8_dual(addr + 0000 + 448, addr + 0000 + 512 + 448);

				read8_dual(addr + 1024 + 0,   addr + 1024 + 512 + 0);
				read8_dual(addr + 1024 + 256, addr + 1024 + 512 + 256);
				read8_dual(addr + 1024 + 128, addr + 1024 + 512 + 128);
				read8_dual(addr + 1024 + 384, addr + 1024 + 512 + 384);
				read8_dual(addr + 1024 + 320, addr + 1024 + 512 + 320);
				read8_dual(addr + 1024 + 64,  addr + 1024 + 512 + 64);
				read8_dual(addr + 1024 + 192, addr + 1024 + 512 + 192);
				read8_dual(addr + 1024 + 448, addr + 1024 + 512 + 448);

				read8_dual(addr + 2048 + 0,   addr + 2048 + 512 + 0);
				read8_dual(addr + 2048 + 256, addr + 2048 + 512 + 256);
				read8_dual(addr + 2048 + 128, addr + 2048 + 512 + 128);
				read8_dual(addr + 2048 + 384, addr + 2048 + 512 + 384);
				read8_dual(addr + 2048 + 320, addr + 2048 + 512 + 320);
				read8_dual(addr + 2048 + 64,  addr + 2048 + 512 + 64);
				read8_dual(addr + 2048 + 192, addr + 2048 + 512 + 192);
				read8_dual(addr + 2048 + 448, addr + 2048 + 512 + 448);

				read8_dual(addr + 3072 + 0,   addr + 3072 + 512 + 0);
				read8_dual(addr + 3072 + 256, addr + 3072 + 512 + 256);
				read8_dual(addr + 3072 + 128, addr + 3072 + 512 + 128);
				read8_dual(addr + 3072 + 384, addr + 3072 + 512 + 384);
				read8_dual(addr + 3072 + 320, addr + 3072 + 512 + 320);
				read8_dual(addr + 3072 + 64,  addr + 3072 + 512 + 64);
				read8_dual(addr + 3072 + 192, addr + 3072 + 512 + 192);
				read8_dual(addr + 3072 + 448, addr + 3072 + 512 + 448);
			}
		}
		break;
	case 2:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 64) {
				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 4096-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and a next one 512 bytes apart. This guarantees to
				 * perform non-contiguous accesses that prevent any streaming
				 * operation from being performed.
				 */
				addr = area + (rnd & mask);
				rnd += 257 * 4096; // 257 is prime, will cover all addresses

				read16_dual(addr + 0000 + 0,   addr + 0000 + 512 + 0);
				read16_dual(addr + 0000 + 256, addr + 0000 + 512 + 256);
				read16_dual(addr + 0000 + 128, addr + 0000 + 512 + 128);
				read16_dual(addr + 0000 + 384, addr + 0000 + 512 + 384);
				read16_dual(addr + 0000 + 320, addr + 0000 + 512 + 320);
				read16_dual(addr + 0000 + 64,  addr + 0000 + 512 + 64);
				read16_dual(addr + 0000 + 192, addr + 0000 + 512 + 192);
				read16_dual(addr + 0000 + 448, addr + 0000 + 512 + 448);

				read16_dual(addr + 1024 + 0,   addr + 1024 + 512 + 0);
				read16_dual(addr + 1024 + 256, addr + 1024 + 512 + 256);
				read16_dual(addr + 1024 + 128, addr + 1024 + 512 + 128);
				read16_dual(addr + 1024 + 384, addr + 1024 + 512 + 384);
				read16_dual(addr + 1024 + 320, addr + 1024 + 512 + 320);
				read16_dual(addr + 1024 + 64,  addr + 1024 + 512 + 64);
				read16_dual(addr + 1024 + 192, addr + 1024 + 512 + 192);
				read16_dual(addr + 1024 + 448, addr + 1024 + 512 + 448);

				read16_dual(addr + 2048 + 0,   addr + 2048 + 512 + 0);
				read16_dual(addr + 2048 + 256, addr + 2048 + 512 + 256);
				read16_dual(addr + 2048 + 128, addr + 2048 + 512 + 128);
				read16_dual(addr + 2048 + 384, addr + 2048 + 512 + 384);
				read16_dual(addr + 2048 + 320, addr + 2048 + 512 + 320);
				read16_dual(addr + 2048 + 64,  addr + 2048 + 512 + 64);
				read16_dual(addr + 2048 + 192, addr + 2048 + 512 + 192);
				read16_dual(addr + 2048 + 448, addr + 2048 + 512 + 448);

				read16_dual(addr + 3072 + 0,   addr + 3072 + 512 + 0);
				read16_dual(addr + 3072 + 256, addr + 3072 + 512 + 256);
				read16_dual(addr + 3072 + 128, addr + 3072 + 512 + 128);
				read16_dual(addr + 3072 + 384, addr + 3072 + 512 + 384);
				read16_dual(addr + 3072 + 320, addr + 3072 + 512 + 320);
				read16_dual(addr + 3072 + 64,  addr + 3072 + 512 + 64);
				read16_dual(addr + 3072 + 192, addr + 3072 + 512 + 192);
				read16_dual(addr + 3072 + 448, addr + 3072 + 512 + 448);
			}
		}
		break;
	case 4:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 64) {
				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 4096-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and a next one 512 bytes apart. This guarantees to
				 * perform non-contiguous accesses that prevent any streaming
				 * operation from being performed.
				 */
				addr = area + (rnd & mask);
				rnd += 257 * 4096; // 257 is prime, will cover all addresses

				read32_dual(addr + 0000 + 0,   addr + 0000 + 512 + 0);
				read32_dual(addr + 0000 + 256, addr + 0000 + 512 + 256);
				read32_dual(addr + 0000 + 128, addr + 0000 + 512 + 128);
				read32_dual(addr + 0000 + 384, addr + 0000 + 512 + 384);
				read32_dual(addr + 0000 + 320, addr + 0000 + 512 + 320);
				read32_dual(addr + 0000 + 64,  addr + 0000 + 512 + 64);
				read32_dual(addr + 0000 + 192, addr + 0000 + 512 + 192);
				read32_dual(addr + 0000 + 448, addr + 0000 + 512 + 448);

				read32_dual(addr + 1024 + 0,   addr + 1024 + 512 + 0);
				read32_dual(addr + 1024 + 256, addr + 1024 + 512 + 256);
				read32_dual(addr + 1024 + 128, addr + 1024 + 512 + 128);
				read32_dual(addr + 1024 + 384, addr + 1024 + 512 + 384);
				read32_dual(addr + 1024 + 320, addr + 1024 + 512 + 320);
				read32_dual(addr + 1024 + 64,  addr + 1024 + 512 + 64);
				read32_dual(addr + 1024 + 192, addr + 1024 + 512 + 192);
				read32_dual(addr + 1024 + 448, addr + 1024 + 512 + 448);

				read32_dual(addr + 2048 + 0,   addr + 2048 + 512 + 0);
				read32_dual(addr + 2048 + 256, addr + 2048 + 512 + 256);
				read32_dual(addr + 2048 + 128, addr + 2048 + 512 + 128);
				read32_dual(addr + 2048 + 384, addr + 2048 + 512 + 384);
				read32_dual(addr + 2048 + 320, addr + 2048 + 512 + 320);
				read32_dual(addr + 2048 + 64,  addr + 2048 + 512 + 64);
				read32_dual(addr + 2048 + 192, addr + 2048 + 512 + 192);
				read32_dual(addr + 2048 + 448, addr + 2048 + 512 + 448);

				read32_dual(addr + 3072 + 0,   addr + 3072 + 512 + 0);
				read32_dual(addr + 3072 + 256, addr + 3072 + 512 + 256);
				read32_dual(addr + 3072 + 128, addr + 3072 + 512 + 128);
				read32_dual(addr + 3072 + 384, addr + 3072 + 512 + 384);
				read32_dual(addr + 3072 + 320, addr + 3072 + 512 + 320);
				read32_dual(addr + 3072 + 64,  addr + 3072 + 512 + 64);
				read32_dual(addr + 3072 + 192, addr + 3072 + 512 + 192);
				read32_dual(addr + 3072 + 448, addr + 3072 + 512 + 448);
			}
		}
		break;
	case 8: {
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 64) {
				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 4096-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and a next one 512 bytes apart. This guarantees to
				 * perform non-contiguous accesses that prevent any streaming
				 * operation from being performed.
				 */
				addr = area + (rnd & mask);
				rnd += 257 * 4096; // 257 is prime, will cover all addresses

				read64_dual(addr + 0000 + 0,   addr + 0000 + 512 + 0);
				read64_dual(addr + 0000 + 256, addr + 0000 + 512 + 256);
				read64_dual(addr + 0000 + 128, addr + 0000 + 512 + 128);
				read64_dual(addr + 0000 + 384, addr + 0000 + 512 + 384);
				read64_dual(addr + 0000 + 320, addr + 0000 + 512 + 320);
				read64_dual(addr + 0000 + 64,  addr + 0000 + 512 + 64);
				read64_dual(addr + 0000 + 192, addr + 0000 + 512 + 192);
				read64_dual(addr + 0000 + 448, addr + 0000 + 512 + 448);

				read64_dual(addr + 1024 + 0,   addr + 1024 + 512 + 0);
				read64_dual(addr + 1024 + 256, addr + 1024 + 512 + 256);
				read64_dual(addr + 1024 + 128, addr + 1024 + 512 + 128);
				read64_dual(addr + 1024 + 384, addr + 1024 + 512 + 384);
				read64_dual(addr + 1024 + 320, addr + 1024 + 512 + 320);
				read64_dual(addr + 1024 + 64,  addr + 1024 + 512 + 64);
				read64_dual(addr + 1024 + 192, addr + 1024 + 512 + 192);
				read64_dual(addr + 1024 + 448, addr + 1024 + 512 + 448);

				read64_dual(addr + 2048 + 0,   addr + 2048 + 512 + 0);
				read64_dual(addr + 2048 + 256, addr + 2048 + 512 + 256);
				read64_dual(addr + 2048 + 128, addr + 2048 + 512 + 128);
				read64_dual(addr + 2048 + 384, addr + 2048 + 512 + 384);
				read64_dual(addr + 2048 + 320, addr + 2048 + 512 + 320);
				read64_dual(addr + 2048 + 64,  addr + 2048 + 512 + 64);
				read64_dual(addr + 2048 + 192, addr + 2048 + 512 + 192);
				read64_dual(addr + 2048 + 448, addr + 2048 + 512 + 448);

				read64_dual(addr + 3072 + 0,   addr + 3072 + 512 + 0);
				read64_dual(addr + 3072 + 256, addr + 3072 + 512 + 256);
				read64_dual(addr + 3072 + 128, addr + 3072 + 512 + 128);
				read64_dual(addr + 3072 + 384, addr + 3072 + 512 + 384);
				read64_dual(addr + 3072 + 320, addr + 3072 + 512 + 320);
				read64_dual(addr + 3072 + 64,  addr + 3072 + 512 + 64);
				read64_dual(addr + 3072 + 192, addr + 3072 + 512 + 192);
				read64_dual(addr + 3072 + 448, addr + 3072 + 512 + 448);
			}
		}
		break;
	}
	case 16:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 64) {
				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 4096-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and a next one 512 bytes apart. This guarantees to
				 * perform non-contiguous accesses that prevent any streaming
				 * operation from being performed.
				 */
				addr = area + (rnd & mask);
				rnd += 257 * 4096; // 257 is prime, will cover all addresses

				read128(addr + 0000 + 0);   read128(addr + 0000 + 512 + 0);
				read128(addr + 0000 + 256); read128(addr + 0000 + 512 + 256);
				read128(addr + 0000 + 128); read128(addr + 0000 + 512 + 128);
				read128(addr + 0000 + 384); read128(addr + 0000 + 512 + 384);
				read128(addr + 0000 + 320); read128(addr + 0000 + 512 + 320);
				read128(addr + 0000 + 64);  read128(addr + 0000 + 512 + 64);
				read128(addr + 0000 + 192); read128(addr + 0000 + 512 + 192);
				read128(addr + 0000 + 448); read128(addr + 0000 + 512 + 448);

				read128(addr + 1024 + 0);   read128(addr + 1024 + 512 + 0);
				read128(addr + 1024 + 256); read128(addr + 1024 + 512 + 256);
				read128(addr + 1024 + 128); read128(addr + 1024 + 512 + 128);
				read128(addr + 1024 + 384); read128(addr + 1024 + 512 + 384);
				read128(addr + 1024 + 320); read128(addr + 1024 + 512 + 320);
				read128(addr + 1024 + 64);  read128(addr + 1024 + 512 + 64);
				read128(addr + 1024 + 192); read128(addr + 1024 + 512 + 192);
				read128(addr + 1024 + 448); read128(addr + 1024 + 512 + 448);

				read128(addr + 2048 + 0);   read128(addr + 2048 + 512 + 0);
				read128(addr + 2048 + 256); read128(addr + 2048 + 512 + 256);
				read128(addr + 2048 + 128); read128(addr + 2048 + 512 + 128);
				read128(addr + 2048 + 384); read128(addr + 2048 + 512 + 384);
				read128(addr + 2048 + 320); read128(addr + 2048 + 512 + 320);
				read128(addr + 2048 + 64);  read128(addr + 2048 + 512 + 64);
				read128(addr + 2048 + 192); read128(addr + 2048 + 512 + 192);
				read128(addr + 2048 + 448); read128(addr + 2048 + 512 + 448);

				read128(addr + 3072 + 0);   read128(addr + 3072 + 512 + 0);
				read128(addr + 3072 + 256); read128(addr + 3072 + 512 + 256);
				read128(addr + 3072 + 128); read128(addr + 3072 + 512 + 128);
				read128(addr + 3072 + 384); read128(addr + 3072 + 512 + 384);
				read128(addr + 3072 + 320); read128(addr + 3072 + 512 + 320);
				read128(addr + 3072 + 64);  read128(addr + 3072 + 512 + 64);
				read128(addr + 3072 + 192); read128(addr + 3072 + 512 + 192);
				read128(addr + 3072 + 448); read128(addr + 3072 + 512 + 448);
			}
		}
		break;
	case 32:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 64) {
				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 4096-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and a next one 512 bytes apart. This guarantees to
				 * perform non-contiguous accesses that prevent any streaming
				 * operation from being performed.
				 */
				addr = area + (rnd & mask);
				rnd += 257 * 4096; // 257 is prime, will cover all addresses

				read256(addr + 0000 + 0);   read256(addr + 0000 + 512 + 0);
				read256(addr + 0000 + 256); read256(addr + 0000 + 512 + 256);
				read256(addr + 0000 + 128); read256(addr + 0000 + 512 + 128);
				read256(addr + 0000 + 384); read256(addr + 0000 + 512 + 384);
				read256(addr + 0000 + 320); read256(addr + 0000 + 512 + 320);
				read256(addr + 0000 + 64);  read256(addr + 0000 + 512 + 64);
				read256(addr + 0000 + 192); read256(addr + 0000 + 512 + 192);
				read256(addr + 0000 + 448); read256(addr + 0000 + 512 + 448);

				read256(addr + 1024 + 0);   read256(addr + 1024 + 512 + 0);
				read256(addr + 1024 + 256); read256(addr + 1024 + 512 + 256);
				read256(addr + 1024 + 128); read256(addr + 1024 + 512 + 128);
				read256(addr + 1024 + 384); read256(addr + 1024 + 512 + 384);
				read256(addr + 1024 + 320); read256(addr + 1024 + 512 + 320);
				read256(addr + 1024 + 64);  read256(addr + 1024 + 512 + 64);
				read256(addr + 1024 + 192); read256(addr + 1024 + 512 + 192);
				read256(addr + 1024 + 448); read256(addr + 1024 + 512 + 448);

				read256(addr + 2048 + 0);   read256(addr + 2048 + 512 + 0);
				read256(addr + 2048 + 256); read256(addr + 2048 + 512 + 256);
				read256(addr + 2048 + 128); read256(addr + 2048 + 512 + 128);
				read256(addr + 2048 + 384); read256(addr + 2048 + 512 + 384);
				read256(addr + 2048 + 320); read256(addr + 2048 + 512 + 320);
				read256(addr + 2048 + 64);  read256(addr + 2048 + 512 + 64);
				read256(addr + 2048 + 192); read256(addr + 2048 + 512 + 192);
				read256(addr + 2048 + 448); read256(addr + 2048 + 512 + 448);

				read256(addr + 3072 + 0);   read256(addr + 3072 + 512 + 0);
				read256(addr + 3072 + 256); read256(addr + 3072 + 512 + 256);
				read256(addr + 3072 + 128); read256(addr + 3072 + 512 + 128);
				read256(addr + 3072 + 384); read256(addr + 3072 + 512 + 384);
				read256(addr + 3072 + 320); read256(addr + 3072 + 512 + 320);
				read256(addr + 3072 + 64);  read256(addr + 3072 + 512 + 64);
				read256(addr + 3072 + 192); read256(addr + 3072 + 512 + 192);
				read256(addr + 3072 + 448); read256(addr + 3072 + 512 + 448);
			}
		}
		break;
	case 64:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 64) {
				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 4096-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and a next one 512 bytes apart. This guarantees to
				 * perform non-contiguous accesses that prevent any streaming
				 * operation from being performed.
				 */
				addr = area + (rnd & mask);
				rnd += 257 * 4096; // 257 is prime, will cover all addresses

				read512(addr + 0000 + 0);   read512(addr + 0000 + 512 + 0);
				read512(addr + 0000 + 256); read512(addr + 0000 + 512 + 256);
				read512(addr + 0000 + 128); read512(addr + 0000 + 512 + 128);
				read512(addr + 0000 + 384); read512(addr + 0000 + 512 + 384);
				read512(addr + 0000 + 320); read512(addr + 0000 + 512 + 320);
				read512(addr + 0000 + 64);  read512(addr + 0000 + 512 + 64);
				read512(addr + 0000 + 192); read512(addr + 0000 + 512 + 192);
				read512(addr + 0000 + 448); read512(addr + 0000 + 512 + 448);

				read512(addr + 1024 + 0);   read512(addr + 1024 + 512 + 0);
				read512(addr + 1024 + 256); read512(addr + 1024 + 512 + 256);
				read512(addr + 1024 + 128); read512(addr + 1024 + 512 + 128);
				read512(addr + 1024 + 384); read512(addr + 1024 + 512 + 384);
				read512(addr + 1024 + 320); read512(addr + 1024 + 512 + 320);
				read512(addr + 1024 + 64);  read512(addr + 1024 + 512 + 64);
				read512(addr + 1024 + 192); read512(addr + 1024 + 512 + 192);
				read512(addr + 1024 + 448); read512(addr + 1024 + 512 + 448);

				read512(addr + 2048 + 0);   read512(addr + 2048 + 512 + 0);
				read512(addr + 2048 + 256); read512(addr + 2048 + 512 + 256);
				read512(addr + 2048 + 128); read512(addr + 2048 + 512 + 128);
				read512(addr + 2048 + 384); read512(addr + 2048 + 512 + 384);
				read512(addr + 2048 + 320); read512(addr + 2048 + 512 + 320);
				read512(addr + 2048 + 64);  read512(addr + 2048 + 512 + 64);
				read512(addr + 2048 + 192); read512(addr + 2048 + 512 + 192);
				read512(addr + 2048 + 448); read512(addr + 2048 + 512 + 448);

				read512(addr + 3072 + 0);   read512(addr + 3072 + 512 + 0);
				read512(addr + 3072 + 256); read512(addr + 3072 + 512 + 256);
				read512(addr + 3072 + 128); read512(addr + 3072 + 512 + 128);
				read512(addr + 3072 + 384); read512(addr + 3072 + 512 + 384);
				read512(addr + 3072 + 320); read512(addr + 3072 + 512 + 320);
				read512(addr + 3072 + 64);  read512(addr + 3072 + 512 + 64);
				read512(addr + 3072 + 192); read512(addr + 3072 + 512 + 192);
				read512(addr + 3072 + 448); read512(addr + 3072 + 512 + 448);
			}
		}
		break;
	}

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
		else {
			fprintf(stderr, "Usage: prog [-b] [-p] [-s] [-q] [-h] <time> <area>\n");
			exit(!!strcmp(argv[1], "-h"));
		}
		argc--;
		argv++;
	}

	if (argc > 1)
		usec = atoi(argv[1]) * 1000;

	if (argc > 2)
		size_max = atol(argv[2]) * 1024;

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
			printf("   size:     4B     8B    16B    32B    64B\n");

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
