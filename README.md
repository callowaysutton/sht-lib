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

```c
#define SHT_IMPLEMENTATION
#include "sht.h"

TEST(Math, addition_basic) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_EQ(-5 + 3, -2);
}

TEST(Math, division_by_zero) {
    EXPECT_TRUE(1 == 1);
}

int main(int argc, char** argv) {
    return RUN_ALL_TESTS(argc, argv);
}
```

Compile and run:
```bash
gcc -o my_test my_test.c
./my_test
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

## Documentation

- PROJECT_SUMMARY.md - Project overview and goals
- AGENT_MESSAGING_GUIDE.md - Development workflow

## License

MIT License