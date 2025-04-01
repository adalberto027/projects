#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define PAGE_SIZE 4096  // 4KB

int main() {
    printf("Process running. PID: %d\n", getpid());

    // allocate a 4KB
    void *mapped_mem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    printf("Memory mapped at address: %p\n", mapped_mem);

    // wait before writing to memory
    printf("Press Enter to write data into memory...\n");
    getchar();

    // write data into the memory page
    strcpy((char *)mapped_mem, "Hello, mmap!");

    // wait again to observe memory usage after writing
    printf("Data written. Press Enter to exit...\n");
    getchar();

    // Cleanup
    munmap(mapped_mem, PAGE_SIZE);
    return 0;
}
