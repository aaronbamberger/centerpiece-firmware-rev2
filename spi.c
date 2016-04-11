#include "spi.h"
#include "util.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pic16f1788.h>

#ifdef __cplusplus
extern "C" {
#endif

static bool spi_rx_tx_complete();
    
#ifdef __cplusplus
}
#endif

void init_spi()
{
    // Make sure SPI lines are routed to the correct pins
    APFCON1bits.SDOSEL = 0; // SDO (MOSI) is on RC5
    APFCON1bits.SDISEL = 0; // SDI (MISO) is on RC4
    APFCON1bits.SCKSEL = 0; // SCK is on RC3
    APFCON2bits.SSSEL = 0; // ~SS is on RA5 (unused pin, since we're not using SPI in slave mode)
    
    // Init SPI line directions
    SPI_MISO_DIRECTION = 1; // MISO input
    SPI_MOSI_DIRECTION = 0; // MOSI output
    SPI_SCK_DIRECTION = 0; // SCK output
    SPI_CS_DIRECTION = 0; // ~CS output
    SPI_CS = 1; // Set chip select inactive
    
    SSPCON1bits.CKP = 1; // Clock idles high
    SSPSTATbits.CKE = 0; // Transmit occurs on falling edge (slave samples on rising edge)
    SSPSTATbits.SMP = 1; // Receive occurs on rising edge
    SSPCON1bits.SSPM = 0xA; // SPI master mode, clock = Fosc / (4 * SSPADD + 1)
    SSPADD = 15; // Fosc = 32MHz, SPI clock = 32MHz / (4 * (15 + 1)), SPI clock = 500kHz
    
    PIR1bits.SSP1IF = 0; // Clear the SPI interrupt flag
    //PIE1bits.SSP1IE = 1; // Enable the SPI interrupt
    
    SSPCON1bits.SSPEN = 1; // Enable the SPI peripheral
}

void perform_blocking_spi_transaction(BlockingSPITransaction* transaction)
{   
    // Dummy read to clear the buffer full flag
    uint8_t dummy = SSPBUF;
    
    // Enable slave select
    SPI_CS = 0;
    
    // Keep going until we've sent and received all requested data
    int bytes_sent = 0;
    int bytes_recvd = 0;
    int send_count = transaction->send_count;
    int recv_count = transaction->recv_count;
    while ((send_count > 0) || (recv_count > 0)) {
        if (send_count > 0) {
            SSPBUF = transaction->send_buf[bytes_sent];
            send_count--;
        } else {
            SSPBUF = 0;
        }
        bytes_sent++;
        
        while (!spi_rx_tx_complete())
        {}
        
        uint8_t recvd_byte = SSPBUF;
        if (recv_count > 0) {
            transaction->recv_buf[bytes_recvd] = recvd_byte;
            recv_count--;
        }
        bytes_recvd++;
    }
    
    // Disable slave select
    SPI_CS = 1;
    
    PIR1bits.SSP1IF = 0; // Clear the SPI interrupt flag
}

void enable_spi_interrupt()
{
    PIR1bits.SSP1IF = 0; // Clear the SPI interrupt flag
    PIE1bits.SSP1IE = 1; // Enable the SPI interrupt
}

void assert_spi_cs()
{
    SPI_CS = 0;
}

void deassert_spi_cs()
{
    SPI_CS = 1;
}

void send_spi_byte(uint8_t byte_to_send)
{
    SSPBUF = byte_to_send;
}

uint8_t read_spi_byte()
{
    return SSPBUF;
}

static bool spi_rx_tx_complete()
{
    return (SSPSTATbits.BF == 1);
}