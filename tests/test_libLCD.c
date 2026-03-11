/*
 * tests/test_libLCD.c
 *
 * Unit tests for the LCD display driver (libLCD.c / libLCD.h).
 *
 * How the LCD is tested on the host
 * -----------------------------------
 * The LCD hardware is a 4-bit parallel interface driven through
 * port P4.  The only way to observe what the driver sends to the
 * display is to monitor P4OUT.
 *
 * The driver pulses the EN pin (P4.6) once per 4-bit nibble inside
 * LCD_pulseEnablePin():
 *
 *   P4OUT |= EN;       <- EN goes high, nibble + RS already on bus
 *   delay_ms(1U);      <- __wfi() -> mock_tick() [see below]
 *   P4OUT &= ~EN;      <- EN goes low
 *   delay_ms(1U);
 *
 * mock_tick() (strong override below) checks whether EN is high.
 * When it is, it records the nibble (P4OUT[3:0]) and the RS bit
 * (P4OUT[4]) into a capture buffer, then calls TA0_ISR_Handler()
 * to advance the internal tick counter so that delay_ms() terminates.
 *
 * After each test the capture buffer is decoded: pairs of nibbles
 * are reassembled into full bytes, and the RS flag indicates whether
 * the byte was a command (RS=0) or character data (RS=1).
 *
 * Pin assignments (from libLCD.h):
 *   D4-D7  = P4.0-P4.3  (bits 0-3)  LCD_MASK_DATA = 0x0F
 *   RS     = P4.4        (bit 4)     LCD_PIN_RS    = 0x10
 *   RW     = P4.5        (bit 5)     LCD_PIN_RW    = 0x20
 *   EN     = P4.6        (bit 6)     LCD_PIN_EN    = 0x40
 */

#include <stdio.h>
#include <string.h>
#include "test_utils.h"
#include "msp.h"
#include "../libLCD.h"

TEST_DEFINE_COUNTERS

/* ------------------------------------------------------------------ *
 *  Nibble capture buffer                                              *
 * ------------------------------------------------------------------ */
#define NIBBLE_BUF_SIZE  256

static uint8_t g_nibbles[NIBBLE_BUF_SIZE];
static uint8_t g_is_data[NIBBLE_BUF_SIZE];  /* 1 = data (RS=1), 0 = command */
static int     g_nibble_count = 0;

static void capture_reset(void)
{
    g_nibble_count = 0;
    /* Also clear register state that persists across calls */
    P4OUT = 0U;
    P4DIR = 0U;
}

/* ------------------------------------------------------------------ *
 *  strong mock_tick — overrides the weak no-op in msp_mock.c         *
 * ------------------------------------------------------------------ */

/* Forward declaration: TA0_ISR_Handler is defined in libLCD.c */
void TA0_ISR_Handler(void);

void mock_tick(void)
{
    /* Capture nibble when EN (P4.6 = 0x40) is high */
    if ((P4OUT & 0x40U) != 0U)
    {
        if (g_nibble_count < NIBBLE_BUF_SIZE)
        {
            g_nibbles[g_nibble_count] = P4OUT & 0x0FU;           /* D4-D7 nibble  */
            g_is_data[g_nibble_count] = (P4OUT & 0x10U) ? 1U : 0U; /* RS state      */
            g_nibble_count++;
        }
    }
    /* Advance the delay_ms() internal counter */
    TA0_ISR_Handler();
}

/* ------------------------------------------------------------------ *
 *  Decode helper: reassemble two consecutive nibbles into one byte    *
 * ------------------------------------------------------------------ */
static uint8_t nibbles_to_byte(int idx)
{
    return (uint8_t)(((uint8_t)(g_nibbles[idx] & 0x0FU) << 4U) |
                      (uint8_t)(g_nibbles[idx + 1] & 0x0FU));
}

/* ================================================================== *
 *  LCD_setCursorPosition                                              *
 * ================================================================== */

/*
 * Expected command encoding (from libLCD.c):
 *   row 0: command = 0x80 | col
 *   row 1: command = 0x80 | 0x40 | col  (= 0xC0 | col)
 */

static void test_LCD_setCursorPosition_row0_col0(void)
{
    capture_reset();
    LCD_setCursorPosition(0U, 0U);
    TEST_ASSERT(g_nibble_count >= 2);
    TEST_ASSERT(g_is_data[0] == 0U);              /* RS=0 → command */
    TEST_ASSERT(nibbles_to_byte(0) == 0x80U);
}

static void test_LCD_setCursorPosition_row0_col5(void)
{
    capture_reset();
    LCD_setCursorPosition(0U, 5U);
    TEST_ASSERT(g_nibble_count >= 2);
    TEST_ASSERT(g_is_data[0] == 0U);
    TEST_ASSERT(nibbles_to_byte(0) == 0x85U);
}

