#include "alloc.h"

// Data structure to track memory blocks
struct mem_block {
    int offset;
    int size;
    int free;
    struct mem_block *next;
};

static char *memory = NULL; // pointer to 4KB memory
static struct mem_block *head = NULL;

// Initialize memory manager
int init_alloc() {
    memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (memory == MAP_FAILED)
        return -1;

    head = malloc(sizeof(struct mem_block));
    if (!head)
        return -1;

    head->offset = 0;
    head->size = PAGESIZE;
    head->free = 1;
    head->next = NULL;

    return 0;
}

// Cleanup memory manager
int cleanup() {
    struct mem_block *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }

    if (munmap(memory, PAGESIZE) == -1)
        return -1;

    return 0;
}

// Allocate memory
char *alloc(int size) {
    if (size <= 0 || size % MINALLOC != 0)
        return NULL;

    struct mem_block *current = head;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size > size) {
                struct mem_block *new_block = malloc(sizeof(struct mem_block));
                if (!new_block)
                    return NULL;

                new_block->offset = current->offset + size;
                new_block->size = current->size - size;
                new_block->free = 1;
                new_block->next = current->next;

                current->size = size;
                current->free = 0;
                current->next = new_block;
            } else {
                current->free = 0;
            }
            return memory + current->offset;
        }
        current = current->next;
    }

    return NULL;
}

// Merge adjacent free blocks
void merge_free_blocks() {
    struct mem_block *current = head;

    while (current && current->next) {
        if (current->free && current->next->free) {
            struct mem_block *next_block = current->next;
            current->size += next_block->size;
            current->next = next_block->next;
            free(next_block);
        } else {
            current = current->next;
        }
    }
}

// Deallocate memory
void dealloc(char *ptr) {
    if (!ptr || ptr < memory || ptr >= memory + PAGESIZE)
        return;

    struct mem_block *current = head;
    int offset = ptr - memory;

    while (current) {
        if (current->offset == offset) {
            current->free = 1;
            merge_free_blocks();
            return;
        }
        current = current->next;
    }
}
