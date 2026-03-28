#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <stdlib.h>
#include <string.h>

static int setup_count = 0;
static int teardown_count = 0;

SET_UP(Fixture);
TEAR_DOWN(Fixture);

static void Fixture_SetUp(void) {
    setup_count++;
}

static void Fixture_TearDown(void) {
    teardown_count++;
}

TEST_F(Fixture, SetupRuns) {
    EXPECT_EQ(setup_count, 1);
    EXPECT_EQ(teardown_count, 0);
}

TEST(Fixture, TeardownRan) {
    EXPECT_GT(teardown_count, 0);
}

TEST(Formatting, AllFormats) {
    char buf[128];

    sht_format_double(buf, sizeof(buf), 3.14159);
    EXPECT_TRUE(strlen(buf) > 0);
    sht_format_double(buf, sizeof(buf), -1.5);
    sht_format_double(buf, sizeof(buf), 0.0);
    sht_format_double(buf, sizeof(buf), 1e308);

    sht_format_value(buf, sizeof(buf), (int64_t)12345);
    EXPECT_TRUE(strlen(buf) > 0);
    sht_format_value(buf, sizeof(buf), (int64_t)-12345);
    sht_format_value(buf, sizeof(buf), (int64_t)0);

    sht_format_uint(buf, sizeof(buf), (uint64_t)123456789);
    EXPECT_TRUE(strlen(buf) > 0);
    sht_format_uint(buf, sizeof(buf), (uint64_t)0);

    sht_format_ptr(buf, sizeof(buf), (void*)0xDEADBEEF);
    EXPECT_TRUE(strlen(buf) > 0);
    sht_format_ptr(buf, sizeof(buf), NULL);
    sht_format_ptr(buf, sizeof(buf), (void*)0x0);

    sht_format_str(buf, sizeof(buf), NULL, 0);
    EXPECT_TRUE(strlen(buf) > 0);

    sht_format_str(buf, sizeof(buf), "", 0);
    EXPECT_TRUE(strlen(buf) > 0);

    sht_format_str(buf, sizeof(buf), "test", 0);

    char long_str[100];
    memset(long_str, 'A', 99);
    long_str[99] = '\0';
    sht_format_str(buf, sizeof(buf), long_str, 1);
    EXPECT_TRUE(strlen(buf) > 0);
    sht_format_str(buf, sizeof(buf), long_str, 0);
}

TEST(Context, FirstTestRegistration) {
    sht_context_t* orig = g_sht_context;
    sht_context_t temp = *orig;
    temp.tests = NULL;
    temp.last_test = NULL;
    temp.test_count = 0;
    g_sht_context = &temp;

    sht_test_t* t = sht_create_test();
    sht_snprintf(t->suite, sizeof(t->suite), "Test");
    sht_snprintf(t->name, sizeof(t->name), "FirstTest");
    t->func = (void (*)(void))(uintptr_t)1;
    sht_register_test(t);

    EXPECT_NOT_NULL(g_sht_context->tests);
    EXPECT_EQ(g_sht_context->test_count, 1);

    g_sht_context = orig;
}

TEST(ErrorHandling, AssertionWithoutContext) {
    sht_context_t* orig = g_sht_context;
    g_sht_context = NULL;

    sht_assertion_failure("test.c", 123, "test message");

    g_sht_context = orig;
}

TEST(ErrorHandling, CreateTestAndRegister) {
    sht_test_t* test = sht_create_test();
    EXPECT_NOT_NULL(test);
    EXPECT_EQ(test->result, SHT_RESULT_SKIP);
    EXPECT_EQ(test->enabled, 1);
    sht_destroy_test(test);
}

TEST(ErrorHandling, RegisterNullTest) {
    sht_register_test(NULL);
}

TEST(ErrorHandling, Snprintf) {
    char buf[64];
    sht_snprintf(buf, sizeof(buf), "test %d", 42);
    EXPECT_TRUE(strlen(buf) > 0);
}

TEST(CoverageFeatures, EnableCoverage) {
    extern int g_sht_coverage_enabled;
    g_sht_coverage_enabled = 1;
    EXPECT_TRUE(1);
}

static int teardown_flag = 0;

SET_UP(TeardownFixture);
TEAR_DOWN(TeardownFixture);

static void TeardownFixture_SetUp(void) {}
static void TeardownFixture_TearDown(void) { teardown_flag = 1; }

TEST_F(TeardownFixture, FailingTestWithXFAIL) {
    XFAIL();
    ASSERT_EQ(1, 2);
}

TEST(TeardownCheck, FlagSet) {
    EXPECT_EQ(teardown_flag, 1);
}

int main() {
    sht_init_context();
    int result = sht_run_all_tests();
    sht_cleanup_context();
    return result;
}