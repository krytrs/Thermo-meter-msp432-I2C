//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include "libLCD.h"

void main(void)
{
	uint16_t i;
	
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer
	
    LCD_init();
    LCD_clearScreen();
    LCD_setCursorPosition(0, 0);
    for (i = 0; i < 60000; i++);
    LCD_printStr("Petr je pasak");



    while (1)
    {

    			;
    }

}
