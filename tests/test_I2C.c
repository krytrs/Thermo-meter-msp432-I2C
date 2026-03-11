/*
 * tests/test_I2C.c
 *
 * Unit tests for the I2C master driver (I2C.c / I2C.h).
 *
 * Strategy
 * --------
 * All MSP432 hardware registers are plain global variables provided by
 * msp_mock.c.  Each test resets the relevant registers to a known state,
 * calls the function under test, then asserts the expected register values.
 *
 * None of the I2C functions call delay_ms(), so mock_tick() is never
 * invoked and the weak no-op default in msp_mock.c is sufficient.
 */

#include <stdio.h>
#include "test_utils.h"

/* Pull in the mock hardware before the source under test */
#include "msp.h"
#include "../I2C.h"

TEST_DEFINE_COUNTERS

/* ------------------------------------------------------------------ *
 *  Helper: reset all I2C-related registers to 0                       *
 * ------------------------------------------------------------------ */
static void reset_i2c_regs(void)
{
    UCB1CTLW0 = 0U;
    UCB1I2CSA = 0U;
    UCB1BRW   = 0U;
    UCB1STATW = 0U;
    UCB1RXBUF = 0U;
    UCB1ADDRX = 0U;
    P6SEL0    = 0U;
    P6SEL1    = 0U;
}

/* ================================================================== *
 *  I2C_setAddress                                                      *
 * ================================================================== */

static void test_I2C_setAddress_stores_address(void)
{
    reset_i2c_regs();
    I2C_setAddress(0x48U);
    TEST_ASSERT(UCB1I2CSA == 0x48U);
}

static void test_I2C_setAddress_stores_zero(void)
{
    reset_i2c_regs();
    UCB1I2CSA = 0xFFU;           /* pre-load a non-zero value */
    I2C_setAddress(0x00U);
    TEST_ASSERT(UCB1I2CSA == 0x00U);
}

static void test_I2C_setAddress_stores_max_7bit(void)
{
    reset_i2c_regs();
    I2C_setAddress(0x7FU);
    TEST_ASSERT(UCB1I2CSA == 0x7FU);
}

/* ================================================================== *
 *  I2C_setMode                                                         *
 * ================================================================== */

static void test_I2C_setMode_transmit_sets_uctr_bit(void)
{
    reset_i2c_regs();
    I2C_setMode(BK_I2C_TRANSMIT);
    TEST_ASSERT((UCB1CTLW0 & BK_I2C_TRANSMIT) != 0U);
}

static void test_I2C_setMode_receive_clears_uctr_bit(void)
{
    reset_i2c_regs();
    UCB1CTLW0 = BK_I2C_TRANSMIT;   /* pre-set transmit bit */
    I2C_setMode(BK_I2C_RECEIVE);
    TEST_ASSERT((UCB1CTLW0 & BK_I2C_TRANSMIT) == 0U);
}

static void test_I2C_setMode_receive_after_transmit(void)
{
    reset_i2c_regs();
    I2C_setMode(BK_I2C_TRANSMIT);
    TEST_ASSERT((UCB1CTLW0 & BK_I2C_TRANSMIT) != 0U);
    I2C_setMode(BK_I2C_RECEIVE);
    TEST_ASSERT((UCB1CTLW0 & BK_I2C_TRANSMIT) == 0U);
}

/* ================================================================== *
 *  I2C_isBusy                                                          *
 * ================================================================== */

static void test_I2C_isBusy_returns_zero_when_idle(void)
{
    reset_i2c_regs();
    UCB1STATW = 0x0000U;           /* bus not busy */
    TEST_ASSERT(I2C_isBusy() == 0U);
}

static void test_I2C_isBusy_returns_nonzero_when_busy(void)
{
    reset_i2c_regs();
    UCB1STATW = UCBBUSY;           /* bus busy flag set */
    TEST_ASSERT(I2C_isBusy() != 0U);
}

static void test_I2C_isBusy_ignores_other_bits(void)
{
    reset_i2c_regs();
    /* Set all bits except UCBBUSY */
    UCB1STATW = (uint16_t)(~(uint16_t)UCBBUSY);
    TEST_ASSERT(I2C_isBusy() == 0U);
}

