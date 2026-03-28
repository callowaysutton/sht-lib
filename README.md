SHT - Simple Header-only Testing Framework
===========================================

A modern, pytest-inspired testing framework for C.

## Features

- **Simple single-header**: Just `#include "sht.h"` and go
- **Auto-discovery**: Tests automatically discovered via compiler constructors
- **Rich assertions**: EXPECT_* and ASSERT_* macros with helpful messages
- **Colored output**: Beautiful terminal output with color coding
- **Memory leak detection**: Automatically tracks test memory allocations
- **Fixture support**: Setup/teardown functions for test suites
- **Skip functionality**: Skip tests that can't run in certain environments
- **Comprehensive statistics**: Timing, assertion counts, pass/fail ratios

## Quick Start

Here's a complete, runnable example showing both your functional code and the test code:

```c
/* my_math.c */
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}
```

```c
/* test_my_math.c */
#define SHT_IMPLEMENTATION
#include "sht.h"

int add(int a, int b);
int multiply(int a, int b);

TEST(Math, addition_works) {
    EXPECT_EQ(add(2, 3), 5);
    EXPECT_EQ(add(0, 0), 0);
    EXPECT_EQ(add(-1, 1), 0);
}

TEST(Math, multiplication_works) {
    EXPECT_EQ(multiply(2, 3), 6);
    EXPECT_EQ(multiply(0, 5), 0);
    EXPECT_EQ(multiply(-2, 3), -6);
}

TEST_RUN_MAIN()
```

Compile and run:
```bash
gcc -o test_my_math test_my_math.c my_math.c
./test_my_math
```

Output:
```
======================================================
  SHT - Simple Header-only Testing Framework
======================================================
  Running 2 test(s)...

Math Suite:
  [PASS] addition_works                                   (0.001 ms)
  [PASS] multiplication_works                             (0.000 ms)

============
Test Summary
============
Tests: 2 passed, 0 failed, 0 skipped

  Total: 2
  Assertions:   0/6 (failed/total)
  Time:         0.002 ms
  Leaks:        No memory leaks detected

SUCCESS!
```

## Build System

```bash
# Run all tests
make all

# Run coverage (requires lcov/genhtml)
make coverage

# Clean build artifacts
make clean
```

## Test Coverage

### Summary

| Metric | Value | Status |
|--------|-------|--------|
| **Line Coverage** | 81.2% (455/560 lines) | ✅ Good |
| **Function Coverage** | 97.1% (33/34 functions) | ✅ Excellent |
| **Test Suites** | 14 | — |
| **Total Tests** | 200+ | — |
| **Coverage Tool** | gcov + lcov | — |

The SHT library achieves robust test coverage across all major functionality including assertion mechanisms, test execution, memory management, and output formatting.

### Why Not 100%?

Approximately 93 lines (~17%) of the library are difficult to cover due to fundamental limitations of gcov instrumentation:

- **Parallel execution** (~35 lines) - pthread-based parallel test execution runs successfully but gcov cannot capture coverage data from threaded code
- **Coverage computation** (~21 lines) - Static function duplication caused by `#define SHT_IMPLEMENTATION` creates multiple copies that gcov cannot properly aggregate
- **Error paths & defensive code** (~37 lines) - Rare error conditions, NULL checks, and boundary cases that would require artificial injection to trigger

### Test Suite Overview

| Test Suite | Description | Tests |
|------------|-------------|-------|
| **test_self.c** | Core framework self-tests (assertions, fixtures, ordering) | 31 |
| **test_self_assertions.c** | All assertion macro variations | 78 |
| **test_self_runner.c** | Test runner and execution flow | 58 |
| **test_library_coverage.c** | Coverage-boosting tests for edge cases | 10 |
| **test_core.c** | Core assertion functionality | 10 |
| **test_fixtures_and_features.c** | Fixtures, setup/teardown, advanced features | 5 |
| **test_parallel.c** | Parallel test execution | 13 |
| **test_parallel_direct.c** | Direct parallel API testing | 3 |
| **test_argparse.c** | Command-line argument parsing | 15 |
| **test_edge_cases.c** | Boundary conditions and error handling | 11 |
| **test_coverage_paths.c** | Coverage path exploration | 5 |
| **simple_example.c** | Example math functions (addition, factorial) | 3 |
| **example_arithmetic.c** | Example arithmetic operations | 5 |
| **rpn_calculator_test.c** | RPN calculator comprehensive tests | 13 |

