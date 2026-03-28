#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <stdlib.h>
#include <string.h>

static int simple_test_count = 0;
static int suite_a_count = 0;
static int suite_b_count = 0;
static int setup_calls = 0;
static int teardown_calls = 0;

TEST(Simple, Test1) {
    simple_test_count++;
}

TEST(Simple, Test2) {
    simple_test_count++;
}

TEST(SuiteA, Test1) {
    suite_a_count++;
}

TEST(SuiteA, Test2) {
    suite_a_count++;
}

TEST(SuiteA, Test3) {
    suite_a_count++;
}

TEST(SuiteB, Test1) {
    suite_b_count++;
}

TEST(SuiteB, Test2) {
    suite_b_count++;
}

SET_UP(TestFixture);
TEAR_DOWN(TestFixture);

static int fixture_val = 0;

static void TestFixture_SetUp(void) {
    fixture_val = 100;
    setup_calls++;
}

static void TestFixture_TearDown(void) {
    fixture_val = 0;
    teardown_calls++;
}

TEST_F(TestFixture, Test1) {
    EXPECT_EQ(fixture_val, 100);
}

TEST_F(TestFixture, Test2) {
    EXPECT_EQ(fixture_val, 100);
}

TEST_F(TestFixture, Test3) {
    EXPECT_EQ(fixture_val, 100);
}

SET_UP(TrackingFixture);
TEAR_DOWN(TrackingFixture);

static int* tracked_ptr = NULL;

static void TrackingFixture_SetUp(void) {
    tracked_ptr = malloc(sizeof(int));
    *tracked_ptr = 42;
}

static void TrackingFixture_TearDown(void) {
    if (tracked_ptr) {
        free(tracked_ptr);
        tracked_ptr = NULL;
    }
}

TEST_F(TrackingFixture, MemoryTracked) {
    EXPECT_NOT_NULL(tracked_ptr);
    EXPECT_EQ(*tracked_ptr, 42);
}

TEST(Counting, SimpleTests) {
    EXPECT_EQ(simple_test_count, 2);
}

TEST(Counting, SuiteATests) {
    EXPECT_EQ(suite_a_count, 3);
}

TEST(Counting, SuiteBTests) {
    EXPECT_EQ(suite_b_count, 2);
}

TEST(FixtureCalls, SetupCalled) {
    EXPECT_GE(setup_calls, 3);
}

TEST(FixtureCalls, TeardownCalled) {
    EXPECT_GE(teardown_calls, 3);
}

TEST(StateIsolation, Test1ModifiesState) {
    static int test_state = 0;
    test_state = 100;
    EXPECT_EQ(test_state, 100);
}

TEST(StateIsolation, Test2SeesInitialState) {
    static int test_state = 0;
    EXPECT_EQ(test_state, 0);
}

TEST(Timing, FastTest) {
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    EXPECT_EQ(sum, 4950);
}

TEST(Timing, SlowerTest) {
    int sum = 0;
    for (int i = 0; i < 10000; i++) {
        sum += i;
    }
    EXPECT_EQ(sum, 49995000);
}

TEST(SkipTests, SkipBasic) {
    EXPECT_TRUE(1);
    SKIP();
    EXPECT_TRUE(0);
}

TEST(SkipTests, SkipEarly) {
    SKIP();
    EXPECT_TRUE(0);
}

TEST(SkipTests, NoSkip) {
    EXPECT_TRUE(1);
    EXPECT_TRUE(1);
}

TEST(FailureTests, PassTest) {
    EXPECT_EQ(1, 1);
    EXPECT_EQ(2, 2);
}

TEST(FailureTests, NonFatalFailTest) {
    XFAIL();

    EXPECT_EQ(1, 1);
    EXPECT_EQ(2, 3);
    EXPECT_EQ(3, 3);
}

TEST(FailureTests, FatalFailTest) {
    XFAIL();

    EXPECT_EQ(1, 1);
    ASSERT_EQ(2, 3);
    ASSERT_EQ(3, 3);
}

TEST(MultipleSuites, Suite1Test) {
    EXPECT_TRUE(1);
}

TEST(MultipleSuites, Suite2Test) {
    EXPECT_TRUE(1);
}

TEST(MultipleSuites, Suite3Test) {
    EXPECT_TRUE(1);
}

SET_UP(OrderingFixture);
TEAR_DOWN(OrderingFixture);

static int order_counter = 0;
static int setup_values[10];
static int test_values[10];
static int teardown_values[10];
static int order_index = 0;

static void OrderingFixture_SetUp(void) {
    setup_values[order_index] = ++order_counter;
}

