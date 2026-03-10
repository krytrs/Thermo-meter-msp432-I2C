/*
 * I2C.c
 *
 *  Created on: 4. 4. 2016
 *      Author: Krytrs
 *
 *  I2C master driver for MSP432 eUSCI_B1 peripheral.
 */

#include "msp.h"
#include "I2C.h"

void I2C_init(void)
{
    UCB1CTLW0 |= UCSWRST;  /* Hold peripheral in reset while configuring */

    /*
     * Configure as:
     *   - Single master, master mode, I2C mode
     *   - 7-bit own address and slave address
     *   - Clock source: SMCLK
     */
    UCB1CTLW0 |= UCMST | UCMODE_3 | UCSSEL__SMCLK;

    /* Set P6.5 (SDA) to secondary function (I2C) */
    P6SEL0 |=  (1U << 5U);
    P6SEL1 &= ~(1U << 5U);

    /* Set P6.4 (SCL) to secondary function (I2C) */
    P6SEL0 |=  (1U << 4U);
    P6SEL1 &= ~(1U << 4U);

    /* Clock prescaler for 100 kHz (SMCLK / 30) */
    UCB1BRW = 30U;
}

void I2C_disable(void)
{
    UCB1CTLW0 |= UCSWRST;   /* Assert software reset */
}

void I2C_enable(void)
{
    UCB1CTLW0 &= ~UCSWRST;  /* Release software reset */
}

void I2C_setAddress(uint8_t slaveAddress)
{
    UCB1I2CSA = slaveAddress;
}

void I2C_setMode(uint8_t mode)
{
    /*
     * Select transmit or receive mode.
     * Pass BK_I2C_TRANSMIT or BK_I2C_RECEIVE.
     */
    UCB1CTLW0 &= ~BK_I2C_TRANSMIT;
    UCB1CTLW0 |=  (uint16_t)mode;
}

uint8_t I2C_isBusy(void)
{
    return (uint8_t)(UCB1STATW & UCBBUSY);
}

void I2C_masterReceiveStart(void)
{
    uint32_t timeout = BK_I2C_ADDR_TIMEOUT;

    I2C_enable();

    __disable_interrupt();      /* Protect START sequence */
    I2C_setMode(BK_I2C_RECEIVE);
    UCB1CTLW0 |= UCTXSTT;      /* Send START + slave address */
    __enable_interrupt();

    while (((UCB1CTLW0 & UCTXSTT) != 0U) && (timeout > 0U))
    {
        timeout--;              /* Guard against a hung bus */
    }
}

uint8_t I2C_masterReceived(void)
{
    return (uint8_t)UCB1RXBUF;
}

void I2C_masterStop(void)
{
    UCB1CTLW0 |= UCTXSTP;      /* Send STOP condition */
}

