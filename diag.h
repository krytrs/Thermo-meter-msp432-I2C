/*
 * diag.h
 *
 *  Created on: 3. 4. 2016
 *      Author: Petr Gallistl
 *
 *  Startup diagnostics: LED blink animation and hardware self-test.
 */

#ifndef DIAG_H_
#define DIAG_H_

#include <stdint.h>

/* Diagnostic status codes returned by diag() */
#define DK_DIAG_STATUS_OK       ((uint8_t)0U)  /* All tests passed          */
#define DK_DIAG_STATUS_I2C_E    ((uint8_t)1U)  /* I2C sensor not detected   */
#define DK_DIAG_STATUS_LCD_E    ((uint8_t)2U)  /* LCD read-back mismatch    */
#define DK_DIAG_STATUS_CHYBA    ((uint8_t)3U)  /* General / unknown error   */
#define DK_DIAG_STATUS_MEM_E    ((uint8_t)4U)  /* SRAM test failure         */
#define DK_DIAG_STATUS_RESET    ((uint8_t)5U)  /* Previous hard-reset event */

#define DK_DIAG_WTD_TIME        ((uint8_t)10U) /* Watchdog timeout in seconds */
#define DK_SENSOR_ADDRESS       ((uint8_t)0x48U) /* I2C address of temperature sensor */
#define MSK_I2C_ADDR            ((uint16_t)0x03FFU) /* Mask for UCBxADDRX address bits */

/* Public function */
uint8_t diag(void);

#endif /* DIAG_H_ */


