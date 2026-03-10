/*
 * tests/msp.h
 *
 * Host-side mock of the MSP432 hardware header ("msp.h").
 * Replaces the TI SDK header during unit testing so that source
 * files can be compiled and exercised on the development host.
 *
 * Every hardware register is backed by a plain global variable
 * (defined in msp_mock.c).  Peripheral structs (NVIC, SYSCTL, …)
 * are thin wrappers around global mock instances.
 * Compiler intrinsics (__wfi, __enable_irq, …) are macros that
 * invoke the test-side simulation helpers.
 */

#ifndef MSP_H_
#define MSP_H_

#include <stdint.h>

/* ------------------------------------------------------------------ *
 *  Port registers                                                      *
 * ------------------------------------------------------------------ */
extern uint8_t  P1DIR, P1OUT, P1REN, P1IES, P1IE, P1IFG;
extern uint8_t  P2DIR, P2OUT;
extern uint8_t  P4DIR, P4OUT, P4IN, P4IFG;
extern uint8_t  P6SEL0, P6SEL1;

/* ------------------------------------------------------------------ *
 *  eUSCI_B1 (I2C) registers                                           *
 * ------------------------------------------------------------------ */
extern uint16_t UCB1CTLW0;
extern uint16_t UCB1I2CSA;
extern uint16_t UCB1BRW;
extern uint16_t UCB1STATW;
extern uint8_t  UCB1RXBUF;
extern uint16_t UCB1ADDRX;
extern uint16_t UCB1IFG;

/* ------------------------------------------------------------------ *
 *  Timer A registers (TA0–TA3)                                        *
 * ------------------------------------------------------------------ */
extern uint16_t TA0CCR0, TA0CCTL0, TA0CTL, TA0R;
extern uint16_t TA1CCR0, TA1CCTL0, TA1CTL, TA1R;
extern uint16_t TA2CCR0, TA2CCTL0, TA2CTL, TA2R;
extern uint16_t TA3CCR0, TA3CCTL0, TA3CTL, TA3R;

/* ------------------------------------------------------------------ *
 *  Peripheral structs                                                  *
 * ------------------------------------------------------------------ */

/* NVIC */
typedef struct { uint32_t ISER[2]; } NVIC_Type;
extern NVIC_Type mock_NVIC;
#define NVIC (&mock_NVIC)

/* SYSCTL */
typedef struct {
    uint32_t SRAM_BANKEN;
    uint32_t SECDATA_UNLOCK;
} SYSCTL_Type;
extern SYSCTL_Type mock_SYSCTL;
#define SYSCTL (&mock_SYSCTL)

/* FLCTL */
typedef struct {
    uint32_t PRGBRST_CTLSTAT;
    uint32_t BANK0_MAIN_WEPROT;
    uint32_t BANK1_MAIN_WEPROT;
} FLCTL_Type;
extern FLCTL_Type mock_FLCTL;
#define FLCTL (&mock_FLCTL)

/* MPU */
typedef struct { uint32_t RASR; } MPU_Type;
extern MPU_Type mock_MPU;
#define MPU (&mock_MPU)

/* RSTCTL */
typedef struct {
    uint32_t HARDRESET_STAT;
    uint32_t HARDRESET_CLR;
    uint32_t HARDRESET_SET;
    uint32_t RESET_REQ;
} RSTCTL_Type;
extern RSTCTL_Type mock_RSTCTL;
#define RSTCTL (&mock_RSTCTL)

/* Watchdog */
extern uint32_t WDTCTL;

/* ------------------------------------------------------------------ *
 *  Bit-field / constant definitions                                   *
 * ------------------------------------------------------------------ */

/* Port bits */
#define BIT0  ((uint8_t)0x01U)
#define BIT1  ((uint8_t)0x02U)
#define BIT2  ((uint8_t)0x04U)
#define BIT3  ((uint8_t)0x08U)
#define BIT4  ((uint8_t)0x10U)
#define BIT5  ((uint8_t)0x20U)
#define BIT6  ((uint8_t)0x40U)
#define BIT7  ((uint8_t)0x80U)

/* eUSCI_B CTLW0 bits */
#define UCSWRST         ((uint16_t)0x0001U)  /* Software reset          */
#define UCMST           ((uint16_t)0x0800U)  /* Master mode             */
#define UCMODE_3        ((uint16_t)0x0600U)  /* I2C mode                */
#define UCSSEL__SMCLK   ((uint16_t)0x0080U)  /* SMCLK clock source      */
#define UCTXSTT         ((uint16_t)0x0002U)  /* Generate START          */
#define UCTXSTP         ((uint16_t)0x0004U)  /* Generate STOP           */

/* eUSCI_B STATW bits */
#define UCBBUSY         ((uint16_t)0x0010U)  /* Bus busy                */

/* Timer A control bits */
#define CCIE                    ((uint16_t)0x0010U)  /* CC interrupt enable  */
#define CCIFG                   ((uint16_t)0x0001U)  /* CC interrupt flag    */
#define MC_3                    ((uint16_t)0x0030U)  /* Mode control mask    */
#define TIMER_A_CTL_TASSEL_2    ((uint16_t)0x0200U)  /* Clock: SMCLK         */
#define TIMER_A_CTL_SSEL__ACLK  ((uint16_t)0x0100U)  /* Clock: ACLK          */
#define TIMER_A_CTL_MC__UP      ((uint16_t)0x0010U)  /* Mode: up             */
#define TIMER_A_CTL_ID_0        ((uint16_t)0x0000U)  /* Divider: /1          */
#define TIMER_A_CTL_ID_1        ((uint16_t)0x0040U)  /* Divider: /2          */
#define TIMER_A_CTL_ID_3        ((uint16_t)0x00C0U)  /* Divider: /8          */

/* System / memory constants */
#define SYSCTL_SRAM_BANKEN_BNK7_EN      ((uint32_t)0x0080U)
#define FLCTL_PRG_CTLSTAT_MODE          ((uint32_t)0x0006U)
#define FLCTL_PRG_CTLSTAT_ENABLE        ((uint32_t)0x0001U)
#define MPU_RASR_AP_PRV_RW_USR_RW       ((uint32_t)0x00300000U)
#define RSTCTL_HARDRESET_SET_SRC1       ((uint32_t)0x0002U)

/* Watchdog */
#define WDTPW    ((uint32_t)0x5A000000U)
#define WDTHOLD  ((uint32_t)0x0080U)

/* IRQ numbers */
typedef enum {
    TA0_0_IRQn  =  8,
    TA1_0_IRQn  = 10,
    TA2_0_IRQn  = 12,
    TA3_0_IRQn  = 14,
    PORT1_IRQn  = 35
} IRQn_Type;

/* ------------------------------------------------------------------ *
 *  Compiler intrinsics — replaced with test-side hooks                *
 * ------------------------------------------------------------------ */

/*
 * mock_tick() is the simulation hook called instead of __wfi().
 *
 * A weak default implementation (no-op) lives in msp_mock.c.
 * Tests that exercise delay_ms() provide a strong override that
 * advances the timer by calling TA0_ISR_Handler() and, optionally,
 * captures LCD nibbles sent while EN is high.
 */
void mock_tick(void);

#define __wfi()               mock_tick()
#define __enable_irq()        ((void)0)
#define __disable_interrupt() ((void)0)
#define __enable_interrupt()  ((void)0)
#define __deep_sleep()        while (1)

/* Cycle-delay intrinsic used in libLCD.h macro */
#define __delay_cycles(x)     ((void)(x))

#endif /* MSP_H_ */