static void test_LCD_setCursorPosition_row1_col0(void)
{
    capture_reset();
    LCD_setCursorPosition(1U, 0U);
    TEST_ASSERT(g_nibble_count >= 2);
    TEST_ASSERT(g_is_data[0] == 0U);
    TEST_ASSERT(nibbles_to_byte(0) == 0xC0U);
}

static void test_LCD_setCursorPosition_row1_col15(void)
{
    capture_reset();
    LCD_setCursorPosition(1U, 15U);
    TEST_ASSERT(g_nibble_count >= 2);
    TEST_ASSERT(g_is_data[0] == 0U);
    TEST_ASSERT(nibbles_to_byte(0) == 0xCFU);
}

static void test_LCD_setCursorPosition_row0_col15(void)
{
    capture_reset();
    LCD_setCursorPosition(0U, 15U);
    TEST_ASSERT(g_nibble_count >= 2);
    TEST_ASSERT(g_is_data[0] == 0U);
    TEST_ASSERT(nibbles_to_byte(0) == 0x8FU);
}

/* ================================================================== *
 *  LCD_clearScreen                                                    *
 * ================================================================== */

/*
 * LCD_clearScreen sends two commands:
 *   CLEAR_DISP_CMD = 0x01
 *   RET_HOME_CMD   = 0x02
 * → 4 nibbles total, all with RS=0.
 */
static void test_LCD_clearScreen_sends_correct_commands(void)
{
    capture_reset();
    LCD_clearScreen();
    TEST_ASSERT(g_nibble_count == 4);
    TEST_ASSERT(g_is_data[0] == 0U);
    TEST_ASSERT(g_is_data[2] == 0U);
    TEST_ASSERT(nibbles_to_byte(0) == 0x01U);   /* CLEAR_DISP_CMD */
    TEST_ASSERT(nibbles_to_byte(2) == 0x02U);   /* RET_HOME_CMD   */
}

/* ================================================================== *
 *  LCD_printChar                                                      *
 * ================================================================== */

static void test_LCD_printChar_sends_character_as_data(void)
{
    capture_reset();
    LCD_printChar('A');            /* 'A' = 0x41 */
    TEST_ASSERT(g_nibble_count == 2);
    TEST_ASSERT(g_is_data[0] == 1U);            /* RS=1 → data */
    TEST_ASSERT(nibbles_to_byte(0) == (uint8_t)'A');
}

static void test_LCD_printChar_sends_digit_zero(void)
{
    capture_reset();
    LCD_printChar('0');            /* '0' = 0x30 */
    TEST_ASSERT(g_nibble_count == 2);
    TEST_ASSERT(g_is_data[0] == 1U);
    TEST_ASSERT(nibbles_to_byte(0) == (uint8_t)'0');
}

static void test_LCD_printChar_sends_space(void)
{
    capture_reset();
    LCD_printChar(' ');            /* ' ' = 0x20 */
    TEST_ASSERT(g_nibble_count == 2);
    TEST_ASSERT(g_is_data[0] == 1U);
    TEST_ASSERT(nibbles_to_byte(0) == (uint8_t)' ');
}

/* ================================================================== *
 *  LCD_printStr                                                       *
 * ================================================================== */

static void test_LCD_printStr_sends_all_characters(void)
{
    capture_reset();
    LCD_printStr("Hi");            /* 'H'=0x48, 'i'=0x69 */
    TEST_ASSERT(g_nibble_count == 4);            /* 2 chars × 2 nibbles */
    TEST_ASSERT(g_is_data[0] == 1U);
    TEST_ASSERT(nibbles_to_byte(0) == (uint8_t)'H');
    TEST_ASSERT(nibbles_to_byte(2) == (uint8_t)'i');
}

static void test_LCD_printStr_empty_sends_nothing(void)
{
    capture_reset();
    LCD_printStr("");
    TEST_ASSERT(g_nibble_count == 0);
}

static void test_LCD_printStr_null_sends_nothing(void)
{
    capture_reset();
    LCD_printStr(NULL);
    TEST_ASSERT(g_nibble_count == 0);
}

static void test_LCD_printStr_single_char(void)
{
    capture_reset();
    LCD_printStr("Z");
    TEST_ASSERT(g_nibble_count == 2);
    TEST_ASSERT(g_is_data[0] == 1U);
    TEST_ASSERT(nibbles_to_byte(0) == (uint8_t)'Z');
}

/* ================================================================== *
 *  LCD_print_data                                                     *
 * ================================================================== */

/*
 * LCD_print_data(data, posX, posY) converts a signed byte to two
 * decimal digit characters:
 *   tens_char = (data / 10) + '0'
 *   units_char = (data % 10) + '0'
 * then sets the cursor to (posY, posX) and prints both chars.
 *
 * Capture layout (6 nibbles):
 *   nibbles 0-1: LCD_setCursorPosition command
 *   nibbles 2-3: tens digit (DATA, RS=1)
 *   nibbles 4-5: units digit (DATA, RS=1)
 */

