/*
 * diag.c
 *
 *  Created on: 3. 4. 2016
 *      Author: Petr Gallistl
 *
 *  Startup diagnostics: LED blink animation and hardware self-test.
 *  Tests: SRAM, I2C sensor presence, LCD read-back.
 *  A software watchdog (Timer A3) resets the device if diagnostics
 *  do not complete within DK_DIAG_WTD_TIME seconds.
 */

#include "msp.h"
#include "diag.h"
#include "libLCD.h"
#include "I2C.h"

/* SRAM region covered by the memory test */
#define SRAM_TEST_START  ((uint32_t)0x20006000U)
#define SRAM_TEST_END    ((uint32_t)0x20007000U)

/* PWM duty-cycle boundaries */
#define PWM_DUTY_MAX     ((uint16_t)1450U)
#define PWM_DUTY_MIN     ((uint16_t)100U)
#define PWM_PERIOD       ((uint16_t)1500U)

/* Module-level variables shared with ISR handlers */
static volatile uint16_t s_pwmDuty      = 200U;
static volatile uint8_t  s_pwmDirection = 0U;
static volatile uint8_t  s_blinkCount   = 0U;
static volatile uint8_t  s_diagStatus   = DK_DIAG_STATUS_OK;
static volatile uint8_t  s_wdtCount     = 0U;


/*---------------------------------------------------------------------------*\
|* HELPER :: diag_showError
|*
|*    Display a two-line error message on the LCD.
\*---------------------------------------------------------------------------*/
static void diag_showError(const char *line0, const char *line1)
{
    LCD_clearScreen();
    LCD_setCursorPosition(0U, 0U);
    LCD_printStr(line0);
    LCD_setCursorPosition(1U, 0U);
    LCD_printStr(line1);
}

/*---------------------------------------------------------------------------*\
|* HELPER :: diag_testSRAMPattern
|*
|*    Write a 32-bit pattern to every word in the test region then verify.
|*    Returns DK_DIAG_STATUS_MEM_E on any mismatch, otherwise OK.
\*---------------------------------------------------------------------------*/
static uint8_t diag_testSRAMPattern(uint32_t pattern)
{
    volatile uint32_t *ptr;
    uint8_t status = DK_DIAG_STATUS_OK;

    for (ptr = (volatile uint32_t *)SRAM_TEST_START;
         ptr < (volatile uint32_t *)SRAM_TEST_END;
         ptr++)
    {
        *ptr = pattern;
    }

    for (ptr = (volatile uint32_t *)SRAM_TEST_START;
         ptr < (volatile uint32_t *)SRAM_TEST_END;
         ptr++)
    {
        if (*ptr != pattern)
        {
            status = DK_DIAG_STATUS_MEM_E;
        }
    }

    return status;
}

/*---------------------------------------------------------------------------*\
|* HELPER :: diag_testSRAM
|*
|*    Unlock flash/SRAM, set MPU permissions, then run two-pattern SRAM test.
\*---------------------------------------------------------------------------*/
static uint8_t diag_testSRAM(void)
{
    uint8_t result;

    SYSCTL->SRAM_BANKEN    = SYSCTL_SRAM_BANKEN_BNK7_EN;
    SYSCTL->SECDATA_UNLOCK = 0x695AU;
    FLCTL->PRGBRST_CTLSTAT &= ~FLCTL_PRG_CTLSTAT_MODE;
    FLCTL->PRGBRST_CTLSTAT |=  FLCTL_PRG_CTLSTAT_ENABLE;
    FLCTL->BANK0_MAIN_WEPROT = 0x0000U;
    FLCTL->BANK1_MAIN_WEPROT = 0x0000U;
    MPU->RASR |= MPU_RASR_AP_PRV_RW_USR_RW;

    result = diag_testSRAMPattern(0x00000000U);
    if (diag_testSRAMPattern(0xFFFFFFFFU) != DK_DIAG_STATUS_OK)
    {
        result = DK_DIAG_STATUS_MEM_E;
    }

    return result;
}

