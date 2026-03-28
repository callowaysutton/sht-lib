#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

TEST(EdgeCases, AssertionFailureWithoutContext) {
    XFAIL();
    sht_assertion_failure("test.c", 42, "Test assertion failure");
}

TEST(EdgeCases, FormatValueTypes) {
    char buf[64];

    sht_format_value(buf, sizeof(buf), 0);
    sht_format_value(buf, sizeof(buf), 1);
    sht_format_value(buf, sizeof(buf), -1);
    sht_format_value(buf, sizeof(buf), INT64_MAX);
    sht_format_value(buf, sizeof(buf), INT64_MIN);
}

TEST(EdgeCases, FormatUintTypes) {
    char buf[64];

    sht_format_uint(buf, sizeof(buf), 0);
    sht_format_uint(buf, sizeof(buf), 1);
    sht_format_uint(buf, sizeof(buf), UINT64_MAX);
}

TEST(EdgeCases, FormatDoubleTypes) {
    char buf[64];

    sht_format_double(buf, sizeof(buf), 0.0);
    sht_format_double(buf, sizeof(buf), 1.0);
    sht_format_double(buf, sizeof(buf), -1.0);
    sht_format_double(buf, sizeof(buf), 1e308);
    sht_format_double(buf, sizeof(buf), -1e308);
    sht_format_double(buf, sizeof(buf), 1e-308);
    sht_format_double(buf, sizeof(buf), 3.14159265359);
}

TEST(EdgeCases, FormatPtrTypes) {
    char buf[64];

    sht_format_ptr(buf, sizeof(buf), NULL);
    sht_format_ptr(buf, sizeof(buf), (void*)0x0);
    sht_format_ptr(buf, sizeof(buf), (void*)0xFFFFFFFF);
    sht_format_ptr(buf, sizeof(buf), (void*)0xDEADBEEF);
}

TEST(EdgeCases, FormatStrNull) {
    char buf[64];
    sht_format_str(buf, sizeof(buf), NULL, 0);
}

TEST(EdgeCases, FormatStrEmpty) {
    char buf[64];
    sht_format_str(buf, sizeof(buf), "", 0);
}

TEST(EdgeCases, FormatStrShort) {
    char buf[64];
    sht_format_str(buf, sizeof(buf), "test", 0);
    sht_format_str(buf, sizeof(buf), "hello world", 0);
}

TEST(EdgeCases, FormatStrLong) {
    char buf[128];
    char long_str[100];
    memset(long_str, 'A', 99);
    long_str[99] = '\0';
    sht_format_str(buf, sizeof(buf), long_str, 0);
}

TEST(EdgeCases, FormatStrTruncate) {
    char buf[64];
    char long_str[100];
    memset(long_str, 'B', 99);
    long_str[99] = '\0';
    sht_format_str(buf, sizeof(buf), long_str, 1);
}

TEST(EdgeCases, CreateTest) {
    sht_test_t* test = sht_create_test();
    EXPECT_NOT_NULL(test);
    EXPECT_EQ(test->result, SHT_RESULT_SKIP);
    EXPECT_EQ(test->enabled, 1);
    sht_destroy_test(test);
}

TEST(EdgeCases, RegisterTestFirst) {
    sht_test_t* test = sht_create_test();
    sht_test_t* orig_last = g_sht_context->last_test;
    sht_test_t* orig_tests = g_sht_context->tests;

    g_sht_context->tests = NULL;
    g_sht_context->last_test = NULL;
    g_sht_context->test_count = 0;

    sht_register_test(test);

    EXPECT_EQ(g_sht_context->tests, test);
    EXPECT_EQ(g_sht_context->last_test, test);

    g_sht_context->tests = orig_tests;
    g_sht_context->last_test = orig_last;
}

TEST(EdgeCases, RegisterTestNull) {
    sht_register_test(NULL);
}

TEST(EdgeCases, Snprintf) {
    char buf[64];
    sht_snprintf(buf, sizeof(buf), "test %d", 42);
    EXPECT_TRUE(strlen(buf) > 0);
}

TEST(EdgeCases, XfailTestThatFails) {
    XFAIL();
    EXPECT_EQ(1, 2);
}

int main() {
    sht_init_context();
    int result = sht_run_all_tests();
    sht_cleanup_context();
    return result;
}