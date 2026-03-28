# SHT - Simple Header-only Testing Framework

A fully-featured, single-header C testing framework equivalent to PyTest for Python.

## Features

- **Single Header**: Just `#include "sht.h"` and go
- **Auto Test Discovery**: Tests register automatically via compiler constructor attributes
- **Rich Assertions**: Both fatal (ASSERT_*) and non-fatal (EXPECT_*) variants
- **Mocking Framework**: Complete mocking system in `sht_mock.h`
- **Coverage Tracking**: Line, function, and branch coverage in `sht_coverage.h`
- **Memory Leak Detection**: Built-in tracking for test allocations
- **Colored Output**: Beautiful terminal output with ANSI/Winterm colors
- **Fixtures**: Setup/teardown support with TEST_F macro
- **Cross-Platform**: Linux, macOS, Windows (GCC/Clang/MSVC)

## Quick Start

Create `my_tests.c`:
```c
#define SHT_IMPLEMENTATION
#include "sht.h"

TEST(Math, Addition) {
    EXPECT_EQ(2 + 2, 4);
}

TEST(Math, Subtraction) {
    EXPECT_NE(5 - 3, 3);
}

TEST_RUN_MAIN();
```

Compile and run:
```bash
gcc my_tests.c -o my_tests -I.
./my_tests
```

## Assertions

### Comparisons
- `ASSERT_EQ(actual, expected)` - Equality (fatal)
- `ASSERT_NE(actual, expected)` - Inequality (fatal)
- `ASSERT_LT/LE/GT/GE(actual, expected)` - Ordering (fatal)
- `EXPECT_*` - Non-fatal counterparts

### Boolean/Pointer
- `ASSERT_TRUE(value)` / `ASSERT_FALSE(value)`
- `ASSERT_NULL(ptr)` / `ASSERT_NOT_NULL(ptr)`

### Strings
- `ASSERT_STR_EQ(actual, expected)` - String equality
- `ASSERT_STR_NE(actual, expected)` - String inequality

### Floating Point
- `ASSERT_FLOAT_EQ(actual, expected)` - Default tolerance
- `ASSERT_FLOAT_NEAR(actual, expected, tolerance)` - Custom tolerance

### Memory
- `ASSERT_MEM_EQ(actual, expected, size)` - Block equality
- `ASSERT_MEM_NE(actual, expected, size)` - Block inequality

## Fixtures

```c
SET_UP(MyFixture) {
    // Setup code
}

TEAR_DOWN(MyFixture) {
    // Cleanup code
}

TEST_F(MyFixture, FirstTest) {
    // Setup runs before, teardown runs after
}
```

## Mocking

```c
#include "sht_mock.h"

int calculate(int x) {
    return mock_doubles(x);
}

MockFunction(mock_doubles);

void test_calculator(void) {
    MOCK_FUNCTION_DECLARE(mock_doubles, int, (int x), return x * 2);
    
    EXPECT_CALL(mock_doubles, ANY, 2, int);
    
    EXPECT_EQ(calculate(3), 6);
    
    VERIFY_MOCKS();
}
```

## Command-Line Options

- `-v, --verbose` - Verbose output
- `-n, --no-color` - Disable colors
- `-h, --help` - Show help

## Project Structure

```
sht_v2/
├── sht.h              # Main testing framework
├── sht_mock.h         # Mocking framework
├── sht_coverage.h     # Coverage tracking
├── examples/          # Example programs
├── tests/             # Self-tests
└── README.md          # This file
```

## Running Tests

```bash
make all              # Build everything
make test             # Run self-tests
make examples         # Build examples
make clean            # Clean build artifacts
```

## Self-Testing

The framework validates itself using its own testing capabilities:
- `tests/test_self.c` - Core system tests
- `tests/test_self_assertions.c` - All assertions tested
- `tests/test_self_runner.c` - Test runner verification

## Backward Compatibility

Old `SHT_*` macro names remain supported:
- `SHT_TEST(suite, test)` → `TEST(suite, test)`
- `SHT_ASSERT_EQ(...)` → `ASSERT_EQ(...)`

## License

MIT License