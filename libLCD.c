/*
 * libLCD.c
 *
 *  Created on: Oct 18, 2012
 *      Author: Stephen Bennett
 */

#include "libLCD.h"


/******************************\
|* Helper Function Prototypes *|
\******************************/

void delay_ms(uint16_t time_ms);
void LCD_sendCommand(char command);
inline void LCD_sendByte(char byteToSend, uint8_t byteType);
void LCD_sendNibble(char nibbleToSend);
void LCD_pulseEnablePin(void);
void LCD_print_data(int8_t data, int8_t poziceX, int8_t poziceY);

/* delay ms
 * This function realize waitnig in multiplacion of 1ms
 * Using aux generator of 32768 Hz
 *
 * PARAMETERS
 * 		time_ms - uint_8 - for delay in ms
 *
 * RETURN
 *
 * 		void
 */
uint16_t time_loop = 0;
void delay_ms(uint16_t time_ms)
{
	time_loop = 0;
	// set timer 0
	TA0CCR0 = 3000;
	TA0CCTL0 |= CCIE;		// enable interupt
	// set clock source to aux on 3 MHz, set timer to count up, set a prescaler to 8
	TA0CTL |= TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_0;

    NVIC->ISER[0] |= 1 << TA0_0_IRQn;

    __enable_irq();
    __enable_interrupt();

	while(1)
	{
			if (time_ms == time_loop)  // if time is up
			{
			    TA0CTL &= ~MC_3;        // stop timer
			    return;
			}
	}
}


void TA0_ISR_Handler(void)
{
    {
        TA0CTL &= ~MC_3;            // stop timer
        TA0CCTL0 &= ~CCIFG;         // drop flag
        TA0R = 0;                   // reset timer time
        time_loop++;
        TA0CTL |= TIMER_A_CTL_MC__UP;
    }

}


/********************\
|* Public Functions *|
\********************/

/*-------------------------------------------------------------------------*\
|* PUBLIC FUNCTION :: LCD_setCursorPosition
|*
|*    Set the position of the cursor on the screen.
|*
|* PARAMETERS
|*
|*    ** Does not check row/col limits **
|*    row - zero based row number < LCD_NUM_ROWS
|*    col - zero based col number < LCD_NUM_COLS
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_setCursorPosition(uint8_t row, uint8_t col)
{
   uint8_t address;

   /* Construct address from (row, col) pair */
   if (row == 0)
   {
      address = 0;
   }
   else
   {
      address = 0x40;
   }

   address |= col;

   LCD_sendCommand(0x80 | address);
}

/*-------------------------------------------------------------------------*\
|* PUBLIC FUNCTION :: LCD_clearScreen
|*
|*    Clear the screen data and return the cursor to home position (0, 0).
|*    Automatically sets the cursor to increment.
|*    Display shifting is unaffected.
|*
|* PARAMETERS
|*
|*    void
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_clearScreen(void)
{
   LCD_sendCommand(CLEAR_DISP_CMD);
   LCD_sendCommand(RET_HOME_CMD);
}

/*-------------------------------------------------------------------------*\
|* PUBLIC FUNCTION :: LCD_init
|*
|*    Initialize the LCD after power-up
|*
|* NOTE:
|*    This routine must not be called twice on the LCD.
|*    This is not uncommon when the power for the MCU and LCD are separate.
|*
|* PARAMETERS
|*
|*    void
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_init(void)
{
   /* Set the MSP pin configurations and bring them LOW */
   LCD_DIR_DATA |= LCD_MASK_DATA;
   LCD_DIR_RS   |= LCD_PIN_RS;
   LCD_DIR_EN   |= LCD_PIN_EN;
   LCD_DIR_RW   |= LCD_PIN_RW;

   LCD_OUT_DATA &= ~(LCD_MASK_DATA);
   LCD_OUT_RS   &= ~LCD_PIN_RS;
   LCD_OUT_EN   &= ~LCD_PIN_EN;
   LCD_OUT_RW   &= ~LCD_PIN_RW;

   /* Wait for the LCD to warm up and reach active regions.
    * Remember MSPs can power up much faster than the LCD.
    * (At least 40ms after Vcc rises above 2.7V)
    */
   delay_ms(80);

   /*****************************\
   |* Initialize the LCD module *|
   \*****************************/
   /* According to Hitachi HD44780 datasheet (fig 24, pg 46) */

   /* 1a. Set to 4-bit input - try three (3) times */
   uint8_t i = 3;
   while (i != 0)
   {
      LCD_sendNibble(0x03);
      delay_ms(5);
      --i;
   }


   /* 1b. Set to 4-bit interface */
   LCD_sendNibble(0x02);
   delay_ms(1);

   /* 1c. Set number of lines, font size, etc. */
   LCD_sendCommand(FUNCTION_SET_CMD);
   delay_ms(1);

   /* 2. Initialize display
    *    Turn display on/off, cursor on/off, cursor blinking on/off
    */
   LCD_sendCommand(DISPLAY_ON);
   delay_ms(1);


   LCD_sendCommand(DISPLAY_ON_CMD);
   delay_ms(1);

   /* 3. Initialize entry mode
    *    Text direction, cursor move, display shifting, etc.
    */
   LCD_sendCommand(ENTRY_MODE_CMD);
   delay_ms(1);
}