/* ================================================================== *
 *  I2C_masterReceived                                                  *
 * ================================================================== */

static void test_I2C_masterReceived_returns_rx_buffer(void)
{
    reset_i2c_regs();
    UCB1RXBUF = 0x42U;
    TEST_ASSERT(I2C_masterReceived() == 0x42U);
}

static void test_I2C_masterReceived_returns_zero(void)
{
    reset_i2c_regs();
    UCB1RXBUF = 0x00U;
    TEST_ASSERT(I2C_masterReceived() == 0x00U);
}

static void test_I2C_masterReceived_returns_max_byte(void)
{
    reset_i2c_regs();
    UCB1RXBUF = 0xFFU;
    TEST_ASSERT(I2C_masterReceived() == 0xFFU);
}

/* ================================================================== *
 *  I2C_disable / I2C_enable                                           *
 * ================================================================== */

static void test_I2C_disable_sets_reset_bit(void)
{
    reset_i2c_regs();
    UCB1CTLW0 = 0x0000U;
    I2C_disable();
    TEST_ASSERT((UCB1CTLW0 & UCSWRST) != 0U);
}

static void test_I2C_disable_preserves_other_bits(void)
{
    reset_i2c_regs();
    UCB1CTLW0 = 0x0800U;           /* UCMST bit set */
    I2C_disable();
    TEST_ASSERT((UCB1CTLW0 & 0x0800U) != 0U);   /* UCMST still set */
    TEST_ASSERT((UCB1CTLW0 & UCSWRST) != 0U);   /* UCSWRST now set */
}

static void test_I2C_enable_clears_reset_bit(void)
{
    reset_i2c_regs();
    UCB1CTLW0 = UCSWRST;
    I2C_enable();
    TEST_ASSERT((UCB1CTLW0 & UCSWRST) == 0U);
}

static void test_I2C_enable_preserves_other_bits(void)
{
    reset_i2c_regs();
    UCB1CTLW0 = UCSWRST | (uint16_t)0x0800U;   /* UCSWRST + UCMST */
    I2C_enable();
    TEST_ASSERT((UCB1CTLW0 & UCSWRST)   == 0U);    /* UCSWRST cleared */
    TEST_ASSERT((UCB1CTLW0 & 0x0800U) != 0U);   /* UCMST preserved */
}

/* ================================================================== *
 *  I2C_masterStop                                                      *
 * ================================================================== */

static void test_I2C_masterStop_sets_stop_bit(void)
{
    reset_i2c_regs();
    I2C_masterStop();
    TEST_ASSERT((UCB1CTLW0 & UCTXSTP) != 0U);
}

static void test_I2C_masterStop_preserves_other_bits(void)
{
    reset_i2c_regs();
    UCB1CTLW0 = UCSWRST;           /* some existing bit */
    I2C_masterStop();
    TEST_ASSERT((UCB1CTLW0 & UCTXSTP) != 0U);
    TEST_ASSERT((UCB1CTLW0 & UCSWRST) != 0U);
}

/* ================================================================== *
 *  I2C_init                                                            *
 * ================================================================== */

static void test_I2C_init_sets_software_reset(void)
{
    reset_i2c_regs();
    I2C_init();
    TEST_ASSERT((UCB1CTLW0 & UCSWRST) != 0U);
}

static void test_I2C_init_sets_master_mode(void)
{
    reset_i2c_regs();
    I2C_init();
    TEST_ASSERT((UCB1CTLW0 & UCMST) != 0U);
}

static void test_I2C_init_sets_i2c_mode(void)
{
    reset_i2c_regs();
    I2C_init();
    TEST_ASSERT((UCB1CTLW0 & UCMODE_3) == UCMODE_3);
}

static void test_I2C_init_sets_smclk_source(void)
{
    reset_i2c_regs();
    I2C_init();
    TEST_ASSERT((UCB1CTLW0 & UCSSEL__SMCLK) != 0U);
}

static void test_I2C_init_sets_baud_prescaler(void)
{
    reset_i2c_regs();
    I2C_init();
    TEST_ASSERT(UCB1BRW == 30U);
}

