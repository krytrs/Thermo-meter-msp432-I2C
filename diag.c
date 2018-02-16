/*
 * diag.c
 *
 *  Created on: 3. 4. 2016
 *      Author: Petr Gallistl
 *      this function will flashing with a led and provide diagnosis
 *
 */
#include "msp.h"
#include "diag.h"
#include "libLCD.h"
#include "I2C.h"

// zkouska

// zadefinovani vnitrnich funkci
int diag(void);

uint16_t delka_PWM = 200;
uint8_t smer_PWM = 0;
uint8_t count = 0;
uint8_t D_diag_status;
uint8_t pocetTestovani;
char D_receiveLCD = 0;
uint8_t WTDdiagnostika = 0;

#define PORTBASE 0x20006000
#define SRAM_konec 0x20007000
unsigned int volatile * pointR = (unsigned int *) PORTBASE;


int i = 0;
char LCD_testovaciSekvence1[16] = "Toto je testovac";
char LCD_testovaciSekvence2[16] = "i sekvence znaku";

int diag(void)
{
    // nastaveni io povoleni modre cervene a modre LED
    P2DIR |= BIT0 + BIT1 + BIT2;
    P2OUT &= ~BIT0;
    P2OUT &= ~BIT1;
    P2OUT &= ~BIT2;


    /* Pokud v predeslem spusteni byl program opu�t�n SW hard resetem (surce 1)
    *  tak smaze obrazovku a vyp�se hla�ku, �e chyba nastala
    *  resetem
    */
    if ((RSTCTL->HARDRESET_STAT & RSTCTL_HARDRESET_SET_SRC1) == RSTCTL_HARDRESET_SET_SRC1)
    {
        LCD_clearScreen();
        LCD_setCursorPosition(0, 0);
        LCD_printStr("Nastala chyba");
        LCD_setCursorPosition(1, 0);
        LCD_printStr("RESET");
        RSTCTL->HARDRESET_CLR = 0xFFFFFFFF;
        return DK_DIAG_STATUS_RESET;
    }


    // pouzivam jako wtd pro diagnostiku
    // nastaveni citace 3 na jednu sec
    TA3CCR0 = 4096;
    TA3CCTL0 |= CCIE;       // povoleni interuptu
    // nastaveni zdroje hodin na hlavni hodiny 32786Hz, nastaveni citace na citani nahoru, nastaveni preddelickz na 8
    TA3CTL |= TIMER_A_CTL_SSEL__ACLK | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_3;
    NVIC->ISER[0] |= 1 << TA3_0_IRQn;   // nastaveni Nvic na povoleni interuptu

    // nasledujici citace jsou pro pwm

    // tento citac se pouziva pro nastaveni stridy PWM
    // nastaveni citace 2
    TA2CCR0 = delka_PWM;
    TA2CCTL0 |= CCIE;       // povoleni interuptu
    // nastaveni zdroje hodin na hlavni hodiny 3MHz, nastaveni citace na citani nahoru, nastaveni preddelickz na 2
    TA2CTL |= TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_1;
    NVIC->ISER[0] |= 1 << TA2_0_IRQn;   // nastaveni Nvic na povoleni interuptu

    // tento citac se pouziva pro periodu PWM
    // nastaveni citace 1
    TA1CCR0 = 1500;
    TA1CCTL0 |= CCIE;       // povoleni interuptu
    // nastaveni zdroje hodin na hlavni hodiny 3MHz, nastaveni citace na citani nahoru, nastaveni preddelickz na 2
    TA1CTL |= TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_MC__UP | TIMER_A_CTL_ID_1;
    NVIC->ISER[0] |= 1 << TA1_0_IRQn;   // nastaveni Nvic na povoleni interuptu

    __enable_irq();
    __enable_interrupt();


    // vypise hlasku Spou�t�m diagnostiku
    LCD_clearScreen();
    LCD_setCursorPosition(0, 0);
    LCD_printStr("Spoustim diag");
    delay_ms(500);


	count = 0;                              // nastaveni poctu smycek blikani

	D_diag_status = DK_DIAG_STATUS_OK;   // vybiram 1 z N


	pocetTestovani = 0;
	while (count < 2)
	{
	    // aby testovani probehlo jen jednou
	  if (pocetTestovani == 0)
	  {
	      pocetTestovani++;

            // sem prijde kod pro diagnostiku soucastek

	      	SYSCTL->SRAM_BANKEN = SYSCTL_SRAM_BANKEN_BNK7_EN;   // Enable all SRAM banks
	      	SYSCTL->SECDATA_UNLOCK = 0x695A;
	      	FLCTL->PRGBRST_CTLSTAT &= ~FLCTL_PRG_CTLSTAT_MODE;
	      	FLCTL->PRGBRST_CTLSTAT |= FLCTL_PRG_CTLSTAT_ENABLE;
          	FLCTL->BANK0_MAIN_WEPROT = 0x0000;
          	FLCTL->BANK1_MAIN_WEPROT = 0x0000;
	      	MPU->RASR |= MPU_RASR_AP_PRV_RW_USR_RW;

	      /* Testovani SRAM
	       * Tato funkce zapise do bloku v pameti sram nuly, ktere pak precte, nasledne zapise 1 a take je precte
	       */

	      for (pointR = PORTBASE; pointR < SRAM_konec; pointR++)
	      	      	{
	      	      		*pointR = 0x00000000;
	      	      	}

	      for (pointR = PORTBASE; pointR < SRAM_konec; pointR++)
	      	      	{
	    	  	  	  if (*pointR != 0x00000000)
	    	  	  	  {
	    	  	  		  D_diag_status = DK_DIAG_STATUS_MEM_E;
	    	  	  	  }
	      	      	}

	      for (pointR = PORTBASE; pointR < SRAM_konec; pointR++)
	      	      	{
	      	      		*pointR = 0xFFFFFFFF;
	      	      	}

	      for (pointR = PORTBASE; pointR < SRAM_konec; pointR++)
	      	      	{
	    	  	  	  if (*pointR != 0xFFFFFFFF)
	    	  	  	  {
	    	  	  		  D_diag_status = DK_DIAG_STATUS_MEM_E;
	    	  	  	  }
	      	      	}




	        /* Testovani pritomnosti cidla in I2C
	         *
	         * Je proveden �tec� impuls z I2C cidla a pokud
	         * je vr�cena nenulova navratova adresa tak je
	         * vyhodnoceno ze je cidlo pritomne. Pokud ne, tak
	         * je vracen chybovy stav I2C
	         */

            I2C_masterReceiveStart();
            I2C_masterReceived();
            I2C_masterStop();

            if ((UCB1ADDRX & MSK_I2C_ADDR) == 0)
            {
                D_diag_status = DK_DIAG_STATUS_I2C_E;
            }

            /* Testov�n� dipleje
             *
             * Zde je na displej zapsana testovaci sekvence znaku
             * a nasledne je zpetne p�e�tena. Po p�e�ten� je porovnana
             * s zapsanou sekvenci, a pokud si odpovidaji, tak program
             * pokracuje. Pokud ne, tak je vracena hodnota chyba LCD
             */
            LCD_clearScreen();
            LCD_setCursorPosition(0,0);
            LCD_printStr(LCD_testovaciSekvence1);
            LCD_setCursorPosition(1,0);
            LCD_printStr(LCD_testovaciSekvence2);

            D_receiveLCD = 0;
            LCD_setCursorPosition(0,0);
            for (i = 0; i<15; i++)
            {
                D_receiveLCD = LCD_receive();
                if (D_receiveLCD != LCD_testovaciSekvence1[i])
                {
                    D_diag_status = DK_DIAG_STATUS_LCD_E;
                }
            }

            LCD_setCursorPosition(1,0);
            for (i = 0; i<15; i++)
            {
                D_receiveLCD = LCD_receive();
                if (D_receiveLCD != LCD_testovaciSekvence2[i])
                {
                    D_diag_status = DK_DIAG_STATUS_LCD_E;
                }
            }

	  }


	    __wfi(); // go to lpm0
	}

	/* Opousteni dignostiky
	 *
	 * Nejprve se ceka na probliknuti ledkou a pak zastavuji v�echny �ita�e
	 * a podle nacteneho chyboveho kodu opoustim diagnostiku a zobrazuji na
	 * display vysledek diagnostiky. Pokud vysledek diagnostiky je OK tak
	 * rozsvitim zelenou LED, pokud ne tak cervenou. Na z�v�r je pridana kratka
	 * doba proto aby se dala informase na displeji vubec precist
	 */
	while(count >= 2)
	{
				TA1CTL &= ~MC_3;					// zastavuji citac 0
				TA2CTL &= ~MC_3;					// zastavuji citac 1
                TA3CTL &= ~MC_3;                    // zastavuji citac 3     WTD pro diagnostiku
			    P2OUT = 0;                          // zhasnu modrou ledkou

		        switch(D_diag_status)
		        {
		        case DK_DIAG_STATUS_OK :
                    LCD_clearScreen();
                    LCD_setCursorPosition(0, 0);
                    LCD_printStr("Diag OK");
		            break;

                case DK_DIAG_STATUS_I2C_E :
                    LCD_clearScreen();
                    LCD_setCursorPosition(0, 0);
                    LCD_printStr("Nastala chyba");
                    LCD_setCursorPosition(1, 0);
                    LCD_printStr("I2C");
                    break;

                case DK_DIAG_STATUS_LCD_E :
                    LCD_clearScreen();
                    LCD_setCursorPosition(0, 0);
                    LCD_printStr("Nastala chyba");
                    LCD_setCursorPosition(1, 0);
                    LCD_printStr("LCD");
                    break;

                case DK_DIAG_STATUS_MEM_E :
                    LCD_clearScreen();
                    LCD_setCursorPosition(0, 0);
                    LCD_printStr("Nastala chyba");
                    LCD_setCursorPosition(1, 0);
                    LCD_printStr("SRAM");
                    break;


		        case DK_DIAG_STATUS_CHYBA :
		        default:
		            LCD_clearScreen();
		            LCD_setCursorPosition(0, 0);
		            LCD_printStr("Nastala neznama");
		            LCD_setCursorPosition(1, 0);
		            LCD_printStr("chyba");
                    break;
		        }
		        if (D_diag_status == DK_DIAG_STATUS_OK)
		            P2OUT |= BIT1;// tak rozsvit zelenou led
		        else
		            P2OUT |= BIT0;// tak rozsvit cervenou led

		        delay_ms(1000);
		        return D_diag_status;


	}
}

