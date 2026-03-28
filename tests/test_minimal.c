#define SHT_IMPLEMENTATION
#include "../sht.h"

TEST(Simple, one_plus_one) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(Simple, two_plus_two) {
    EXPECT_EQ(2 + 2, 4);
}

TEST_RUN_MAIN()