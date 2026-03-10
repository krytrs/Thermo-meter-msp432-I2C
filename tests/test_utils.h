/*
 * tests/test_utils.h
 *
 * Minimal test framework used by all test files in this directory.
 *
 * Usage:
 *   TEST_ASSERT(expr)           — abort current test with failure message
 *   RUN_TEST(fn)                — call fn(); track pass/fail count
 *   PRINT_RESULTS()             — print summary and return exit code
 */

#ifndef TEST_UTILS_H_
#define TEST_UTILS_H_

#include <stdio.h>
#include <stdlib.h>

/* Counters — define in exactly one translation unit via TEST_DEFINE_COUNTERS */
extern int g_tests_run;
extern int g_tests_failed;

#define TEST_DEFINE_COUNTERS   int g_tests_run = 0; int g_tests_failed = 0;

/* ------------------------------------------------------------------ *
 *  TEST_ASSERT                                                         *
 * ------------------------------------------------------------------ */
#define TEST_ASSERT(expr)                                              \
    do {                                                               \
        if (!(expr)) {                                                 \
            printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #expr); \
            g_tests_failed++;                                          \
            return;                                                    \
        }                                                              \
    } while (0)

/* ------------------------------------------------------------------ *
 *  RUN_TEST                                                            *
 * ------------------------------------------------------------------ */
#define RUN_TEST(fn)                  \
    do {                              \
        printf("  RUN   " #fn "\n"); \
        g_tests_run++;                \
        fn();                         \
    } while (0)

/* ------------------------------------------------------------------ *
 *  PRINT_RESULTS — print summary; evaluates to an exit-code integer   *
 * ------------------------------------------------------------------ */
#define PRINT_RESULTS()                                                 \
    do {                                                                \
        int passed = g_tests_run - g_tests_failed;                     \
        printf("\n%d/%d tests passed", passed, g_tests_run);           \
        if (g_tests_failed == 0) {                                      \
            printf(" — OK\n");                                          \
        } else {                                                        \
            printf(" — %d FAILED\n", g_tests_failed);                  \
        }                                                               \
    } while (0)

/* Convenience: exit with 0 on success, 1 on any failure */
#define RETURN_TEST_RESULT() return (g_tests_failed == 0) ? 0 : 1

#endif /* TEST_UTILS_H_ */
