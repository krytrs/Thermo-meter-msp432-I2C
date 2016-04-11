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
	unsigned char teplota = 0, teplotaOld;


    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    LCD_init();
    I2C_init();
    I2C_setAddress(0x48); // adresa 1001000
    D_stav_diagnostiky = diag();
    if (D_stav_diagnostiky == DK_DIAG_STATUS_OK)
    {
        P2OUT &= ~BIT1;      // Pokud probehla diagnostika v poradku, zhasni zelenou diodu
    }
    else
    {
        P2OUT |= BIT0;      // Pokud neprobehla diagnostika v poradku, rozsvit cervenou diodu
        __deep_sleep()      // a prejdi do lpm3
    }


    P1DIR &= ~BIT1;     // nastaveni port1 pin1 vstupni
    P1REN |=  BIT1;     // nastaveni pulluppu
    P1OUT |=  BIT1;     // pullup proti Vcc
    P1IES |=  BIT1;     // nastaveni interuptu na sestupnou hranu
    P1IE  |=  BIT1;     // nastaveni povoleni interuptu
    P1IFG &= ~BIT1;     // schozeni flagy interuptu

    P1DIR |=  BIT0;     // nastaveni port 1 pin 0 na vystup, cervena led
    P1OUT &= ~BIT0;     // zhasnuti


    NVIC->ISER[1] |= 1 << (PORT1_IRQn-32);     /* 51 PORT1 Interrupt */

    __enable_irq();
    __enable_interrupt();
    teplotaOld = 200;       // nejak blbne zobrazovani teploty, zajistuji aby smycka probehla aspon jednou
    while (1)
    {
        I2C_masterReceiveStart();
        teplota = I2C_masterReceived();
        I2C_masterStop();
        UCB1IFG = 0;

        if (teplota != teplotaOld)
        {
        	LCD_clearScreen();
        	LCD_setCursorPosition(0,0);
        	LCD_printStr("Teplota: ");
        	LCD_print_data(teplota, 9, 0);
        	teplotaOld = teplota;
        }
        delay_ms(1000);
    }

}


void port1_ISR_Handler(void)
{
    P1IFG &= ~BIT1;         // Schozeni flagy interruptu
    P1OUT ^= BIT0;          // zmenit stav ledky

}
