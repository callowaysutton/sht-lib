#define SHT_IMPLEMENTATION
#include "../sht.h"
#include <stdlib.h>
#include <string.h>

TEST(ArgParse, VerboseFlag) {
    char* args[] = {"test", "--verbose"};
    int argc = 2;
    g_sht_context->verbose_enabled = 0;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_context->verbose_enabled, 1);
}

TEST(ArgParse, VerboseShortFlag) {
    char* args[] = {"test", "-v"};
    int argc = 2;
    g_sht_context->verbose_enabled = 0;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_context->verbose_enabled, 1);
}

TEST(ArgParse, NoColorFlag) {
    char* args[] = {"test", "--no-color"};
    int argc = 2;
    g_sht_context->color_enabled = 1;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_context->color_enabled, 0);
}

TEST(ArgParse, NoColorShortFlag) {
    char* args[] = {"test", "-n"};
    int argc = 2;
    g_sht_context->color_enabled = 1;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_context->color_enabled, 0);
}

TEST(ArgParse, CoverageFlag) {
    char* args[] = {"test", "--coverage"};
    int argc = 2;
    g_sht_coverage_enabled = 0;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_coverage_enabled, 1);
}

TEST(ArgParse, CoverageShortFlag) {
    char* args[] = {"test", "-c"};
    int argc = 2;
    g_sht_coverage_enabled = 0;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_coverage_enabled, 1);
}

TEST(ArgParse, ParallelFlag) {
    char* args[] = {"test", "--parallel"};
    int argc = 2;
    g_sht_parallel_enabled = 0;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_parallel_enabled, 1);
}

TEST(ArgParse, ParallelShortFlag) {
    char* args[] = {"test", "-p"};
    int argc = 2;
    g_sht_parallel_enabled = 0;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_parallel_enabled, 1);
}

TEST(ArgParse, ParallelWithJobs) {
    char* args[] = {"test", "-j", "4"};
    int argc = 3;
    g_sht_parallel_enabled = 0;
    g_sht_num_workers = 1;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_parallel_enabled, 1);
    EXPECT_EQ(g_sht_num_workers, 4);
}

TEST(ArgParse, MultipleFlags) {
    char* args[] = {"test", "-v", "-c", "-p"};
    int argc = 4;
    g_sht_context->verbose_enabled = 0;
    g_sht_coverage_enabled = 0;
    g_sht_parallel_enabled = 0;
    sht_parse_arguments(argc, args);
    EXPECT_EQ(g_sht_context->verbose_enabled, 1);
    EXPECT_EQ(g_sht_coverage_enabled, 1);
    EXPECT_EQ(g_sht_parallel_enabled, 1);
}

TEST(ArgParse, HelpFlag) {
    char* args[] = {"test", "--help"};
    int argc = 2;
    int result = sht_parse_arguments(argc, args);
    EXPECT_EQ(result, 1);
}

TEST(ArgParse, HelpShortFlag) {
    char* args[] = {"test", "-h"};
    int argc = 2;
    int result = sht_parse_arguments(argc, args);
    EXPECT_EQ(result, 1);
}

TEST(ArgParse, UnknownFlag) {
    char* args[] = {"test", "--unknown"};
    int argc = 2;
    int result = sht_parse_arguments(argc, args);
    EXPECT_EQ(result, 0);
}

int main() {
    sht_init_context();
    int result = sht_run_all_tests();
    sht_cleanup_context();
    return result;
}