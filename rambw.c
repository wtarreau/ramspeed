#ifdef __SSE2__
#include <x86intrin.h>
#endif

#include <sys/mman.h>
#include <sys/time.h>
#include <malloc.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_THREADS 1024

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
#define HAS_MANY_REGISTERS 0
#else
#define HAS_MANY_REGISTERS 1
#endif

/* some archs provide a shorter encoding for short negative offsets, let's make
 * use of them when possible.
 */
#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__)
#define RELATIVE_OFS -128
#else
#define RELATIVE_OFS 0
#endif

/* some archs have good branch prediction and prefer short loops.
 * This must be a power of two between 256 and 2048 inclusive.
 */
#if defined(__x86_64__)
#define BYTES_PER_ROUND   256
#else
#define BYTES_PER_ROUND  2048
#endif

#define USE_GENERIC 0
#define USE_SSE     1
#define USE_VFP     2
#define USE_ARMV7   4
#define USE_ARMV8   8
#define USE_AVX    16

struct stats {
	uint64_t rnd;    // work value
	uint64_t last;   // copy at interrupt time
	uint64_t prev;   // copy of previous last

	void *area;
	size_t size;
	size_t mask;
	pthread_t pth;   // pthread of the thread
	int thr;
} __attribute__((aligned(64)));

struct stats stats[MAX_THREADS];

/* set once the end is reached, reset when setting an alarm */
static volatile int slowstart;
static volatile int stop_now;
static volatile unsigned int meas_count;
static volatile uint64_t start_time;
static unsigned int interval_usec;
static int nbthreads = 1;
static __thread int thread_num;
static int no_hugepages;

void *(*run)(void *private);
void set_alarm(unsigned int usec);

static inline void read512(const char *addr, const unsigned long ofs)
{
	if (HAS_MANY_REGISTERS) {
		asm volatile("" : :
			     "r" (*(uint64_t *)(addr + ofs +  0)), "r" (*(uint64_t *)(addr + ofs +  8)),
			     "r" (*(uint64_t *)(addr + ofs + 16)), "r" (*(uint64_t *)(addr + ofs + 24)));
		asm volatile("" : :
			     "r" (*(uint64_t *)(addr + ofs + 32)), "r" (*(uint64_t *)(addr + ofs + 40)),
			     "r" (*(uint64_t *)(addr + ofs + 48)), "r" (*(uint64_t *)(addr + ofs + 56)));
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

/* runs the 512-bit test */
void *run512_generic(void *private)
{
	struct stats *ctx = private;
	size_t size = ctx->size;
	size_t mask = ctx->mask;
	void *area = ctx->area;
	const char *addr;
	uint64_t rnd;

	memset(area, 0, size);

	while (slowstart)
		;

	thread_num = ctx->thr;
	area -= RELATIVE_OFS;
	for (rnd = ctx->rnd; !stop_now; ) {
		__atomic_store_n(&ctx->rnd, rnd, __ATOMIC_RELEASE);
		addr = area + (rnd & mask);
		rnd += BYTES_PER_ROUND;

		__builtin_prefetch(addr + 512 + RELATIVE_OFS, 0);

		read512(addr,    0 + RELATIVE_OFS);
		read512(addr,   64 + RELATIVE_OFS);
		read512(addr,  128 + RELATIVE_OFS);
		read512(addr,  192 + RELATIVE_OFS);

#if BYTES_PER_ROUND > 256
		read512(addr, 256);
		read512(addr, 320);
		read512(addr, 384);
		read512(addr, 448);
#endif

#if BYTES_PER_ROUND > 512
		addr += 512;
		read512(addr,   0);
		read512(addr,  64);
		read512(addr, 128);
		read512(addr, 192);
		read512(addr, 256);
		read512(addr, 320);
		read512(addr, 384);
		read512(addr, 448);
#endif

#if BYTES_PER_ROUND > 1024
		addr += 512;
		read512(addr,   0);
		read512(addr,  64);
		read512(addr, 128);
		read512(addr, 192);
		read512(addr, 256);
		read512(addr, 320);
		read512(addr, 384);
		read512(addr, 448);

		addr += 512;
		read512(addr,   0);
		read512(addr,  64);
		read512(addr, 128);
		read512(addr, 192);
		read512(addr, 256);
		read512(addr, 320);
		read512(addr, 384);
		read512(addr, 448);
#endif
	}
	return NULL;
}

#ifdef __SSE2__
static inline void read512_sse(const char *addr, const unsigned long ofs)
{
	__m128i xmm0, xmm1, xmm2, xmm3;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1), "=xm" (xmm2), "=xm" (xmm3) :
	             "0" (_mm_load_si128((void *)(addr + ofs +   0))),
	             "1" (_mm_load_si128((void *)(addr + ofs +  16))),
	             "2" (_mm_load_si128((void *)(addr + ofs +  32))),
	             "3" (_mm_load_si128((void *)(addr + ofs +  48))));
}