static void test_LCD_print_data_value_42(void)
{
    capture_reset();
    LCD_print_data(42, 0, 0);
    TEST_ASSERT(g_nibble_count == 6);
    /* cursor command */
    TEST_ASSERT(g_is_data[0] == 0U);
    TEST_ASSERT(nibbles_to_byte(0) == 0x80U);   /* row0, col0 */
    /* tens digit '4' = 0x34 */
    TEST_ASSERT(g_is_data[2] == 1U);
    TEST_ASSERT(nibbles_to_byte(2) == (uint8_t)'4');
    /* units digit '2' = 0x32 */
    TEST_ASSERT(g_is_data[4] == 1U);
    TEST_ASSERT(nibbles_to_byte(4) == (uint8_t)'2');
}

static void test_LCD_print_data_value_0(void)
{
    capture_reset();
    LCD_print_data(0, 0, 0);
    TEST_ASSERT(g_nibble_count == 6);
    TEST_ASSERT(nibbles_to_byte(2) == (uint8_t)'0');
    TEST_ASSERT(nibbles_to_byte(4) == (uint8_t)'0');
}

static void test_LCD_print_data_value_99(void)
{
    capture_reset();
    LCD_print_data(99, 0, 0);
    TEST_ASSERT(g_nibble_count == 6);
    TEST_ASSERT(nibbles_to_byte(2) == (uint8_t)'9');
    TEST_ASSERT(nibbles_to_byte(4) == (uint8_t)'9');
}

static void test_LCD_print_data_value_10(void)
{
    capture_reset();
    LCD_print_data(10, 0, 0);
    TEST_ASSERT(g_nibble_count == 6);
    TEST_ASSERT(nibbles_to_byte(2) == (uint8_t)'1');
    TEST_ASSERT(nibbles_to_byte(4) == (uint8_t)'0');
}

static void test_LCD_print_data_cursor_position(void)
{
    /* Verify the cursor is placed at the requested (row, col) */
    capture_reset();
    LCD_print_data(25, 9, 0);     /* posX=9, posY=0 → row=0, col=9 */
    TEST_ASSERT(g_nibble_count == 6);
    /* command = 0x80 | 0x09 = 0x89 */
    TEST_ASSERT(nibbles_to_byte(0) == 0x89U);
}

static void test_LCD_print_data_cursor_row1(void)
{
    capture_reset();
    LCD_print_data(7, 5, 1);      /* posX=5, posY=1 → row=1, col=5 */
    TEST_ASSERT(g_nibble_count == 6);
    /* command = 0xC0 | 0x05 = 0xC5 */
    TEST_ASSERT(nibbles_to_byte(0) == 0xC5U);
    TEST_ASSERT(nibbles_to_byte(2) == (uint8_t)'0');
    TEST_ASSERT(nibbles_to_byte(4) == (uint8_t)'7');
}

/* ================================================================== *
 *  main                                                               *
 * ================================================================== */

int main(void)
{
    printf("=== libLCD tests ===\n");

    /* LCD_setCursorPosition */
    RUN_TEST(test_LCD_setCursorPosition_row0_col0);
    RUN_TEST(test_LCD_setCursorPosition_row0_col5);
    RUN_TEST(test_LCD_setCursorPosition_row1_col0);
    RUN_TEST(test_LCD_setCursorPosition_row1_col15);
    RUN_TEST(test_LCD_setCursorPosition_row0_col15);

    /* LCD_clearScreen */
    RUN_TEST(test_LCD_clearScreen_sends_correct_commands);

    /* LCD_printChar */
    RUN_TEST(test_LCD_printChar_sends_character_as_data);
    RUN_TEST(test_LCD_printChar_sends_digit_zero);
    RUN_TEST(test_LCD_printChar_sends_space);

    /* LCD_printStr */
    RUN_TEST(test_LCD_printStr_sends_all_characters);
    RUN_TEST(test_LCD_printStr_empty_sends_nothing);
    RUN_TEST(test_LCD_printStr_null_sends_nothing);
    RUN_TEST(test_LCD_printStr_single_char);

    /* LCD_print_data */
    RUN_TEST(test_LCD_print_data_value_42);
    RUN_TEST(test_LCD_print_data_value_0);
    RUN_TEST(test_LCD_print_data_value_99);
    RUN_TEST(test_LCD_print_data_value_10);
    RUN_TEST(test_LCD_print_data_cursor_position);
    RUN_TEST(test_LCD_print_data_cursor_row1);

    PRINT_RESULTS();
    RETURN_TEST_RESULT();
}
