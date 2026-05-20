#define MEMGUARD_ENABLED
#include "../include/memguard.h"
#include <stdio.h>

int main() {
    mg_init();
    
    // Test 1: Simple allocation and free
    int* x = malloc(sizeof(int));
    *x = 42;
    printf("x = %d\n", *x);
    free(x);
    
    // Test 2: Intentional leak
    char* leak = malloc(100);
    // Don't free!
    free(leak);  // Comment out to test leak detection
    
    mg_cleanup();
    mg_print_statistics();
    return 0;
}