#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

typedef struct FreeBlock {
    size_t size;
    struct FreeBlock *next;
} FreeBlock;

static void *memory_page = NULL;
static FreeBlock *free_list = NULL;

int init_alloc() {
    memory_page = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (memory_page == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }
    free_list = (FreeBlock *)memory_page;
    free_list->size = PAGESIZE - sizeof(FreeBlock);
    free_list->next = NULL;
    return 0;
}

int cleanup() {
    if (memory_page) {
        if (munmap(memory_page, PAGESIZE) == 0) {
            memory_page = NULL;
            free_list = NULL;
            return 0; // Success
        }
        return -1; // Error in munmap
    }
    return 0; // Nothing to clean up
}

char *alloc(int size) {
    if (size <= 0 || size % MINALLOC != 0 || size > PAGESIZE) {
        printf("Invalid allocation request: %d bytes\n", size);
        return NULL;
    }

    FreeBlock *prev = NULL;
    FreeBlock *curr = free_list;

    while (curr) {
        if (curr->size >= size) {
            if (curr->size > size + sizeof(FreeBlock)) {
                FreeBlock *new_block = (FreeBlock *)((char *)curr + sizeof(FreeBlock) + size);
                new_block->size = curr->size - size - sizeof(FreeBlock);
                new_block->next = curr->next;

                if (prev) {
                    prev->next = new_block;
                } else {
                    free_list = new_block;
                }
                curr->size = size;
            } else {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    free_list = curr->next;
                }
            }

            printf("Allocated %d bytes at: %p\n", size, (void *)((char *)curr + sizeof(FreeBlock)));
            return (char *)((char *)curr + sizeof(FreeBlock));
        }
        prev = curr;
        curr = curr->next;
    }
    
    printf("Allocation failed for %d bytes\n", size);
    return NULL;
}

void dealloc(char *ptr) {
    if (!ptr || ptr < (char *)memory_page || ptr >= (char *)memory_page + PAGESIZE) {
        printf("Invalid dealloc request at: %p\n", ptr);
        return;
    }

    printf("Freeing memory at: %p\n", ptr);
    FreeBlock *block = (FreeBlock *)((char *)ptr - sizeof(FreeBlock));
    block->next = free_list;
    free_list = block;

    // Merge adjacent free blocks
    FreeBlock *current = free_list;
    while (current && current->next) {
        if ((char *)current + current->size + sizeof(FreeBlock) == (char *)current->next) {
            current->size += current->next->size + sizeof(FreeBlock);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}