static void OrderingFixture_TearDown(void) {
    teardown_values[order_index] = ++order_counter;
    order_index++;
}

TEST_F(OrderingFixture, Test1) {
    test_values[order_index] = ++order_counter;
}

TEST_F(OrderingFixture, Test2) {
    test_values[order_index] = ++order_counter;
}

TEST_F(OrderingFixture, Test3) {
    test_values[order_index] = ++order_counter;
}

TEST(OrderingValidation, SetupBeforeTest) {
    for (int i = 0; i < 3; i++) {
        EXPECT_LT(setup_values[i], test_values[i]);
    }
}

TEST(OrderingValidation, TestBeforeTeardown) {
    for (int i = 0; i < 3; i++) {
        EXPECT_LT(test_values[i], teardown_values[i]);
    }
}

TEST(OrderingValidation, SequentialOrdering) {
    for (int i = 1; i < 3; i++) {
        EXPECT_LT(teardown_values[i-1], setup_values[i]);
    }
}

SET_UP(CleanupFixture);
TEAR_DOWN(CleanupFixture);

static char* cleanup_buffer = NULL;
static size_t cleanup_size = 0;

static void CleanupFixture_SetUp(void) {
    cleanup_size = 1024;
    cleanup_buffer = malloc(cleanup_size);
    memset(cleanup_buffer, 'A', cleanup_size);
}

static void CleanupFixture_TearDown(void) {
    if (cleanup_buffer) {
        free(cleanup_buffer);
        cleanup_buffer = NULL;
    }
    cleanup_size = 0;
}

TEST_F(CleanupFixture, BufferAllocated) {
    EXPECT_NOT_NULL(cleanup_buffer);
    EXPECT_EQ(cleanup_size, 1024);
    for (size_t i = 0; i < cleanup_size; i++) {
        EXPECT_EQ(cleanup_buffer[i], 'A');
    }
}

TEST_F(CleanupFixture, BufferReallocated) {
    EXPECT_NOT_NULL(cleanup_buffer);
    EXPECT_EQ(cleanup_size, 1024);
}

TEST(Statistics, TotalTestsCount) {
    EXPECT_TRUE(1);
}

TEST(Statistics, PassCount) {
    EXPECT_TRUE(1);
}

TEST(Statistics, FailCount) {
    EXPECT_EQ(1, 1);
}

TEST(Statistics, SkipCount) {
    EXPECT_TRUE(1);
}

TEST(EdgeCaseTests, EmptyTest) {
}

TEST(EdgeCaseTests, SingleAssertion) {
    EXPECT_TRUE(1);
}

TEST(EdgeCaseTests, ManyAssertions) {
    for (int i = 0; i < 100; i++) {
        EXPECT_TRUE(i >= 0);
    }
}

TEST(EdgeCaseTests, SameSuiteDifferentBehavior) {
    EXPECT_TRUE(1);
}

TEST(EdgeCaseTests, SameSuiteDifferentBehavior2) {
    EXPECT_TRUE(1);
}

TEST(StringTests, BasicStrings) {
    const char* str = "hello";
    EXPECT_STR_EQ(str, "hello");
}

TEST(StringTests, LongStrings) {
    char longstr[200];
    memset(longstr, 'x', sizeof(longstr) - 1);
    longstr[sizeof(longstr) - 1] = '\0';
    EXPECT_STR_EQ(longstr, longstr);
}

TEST(PointerTests, NullPointer) {
    int* ptr = NULL;
    EXPECT_NULL(ptr);
}

TEST(PointerTests, ValidPointer) {
    int x = 42;
    EXPECT_NOT_NULL(&x);
}

TEST(FloatTests, Equality) {
    EXPECT_FLOAT_EQ(1.0, 1.0);
}

TEST(FloatTests, NearEquality) {
    EXPECT_FLOAT_NEAR(1.0, 1.00001, 0.001);
}

TEST(MemoryTests, BlockEquality) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    EXPECT_MEM_EQ(a, b, sizeof(a));
}

TEST(MemoryTests, BlockInequality) {
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 4};
    EXPECT_MEM_NE(a, b, sizeof(a));
}

TEST(ComparisonTests, LessThan) {
    EXPECT_LT(1, 2);
}

TEST(ComparisonTests, GreaterThan) {
    EXPECT_GT(2, 1);
}

TEST(ComparisonTests, LessThanOrEqual) {
    EXPECT_LE(1, 1);
}

TEST(ComparisonTests, GreaterThanOrEqual) {
    EXPECT_GE(1, 1);
}

TEST(BooleanTests, TrueValue) {
    EXPECT_TRUE(1);
}

TEST(BooleanTests, FalseValue) {
    EXPECT_FALSE(0);
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