static void test_I2C_init_configures_p6_sel0(void)
{
    reset_i2c_regs();
    P6SEL0 = 0U;
    I2C_init();
    /* Both P6.4 (SCL) and P6.5 (SDA) must be selected */
    TEST_ASSERT((P6SEL0 & (1U << 4U)) != 0U);
    TEST_ASSERT((P6SEL0 & (1U << 5U)) != 0U);
}

static void test_I2C_init_clears_p6_sel1(void)
{
    reset_i2c_regs();
    P6SEL1 = 0xFFU;               /* pre-set all bits */
    I2C_init();
    TEST_ASSERT((P6SEL1 & (1U << 4U)) == 0U);
    TEST_ASSERT((P6SEL1 & (1U << 5U)) == 0U);
}

/* ================================================================== *
 *  I2C_masterReceiveStart                                              *
 * ================================================================== */

static void test_I2C_masterReceiveStart_sets_start_bit(void)
{
    reset_i2c_regs();
    /* UCTXSTT is set by the function; the mock never clears it
     * (no real hardware), so the while-loop runs until timeout
     * (BK_I2C_ADDR_TIMEOUT = 10 000 iterations) and then exits.
     * After return, UCTXSTT must still be set in the register. */
    I2C_masterReceiveStart();
    TEST_ASSERT((UCB1CTLW0 & UCTXSTT) != 0U);
}

static void test_I2C_masterReceiveStart_clears_transmit_bit(void)
{
    reset_i2c_regs();
    UCB1CTLW0 = BK_I2C_TRANSMIT;  /* pre-set transmit mode */
    I2C_masterReceiveStart();
    TEST_ASSERT((UCB1CTLW0 & BK_I2C_TRANSMIT) == 0U);
}

/* ================================================================== *
 *  main                                                                *
 * ================================================================== */

int main(void)
{
    printf("=== I2C tests ===\n");

    /* I2C_setAddress */
    RUN_TEST(test_I2C_setAddress_stores_address);
    RUN_TEST(test_I2C_setAddress_stores_zero);
    RUN_TEST(test_I2C_setAddress_stores_max_7bit);

    /* I2C_setMode */
    RUN_TEST(test_I2C_setMode_transmit_sets_uctr_bit);
    RUN_TEST(test_I2C_setMode_receive_clears_uctr_bit);
    RUN_TEST(test_I2C_setMode_receive_after_transmit);

    /* I2C_isBusy */
    RUN_TEST(test_I2C_isBusy_returns_zero_when_idle);
    RUN_TEST(test_I2C_isBusy_returns_nonzero_when_busy);
    RUN_TEST(test_I2C_isBusy_ignores_other_bits);

    /* I2C_masterReceived */
    RUN_TEST(test_I2C_masterReceived_returns_rx_buffer);
    RUN_TEST(test_I2C_masterReceived_returns_zero);
    RUN_TEST(test_I2C_masterReceived_returns_max_byte);

    /* I2C_disable / I2C_enable */
    RUN_TEST(test_I2C_disable_sets_reset_bit);
    RUN_TEST(test_I2C_disable_preserves_other_bits);
    RUN_TEST(test_I2C_enable_clears_reset_bit);
    RUN_TEST(test_I2C_enable_preserves_other_bits);

    /* I2C_masterStop */
    RUN_TEST(test_I2C_masterStop_sets_stop_bit);
    RUN_TEST(test_I2C_masterStop_preserves_other_bits);

    /* I2C_init */
    RUN_TEST(test_I2C_init_sets_software_reset);
    RUN_TEST(test_I2C_init_sets_master_mode);
    RUN_TEST(test_I2C_init_sets_i2c_mode);
    RUN_TEST(test_I2C_init_sets_smclk_source);
    RUN_TEST(test_I2C_init_sets_baud_prescaler);
    RUN_TEST(test_I2C_init_configures_p6_sel0);
    RUN_TEST(test_I2C_init_clears_p6_sel1);

    /* I2C_masterReceiveStart */
    RUN_TEST(test_I2C_masterReceiveStart_sets_start_bit);
    RUN_TEST(test_I2C_masterReceiveStart_clears_transmit_bit);

    PRINT_RESULTS();
    RETURN_TEST_RESULT();
}
