#include "adc.h"

#include <pic16f1788.h>

void init_adc()
{
    // Configure ADC inputs
    // Configure Port A pins 0, 1, 2, 3 to be inputs
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;
    // Configure AN0, AN1, AN2, AN3 to be analog inputs
    ANSELAbits.ANSA0 = 1;
    ANSELAbits.ANSA1 = 1;
    ANSELAbits.ANSA2 = 1;
    ANSELAbits.ANSA3 = 1;
    // Configure conversion clock to Fosc/16 (minimum allowed conversion clock with 16MHz system clock)
    ADCON1bits.ADCS = 0x5;
    // Set up ADC voltage references
    ADCON1bits.ADPREF = 0; // Positive reference is Vdd
    ADCON1bits.ADNREF = 0; // Negative reference is Vss
    // Disable auto conversion trigger
    ADCON2bits.TRIGSEL = 0;
    // Set negative differential input to ADC negative reference
    ADCON2bits.CHSN = 0xF;
    // Set ADC format to 2's complement
    ADCON1bits.ADFM = 1;
    // Set ADC 12-bit result
    ADCON0bits.ADRMD = 0;
    // Select channel AN0 as first conversion channel
    ADCON0bits.CHS = 0;
    // Enable ADC module
    ADCON0bits.ADON = 1;
    // Clear ADC interrupt flag
    PIR1bits.ADIF = 0;
    // Enable ADC interrupt
    PIE1bits.ADIE = 1;
}

void start_adc_conversion()
{
    ADCON0bits.GO_nDONE = 1;
}
