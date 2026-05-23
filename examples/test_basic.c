/**
 * Example program to test MemGuard
 * This program intentionally has memory issues
 */

#define MEMGUARD_ENABLED
#include "../include/memguard.h"

#include <stdio.h>
#include <string.h>

void test_basic_allocation() {
    printf("\n=== Test 1: Basic Allocation ===\n");
    
    int* numbers = (int*)malloc(10 * sizeof(int));
    for (int i = 0; i < 10; i++) {
        numbers[i] = i * 2;
    }
    
    printf("Allocated array of 10 integers\n");
    free(numbers);
    printf("Freed successfully\n");
}

void test_memory_leak() {
    printf("\n=== Test 2: Memory Leak (Intentional) ===\n");
    
    char* message = (char*)malloc(100);
    strcpy(message, "This memory will leak!");
    printf("Message: %s\n", message);
    
    /* Oops! Forgot to free() */
    printf("Forgot to free - this will be detected!\n");
}

void test_double_free() {
    printf("\n=== Test 3: Double Free (Intentional) ===\n");
    
    int* data = (int*)malloc(sizeof(int));
    *data = 42;
    printf("Data: %d\n", *data);
    
    free(data);
    printf("Freed once\n");
    
    /* This should be detected! */
    free(data); 
    
}

void test_use_after_free(){
    printf("\n=== Test 5 : Use-After-Free (Intentional) ===\n");

    int* data = (int*)malloc(sizeof(int));
    *data = 69;
    printf("Value Before Free : %d\n", *data);

    free(data);
    printf("Freed the Memory\n");

    /* This should be detected as use-after-free */
    *data = 100; // Modifying memory after free
    printf("Wrote %d to freed memory\n", *data);

    
}

void test_buffer_overflow() {
    printf("\n=== Test 4: Buffer Overflow (Intentional) ===\n");
    printf("Note: Writing just past end of buffer\n");
    
    char* buffer = (char*)malloc(10);
    strcpy(buffer, "Short");
    printf("Buffer: %s\n", buffer);
    
    /* Controlled overflow - corrupts back canary only */
    buffer[10] = 'X';
    buffer[11] = 'X';
    buffer[12] = 'X';
    buffer[13] = 'X';

    printf("Wrote past end of buffer\n");

    free(buffer);
}

int main() {
    printf("MemGuard Test Program\n");
    printf("=====================\n");
    
    mg_init();
    
    test_basic_allocation();
    test_memory_leak();
    test_double_free();
    test_use_after_free();
    test_buffer_overflow();
    
    printf("\n=== Final Report ===\n");
    mg_cleanup();
    
    return mg_has_errors() ? 1 : 0;
}
