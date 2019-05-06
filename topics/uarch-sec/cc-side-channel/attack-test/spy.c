#include "common.h"

typedef enum {
    reset = 0,
    recv = 1,
    boundary = 2
} SpyState;

#define AVG_SIZE 1000

unsigned long long avg(unsigned long long arr[AVG_SIZE]) {
    unsigned long long sum = 0;
    for (int i = 0; i < AVG_SIZE; i++) {
        sum += arr[i];
    }
    return sum / AVG_SIZE;
}

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

static void wait_cycles(unsigned long long num_cycles) {
    volatile unsigned long long endtime = rdtsc() + num_cycles;
    while (rdtsc() < endtime) {}
}

static inline void flush(const char *adrs) {
    asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (adrs) :);
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

int main() {
    //printf("Started Spy\n");
    pin_thread(6);

    char msg[1000];
    int msg_idx = 0;


    void * shlib = dlopen("./shlib.so", RTLD_LAZY);
    void * mem = dlsym(shlib, "get_foo");

    unsigned long long tarr[AVG_SIZE];
    int cur_idx = 0;

    SpyState state = reset;
    uint8_t cur_byte = 0;
    uint8_t cur_bit = 0;

    unsigned long long t = 0;
    int counter = 0;
    int reset_counter = 0;

    for (int s = 0; s < T_step; s++) {
        tarr[cur_idx] = time_access_no_flush((void *)mem);
        flush(mem);
        cur_idx = (cur_idx + 1) % AVG_SIZE;
    }

    unsigned long long next_sample_time = rdtsc() + T_step;

    unsigned long long wait_count = 0;

    while (1) {
        while (rdtsc() < next_sample_time) {wait_count += 1;}
        next_sample_time += T_step;
        tarr[cur_idx] = time_access_no_flush((void *)mem);
        flush(mem);

        cur_idx = (cur_idx + 1) % AVG_SIZE;
        t = avg(tarr);

        //continue;
        if (state == reset && t > CYC_thresh) {
            state = recv;
            counter = 0;
            cur_byte = 0;
            cur_bit = 0;
        }
        else if (state == recv) {
            counter += 1;

            if (t < CYC_thresh) {
                state = boundary;
                reset_counter = 0;

                // printf("%d\n", counter);

                if (counter > T_thresh) {
                    cur_byte |= (1 << cur_bit);
                }
                cur_bit++;

                if (cur_bit == 8) {
                    msg[msg_idx++] = cur_byte;
                    if (msg_idx >= 1000) msg_idx = 0;
                    cur_byte = 0;
                    cur_bit = 0;
                }
            }
        }
        else if (state == boundary) {
            reset_counter += 1;

            if (t > CYC_thresh) {
                if (reset_counter < T_boundary - T_tol) {
                    state = reset;
                }
                state = recv;
                counter = 0;
            }

            if (reset_counter > T_reset - 10) {
                reset_counter = 0;
                state = reset;
                msg[msg_idx] = 0;

                for (int i = 0; i < msg_idx; i++) {
                    if (!((msg[i] >= 'A' && msg[i] <= 'Z') || (msg[i] >= 'a' && msg[i] <= 'z'))) {
                        msg[i] = ' ';
                    }
                }
                printf("\nReset\n");
                printf("msg = /%s/\n", msg);
                printf("msg_idx = %d\n", msg_idx);
                printf("wait_count = %lld\n", wait_count);
                wait_count = 0;
                msg_idx = 0;
                fflush(stdout);
            }
        }
    }

    return 0;
}