/*---------------------------------------------------------------------------*\
|* HELPER :: diag_testI2C
|*
|*    Attempt a single-byte read from the configured I2C address.
|*    Returns DK_DIAG_STATUS_I2C_E if no address was ACKed.
\*---------------------------------------------------------------------------*/
static uint8_t diag_testI2C(void)
{
    I2C_masterReceiveStart();
    (void)I2C_masterReceived();
    I2C_masterStop();

    if ((UCB1ADDRX & MSK_I2C_ADDR) == 0U)
    {
        return DK_DIAG_STATUS_I2C_E;
    }

    return DK_DIAG_STATUS_OK;
}

/*---------------------------------------------------------------------------*\
|* HELPER :: diag_testLCD
|*
|*    Write two known strings to the LCD and read them back.
|*    Returns DK_DIAG_STATUS_LCD_E on any character mismatch.
\*---------------------------------------------------------------------------*/
static uint8_t diag_testLCD(void)
{
    static const char line1[16] = "Toto je testovac";
    static const char line2[16] = "i sekvence znaku";
    uint8_t i;
    char    received;

    LCD_clearScreen();
    LCD_setCursorPosition(0U, 0U);
    LCD_printStr(line1);
    LCD_setCursorPosition(1U, 0U);
    LCD_printStr(line2);

    LCD_setCursorPosition(0U, 0U);
    for (i = 0U; i < 15U; i++)
    {
        received = LCD_receive();
        if (received != line1[i])
        {
            return DK_DIAG_STATUS_LCD_E;
        }
    }

    LCD_setCursorPosition(1U, 0U);
    for (i = 0U; i < 15U; i++)
    {
        received = LCD_receive();
        if (received != line2[i])
        {
            return DK_DIAG_STATUS_LCD_E;
        }
    }

    return DK_DIAG_STATUS_OK;
}


/*---------------------------------------------------------------------------*\
|* PUBLIC FUNCTION :: diag
|*
|*    Run the startup self-test sequence.
|*    Returns one of the DK_DIAG_STATUS_* codes.
\*---------------------------------------------------------------------------*/
uint8_t diag(void)
{
    /* Configure LED output pins (blue = BIT2, red = BIT0, green = BIT1) */
    P2DIR |= BIT0 | BIT1 | BIT2;
    P2OUT &= ~(BIT0 | BIT1 | BIT2);

    /* If a previous run ended with a software hard-reset, report it */
    if ((RSTCTL->HARDRESET_STAT & RSTCTL_HARDRESET_SET_SRC1) ==
         RSTCTL_HARDRESET_SET_SRC1)
    {
        diag_showError("Nastala chyba", "RESET");
        RSTCTL->HARDRESET_CLR = 0xFFFFFFFFU;
        return DK_DIAG_STATUS_RESET;
    }

    /* Timer A3 — software watchdog: fires every ~1 s (ACLK 4096 Hz / 8) */
    TA3CCR0 = 4096U;
    TA3CCTL0 |= CCIE;
    TA3CTL |= TIMER_A_CTL_SSEL__ACLK | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_3;
    NVIC->ISER[0] |= 1U << (uint32_t)TA3_0_IRQn;

    /* Timer A2 — PWM duty-cycle control (SMCLK / 2) */
    TA2CCR0 = s_pwmDuty;
    TA2CCTL0 |= CCIE;
    TA2CTL |= TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_1;
    NVIC->ISER[0] |= 1U << (uint32_t)TA2_0_IRQn;

    /* Timer A1 — PWM period control (SMCLK / 2) */
    TA1CCR0 = PWM_PERIOD;
    TA1CCTL0 |= CCIE;
    TA1CTL |= TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_1;
    NVIC->ISER[0] |= 1U << (uint32_t)TA1_0_IRQn;

    __enable_irq();

    LCD_clearScreen();
    LCD_setCursorPosition(0U, 0U);
    LCD_printStr("Spoustim diag");
    delay_ms(500U);

    s_blinkCount = 0U;
    s_diagStatus = DK_DIAG_STATUS_OK;

    /* Run all hardware tests once, then wait for the blink animation to end */
    s_diagStatus = diag_testSRAM();

    if (s_diagStatus == DK_DIAG_STATUS_OK)
    {
        s_diagStatus = diag_testI2C();
    }

    if (s_diagStatus == DK_DIAG_STATUS_OK)
    {
        s_diagStatus = diag_testLCD();
    }

    while (s_blinkCount < 2U)
    {
        __wfi();    /* Wait for interrupt (low-power wait) */
    }

    /* Stop all timers and turn off the blue LED */
    TA1CTL &= ~MC_3;
    TA2CTL &= ~MC_3;
    TA3CTL &= ~MC_3;
    P2OUT = 0U;

    /* Display result */
    switch (s_diagStatus)
    {
        case DK_DIAG_STATUS_OK:
            LCD_clearScreen();
            LCD_setCursorPosition(0U, 0U);
            LCD_printStr("Diag OK");
            break;

        case DK_DIAG_STATUS_I2C_E:
            diag_showError("Nastala chyba", "I2C");
            break;

        case DK_DIAG_STATUS_LCD_E:
            diag_showError("Nastala chyba", "LCD");
            break;

        case DK_DIAG_STATUS_MEM_E:
            diag_showError("Nastala chyba", "SRAM");
            break;

        case DK_DIAG_STATUS_CHYBA:
            /* falls through */
        default:
            diag_showError("Nastala neznama", "chyba");
            break;
    }

    if (s_diagStatus == DK_DIAG_STATUS_OK)
    {
        P2OUT |= BIT1;  /* Green LED: pass */
    }
    else
    {
        P2OUT |= BIT0;  /* Red LED:   fail */
    }

    delay_ms(1000U);

    return s_diagStatus;
}


