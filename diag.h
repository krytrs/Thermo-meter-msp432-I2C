/*
 * diag.h
 *
 *  Created on: 3. 4. 2016
 *      Author: Petr Gallistl
 *      this function wil flashing with a led and will do diagnostick
 *
 */

#ifndef diag_H_
#define diag_H_

#define DK_DIAG_STATUS_OK       0        // diagnostika skoncila stavem v poradku
#define DK_DIAG_STATUS_I2C_E    1        // chzba I2C
#define DK_DIAG_STATUS_LCD_E    2        // diagnostika displeje skoncila neuspesne
#define DK_DIAG_STATUS_CHYBA    3        // diagnostika skoncila neuspesne
#define DK_DIAG_STATUS_MEM_E	4		 // diagnostika skoncila chybou testu pameti
#define DK_DIAG_STATUS_RESET    5        // diagnostika skoncila resetem

#define DK_DIAG_WTD_TIME        10        // pocet vterin pro WTD v diagnostice, za jak dlouho se ma diagnostika stihnout
#define DK_SENSOR_ADDRESS       0x48     // adresa senzoru na i2c
#define MSK_I2C_ADDR            0x3ff    // vymaskovani bitu pro ziskani adresy z registru UCBxADDRX

// zadefinovani verejnych funkci
int diag(void);

#endif /* diag_H_ */


