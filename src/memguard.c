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

    if(size == 0){
        return NULL;          // allocation 0 byte returns NULL (or a unique pointer that can be freed, but we choose NULL for simplicity)
    }

    // calculate total size needed (front canary + user size + back canary)
    size_t total_size = sizeof(unsigned int)       // front canary
                        + size                     // user allocated size (user's data)
                        + sizeof(unsigned int);    // back canary
    

    void* real_ptr = malloc(total_size);

    if (!real_ptr) {
        return NULL;       // allocation failed 
    }

    // set front canary (before user data)
    unsigned int* front_canary = (unsigned int*)real_ptr;
    *front_canary = MG_CANARY_VALUE;

    //Calculate user pointer (after front canary)
    void* user_ptr = (void*)(front_canary + 1); // move past front canary


    // set back canary (after user data)
    unsigned int* back_canary = (unsigned int*)((char*)user_ptr + size);
    *back_canary = MG_CANARY_VALUE;

    
    // Create a tracking structure
    allocation_t* alloc = (allocation_t*)malloc(sizeof(allocation_t));
    if (!alloc) {
        free(real_ptr);
        return NULL;
    }
    
    // Fill in the tracking information
    alloc->user_ptr = user_ptr;                    // pointer returned to user (after front canary)
    alloc->real_ptr = real_ptr;                    // pointer to the actual malloc'd block (including canaries)
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
    
    printf("[MemGuard] Allocated %lu bytes at %s:%d\n", (unsigned long)size, file, line);
    
    return user_ptr;                   // Return pointer to user data (after front canary, not the real pointer)
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
        g_state.double_free_count++;
        return;  // Don't actually free again
    }

    // Checking Canaries for buffer overflow detection
    // unsigned int* front_canary = (unsigned int*)alloc->real_ptr;
    // unsigned int* back_canary = (unsigned int*)((char*)alloc->user_ptr + alloc->size);

    // int front_ok = (*front_canary == MG_CANARY_VALUE);
    // int back_ok = (*back_canary == MG_CANARY_VALUE);

    // if(!front_ok){
    //     printf(COLOR_RED "[BUFFER UNDERFLOW] Detected (front canary corrupted) at %s:%d\n" COLOR_RESET, file, line);
    //     printf("  Memory allocated at %s:%d\n", alloc->file, alloc->line);
    //     printf("  Front Canary Corrupted! :: [ Expected : 0x%X ] [ Found : 0x%X ]\n", MG_CANARY_VALUE, *front_canary);
    // }

    // if(!back_ok){
    //     printf(COLOR_RED "[BUFFER OVERFLOW] Detected (back canary corrupted) at %s:%d\n" COLOR_RESET, file,line);
    //     printf("  Memory allocated at %s:%d\n", alloc->file, alloc->line);
    //     printf("  Back Canary Corrupted! :: [Expected : 0x%X ] [ Found : 0x%X ]\n", MG_CANARY_VALUE, *back_canary);

    // }

    check_canary(alloc);

    memset(alloc->user_ptr, 0xDD, alloc->size); // Poison user memory to help detect use-after-free
    

    alloc->is_freed = 1; // Mark as freed for use-after-free detection
    
    // Step 5: Update statistics
    g_state.current_memory -= alloc->size;
    g_state.free_count++;
    
    printf("[MemGuard] Freed %lu bytes at %s:%d (poisoned)\n", (unsigned long)alloc->size, file, line);
    
    // Step 6: DON'T free real_ptr yet!
    // Keep poisoned memory alive until mg_cleanup()
    // So we can detect use-after-free
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
    
    if(alloc == NULL || alloc->real_ptr == NULL || alloc->user_ptr == NULL){
        return 0;  // Invalid allocation 
    }

    // get front and back canary pointers
    unsigned int* front_canary = (unsigned int*)alloc->real_ptr;
    unsigned int* back_canary = (unsigned int*)((char*)alloc->user_ptr + alloc->size);

    // check if canaries are intact

    int front_ok = (*front_canary == MG_CANARY_VALUE);
    int back_ok = (*back_canary == MG_CANARY_VALUE);

    if(front_ok == 0){
        printf(COLOR_RED "[CANARY CORRUPTED] FRONT CANARY Corrupted for allocation at %s:%d\n" COLOR_RESET, alloc->file, alloc->line);
        printf(" - [Expected : 0x%X] [Found : 0x%X]\n", MG_CANARY_VALUE, *front_canary);
        g_state.underflow_count++;
    }

    if(back_ok == 0){
        printf(COLOR_RED "[CANARY CORRUPTED] BACK CANARY Corrupted for allocation at %s:%d\n" COLOR_RESET, alloc->file, alloc->line);
        printf(" - [Expected : 0x%X] [Found : 0x%X]\n", MG_CANARY_VALUE, *back_canary);
        g_state.overflow_count++;
    }

    return front_ok && back_ok;  /* Placeholder */
}

