#include "common.h"
#include <stdlib.h>

#define SAMPLE_SIZE 100

typedef enum {
    shared,
    exclusive
} MemState;

pthread_barrier_t barrier;

volatile MemState mem_state = 0;
volatile int done = 0;

static inline void flush(const char *adrs) {
#ifndef GEM5MODE
    asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (adrs) :);
#endif
}

static inline unsigned long long time_access_no_flush(const char *adrs) {
    volatile unsigned long time;
    asm __volatile__ (
        "  mfence             \n"
        "  lfence             \n"
        "  rdtsc              \n"
        "  lfence             \n"
        "  movl %%eax, %%esi  \n"
        "  movl (%1), %%eax   \n"
        "  lfence             \n"
        "  rdtsc              \n"
        "  subl %%esi, %%eax  \n"
        : "=a" (time)
        : "c" (adrs)
        :  "%esi", "%edx");
    return time;
}

static __inline__ unsigned long long rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void * thread0(void * arg) {
    uint64_t * rptr = arg;

#ifndef GEM5MODE
    pin_thread(0);
#endif

    while (!done) {
        asm __volatile__ (
            "  movl (%0), %%eax   \n"
            "  lfence             \n"
            :
            : "c" (rptr)
            :  "%eax");
    }

    return NULL;
}

void * thread1(void * arg) {
    uint64_t * rptr = arg;

#ifndef GEM5MODE
    pin_thread(1);
#endif

    if (mem_state != shared) {
        while (!done) {}
    }
    else {
        while (!done) {
            asm __volatile__ (
                "  movl (%0), %%eax   \n"
                "  lfence             \n"
                :
                : "c" (rptr)
                :  "%eax");
        }
    }

    return NULL;
}

void* spy_thread(void* arg) {

#ifndef GEM5MODE
    pin_thread(2);
#endif

    int i;
    int num_samples = 0;
    unsigned long long sample_time = 0;
    unsigned long long total_time = 0;
    unsigned long long min = (unsigned long long) -1;
    unsigned long long max = 0;

    usleep(100);

    while (!done) {
        for (i = 0; i < SAMPLE_SIZE; i++) {
            sample_time += time_access_no_flush(arg);
            flush(arg);
            usleep(50);
        }

        sample_time /= SAMPLE_SIZE;
        total_time += sample_time;
        num_samples++;

        if (num_samples > 10) {
            if (sample_time < min) min = sample_time;
            if (sample_time > max) max = sample_time;
        }

        printf(
            "%llu\t%llu\t%llu\t%llu\n", 
            sample_time,
            min,
            total_time / num_samples,
            max);

        fflush(stdout);

        sample_time = 0;
    }
}

void usage() {
    printf("Usage: ./trojan (s|e)\n");
}

int main(int argc, char * argv[]) {

#ifndef GEM5MODE
    pin_thread(0);
#endif

    void * mem = malloc(8);
    void * ret;

    if (argc != 2) {
        usage();
        return 0;
    }

    if (strcmp(argv[1], "s") == 0) {
        mem_state = shared;
    }
    else if (strcmp(argv[1], "e") == 0) {
        mem_state = exclusive;
    }
    else {
        usage();
        return 0;
    }

    pthread_t thr0, thr1, thr3;

    fprintf(stderr, "%s\n", (mem_state == shared) ? "shared" : "exclusive");

    flush(mem);

    pthread_create(&thr0, NULL, thread0, mem);
    pthread_create(&thr1, NULL, thread1, mem);

    usleep(100);
    pthread_create(&thr3, NULL, spy_thread, mem);

#ifdef GEM5MODE
    usleep(1000 * 1000);
#else
    unsigned long long tend = rdtsc() + 20000000000;
    while (rdtsc() < tend) {}
    done = 1;
#endif

    pthread_join(thr3, &ret);
    pthread_join(thr0, &ret);
    pthread_join(thr1, &ret);

    return 0;
}
