/*
 * I2C.c
 *
 *  Created on: 4. 4. 2016
 *      Author: Krytrs
 */
#include "msp.h"
#include "libLCD.h"
#include "I2C.h"

void I2C_init(void);
void I2C_disable(void);
void I2C_enable(void);
void I2C_setMode(uint8_t I2C_mode);
void I2C_setAddress(unsigned char slaveAddress);
void I2C_masterReceiveStart(void);
unsigned char I2C_masterReceive(void);
void I2C_masterStop(void);

void I2C_init(void)
{


    UCB1CTLW0 |= UCSWRST; // hold in reset for setting
    /*
     * Seting
     * Own 7bit addr
     * Slave 7bit addr
     * Single master
     * Master mode
     * I2C mode
     * Clock source SMCLK
     */
    UCB1CTLW0 |= UCMST | UCMODE_3 | UCSSEL__SMCLK;

    // Seting P6.5 as secondary function, I2c
    P6SEL0 |= 1 << 5;
    P6SEL1 &=  ~(1 << 5);
    // Seting P6.4 as secondary function, I2c
    P6SEL0 |=  1 << 4;
    P6SEL1 &= ~(1 << 4);

    /*
     * Seting clock prescaler for 100kHz
     */
    UCB1BRW = 300;

}


void I2C_disable(void)
{
    // set reset bit in high
    UCB1CTLW0 |= UCSWRST;
}


void I2C_enable(void)
{
    // set reset bit to 0
    UCB1CTLW0 &= ~UCSWRST;
}

void I2C_setAddress(unsigned char slaveAddress)
{
    // set slave address to register
    UCB1I2CSA = slaveAddress;
}

void I2C_setMode(uint8_t I2C_mode)
{
    /* set mode of I2C transiver reciever
     * BK_I2C_TRANSMIT for transmit
     * BK_I2C_RECEIVE  for receive
    */
    UCB1CTLW0 &= ~BK_I2C_TRANSMIT;
    UCB1CTLW0 |=  I2C_mode;

}

uint8_t I2C_icBussy(void)
{
    // returning I2C bus status 
    return (UCB1STATW & UCBBUSY);
}

void I2C_masterReceiveStart(void)
{
    // enable i2c interface
    I2C_enable();

    // disable all interupts
    __disable_interrupt();

    // set i2c to receive mode
    I2C_setMode(BK_I2C_RECEIVE);

    // send start
    UCB1CTLW0 |= UCTXSTT;

    // wait until address is send
    while (UCB1CTLW0 & UCTXSTT);

    // disable all interupts
    __enable_interrupt();
}

unsigned char I2C_masterReceive()
{
    // will return receive buffer
    unsigned char buffer = UCB1RXBUF;
    return buffer;
}

void I2C_masterStop(void)
{
// send stop
UCB1CTLW0 |= UCTXSTP;
}

