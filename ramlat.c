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
volatile static int stop_now;

static inline uint64_t rdtsc()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

/* reads 8 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read8(char *area, off_t off)
{
	asm volatile("" : : "r" (*(uint8_t *)(area + off)));
}

/* same with two addresses at once */
static inline void read8_dual(char *area, off_t off1, off_t off2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint8_t *)(area + off1)), "r" (*(uint8_t *)(area + off2)));
	}
	else {
		asm volatile("" : : "r" (*(uint8_t *)(area + off1)));
		asm volatile("" : : "r" (*(uint8_t *)(area + off2)));
	}
}

/* reads 16 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read16(char *area, off_t off)
{
	asm volatile("" : : "r" (*(uint16_t *)(area + off)));
}

/* same with two addresses at once */
static inline void read16_dual(char *area, off_t off1, off_t off2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint16_t *)(area + off1)), "r" (*(uint16_t *)(area + off2)));
	}
	else {
		asm volatile("" : : "r" (*(uint16_t *)(area + off1)));
		asm volatile("" : : "r" (*(uint16_t *)(area + off2)));
	}
}

/* reads 32 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read32(char *area, off_t off)
{
	asm volatile("" : : "r" (*(uint32_t *)(area + off)));
}

/* same with two addresses at once */
static inline void read32_dual(char *area, off_t off1, off_t off2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint32_t *)(area + off1)), "r" (*(uint32_t *)(area + off2)));
	}
	else {
		asm volatile("" : : "r" (*(uint32_t *)(area + off1)));
		asm volatile("" : : "r" (*(uint32_t *)(area + off2)));
	}
}

/* reads 64 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read64(char *area, off_t off)
{
	asm volatile("" : : "r" (*(uint64_t *)(area + off)));
}

/* same with two addresses at once */
static inline void read64_dual(char *area, off_t off1, off_t off2)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(area + off1)), "r" (*(uint64_t *)(area + off2)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(area + off1)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off2)));
	}
}

/* reads 128 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read128(char *area, off_t off)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(area + off)), "r" (*(uint64_t *)(area + off + 8)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(area + off)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 8)));
	}
}

/* reads 256 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read256(char *area, off_t off)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(area + off)),      "r" (*(uint64_t *)(area + off + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 16)), "r" (*(uint64_t *)(area + off + 24)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(area + off)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 24)));
	}
}

/* reads 512 bits from memory area <area>, offset <off> as efficiently as
 * possible for the current architecture.
 */
static inline void read512(char *area, off_t off)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : : "r" (*(uint64_t *)(area + off)),      "r" (*(uint64_t *)(area + off + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 16)), "r" (*(uint64_t *)(area + off + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 32)), "r" (*(uint64_t *)(area + off + 40)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 48)), "r" (*(uint64_t *)(area + off + 56)));
	}
	else {
		asm volatile("" : : "r" (*(uint64_t *)(area + off)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 8)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 16)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 24)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 32)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 40)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 48)));
		asm volatile("" : : "r" (*(uint64_t *)(area + off + 56)));
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
	int shift = 1;

	while (shift < 8 * sizeof(mask) && (size = mask >> shift)) {
		mask |= size;
		shift <<= 1;
	}
	return mask >> 1;
}

/* Randomly accesses aligned words of size <word> bytes over <size> bytes of
 * area <area> for about <usec> microseconds, then returns the number of words
 * read per microsecond. Note: size is rounded down the the lower power of two.
 * It is mandatory that <word> is a power of two.
 */
