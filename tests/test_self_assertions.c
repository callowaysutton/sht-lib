#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int fatal_assertion_count = 0;
static int non_fatal_assertion_count = 0;

TEST(Assertions, ASSERT_TRUE_Passes) {
    ASSERT_TRUE(1);
    ASSERT_TRUE(100);
    ASSERT_TRUE(-1);
}

TEST(Assertions, ASSERT_TRUE_Fails) {
    XFAIL();
    ASSERT_TRUE(0);
}

TEST(Assertions, ASSERT_FALSE_Passes) {
    ASSERT_FALSE(0);
    ASSERT_FALSE(100 == 200);
}

TEST(Assertions, ASSERT_FALSE_Fails) {
    XFAIL();
    ASSERT_FALSE(1);
}

TEST(Assertions, ASSERT_EQ_Passes) {
    ASSERT_EQ(0, 0);
    ASSERT_EQ(1, 1);
    ASSERT_EQ(-1, -1);
    ASSERT_EQ(42, 42);
}

TEST(Assertions, ASSERT_EQ_Fails) {
    XFAIL();
    ASSERT_EQ(1, 2);
}

TEST(Assertions, ASSERT_NE_Passes) {
    ASSERT_NE(1, 2);
    ASSERT_NE(0, 1);
    ASSERT_NE(-1, 1);
}

TEST(Assertions, ASSERT_NE_Fails) {
    XFAIL();
    ASSERT_NE(42, 42);
}

TEST(Assertions, ASSERT_LT_Passes) {
    ASSERT_LT(0, 1);
    ASSERT_LT(-10, 10);
    ASSERT_LT(INT_MIN, INT_MAX);
}

TEST(Assertions, ASSERT_LT_Fails) {
    XFAIL();
    ASSERT_LT(10, 5);
}

TEST(Assertions, ASSERT_LE_Passes) {
    ASSERT_LE(1, 1);
    ASSERT_LE(0, 1);
    ASSERT_LE(-5, -5);
}

TEST(Assertions, ASSERT_LE_Fails) {
    XFAIL();
    ASSERT_LE(10, 5);
}

TEST(Assertions, ASSERT_GT_Passes) {
    ASSERT_GT(1, 0);
    ASSERT_GT(10, -10);
    ASSERT_GT(INT_MAX, INT_MIN);
}

TEST(Assertions, ASSERT_GT_Fails) {
    XFAIL();
    ASSERT_GT(5, 10);
}

TEST(Assertions, ASSERT_GE_Passes) {
    ASSERT_GE(1, 1);
    ASSERT_GE(10, 5);
    ASSERT_GE(-5, -5);
}

TEST(Assertions, ASSERT_GE_Fails) {
    XFAIL();
    ASSERT_GE(5, 10);
}

TEST(Assertions, ASSERT_NULL_Passes) {
    ASSERT_NULL(NULL);
}

TEST(Assertions, ASSERT_NULL_Fails) {
    XFAIL();
    int x = 42;
    ASSERT_NULL(&x);
}

TEST(Assertions, ASSERT_NOT_NULL_Passes) {
    int x = 42;
    ASSERT_NOT_NULL(&x);
}

TEST(Assertions, ASSERT_NOT_NULL_Fails) {
    XFAIL();
    ASSERT_NOT_NULL(NULL);
}

TEST(Assertions, ASSERT_STR_EQ_Passes) {
    ASSERT_STR_EQ("hello", "hello");
    ASSERT_STR_EQ("", "");
    ASSERT_STR_EQ(NULL, NULL);
}

TEST(Assertions, ASSERT_STR_EQ_Fails) {
    XFAIL();
    ASSERT_STR_EQ("hello", "world");
}

TEST(Assertions, ASSERT_STR_NE_Passes) {
    ASSERT_STR_NE("hello", "world");
    ASSERT_STR_NE("a", "b");
    ASSERT_STR_NE("test", NULL);
}

TEST(Assertions, ASSERT_STR_NE_Fails) {
    XFAIL();
    ASSERT_STR_NE("hello", "hello");
}

TEST(Assertions, ASSERT_FLOAT_EQ_Passes) {
    ASSERT_FLOAT_EQ(1.0, 1.0);
    ASSERT_FLOAT_EQ(0.0, 0.0);
    ASSERT_FLOAT_EQ(-1.5, -1.5);
}

TEST(Assertions, ASSERT_FLOAT_EQ_Fails) {
    XFAIL();
    ASSERT_FLOAT_EQ(1.0, 2.0);
}

TEST(Assertions, ASSERT_FLOAT_NEAR_Passes) {
    ASSERT_FLOAT_NEAR(1.0, 1.0001, 0.001);
    ASSERT_FLOAT_NEAR(0.0, 0.0000001, 0.001);
}

