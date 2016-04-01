//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include "libLCD.h"

void main(void)
{
	
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
	
    LCD_init();
    LCD_clearScreen();
    LCD_setCursorPosition(0, 0);
    LCD_printStr("Petr je pasak");



    while (1)
    {

    			;
    }

}
