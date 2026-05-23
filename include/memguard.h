/**
 * MemGuard - Smart Memory Debugger for C
 * 
 * A lightweight memory debugging tool that detects:
 * - Memory leaks
 * - Buffer overflows
 * - Double frees
 * - Use-after-free errors
 */

#ifndef MEMGUARD_H
#define MEMGUARD_H

#include <stddef.h>

/* Configuration */
#define MG_ENABLE_OVERFLOW_DETECTION 1
#define MG_ENABLE_STATISTICS 1
#define MG_CANARY_VALUE 0xDEADBEEF

/* Core API */
void* mg_malloc(size_t size, const char* file, int line);
void* mg_calloc(size_t num, size_t size, const char* file, int line);
void* mg_realloc(void* ptr, size_t size, const char* file, int line);
void mg_free(void* ptr, const char* file, int line);

/* Reporting */
void mg_report(void);
void mg_report_to_file(const char* filename);
void mg_print_statistics(void);

/* Initialization and cleanup */
void mg_init(void);
void mg_cleanup(void);
int mg_has_errors(void);

/* Macros to replace standard functions */
#ifdef MEMGUARD_ENABLED
    #define malloc(s)       mg_malloc(s, __FILE__, __LINE__)
    #define calloc(n, s)    mg_calloc(n, s, __FILE__, __LINE__)
    #define realloc(p, s)   mg_realloc(p, s, __FILE__, __LINE__)
    #define free(p)         mg_free(p, __FILE__, __LINE__)
#endif

#endif /* MEMGUARD_H */
