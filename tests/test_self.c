#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int test_setup_count = 0;
static int test_teardown_count = 0;
static int test_order[10];
static int test_order_index = 0;

TEST(SelfTest, BasicAssertionsWork) {
    EXPECT_TRUE(1);
    EXPECT_FALSE(0);
    ASSERT_TRUE(1);
    ASSERT_FALSE(0);
}

TEST(SelfTest, IntegerComparisons) {
    EXPECT_EQ(42, 42);
    EXPECT_NE(42, 43);
    EXPECT_LT(10, 20);
    EXPECT_LE(10, 10);
    EXPECT_GT(20, 10);
    EXPECT_GE(10, 10);

    ASSERT_EQ(42, 42);
    ASSERT_NE(42, 43);
    ASSERT_LT(10, 20);
    ASSERT_LE(10, 10);
    ASSERT_GT(20, 10);
    ASSERT_GE(10, 10);
}

TEST(SelfTest, StringComparisons) {
    EXPECT_STR_EQ("hello", "hello");
    EXPECT_STR_NE("hello", "world");

    ASSERT_STR_EQ("hello", "hello");
    ASSERT_STR_NE("hello", "world");
}

TEST(SelfTest, FloatingPointComparisons) {
    EXPECT_FLOAT_EQ(1.0, 1.0);
    EXPECT_FLOAT_NEAR(1.0, 1.000001, 0.0001);

    ASSERT_FLOAT_EQ(1.0, 1.0);
    ASSERT_FLOAT_NEAR(1.0, 1.000001, 0.0001);
}

TEST(SelfTest, MemoryComparisons) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    int c[] = {1, 2, 4};

    EXPECT_MEM_EQ(a, b, sizeof(a));
    EXPECT_MEM_NE(a, c, sizeof(a));

    ASSERT_MEM_EQ(a, b, sizeof(a));
    ASSERT_MEM_NE(a, c, sizeof(a));
}

TEST(SelfTest, PointerComparisons) {
    int x = 42;
    int* ptr = &x;

    EXPECT_NOT_NULL(ptr);
    EXPECT_NULL(NULL);

    ASSERT_NOT_NULL(ptr);
    ASSERT_NULL(NULL);
}

SET_UP(SelfTestFixture);
TEAR_DOWN(SelfTestFixture);

static int fixture_state = 0;

static void SelfTestFixture_SetUp(void) {
    fixture_state = 1;
    test_setup_count++;
}

static void SelfTestFixture_TearDown(void) {
    fixture_state = 0;
    test_teardown_count++;
}

TEST_F(SelfTestFixture, SetupAndTeardownWork) {
    EXPECT_EQ(fixture_state, 1);
}

TEST_F(SelfTestFixture, MultipleTestsCleanup) {
    EXPECT_EQ(fixture_state, 1);
}

TEST(SelfTest, TestOrdering) {
    test_order[test_order_index++] = __LINE__;
}

TEST(SelfTest, TestOrdering2) {
    test_order[test_order_index++] = __LINE__;
}

TEST(SelfTest, TestOrdering3) {
    test_order[test_order_index++] = __LINE__;
}

TEST(SelfTest, SkipFunctionality) {
    EXPECT_TRUE(1);
    SKIP();
    EXPECT_TRUE(0);
}

TEST(SelfTest, StringEdgeCases) {
    const char* empty = "";
    EXPECT_STR_EQ(empty, "");
    EXPECT_STR_NE(empty, "x");

    char long_str[100];
    memset(long_str, 'a', sizeof(long_str) - 1);
    long_str[sizeof(long_str) - 1] = '\0';
    EXPECT_STR_EQ(long_str, long_str);
}

TEST(SelfTest, IntegerEdgeCases) {
    EXPECT_EQ(0, 0);
    EXPECT_EQ(-1, -1);
    EXPECT_EQ(INT_MAX, INT_MAX);
    EXPECT_EQ(INT_MIN, INT_MIN);

    EXPECT_NE(0, 1);
    EXPECT_NE(-1, 1);
}