/* runs the 512-bit test */
void *run512_sse(void *private)
{
	struct stats *ctx = private;
	size_t size = ctx->size;
	size_t mask = ctx->mask;
	void *area = ctx->area;
	const char *addr;
	uint64_t rnd;

	memset(area, 0, size);

	while (slowstart)
		;

	thread_num = ctx->thr;
	area -= RELATIVE_OFS;
	for (rnd = ctx->rnd; !stop_now; ) {
		__atomic_store_n(&ctx->rnd, rnd, __ATOMIC_RELEASE);
		addr = area + (rnd & mask);
		rnd += BYTES_PER_ROUND;

		__builtin_prefetch(addr + 1024 + RELATIVE_OFS, 0);

		read512_sse(addr,   0 + RELATIVE_OFS);
		read512_sse(addr,  64 + RELATIVE_OFS);
		read512_sse(addr, 128 + RELATIVE_OFS);
		read512_sse(addr, 192 + RELATIVE_OFS);

#if BYTES_PER_ROUND > 256
		read512_sse(addr, 256 + RELATIVE_OFS);
		read512_sse(addr, 320 + RELATIVE_OFS);
		read512_sse(addr, 384 + RELATIVE_OFS);
		read512_sse(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 512
		addr += 512;
		read512_sse(addr,   0 + RELATIVE_OFS);
		read512_sse(addr,  64 + RELATIVE_OFS);
		read512_sse(addr, 128 + RELATIVE_OFS);
		read512_sse(addr, 192 + RELATIVE_OFS);
		read512_sse(addr, 256 + RELATIVE_OFS);
		read512_sse(addr, 320 + RELATIVE_OFS);
		read512_sse(addr, 384 + RELATIVE_OFS);
		read512_sse(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 1024
		addr += 512;
		read512_sse(addr,   0 + RELATIVE_OFS);
		read512_sse(addr,  64 + RELATIVE_OFS);
		read512_sse(addr, 128 + RELATIVE_OFS);
		read512_sse(addr, 192 + RELATIVE_OFS);
		read512_sse(addr, 256 + RELATIVE_OFS);
		read512_sse(addr, 320 + RELATIVE_OFS);
		read512_sse(addr, 384 + RELATIVE_OFS);
		read512_sse(addr, 448 + RELATIVE_OFS);

		addr += 512;
		read512_sse(addr,   0 + RELATIVE_OFS);
		read512_sse(addr,  64 + RELATIVE_OFS);
		read512_sse(addr, 128 + RELATIVE_OFS);
		read512_sse(addr, 192 + RELATIVE_OFS);
		read512_sse(addr, 256 + RELATIVE_OFS);
		read512_sse(addr, 320 + RELATIVE_OFS);
		read512_sse(addr, 384 + RELATIVE_OFS);
		read512_sse(addr, 448 + RELATIVE_OFS);
#endif
	}
	return NULL;
}
#endif

#ifdef __AVX__
static inline void read512_avx(const char *addr, const unsigned long ofs)
{
	__m256i xmm0, xmm1;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1) :
	             "0" (_mm256_load_si256((void *)(addr + ofs +  0))),
	             "1" (_mm256_load_si256((void *)(addr + ofs + 32))));
}