int check_use_after_free(allocation_t* alloc) {

    if(alloc == NULL || alloc->is_freed == 0 || alloc->user_ptr == NULL){
        return 0; // Not freed or invalid allocation
    }

    // check if poisoned memory is modified
    unsigned char* mem = (unsigned char*)alloc->user_ptr;
    
    int first_modified_byte = -1;
    unsigned char found_value = 0;

    for(size_t i = 0;i<alloc->size;i++){
        if(mem[i]!=0xDD){
            
            if(first_modified_byte == -1){
                first_modified_byte = (int)i;
                found_value = mem[i];
            }
        }
    }

    if(first_modified_byte != -1){
        printf(COLOR_RED "[USE-AFTER-FREE] Detected!\n" COLOR_RESET);
        printf(" - Allocation at %s:%d\n", alloc->file, alloc->line);
        printf(" - Size : %lu bytes\n", (unsigned long)alloc->size);
        printf(" - First Modified Byte Offset: %d\n", first_modified_byte);
        printf(" - [Expected : 0xDD] [Found : 0x%02X]\n", found_value);
        g_state.use_after_free_count++;

        return 1; // Use-after-free detected
    }

    return 0; // No modification detected
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
            // This allocation was never freed - report as a leak
            printf(COLOR_RED "[LEAK]" COLOR_RESET " %lu bytes at %s:%d\n",
                   (unsigned long)current->size, current->file, current->line);
            leak_count++;
            leaked_bytes += current->size;
        }
        else{
            // Check for use-after-free on freed allocations
            check_use_after_free(current);
        }
        current = current->next;
    }

    g_state.leak_count = leak_count;
    printf("\n");
    if (g_state.leak_count == 0 && g_state.overflow_count == 0 && g_state.underflow_count == 0 && g_state.double_free_count == 0 && g_state.use_after_free_count == 0) {
        printf(COLOR_GREEN "- No Memory Leaks Detected !! \n" COLOR_RESET);
    } else {
        if(g_state.leak_count > 0){
            printf(COLOR_RED "- Total Leaks: %d, Total Leaked Bytes: %lu\n" COLOR_RESET, g_state.leak_count, (unsigned long)leaked_bytes);
        }
        if(g_state.overflow_count > 0){
            printf(COLOR_RED "- Total Buffer Overflows Detected: %d\n" COLOR_RESET, g_state.overflow_count);
        }
        if(g_state.underflow_count > 0){
            printf(COLOR_RED "- Total Buffer Underflows Detected: %d\n" COLOR_RESET, g_state.underflow_count);
        }
        if(g_state.double_free_count > 0){
            printf(COLOR_RED "- Total Double Frees Detected: %d\n" COLOR_RESET, g_state.double_free_count);
        }
        if(g_state.use_after_free_count > 0){
            printf(COLOR_RED "- Total Use-After-Free Errors Detected: %d\n" COLOR_RESET, g_state.use_after_free_count);
        }
    }

    mg_print_statistics();
    
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