/*-------------------------------------------------------------------------*\
|* PUBLIC FUNCTION :: LCD_printStr
|*
|*    Print a string of characters to the screen.
|*
|* PARAMETERS
|*
|*    text - NULL terminated string of chars
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_printStr(char *text)
{
   char *c;
   c = text;

   while ((c != 0) && (*c != 0))
   {
      LCD_sendByte(*c, DATA);
      c++;
   }
}
// tato funkce prevadi data a pak je tiskne na displej
void LCD_print_data(int8_t data, int8_t poziceX, int8_t poziceY)
{
	char dataDesitky, dataJednotky;
	dataDesitky = (data/10)+48;
	dataJednotky = (data%10)+48;
	LCD_setCursorPosition(poziceY, poziceX);
	LCD_printChar(dataDesitky);
	LCD_printChar(dataJednotky);
}



/*-------------------------------------------------------------------------*\
|* PUBLIC FUNCTION :: LCD_printChar
|*
|*    Print a character to the screen.
|*
|* PARAMETERS
|*
|*    character - character to be printed
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_printChar(char character)
{
   LCD_sendByte(character, DATA);
}

/*-------------------------------------------------------------------------*\
|* PUBLIC FUNCTION :: LCD_sendCommand
|*
|*    Send a command to the LCD on the data bus in 4 bit mode.
|*
|* PARAMETERS
|*
|*    command - command to send
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_sendCommand(char command) {
   LCD_sendByte(command, COMMAND);
}


/********************\
|* Helper Functions *|
\********************/

/*-------------------------------------------------------------------------*\
|* HELPER FUNCTION :: LCD_sendByte
|*
|*    Send a byte on the data bus in 4 bit mode.
|*    This requires sending the data in two chunks.
|*    High nibble first, then low nibble.
|*
|* PARAMETERS
|*
|*    byteToSend - single byte to send
|*    byteType   - either DATA or COMMAND
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
inline void LCD_sendByte(char byteToSend, uint8_t byteType)
{
   /* Set Reg Select line to appropriate mode (HIGH: data | LOW: command) */
   if (byteType == COMMAND)
   {
      LCD_OUT_RS &= ~LCD_PIN_RS;
   }
   else // (byteType == DATA)
   {
      LCD_OUT_RS |= LCD_PIN_RS;
   }

   /* set High Nibble (HN) on data lines */
   LCD_sendNibble( (byteToSend & 0xF0) >> 4);

   /* set Low Nibble (LN) on data lines */
   LCD_sendNibble( byteToSend & 0x0F);
}

/*-------------------------------------------------------------------------*\
|* HELPER FUNCTION :: LCD_sendNibble
|*
|*    Send a nibble on the data bus in 4 bit mode.
|*
|* PARAMETERS
|*
|*    byteToSend - single byte to send
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_sendNibble(char nibbleToSend)
{
   /* Clear out all data pins */
   LCD_OUT_DATA &= ~(LCD_MASK_DATA);

   /* Set the nibble */
   LCD_OUT_DATA |= nibbleToSend;

   /* Data lines to LCD now set up - tell it to read them */
   LCD_pulseEnablePin();
}

/*-------------------------------------------------------------------------*\
|* HELPER FUNCTION :: LCD_pulseEnablePin
|*
|*    This function must be called whenever the LCD needs
|*    to be told to scan it's data bus.
|*
|* PARAMETERS
|*
|*    void
|*
|* RETURN
|*
|*    void
|*
\*-------------------------------------------------------------------------*/
void LCD_pulseEnablePin(void)
{
   /* Pull EN bit low */
//   LCD_OUT_EN &= ~LCD_PIN_EN;
//   delay_ms(20);

   /* Pull EN bit high */
   LCD_OUT_EN |= LCD_PIN_EN;
   delay_ms(1);

   /* Pull EN bit low again */
   LCD_OUT_EN &= ~LCD_PIN_EN;
   delay_ms(1);
}

// vim: tabstop=3 expandtab shiftwidth=3 softtabstop=3
