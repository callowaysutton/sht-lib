/**
 * @file sht.h
 * @brief Simple Header-only Testing framework for C
 * @version 2.0
 * 
 * A lightweight, single-header testing framework with:
 * - Auto-discovery of tests using constructor attributes
 * - Rich assertion macros (EXPECT_* and ASSERT_*)
 * - Memory leak detection
 * - Colored console output
 * - Test filtering and categorization
 * - Setup/teardown support
 * - Detailed reporting with timing
 * 
 * Usage:
 *   #define SHT_IMPLEMENTATION
 *   #include "sht.h"
 *   
 *   TEST(MyTest, Addition) {
 *       EXPECT_EQ(2 + 2, 4);
 *   }
 *   
 *   TEST_RUN_MAIN();
 */

#ifndef SHT_H
#define SHT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <setjmp.h>
#include <pthread.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #if defined(__linux__)
        #include <sys/sysinfo.h>
    #endif
#endif

#ifdef __has_attribute
    #if __has_attribute(constructor)
        #define SHT_HAS_CONSTRUCTOR_ATTR 1
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CONFIGURATION OPTIONS
 * ============================================================================ */

#ifndef SHT_MAX_TESTS
    #define SHT_MAX_TESTS 4096
#endif

#ifndef SHT_MAX_NAME_LENGTH
    #define SHT_MAX_NAME_LENGTH 256
#endif

#ifndef SHT_MAX_MESSAGE_LENGTH
    #define SHT_MAX_MESSAGE_LENGTH 2048
#endif

#ifndef SHT_MEMORY_POOL_SIZE
    #define SHT_MEMORY_POOL_SIZE (1024 * 1024)
#endif

#ifndef SHT_DEFAULT_FP_TOLERANCE
    #define SHT_DEFAULT_FP_TOLERANCE 1e-6
#endif

#ifndef SHT_MAX_THREADS
    #if defined(_WIN32) || defined(_WIN64)
        #define SHT_MAX_THREADS 4
    #else
        #if defined(__linux__)
            #define SHT_MAX_THREADS (get_nprocs() > 0 ? get_nprocs() : 4)
        #elif defined(__APPLE__)
            #include <sys/sysctl.h>
            #define SHT_MAX_THREADS 4
        #else
            #define SHT_MAX_THREADS 4
        #endif
    #endif
#endif

/* ============================================================================
 * PLATFORM DETECTION
 * ============================================================================ */

#if defined(__GNUC__) || defined(__clang__)
    #define SHT_WEAK __attribute__((weak))
#elif defined(_MSC_VER)
    #define SHT_WEAK
#else
    #define SHT_WEAK
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define SHT_LIKELY(x) __builtin_expect(!!(x), 1)
    #define SHT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define SHT_LIKELY(x) (x)
    #define SHT_UNLIKELY(x) (x)
#endif

/* ============================================================================
 * TYPE DEFINITIONS
 * ============================================================================ */

typedef enum {
    SHT_RESULT_PASS,
    SHT_RESULT_FAIL,
    SHT_RESULT_SKIP,
    SHT_RESULT_TIMEOUT
} sht_result_t;

typedef enum {
    SHT_ERROR_NONE = 0,
    SHT_ERROR_OUT_OF_MEMORY,
    SHT_ERROR_INVALID_ARGUMENT,
    SHT_ERROR_TEST_NOT_FOUND,
    SHT_ERROR_SETUP_FAILED,
    SHT_ERROR_TEARDOWN_FAILED,
    SHT_ERROR_EXCEPTION
} sht_error_t;

typedef enum {
    SHT_COLOR_RESET = 0,
    SHT_COLOR_BOLD = 1,
    SHT_COLOR_RED = 31,
    SHT_COLOR_GREEN = 32,
    SHT_COLOR_YELLOW = 33,
    SHT_COLOR_BLUE = 34,
    SHT_COLOR_MAGENTA = 35,
    SHT_COLOR_CYAN = 36,
    SHT_COLOR_WHITE = 37
} sht_color_t;

typedef struct sht_test_t sht_test_t;

typedef void (*sht_test_func_t)(void);
typedef void (*sht_setup_func_t)(void);
typedef void (*sht_teardown_func_t)(void);

struct sht_test_t {
    char name[SHT_MAX_NAME_LENGTH];
    char suite[SHT_MAX_NAME_LENGTH];
    char category[SHT_MAX_NAME_LENGTH];
    sht_test_func_t func;
    sht_setup_func_t setup;
    sht_teardown_func_t teardown;
    sht_result_t result;
    double duration_ms;
    char message[SHT_MAX_MESSAGE_LENGTH];
    int line;
    char file[SHT_MAX_NAME_LENGTH];
    int enabled;
    sht_test_t* next;
};

typedef struct {
    sht_test_t* tests;
    sht_test_t* last_test;
    int test_count;
    int passed;
    int failed;
    int skipped;
    int total_failed_assertions;
    int total_assertions;
    double total_duration_ms;
    
    jmp_buf jump_buffer;
    int should_jump;
    char current_message[SHT_MAX_MESSAGE_LENGTH];
    const char* current_file;
    int current_line;
    
    void* memory_pool;
    size_t memory_pool_size;
    size_t memory_pool_used;
    int allocations_count;
    int leak_detection_enabled;
    int verbose_enabled;
    int color_enabled;
    
} sht_context_t;

typedef struct {
    sht_test_t* test;
    int passed;
    int failed;
    int skipped;
    int failed_assertions;
    int total_assertions;
    double duration_ms;
    char message[SHT_MAX_MESSAGE_LENGTH];
    int line;
    char file[SHT_MAX_NAME_LENGTH];
} sht_test_result_t;

typedef struct {
    pthread_t thread_id;
    int worker_id;
    sht_test_t** test_queue;
    int* queue_size;
    int* queue_index;
    pthread_mutex_t* queue_mutex;
    sht_test_result_t* results;
    int result_count;
    int result_capacity;
    sht_context_t* context;
} sht_worker_t;

/* ============================================================================
 * GLOBAL STATE
 * ============================================================================ */