unsigned int random_read_over_area(void *area, unsigned int usec, size_t size, size_t word)
{
	size_t mask;
	unsigned int rounds, loop;
	uint64_t before, after;
	off_t rnd = 0;

	mask = mask_rounded_down(size);
	mask &= ~(word - 1);
	mask &= ~(256UL | 128UL | 64UL); // these ones are manipulated in the loops
	memset(area, 0, mask + 1);
	rounds = 0;

	set_alarm(usec);
	before = rdtsc();

	switch (word) {
	case 1:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
				register off_t addr1, addr2;

				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 512-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and its mirror over the address mask. This ensures
				 * non-contiguous accesses that prevent any streaming operation
				 * from being performed.
				 */
				rnd += 0x20200; // that's 257(prime) times 512

				addr1 = rnd   & mask;
				addr2 = addr1 ^ mask;
				read8_dual(area, addr1 + 0,   addr2 + 0);
				read8_dual(area, addr1 + 256, addr2 + 256);
				read8_dual(area, addr1 + 128, addr2 + 128);
				read8_dual(area, addr1 + 384, addr2 + 384);
				read8_dual(area, addr1 + 320, addr2 + 320);
				read8_dual(area, addr1 + 64,  addr2 + 64);
				read8_dual(area, addr1 + 192, addr2 + 192);
				read8_dual(area, addr1 + 448, addr2 + 448);
			}
		}
		break;
	case 2:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
				register off_t addr1, addr2;

				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 512-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and its mirror over the address mask. This ensures
				 * non-contiguous accesses that prevent any streaming operation
				 * from being performed.
				 */
				rnd += 0x20200; // that's 257(prime) times 512

				addr1 = rnd   & mask;
				addr2 = addr1 ^ mask;
				read16_dual(area, addr1 + 0,   addr2 + 0);
				read16_dual(area, addr1 + 256, addr2 + 256);
				read16_dual(area, addr1 + 128, addr2 + 128);
				read16_dual(area, addr1 + 384, addr2 + 384);
				read16_dual(area, addr1 + 320, addr2 + 320);
				read16_dual(area, addr1 + 64,  addr2 + 64);
				read16_dual(area, addr1 + 192, addr2 + 192);
				read16_dual(area, addr1 + 448, addr2 + 448);
			}
		}
		break;
	case 4:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
				register off_t addr1, addr2;

				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 512-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and its mirror over the address mask. This ensures
				 * non-contiguous accesses that prevent any streaming operation
				 * from being performed.
				 */
				rnd += 0x20200; // that's 257(prime) times 512

				addr1 = rnd   & mask;
				addr2 = addr1 ^ mask;
				read32_dual(area, addr1 + 0,   addr2 + 0);
				read32_dual(area, addr1 + 256, addr2 + 256);
				read32_dual(area, addr1 + 128, addr2 + 128);
				read32_dual(area, addr1 + 384, addr2 + 384);
				read32_dual(area, addr1 + 320, addr2 + 320);
				read32_dual(area, addr1 + 64,  addr2 + 64);
				read32_dual(area, addr1 + 192, addr2 + 192);
				read32_dual(area, addr1 + 448, addr2 + 448);
			}
		}
		break;
	case 8: {
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
				register off_t addr1, addr2;

				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 512-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and its mirror over the address mask. This ensures
				 * non-contiguous accesses that prevent any streaming operation
				 * from being performed.
				 */
				rnd += 0x20200; // that's 257(prime) times 512

				addr1 = rnd   & mask;
				addr2 = addr1 ^ mask;
				read64_dual(area, addr1 + 0,   addr2 + 0);
				read64_dual(area, addr1 + 256, addr2 + 256);
				read64_dual(area, addr1 + 128, addr2 + 128);
				read64_dual(area, addr1 + 384, addr2 + 384);
				read64_dual(area, addr1 + 320, addr2 + 320);
				read64_dual(area, addr1 + 64,  addr2 + 64);
				read64_dual(area, addr1 + 192, addr2 + 192);
				read64_dual(area, addr1 + 448, addr2 + 448);
			}
		}
		break;
	}
	case 16:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
				register off_t addr1, addr2;

				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 512-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and its mirror over the address mask. This ensures
				 * non-contiguous accesses that prevent any streaming operation
				 * from being performed.
				 */
				rnd += 0x20200; // that's 257(prime) times 512

				addr1 = rnd   & mask; // @0 mod 512
				read128(area, addr1);
				addr2 = addr1 ^ mask; // @0 mod 512
				read128(area, addr2);

				addr1 += 256;  // @256 mod 512
				read128(area, addr1);
				addr2 += 256;  // @256 mod 512
				read128(area, addr2);

				addr1 -= 128;  // @128 mod 512
				read128(area, addr1);
				addr2 -= 128;  // @128 mod 512
				read128(area, addr2);

				addr1 += 256;  // @384 mod 512
				read128(area, addr1);
				addr2 += 256;  // @384 mod 512
				read128(area, addr2);

				addr1 -= 64;   // @320 mod 512
				read128(area, addr1);
				addr2 -= 64;   // @320 mod 512
				read128(area, addr2);

				addr1 -= 256;  // @64 mod 512
				read128(area, addr1);
				addr2 -= 256;  // @64 mod 512
				read128(area, addr2);

				addr1 += 128;  // @192 mod 512
				read128(area, addr1);
				addr2 += 128;  // @192 mod 512
				read128(area, addr2);

				addr1 += 256;  // @448 mod 512
				read128(area, addr1);
				addr2 += 256;  // @448 mod 512
				read128(area, addr2);
			}
		}
		break;
	case 32:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
				register off_t addr1, addr2;

				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 512-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and its mirror over the address mask. This ensures
				 * non-contiguous accesses that prevent any streaming operation
				 * from being performed.
				 */
				rnd += 0x20200; // that's 257(prime) times 512

				addr1 = rnd   & mask; // @0 mod 512
				read256(area, addr1);
				addr2 = addr1 ^ mask; // @0 mod 512
				read256(area, addr2);

				addr1 += 256;  // @256 mod 512
				read256(area, addr1);
				addr2 += 256;  // @256 mod 512
				read256(area, addr2);

				addr1 -= 128;  // @128 mod 512
				read256(area, addr1);
				addr2 -= 128;  // @128 mod 512
				read256(area, addr2);

				addr1 += 256;  // @384 mod 512
				read256(area, addr1);
				addr2 += 256;  // @384 mod 512
				read256(area, addr2);

				addr1 -= 64;   // @320 mod 512
				read256(area, addr1);
				addr2 -= 64;   // @320 mod 512
				read256(area, addr2);

				addr1 -= 256;  // @64 mod 512
				read256(area, addr1);
				addr2 -= 256;  // @64 mod 512
				read256(area, addr2);

				addr1 += 128;  // @192 mod 512
				read256(area, addr1);
				addr2 += 128;  // @192 mod 512
				read256(area, addr2);

				addr1 += 256;  // @448 mod 512
				read256(area, addr1);
				addr2 += 256;  // @448 mod 512
				read256(area, addr2);
			}
		}
		break;
	case 64:
		for (rounds = 0; !stop_now; rounds++) {
			for (loop = 0; loop < LOOPS_PER_ROUND; loop += 16) {
				register off_t addr1, addr2;

				/* Walk following a pseudo-random pattern and limit redundancy.
				 * A 512-byte address space is crossed following pseudo-random
				 * moves within 64 byte locations and for each we test both the
				 * position and its mirror over the address mask. This ensures
				 * non-contiguous accesses that prevent any streaming operation
				 * from being performed.
				 */
				rnd += 0x20200; // that's 257(prime) times 512

				addr1 = rnd   & mask; // @0 mod 512
				read512(area, addr1);
				addr2 = addr1 ^ mask; // @0 mod 512
				read512(area, addr2);

				addr1 += 256;  // @256 mod 512
				read512(area, addr1);
				addr2 += 256;  // @256 mod 512
				read512(area, addr2);

				addr1 -= 128;  // @128 mod 512
				read512(area, addr1);
				addr2 -= 128;  // @128 mod 512
				read512(area, addr2);

				addr1 += 256;  // @384 mod 512
				read512(area, addr1);
				addr2 += 256;  // @384 mod 512
				read512(area, addr2);

				addr1 -= 64;   // @320 mod 512
				read512(area, addr1);
				addr2 -= 64;   // @320 mod 512
				read512(area, addr2);

				addr1 -= 256;  // @64 mod 512
				read512(area, addr1);
				addr2 -= 256;  // @64 mod 512
				read512(area, addr2);

				addr1 += 128;  // @192 mod 512
				read512(area, addr1);
				addr2 += 128;  // @192 mod 512
				read512(area, addr2);

				addr1 += 256;  // @448 mod 512
				read512(area, addr1);
				addr2 += 256;  // @448 mod 512
				read512(area, addr2);
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

	usec = 100000;
	size_max = 1048576;

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
		else {
			fprintf(stderr, "Usage: prog [-p] [-s] [-q] [-h] <time> <area>\n");
			exit(!!strcmp(argv[1], "-h"));
		}
		argc--;
		argv++;
	}

	if (argc > 1)
		usec = atoi(argv[1]) * 1000;

	if (argc > 2)
		size_max = atol(argv[2]) * 1024;

	area = malloc(size_max);

	if (slowstart) {
		set_alarm(500000);
		memset(area, 0, size_max);
		while (!stop_now);
		set_alarm(0);
	}

	if (ptr_only) {
		if (!quiet)
			printf("   size:  void*(%d bits)\n", sizeof(void*) * 8);

		for (size = 1024; size <= size_max; size *= 2) {
			printf(quiet ? "%6u " : "%6uk: ", (unsigned int)(size >> 10U));
			ret = random_read_over_area(area, usec, size, sizeof(void *));
			printf("%6u\n", ret);
		}
	}
	else {
		if (!quiet)
			printf("   size:     4B     8B    16B    32B    64B\n");

		for (size = 1024; size <= size_max; size *= 2) {
			printf(quiet ? "%6u " : "%6uk: ", (unsigned int)(size >> 10U));
			for (word = 4; word <= 64; word *= 2) {
				ret = random_read_over_area(area, usec, size, word);
				printf("%6u ", ret);
			}
			printf("\n");
		}
	}
	exit(0);
}
