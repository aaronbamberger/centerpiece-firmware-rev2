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

static void send_byte(uint8_t byte_to_send);
static uint8_t recv_byte();

#ifdef __cplusplus
}
#endif

#define INTERNAL_BUF_SIZE 10

volatile bool transaction_in_progress = false;
SPITransaction current_transaction;
uint8_t current_send_buf[INTERNAL_BUF_SIZE];
uint8_t current_recv_buf[INTERNAL_BUF_SIZE];
int num_bytes_written = 0;
int num_bytes_read = 0;

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

bool start_spi_transaction(SPITransaction* transaction)
{
    if (is_transaction_in_progress()) {
        return false;
    }
    
    // Copy the transaction to the internal transaction
    memcpy(&current_transaction, transaction, sizeof(SPITransaction));
    num_bytes_written = 0;
    num_bytes_read = 0;
    
    // Copy the send and receive buffer contents to the internal send and receive buffers
    memcpy(current_send_buf, transaction->send_buf, min(transaction->send_count, INTERNAL_BUF_SIZE));
    memcpy(current_recv_buf, transaction->recv_buf, min(transaction->recv_count, INTERNAL_BUF_SIZE));
    
    if (transaction->send_count > 0) {
        // Mark the transaction as having started
        update_transaction_state(true);
        // Start the transaction by sending the 1st byte
        // This will cause an interrupt when it completes
        send_byte(current_send_buf[num_bytes_written++]);
        
        return true;
    } else {
        return false;
    }
}

void update_transaction_state(bool in_progress)
{
    disable_global_interrupts();
    transaction_in_progress = in_progress;
    enable_global_interrupts();
}

bool is_transaction_in_progress()
{
    disable_global_interrupts();
    bool result = transaction_in_progress;
    enable_global_interrupts();
    
    return result;
}

void continue_transaction()
{
    if (num_bytes_read < current_transaction.recv_count) {
        current_recv_buf[num_bytes_read++] = recv_byte();
    }
    
    if (num_bytes_written < current_transaction.send_count) {
        send_byte(current_send_buf[num_bytes_written++]);
    } else {
        
    }
}

static void send_byte(uint8_t byte_to_send)
{
    SSP1BUF = byte_to_send;
}

static uint8_t recv_byte()
{
    return SSP1BUF;
}