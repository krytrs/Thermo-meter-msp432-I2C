/*
 * main.c
 *
 * MSP432 Thermometer — main application
 */

#include "msp.h"
#include "libLCD.h"
#include "diag.h"
#include "I2C.h"


int main(void)
{
    uint8_t diagStatus;
    uint8_t temperature    = 0U;
    uint8_t temperatureOld = 200U;  /* Initial value forces display update on first read */

    WDTCTL = WDTPW | WDTHOLD;      /* Stop watchdog timer */

    LCD_init();
    I2C_init();
    I2C_setAddress(DK_SENSOR_ADDRESS);

    diagStatus = diag();

    if (diagStatus == DK_DIAG_STATUS_OK)
    {
        P2OUT &= ~BIT1;             /* Diagnostics passed: turn off green LED */
    }
    else
    {
        P2OUT |= BIT0;              /* Diagnostics failed: turn on red LED    */
        __deep_sleep();             /* Enter deep sleep */
    }

    P1DIR &= ~BIT1;                 /* P1.1: input                          */
    P1REN |=  BIT1;                 /* Enable pull resistor                 */
    P1OUT |=  BIT1;                 /* Pull-up to Vcc                       */
    P1IES |=  BIT1;                 /* Interrupt on falling edge            */
    P1IE  |=  BIT1;                 /* Enable interrupt                     */
    P1IFG &= ~BIT1;                 /* Clear interrupt flag                 */

    P1DIR |=  BIT0;                 /* P1.0: output (red LED)               */
    P1OUT &= ~BIT0;                 /* Turn off                             */

    NVIC->ISER[1] |= 1U << ((uint32_t)PORT1_IRQn - 32U); /* Enable PORT1 interrupt */

    __enable_irq();

    while (1)
    {
        I2C_masterReceiveStart();
        temperature = I2C_masterReceived();
        I2C_masterStop();
        UCB1IFG = 0U;

        if (temperature != temperatureOld)
        {
            LCD_clearScreen();
            LCD_setCursorPosition(0U, 0U);
            LCD_printStr("Teplota: ");
            LCD_print_data(temperature, 9U, 0U);
            temperatureOld = temperature;
        }

        delay_ms(1000U);
    }

    return 0;
}


void port1_ISR_Handler(void)
{
    P1IFG &= ~BIT1;                 /* Clear interrupt flag */
    P1OUT ^=  BIT0;                 /* Toggle LED           */
}

