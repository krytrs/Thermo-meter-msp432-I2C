//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include "libLCD.h"
#include "diag.h"

void main(void)
{
	uint8_t D_stav_diagnostiky = 0;
	
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    LCD_init();
    LCD_clearScreen();
    LCD_setCursorPosition(0, 0);
    LCD_printStr("Petr je pasak");

    while (1)
    {
        LCD_clearScreen();
        LCD_setCursorPosition(0, 0);
        LCD_printStr("Spoustim diag");
        D_stav_diagnostiky = diag();
        switch(D_stav_diagnostiky)
        {
        case DK_DIAG_STATUS_OK :
            break;

        case DK_DIAG_STATUS_CHYBA :
            LCD_clearScreen();
            LCD_setCursorPosition(0, 0);
            LCD_printStr("Nastala chyba");
            break;

        default:
            LCD_clearScreen();
            LCD_setCursorPosition(0, 0);
            LCD_printStr("Nastala neznama");
            LCD_setCursorPosition(1, 0);
            LCD_printStr("chyba");
        }

    }

}