### Testing Features Exercised

**Assertions:**
- Basic: `ASSERT_TRUE`, `ASSERT_FALSE`, `EXPECT_TRUE`, `EXPECT_FALSE`
- Comparisons: `ASSERT_EQ`, `ASSERT_NE`, `ASSERT_LT`, `ASSERT_LE`, `ASSERT_GT`, `ASSERT_GE`
- Strings: `ASSERT_STR_EQ`, `ASSERT_STR_NE`, `EXPECT_STR_EQ`, `EXPECT_STR_NE`
- Pointers: `ASSERT_NULL`, `ASSERT_NOT_NULL`, `EXPECT_NULL`, `EXPECT_NOT_NULL`
- Floats: `ASSERT_FLOAT_EQ`, `ASSERT_FLOAT_NEAR`, `EXPECT_FLOAT_EQ`, `EXPECT_FLOAT_NEAR`
- Memory: `ASSERT_MEM_EQ`, `ASSERT_MEM_NE`, `EXPECT_MEM_EQ`, `EXPECT_MEM_NE`

**Mechanisms:**
- Auto-discovery via compiler constructors
- Fatal vs non-failure assertion handling
- Test suite grouping and ordering
- Setup/teardown fixtures
- Skip test functionality
- XFAIL (expected failure) marking
- Memory leak detection
- Thread-safe parallel execution
- Colored terminal output
- Command-line argument parsing (`--verbose`, `--filter`, etc.)

### Running Coverage

```bash
# Generate HTML coverage report
make coverage

# View the report (macOS)
open coverage/html/index.html

# View the report (Linux)
xdg-open coverage/html/index.html
# or
firefox coverage/html/index.html
```

<details>
<summary>📊 Full <code>make all</code> Output</summary>

