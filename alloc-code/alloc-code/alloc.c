#include "alloc.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PAGESIZE 4096
#define MINALLOC 8
#define MAX_BLOCKS (PAGESIZE / MINALLOC) // Máximo 512 bloques

// Structure to manage memory blocks
struct mem_block {
    int offset;
    int size;
    int free;
};

static char *memory = NULL;
static struct mem_block blocks[MAX_BLOCKS];
static int num_blocks = 0;

// Initialize the memory manager
int init_alloc() {
    memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (memory == MAP_FAILED)
        return -1;

    blocks[0].offset = 0;
    blocks[0].size = PAGESIZE;
    blocks[0].free = 1;
    num_blocks = 1;

    return 0;
}

// Cleanup memory manager
int cleanup() {
    if (munmap(memory, PAGESIZE) == -1)
        return -1;

    num_blocks = 0;

    return 0;
}

// Merge adjacent free blocks
void merge_free_blocks() {
    for (int i = 0; i < num_blocks - 1; i++) {
        if (blocks[i].free && blocks[i + 1].free) {
            blocks[i].size += blocks[i + 1].size;
            // Desplazar el arreglo hacia atrás
            for (int j = i + 1; j < num_blocks - 1; j++) {
                blocks[j] = blocks[j + 1];
            }
            num_blocks--;
            i--; // Revisar nuevamente el mismo índice
        }
    }
}

// Allocate memory
char *alloc(int size) {
    if (size <= 0 || size % MINALLOC != 0)
        return NULL;

    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i].free && blocks[i].size >= size) {
            if (blocks[i].size > size) {
                if (num_blocks >= MAX_BLOCKS)
                    return NULL; // No más bloques disponibles

                // Crear un nuevo bloque
                for (int j = num_blocks; j > i + 1; j--) {
                    blocks[j] = blocks[j - 1];
                }
                blocks[i + 1].offset = blocks[i].offset + size;
                blocks[i + 1].size = blocks[i].size - size;
                blocks[i + 1].free = 1;

                blocks[i].size = size;
                num_blocks++;
            }
            blocks[i].free = 0;
            return memory + blocks[i].offset;
        }
    }

    return NULL;
}

// Free allocated memory
void dealloc(char *ptr) {
    if (!ptr || ptr < memory || ptr >= memory + PAGESIZE)
        return;

    int offset = ptr - memory;

    for (int i = 0; i < num_blocks; i++) {
        if (blocks[i].offset == offset) {
            blocks[i].free = 1;
            merge_free_blocks();
            return;
        }
    }
}