/*
 * ISR handlers for the PWM blink animation (Timer A1 = period, Timer A2 = duty)
 * and the diagnostic watchdog (Timer A3).
 */

void TA2_ISR_Handler(void)
{
    TA2CTL    &= ~MC_3;             /* Stop timer         */
    TA2CCTL0  &= ~CCIFG;            /* Clear flag         */
    TA2R       = 0U;                /* Reset counter      */
    P2OUT     &= ~BIT2;             /* Blue LED off       */

    if (s_pwmDirection == 1U)
    {
        s_pwmDuty++;
    }
    else
    {
        s_pwmDuty--;
    }

    if (s_pwmDuty >= PWM_DUTY_MAX)
    {
        s_blinkCount++;             /* Count one full blink cycle */
        s_pwmDirection = 0U;
    }

    if (s_pwmDuty <= PWM_DUTY_MIN)
    {
        s_pwmDirection = 1U;
    }

    TA2CCR0 = s_pwmDuty;
}

void TA1_ISR_Handler(void)
{
    TA1CTL   &= ~MC_3;              /* Stop timer          */
    TA1CCTL0 &= ~CCIFG;             /* Clear flag          */
    TA1R      = 0U;                 /* Reset counter       */

    TA1CTL |= TIMER_A_CTL_MC__UP;  /* Restart period timer */
    TA2CTL |= TIMER_A_CTL_MC__UP;  /* Restart duty timer   */
    P2OUT  |= BIT2;                 /* Blue LED on          */
}

void TA3_ISR_Handler(void)
{
    TA3CTL   &= ~MC_3;              /* Stop timer          */
    TA3CCTL0 &= ~CCIFG;             /* Clear flag          */
    TA3R      = 0U;                 /* Reset counter       */

    if (s_wdtCount >= DK_DIAG_WTD_TIME)
    {
        /* Diagnostics took too long — trigger a hard reset (source 1) */
        s_diagStatus = DK_DIAG_STATUS_CHYBA;
        RSTCTL->HARDRESET_SET = RSTCTL_HARDRESET_SET_SRC1;
        RSTCTL->RESET_REQ     = 0x6901U;
    }
    else
    {
        s_wdtCount++;
    }

    TA3CTL |= TIMER_A_CTL_MC__UP;  /* Restart watchdog timer */
}




