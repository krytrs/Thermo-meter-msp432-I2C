/*
 * diag.c
 *
 *  Created on: 3. 4. 2016
 *      Author: Petr Gallistl
 *      this function wil flashing with a led
 *
 */

#include "diag.h"
#include "libLCD.h"
#include "msp.h"

// zadefinovani vnitrnich funkci
int diag(void);

uint16_t delka_PWM = 200;
uint8_t smer_PWM = 0;
uint8_t count = 0;
uint8_t D_diag_status;

int diag(void)
{
    LCD_clearScreen();
    LCD_setCursorPosition(0, 0);
    LCD_printStr("Spoustim diag");


	// nastaveni io
	P2DIR |= BIT0 + BIT1 + BIT2;
	P2OUT &= ~BIT0;
	P2OUT &= ~BIT1;
	P2OUT &= ~BIT2;

	// nastaveni citace 2
	TA2CCR0 = delka_PWM;
	TA2CCTL0 |= CCIE;		// povoleni interuptu
	// nastaveni zdroje hodin na pomocne hodiny 32768 Hz, nastaveni citace na citani nahoru, nastaveni preddelickz na 8
	TA2CTL |= TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_1;

	// nastaveni citace 0
	TA1CCR0 = 1500;
	TA1CCTL0 |= CCIE;		// povoleni interuptu
	// nastaveni zdroje hodin na pomocne hodiny 32768 Hz, nastaveni citace na citani nahoru, nastaveni preddelickz na 8
	TA1CTL |= TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_1;

	NVIC->ISER[0] |= 1 << TA2_0_IRQn;	// nastaveni Nvic na povoleni interuptu
	NVIC->ISER[0] |= 1 << TA1_0_IRQn;

	__enable_irq();
	__enable_interrupt();
	__wfi();


	count = 0;                              // nastaveni poctu smycek blikani
	D_diag_status = DK_DIAG_STATUS_CHYBA;   // vybiram 1 z N

	while (count < 2)
	{
	    // sem prijde kod pro diagnostiku soucastek
	    __wfi(); // go to lpm0



	}






	// Opousteni dignostiky
	while(count >= 2)
	{
				TA1CTL &= ~MC_3;					// zastavuji citac 0
				TA2CTL &= ~MC_3;					// zastavuji citac 1
			    P2OUT = 0;                          // zhasnu modrou ledkou

		        switch(D_diag_status)
		        {
		        case DK_DIAG_STATUS_OK :
                    P2OUT |= BIT1;// tak rozsvit zelenou led
                    LCD_clearScreen();
                    LCD_setCursorPosition(0, 0);
                    LCD_printStr("Diag OK");
                    delay_ms(1000);
                    return DK_DIAG_STATUS_OK; // opustit proceduru diagnostiky bez problemu
		            break;

		        case DK_DIAG_STATUS_CHYBA :
                    P2OUT |= BIT0;// tak rozsvit cervenou led
                    LCD_clearScreen();
                    LCD_setCursorPosition(0, 0);
                    LCD_printStr("Nastala chyba");
                    delay_ms(1000);
                    return DK_DIAG_STATUS_CHYBA; // opustit proceduru diagnostiky s chybovou hlaskou
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




void TA2_ISR_Handler(void)
{
	TA2CTL &= ~MC_3;					// zastavuji citac 0
	TA2CCTL0 &= ~CCIFG;					// mazu flag interuptu t0

	TA2R = 0;							// nuluji registr citace
    P2OUT &= ~BIT2;						// zhasnu modrou ledkou
    if (smer_PWM == 1)
    	delka_PWM++;  					// nastaveni doby sviceni
	if (smer_PWM == 0)
		delka_PWM--;
	if (delka_PWM >= 1450)              // pokud docitam nahoru prepnu smer
	{
		count++;                        // prictu jedna k dobe blikani smycky
		smer_PWM = 0;                   // zmenim smer
	}

	if (delka_PWM <= 100)               // otacim smer nahoru
	{
		smer_PWM = 1;
	}
	TA2CCR0 = delka_PWM; 				// nastaveni doby sviceni

}

void TA1_ISR_Handler(void)
{
	TA1CTL &= ~MC_3;					// zastavuji citac
	TA1CCTL0 &= ~CCIFG;					// mazu flag interupt
	TA1R = 0;							// nuluji registr citace

	TA1CTL |= TIMER_A_CTL_MC__UP;		// zapinam citac 1
	TA2CTL |= TIMER_A_CTL_MC__UP;		// zapinam citac 2
	P2OUT |= BIT2;						// rozsvitim modrou ledkou

}



