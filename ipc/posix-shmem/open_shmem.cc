#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>


int main() {
    const int mem_size = 128;
    const char * name = "/tmp";
    int shmfd;
    void * mem;
    uint8_t * bytes;

    //
    // Open a shared memory object. Note that name must match exactly with the
    // producer's name. Also, no O_CREAT here since this process does not create
    // the section, just maps it.
    //

    shmfd = shm_open(name, O_RDWR, 0666);

    if (shmfd < 0) {
        perror("shm_open");
        return 1;
    }

    //
    // Map it into our address space.
    //

    mem = mmap(0, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    //
    // Read the data written to the shared section from the producer.
    //

    bytes = (uint8_t *)mem;

    for (int i = 0; i < mem_size; i++) {
        printf("bytes[%d] = %d\n", i, bytes[i]);
    }

    //
    // Unmap the shared section.
    //

    if (munmap(mem, mem_size) == -1) {
        perror("munmap");
        return 1;
    }

    //
    // Close it.
    //

    if (close(shmfd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}