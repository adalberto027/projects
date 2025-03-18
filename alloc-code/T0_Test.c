#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096  // standard page size (4KB)

int main() {
    // print process ID
    printf("Process running. PID: %d\n", getpid());

    // memory map an anonymous 4KB page
    void *mapped_mem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    printf("Memory mapped at address: %p\n", mapped_mem);
    printf("Press Enter to exit...\n");

    // wait for a user input
    getchar();

    // clean up the mapped memory
    munmap(mapped_mem, PAGE_SIZE);
    return 0;
}
