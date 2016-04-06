//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include "libLCD.h"
#include "diag.h"
#include "I2C.h"

void main(void)
{
	uint8_t D_stav_diagnostiky = 0;
	unsigned char teplota;


    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    LCD_init();
    I2C_init();
    I2C_setAddress(0x48); // adresa 1001000
    P1DIR &= ~BIT1;     // nastaveni port1 pin1 vstupni
    P1REN |=  BIT1;     // nastaveni pulluppu
    P1OUT |=  BIT1;     // pullup proti Vcc
    P1IES |=  BIT1;     // nastaveni interuptu na sestupnou hranu
    P1IE  |=  BIT1;     // nastaveni povoleni interuptu
    P1IFG &= ~BIT1;     // schozeni flagy interuptu

    P1DIR |=  BIT0;     // nastaveni port 1 pin 0 na vystup, cervena led
    P1OUT &= ~BIT0;     // zhasnuti

    D_stav_diagnostiky = diag();

    NVIC->ISER[1] |= 1 << (PORT1_IRQn-32);     /* 51 PORT1 Interrupt */

    __enable_irq();
    __enable_interrupt();

    while (1)
    {
        I2C_masterReceiveStart();
        delay_ms(20);
        teplota = I2C_masterReceive();
        I2C_masterStop();
       __deep_sleep()   // enter lpm3
    }

}


void port1_ISR_Handler(void)
{
    P1IFG &= ~BIT1;         // Schozeni flagy interruptu
    P1OUT ^= BIT0;          // zmenit stav ledky

}
