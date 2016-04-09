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

typedef void (*SPITransactionCallback)(struct _SPITransaction* transaction, void* user_arg);

typedef struct _SPITransaction {
    uint8_t* send_buf;
    uint8_t* recv_buf;
    int send_count;
    int recv_count;
    SPITransactionCallback complete_callback;
    void* user_arg;
} SPITransaction;

#ifdef	__cplusplus
extern "C" {
#endif

bool is_transaction_in_progress();
void update_transaction_state(bool in_progress);
    
#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

