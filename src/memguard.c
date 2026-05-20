/**
 * MemGuard Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/memguard.h"
#include "../include/memguard_internal.h"

/* Global state */
memguard_state_t g_state = {
    .head = NULL,
    .total_allocated = 0,
    .peak_memory = 0,
    .current_memory = 0,
    .allocation_count = 0,
    .free_count = 0
};

/* ANSI Color codes for terminal output */
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

/* ========================================
 * PHASE 1: Core Memory Wrapper (Week 1-3)
 * ======================================== */

void mg_init(void) {
    /* TODO: Initialize global state */
    memset(&g_state, 0, sizeof(memguard_state_t));
    printf(COLOR_GREEN "MemGuard initialized\n" COLOR_RESET);
}

void* mg_malloc(size_t size, const char* file, int line) {
    /* TODO: Implement malloc wrapper
     * 
     * Steps:
     * 1. Allocate extra space for canary bytes (before and after)
     * 2. Call real malloc()
     * 3. Set canary values
     * 4. Create allocation_t structure
     * 5. Add to linked list
     * 6. Update statistics
     * 7. Return pointer to user (after front canary)
     */
    
    printf("[DEBUG] malloc(%lu) called from %s:%d\n", (unsigned long)size, file, line);
    
    void* ptr = malloc(size);
    if (!ptr) {
        return NULL;
    }
    
    // Create a tracking structure
    allocation_t* alloc = (allocation_t*)malloc(sizeof(allocation_t));
    if (!alloc) {
        free(ptr);
        return NULL;
    }
    
    // Fill in the tracking information
    alloc->user_ptr = ptr;
    alloc->real_ptr = ptr;
    alloc->size = size;
    alloc->file = file;
    alloc->line = line;
    alloc->timestamp = time(NULL);
    alloc->is_freed = 0;
    alloc->next = NULL;
    
    // Add to the global linked list
    add_allocation(alloc);
    
    // Update statistics
    g_state.total_allocated += size;
    g_state.current_memory += size;
    g_state.allocation_count++;
    
    if (g_state.current_memory > g_state.peak_memory) {
        g_state.peak_memory = g_state.current_memory;
    }
    
    printf("[MemGuard] Allocated %lu bytes at %s : %d\n", (unsigned long)size, file, line);
    
    return ptr;
}

void mg_free(void* ptr, const char* file, int line) {
    /* TODO: Implement free wrapper
     * 
     * Steps:
     * 1. Find allocation in linked list
     * 2. Check if already freed (double-free detection)
     * 3. Check canary values (overflow detection)
     * 4. Mark as freed (for use-after-free detection)
     * 5. Remove from list
     * 6. Update statistics
     * 7. Call real free()
     */
    
    printf("[DEBUG] free(%p) called from %s:%d\n", ptr, file, line);
    
    if (ptr == NULL) {
        return;  /* Freeing NULL is valid */
    }
       
    // Step 1: Find this allocation in our list
    allocation_t* alloc = find_allocation(ptr);
    
    if (!alloc) {
        printf(COLOR_RED "[ERROR] Free of untracked pointer at %s:%d\n" COLOR_RESET, 
               file, line);
        return;
    }
    
    // Step 2: Check for double-free
    if (alloc->is_freed) {
        printf(COLOR_RED "[DOUBLE-FREE] at %s:%d\n" COLOR_RESET, file, line);
        printf("  Originally allocated at %s:%d\n", alloc->file, alloc->line);
        return;  // Don't actually free again
    }
    
    // Step 3: Mark as freed
    alloc->is_freed = 1;
    
    // Step 4: Update statistics
    g_state.current_memory -= alloc->size;
    g_state.free_count++;
    
    printf("[MemGuard] Freed %lu bytes at %s:%d\n", (unsigned long)alloc->size, file, line);
    
    // Step 5: Actually free the memory
    free(ptr);
}