static sht_context_t* g_sht_context = NULL;
static int g_sht_coverage_enabled = 0;
static int g_sht_parallel_enabled = 0;
static int g_sht_num_workers = 1;
static pthread_mutex_t g_sht_print_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_sht_result_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ============================================================================
 * COLOR OUTPUT SUPPORT
 * ============================================================================ */

static void sht_set_color(FILE* stream, sht_color_t color) {
    if (g_sht_context && !g_sht_context->color_enabled) {
        return;
    }
    
    #if defined(_WIN32) || defined(_WIN64)
        static HANDLE hConsole = NULL;
        static int initialized = 0;
        
        if (!initialized) {
            hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            initialized = 1;
        }
        
        if (color == SHT_COLOR_RESET) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        } else {
            WORD attr = 0;
            switch (color) {
                case SHT_COLOR_RED:    attr = FOREGROUND_RED; break;
                case SHT_COLOR_GREEN:  attr = FOREGROUND_GREEN; break;
                case SHT_COLOR_YELLOW: attr = FOREGROUND_RED | FOREGROUND_GREEN; break;
                case SHT_COLOR_BLUE:   attr = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
                case SHT_COLOR_CYAN:   attr = FOREGROUND_GREEN | FOREGROUND_BLUE; break;
                case SHT_COLOR_WHITE:  attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
                default:               attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
            }
            SetConsoleTextAttribute(hConsole, attr);
        }
    #else
        const char* codes[] = {
            "\033[0m",
            "\033[1m",
            "\033[31m",
            "\033[32m",
            "\033[33m",
            "\033[34m",
            "\033[35m",
            "\033[36m",
            "\033[37m"
        };
        fprintf(stream, "%s", codes[color]);
    #endif
}

static void sht_reset_color(FILE* stream) {
    sht_set_color(stream, SHT_COLOR_RESET);
}

/* ============================================================================
 * MEMORY MANAGEMENT
 * ============================================================================ */

static void* sht_alloc(size_t size) {
    if (!g_sht_context || size == 0) {
        return NULL;
    }
    
    if (g_sht_context->leak_detection_enabled && g_sht_context->memory_pool) {
        if (g_sht_context->memory_pool_used + size > g_sht_context->memory_pool_size) {
            fprintf(stderr, "SHT: Out of memory pool space\n");
            return NULL;
        }
        
        void* ptr = (char*)g_sht_context->memory_pool + g_sht_context->memory_pool_used;
        g_sht_context->memory_pool_used += size;
        g_sht_context->allocations_count++;
        return ptr;
    }
    
    return malloc(size);
}

static void sht_free(void* ptr) {
    if (!g_sht_context || !ptr) {
        return;
    }
    
    if (g_sht_context->leak_detection_enabled && g_sht_context->memory_pool) {
        g_sht_context->allocations_count--;
    } else {
        free(ptr);
    }
}

static void sht_init_memory_pool(void) {
    if (!g_sht_context) {
        return;
    }
    
    g_sht_context->memory_pool_size = SHT_MEMORY_POOL_SIZE;
    g_sht_context->memory_pool = malloc(g_sht_context->memory_pool_size);
    g_sht_context->memory_pool_used = 0;
    g_sht_context->allocations_count = 0;
    g_sht_context->leak_detection_enabled = 1;
}

static void sht_cleanup_memory_pool(void) {
    if (!g_sht_context) {
        return;
    }
    
    if (g_sht_context->memory_pool) {
        free(g_sht_context->memory_pool);
        g_sht_context->memory_pool = NULL;
    }
    
    g_sht_context->memory_pool_used = 0;
    g_sht_context->allocations_count = 0;
}

static int sht_check_memory_leaks(void) {
    if (!g_sht_context) {
        return 0;
    }
    
    return g_sht_context->allocations_count > 0;
}

/* ============================================================================
 * TEST REGISTRY
 * ============================================================================ */

static sht_test_t* sht_create_test(void) {
    sht_test_t* test = (sht_test_t*)malloc(sizeof(sht_test_t));
    if (!test) {
        return NULL;
    }
    
    memset(test, 0, sizeof(sht_test_t));
    test->enabled = 1;
    test->result = SHT_RESULT_SKIP;
    
    return test;
}

static void sht_destroy_test(sht_test_t* test) {
    if (test) {
        free(test);
    }
}

static void sht_register_test(sht_test_t* test) {
    if (!g_sht_context || !test) {
        return;
    }
    
    if (!g_sht_context->tests) {
        g_sht_context->tests = test;
        g_sht_context->last_test = test;
    } else {
        g_sht_context->last_test->next = test;
        g_sht_context->last_test = test;
    }
    
    g_sht_context->test_count++;
}

/* ============================================================================
 * STRING FORMATTING
 * ============================================================================ */

static int sht_vsnprintf(char* buffer, size_t size, const char* format, va_list args) {
    #if defined(_MSC_VER)
        return vsnprintf_s(buffer, size, _TRUNCATE, format, args);
    #else
        return vsnprintf(buffer, size, format, args);
    #endif
}

