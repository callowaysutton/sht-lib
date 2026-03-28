#ifndef SHT_MOCK_H
#define SHT_MOCK_H

#include "sht.h"
#include <stdarg.h>

#if defined(__GNUC__) || defined(__clang__)
    #define SHT_INLINE static inline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define SHT_INLINE static __forceinline
#else
    #define SHT_INLINE static inline
#endif

/*==============================================================================
 * COMPREHENSIVE MOCKING FRAMEWORK
 *
 * This framework provides:
 * - Mock registration and management
 * - Function interception and call tracking
 * - Argument matching with various matchers
 * - Return value management and sequences
 * - Expectation validation and verification
 * - Side effects and callbacks
 * - Advanced features like partial mocking
==============================================================================*/

#define SHT_MAX_MOCKS 500
#define SHT_MAX_EXPECTATIONS 2000
#define SHT_MAX_ARGS 16
#define SHT_MAX_CALLS 10000
#define SHT_MAX_RETURN_SEQ 100

/*==============================================================================
 * CORE DATA STRUCTURES
==============================================================================*/

typedef enum {
    SHT_MOCK_MATCH_EXACT,
    SHT_MOCK_MATCH_ANY,
    SHT_MOCK_MATCH_GT,
    SHT_MOCK_MATCH_LT,
    SHT_MOCK_MATCH_GE,
    SHT_MOCK_MATCH_LE,
    SHT_MOCK_MATCH_NE,
    SHT_MOCK_MATCH_STR,
    SHT_MOCK_MATCH_STR_CONTAINS,
    SHT_MOCK_MATCH_IN_RANGE,
    SHT_MOCK_MATCH_CUSTOM
} sht_mock_matcher_type_t;

typedef int (*sht_custom_matcher_fn)(const void* actual);

typedef struct sht_mock_matcher {
    sht_mock_matcher_type_t type;
    union {
        long long as_int;
        double as_float;
        const char* as_string;
        struct { long long min; long long max; } as_range;
        sht_custom_matcher_fn as_custom;
    } expected;
    char name[256];
} sht_mock_matcher_t;

typedef struct sht_mock_call {
    long long args[SHT_MAX_ARGS];
    size_t arg_count;
    const char* file;
    int line;
} sht_mock_call_t;

typedef struct sht_mock_expectation {
    sht_mock_matcher_t matchers[SHT_MAX_ARGS];
    size_t matcher_count;
    long long return_value_raw;
    void* return_value;
    void** return_sequence;
    size_t return_sequence_size;
    size_t return_sequence_index;
    void (*side_effect)(void* args, void* return_value);
    int min_calls;
    int max_calls;
    int actual_calls;
    int is_default;
    int active;
    char caller_file[256];
    int caller_line;
    char description[512];
} sht_mock_expectation_t;

typedef struct sht_mock {
    const char* function_name;
    const char* return_type;
    void* original_function;
    sht_mock_call_t calls[SHT_MAX_CALLS];
    int call_count;
    sht_mock_expectation_t* expectations[SHT_MAX_EXPECTATIONS];
    int expectation_count;
    sht_mock_expectation_t* default_expectation;
    size_t args_size;
    int enabled;
    int active;
    int partial_mock;
} sht_mock_t;

/*==============================================================================
 * MOCK CONTEXT MANAGEMENT
==============================================================================*/

typedef struct {
    sht_mock_t* mocks[SHT_MAX_MOCKS];
    sht_mock_expectation_t expectation_pool[SHT_MAX_EXPECTATIONS];
    int mock_count;
    int expectation_pool_index;
    int verbose_expectation_errors;
} sht_mock_context_t;

static sht_mock_context_t g_sht_mock_ctx = {0};

/*==============================================================================
 * MOCK CORE IMPLEMENTATION
==============================================================================*/

