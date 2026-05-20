/**
 * Internal data structures for MemGuard
 * This file is not exposed to users
 */

#ifndef MEMGUARD_INTERNAL_H
#define MEMGUARD_INTERNAL_H

#include <stddef.h>
#include <time.h>

/* Allocation tracking structure */
typedef struct allocation {
    void* user_ptr;           /* Pointer given to user */
    void* real_ptr;           /* Actual malloc pointer (includes canary) */
    size_t size;              /* Size requested by user */
    const char* file;         /* Source file where allocated */
    int line;                 /* Line number */
    time_t timestamp;         /* When allocated */
    int is_freed;             /* Flag to detect use-after-free */
    struct allocation* next;  /* Next allocation in linked list */
} allocation_t;

/* Global state */
typedef struct {
    allocation_t* head;       /* Linked list of allocations */
    size_t total_allocated;   /* Total bytes allocated */
    size_t peak_memory;       /* Peak memory usage */
    size_t current_memory;    /* Current memory in use */
    int allocation_count;     /* Total number of allocations */
    int free_count;           /* Total number of frees */
} memguard_state_t;

/* Global state (defined in memguard.c) */
extern memguard_state_t g_state;

/* Internal helper functions */
allocation_t* find_allocation(void* ptr);
void add_allocation(allocation_t* alloc);
void remove_allocation(allocation_t* alloc);
int check_canary(allocation_t* alloc);

#endif /* MEMGUARD_INTERNAL_H */
