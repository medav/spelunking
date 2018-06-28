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
    // Create a shared memory object. Note: name must point to a valid path in
    // the filesystem and can be a folder. O_CREAT is used by the owner of the
    // shmem section to create it. 0666 is the permissions.
    //

    shmfd = shm_open(name, O_CREAT | O_RDWR, 0666);

    if (shmfd < 0) {
        perror("shm_open");
        return 1;
    }

    //
    // Set the size of the shared memory section.
    //

    if (ftruncate(shmfd, mem_size) != 0) {
        perror("ftruncate");
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
    // In this trivial example, just write some values into the shared memory
    // section then wait for the user to press a button to continue.
    //

    bytes = (uint8_t *)mem;

    for (int i = 0; i < mem_size; i++) {
        bytes[i] = i;
    }

    scanf(" ");

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


    //
    // Unlink it from the filesystem. This deletes the section.
    //

    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        return 1;
    }

    return 0;
}