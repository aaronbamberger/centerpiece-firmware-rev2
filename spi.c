#include "spi.h"

#include <pic16f1788.h>

void init_spi()
{
    // Make sure SPI lines are routed to the correct pins
    APFCON1bits.SDOSEL = 0; // SDO (MOSI) is on RC5
    APFCON1bits.SDISEL = 0; // SDI (MISO) is on RC4
    APFCON1bits.SCKSEL = 0; // SCK is on RC3
    APFCON2bits.SSSEL = 0; // ~SS is on RA5 (unused pin, since we're not using SPI in slave mode)
    
    // Init SPI line directions
    TRISCbits.TRISC4 = 1; // MISO
    TRISCbits.TRISC5 = 0; // MOSI
    TRISCbits.TRISC3 = 0; // SCK
    TRISCbits.TRISC7 = 0; // ~CS
    
    SSPCON1bits.CKP = 1; // Clock idles high
    SSPSTATbits.CKE = 0; // Transmit occurs on falling edge (slave samples on rising edge)
    SSPSTATbits.SMP = 1; // Receive occurs on rising edge
    SSPCON1bits.SSPM = 0xA; // SPI master mode, clock = Fosc / (4 * SSPADD + 1)
    SSPADD = 15; // Fosc = 32MHz, SPI clock = 32MHz / (4 * (15 + 1)), SPI clock = 500kHz
    
    PIR1bits.SSP1IF = 0; // Clear the SPI interrupt flag
    PIE1bits.SSP1IE = 1; // Enable the SPI interrupt
    
    SSPCON1bits.SSPEN = 1; // Enable the SPI peripheral
}