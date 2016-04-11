/* 
 * File:   spi.h
 * Author: Aaron
 *
 * Created on March 26, 2016, 1:14 PM
 */

#ifndef SPI_H
#define	SPI_H

#include <stdint.h>
#include <stdbool.h>
#include <pic16f1788.h>

#define SPI_SCK_DIRECTION TRISCbits.TRISC3
#define SPI_MOSI_DIRECTION TRISCbits.TRISC5
#define SPI_MISO_DIRECTION TRISCbits.TRISC4
#define SPI_CS_DIRECTION TRISCbits.TRISC7
#define SPI_CS LATCbits.LATC7

typedef struct {
    uint8_t* send_buf;
    uint8_t* recv_buf;
    int send_count;
    int recv_count;
} BlockingSPITransaction;

#ifdef	__cplusplus
extern "C" {
#endif

void init_spi();
void perform_blocking_spi_transaction(BlockingSPITransaction* transaction);
void enable_spi_interrupt();
void assert_spi_cs();
void deassert_spi_cs();
void send_spi_byte(uint8_t byte_to_send);
uint8_t read_spi_byte();
    
#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

