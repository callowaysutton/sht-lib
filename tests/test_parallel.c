#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <pthread.h>

static int parallel_count = 0;
static pthread_mutex_t parallel_mutex = PTHREAD_MUTEX_INITIALIZER;

TEST(ParallelTest, Test1) {
    pthread_mutex_lock(&parallel_mutex);
    parallel_count++;
    pthread_mutex_unlock(&parallel_mutex);
}

TEST(ParallelTest, Test2) {
    pthread_mutex_lock(&parallel_mutex);
    parallel_count++;
    pthread_mutex_unlock(&parallel_mutex);
}

TEST(ParallelTest, Test3) {
    pthread_mutex_lock(&parallel_mutex);
    parallel_count++;
    pthread_mutex_unlock(&parallel_mutex);
}

TEST(ParallelTest, Test4) {
    pthread_mutex_lock(&parallel_mutex);
    parallel_count++;
    pthread_mutex_unlock(&parallel_mutex);
}

TEST(ParallelTest, Test5) {
    pthread_mutex_lock(&parallel_mutex);
    parallel_count++;
    pthread_mutex_unlock(&parallel_mutex);
}

int main(int argc, char* argv[]) {
    sht_init_context();
    
    char* args[] = {argv[0], "--parallel"};
    sht_parse_arguments(2, args);
    
    int result = sht_run_all_tests();
    sht_cleanup_context();
    return result;
}