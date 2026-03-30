/**
 * @file sht_util.h
 * @brief Optional subprocess I/O helpers for the SHT testing framework.
 *
 * This companion header provides two utilities for keeping test output clean
 * when tests exercise code that prints to stdout/stderr:
 *
 *   sht_sys_log(logfile, cmd)
 *     Drop-in for system(cmd). Forks a child, redirects its stdout+stderr
 *     to `logfile`, and returns the command's exit code. The parent process's
 *     file descriptors are untouched, so SHT's own output is unaffected.
 *
 *   sht_redirect_begin(logfile, &saved_out, &saved_err)
 *   sht_redirect_end(&saved_out, &saved_err)
 *     Bracket in-process function calls that print directly. Redirects the
 *     current process's stdout+stderr to `logfile` for the duration of the
 *     call, then restores them.
 *
 *     Note: gcov/llvm-cov write coverage data via their own internal file
 *     descriptors — NOT fd 1/2 — so this redirection does not affect
 *     coverage instrumentation.
 *
 * Typical usage:
 *
 *   #include "sht_util.h"
 *
 *   TEST(MyLib, noisy_operation) {
 *       // Shell commands: use sht_sys_log
 *       sht_sys_log("mylib_output.log", "git clone https://example.com/repo /tmp/repo");
 *
 *       // In-process calls: use sht_redirect_begin/end
 *       int sout, serr;
 *       sht_redirect_begin("mylib_output.log", &sout, &serr);
 *       int rc = my_noisy_library_function();
 *       sht_redirect_end(&sout, &serr);
 *       ASSERT_EQ(rc, 0);
 *   }
 *
 * Each call appends to the log file (O_APPEND), so multiple tests can share
 * the same log file and produce a continuous record of their activity.
 */

#ifndef SHT_UTIL_H
#define SHT_UTIL_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Run a shell command, redirecting its stdout+stderr to a log file.
 *
 * @param logfile  Path to the log file (created/appended if needed).
 * @param cmd      Shell command string passed to /bin/sh -c.
 * @return         Exit code of the command, or -1 on fork/exec failure.
 *                 Falls back to system(cmd) if the log file cannot be opened.
 */
static inline int sht_sys_log(const char *logfile, const char *cmd) {
    int fd = open(logfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0)
        return system(cmd);

    pid_t pid = fork();
    if (pid < 0) {
        close(fd);
        return -1;
    }

    if (pid == 0) {
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }

    close(fd);
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/**
 * Redirect the current process's stdout+stderr to a log file.
 * Call sht_redirect_end() to restore them afterwards.
 *
 * @param logfile    Path to the log file (created/appended if needed).
 * @param saved_out  Receives the saved stdout fd; pass to sht_redirect_end().
 * @param saved_err  Receives the saved stderr fd; pass to sht_redirect_end().
 */
static inline void sht_redirect_begin(const char *logfile, int *saved_out,
                                       int *saved_err) {
    fflush(stdout);
    fflush(stderr);

    *saved_out = dup(STDOUT_FILENO);
    *saved_err = dup(STDERR_FILENO);

    int fd = open(logfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd >= 0) {
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
}

/**
 * Restore stdout+stderr after a sht_redirect_begin() call.
 *
 * @param saved_out  The saved stdout fd returned by sht_redirect_begin().
 * @param saved_err  The saved stderr fd returned by sht_redirect_begin().
 */
static inline void sht_redirect_end(int *saved_out, int *saved_err) {
    fflush(stdout);
    fflush(stderr);
    dup2(*saved_out, STDOUT_FILENO);
    dup2(*saved_err, STDERR_FILENO);
    close(*saved_out);
    close(*saved_err);
}

#endif /* SHT_UTIL_H */
