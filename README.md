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

