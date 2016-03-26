#include "timers.h"

#include <pic16f1788.h>

void init_timers()
{
    // Configure Timer 0
    OPTION_REGbits.TMR0CS = 0; // Internal clock source for timer 0 (Fosc/4, Fosc = 16MHz)
    OPTION_REGbits.PSA = 0; // Assign prescaler to Timer 0
    OPTION_REGbits.PS = 0x7; // Set prescaler to 256
    INTCONbits.TMR0IF = 0; // Clear timer 0 interrupt flag
    INTCONbits.TMR0IE = 1; // Enable timer 0 interrupt
    
    // Configure Timer 1
    // Set timer 1 clock source to system clock
    T1CONbits.TMR1CS = 1;
    // Set 1:1 prescaler
    T1CONbits.T1CKPS = 0;
    // Clear timer 1 interrupt flag
    PIR1bits.TMR1IF = 0;
    // Enable timer 1 interrupt
    PIE1bits.TMR1IE = 1;
}