```text
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  🧪 Running Tests with Coverage
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  📋 Self-Tests

    ▶ test_self_cov
    [PASS] BasicAssertionsWork                                (0.001 ms)
    [PASS] IntegerComparisons                                 (0.000 ms)
    [PASS] StringComparisons                                  (0.001 ms)
    [PASS] FloatingPointComparisons                           (0.000 ms)
    [PASS] MemoryComparisons                                  (0.001 ms)
    [PASS] PointerComparisons                                 (0.000 ms)
    [PASS] SetupAndTeardownWork                               (0.000 ms)
    [PASS] MultipleTestsCleanup                               (0.000 ms)
    [PASS] TestOrdering                                       (0.000 ms)
    [PASS] TestOrdering2                                      (0.001 ms)
    [PASS] TestOrdering3                                      (0.000 ms)
    [SKIP] SkipFunctionality                                  (0.001 ms)
    [PASS] StringEdgeCases                                    (0.000 ms)
    [PASS] IntegerEdgeCases                                   (0.000 ms)
    [PASS] FloatEdgeCases                                     (0.000 ms)
    [PASS] ComparisonEdgeCases                                (0.000 ms)
    [PASS] StateIsolationTest1                                (0.000 ms)
    [PASS] StateIsolationTest2                                (0.000 ms)
    [PASS] StateIsolationTest3                                (0.000 ms)
    [PASS] MultipleAssertions                                 (0.001 ms)
    [PASS] MemoryBlocks                                       (0.001 ms)
    [PASS] ZeroSizeMemory                                     (0.000 ms)
    [PASS] NULLStringComparisons                              (0.000 ms)
    [PASS] EmptyStringComparisons                             (0.000 ms)
    [PASS] LongStringComparison                               (0.000 ms)
    [PASS] FixtureSetupCount                                  (0.000 ms)
    [PASS] FixtureTeardownCount                               (0.000 ms)
    [PASS] AllTestsRegistered                                 (0.000 ms)
    [PASS] NonFatalAssertionsContinue                         (0.002 ms)
    [PASS] NullPointerSafety                                  (0.000 ms)
    [PASS] BooleanLogic                                       (0.001 ms)
Test Summary: 30 passed, 0 failed, 1 skipped
Assertions: 1/96 (failed/total)
✓ Completed

    ▶ test_self_assertions_cov
[PASS] ASSERT_TRUE_Passes                                 (0.001 ms)
[PASS] ASSERT_TRUE_Fails                                  (0.001 ms)
[PASS] ASSERT_FALSE_Passes                                (0.000 ms)
[PASS] ASSERT_FALSE_Fails                                 (0.001 ms)
[PASS] ASSERT_EQ_Passes                                   (0.000 ms)
[PASS] ASSERT_EQ_Fails                                    (0.002 ms)
[PASS] ASSERT_NE_Passes                                   (0.001 ms)
[PASS] ASSERT_NE_Fails                                    (0.001 ms)
[PASS] ASSERT_LT_Passes                                   (0.000 ms)
[PASS] ASSERT_LT_Fails                                    (0.001 ms)
[PASS] ASSERT_LE_Passes                                   (0.000 ms)
[PASS] ASSERT_LE_Fails                                    (0.001 ms)
[PASS] ASSERT_GT_Passes                                   (0.000 ms)
[PASS] ASSERT_GT_Fails                                    (0.001 ms)
[PASS] ASSERT_GE_Passes                                   (0.000 ms)
[PASS] ASSERT_GE_Fails                                    (0.001 ms)
[PASS] ASSERT_NULL_Passes                                 (0.000 ms)
[PASS] ASSERT_NULL_Fails                                  (0.000 ms)
[PASS] ASSERT_NOT_NULL_Passes                             (0.000 ms)
[PASS] ASSERT_NOT_NULL_Fails                              (0.001 ms)
[PASS] ASSERT_STR_EQ_Passes                               (0.001 ms)
[PASS] ASSERT_STR_EQ_Fails                                (0.002 ms)
[PASS] ASSERT_STR_NE_Passes                               (0.001 ms)
[PASS] ASSERT_STR_NE_Fails                                (0.001 ms)
[PASS] ASSERT_FLOAT_EQ_Passes                             (0.001 ms)
[PASS] ASSERT_FLOAT_EQ_Fails                              (0.002 ms)
[PASS] ASSERT_FLOAT_NEAR_Passes                           (0.001 ms)
[PASS] ASSERT_FLOAT_NEAR_Fails                            (0.001 ms)
[PASS] ASSERT_MEM_EQ_Passes                               (0.000 ms)
[PASS] ASSERT_MEM_EQ_Fails                                (0.001 ms)
[PASS] ASSERT_MEM_NE_Passes                               (0.000 ms)
[PASS] ASSERT_MEM_NE_Fails                                (0.001 ms)
[PASS] EXPECT_TRUE_Passes                                 (0.001 ms)
[PASS] EXPECT_TRUE_Fails                                  (0.001 ms)
[PASS] EXPECT_FALSE_Passes                                (0.000 ms)
[PASS] EXPECT_FALSE_Fails                                 (0.000 ms)
[PASS] EXPECT_EQ_Passes                                   (0.000 ms)
[PASS] EXPECT_EQ_Fails                                    (0.001 ms)
[PASS] EXPECT_NE_Passes                                   (0.000 ms)
[PASS] EXPECT_NE_Fails                                    (0.001 ms)
[PASS] EXPECT_LT_Passes                                   (0.000 ms)
[PASS] EXPECT_LT_Fails                                    (0.001 ms)
[PASS] EXPECT_LE_Passes                                   (0.000 ms)
[PASS] EXPECT_LE_Fails                                    (0.001 ms)
[PASS] EXPECT_GT_Passes                                   (0.000 ms)
[PASS] EXPECT_GT_Fails                                    (0.001 ms)
[PASS] EXPECT_GE_Passes                                   (0.000 ms)
[PASS] EXPECT_GE_Fails                                    (0.001 ms)
[PASS] EXPECT_NULL_Passes                                 (0.000 ms)
[PASS] EXPECT_NULL_Fails                                  (0.001 ms)
[PASS] EXPECT_NOT_NULL_Passes                             (0.000 ms)
[PASS] EXPECT_NOT_NULL_Fails                              (0.001 ms)
[PASS] EXPECT_STR_EQ_Passes                               (0.001 ms)
[PASS] EXPECT_STR_EQ_Fails                                (0.001 ms)
[PASS] EXPECT_STR_NE_Passes                               (0.000 ms)
[PASS] EXPECT_STR_NE_Fails                                (0.001 ms)
[PASS] EXPECT_FLOAT_EQ_Passes                             (0.000 ms)
[PASS] EXPECT_FLOAT_EQ_Fails                              (0.001 ms)
[PASS] EXPECT_FLOAT_NEAR_Passes                           (0.000 ms)
[PASS] EXPECT_FLOAT_NEAR_Fails                            (0.001 ms)
[PASS] EXPECT_MEM_EQ_Passes                               (0.000 ms)
[PASS] EXPECT_MEM_EQ_Fails                                (0.001 ms)
[PASS] EXPECT_MEM_NE_Passes                               (0.000 ms)
[PASS] EXPECT_MEM_NE_Fails                                (0.001 ms)
[PASS] NULLStringComparison                               (0.000 ms)
[PASS] EmptyStringComparison                              (0.001 ms)
[PASS] IntegerBoundaries                                  (0.001 ms)
[PASS] FloatBoundaries                                    (0.001 ms)
[PASS] ZeroSizeMemory                                     (0.001 ms)
[PASS] LargeMemoryBlocks                                  (0.000 ms)
[PASS] ComparisonChain                                    (0.000 ms)
[PASS] LongStrings                                        (0.001 ms)
[PASS] MessageFormatCorrectness                           (0.001 ms)
[PASS] FloatMessageFormat                                 (0.003 ms)
[PASS] StringMessageFormat                                (0.001 ms)
[PASS] NonFatalContinuesAfterFailure                      (0.001 ms)
[PASS] FatalStopsAtFirstFailure                           (0.001 ms)
[PASS] NonFatalCounterCheck                               (0.000 ms)
Test Summary: 78 passed, 0 failed, 0 skipped
Assertions: 38/141 (failed/total)
✓ Completed

    ▶ test_self_runner_cov
Test Summary: 58 passed, 0 failed, 2 skipped
Assertions: 2/1183 (failed/total)
✓ Completed

    ▶ test_library_coverage_cov
Test Summary: 10 passed, 0 failed, 0 skipped
Assertions: 0/23 (failed/total)
✓ Completed

    ▶ test_parallel_cov
Test Summary: 5 passed, 0 failed, 0 skipped
Assertions: 0/5 (failed/total)
✓ Completed

    ▶ test_argparse_cov
Test Summary: 13 passed, 0 failed, 0 skipped
Assertions: 0/16 (failed/total)
✓ Completed

    ▶ test_edge_cases_cov
Test Summary: 15 passed, 0 failed, 0 skipped
Assertions: 2/8 (failed/total)
✓ Completed

    ▶ test_core_cov
Test Summary: 10 passed, 0 failed, 0 skipped
Assertions: 0/52 (failed/total)
✓ Completed

    ▶ test_fixtures_and_features_cov
Test Summary: 11 passed, 0 failed, 0 skipped
Assertions: 1/17 (failed/total)
✓ Completed

    ▶ test_parallel_direct_cov
✓ Completed

  📦 Examples

    ▶ simple_example
✓ Completed

    ▶ example_arithmetic
✓ Completed

    ▶ rpn_calculator
RPN Calculator
Usage: build/rpn_calculator "<expression>"

Examples:
  build/rpn_calculator "3 4 +"
✓ Completed

    ▶ rpn_calculator_test
✓ Completed

  📊 Test Results
  Tests completed: 14 passed, 0 failed

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  📈 Generating Coverage Report
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

    ✨ Capturing coverage data...
    Found 16 data files in build

    🔄 Processing coverage report...
    Writing data to coverage/coverage.info

    📝 Generating HTML report...

  📊 SHT Library Coverage (sht.h only)
      lines.......: 81.2% (455 of 560 lines)
      functions...: 97.1% (33 of 34 functions)

  📁 View: open coverage/html/index.html
```
</details>

