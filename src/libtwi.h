/**
* \file
* \date 08.07.17
* \authors Alexander A. Kuzkin <xbevice@gmail.com>
*/

#include <avr/io.h>

#ifndef _LIBTWI_H
#define _LIBTWI_H

#define TWI_STATUS_OK           0x00
#define TWI_STATUS_OK_NOACK     0x01
#define TWI_STATUS_ERROR        0xFF

typedef void (*twi_callback_t)(uint8_t status);

void twi_receive_data(uint8_t addr, uint8_t count, uint8_t *buffer, twi_callback_t callback);

void twi_init();

void twi_transmit_data(uint8_t addr, uint8_t count, uint8_t *buffer, twi_callback_t callback);

void twi_receive_data_adr8(uint8_t addr, uint8_t reg_adr, uint8_t count, uint8_t *buffer, twi_callback_t callback);

void
twi_tx_rx_data(uint8_t addr, uint8_t tx_count, uint8_t *tx_data, uint8_t count, uint8_t *buffer, twi_callback_t cb);

uint8_t twi_ready();

void twi_force_stop();

void twi_reset_line();

#endif //_LIBTWI_H
