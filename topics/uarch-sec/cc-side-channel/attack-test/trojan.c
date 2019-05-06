#include "common.h"
#include <stdlib.h>

typedef enum {
    shared,
    exclusive
} MemState;

volatile MemState mem_state = 0;

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

void * thread0(void * arg) {
    uint64_t * rptr = arg;
    uint64_t rdata;

    pin_thread(0);

    while (1) {
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
    uint64_t rdata;

    pin_thread(1);

    while (1) {
        if (mem_state == shared) {
            asm __volatile__ (
                "  movl (%0), %%eax   \n"
                "  lfence             \n"
                :
                : "c" (rptr)
                :  "%eax");
        }
        else {
            while (mem_state != shared) { }
        }
    }

    return NULL;
}

static inline void send_bit(uint8_t bit) {
    int wait_time = bit == 0 ? T_zero : T_one;

    mem_state = exclusive;
    wait_cycles(wait_time * T_step);

    mem_state = shared;
    wait_cycles(T_boundary * T_step);
}

void send_msg(const char msg[], int length) {
    for (int i = 0; i < length; i++) {
        char ch = msg[i];
        // Here I manually unroll the loop to avoid instruction overhead of
        // maintaining the loop state (just makes things more deterministic)
        send_bit(ch & 0x01);
        send_bit(ch & 0x02);
        send_bit(ch & 0x04);
        send_bit(ch & 0x08);
        send_bit(ch & 0x10);
        send_bit(ch & 0x20);
        send_bit(ch & 0x40);
        send_bit(ch & 0x80);
    }
}

int main(int argc, char * argv[]) {
    printf("Started Trojan\n");
    const char msg[] = "HELLO WORLD";
    const int msg_len = strlen(msg);

    void * dll = dlopen("./shlib.so", RTLD_LAZY);
    void * mem = dlsym(dll, "get_foo");

    mem_state = shared;

    pthread_t thr0, thr1;

    pthread_create(&thr0, NULL, thread0, mem);
    pthread_create(&thr1, NULL, thread1, mem);


    printf("Reset\n");
    wait_cycles(T_reset * T_step * 2);
    printf("Running\n");

    while (1) {
        printf("Sending Message...\n");
        send_msg(msg, msg_len);
        printf("Reset\n");
        wait_cycles(T_reset * T_step * 2);
        //send_bit(1);
    }

    return 0;
}