/* Timery 1 a 2 se pouzivaji pro generovani PWM
 *
 */


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
    TA1CTL &= ~MC_3;                    // zastavuji citac
    TA1CCTL0 &= ~CCIFG;                 // mazu flag interupt
    TA1R = 0;                           // nuluji registr citace

    TA1CTL |= TIMER_A_CTL_MC__UP;       // zapinam citac 1
    TA2CTL |= TIMER_A_CTL_MC__UP;       // zapinam citac 2
    P2OUT |= BIT2;                      // rozsvitim modrou ledkou

}

void TA3_ISR_Handler(void)
{
    TA3CTL &= ~MC_3;                    // zastavuji citac
    TA3CCTL0 &= ~CCIFG;                 // mazu flag interupt
    TA3R = 0;                           // nuluji registr citace

    /* Spusteni resetu
     * Tato funkce resi funkci WTD a to tak, ze pokud
     * Handler 1 Sec interuptu probehne toliktar kolik je uvedeno v
     * DK_DIAG_WTD_TIME tak se spusti reset a nastavi se priznak resetu
     * na source 1
     */
    if (WTDdiagnostika == DK_DIAG_WTD_TIME)            // pokud WTD dojde
    {
        D_diag_status = DK_DIAG_STATUS_CHYBA;   // nastavit status chybu, zbytecne
        RSTCTL->HARDRESET_SET = RSTCTL_HARDRESET_SET_SRC1;  // nastavit status reset
        RSTCTL->RESET_REQ = 0x6901;             // resetovat zarizeni
    }
    else
    WTDdiagnostika++;

    TA3CTL |= TIMER_A_CTL_MC__UP;       // zapinam citac 1


}



