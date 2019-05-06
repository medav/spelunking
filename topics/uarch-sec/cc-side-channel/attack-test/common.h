#ifndef __COMMON__
#define __COMMON__

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sched.h>
#include <stdint.h>
#include <time.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

int pin_thread(int core_id) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
   if (core_id < 0 || core_id >= num_cores) {
      printf("Can't pin to non-existent core: %d\n", core_id);
      return 0;
    }

   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();
   int status = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);

  if (status != 0) perror("pthread_setaffinity_np");

  return status;
}

#define CYC_shared 70
#define CYC_exclusive 90
#define CYC_thresh (CYC_shared + CYC_exclusive) / 2

#define T_step 10000 // Number of cycles to wait between samples in spy

#define T_one 5000 // Assumed to always be greater than T_zero
#define T_zero 1000
#define T_thresh (T_one + T_zero) / 2
#define T_boundary 5000
#define T_tol 200
#define T_reset 20000

#endif
