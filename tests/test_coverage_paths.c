#define SHT_IMPLEMENTATION
#include "../sht.h"

TEST(CoveragePaths, TriggerCoverageInit) {
    extern int g_sht_coverage_enabled;
    g_sht_coverage_enabled = 1;
    EXPECT_TRUE(1);
}

TEST(CoveragePaths, AssertFailure) {
    XFAIL();
    ASSERT_EQ(1, 2);
}

TEST_RUN_MAIN();