SHT_INLINE sht_mock_expectation_t* sht_mock_alloc_expectation(void) {
    if (g_sht_mock_ctx.expectation_pool_index >= SHT_MAX_EXPECTATIONS) {
        fprintf(stderr, "SHT Mock: Maximum expectations exceeded\n");
        return NULL;
    }
    sht_mock_expectation_t* exp = &g_sht_mock_ctx.expectation_pool[g_sht_mock_ctx.expectation_pool_index++];
    memset(exp, 0, sizeof(sht_mock_expectation_t));
    return exp;
}

SHT_INLINE void sht_mock_free_expectation(sht_mock_expectation_t* exp) {
    if (!exp) return;
    
    /* Only free return_value if it's not pointing to return_value_raw */
    if (exp->return_value && exp->return_value != &exp->return_value_raw) {
        free(exp->return_value);
    }
    exp->return_value = NULL;
    
    if (exp->return_sequence) {
        for (size_t i = 0; i < exp->return_sequence_size; i++) {
            free(exp->return_sequence[i]);
        }
        free(exp->return_sequence);
        exp->return_sequence = NULL;
        exp->return_sequence_size = 0;
    }
}

SHT_INLINE sht_mock_t* sht_mock_find_mock(const char* function_name) {
    for (int i = 0; i < g_sht_mock_ctx.mock_count; i++) {
        if (strcmp(g_sht_mock_ctx.mocks[i]->function_name, function_name) == 0) {
            return g_sht_mock_ctx.mocks[i];
        }
    }
    return NULL;
}

SHT_INLINE sht_mock_t* sht_mock_create_mock(const char* function_name, const char* return_type) {
    if (g_sht_mock_ctx.mock_count >= SHT_MAX_MOCKS) {
        fprintf(stderr, "SHT Mock: Maximum mocks exceeded\n");
        return NULL;
    }
    
    sht_mock_t* mock = (sht_mock_t*)calloc(1, sizeof(sht_mock_t));
    if (!mock) return NULL;
    
    mock->function_name = function_name;
    mock->return_type = return_type;
    mock->call_count = 0;
    mock->expectation_count = 0;
    mock->enabled = 1;
    mock->active = 1;
    mock->partial_mock = 0;
    
    g_sht_mock_ctx.mocks[g_sht_mock_ctx.mock_count++] = mock;
    return mock;
}

SHT_INLINE void sht_mock_record_call(sht_mock_t* mock, long long* args, size_t arg_count, const char* file, int line) {
    if (!mock || mock->call_count >= SHT_MAX_CALLS) return;
    
    sht_mock_call_t* call = &mock->calls[mock->call_count];
    call->arg_count = arg_count;
    for (size_t i = 0; i < arg_count && i < SHT_MAX_ARGS; i++) {
        call->args[i] = args[i];
    }
    call->file = file;
    call->line = line;
    
    mock->call_count++;
}

SHT_INLINE int sht_mock_match_argument(const sht_mock_matcher_t* matcher, long long actual_value) {
    switch (matcher->type) {
        case SHT_MOCK_MATCH_ANY:
            return 1;
        case SHT_MOCK_MATCH_EXACT:
            return (actual_value == matcher->expected.as_int);
        case SHT_MOCK_MATCH_GT:
            return (actual_value > matcher->expected.as_int);
        case SHT_MOCK_MATCH_LT:
            return (actual_value < matcher->expected.as_int);
        case SHT_MOCK_MATCH_GE:
            return (actual_value >= matcher->expected.as_int);
        case SHT_MOCK_MATCH_LE:
            return (actual_value <= matcher->expected.as_int);
        case SHT_MOCK_MATCH_NE:
            return (actual_value != matcher->expected.as_int);
        case SHT_MOCK_MATCH_IN_RANGE:
            return (actual_value >= matcher->expected.as_range.min &&
                    actual_value <= matcher->expected.as_range.max);
        case SHT_MOCK_MATCH_CUSTOM:
            return matcher->expected.as_custom((const void*)actual_value);
        default:
            return 0;
    }
}

