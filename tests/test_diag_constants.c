/*
 * tests/test_diag_constants.c
 *
 * Unit tests for the compile-time constants defined in diag.h.
 *
 * diag.h is a pure header (only #define and a function prototype) so
 * it does not need msp.h or any hardware mock.  These tests verify
 * that every status code and configuration constant has the exact
 * numerical value expected by the rest of the firmware.
 */

#include <stdio.h>
#include "test_utils.h"
#include "../diag.h"

TEST_DEFINE_COUNTERS

/* ================================================================== *
 *  Diagnostic status codes                                            *
 * ================================================================== */

static void test_status_ok_is_zero(void)
{
    TEST_ASSERT(DK_DIAG_STATUS_OK == 0U);
}

static void test_status_i2c_error_is_one(void)
{
    TEST_ASSERT(DK_DIAG_STATUS_I2C_E == 1U);
}

static void test_status_lcd_error_is_two(void)
{
    TEST_ASSERT(DK_DIAG_STATUS_LCD_E == 2U);
}

static void test_status_general_error_is_three(void)
{
    TEST_ASSERT(DK_DIAG_STATUS_CHYBA == 3U);
}

static void test_status_mem_error_is_four(void)
{
    TEST_ASSERT(DK_DIAG_STATUS_MEM_E == 4U);
}

static void test_status_reset_is_five(void)
{
    TEST_ASSERT(DK_DIAG_STATUS_RESET == 5U);
}

/* Status codes must be unique */
static void test_status_codes_are_unique(void)
{
    TEST_ASSERT(DK_DIAG_STATUS_OK    != DK_DIAG_STATUS_I2C_E);
    TEST_ASSERT(DK_DIAG_STATUS_OK    != DK_DIAG_STATUS_LCD_E);
    TEST_ASSERT(DK_DIAG_STATUS_OK    != DK_DIAG_STATUS_CHYBA);
    TEST_ASSERT(DK_DIAG_STATUS_OK    != DK_DIAG_STATUS_MEM_E);
    TEST_ASSERT(DK_DIAG_STATUS_OK    != DK_DIAG_STATUS_RESET);
    TEST_ASSERT(DK_DIAG_STATUS_I2C_E != DK_DIAG_STATUS_LCD_E);
    TEST_ASSERT(DK_DIAG_STATUS_I2C_E != DK_DIAG_STATUS_CHYBA);
    TEST_ASSERT(DK_DIAG_STATUS_I2C_E != DK_DIAG_STATUS_MEM_E);
    TEST_ASSERT(DK_DIAG_STATUS_I2C_E != DK_DIAG_STATUS_RESET);
    TEST_ASSERT(DK_DIAG_STATUS_LCD_E != DK_DIAG_STATUS_CHYBA);
    TEST_ASSERT(DK_DIAG_STATUS_LCD_E != DK_DIAG_STATUS_MEM_E);
    TEST_ASSERT(DK_DIAG_STATUS_LCD_E != DK_DIAG_STATUS_RESET);
    TEST_ASSERT(DK_DIAG_STATUS_CHYBA != DK_DIAG_STATUS_MEM_E);
    TEST_ASSERT(DK_DIAG_STATUS_CHYBA != DK_DIAG_STATUS_RESET);
    TEST_ASSERT(DK_DIAG_STATUS_MEM_E != DK_DIAG_STATUS_RESET);
}

/* ================================================================== *
 *  Configuration constants                                            *
 * ================================================================== */

static void test_watchdog_timeout_is_ten_seconds(void)
{
    TEST_ASSERT(DK_DIAG_WTD_TIME == 10U);
}

static void test_sensor_address_is_0x48(void)
{
    /* DS75LVS+ default I2C address with A2-A0 tied to GND */
    TEST_ASSERT(DK_SENSOR_ADDRESS == 0x48U);
}

static void test_i2c_address_mask_covers_10bit_field(void)
{
    /* UCBxADDRX holds up to 10 address bits; the mask must cover all of them */
    TEST_ASSERT(MSK_I2C_ADDR == 0x03FFU);
}

/* Sensor address must fit inside the I2C address mask */
static void test_sensor_address_fits_in_mask(void)
{
    TEST_ASSERT((DK_SENSOR_ADDRESS & MSK_I2C_ADDR) == DK_SENSOR_ADDRESS);
}

/* ================================================================== *
 *  main                                                               *
 * ================================================================== */

int main(void)
{
    printf("=== diag constants tests ===\n");

    /* Status codes */
    RUN_TEST(test_status_ok_is_zero);
    RUN_TEST(test_status_i2c_error_is_one);
    RUN_TEST(test_status_lcd_error_is_two);
    RUN_TEST(test_status_general_error_is_three);
    RUN_TEST(test_status_mem_error_is_four);
    RUN_TEST(test_status_reset_is_five);
    RUN_TEST(test_status_codes_are_unique);

    /* Configuration constants */
    RUN_TEST(test_watchdog_timeout_is_ten_seconds);
    RUN_TEST(test_sensor_address_is_0x48);
    RUN_TEST(test_i2c_address_mask_covers_10bit_field);
    RUN_TEST(test_sensor_address_fits_in_mask);

    PRINT_RESULTS();
    RETURN_TEST_RESULT();
}
