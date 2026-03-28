#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

TEST(CoreAssertions, BasicAssert) {
    ASSERT_TRUE(1);
    ASSERT_FALSE(0);
    ASSERT_EQ(1, 1);
    ASSERT_NE(1, 2);
    ASSERT_LT(1, 2);
    ASSERT_LE(1, 1);
    ASSERT_GT(2, 1);
    ASSERT_GE(1, 1);
}

TEST(CoreAssertions, BasicExpect) {
    EXPECT_TRUE(1);
    EXPECT_FALSE(0);
    EXPECT_EQ(1, 1);
    EXPECT_NE(1, 2);
    EXPECT_LT(1, 2);
    EXPECT_LE(1, 1);
    EXPECT_GT(2, 1);
    EXPECT_GE(1, 1);
}

TEST(CoreAssertions, StringAssertions) {
    ASSERT_STR_EQ("hello", "hello");
    ASSERT_STR_NE("hello", "world");
    EXPECT_STR_EQ("hello", "hello");
    EXPECT_STR_NE("hello", "world");
}

TEST(CoreAssertions, PointerAssertions) {
    ASSERT_NULL(NULL);
    ASSERT_NOT_NULL(&"test");
    EXPECT_NULL(NULL);
    EXPECT_NOT_NULL(&"test");
}

TEST(CoreAssertions, FloatAssertions) {
    ASSERT_FLOAT_EQ(1.0, 1.0);
    ASSERT_FLOAT_NEAR(1.0, 1.1, 0.2);
    EXPECT_FLOAT_EQ(1.0, 1.0);
    EXPECT_FLOAT_NEAR(1.0, 1.1, 0.2);
}

TEST(CoreAssertions, MemoryAssertions) {
    char buf1[] = "abc";
    char buf2[] = "abc";
    char buf3[] = "abd";
    ASSERT_MEM_EQ(buf1, buf2, 3);
    ASSERT_MEM_NE(buf1, buf3, 3);
    EXPECT_MEM_EQ(buf1, buf2, 3);
    EXPECT_MEM_NE(buf1, buf3, 3);
}

TEST(CoreAssertions, BoundaryCases) {
    ASSERT_EQ(INT_MAX, INT_MAX);
    ASSERT_EQ(INT_MIN, INT_MIN);
    ASSERT_NE(INT_MAX, INT_MIN);
    EXPECT_EQ(INT_MAX, INT_MAX);
    EXPECT_EQ(INT_MIN, INT_MIN);
    ASSERT_FLOAT_EQ(0.0, 0.0);
    ASSERT_FLOAT_EQ(-0.0, 0.0);
    ASSERT_FLOAT_EQ(1.0e10, 1.0e10);
    EXPECT_FLOAT_EQ(0.0, 0.0);
    EXPECT_FLOAT_EQ(1.0e-10, 1.0e-10);
}

TEST(CoreAssertions, NULLAndEmptyStrings) {
    ASSERT_STR_EQ(NULL, NULL);
    ASSERT_STR_NE(NULL, "test");
    EXPECT_STR_EQ(NULL, NULL);
    EXPECT_STR_NE("test", NULL);
    const char* empty = "";
    ASSERT_STR_EQ(empty, "");
    ASSERT_STR_NE(empty, "a");
    EXPECT_STR_EQ(empty, "");
    EXPECT_STR_NE(empty, "a");
}

TEST(MemoryManagement, Allocation) {
    sht_init_memory_pool();

    void* ptr = sht_alloc(128);
    EXPECT_NOT_NULL(ptr);
    sht_free(ptr);

    sht_cleanup_memory_pool();
    EXPECT_EQ(sht_check_memory_leaks(), 0);
}

TEST(Colors, AllColors) {
    sht_set_color(stdout, SHT_COLOR_RED);
    sht_set_color(stdout, SHT_COLOR_GREEN);
    sht_set_color(stdout, SHT_COLOR_BLUE);
    sht_set_color(stdout, SHT_COLOR_CYAN);
    sht_set_color(stdout, SHT_COLOR_WHITE);
    sht_set_color(stdout, SHT_COLOR_YELLOW);
    sht_set_color(stdout, SHT_COLOR_MAGENTA);
    sht_set_color(stdout, SHT_COLOR_RESET);
}

int main() {
    if (sht_init_context() != 0) {
        return -1;
    }

    sht_parse_arguments(0, NULL);

    int result = sht_run_all_tests();
    sht_cleanup_context();

    return result;
}