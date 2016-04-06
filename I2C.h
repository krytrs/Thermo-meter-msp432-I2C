/*
 * I2C.h
 *
 *  Created on: 4. 4. 2016
 *      Author: Krytrs
 */

#ifndef I2C_H_
#define I2C_H_

#define BK_I2C_TRANSMIT    ((uint16_t)0x0010)
#define BK_I2C_RECEIVE     ((uint16_t)0x0000)

// define public function

void I2C_init(void);
void I2C_disable(void);
void I2C_enable(void);
void I2C_setMode(uint8_t I2C_mode);
void I2C_setAddress(unsigned char slaveAddress);
void I2C_masterReceiveStart(void);
unsigned char I2C_masterReceive(void);
void I2C_masterStop(void);

#endif /* I2C_H_ */