SHT_INLINE sht_mock_expectation_t* sht_mock_find_matching_expectation(sht_mock_t* mock, long long* args, size_t arg_count) {
    for (int i = 0; i < mock->expectation_count; i++) {
        sht_mock_expectation_t* exp = mock->expectations[i];
        if (!exp->active || exp->is_default) continue;
        
        int matches = 1;
        for (size_t j = 0; j < exp->matcher_count && j < arg_count; j++) {
            if (!sht_mock_match_argument(&exp->matchers[j], args[j])) {
                matches = 0;
                break;
            }
        }
        
        if (matches) {
            exp->actual_calls++;
            return exp;
        }
    }
    
    if (mock->default_expectation) {
        mock->default_expectation->actual_calls++;
    }
    
    return mock->default_expectation;
}

SHT_INLINE void* sht_mock_get_return_value(sht_mock_expectation_t* exp) {
    if (!exp) return NULL;
    
    if (exp->return_sequence && exp->return_sequence_size > 0) {
        if (exp->return_sequence_index < exp->return_sequence_size) {
            return exp->return_sequence[exp->return_sequence_index++];
        }
    }
    
    return exp->return_value;
}

SHT_INLINE void sht_mock_execute_side_effect(sht_mock_expectation_t* exp, void* args, void* return_value) {
    if (exp && exp->side_effect) {
        exp->side_effect(args, return_value);
    }
}

SHT_INLINE int sht_mock_verify_expectations(void) {
    int all_passed = 1;
    
    for (int i = 0; i < g_sht_mock_ctx.mock_count; i++) {
        sht_mock_t* mock = g_sht_mock_ctx.mocks[i];
        
        for (int j = 0; j < mock->expectation_count; j++) {
            sht_mock_expectation_t* exp = mock->expectations[j];
            if (!exp->active || exp->is_default) continue;
            
            if (exp->actual_calls < exp->min_calls) {
                if (g_sht_mock_ctx.verbose_expectation_errors || exp->description[0]) {
                    fprintf(stderr, "  ✗ Expected %s to be called at least %d time(s), but was called %d time(s) %s(set at %s:%d)\n",
                           mock->function_name, exp->min_calls, exp->actual_calls,
                           exp->description[0] ? "[" : "",
                           exp->caller_file, exp->caller_line);
                    if (exp->description[0]) {
                        fprintf(stderr, "    Description: %s\n", exp->description);
                    }
                } else {
                    fprintf(stderr, "  ✗ Expected %s to be called at least %d time(s), but was called %d time(s)\n",
                           mock->function_name, exp->min_calls, exp->actual_calls);
                }
                all_passed = 0;
            } else if (exp->max_calls >= 0 && exp->actual_calls > exp->max_calls) {
                fprintf(stderr, "  ✗ Expected %s to be called at most %d time(s), but was called %d time(s) (set at %s:%d)\n",
                       mock->function_name, exp->max_calls, exp->actual_calls,
                       exp->caller_file, exp->caller_line);
                all_passed = 0;
            }
        }
    }
    
    return all_passed;
}

SHT_INLINE void sht_mock_reset_expectations(void) {
    for (int i = 0; i < g_sht_mock_ctx.mock_count; i++) {
        sht_mock_t* mock = g_sht_mock_ctx.mocks[i];
        mock->call_count = 0;
        
        for (int j = 0; j < mock->expectation_count; j++) {
            sht_mock_free_expectation(mock->expectations[j]);
        }
        mock->expectation_count = 0;
        
        if (mock->default_expectation) {
            sht_mock_free_expectation(mock->default_expectation);
            mock->default_expectation = NULL;
        }
    }
    
    g_sht_mock_ctx.expectation_pool_index = 0;
}

SHT_INLINE void sht_mock_cleanup_all(void) {
    sht_mock_reset_expectations();
    
    for (int i = 0; i < g_sht_mock_ctx.mock_count; i++) {
        free(g_sht_mock_ctx.mocks[i]);
    }
    g_sht_mock_ctx.mock_count = 0;
}

/*==============================================================================
 * MOCKING MACROS - NO ARGUMENTS
==============================================================================*/

