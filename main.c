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



    while (1)
    {

        LCD_clearScreen();
        LCD_printStr("Connor...");
               LCD_setCursorPosition(1, 6);
    }

}