TEST(Assertions, ASSERT_FLOAT_NEAR_Fails) {
    XFAIL();
    ASSERT_FLOAT_NEAR(1.0, 2.0, 0.001);
}

TEST(Assertions, ASSERT_MEM_EQ_Passes) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    ASSERT_MEM_EQ(a, b, sizeof(a));
}

TEST(Assertions, ASSERT_MEM_EQ_Fails) {
    XFAIL();
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 4};
    ASSERT_MEM_EQ(a, b, sizeof(a));
}

TEST(Assertions, ASSERT_MEM_NE_Passes) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 4};
    ASSERT_MEM_NE(a, b, sizeof(a));
}

TEST(Assertions, ASSERT_MEM_NE_Fails) {
    XFAIL();
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    ASSERT_MEM_NE(a, b, sizeof(a));
}

TEST(NonFatalAssertions, EXPECT_TRUE_Passes) {
    EXPECT_TRUE(1);
    EXPECT_TRUE(100);
    EXPECT_TRUE(-1);
}

TEST(NonFatalAssertions, EXPECT_TRUE_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_TRUE(0);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_FALSE_Passes) {
    EXPECT_FALSE(0);
    EXPECT_FALSE(100 == 200);
}

TEST(NonFatalAssertions, EXPECT_FALSE_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_FALSE(1);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_EQ_Passes) {
    EXPECT_EQ(0, 0);
    EXPECT_EQ(1, 1);
    EXPECT_EQ(-1, -1);
}

TEST(NonFatalAssertions, EXPECT_EQ_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_EQ(1, 2);
    non_fatal_assertion_count++;
    EXPECT_EQ(3, 4);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_NE_Passes) {
    EXPECT_NE(1, 2);
    EXPECT_NE(0, 1);
}

TEST(NonFatalAssertions, EXPECT_NE_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_NE(42, 42);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_LT_Passes) {
    EXPECT_LT(0, 1);
    EXPECT_LT(-10, 10);
}

TEST(NonFatalAssertions, EXPECT_LT_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_LT(10, 5);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_LE_Passes) {
    EXPECT_LE(1, 1);
    EXPECT_LE(0, 1);
}

TEST(NonFatalAssertions, EXPECT_LE_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_LE(10, 5);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_GT_Passes) {
    EXPECT_GT(1, 0);
    EXPECT_GT(10, -10);
}

TEST(NonFatalAssertions, EXPECT_GT_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_GT(5, 10);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_GE_Passes) {
    EXPECT_GE(1, 1);
    EXPECT_GE(10, 5);
}

TEST(NonFatalAssertions, EXPECT_GE_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_GE(5, 10);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_NULL_Passes) {
    EXPECT_NULL(NULL);
}

TEST(NonFatalAssertions, EXPECT_NULL_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    int x = 42;
    EXPECT_NULL(&x);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_NOT_NULL_Passes) {
    int x = 42;
    EXPECT_NOT_NULL(&x);
}

TEST(NonFatalAssertions, EXPECT_NOT_NULL_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_NOT_NULL(NULL);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_STR_EQ_Passes) {
    EXPECT_STR_EQ("hello", "hello");
    EXPECT_STR_EQ("", "");
    EXPECT_STR_EQ(NULL, NULL);
}

TEST(NonFatalAssertions, EXPECT_STR_EQ_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_STR_EQ("hello", "world");
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_STR_NE_Passes) {
    EXPECT_STR_NE("hello", "world");
    EXPECT_STR_NE("a", "b");
}

TEST(NonFatalAssertions, EXPECT_STR_NE_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_STR_NE("hello", "hello");
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_FLOAT_EQ_Passes) {
    EXPECT_FLOAT_EQ(1.0, 1.0);
    EXPECT_FLOAT_EQ(0.0, 0.0);
}

TEST(NonFatalAssertions, EXPECT_FLOAT_EQ_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_FLOAT_EQ(1.0, 2.0);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_FLOAT_NEAR_Passes) {
    EXPECT_FLOAT_NEAR(1.0, 1.0001, 0.001);
}

TEST(NonFatalAssertions, EXPECT_FLOAT_NEAR_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_FLOAT_NEAR(1.0, 2.0, 0.001);
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_MEM_EQ_Passes) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    EXPECT_MEM_EQ(a, b, sizeof(a));
}

TEST(NonFatalAssertions, EXPECT_MEM_EQ_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 4};
    EXPECT_MEM_EQ(a, b, sizeof(a));
    non_fatal_assertion_count++;
}

TEST(NonFatalAssertions, EXPECT_MEM_NE_Passes) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 4};
    EXPECT_MEM_NE(a, b, sizeof(a));
}

TEST(NonFatalAssertions, EXPECT_MEM_NE_Fails) {
    XFAIL();
    non_fatal_assertion_count++;
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    EXPECT_MEM_NE(a, b, sizeof(a));
    non_fatal_assertion_count++;
}