#define MOCK_FUNCTION(ret_type, func_name) \
    typedef ret_type (*func_name##_fn_t)(void); \
    static func_name##_fn_t func_name##_real __attribute__((unused)) = NULL; \
    static sht_mock_t* func_name##_mock_obj = NULL; \
    static sht_mock_expectation_t* func_name##_last_expectation = NULL; \
    \
    SHT_INLINE ret_type func_name(void) { \
        if (!func_name##_mock_obj) { \
            func_name##_mock_obj = sht_mock_create_mock(#func_name, #ret_type); \
        } \
        \
        if (!func_name##_mock_obj->active) { \
            return (ret_type)0; \
        } \
        \
        sht_mock_record_call(func_name##_mock_obj, NULL, 0, __FILE__, __LINE__); \
        \
        sht_mock_expectation_t* exp = sht_mock_find_matching_expectation(func_name##_mock_obj, NULL, 0); \
        \
        ret_type ret = (ret_type)0; \
        void* ret_ptr = sht_mock_get_return_value(exp); \
        if (ret_ptr) { \
            memcpy(&ret, ret_ptr, sizeof(ret_type)); \
        } \
        \
        sht_mock_execute_side_effect(exp, NULL, &ret); \
        return ret; \
    }

#define MOCK_FUNCTION1(ret_type, func_name, arg1_type) \
    MOCK_FUNCTIONARGS(ret_type, func_name, arg1_type, 1)

#define MOCK_FUNCTION2(ret_type, func_name, arg1_type, arg2_type) \
    MOCK_FUNCTIONARGS(ret_type, func_name, arg1_type, 2)

#define MOCK_FUNCTION3(ret_type, func_name, arg1_type, arg2_type, arg3_type) \
    MOCK_FUNCTIONARGS(ret_type, func_name, arg1_type, 3)

#define MOCK_FUNCTION4(ret_type, func_name, arg1_type, arg2_type, arg3_type, arg4_type) \
    MOCK_FUNCTIONARGS(ret_type, func_name, arg1_type, 4)

/*==============================================================================
 * MOCKING MACROS - WITH ARGUMENTS
==============================================================================*/

#define MOCK_FUNCTIONARGS(ret_type, func_name, first_arg_type, arg_count) \
    typedef ret_type (*func_name##_fn_t)(first_arg_type, ...); \
    static sht_mock_t* func_name##_mock_obj = NULL; \
    static sht_mock_expectation_t* func_name##_last_expectation = NULL; \
    \
    SHT_INLINE ret_type func_name(first_arg_type a1, ...) { \
        if (!func_name##_mock_obj) { \
            func_name##_mock_obj = sht_mock_create_mock(#func_name, #ret_type); \
        } \
        \
        if (!func_name##_mock_obj->active) { \
            return (ret_type)0; \
        } \
        \
        long long args[arg_count]; \
        args[0] = (long long)a1; \
        if (arg_count > 1) { \
            va_list __va_args; \
            va_start(__va_args, a1); \
            for (int i = 1; i < arg_count; i++) { \
                args[i] = va_arg(__va_args, long long); \
            } \
            va_end(__va_args); \
        } \
        \
        sht_mock_record_call(func_name##_mock_obj, args, arg_count, __FILE__, __LINE__); \
        \
        sht_mock_expectation_t* exp = sht_mock_find_matching_expectation(func_name##_mock_obj, args, arg_count); \
        \
        ret_type ret = (ret_type)0; \
        void* ret_ptr = sht_mock_get_return_value(exp); \
        if (ret_ptr) { \
            memcpy(&ret, ret_ptr, sizeof(ret_type)); \
        } \
        \
        sht_mock_execute_side_effect(exp, args, &ret); \
        return ret; \
    }

/*==============================================================================
 * EXPECTATION MACROS
==============================================================================*/

#define EXPECT_CALL(func_name) \
    do { \
        if (!func_name##_mock_obj) { \
            func_name##_mock_obj = sht_mock_create_mock(#func_name, "int"); \
        } \
        \
        sht_mock_expectation_t* exp = sht_mock_alloc_expectation(); \
        if (!exp) break; \
        \
        exp->min_calls = 1; \
        exp->max_calls = 1; \
        exp->actual_calls = 0; \
        exp->is_default = 0; \
        exp->active = 1; \
        exp->side_effect = NULL; \
        exp->return_sequence = NULL; \
        exp->return_sequence_size = 0; \
        exp->return_sequence_index = 0; \
        exp->description[0] = '\0'; \
        strncpy(exp->caller_file, __FILE__, 255); \
        exp->caller_line = __LINE__; \
        \
        if (func_name##_mock_obj->expectation_count < SHT_MAX_EXPECTATIONS) { \
            func_name##_mock_obj->expectations[func_name##_mock_obj->expectation_count++] = exp; \
        } \
        \
        func_name##_last_expectation = exp; \
    } while (0)

#define EXPECT_CALL_ARGS(func_name, ...) \
    do { \
        if (!func_name##_mock_obj) { \
            func_name##_mock_obj = sht_mock_create_mock(#func_name, "int"); \
        } \
        \
        sht_mock_expectation_t* exp = sht_mock_alloc_expectation(); \
        if (!exp) break; \
        \
        exp->min_calls = 1; \
        exp->max_calls = 1; \
        exp->actual_calls = 0; \
        exp->is_default = 0; \
        exp->active = 1; \
        strncpy(exp->caller_file, __FILE__, 255); \
        exp->caller_line = __LINE__; \
        exp->description[0] = '\0'; \
        \
        long long args[] = { __VA_ARGS__ }; \
        exp->matcher_count = sizeof(args) / sizeof(args[0]); \
        for (size_t i = 0; i < exp->matcher_count && i < SHT_MAX_ARGS; i++) { \
            exp->matchers[i].type = SHT_MOCK_MATCH_EXACT; \
            exp->matchers[i].expected.as_int = args[i]; \
            snprintf(exp->matchers[i].name, 255, "%lld", args[i]); \
        } \
        \
        if (func_name##_mock_obj->expectation_count < SHT_MAX_EXPECTATIONS) { \
            func_name##_mock_obj->expectations[func_name##_mock_obj->expectation_count++] = exp; \
        } \
        \
        func_name##_last_expectation = exp; \
    } while (0)

#define EXPECT_CALL_WITH_MATCHERS(func_name, ...) \
    do { \
        if (!func_name##_mock_obj) { \
            func_name##_mock_obj = sht_mock_create_mock(#func_name, "int"); \
        } \
        \
        sht_mock_expectation_t* exp = sht_mock_alloc_expectation(); \
        if (!exp) break; \
        \
        exp->min_calls = 1; \
        exp->max_calls = 1; \
        exp->actual_calls = 0; \
        exp->is_default = 0; \
        exp->active = 1; \
        strncpy(exp->caller_file, __FILE__, 255); \
        exp->caller_line = __LINE__; \
        exp->description[0] = '\0'; \
        \
        sht_mock_matcher_t matchers[] = { __VA_ARGS__ }; \
        exp->matcher_count = sizeof(matchers) / sizeof(matchers[0]); \
        for (size_t i = 0; i < exp->matcher_count && i < SHT_MAX_ARGS; i++) { \
            exp->matchers[i] = matchers[i]; \
        } \
        \
        if (func_name##_mock_obj->expectation_count < SHT_MAX_EXPECTATIONS) { \
            func_name##_mock_obj->expectations[func_name##_mock_obj->expectation_count++] = exp; \
        } \
        \
        func_name##_last_expectation = exp; \
    } while (0)

#define ON_CALL(func_name) \
    do { \
        if (!func_name##_mock_obj) { \
            func_name##_mock_obj = sht_mock_create_mock(#func_name, "int"); \
        } \
        \
        sht_mock_expectation_t* exp = sht_mock_alloc_expectation(); \
        if (!exp) break; \
        \
        exp->is_default = 1; \
        exp->active = 1; \
        exp->min_calls = -1; \
        exp->max_calls = -1; \
        exp->actual_calls = 0; \
        exp->side_effect = NULL; \
        exp->description[0] = '\0'; \
        strncpy(exp->caller_file, __FILE__, 255); \
        exp->caller_line = __LINE__; \
        \
        func_name##_mock_obj->default_expectation = exp; \
        \
        func_name##_last_expectation = exp; \
    } while (0)

/*==============================================================================
 * RETURN VALUE MACROS
==============================================================================*/

#define WILL_RETURN(func_name, value) \
    do { \
        if (func_name##_last_expectation) { \
            if (func_name##_last_expectation->return_value) { \
                free(func_name##_last_expectation->return_value); \
            } \
            func_name##_last_expectation->return_value_raw = (long long)(value); \
            func_name##_last_expectation->return_value = &func_name##_last_expectation->return_value_raw; \
        } \
    } while (0)

#define WILL_RETURN_SEQ(func_name, ...) \
    do { \
        if (func_name##_last_expectation) { \
            long long _vals[] = { __VA_ARGS__ }; \
            size_t count = sizeof(_vals) / sizeof(_vals[0]); \
            \
            if (func_name##_last_expectation->return_sequence) { \
                free(func_name##_last_expectation->return_sequence); \
            } \
            \
            func_name##_last_expectation->return_sequence = malloc(count * sizeof(long long*)); \
            func_name##_last_expectation->return_sequence_size = count; \
            func_name##_last_expectation->return_sequence_index = 0; \
            \
            for (size_t i = 0; i < count; i++) { \
                long long* val_ptr = malloc(sizeof(long long)); \
                *val_ptr = _vals[i]; \
                func_name##_last_expectation->return_sequence[i] = val_ptr; \
            } \
        } \
    } while (0)

/*==============================================================================
 * CALL COUNT MACROS
==============================================================================*/

#define EXPECTED_CALLS_IMPL(func_name, min, max) \
    do { \
        if (func_name##_last_expectation) { \
            func_name##_last_expectation->min_calls = (min); \
            func_name##_last_expectation->max_calls = (max); \
        } \
    } while (0)

#define EXPECTED_CALLS(min, max) \
    do { \
        fprintf(stderr, "ERROR: EXPECTED_CALLS must be used with EXPECTED_CALL_ONCE() or EXPECTED_CALL_AT_LEAST()\n"); \
    } while (0)

#define EXPECTED_CALL_ONCE() EXPECTED_CALLS(1, 1)
#define EXPECTED_CALL_AT_LEAST(times) EXPECTED_CALLS(times, -1)
#define EXPECTED_CALL_EXACTLY(times) EXPECTED_CALLS(times, times)

/*==============================================================================
 * SIDE EFFECT MACROS
==============================================================================*/

#define WITH_SIDE_EFFECT(func_name, fn) \
    do { \
        if (func_name##_last_expectation) { \
            func_name##_last_expectation->side_effect = (void(*)(void*, void*))fn; \
        } \
    } while (0)

/*==============================================================================
 * ARGUMENT MATCHERS
==============================================================================*/

#define MATCHER_ANY \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_ANY, .name = "ANY" })

#define MATCHER_GT(val) \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_GT, .expected.as_int = (val), .name = "GT(" #val ")" })

#define MATCHER_LT(val) \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_LT, .expected.as_int = (val), .name = "LT(" #val ")" })

#define MATCHER_GE(val) \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_GE, .expected.as_int = (val), .name = "GE(" #val ")" })

#define MATCHER_LE(val) \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_LE, .expected.as_int = (val), .name = "LE(" #val ")" })

#define MATCHER_NE(val) \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_NE, .expected.as_int = (val), .name = "NE(" #val ")" })

#define MATCHER_IN_RANGE(min, max) \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_IN_RANGE, .expected.as_range.min = (min), .expected.as_range.max = (max), .name = "IN_RANGE(" #min ", " #max ")" })

#define MATCHER_CUSTOM(fn) \
    ((sht_mock_matcher_t){ .type = SHT_MOCK_MATCH_CUSTOM, .expected.as_custom = (fn), .name = "CUSTOM" })

#define _ MATCHER_ANY
#define GT(val) MATCHER_GT(val)
#define LT(val) MATCHER_LT(val)
#define GE(val) MATCHER_GE(val)
#define LE(val) MATCHER_LE(val)
#define NE(val) MATCHER_NE(val)
#define IN_RANGE(min, max) MATCHER_IN_RANGE(min, max)
#define CUSTOM(fn) MATCHER_CUSTOM(fn)

/*==============================================================================
 * VERIFICATION MACROS
==============================================================================*/

#define VERIFY_MOCKS() sht_mock_verify_expectations()
#define RESET_MOCKS() sht_mock_reset_expectations()
#define MOCK_CLEANUP() sht_mock_cleanup_all()

#define ASSERT_MOCK_CALLED(func_name) \
    do { \
        if (func_name##_mock_obj) { \
            ASSERT_TRUE(func_name##_mock_obj->call_count > 0); \
        } \
    } while (0)

#define ASSERT_MOCK_CALL_COUNT(func_name, count) \
    do { \
        if (func_name##_mock_obj) { \
            ASSERT_EQ(count, func_name##_mock_obj->call_count); \
        } \
    } while (0)

#define GET_MOCK_CALL_COUNT(func_name) \
    (func_name##_mock_obj ? func_name##_mock_obj->call_count : 0)

#define GET_MOCK_WAS_CALLED(func_name) \
    (func_name##_mock_obj ? func_name##_mock_obj->call_count > 0 : 0)

/*==============================================================================
 * MOCK CONTROL MACROS
==============================================================================*/

#define ENABLE_MOCK(func_name) \
    do { \
        if (func_name##_mock_obj) func_name##_mock_obj->active = 1; \
    } while (0)

#define DISABLE_MOCK(func_name) \
    do { \
        if (func_name##_mock_obj) func_name##_mock_obj->active = 0; \
    } while (0)

/*==============================================================================
 * DESCRIPTION MACRO
==============================================================================*/

#define EXPECTATION_DESC(desc) \
    do { \
        if (func_name##_last_expectation) { \
            snprintf(func_name##_last_expectation->description, 511, "%s", desc); \
        } \
    } while (0)

/*==============================================================================
 * VERBOSE ERROR REPORTING
==============================================================================*/

#define SHT_MOCK_SET_VERBOSE_ERRORS(verbose) \
    do { \
        g_sht_mock_ctx.verbose_expectation_errors = (verbose); \
    } while (0)

/*==============================================================================
 * INTEGRATION WITH TEST RUNNER
==============================================================================*/

SHT_INLINE void sht_mock_integration_runner(void) {
    if (!sht_mock_verify_expectations()) {
        if (g_sht_context) {
            strncpy(g_sht_context->current_message, "Mock expectations failed", 
                    SHT_MAX_MESSAGE_LENGTH - 1);
            g_sht_context->current_line = __LINE__;
            g_sht_context->current_file = __FILE__;
            longjmp(g_sht_context->jump_buffer, 1);
        }
    }
}

#define SHT_MOCK_VERIFY_INTEGRATION() sht_mock_integration_runner()

/*==============================================================================
 * MOCK STATISTICS
==============================================================================*/

SHT_INLINE int sht_mock_get_total_mock_count(void) {
    return g_sht_mock_ctx.mock_count;
}

SHT_INLINE int sht_mock_get_total_expectation_count(void) {
    int total = 0;
    for (int i = 0; i < g_sht_mock_ctx.mock_count; i++) {
        total += g_sht_mock_ctx.mocks[i]->expectation_count;
        if (g_sht_mock_ctx.mocks[i]->default_expectation) {
            total++;
        }
    }
    return total;
}

SHT_INLINE int sht_mock_get_total_call_count(void) {
    int total = 0;
    for (int i = 0; i < g_sht_mock_ctx.mock_count; i++) {
        total += g_sht_mock_ctx.mocks[i]->call_count;
    }
    return total;
}

#endif /* SHT_MOCK_H */