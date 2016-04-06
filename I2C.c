/*
 * I2C.c
 *
 *  Created on: 4. 4. 2016
 *      Author: Krytrs
 */
#include "msp.h"
#include "libLCD.h"
#include "I2C.h"

uint8_t I2C_init(void)
{
    // Seting P6.5 as secondary function, I2c
    P6SEL0 &= ~1 << 5;
    P6SEL1 |=  1 << 5;
    // Seting P6.4 as secondary function, I2c
    P6SEL0 &= ~1 << 4;
    P6SEL1 |=  1 << 4;

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

    /*
     * Seting clock prescaler for 100kHz
     */
    UCB1BRW = 30;
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








