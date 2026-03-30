#define SHT_IMPLEMENTATION
#include "../sht.h"
#include "../sht_util.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Scratch log path used across tests */
#define UTIL_LOG "/tmp/sht_util_test.log"

/* Remove the log file if it exists */
static void remove_log(void) { unlink(UTIL_LOG); }

/* Return the size of the log file, or -1 if it doesn't exist */
static long log_size(void) {
    struct stat st;
    return (stat(UTIL_LOG, &st) == 0) ? (long)st.st_size : -1L;
}

/* ────────────────────────────── sht_sys_log ─────────────────────────── */

TEST(ShtUtil_SysLog, SuccessfulCommand_ExitCodeZero) {
    remove_log();
    int rc = sht_sys_log(UTIL_LOG, "exit 0");
    EXPECT_EQ(rc, 0);
    remove_log();
}

TEST(ShtUtil_SysLog, FailingCommand_ExitCodeNonZero) {
    remove_log();
    int rc = sht_sys_log(UTIL_LOG, "exit 42");
    EXPECT_EQ(rc, 42);
    remove_log();
}

TEST(ShtUtil_SysLog, OutputGoesToLogFile) {
    remove_log();
    sht_sys_log(UTIL_LOG, "echo hello_from_sht_sys_log");

    FILE *f = fopen(UTIL_LOG, "r");
    ASSERT_NOT_NULL(f);

    char buf[128] = {0};
    fgets(buf, sizeof(buf), f);
    fclose(f);

    /* The echo output should contain our sentinel string */
    EXPECT_NOT_NULL(strstr(buf, "hello_from_sht_sys_log"));
    remove_log();
}

TEST(ShtUtil_SysLog, StderrAlsoGoesToLogFile) {
    remove_log();
    sht_sys_log(UTIL_LOG, "echo stderr_line >&2");

    long size = log_size();
    EXPECT_GT(size, 0L);
    remove_log();
}

TEST(ShtUtil_SysLog, AppendsOnMultipleCalls) {
    remove_log();
    sht_sys_log(UTIL_LOG, "echo line1");
    long size_after_first = log_size();

    sht_sys_log(UTIL_LOG, "echo line2");
    long size_after_second = log_size();

    /* Second call must have appended more bytes */
    EXPECT_GT(size_after_second, size_after_first);
    remove_log();
}

TEST(ShtUtil_SysLog, InvalidLogPath_FallsBackToSystem) {
    /* A path that cannot be created; system() fallback should still work */
    int rc = sht_sys_log("/no_such_dir/sht_util_test.log", "exit 0");
    EXPECT_EQ(rc, 0);
}

/* ─────────────────── sht_redirect_begin / sht_redirect_end ──────────── */

TEST(ShtUtil_Redirect, BeginAndEndRestore) {
    remove_log();

    int sout, serr;
    sht_redirect_begin(UTIL_LOG, &sout, &serr);
    /* Write something while redirected */
    printf("redirected_stdout\n");
    fflush(stdout);
    sht_redirect_end(&sout, &serr);

    /* After end(), stdout should be back — we should be able to write without
       crashing and the log file should contain our redirected output. */
    long size = log_size();
    EXPECT_GT(size, 0L);
    remove_log();
}

TEST(ShtUtil_Redirect, CapturedStdoutContent) {
    remove_log();

    int sout, serr;
    sht_redirect_begin(UTIL_LOG, &sout, &serr);
    printf("sentinel_stdout_capture\n");
    fflush(stdout);
    sht_redirect_end(&sout, &serr);

    FILE *f = fopen(UTIL_LOG, "r");
    ASSERT_NOT_NULL(f);
    char buf[128] = {0};
    fgets(buf, sizeof(buf), f);
    fclose(f);

    EXPECT_NOT_NULL(strstr(buf, "sentinel_stdout_capture"));
    remove_log();
}

TEST(ShtUtil_Redirect, CapturedStderrContent) {
    remove_log();

    int sout, serr;
    sht_redirect_begin(UTIL_LOG, &sout, &serr);
    fprintf(stderr, "sentinel_stderr_capture\n");
    fflush(stderr);
    sht_redirect_end(&sout, &serr);

    FILE *f = fopen(UTIL_LOG, "r");
    ASSERT_NOT_NULL(f);
    char buf[128] = {0};
    fgets(buf, sizeof(buf), f);
    fclose(f);

    EXPECT_NOT_NULL(strstr(buf, "sentinel_stderr_capture"));
    remove_log();
}

TEST(ShtUtil_Redirect, MultipleBeginEnd_Appends) {
    remove_log();

    int sout, serr;

    sht_redirect_begin(UTIL_LOG, &sout, &serr);
    printf("pass1\n");
    fflush(stdout);
    sht_redirect_end(&sout, &serr);

    long size1 = log_size();

    sht_redirect_begin(UTIL_LOG, &sout, &serr);
    printf("pass2\n");
    fflush(stdout);
    sht_redirect_end(&sout, &serr);

    long size2 = log_size();

    EXPECT_GT(size2, size1);
    remove_log();
}

TEST(ShtUtil_Redirect, InvalidLogPath_NocrashOnBeginEnd) {
    /* When the log file can't be opened, begin/end should not crash.
       The saved fds are still valid (dup of the real descriptors). */
    int sout, serr;
    sht_redirect_begin("/no_such_dir/sht_util_test.log", &sout, &serr);
    sht_redirect_end(&sout, &serr);
    /* If we reach here without a crash the test passes */
    EXPECT_TRUE(1);
}

int main(void) {
    sht_init_context();
    sht_parse_arguments(0, NULL);
    int result = sht_run_all_tests();
    sht_cleanup_context();
    return result;
}
