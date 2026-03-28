/*
 * Example: Basic Arithmetic Testing with SHT
 * Demonstrates simple test cases and assertion macros
 */

#define SHT_IMPLEMENTATION
#include "../sht.h"

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    if (b == 0) {
        return 0;
    }
    return a / b;
}

TEST(Arithmetic, addition_basic) {
    EXPECT_EQ(add(2, 3), 5);
    EXPECT_EQ(add(-1, 1), 0);
    EXPECT_EQ(add(0, 0), 0);
}

TEST(Arithmetic, addition_negative) {
    EXPECT_EQ(add(-5, -3), -8);
    EXPECT_EQ(add(100, -50), 50);
}

TEST(Arithmetic, multiplication_basic) {
    EXPECT_EQ(multiply(3, 4), 12);
    EXPECT_EQ(multiply(-2, 3), -6);
    EXPECT_EQ(multiply(0, 10), 0);
}

TEST(Arithmetic, division_basic) {
    EXPECT_EQ(divide(10, 2), 5);
    EXPECT_EQ(divide(-6, 3), -2);
}

TEST(Arithmetic, division_by_zero) {
    EXPECT_EQ(divide(10, 0), 0);
    EXPECT_EQ(divide(0, 0), 0);
}

TEST_RUN_MAIN()