void mg_report_to_json(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to open %s\n", filename);
        return;
    }
    
    fprintf(fp, "{\n");
    
    /* Summary section */
    fprintf(fp, "  \"summary\": {\n");
    fprintf(fp, "    \"leaks\": %d,\n",            g_state.leak_count);
    fprintf(fp, "    \"overflows\": %d,\n",         g_state.overflow_count);
    fprintf(fp, "    \"underflows\": %d,\n",        g_state.underflow_count);
    fprintf(fp, "    \"double_frees\": %d,\n",      g_state.double_free_count);
    fprintf(fp, "    \"use_after_free\": %d,\n",    g_state.use_after_free_count);
    fprintf(fp, "    \"total_errors\": %d\n",
            g_state.leak_count +
            g_state.overflow_count +
            g_state.underflow_count +
            g_state.double_free_count +
            g_state.use_after_free_count);
    fprintf(fp, "  },\n");
    
    /* Statistics section */
    fprintf(fp, "  \"statistics\": {\n");
    fprintf(fp, "    \"total_allocated_bytes\": %lu,\n",
            (unsigned long)g_state.total_allocated);
    fprintf(fp, "    \"peak_memory_bytes\": %lu,\n",
            (unsigned long)g_state.peak_memory);
    fprintf(fp, "    \"total_allocations\": %d,\n",
            g_state.allocation_count);
    fprintf(fp, "    \"total_frees\": %d\n",
            g_state.free_count);
    fprintf(fp, "  },\n");
    
/* Issues section */
    fprintf(fp, "  \"issues\": [\n");
    
    int first = 1;
    allocation_t* current = g_state.head;
    while (current != NULL) {
        /* LEAKS */
        if (!current->is_freed) {
            if (!first) fprintf(fp, ",\n");
            fprintf(fp, "    {\n");
            fprintf(fp, "      \"type\": \"LEAK\",\n");
            fprintf(fp, "      \"file\": \"%s\",\n", current->file);
            fprintf(fp, "      \"line\": %d,\n",     current->line);
            fprintf(fp, "      \"size\": %lu\n",     (unsigned long)current->size);
            fprintf(fp, "    }");
            first = 0;
        }
        current = current->next;
    }
    
    /* Add other error types to issues */
    if (g_state.overflow_count > 0) {
        if (!first) fprintf(fp, ",\n");
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"type\": \"BUFFER_OVERFLOW\",\n");
        fprintf(fp, "      \"count\": %d\n", g_state.overflow_count);
        fprintf(fp, "    }");
        first = 0;
    }
    
    if (g_state.double_free_count > 0) {
        if (!first) fprintf(fp, ",\n");
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"type\": \"DOUBLE_FREE\",\n");
        fprintf(fp, "      \"count\": %d\n", g_state.double_free_count);
        fprintf(fp, "    }");
        first = 0;
    }
    
    if (g_state.use_after_free_count > 0) {
        if (!first) fprintf(fp, ",\n");
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"type\": \"USE_AFTER_FREE\",\n");
        fprintf(fp, "      \"count\": %d\n", g_state.use_after_free_count);
        fprintf(fp, "    }");
        first = 0;
    }
    
    fprintf(fp, "\n  ]\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("[MemGuard] JSON report written to %s\n", filename);
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

    mg_report_to_json("memguard_report.json");

    // then free all allocations and tracking structures
    allocation_t* current = g_state.head;
    while(current!=NULL){
        allocation_t* next = current->next;

        if(current->real_ptr){
            free(current->real_ptr); // Free the actual memory (including canaries)
        }

        if(current->freed_snapshot){
            free(current->freed_snapshot); // Free the snapshot memory
        }

        free(current); // Free the tracking structure
        current = next;

    }

    g_state.head = NULL; // Reset head to NULL after cleanup

}

int mg_has_errors(void){
        return (g_state.leak_count > 0||
                g_state.overflow_count > 0 ||
                g_state.underflow_count > 0 ||
                g_state.double_free_count > 0 ||
                g_state.use_after_free_count > 0);
    }
