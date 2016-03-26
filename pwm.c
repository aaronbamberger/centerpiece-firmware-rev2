#include "pwm.h"

#include <pic16f1788.h>

void init_pwm()
{
    // Make sure PWM outputs are on the correct pins
    APFCON1bits.CCP1SEL = 0; // CCP1 is on RC2
    APFCON1bits.CCP2SEL = 0; // CCP2 is on RC1
    APFCON2bits.CCP3SEL = 0; // CCP3 is on RC6
    
    // Configure PWM Channels
    // Disable gpio output drivers on PWM channels 1, 2 and 3
    TRISCbits.TRISC2 = 1; // CCP1
    TRISCbits.TRISC1 = 1; // CCP2
    TRISCbits.TRISC6 = 1; // CCP3
    // Configure PWM period register for maximum resolution (10 bits)
    PR2 = 0xFF;
    // Set CCP1, CCP2, CCP3 for PWM mode
    CCP1CONbits.CCP1M = 0xF;
    CCP2CONbits.CCP2M = 0xF;
    CCP3CONbits.CCP3M = 0xF;
    // Start all PWM duty cycles at 0 to begin with
    CCPR1L = 0x0;
    CCP1CONbits.DC1B = 0x0;
    CCPR2L = 0x0;
    CCP2CONbits.DC2B = 0x0;
    CCPR3L = 0x0;
    CCP3CONbits.DC3B = 0x0;
    // Clear timer 2 interrupt flag
    PIR1bits.TMR2IF = 0;
    // Configure timer 2 prescaler to 1:1
    T2CONbits.T2CKPS = 0;
    // Enable timer 2
    T2CONbits.TMR2ON = 1;
    // Enable PWM output drivers
    TRISCbits.TRISC2 = 0; // CCP1
    TRISCbits.TRISC1 = 0; // CCP2
    TRISCbits.TRISC6 = 0; // CCP3
}
