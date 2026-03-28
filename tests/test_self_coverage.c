#define SHT_IMPLEMENTATION
#define SHT_COVERAGE_IMPLEMENTATION
#include "../sht.h"
#include "../sht_coverage.h"

// Comprehensive self-tests to exercise all SHT features
TEST(SelfCoverage, AllAssertions) {
    // Exercise all assertion types
    ASSERT_TRUE(1);
    ASSERT_FALSE(0);
    ASSERT_EQ(1, 1);
    ASSERT_NE(1, 2);
    ASSERT_LT(1, 2);
    ASSERT_LE(1, 1);
    ASSERT_GT(2, 1);
    ASSERT_GE(1, 1);
    ASSERT_STR_EQ("test", "test");
    ASSERT_STR_NE("a", "b");
    ASSERT_NULL(NULL);
    ASSERT_NOT_NULL(&"test");
    ASSERT_FLOAT_EQ(1.0, 1.0);
    ASSERT_FLOAT_NEAR(1.0, 1.0, 0.1);
    
    EXPECT_TRUE(1);
    EXPECT_FALSE(0);
    EXPECT_EQ(1, 1);
    EXPECT_STR_EQ("test", "test");
}

TEST(SelfCoverage, ColorOutput) {
    sht_set_color(stdout, SHT_COLOR_GREEN);
    printf("Green\n");
    sht_reset_color(stdout);
}

TEST(SelfCoverage, MemoryManagement) {
    void* ptr = malloc(100);
    ASSERT_NOT_NULL(ptr);
    free(ptr);
}

TEST(SelfCoverage, CoverageTracking) {
    // Exercise coverage tracking
    sht_coverage_init();
    sht_record_line(__FILE__, __LINE__);
    sht_cleanup();
}

TEST_RUN_MAIN();
