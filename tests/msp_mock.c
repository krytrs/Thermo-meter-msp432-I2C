/*
 * tests/msp_mock.c
 *
 * Definitions of all mock hardware register variables declared in msp.h.
 * Also provides a weak default implementation of mock_tick() that is a
 * no-op — sufficient for tests that never call delay_ms().  Tests that
 * do exercise delay_ms() (e.g. test_libLCD.c) supply a strong override
 * that calls TA0_ISR_Handler() to advance the internal tick counter and,
 * optionally, capture LCD nibble data.
 */

#include "msp.h"

/* ------------------------------------------------------------------ *
 *  Port registers                                                      *
 * ------------------------------------------------------------------ */
uint8_t P1DIR = 0U, P1OUT = 0U, P1REN = 0U, P1IES = 0U, P1IE = 0U, P1IFG = 0U;
uint8_t P2DIR = 0U, P2OUT = 0U;
uint8_t P4DIR = 0U, P4OUT = 0U, P4IN  = 0U, P4IFG = 0U;
uint8_t P6SEL0 = 0U, P6SEL1 = 0U;

/* ------------------------------------------------------------------ *
 *  eUSCI_B1 (I2C) registers                                           *
 * ------------------------------------------------------------------ */
uint16_t UCB1CTLW0 = 0U;
uint16_t UCB1I2CSA = 0U;
uint16_t UCB1BRW   = 0U;
uint16_t UCB1STATW = 0U;
uint8_t  UCB1RXBUF = 0U;
uint16_t UCB1ADDRX = 0U;
uint16_t UCB1IFG   = 0U;

/* ------------------------------------------------------------------ *
 *  Timer A registers (TA0–TA3)                                        *
 * ------------------------------------------------------------------ */
uint16_t TA0CCR0 = 0U, TA0CCTL0 = 0U, TA0CTL = 0U, TA0R = 0U;
uint16_t TA1CCR0 = 0U, TA1CCTL0 = 0U, TA1CTL = 0U, TA1R = 0U;
uint16_t TA2CCR0 = 0U, TA2CCTL0 = 0U, TA2CTL = 0U, TA2R = 0U;
uint16_t TA3CCR0 = 0U, TA3CCTL0 = 0U, TA3CTL = 0U, TA3R = 0U;

/* ------------------------------------------------------------------ *
 *  Peripheral struct instances                                         *
 * ------------------------------------------------------------------ */
NVIC_Type   mock_NVIC   = {{0U, 0U}};
SYSCTL_Type mock_SYSCTL = {0U, 0U};
FLCTL_Type  mock_FLCTL  = {0U, 0U, 0U};
MPU_Type    mock_MPU    = {0U};
RSTCTL_Type mock_RSTCTL = {0U, 0U, 0U, 0U};
uint32_t    WDTCTL      = 0U;

/* ------------------------------------------------------------------ *
 *  weak mock_tick — no-op default                                     *
 * ------------------------------------------------------------------ */
__attribute__((weak)) void mock_tick(void)
{
    /* Default: nothing to do.
     * Tests that need delay_ms() to terminate must provide a strong
     * override that calls TA0_ISR_Handler() to increment time_loop. */
    (void)0;
}
