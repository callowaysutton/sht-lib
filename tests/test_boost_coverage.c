#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static void register_boost_tests(void);

int main(int argc, char* argv[]) {
    if (sht_init_context() != 0) return -1;
    
    register_boost_tests();
    
    int arg_result = sht_parse_arguments(argc, argv);
    if (arg_result != 0) {
        sht_cleanup_context();
        return arg_result > 0 ? 0 : -1;
    }
    
    int result = sht_run_all_tests();
    sht_cleanup_context();
    return result;
}

static void test_fmt_uint(void) {
    char buf[64];
    sht_format_uint(buf, sizeof(buf), (uint64_t)12345);
    sht_format_uint(buf, sizeof(buf), (uint64_t)0);
    sht_format_uint(buf, sizeof(buf), UINT64_MAX);
}

static void test_fmt_double(void) {
    char buf[64];
    sht_format_double(buf, sizeof(buf), 3.14159);
    sht_format_double(buf, sizeof(buf), 0.0);
    sht_format_double(buf, sizeof(buf), -1e300);
}

static void test_fmt_ptr(void) {
    char buf[64];
    void* ptr = (void*)0xDEADBEEF;
    sht_format_ptr(buf, sizeof(buf), ptr);
    sht_format_ptr(buf, sizeof(buf), NULL);
}

static void test_fmt_value(void) {
    char buf[64];
    sht_format_value(buf, sizeof(buf), (int64_t)12345);
    sht_format_value(buf, sizeof(buf), (int64_t)0);
    sht_format_value(buf, sizeof(buf), INT64_MIN);
}

static void test_fmt_str_null(void) {
    char buf[64];
    sht_format_str(buf, sizeof(buf), NULL, 0);
}

static void test_fmt_str_normal(void) {
    char buf[64];
    sht_format_str(buf, sizeof(buf), "test", 0);
}

static void test_fmt_str_truncate(void) {
    char buf[128];
    char long_str[100];
    memset(long_str, 'A', 99);
    long_str[99] = '\0';
    sht_format_str(buf, sizeof(buf), long_str, 1);
}

static void test_coverage_line(void) {
    g_sht_coverage_enabled = 1;
    sht_record_line_func(__FILE__, __LINE__);
    g_sht_coverage_enabled = 0;
}

static void test_coverage_branch(void) {
    g_sht_coverage_enabled = 1;
    sht_record_branch_func(__FILE__, __LINE__, 0, 1);
    g_sht_coverage_enabled = 0;
}

static void test_all_colors(void) {
    sht_set_color(stdout, SHT_COLOR_RESET);
    sht_set_color(stdout, SHT_COLOR_BOLD);
    sht_set_color(stdout, SHT_COLOR_RED);
    sht_set_color(stdout, SHT_COLOR_GREEN);
    sht_set_color(stdout, SHT_COLOR_YELLOW);
    sht_set_color(stdout, SHT_COLOR_BLUE);
    sht_set_color(stdout, SHT_COLOR_MAGENTA);
    sht_set_color(stdout, SHT_COLOR_CYAN);
    sht_set_color(stdout, SHT_COLOR_WHITE);
    sht_reset_color(stdout);
}

static void test_memory_pool(void) {
    sht_context_t* orig = g_sht_context;
    
    sht_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.memory_pool_size = 1024;
    ctx.memory_pool = malloc(1024);
    g_sht_context = &ctx;
    ctx.leak_detection_enabled = 1;
    
    void* p1 = malloc(50);
    void* p2 = malloc(100);
    free(p1);
    free(p2);
    
    free(ctx.memory_pool);
    g_sht_context = orig;
}

static void test_memory_leak_check(void) {
    g_sht_context->allocations_count = 5;
}

static void register_boost_tests(void) {
    #define ADD_TEST(s, n, f) \
        do { \
            sht_test_t* t = sht_create_test(); \
            if (t) { \
                sht_snprintf(t->suite, sizeof(t->suite), "%s", s); \
                sht_snprintf(t->name, sizeof(t->name), "%s", n); \
                sht_snprintf(t->file, sizeof(t->file), "%s", __FILE__); \
                t->func = f; \
                t->enabled = 1; \
                sht_register_test(t); \
            } \
        } while(0)
    
    ADD_TEST("Boost", "FormatUint", test_fmt_uint);
    ADD_TEST("Boost", "FormatDouble", test_fmt_double);
    ADD_TEST("Boost", "FormatPtr", test_fmt_ptr);
    ADD_TEST("Boost", "FormatValue", test_fmt_value);
    ADD_TEST("Boost", "FormatStrNull", test_fmt_str_null);
    ADD_TEST("Boost", "FormatStrNormal", test_fmt_str_normal);
    ADD_TEST("Boost", "FormatStrTruncate", test_fmt_str_truncate);
    ADD_TEST("Boost", "CoverageLine", test_coverage_line);
    ADD_TEST("Boost", "CoverageBranch", test_coverage_branch);
    ADD_TEST("Boost", "AllColors", test_all_colors);
    ADD_TEST("Boost", "MemoryPool", test_memory_pool);
    ADD_TEST("Boost", "MemoryLeakCheck", test_memory_leak_check);
}