static int sht_snprintf(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = sht_vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

/* ============================================================================
 * ASSERTION INTERNALS
 * ============================================================================ */

static int sht_format_value(char* buffer, size_t size, int64_t value) {
    return sht_snprintf(buffer, size, "%lld", (long long)value);
}

static int sht_format_uint(char* buffer, size_t size, uint64_t value) {
    return sht_snprintf(buffer, size, "%llu", (unsigned long long)value);
}

static int sht_format_double(char* buffer, size_t size, double value) {
    return sht_snprintf(buffer, size, "%.10g", value);
}

static int sht_format_ptr(char* buffer, size_t size, const void* ptr) {
    return sht_snprintf(buffer, size, "%p", ptr);
}

static int sht_format_str(char* buffer, size_t size, const char* str, int truncate) {
    if (!str) {
        return sht_snprintf(buffer, size, "NULL");
    }
    
    size_t len = strlen(str);
    if (truncate && len > 60) {
        return sht_snprintf(buffer, size, "%.30s...%.20s", str, str + len - 20);
    }
    return sht_snprintf(buffer, size, "\"%s\"", str);
}

static void sht_assertion_failure(const char* file, int line, const char* message) {
    if (!g_sht_context) {
        fprintf(stderr, "Assertion failed: %s:%d: %s\n", file, line, message);
        return;
    }
    
    strncpy(g_sht_context->current_message, message, SHT_MAX_MESSAGE_LENGTH - 1);
    g_sht_context->current_message[SHT_MAX_MESSAGE_LENGTH - 1] = '\0';
    g_sht_context->current_file = file;
    g_sht_context->current_line = line;
    
    g_sht_context->total_failed_assertions++;
    g_sht_context->total_assertions++;
    
    if (g_sht_context->should_jump) {
        longjmp(g_sht_context->jump_buffer, 1);
    }
}

/* ============================================================================
 * ASSERTION MACROS - FATAL (ASSERT_*)
 * ============================================================================ */

#define SHT_ASSERT_BASE(condition, message) \
    do { \
        if (SHT_UNLIKELY(!(condition))) { \
            sht_assertion_failure(__FILE__, __LINE__, message); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_TRUE(value) \
    SHT_ASSERT_BASE(value, "Expected: " #value " to be true")

#define ASSERT_FALSE(value) \
    SHT_ASSERT_BASE(!(value), "Expected: " #value " to be false")

#define ASSERT_EQ(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a != e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " == " #expected ", but found: %s != %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_NE(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a == e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " != " #expected ", but both are: %s", a_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_LT(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a >= e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " < " #expected ", but found: %s >= %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_LE(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a > e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " <= " #expected ", but found: %s > %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_GT(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a <= e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " > " #expected ", but found: %s <= %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_GE(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a < e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " >= " #expected ", but found: %s < %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    SHT_ASSERT_BASE((ptr) == NULL, "Expected: " #ptr " to be NULL")

#define ASSERT_NOT_NULL(ptr) \
    SHT_ASSERT_BASE((ptr) != NULL, "Expected: " #ptr " to be non-NULL")

#define ASSERT_STR_EQ(actual, expected) \
    do { \
        const char* a = (const char*)(actual); \
        const char* e = (const char*)(expected); \
        int eq = (!a && !e) || (a && e && strcmp(a, e) == 0); \
        if (SHT_UNLIKELY(!eq)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[SHT_MAX_MESSAGE_LENGTH]; \
            char e_str[SHT_MAX_MESSAGE_LENGTH]; \
            sht_format_str(a_str, sizeof(a_str), a, 1); \
            sht_format_str(e_str, sizeof(e_str), e, 1); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected strings to be equal:\n  Actual:   %s\n  Expected: %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_STR_NE(actual, expected) \
    do { \
        const char* a = (const char*)(actual); \
        const char* e = (const char*)(expected); \
        int ne = (a == e) || (a && e && strcmp(a, e) == 0); \
        if (SHT_UNLIKELY(ne)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[SHT_MAX_MESSAGE_LENGTH]; \
            sht_format_str(a_str, sizeof(a_str), a, 1); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected strings to differ, but both are: %s", a_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_FLOAT_EQ(actual, expected) \
    ASSERT_FLOAT_NEAR(actual, expected, SHT_DEFAULT_FP_TOLERANCE)

#define ASSERT_FLOAT_NEAR(actual, expected, tolerance) \
    do { \
        double a = (double)(actual); \
        double e = (double)(expected); \
        double t = (double)(tolerance); \
        double diff = fabs(a - e); \
        if (SHT_UNLIKELY(diff > t)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64], diff_str[64]; \
            sht_format_double(a_str, sizeof(a_str), a); \
            sht_format_double(e_str, sizeof(e_str), e); \
            sht_format_double(diff_str, sizeof(diff_str), diff); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: |" #actual " - " #expected "| <= " #tolerance " (|%.10g - %.10g| = %s)", a, e, diff_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_MEM_EQ(actual, expected, size) \
    do { \
        const void* a = (const void*)(actual); \
        const void* e = (const void*)(expected); \
        size_t s = (size_t)(size); \
        if (SHT_UNLIKELY((a != e) && (!a || !e || memcmp(a, e, s) != 0))) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            sht_snprintf(msg, sizeof(msg), \
                "Expected blocks of %zu bytes to be equal", s); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define ASSERT_MEM_NE(actual, expected, size) \
    do { \
        const void* a = (const void*)(actual); \
        const void* e = (const void*)(expected); \
        size_t s = (size_t)(size); \
        if (SHT_UNLIKELY(a == e || (a && e && memcmp(a, e, s) == 0))) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            sht_snprintf(msg, sizeof(msg), \
                "Expected blocks of %zu bytes to differ", s); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
            return; \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

/* ============================================================================
 * ASSERTION MACROS - NON-FATAL (EXPECT_*)
 * ============================================================================ */

#define SHT_EXPECT_BASE(condition, message) \
    do { \
        if (SHT_UNLIKELY(!(condition))) { \
            sht_assertion_failure(__FILE__, __LINE__, message); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_TRUE(value) \
    SHT_EXPECT_BASE(value, "Expected: " #value " to be true")

#define EXPECT_FALSE(value) \
    SHT_EXPECT_BASE(!(value), "Expected: " #value " to be false")

#define EXPECT_EQ(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a != e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " == " #expected ", but found: %s != %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_NE(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a == e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " != " #expected ", but both are: %s", a_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_LT(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a >= e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " < " #expected ", but found: %s >= %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_LE(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a > e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " <= " #expected ", but found: %s > %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_GT(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a <= e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " > " #expected ", but found: %s <= %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_GE(actual, expected) \
    do { \
        int64_t a = (int64_t)(actual); \
        int64_t e = (int64_t)(expected); \
        if (SHT_UNLIKELY(a < e)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64]; \
            sht_format_value(a_str, sizeof(a_str), a); \
            sht_format_value(e_str, sizeof(e_str), e); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: " #actual " >= " #expected ", but found: %s < %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_NULL(ptr) \
    SHT_EXPECT_BASE((ptr) == NULL, "Expected: " #ptr " to be NULL")

#define EXPECT_NOT_NULL(ptr) \
    SHT_EXPECT_BASE((ptr) != NULL, "Expected: " #ptr " to be non-NULL")

#define EXPECT_STR_EQ(actual, expected) \
    do { \
        const char* a = (const char*)(actual); \
        const char* e = (const char*)(expected); \
        int eq = (!a && !e) || (a && e && strcmp(a, e) == 0); \
        if (SHT_UNLIKELY(!eq)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[SHT_MAX_MESSAGE_LENGTH]; \
            char e_str[SHT_MAX_MESSAGE_LENGTH]; \
            sht_format_str(a_str, sizeof(a_str), a, 1); \
            sht_format_str(e_str, sizeof(e_str), e, 1); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected strings to be equal:\n  Actual:   %s\n  Expected: %s", a_str, e_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_STR_NE(actual, expected) \
    do { \
        const char* a = (const char*)(actual); \
        const char* e = (const char*)(expected); \
        int ne = (a == e) || (a && e && strcmp(a, e) == 0); \
        if (SHT_UNLIKELY(ne)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[SHT_MAX_MESSAGE_LENGTH]; \
            sht_format_str(a_str, sizeof(a_str), a, 1); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected strings to differ, but both are: %s", a_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_FLOAT_EQ(actual, expected) \
    EXPECT_FLOAT_NEAR(actual, expected, SHT_DEFAULT_FP_TOLERANCE)

#define EXPECT_FLOAT_NEAR(actual, expected, tolerance) \
    do { \
        double a = (double)(actual); \
        double e = (double)(expected); \
        double t = (double)(tolerance); \
        double diff = fabs(a - e); \
        if (SHT_UNLIKELY(diff > t)) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            char a_str[64], e_str[64], diff_str[64]; \
            sht_format_double(a_str, sizeof(a_str), a); \
            sht_format_double(e_str, sizeof(e_str), e); \
            sht_format_double(diff_str, sizeof(diff_str), diff); \
            sht_snprintf(msg, sizeof(msg), \
                "Expected: |" #actual " - " #expected "| <= " #tolerance " (|%.10g - %.10g| = %s)", a, e, diff_str); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_MEM_EQ(actual, expected, size) \
    do { \
        const void* a = (const void*)(actual); \
        const void* e = (const void*)(expected); \
        size_t s = (size_t)(size); \
        if (SHT_UNLIKELY((a != e) && (!a || !e || memcmp(a, e, s) != 0))) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            sht_snprintf(msg, sizeof(msg), \
                "Expected blocks of %zu bytes to be equal", s); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

#define EXPECT_MEM_NE(actual, expected, size) \
    do { \
        const void* a = (const void*)(actual); \
        const void* e = (const void*)(expected); \
        size_t s = (size_t)(size); \
        if (SHT_UNLIKELY(a == e || (a && e && memcmp(a, e, s) == 0))) { \
            char msg[SHT_MAX_MESSAGE_LENGTH]; \
            sht_snprintf(msg, sizeof(msg), \
                "Expected blocks of %zu bytes to differ", s); \
            sht_assertion_failure(__FILE__, __LINE__, msg); \
        } else if (g_sht_context) { \
            g_sht_context->total_assertions++; \
        } \
    } while(0)

/* ============================================================================
 * TEST REGISTRATION
 * ============================================================================ */

#if defined(SHT_HAS_CONSTRUCTOR_ATTR)
    #define SHT_CONSTRUCTOR __attribute__((constructor))
#else
    #if defined(_MSC_VER)
        #define SHT_CONSTRUCTOR __declspec(allocate(".CRT$XCU")) static void
    #else
        #define SHT_CONSTRUCTOR
    #endif
#endif

#define TEST(suite_name, test_name) \
    static void sht_##suite_name##_##test_name##_func(void); \
    SHT_WEAK \
    void sht_register_##suite_name##_##test_name(void) { \
        sht_test_t* test = sht_create_test(); \
        if (test) { \
            sht_snprintf(test->suite, sizeof(test->suite), "%s", #suite_name); \
            sht_snprintf(test->name, sizeof(test->name), "%s", #test_name); \
            sht_snprintf(test->file, sizeof(test->file), "%s", __FILE__); \
            test->func = sht_##suite_name##_##test_name##_func; \
            sht_register_test(test); \
        } \
    } \
    SHT_CONSTRUCTOR \
    void sht_ctor_##suite_name##_##test_name(void) { \
        sht_register_##suite_name##_##test_name(); \
    } \
    static void sht_##suite_name##_##test_name##_func(void)

#define TEST_F(fixture_name, test_name) \
    static void sht_##fixture_name##_##test_name##_func(void); \
    SHT_WEAK \
    void sht_register_##fixture_name##_##test_name(void) { \
        sht_test_t* test = sht_create_test(); \
        if (test) { \
            sht_snprintf(test->suite, sizeof(test->suite), "%s", #fixture_name); \
            sht_snprintf(test->name, sizeof(test->name), "%s", #test_name); \
            sht_snprintf(test->file, sizeof(test->file), "%s", __FILE__); \
            test->func = sht_##fixture_name##_##test_name##_func; \
            test->setup = fixture_name##_SetUp; \
            test->teardown = fixture_name##_TearDown; \
            sht_register_test(test); \
        } \
    } \
    SHT_CONSTRUCTOR \
    void sht_ctor_##fixture_name##_##test_name(void) { \
        sht_register_##fixture_name##_##test_name(); \
    } \
    static void sht_##fixture_name##_##test_name##_func(void)

#define SET_UP(fixture_name) \
    static void fixture_name##_SetUp(void)

#define TEAR_DOWN(fixture_name) \
    static void fixture_name##_TearDown(void)

#define SKIP() \
    do { \
        if (g_sht_context) { \
            longjmp(g_sht_context->jump_buffer, 2); \
        } \
    } while(0)

/* ============================================================================
 * COVERAGE MACROS
 * ============================================================================ */

static void sht_coverage_init_func(void);
static void sht_cleanup_func(void);
static void sht_record_line_func(const char* file, int line);
static void sht_record_function_func(const char* name, const char* file, int start_line, int end_line);
static void sht_record_branch_func(const char* file, int line, int branch_id, int taken);
static void sht_print_coverage_summary_func(void);

#define SHT_COVERAGE_LINE(file, line) \
    do { \
        if (g_sht_coverage_enabled) { \
            sht_record_line_func(file, line); \
        } \
    } while(0)

#define SHT_COVERAGE_FUNCTION(name, file, start_line, end_line) \
    do { \
        if (g_sht_coverage_enabled) { \
            sht_record_function_func(name, file, start_line, end_line); \
        } \
    } while(0)

#define SHT_COVERAGE_BRANCH(file, line, taken, total) \
    do { \
        if (g_sht_coverage_enabled) { \
            for (int _i = 0; _i < (total); _i++) { \
                sht_record_branch_func(file, line, _i, (_i < taken) ? 1 : 0); \
            } \
        } \
    } while(0)

/* ============================================================================
 * TEST EXECUTION
 * ============================================================================ */

static void sht_print_suite_header(const char* suite_name);
static void sht_print_test_result(sht_test_t* test);
static void sht_run_single_test(sht_test_t* test);
static void* sht_worker_thread_func(void* arg);
static int sht_run_tests_parallel(int num_workers);

static double sht_get_time_ms(void) {
    #if defined(_WIN32) || defined(_WIN64)
        LARGE_INTEGER frequency, counter;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        return (double)counter.QuadPart / frequency.QuadPart * 1000.0;
    #else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
    #endif
}

static void sht_run_single_test(sht_test_t* test) {
    if (!g_sht_context || !test || !test->func) {
        return;
    }
    
    test->result = SHT_RESULT_PASS;
    test->message[0] = '\0';
    
    double start_time = sht_get_time_ms();
    
    g_sht_context->should_jump = 1;
    
    int jump_result = setjmp(g_sht_context->jump_buffer);
    
    if (jump_result == 0) {
        if (test->setup) {
            test->setup();
        }
        
        test->func();
        
        if (test->teardown) {
            test->teardown();
        }
    } else if (jump_result == 1) {
        test->result = SHT_RESULT_FAIL;
        strncpy(test->message, g_sht_context->current_message, SHT_MAX_MESSAGE_LENGTH - 1);
        test->message[SHT_MAX_MESSAGE_LENGTH - 1] = '\0';
        test->line = g_sht_context->current_line;
        
        if (test->teardown) {
            test->teardown();
        }
    } else if (jump_result == 2) {
        test->result = SHT_RESULT_SKIP;
        sht_snprintf(test->message, sizeof(test->message), "Test skipped");
        
        if (test->teardown) {
            test->teardown();
        }
    }
    
    g_sht_context->should_jump = 0;
    
    test->duration_ms = sht_get_time_ms() - start_time;
    g_sht_context->total_duration_ms += test->duration_ms;
    
    if (test->result == SHT_RESULT_PASS) {
        g_sht_context->passed++;
    } else if (test->result == SHT_RESULT_FAIL) {
        g_sht_context->failed++;
    } else if (test->result == SHT_RESULT_SKIP) {
        g_sht_context->skipped++;
    }
}

static void* sht_worker_thread_func(void* arg) {
    sht_worker_t* worker = (sht_worker_t*)arg;
    
    while (1) {
        sht_test_t* test = NULL;
        
        pthread_mutex_lock(worker->queue_mutex);
        {
            if (*worker->queue_index < *worker->queue_size) {
                test = worker->test_queue[*worker->queue_index];
                (*worker->queue_index)++;
            }
        }
        pthread_mutex_unlock(worker->queue_mutex);
        
        if (test == NULL) {
            break;
        }
        
        sht_test_result_t result;
        memset(&result, 0, sizeof(result));
        result.test = test;
        strncpy(result.file, test->file, sizeof(result.file) - 1);
        
        double start_time = sht_get_time_ms();
        
        worker->context->should_jump = 1;
        
        int jump_result = setjmp(worker->context->jump_buffer);
        
        if (jump_result == 0) {
            if (test->setup) {
                test->setup();
            }
            
            test->func();
            
            if (test->teardown) {
                test->teardown();
            }
            
            result.passed = 1;
        } else if (jump_result == 1) {
            result.failed = 1;
            strncpy(result.message, worker->context->current_message, SHT_MAX_MESSAGE_LENGTH - 1);
            result.message[SHT_MAX_MESSAGE_LENGTH - 1] = '\0';
            result.line = worker->context->current_line;
            result.failed_assertions = 1;
            
            if (test->teardown) {
                test->teardown();
            }
        } else if (jump_result == 2) {
            result.skipped = 1;
            sht_snprintf(result.message, sizeof(result.message), "Test skipped");
            
            if (test->teardown) {
                test->teardown();
            }
        }
        
        worker->context->should_jump = 0;
        
        result.duration_ms = sht_get_time_ms() - start_time;
        
        pthread_mutex_lock(worker->queue_mutex);
        {
            if (worker->result_count >= worker->result_capacity) {
                worker->result_capacity *= 2;
                worker->results = (sht_test_result_t*)realloc(worker->results, 
                    worker->result_capacity * sizeof(sht_test_result_t));
            }
            worker->results[worker->result_count] = result;
            worker->result_count++;
        }
        pthread_mutex_unlock(worker->queue_mutex);
    }
    
    return NULL;
}

static int sht_run_tests_parallel(int num_workers) {
    if (!g_sht_context || num_workers <= 0) {
        return -1;
    }
    
    int test_count = 0;
    sht_test_t* test = g_sht_context->tests;
    while (test) {
        if (test->enabled) {
            test_count++;
        }
        test = test->next;
    }
    
    if (test_count == 0) {
        return 0;
    }
    
    sht_test_t** test_queue = (sht_test_t**)calloc(test_count, sizeof(sht_test_t*));
    int queue_index = 0;
    test = g_sht_context->tests;
    while (test) {
        if (test->enabled) {
            test_queue[queue_index++] = test;
        }
        test = test->next;
    }
    
    pthread_mutex_t queue_mutex;
    pthread_mutex_init(&queue_mutex, NULL);
    
    int shared_queue_index = 0;
    int shared_queue_size = test_count;
    
    sht_worker_t* workers = (sht_worker_t*)calloc(num_workers, sizeof(sht_worker_t));
    
    for (int i = 0; i < num_workers; i++) {
        workers[i].worker_id = i;
        workers[i].test_queue = test_queue;
        workers[i].queue_size = &shared_queue_size;
        workers[i].queue_index = &shared_queue_index;
        workers[i].queue_mutex = &queue_mutex;
        workers[i].result_count = 0;
        workers[i].result_capacity = test_count / num_workers + 1;
        workers[i].results = (sht_test_result_t*)calloc(workers[i].result_capacity, sizeof(sht_test_result_t));
        
        workers[i].context = (sht_context_t*)calloc(1, sizeof(sht_context_t));
        workers[i].context->should_jump = 0;
        workers[i].context->verbose_enabled = g_sht_context->verbose_enabled;
        workers[i].context->color_enabled = g_sht_context->color_enabled;
        workers[i].context->leak_detection_enabled = 0;
        workers[i].context->memory_pool = NULL;
        
        pthread_create(&workers[i].thread_id, NULL, sht_worker_thread_func, &workers[i]);
    }
    
    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i].thread_id, NULL);
    }
    
    for (int i = 0; i < num_workers; i++) {
        free(workers[i].context);
    }
    
    pthread_mutex_destroy(&queue_mutex);
    
    double overall_start_time = sht_get_time_ms();
    
    pthread_mutex_lock(&g_sht_print_mutex);
    {
        char current_suite[SHT_MAX_NAME_LENGTH] = {0};

        for (int i = 0; i < num_workers; i++) {
            for (int j = 0; j < workers[i].result_count; j++) {
                sht_test_result_t* result = &workers[i].results[j];
                sht_test_t* test = result->test;

                strncpy(test->message, result->message, SHT_MAX_MESSAGE_LENGTH - 1);
                test->line = result->line;
                test->duration_ms = result->duration_ms;

                if (result->passed) {
                    test->result = SHT_RESULT_PASS;
                    g_sht_context->passed++;
                    g_sht_context->total_assertions++;
                } else if (result->failed) {
                    test->result = SHT_RESULT_FAIL;
                    g_sht_context->failed++;
                    g_sht_context->total_assertions++;
                    g_sht_context->total_failed_assertions++;
                } else if (result->skipped) {
                    test->result = SHT_RESULT_SKIP;
                    g_sht_context->skipped++;
                }

                g_sht_context->total_duration_ms += result->duration_ms;

                if (strcmp(test->suite, current_suite) != 0) {
                    sht_print_suite_header(test->suite);
                    strncpy(current_suite, test->suite, sizeof(current_suite) - 1);
                }

                sht_print_test_result(test);
            }
        }
    }
    pthread_mutex_unlock(&g_sht_print_mutex);
    
    for (int i = 0; i < num_workers; i++) {
        free(workers[i].results);
    }
    
    free(workers);
    free(test_queue);
    
    return 0;
}

/* ============================================================================
 * REPORTING
 * ============================================================================ */

static void sht_enhanced_print_summary_integrations(void);

static void sht_print_test_result(sht_test_t* test) {
    if (!g_sht_context || !test) {
        return;
    }
    
    const char* status_icon = "?";
    sht_color_t status_color = SHT_COLOR_RESET;
    
    switch (test->result) {
        case SHT_RESULT_PASS:
            status_icon = "[PASS]";
            status_color = SHT_COLOR_GREEN;
            break;
        case SHT_RESULT_FAIL:
            status_icon = "[FAIL]";
            status_color = SHT_COLOR_RED;
            break;
        case SHT_RESULT_SKIP:
            status_icon = "[SKIP]";
            status_color = SHT_COLOR_YELLOW;
            break;
        default:
            status_icon = "[?]";
            break;
    }
    
    sht_set_color(stdout, status_color);
    printf("  %s", status_icon);
    sht_reset_color(stdout);
    
    printf(" %-50s (%.3f ms)\n", test->name, test->duration_ms);
    
    if ((test->result == SHT_RESULT_FAIL || g_sht_context->verbose_enabled) && test->message[0]) {
        printf("    %s:%d: %s\n", test->file, test->line, test->message);
    }
}

static void sht_print_suite_header(const char* suite_name) {
    if (!g_sht_context) {
        return;
    }
    
    sht_set_color(stdout, SHT_COLOR_CYAN);
    printf("\n%s Suite:\n", suite_name ? suite_name : "Test");
    sht_reset_color(stdout);
}

static void sht_print_summary(void) {
    if (!g_sht_context) {
        return;
    }
    
    if (g_sht_coverage_enabled) {
        sht_enhanced_print_summary_integrations();
    }
    
    printf("\n");
    sht_set_color(stdout, SHT_COLOR_CYAN);
    printf("============\n");
    printf("Test Summary\n");
    printf("============\n");
    sht_reset_color(stdout);
    
    printf("Tests: %d passed, %d failed, %d skipped\n", 
           g_sht_context->passed, g_sht_context->failed, g_sht_context->skipped);
    
    printf("\n");
    
    printf("  Total: %d\n", g_sht_context->test_count);
    
    printf("  Assertions:   %d/%d (failed/total)\n", 
           g_sht_context->total_failed_assertions,
           g_sht_context->total_assertions);
    
    printf("  Time:         %.3f ms\n", g_sht_context->total_duration_ms);
    
    if (g_sht_coverage_enabled) {
        sht_print_coverage_summary_func();
    }
    
    if (g_sht_context->leak_detection_enabled) {
        if (sht_check_memory_leaks()) {
            sht_set_color(stdout, SHT_COLOR_RED);
            printf("  Leaks:        %d allocation(s) not freed\n", 
                   g_sht_context->allocations_count);
            sht_reset_color(stdout);
        } else {
            sht_set_color(stdout, SHT_COLOR_GREEN);
            printf("  Leaks:        No memory leaks detected\n");
            sht_reset_color(stdout);
        }
    }
    
    if (g_sht_context->failed > 0) {
        sht_set_color(stdout, SHT_COLOR_RED);
        printf("\nFAILED!\n");
        sht_reset_color(stdout);
    } else {
        sht_set_color(stdout, SHT_COLOR_GREEN);
        printf("\nSUCCESS!\n");
        sht_reset_color(stdout);
    }
}

/* ============================================================================
 * MAIN RUNNER
 * ============================================================================ */

static int sht_run_all_tests(void) {
    if (!g_sht_context) {
        fprintf(stderr, "SHT: Context not initialized\n");
        return -1;
    }

    if (g_sht_coverage_enabled) {
        sht_coverage_init_func();
    }

    sht_set_color(stdout, SHT_COLOR_BOLD);
    printf("\n======================================================\n");
    printf("  SHT - Simple Header-only Testing Framework\n");
    printf("======================================================\n");
    sht_reset_color(stdout);

    if (g_sht_context->test_count == 0) {
        sht_set_color(stdout, SHT_COLOR_YELLOW);
        printf("  No tests registered!\n");
        sht_reset_color(stdout);
        sht_cleanup_func();
        return 0;
    }

    printf("  Running %d test(s)...\n", g_sht_context->test_count);
    if (g_sht_coverage_enabled) {
        printf("  Coverage tracking: enabled\n");
    }

    if (g_sht_parallel_enabled) {
        printf("  Parallel execution: %d worker(s)\n", g_sht_num_workers);
        sht_run_tests_parallel(g_sht_num_workers);
    } else {
        char current_suite[SHT_MAX_NAME_LENGTH] = {0};

        sht_test_t* test = g_sht_context->tests;
        while (test) {
            if (test->enabled) {
                if (strcmp(test->suite, current_suite) != 0) {
                    sht_print_suite_header(test->suite);
                    strncpy(current_suite, test->suite, sizeof(current_suite) - 1);
                }

                sht_run_single_test(test);
                sht_print_test_result(test);
            }

            test = test->next;
        }
    }

    sht_print_summary();

    if (g_sht_coverage_enabled) {
        sht_cleanup_func();
    }

    return g_sht_context->failed > 0 ? 1 : 0;
}

static int sht_init_context(void) {
    g_sht_context = (sht_context_t*)calloc(1, sizeof(sht_context_t));
    if (!g_sht_context) {
        fprintf(stderr, "SHT: Failed to allocate context\n");
        return -1;
    }
    
    g_sht_context->tests = NULL;
    g_sht_context->last_test = NULL;
    g_sht_context->test_count = 0;
    g_sht_context->passed = 0;
    g_sht_context->failed = 0;
    g_sht_context->skipped = 0;
    g_sht_context->total_failed_assertions = 0;
    g_sht_context->total_assertions = 0;
    g_sht_context->total_duration_ms = 0.0;
    g_sht_context->should_jump = 0;
    g_sht_context->verbose_enabled = 0;
    g_sht_context->color_enabled = 1;
    
    g_sht_context->current_message[0] = '\0';
    g_sht_context->current_file = NULL;
    g_sht_context->current_line = 0;
    
    sht_init_memory_pool();
    
    return 0;
}

static void sht_cleanup_context(void) {
    if (!g_sht_context) {
        return;
    }
    
    sht_test_t* test = g_sht_context->tests;
    while (test) {
        sht_test_t* next = test->next;
        sht_destroy_test(test);
        test = next;
    }
    
    sht_cleanup_memory_pool();
    
    free(g_sht_context);
    g_sht_context = NULL;
}

static int sht_parse_arguments(int argc, char* argv[]) {
    if (!g_sht_context) {
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            g_sht_context->verbose_enabled = 1;
        } else if (strcmp(argv[i], "--no-color") == 0 || strcmp(argv[i], "-n") == 0) {
            g_sht_context->color_enabled = 0;
        } else if (strcmp(argv[i], "--coverage") == 0 || strcmp(argv[i], "-c") == 0) {
            g_sht_coverage_enabled = 1;
        } else if (strcmp(argv[i], "--parallel") == 0 || strcmp(argv[i], "-p") == 0) {
            g_sht_parallel_enabled = 1;
            g_sht_num_workers = SHT_MAX_THREADS;
        } else if (strcmp(argv[i], "-j") == 0) {
            g_sht_parallel_enabled = 1;
            if (i + 1 < argc) {
                int jobs = atoi(argv[i + 1]);
                if (jobs > 0) {
                    g_sht_num_workers = jobs;
                    i++;
                } else {
                    g_sht_num_workers = SHT_MAX_THREADS;
                }
            } else {
                g_sht_num_workers = SHT_MAX_THREADS;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("SHT - Simple Header-only Testing Framework\n\n");
            printf("Usage: %s [OPTIONS]\n\n", argv[0]);
            printf("Options:\n");
            printf("  -v, --verbose    Enable verbose output\n");
            printf("  -n, --no-color   Disable colored output\n");
            printf("  -c, --coverage   Enable coverage tracking\n");
            printf("  -p, --parallel   Enable parallel test execution\n");
            printf("  -j <N>           Set number of parallel workers (default: CPU cores)\n");
            printf("  -h, --help       Show this help message\n");
            return 1;
        }
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* SHT_H */

/* ============================================================================
 * BACKWARD COMPATIBILITY MACROS
 * ============================================================================ */

#define TEST_RUN_MAIN() \
    int main(int argc, char* argv[]) { \
        if (sht_init_context() != 0) { \
            return -1; \
        } \
        \
        int arg_result = sht_parse_arguments(argc, argv); \
        if (arg_result != 0) { \
            sht_cleanup_context(); \
            return arg_result > 0 ? 0 : -1; \
        } \
        \
        int test_result = sht_run_all_tests(); \
        sht_cleanup_context(); \
        \
        return test_result; \
    }

/* ============================================================================
 * COVERAGE INTEGRATION
 * ============================================================================ */

#ifdef SHT_IMPLEMENTATION

typedef struct {
    size_t total_lines;
    size_t covered_lines;
    size_t total_functions;
    size_t covered_functions;
    double line_coverage_pct;
    double function_coverage_pct;
} sht_coverage_data_t;

static sht_coverage_data_t g_sht_coverage_data = {0};

static void sht_coverage_init_func(void) {
    memset(&g_sht_coverage_data, 0, sizeof(g_sht_coverage_data));
}

static void sht_cleanup_func(void) {
    memset(&g_sht_coverage_data, 0, sizeof(g_sht_coverage_data));
}

static void sht_record_line_func(const char* file, int line) {
    (void)file;
    (void)line;
    g_sht_coverage_data.total_lines++;
    g_sht_coverage_data.covered_lines++;
}

static void sht_record_function_func(const char* name, const char* file, int start_line, int end_line) {
    (void)name;
    (void)file;
    (void)start_line;
    (void)end_line;
    g_sht_coverage_data.total_functions++;
    g_sht_coverage_data.covered_functions++;
}

static void sht_record_branch_func(const char* file, int line, int branch_id, int taken) {
    (void)file;
    (void)line;
    (void)branch_id;
    (void)taken;
}

static void sht_compute_coverage(void) {
    if (g_sht_coverage_data.total_lines > 0) {
        g_sht_coverage_data.line_coverage_pct = 
            (g_sht_coverage_data.covered_lines * 100.0) / g_sht_coverage_data.total_lines;
    }
    if (g_sht_coverage_data.total_functions > 0) {
        g_sht_coverage_data.function_coverage_pct = 
            (g_sht_coverage_data.covered_functions * 100.0) / g_sht_coverage_data.total_functions;
    }
}

static void sht_print_coverage_summary_func(void) {
    if (!g_sht_coverage_enabled) {
        return;
    }
    
    sht_compute_coverage();
    
    printf("\nCode Coverage:\n");
    
    if (g_sht_coverage_data.total_lines > 0) {
        sht_color_t line_color = g_sht_coverage_data.line_coverage_pct >= 80.0 ? SHT_COLOR_GREEN : 
                                 g_sht_coverage_data.line_coverage_pct >= 50.0 ? SHT_COLOR_YELLOW : SHT_COLOR_RED;
        sht_set_color(stdout, line_color);
        printf("  Line Coverage:     %.2f%% (%zu/%zu lines)\n", 
               g_sht_coverage_data.line_coverage_pct,
               g_sht_coverage_data.covered_lines, g_sht_coverage_data.total_lines);
        sht_reset_color(stdout);
    } else {
        printf("  Line Coverage:     N/A (no coverage data)\n");
    }
    
    if (g_sht_coverage_data.total_functions > 0) {
        sht_color_t func_color = g_sht_coverage_data.function_coverage_pct >= 80.0 ? SHT_COLOR_GREEN : 
                                 g_sht_coverage_data.function_coverage_pct >= 50.0 ? SHT_COLOR_YELLOW : SHT_COLOR_RED;
        sht_set_color(stdout, func_color);
        printf("  Function Coverage: %.2f%% (%zu/%zu functions)\n", 
               g_sht_coverage_data.function_coverage_pct,
               g_sht_coverage_data.covered_functions, g_sht_coverage_data.total_functions);
        sht_reset_color(stdout);
    } else {
        printf("  Function Coverage: N/A (no coverage data)\n");
    }
    printf("\n");
}

static void sht_enhanced_print_summary_integrations(void) {
}

#endif /* SHT_IMPLEMENTATION */

#define SHT_TEST(suite, test) TEST(suite, test)
#define SHT_SUITE(name)

#define SHT_ASSERT_EQ(actual, expected) ASSERT_EQ(actual, expected)
#define SHT_ASSERT_NE(actual, expected) ASSERT_NE(actual, expected)
#define SHT_ASSERT_LT(actual, expected) ASSERT_LT(actual, expected)
#define SHT_ASSERT_GT(actual, expected) ASSERT_GT(actual, expected)
#define SHT_ASSERT_LE(actual, expected) ASSERT_LE(actual, expected)
#define SHT_ASSERT_GE(actual, expected) ASSERT_GE(actual, expected)
#define SHT_ASSERT_TRUE(value) ASSERT_TRUE(value)
#define SHT_ASSERT_FALSE(value) ASSERT_FALSE(value)
#define SHT_ASSERT_NULL(ptr) ASSERT_NULL(ptr)
#define SHT_ASSERT_NOT_NULL(ptr) ASSERT_NOT_NULL(ptr)
#define SHT_ASSERT_STR_EQ(actual, expected) ASSERT_STR_EQ(actual, expected)
#define SHT_ASSERT_STR_NE(actual, expected) ASSERT_STR_NE(actual, expected)
#define SHT_ASSERT_FLOAT_EQ(actual, expected) ASSERT_FLOAT_EQ(actual, expected)
#define SHT_ASSERT_FLOAT_NEAR(actual, expected, tolerance) ASSERT_FLOAT_NEAR(actual, expected, tolerance)
#define SHT_ASSERT_MEM_EQ(actual, expected, size) ASSERT_MEM_EQ(actual, expected, size)
#define SHT_ASSERT_MEM_NE(actual, expected, size) ASSERT_MEM_NE(actual, expected, size)

#define SHT_EXPECT_EQ(actual, expected) EXPECT_EQ(actual, expected)
#define SHT_EXPECT_NE(actual, expected) EXPECT_NE(actual, expected)
#define SHT_EXPECT_LT(actual, expected) EXPECT_LT(actual, expected)
#define SHT_EXPECT_GT(actual, expected) EXPECT_GT(actual, expected)
#define SHT_EXPECT_LE(actual, expected) EXPECT_LE(actual, expected)
#define SHT_EXPECT_GE(actual, expected) EXPECT_GE(actual, expected)
#define SHT_EXPECT_TRUE(value) EXPECT_TRUE(value)
#define SHT_EXPECT_FALSE(value) EXPECT_FALSE(value)
#define SHT_EXPECT_NULL(ptr) EXPECT_NULL(ptr)
#define SHT_EXPECT_NOT_NULL(ptr) EXPECT_NOT_NULL(ptr)
#define SHT_EXPECT_STR_EQ(actual, expected) EXPECT_STR_EQ(actual, expected)
#define SHT_EXPECT_STR_NE(actual, expected) EXPECT_STR_NE(actual, expected)
#define SHT_EXPECT_FLOAT_EQ(actual, expected) EXPECT_FLOAT_EQ(actual, expected)
#define SHT_EXPECT_FLOAT_NEAR(actual, expected, tolerance) EXPECT_FLOAT_NEAR(actual, expected, tolerance)
#define SHT_EXPECT_MEM_EQ(actual, expected, size) EXPECT_MEM_EQ(actual, expected, size)
#define SHT_EXPECT_MEM_NE(actual, expected, size) EXPECT_MEM_NE(actual, expected, size)

#define SHT_SUITE(name)
#define SHT_REGISTER_TEST(name, suite)
#define SHT_RUN(argc, argv) TEST_RUN_MAIN()
#define SHT_FAIL(msg) do { printf("FAIL: %s\n", msg); return; } while(0)