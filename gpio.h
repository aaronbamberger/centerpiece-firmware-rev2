/* 
 * File:   gpio.h
 * Author: Aaron
 *
 * Created on March 26, 2016, 12:56 PM
 */

#ifndef GPIO_H
#define	GPIO_H

#include <pic16f1788.h>

#define USER_LED_DIRECTION TRISBbits.TRISB4
#define USER_LED LATBbits.LATB4

#define PWM_ENABLE_DIRECTION TRISCbits.TRISC0
#define PWM_ENABLE LATCbits.LATC0

#ifdef	__cplusplus
extern "C" {
#endif

void init_gpio();

#ifdef	__cplusplus
}
#endif

#endif	/* GPIO_H */

