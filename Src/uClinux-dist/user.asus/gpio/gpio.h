/*
 * GPIO char driver
 *
 * Copyright 2004, ASUSTek COMPUTER INC.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUSTek GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */
#ifndef __SPLINK_LED_H__
#define __SPLINK_LED_H__
 
#define LED_MAJOR	244
#define LED_NAME	"led"
 
#define LED_BLINK_MAX	5
 
#define LED_READY	0
#define LED_AIR_11A	1
#define LED_AIR_11B	2
#define LED_LPT  	3
#define LED_RST		4

 
#ifdef LIBERTAS
#define GPIO_BASE 	0x8000d000
#define GPIO_REG(offset) (GPIO_BASE|offset)
 
#define GPIO_REG_SELECT_0_7                     GPIO_REG(0x00)
#define GPIO_REG_SELECT_8_15                    GPIO_REG(0x04)
#define GPIO_REG_OUTPUT_ENABLE                  GPIO_REG(0x08)
#define GPIO_REG_OUTPUTS                        GPIO_REG(0x0C)
#define GPIO_REG_INPUTS                         GPIO_REG(0x10)
#define GPIO_REG_IER                            GPIO_REG(0x14)
#define GPIO_REG_IMR                            GPIO_REG(0x18)
#define GPIO_REG_ISR                            GPIO_REG(0x20)
#define SW_LED_OUTPUT_ENABLE                    GPIO_REG(0x24)
#define SW_LED_CYCLE_3_0                        GPIO_REG(0x28)
#define SW_LED_CYCLE_7_4                        GPIO_REG(0x2c)
#define SW_LED_CYCLE_11_8                       GPIO_REG(0x30)
#define SW_LED_CYCLE_15_12                      GPIO_REG(0x34)
#define SW_LED_DUTY_CYCLE_3_0                   GPIO_REG(0x38)
#define SW_LED_DUTY_CYCLE_7_4                   GPIO_REG(0x3C)
#define SW_LED_DUTY_CYCLE_11_8                  GPIO_REG(0x40)
#define SW_LED_DUTY_CYCLE_15_12                 GPIO_REG(0x44)
#define LED_OUTPUT_ENABLE_FOR_STATUS_SIGNALS    GPIO_REG(0x48)
#define LED_STRETCH_DURATION_FOR_STATUS_SIGNALS GPIO_REG(0x4c)
#define PCLK_CLOCK_DIVIDER_TO_GENERATE_LED_CLK  GPIO_REG(0x50)

#define SELECT_GPIO                             0x01
#define SELECT_SW_LED                           0x03

#define RATE_37MS                               0x00
#define RATE_74MS                               0x01
#define RATE_149MS                              0x02
#define RATE_298MS                              0x03
#define RATE_596MS                              0x04
#define RATE_1192MS                             0x05

#define DUTY_50_PERCENT                         0x00
#define DUTY_25_PERCENT                         0x01
#define DUTY_12_PERCENT                         0x02
#define DUTY_6_PERCENT                          0x03
#define DUTY_3_PERCENT                          0x04




#define LED_REG(x) (*(unsigned long *)x)

#define gpioSetRegBits(x, val) (LED_REG(x) |=  val)
#define gpioClrRegBits(x, val) (LED_REG(x) &= ~val)

#define gpioSetRegWithMask(x, val, mask) (LED_REG(x) = val | (~mask & LED_REG(x)))

void apctlLedOn(unsigned char port);
void apctlLedOff(unsigned char port);
void apctlLedBlinkStart(unsigned char port);
void apctlLedBlinkStop(unsigned char port);
void apctlLedSetRate(unsigned char port, unsigned long rate, unsigned long duty);

#define GPIO_GPIO0	0
#define GPIO_GPIO1	1
#define GPIO_GPIO5	5
#define GPIO_GPIO6	6
#define GPIO_GPIO10	10
#define GPIO_GPIO12	12
#define GPIO_GPIO15	15

#define GPIO_WLAN_TX 	2
#define GPIO_WLAN_RX 	2	// Fixed by PaN
#define GPIO_LAN_TX   	0
#define GPIO_LAN_RX  	0	// Fixed by PaN

#ifdef SNAPAP
#define GPIO_POWER   	GPIO_GPIO1
#else
#define GPIO_POWER	GPIO_GPIO6
#endif
#define GPIO_SELECT	13	// Added by PaN	
#define GPIO_SWITCH	11	// Added by PaN
#endif 
#endif
 