TEST(EdgeCases, NULLStringComparison) {
    ASSERT_STR_EQ(NULL, NULL);
    ASSERT_STR_NE(NULL, "test");
    EXPECT_STR_EQ(NULL, NULL);
    EXPECT_STR_NE("test", NULL);
}

TEST(EdgeCases, EmptyStringComparison) {
    const char* empty = "";
    ASSERT_STR_EQ(empty, "");
    ASSERT_STR_NE(empty, "a");
    EXPECT_STR_EQ(empty, "");
    EXPECT_STR_NE(empty, "a");
}

TEST(EdgeCases, IntegerBoundaries) {
    ASSERT_EQ(INT_MAX, INT_MAX);
    ASSERT_EQ(INT_MIN, INT_MIN);
    ASSERT_NE(INT_MAX, INT_MIN);
    
    EXPECT_EQ(INT_MAX, INT_MAX);
    EXPECT_EQ(INT_MIN, INT_MIN);
    EXPECT_NE(INT_MAX, INT_MIN);
}

TEST(EdgeCases, FloatBoundaries) {
    ASSERT_FLOAT_EQ(0.0, 0.0);
    ASSERT_FLOAT_EQ(-0.0, 0.0);
    ASSERT_FLOAT_EQ(1.0e10, 1.0e10);
    ASSERT_FLOAT_EQ(1.0e-10, 1.0e-10);
    
    EXPECT_FLOAT_EQ(0.0, 0.0);
    EXPECT_FLOAT_EQ(-0.0, 0.0);
    EXPECT_FLOAT_EQ(1.0e10, 1.0e10);
    EXPECT_FLOAT_EQ(1.0e-10, 1.0e-10);
}

TEST(EdgeCases, ZeroSizeMemory) {
    XFAIL();
    int x = 42;
    ASSERT_MEM_EQ(&x, &x, 0);
    ASSERT_MEM_NE(&x, &x, 0);
    EXPECT_MEM_EQ(&x, &x, 0);
    EXPECT_MEM_NE(&x, &x, 0);
}

TEST(EdgeCases, LargeMemoryBlocks) {
    char block1[1000];
    char block2[1000];
    
    memset(block1, 'a', sizeof(block1));
    memset(block2, 'a', sizeof(block2));
    
    ASSERT_MEM_EQ(block1, block2, sizeof(block1));
    block2[0] = 'b';
    ASSERT_MEM_NE(block1, block2, sizeof(block1));
}

TEST(EdgeCases, ComparisonChain) {
    ASSERT_LT(INT_MIN, 0);
    ASSERT_LT(0, INT_MAX);
    
    EXPECT_LT(INT_MIN, 0);
    EXPECT_LT(0, INT_MAX);
}

TEST(EdgeCases, LongStrings) {
    char long1[300];
    char long2[300];
    char long3[300];
    
    memset(long1, 'x', sizeof(long1) - 1);
    long1[sizeof(long1) - 1] = '\0';
    
    memset(long2, 'x', sizeof(long2) - 1);
    long2[sizeof(long2) - 1] = '\0';
    
    memset(long3, 'x', sizeof(long3) - 1);
    long3[sizeof(long3) - 1] = '\0';
    long3[0] = 'y';
    
    ASSERT_STR_EQ(long1, long2);
    ASSERT_STR_NE(long1, long3);
    
    EXPECT_STR_EQ(long1, long2);
    EXPECT_STR_NE(long1, long3);
}

TEST(ErrorMessages, MessageFormatCorrectness) {
    XFAIL();
    int x = 5;
    int y = 10;
    
    ASSERT_EQ(x, y);
}

TEST(ErrorMessages, FloatMessageFormat) {
    XFAIL();
    double a = 1.23456789;
    double b = 9.87654321;
    
    ASSERT_FLOAT_NEAR(a, b, 0.0001);
}

TEST(ErrorMessages, StringMessageFormat) {
    XFAIL();
    ASSERT_STR_EQ("expected this", "but got that");
}

TEST(AssertionDifferences, NonFatalContinuesAfterFailure) {
    XFAIL();
    non_fatal_assertion_count++;
    EXPECT_EQ(1, 2);
    non_fatal_assertion_count++;
    EXPECT_EQ(3, 4);
    non_fatal_assertion_count++;
    EXPECT_EQ(5, 5);
    non_fatal_assertion_count++;
}

TEST(AssertionDifferences, FatalStopsAtFirstFailure) {
    XFAIL();
    ASSERT_EQ(1, 2);
    ASSERT_EQ(3, 4);
    ASSERT_EQ(5, 5);
}

TEST(NonFatalAssertions, NonFatalCounterCheck) {
    EXPECT_GT(non_fatal_assertion_count, 0);
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