void* mg_calloc(size_t num, size_t size, const char* file, int line) {
    /* TODO: Implement calloc wrapper */
    size_t total_size = num * size;
    void* ptr = mg_malloc(total_size, file, line);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void* mg_realloc(void* ptr, size_t size, const char* file, int line) {
    /* TODO: Implement realloc wrapper
     * This is tricky - you need to:
     * 1. Allocate new memory with mg_malloc
     * 2. Copy old data
     * 3. Free old memory with mg_free
     */
    
    if (ptr == NULL) {
        return mg_malloc(size, file, line);
    }
    
    /* Simple implementation - enhance later */
    return realloc(ptr, size);
}

/* ========================================
 * PHASE 2: Allocation Tracking (Week 4-6)
 * ======================================== */

allocation_t* find_allocation(void* ptr) {
    /* TODO: Search linked list for allocation
     * Return the allocation_t* if found, NULL otherwise
     */
    
    allocation_t* current = g_state.head;
    while (current != NULL) {
        if (current->user_ptr == ptr) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void add_allocation(allocation_t* alloc) {
    /* TODO: Add allocation to front of linked list */
    alloc->next = g_state.head;
    g_state.head = alloc;
}

void remove_allocation(allocation_t* alloc) {
    /* TODO: Remove allocation from linked list */
    
    if (g_state.head == alloc) {
        g_state.head = alloc->next;
        return;
    }
    
    allocation_t* current = g_state.head;
    while (current != NULL && current->next != alloc) {
        current = current->next;
    }
    
    if (current != NULL) {
        current->next = alloc->next;
    }
}

int check_canary(allocation_t* alloc) {
    /* TODO: Check if canary values are intact
     * Return 1 if OK, 0 if corrupted (buffer overflow detected)
     */
    
    /* Implementation hint:
     * - Front canary is at: alloc->real_ptr
     * - Back canary is at: alloc->real_ptr + sizeof(canary) + alloc->size
     * - Compare with MG_CANARY_VALUE
     */
    
    return 1;  /* Placeholder */
}

/* ========================================
 * PHASE 3: Reporting System (Week 7-9)
 * ======================================== */

void mg_report(void) {
    printf("\n" COLOR_BLUE "====={ MEMGUARD REPORT }=====" COLOR_RESET "\n\n");
    
    int leak_count = 0;
    size_t leaked_bytes = 0;
    
    allocation_t* current = g_state.head;
    while (current != NULL) {
        if (!current->is_freed) {
            printf(COLOR_RED "[LEAK]" COLOR_RESET " %lu bytes at %s:%d\n",
                   (unsigned long)current->size, current->file, current->line);
            leak_count++;
            leaked_bytes += current->size;
        }
        current = current->next;
    }
    
    if (leak_count == 0) {
        printf(COLOR_GREEN "- No Memory Leaks Detected !! \n" COLOR_RESET);
    } else {
        printf("\n" COLOR_RED "Total: %d leaks, %lu bytes\n" COLOR_RESET,
               leak_count, (unsigned long)leaked_bytes);
    }
    
    printf("\n");
}

void mg_report_to_file(const char* filename) {
    /* TODO: Write report to JSON file
     * 
     * Format:
     * {
     *   "leaks": [
     *     {"file": "main.c", "line": 42, "size": 1024},
     *     ...
     *   ],
     *   "statistics": {...}
     * }
     */
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to open %s\n", filename);
        return;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"leaks\": [\n");
    
    /* TODO: Write leak data */
    
    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Report written to %s\n", filename);
}

void mg_print_statistics(void) {
    /* TODO: Print memory usage statistics
     * - Peak memory usage
     * - Total allocations
     * - Total frees
     * - Current memory usage
     */
    
    printf("\n" COLOR_BLUE "====={ MEMORY STATISTICS }=====" COLOR_RESET "\n");
    printf("Total allocated:  %lu bytes\n", (unsigned long)g_state.total_allocated);
    printf("Peak memory:      %lu bytes\n", (unsigned long)g_state.peak_memory);
    printf("Current memory:   %lu bytes\n", (unsigned long)g_state.current_memory);
    printf("Allocations:      %d\n", g_state.allocation_count);
    printf("Frees:            %d\n", g_state.free_count);
    printf("\n");
}

void mg_cleanup(void) {
    /* TODO: Clean up all tracking structures */
    printf(COLOR_YELLOW "MemGuard cleanup\n" COLOR_RESET);
    mg_report();
}
