#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <pthread.h>

static int test_count = 0;

TEST(ParallelForced, Test1) {
    test_count++;
    EXPECT_GT(test_count, 0);
}

TEST(ParallelForced, Test2) {
    test_count++;
    EXPECT_GT(test_count, 0);
}

TEST(ParallelForced, Test3) {
    test_count++;
    EXPECT_GT(test_count, 0);
}

int main() {
    sht_init_context();

    extern int sht_run_tests_parallel(int);
    sht_run_tests_parallel(2);

    sht_cleanup_context();

    return 0;
}