TEST(SelfTest, FloatEdgeCases) {
    EXPECT_FLOAT_EQ(0.0, 0.0);
    EXPECT_FLOAT_EQ(-0.0, 0.0);
    EXPECT_FLOAT_EQ(1.0e10, 1.0e10);
    EXPECT_FLOAT_EQ(1.0e-10, 1.0e-10);

    EXPECT_FLOAT_NEAR(1.0, 1.0 + 1e-7, 1e-6);
}

TEST(SelfTest, ComparisonEdgeCases) {
    EXPECT_LT(INT_MIN, INT_MAX);
    EXPECT_LE(INT_MIN, INT_MAX);
    EXPECT_GT(INT_MAX, INT_MIN);
    EXPECT_GE(INT_MAX, INT_MAX);

    EXPECT_LT(0.0, 1.0);
    EXPECT_LE(0.0, 0.0);
    EXPECT_GT(1.0, 0.0);
    EXPECT_GE(1.0, 1.0);
}

static int cleanup_test_value = 0;

TEST(SelfTest, StateIsolationTest1) {
    cleanup_test_value = 1;
    EXPECT_EQ(cleanup_test_value, 1);
}

TEST(SelfTest, StateIsolationTest2) {
}

TEST(SelfTest, StateIsolationTest3) {
    EXPECT_EQ(cleanup_test_value, 1);
}

TEST(SelfTest, MultipleAssertions) {
    EXPECT_TRUE(1);
    EXPECT_FALSE(0);
    EXPECT_EQ(1, 1);
    EXPECT_NE(1, 2);
    EXPECT_LT(1, 2);
    EXPECT_LE(1, 1);
    EXPECT_GT(2, 1);
    EXPECT_GE(1, 1);
}

TEST(SelfTest, MemoryBlocks) {
    char block1[10] = {0};
    char block2[10] = {0};
    char block3[10] = {1, 2, 3, 4, 5};

    EXPECT_MEM_EQ(block1, block2, 10);
    EXPECT_MEM_NE(block1, block3, 10);

    ASSERT_MEM_EQ(block1, block2, 10);
    ASSERT_MEM_NE(block1, block3, 10);
}

TEST(SelfTest, ZeroSizeMemory) {
    int x = 42;
    EXPECT_MEM_EQ(&x, &x, 0);
    ASSERT_MEM_EQ(&x, &x, 0);
}

TEST(SelfTest, NULLStringComparisons) {
    EXPECT_STR_EQ(NULL, NULL);
    EXPECT_STR_NE(NULL, "test");
    EXPECT_STR_NE("test", NULL);

    ASSERT_STR_EQ(NULL, NULL);
    ASSERT_STR_NE(NULL, "test");
}

TEST(SelfTest, EmptyStringComparisons) {
    const char* empty = "";
    EXPECT_STR_EQ(empty, empty);
    EXPECT_STR_NE(empty, "a");

    ASSERT_STR_EQ(empty, empty);
    ASSERT_STR_NE(empty, "a");
}

TEST(SelfTest, LongStringComparison) {
    char str1[200];
    char str2[200];

    for (int i = 0; i < 199; i++) {
        str1[i] = 'a';
        str2[i] = 'a';
    }
    str1[199] = '\0';
    str2[199] = '\0';

    EXPECT_STR_EQ(str1, str2);
}

TEST(SelfTest, FixtureSetupCount) {
    EXPECT_GT(test_setup_count, 0);
}

TEST(SelfTest, FixtureTeardownCount) {
    EXPECT_GT(test_teardown_count, 0);
}

TEST(SelfTest, AllTestsRegistered) {
    EXPECT_TRUE(test_order_index >= 0);
}

TEST(SelfTest, NonFatalAssertionsContinue) {
    EXPECT_EQ(1, 1);
    EXPECT_EQ(1, 2);
    EXPECT_EQ(2, 2);
}

TEST(SelfTest, NullPointerSafety) {
    EXPECT_NULL(NULL);
    EXPECT_NOT_NULL(&test_setup_count);

    ASSERT_NULL(NULL);
    ASSERT_NOT_NULL(&test_teardown_count);
}

TEST(SelfTest, BooleanLogic) {
    int a = 1, b = 0;

    EXPECT_TRUE(a && !b);
    EXPECT_FALSE(a && b);

    ASSERT_TRUE(a && !b);
    ASSERT_FALSE(a && b);
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