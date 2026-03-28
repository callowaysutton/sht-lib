/*
 * Simple Example: Demonstrates basic SHT testing
 * Run with: gcc examples/simple_example.c -o build/simple_example && ./build/simple_example
 *
 * To see coverage:
 *   ./run_example_coverage.sh simple_example
 */

#define SHT_IMPLEMENTATION
#include "../sht.h"

int add(int a, int b) {
    return a + b;
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

TEST(Math, addition_works) {
    EXPECT_EQ(add(2, 3), 5);
    EXPECT_EQ(add(0, 0), 0);
    EXPECT_EQ(add(-1, 1), 0);
}

TEST(Math, factorial_baseline) {
    EXPECT_EQ(factorial(0), 1);
    EXPECT_EQ(factorial(1), 1);
}

TEST(Math, factorial_recursive) {
    EXPECT_EQ(factorial(5), 120);
    EXPECT_EQ(factorial(3), 6);
}

TEST_RUN_MAIN()