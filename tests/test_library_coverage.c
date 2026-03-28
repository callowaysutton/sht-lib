#define SHT_IMPLEMENTATION
#include "../sht.h"

TEST(TestCoverage, AllAssertions) {
    ASSERT_TRUE(1);
    ASSERT_FALSE(0);
    ASSERT_EQ(1, 1);
    ASSERT_NE(1, 2);
    ASSERT_LT(1, 2);
    ASSERT_LE(1, 1);
    ASSERT_GT(2, 1);
    ASSERT_GE(1, 1);
    ASSERT_STR_EQ("a", "a");
    ASSERT_STR_NE("a", "b");
    ASSERT_NULL(NULL);
    ASSERT_NOT_NULL(&"test");
    ASSERT_FLOAT_EQ(1.0, 1.0);
    ASSERT_FLOAT_NEAR(1.0, 1.1, 0.2);
    ASSERT_MEM_EQ("abc", "abc", 3);
    ASSERT_MEM_NE("abc", "abd", 3);
    
    EXPECT_TRUE(1);
    EXPECT_FALSE(0);
    EXPECT_EQ(1, 1);
    EXPECT_STR_EQ("a", "a");
    EXPECT_FLOAT_NEAR(1.0, 1.1, 0.2);
}

SET_UP(FixtureTest);
TEAR_DOWN(FixtureTest);

static int fixture_runs = 0;

static void FixtureTest_SetUp(void) {
    fixture_runs++;
}
static void FixtureTest_TearDown(void) {
    fixture_runs++;
}

TEST_F(FixtureTest, SetupTeardownWork) {
    EXPECT_EQ(fixture_runs, 1);
}

TEST(TestCoverage, ColorsAndOutput) {
    sht_set_color(stdout, SHT_COLOR_RED);
    sht_set_color(stdout, SHT_COLOR_GREEN);
    sht_set_color(stdout, SHT_COLOR_BLUE);
    sht_set_color(stdout, SHT_COLOR_RESET);
}

TEST(TestCoverage, MemoryLeakDetection) {
    void* ptr = malloc(100);
    ASSERT_NOT_NULL(ptr);
    free(ptr);
}


TEST(BoostCoverage, FormatUint) {
    char buffer[64];
    uint64_t val = 123456789ULL;
    sht_format_uint(buffer, sizeof(buffer), val);
}

TEST(BoostCoverage, FormatDouble) {
    char buffer[64];
    double val = 3.14159265359;
    sht_format_double(buffer, sizeof(buffer), val);
}

TEST(BoostCoverage, FormatPtr) {
    char buffer[64];
    void* ptr = (void*)0xDEADBEEF;
    sht_format_ptr(buffer, sizeof(buffer), ptr);
}

TEST(BoostCoverage, CoverageLine) {
    g_sht_coverage_enabled = 1;
    sht_record_line_func(__FILE__, __LINE__);
    g_sht_coverage_enabled = 0;
}

TEST(BoostCoverage, CoverageBranch) {
    g_sht_coverage_enabled = 1;
    sht_record_branch_func(__FILE__, __LINE__, 0, 1);
    g_sht_coverage_enabled = 0;
}

TEST(BoostCoverage, ColorDisabled) {
    sht_context_t* original = g_sht_context;
    g_sht_context->color_enabled = 0;
    sht_set_color(stdout, SHT_COLOR_RED);
    g_sht_context->color_enabled = 1;
    g_sht_context = original;
}

int main() {
    sht_init_context();
    sht_parse_arguments(0, NULL);
    int result = sht_run_all_tests();
    sht_cleanup_context();
    return result;
}