static inline void read1024_avx(const char *addr, const unsigned long ofs)
{
	__m256i xmm0, xmm1, xmm2, xmm3;
	asm volatile("" : "=xm" (xmm0), "=xm" (xmm1), "=xm" (xmm2), "=xm" (xmm3) :
	             "0" (_mm256_load_si256((void *)(addr + ofs +  0))),
	             "1" (_mm256_load_si256((void *)(addr + ofs + 32))),
	             "2" (_mm256_load_si256((void *)(addr + ofs + 64))),
	             "3" (_mm256_load_si256((void *)(addr + ofs + 96))));
}

/* runs the 512-bit test */
void *run512_avx(void *private)
{
	struct stats *ctx = private;
	size_t size = ctx->size;
	size_t mask = ctx->mask;
	void *area = ctx->area;
	const char *addr;
	uint64_t rnd;

	memset(area, 0, size);

	while (slowstart)
		;

	thread_num = ctx->thr;
	area -= RELATIVE_OFS;
	for (rnd = ctx->rnd; !stop_now; ) {
		__atomic_store_n(&ctx->rnd, rnd, __ATOMIC_RELEASE);
		addr = area + (rnd & mask);
		rnd += BYTES_PER_ROUND;

		__builtin_prefetch(addr + 1024 + RELATIVE_OFS, 0);

		read512_avx(addr,   0 + RELATIVE_OFS);
		read512_avx(addr,  64 + RELATIVE_OFS);
		read512_avx(addr, 128 + RELATIVE_OFS);
		read512_avx(addr, 192 + RELATIVE_OFS);

#if BYTES_PER_ROUND > 256
		read512_avx(addr, 256 + RELATIVE_OFS);
		read512_avx(addr, 320 + RELATIVE_OFS);
		read512_avx(addr, 384 + RELATIVE_OFS);
		read512_avx(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 512
		addr += 512;
		read512_avx(addr,   0 + RELATIVE_OFS);
		read512_avx(addr,  64 + RELATIVE_OFS);
		read512_avx(addr, 128 + RELATIVE_OFS);
		read512_avx(addr, 192 + RELATIVE_OFS);
		read512_avx(addr, 256 + RELATIVE_OFS);
		read512_avx(addr, 320 + RELATIVE_OFS);
		read512_avx(addr, 384 + RELATIVE_OFS);
		read512_avx(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 1024
		addr += 512;
		read512_avx(addr,   0 + RELATIVE_OFS);
		read512_avx(addr,  64 + RELATIVE_OFS);
		read512_avx(addr, 128 + RELATIVE_OFS);
		read512_avx(addr, 192 + RELATIVE_OFS);
		read512_avx(addr, 256 + RELATIVE_OFS);
		read512_avx(addr, 320 + RELATIVE_OFS);
		read512_avx(addr, 384 + RELATIVE_OFS);
		read512_avx(addr, 448 + RELATIVE_OFS);

		addr += 512;
		read512_avx(addr,   0 + RELATIVE_OFS);
		read512_avx(addr,  64 + RELATIVE_OFS);
		read512_avx(addr, 128 + RELATIVE_OFS);
		read512_avx(addr, 192 + RELATIVE_OFS);
		read512_avx(addr, 256 + RELATIVE_OFS);
		read512_avx(addr, 320 + RELATIVE_OFS);
		read512_avx(addr, 384 + RELATIVE_OFS);
		read512_avx(addr, 448 + RELATIVE_OFS);
#endif
	}
	return NULL;
}
#endif


#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
static inline void read512_vfp(const char *addr, const unsigned long ofs)
{
	asm volatile("vldr %%d0, [%0,%1]\n\t"
	             "vldr %%d1, [%0,%1+8]\n\t"
	             "vldr %%d2, [%0,%1+16]\n\t"
	             "vldr %%d3, [%0,%1+24]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs)
	             : "%d0", "%d1", "%d2", "%d3");

	asm volatile("vldr %%d4, [%0,%1+32]\n\t"
	             "vldr %%d5, [%0,%1+40]\n\t"
	             "vldr %%d6, [%0,%1+48]\n\t"
	             "vldr %%d7, [%0,%1+56]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs)
	             : "%d4", "%d5", "%d6", "%d7");
}

/* runs the 512-bit test */
void *run512_vfp(void *private)
{
	struct stats *ctx = private;
	size_t size = ctx->size;
	size_t mask = ctx->mask;
	void *area = ctx->area;
	const char *addr;
	uint64_t rnd;

	memset(area, 0, size);

	while (slowstart)
		;

	thread_num = ctx->thr;
	area -= RELATIVE_OFS;
	for (rnd = ctx->rnd; !stop_now; ) {
		__atomic_store_n(&ctx->rnd, rnd, __ATOMIC_RELEASE);
		addr = area + (rnd & mask);
		rnd += BYTES_PER_ROUND;

		read512_vfp(addr,   0 + RELATIVE_OFS);
		read512_vfp(addr,  64 + RELATIVE_OFS);
		read512_vfp(addr, 128 + RELATIVE_OFS);
		read512_vfp(addr, 192 + RELATIVE_OFS);

#if BYTES_PER_ROUND > 256
		read512_vfp(addr, 256 + RELATIVE_OFS);
		read512_vfp(addr, 320 + RELATIVE_OFS);
		read512_vfp(addr, 384 + RELATIVE_OFS);
		read512_vfp(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 512
		addr += 512;
		read512_vfp(addr,   0 + RELATIVE_OFS);
		read512_vfp(addr,  64 + RELATIVE_OFS);
		read512_vfp(addr, 128 + RELATIVE_OFS);
		read512_vfp(addr, 192 + RELATIVE_OFS);
		read512_vfp(addr, 256 + RELATIVE_OFS);
		read512_vfp(addr, 320 + RELATIVE_OFS);
		read512_vfp(addr, 384 + RELATIVE_OFS);
		read512_vfp(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 1024
		addr += 512;
		read512_vfp(addr,   0 + RELATIVE_OFS);
		read512_vfp(addr,  64 + RELATIVE_OFS);
		read512_vfp(addr, 128 + RELATIVE_OFS);
		read512_vfp(addr, 192 + RELATIVE_OFS);
		read512_vfp(addr, 256 + RELATIVE_OFS);
		read512_vfp(addr, 320 + RELATIVE_OFS);
		read512_vfp(addr, 384 + RELATIVE_OFS);
		read512_vfp(addr, 448 + RELATIVE_OFS);

		addr += 512;
		read512_vfp(addr,   0 + RELATIVE_OFS);
		read512_vfp(addr,  64 + RELATIVE_OFS);
		read512_vfp(addr, 128 + RELATIVE_OFS);
		read512_vfp(addr, 192 + RELATIVE_OFS);
		read512_vfp(addr, 256 + RELATIVE_OFS);
		read512_vfp(addr, 320 + RELATIVE_OFS);
		read512_vfp(addr, 384 + RELATIVE_OFS);
		read512_vfp(addr, 448 + RELATIVE_OFS);
#endif
	}
	return NULL;
}
#endif

#if defined(__ARM_ARCH_7A__)
static inline void read512_armv7(const char *addr, const unsigned long ofs)
{
	asm volatile("ldmia %0, { r4-r11 }" :: "r" (addr + ofs +  0) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
	asm volatile("ldmia %0, { r4-r11 }" :: "r" (addr + ofs + 32) : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11");
}

/* runs the 512-bit test */
void *run512_armv7(void *private)
{
	struct stats *ctx = private;
	size_t size = ctx->size;
	size_t mask = ctx->mask;
	void *area = ctx->area;
	const char *addr;
	uint64_t rnd;

	memset(area, 0, size);

	while (slowstart)
		;

	thread_num = ctx->thr;
	area -= RELATIVE_OFS;
	for (rnd = ctx->rnd; !stop_now; ) {
		__atomic_store_n(&ctx->rnd, rnd, __ATOMIC_RELEASE);
		addr = area + (rnd & mask);
		rnd += BYTES_PER_ROUND;

		read512_armv7(addr,   0 + RELATIVE_OFS);
		read512_armv7(addr,  64 + RELATIVE_OFS);
		read512_armv7(addr, 128 + RELATIVE_OFS);
		read512_armv7(addr, 192 + RELATIVE_OFS);

#if BYTES_PER_ROUND > 256
		read512_armv7(addr, 256 + RELATIVE_OFS);
		read512_armv7(addr, 320 + RELATIVE_OFS);
		read512_armv7(addr, 384 + RELATIVE_OFS);
		read512_armv7(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 512
		addr += 512;
		read512_armv7(addr,   0 + RELATIVE_OFS);
		read512_armv7(addr,  64 + RELATIVE_OFS);
		read512_armv7(addr, 128 + RELATIVE_OFS);
		read512_armv7(addr, 192 + RELATIVE_OFS);
		read512_armv7(addr, 256 + RELATIVE_OFS);
		read512_armv7(addr, 320 + RELATIVE_OFS);
		read512_armv7(addr, 384 + RELATIVE_OFS);
		read512_armv7(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 1024
		addr += 512;
		read512_armv7(addr,   0 + RELATIVE_OFS);
		read512_armv7(addr,  64 + RELATIVE_OFS);
		read512_armv7(addr, 128 + RELATIVE_OFS);
		read512_armv7(addr, 192 + RELATIVE_OFS);
		read512_armv7(addr, 256 + RELATIVE_OFS);
		read512_armv7(addr, 320 + RELATIVE_OFS);
		read512_armv7(addr, 384 + RELATIVE_OFS);
		read512_armv7(addr, 448 + RELATIVE_OFS);

		addr += 512;
		read512_armv7(addr,   0 + RELATIVE_OFS);
		read512_armv7(addr,  64 + RELATIVE_OFS);
		read512_armv7(addr, 128 + RELATIVE_OFS);
		read512_armv7(addr, 192 + RELATIVE_OFS);
		read512_armv7(addr, 256 + RELATIVE_OFS);
		read512_armv7(addr, 320 + RELATIVE_OFS);
		read512_armv7(addr, 384 + RELATIVE_OFS);
		read512_armv7(addr, 448 + RELATIVE_OFS);
#endif
	}
	return NULL;
}
#endif

#if defined(__ARM_ARCH_8A) || defined(__AARCH64EL__)
static inline void read512_armv8(const char *addr, const unsigned long ofs)
{
	asm volatile("ldnp q0, q1, [%0,#%1]\n\t"
	             "ldnp q2, q3, [%0,#%1+32]\n\t"
	             : /* no output */
	             : "r" (addr), "I" (ofs)
	             : "q0", "q1", "q2", "q3");
}

/* runs the 512-bit test using ARMv8 optimizations */
void *run512_armv8(void *private)
{
	struct stats *ctx = private;
	size_t size = ctx->size;
	size_t mask = ctx->mask;
	void *area = ctx->area;
	const char *addr;
	uint64_t rnd;

	memset(area, 0, size);

	while (slowstart)
		;

	thread_num = ctx->thr;
	area -= RELATIVE_OFS;
	for (rnd = ctx->rnd; !stop_now; ) {
		__atomic_store_n(&ctx->rnd, rnd, __ATOMIC_RELEASE);
		addr = area + (rnd & mask);
		rnd += BYTES_PER_ROUND;

		read512_armv8(addr,   0 + RELATIVE_OFS);
		read512_armv8(addr,  64 + RELATIVE_OFS);
		read512_armv8(addr, 128 + RELATIVE_OFS);
		read512_armv8(addr, 192 + RELATIVE_OFS);

#if BYTES_PER_ROUND > 256
		read512_armv8(addr, 256 + RELATIVE_OFS);
		read512_armv8(addr, 320 + RELATIVE_OFS);
		read512_armv8(addr, 384 + RELATIVE_OFS);
		read512_armv8(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 512
		addr += 512;
		read512_armv8(addr,   0 + RELATIVE_OFS);
		read512_armv8(addr,  64 + RELATIVE_OFS);
		read512_armv8(addr, 128 + RELATIVE_OFS);
		read512_armv8(addr, 192 + RELATIVE_OFS);
		read512_armv8(addr, 256 + RELATIVE_OFS);
		read512_armv8(addr, 320 + RELATIVE_OFS);
		read512_armv8(addr, 384 + RELATIVE_OFS);
		read512_armv8(addr, 448 + RELATIVE_OFS);
#endif

#if BYTES_PER_ROUND > 1024
		addr += 512;
		read512_armv8(addr,   0 + RELATIVE_OFS);
		read512_armv8(addr,  64 + RELATIVE_OFS);
		read512_armv8(addr, 128 + RELATIVE_OFS);
		read512_armv8(addr, 192 + RELATIVE_OFS);
		read512_armv8(addr, 256 + RELATIVE_OFS);
		read512_armv8(addr, 320 + RELATIVE_OFS);
		read512_armv8(addr, 384 + RELATIVE_OFS);
		read512_armv8(addr, 448 + RELATIVE_OFS);

		addr += 512;
		read512_armv8(addr,   0 + RELATIVE_OFS);
		read512_armv8(addr,  64 + RELATIVE_OFS);
		read512_armv8(addr, 128 + RELATIVE_OFS);
		read512_armv8(addr, 192 + RELATIVE_OFS);
		read512_armv8(addr, 256 + RELATIVE_OFS);
		read512_armv8(addr, 320 + RELATIVE_OFS);
		read512_armv8(addr, 384 + RELATIVE_OFS);
		read512_armv8(addr, 448 + RELATIVE_OFS);
#endif
	}
	return NULL;
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

/* sets the start_time() value as accurately as possible */
static inline void set_start_time()
{
	uint64_t before, after;

	after = rdtsc();
	before = rdtsc();
	// compensate for the syscall time
	before += before - after;
	start_time = before;
}

/* If interval_usec is zero, just marks the alarm as received, otherwise
 * prints stats and rearms the timer for the same duration.
 */
void alarm_handler(int sig)
{
	uint64_t now, usec, rounds;
	int thr;

	//printf("thread_num=%d\n", thread_num);

	if (slowstart) {
		/* that was the pre-heating phase */
		slowstart = 0;
		return;
	}

	/* measure the time since last pass, only on first thread */
	now = rdtsc();

	for (rounds = thr = 0; thr < nbthreads; thr++) {
		stats[thr].last = stats[thr].rnd;
		//printf("thr %d : %llu\n", thr, stats[thr].last - stats[thr].prev);
		rounds += stats[thr].last - stats[thr].prev;
		stats[thr].prev = stats[thr].last;
	}

	/* speed = rounds per microsecond. Use 64-bit computations to avoid
	 * overflows.
	 */
	usec = now - start_time;
	if (usec < 1)
		usec = 1;

	rounds /= usec; // express it in B/us = MB/s
	printf("%llu\n", (unsigned long long)rounds);
	//printf("now=%llu usec=%llu %llu intv=%d\n", now, usec, (unsigned long long)rounds);

	if (meas_count && --meas_count) {
		/* rearm the timer for another measure */
		start_time = now;
		set_alarm(interval_usec);
	}

	if (!meas_count)
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
	setitimer(ITIMER_REAL, &timer, NULL);
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

/* Access aligned words of optimal size over <size> bytes for each thread.
 * Note: size is rounded down to the lower power of two, and must be at
 * least 4kB.
 */
unsigned int random_read_over_area(size_t size)
{
	size_t mask;
	int thr;

	mask = mask_rounded_down(size);


	if (!run)
		return 0;

	/* create threads for thread 1 and above */
	for (thr = 0; thr < nbthreads; thr++) {
		stats[thr].size = size;
		stats[thr].mask = mask;
		stats[thr].thr = thr;
		stats[thr].rnd = 0;

		stats[thr].area = memalign(size / 4, size);
		if (!stats[thr].area) {
			printf("Failed to allocate memory for thread %d\n", thr);
			exit(1);
		}

#ifdef MADV_DONTDUMP
		madvise(stats[thr].area, size, MADV_DONTDUMP);
#endif
#ifdef MADV_HUGEPAGE
		if (no_hugepages)
			madvise(stats[thr].area, size, MADV_NOHUGEPAGE);
		else
			madvise(stats[thr].area, size, MADV_HUGEPAGE);
#endif
		if (thr > 0 && pthread_create(&stats[thr].pth, NULL, run, &stats[thr]) < 0) {
			fprintf(stderr, "Failed to start thread #%d; aborting.\n", thr);
			exit(1);
		}
	}

	if (slowstart) {
		set_alarm(500000);
		while (slowstart)
			;
		set_alarm(0);
	}

	set_alarm(interval_usec);

	set_start_time();

	run(&stats[0]);

	set_alarm(0);
	return 0;
}

int main(int argc, char **argv)
{
	unsigned int usec;
	size_t size, size_thr;
	int implementation;

	/* set default implementation bits */
	implementation = USE_GENERIC;
#ifdef __SSE4_1__
	/* don't enable it by default when there's only SSE2, as it's slower
	 * than generic.
	 */
	implementation |= USE_SSE;
#endif
#ifdef __AVX__
	implementation = USE_AVX;
#endif
#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
	implementation |= USE_VFP;
#endif
#if defined(__ARM_ARCH_7A__)
	implementation |= USE_ARMV7;
#endif
#if defined(__ARM_ARCH_8A) || defined(__AARCH64EL__)
	implementation |= USE_ARMV8;
#endif
	usec = 100000;
	size = 16 * 1048576;

	while (argc > 1 && *argv[1] == '-') {
		if (strcmp(argv[1], "-s") == 0) {
			slowstart = 1;
		}
		else if (strcmp(argv[1], "-H") == 0) {
			no_hugepages = 1;
		}
		else if (strcmp(argv[1], "-t") == 0 && argc > 2) {
			nbthreads = atoi(argv[2]);
			argc--; argv++;
		}
		else if (strcmp(argv[1], "-G") == 0) {
			implementation = USE_GENERIC;
		}
#ifdef __SSE2__
		else if (strcmp(argv[1], "-S") == 0) {
			implementation = USE_SSE;
		}
#endif
#ifdef __AVX__
		else if (strcmp(argv[1], "-A") == 0) {
			implementation = USE_AVX;
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
#if defined(__ARM_ARCH_8A) || defined(__AARCH64EL__)
		else if (strcmp(argv[1], "-8") == 0) {
			implementation = USE_ARMV8;
		}
#endif
		else {
			fprintf(stderr,
				"Usage: prog [options]* [<time_ms> [<count> [<size_kB>]]]\n"
				"  -t <threads> : start this number of threads each with its own area.\n"
				"  -s : slowstart : pre-heat for 500ms to let cpufreq adapt\n"
#ifdef MADV_HUGEPAGE
				"  -H : disable Huge Pages when supported\n"
#endif
				"  -h : show this help\n"
				"  -G : use generic code only\n"
#ifdef __SSE2__
				"  -S : use SSE\n"
#endif
#ifdef __AVX__
				"  -A : use AVX\n"
#endif
#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
				"  -V : use VFP\n"
#endif
#if defined(__ARM_ARCH_7A__)
				"  -7 : use ARMv7\n"
#endif
#if defined(__ARM_ARCH_8A) || defined(__AARCH64EL__)
				"  -8 : use ARMv8\n"
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
		meas_count = atoi(argv[2]);

	if (argc > 3)
		size = atol(argv[3]) * 1024;

	run = run512_generic;

#ifdef __SSE2__
	if (implementation & USE_SSE) {
		run = run512_sse;
	}
#endif
#ifdef __AVX__
	if (implementation & USE_AVX) {
		run = run512_avx;
	}
#endif
#if defined(__ARM_ARCH_7A__)
	if (implementation & USE_ARMV7) {
		run = run512_armv7;
	}
#endif
#if defined (__VFP_FP__) && defined(__ARM_ARCH_7A__)
	if (implementation & USE_VFP) {
		run = run512_vfp;
	}
#endif
#if defined(__ARM_ARCH_8A) || defined(__AARCH64EL__)
	if (implementation & USE_ARMV8) {
		run = run512_armv8;
	}
#endif

	interval_usec = usec;
	meas_count = meas_count > 0 ? meas_count : 1;
	if (nbthreads < 1 || nbthreads > MAX_THREADS) {
		fprintf(stderr, "Fatal: invalid number of threads, accepted range is 1..%d.\n", MAX_THREADS);
		exit(1);
	}

	size_thr = size / nbthreads;

	/* round it down to the largest power of 2 */
	while (size_thr & (size_thr - 1))
		size_thr = size_thr & (size_thr - 1);

	if (size_thr < 1024) {
		fprintf(stderr, "Fatal: too small area size, minimum is 1kB per thread\n");
		exit(1);
	}

	if (size_thr * nbthreads != size)
		fprintf(stderr, "Notice: using %lu bytes per thread (%lu kB total)\n",
			(unsigned long)size_thr, (unsigned long)(size_thr * nbthreads) / 1024);

	random_read_over_area(size_thr);
	exit(0);
}
