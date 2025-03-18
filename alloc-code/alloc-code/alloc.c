#include "alloc.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PAGESIZE 4096
#define MINALLOC 8

// Estructura para gestionar los bloques de memoria
struct mem_block {
    int offset;
    int size;
    int free;
    struct mem_block *next;
};

static char *memory = NULL;
static struct mem_block *head = NULL;

// Inicializa el administrador de memoria
int init_alloc() {
    memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (memory == MAP_FAILED)
        return -1;

    head = mmap(NULL, sizeof(struct mem_block), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (head == MAP_FAILED)
        return -1;

    head->offset = 0;
    head->size = PAGESIZE;
    head->free = 1;
    head->next = NULL;

    return 0;
}

// Libera el administrador de memoria
int cleanup() {
    struct mem_block *temp;
    while (head) {
        temp = head;
        head = head->next;
        munmap(temp, sizeof(struct mem_block));
    }

    if (munmap(memory, PAGESIZE) == -1)
        return -1;

    return 0;
}

// Fusiona bloques libres adyacentes
void merge_free_blocks() {
    struct mem_block *current = head;
    while (current && current->next) {
        if (current->free && current->next->free) {
            struct mem_block *next_block = current->next;
            current->size += next_block->size;
            current->next = next_block->next;
            munmap(next_block, sizeof(struct mem_block));
        } else {
            current = current->next;
        }
    }
}

// Asigna memoria
char *alloc(int size) {
    if (size <= 0 || size % MINALLOC != 0)
        return NULL;

    struct mem_block *current = head;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size > size + sizeof(struct mem_block)) {
                struct mem_block *new_block = mmap(NULL, sizeof(struct mem_block), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
                if (new_block == MAP_FAILED)
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

// Libera memoria asignada
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
