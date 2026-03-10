/*
 * I2C.h
 *
 *  Created on: 4. 4. 2016
 *      Author: Krytrs
 *
 *  I2C master driver for MSP432 eUSCI_B1 peripheral.
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

#define BK_I2C_TRANSMIT    ((uint16_t)0x0010U) /* UCTR bit: transmit mode */
#define BK_I2C_RECEIVE     ((uint16_t)0x0000U) /* UCTR=0:  receive mode  */
#define BK_I2C_ADDR_TIMEOUT ((uint32_t)10000U) /* Max iterations waiting for address phase */

/* Public function prototypes */
void    I2C_init(void);
void    I2C_disable(void);
void    I2C_enable(void);
void    I2C_setMode(uint8_t mode);
void    I2C_setAddress(uint8_t slaveAddress);
void    I2C_masterReceiveStart(void);
uint8_t I2C_masterReceived(void);
void    I2C_masterStop(void);
uint8_t I2C_isBusy(void);

#endif /* I2C_H_ */
