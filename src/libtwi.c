/**
* \file
* \date 08.07.17
* \authors Alexander A. Kuzkin <xbevice@gmail.com>
*/

#include "libtwi.h"

#include <avr/interrupt.h>
#include "macro.h"
#include <util/delay.h>

#define TWI_START           0
#define TWI_RESTART         1
#define TWI_STOP            2
#define TWI_TRANSMIT        3
#define TWI_RECEIVE_ACK     4
#define TWI_RECEIVE_NACK    5

static volatile twi_callback_t current_callback = 0;

void twi(uint8_t action) {
    switch (action) {
        case TWI_START:
        case TWI_RESTART:
            TWCR = _BV(TWSTA) | _BV(TWEN) | _BV(TWINT) | _BV(TWIE);
            break;
        case TWI_STOP:
            TWCR = _BV(TWSTO) | _BV(TWEN) | _BV(TWINT) | _BV(TWIE);
            break;
        case TWI_TRANSMIT:
            TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWIE);
            break;
        case TWI_RECEIVE_ACK:
            TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA) | _BV(TWIE);
            break;
        case TWI_RECEIVE_NACK:
            TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWIE);
            break;
    }
//    if(action != TWI_STOP)while (!(TWCR & _BV(TWINT)));
//    return (TWSR & 0xF8);
}

uint8_t twi_main(uint8_t action) {
    switch (action) {
        case TWI_START:
        case TWI_RESTART:
            TWCR = _BV(TWSTA) | _BV(TWEN) | _BV(TWINT);
            break;
        case TWI_STOP:
            TWCR = _BV(TWSTO) | _BV(TWEN) | _BV(TWINT);
            break;
        case TWI_TRANSMIT:
            TWCR = _BV(TWEN) | _BV(TWINT);
            break;
        case TWI_RECEIVE_ACK:
            TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
            break;
        case TWI_RECEIVE_NACK:
            TWCR = _BV(TWEN) | _BV(TWINT);
            break;
    }
    if (action != TWI_STOP)while (!(TWCR & _BV(TWINT)));
    return (TWSR & 0xF8);
}

volatile uint8_t *current_buffer;
volatile uint8_t slave_addr;
volatile uint8_t rcv_bytes;
volatile uint8_t tx_bytes;
volatile uint8_t mode;;
volatile uint8_t *tx_buff;
volatile uint8_t tx_buff_count;
#define MODE_IDLE 0
#define MODE_RECEIVE 1
#define MODE_TRANSMIT 2
#define MODE_TX_RX 3
#define MODE_RESET 0xFF

void twi_receive_data(uint8_t addr, uint8_t count, uint8_t *buffer, twi_callback_t  cb) {
    if(!twi_ready())
        return;
    current_buffer = (volatile uint8_t *) buffer;
    slave_addr = addr | 1;
    rcv_bytes = count;
    mode = MODE_RECEIVE;
    current_callback = cb;
    twi(TWI_START);
}
volatile uint8_t twi_adr_tmp;
void twi_receive_data_adr8(uint8_t addr, uint8_t reg_adr, uint8_t count, uint8_t* buffer, twi_callback_t  callback){
    twi_adr_tmp = reg_adr;
    twi_tx_rx_data(addr, 1, (uint8_t *)&twi_adr_tmp, count, buffer,  callback);
}
void twi_tx_rx_data(uint8_t addr, uint8_t tx_count, uint8_t * tx_data, uint8_t count, uint8_t *buffer, twi_callback_t  cb) {
    if(!twi_ready())
        return;
    current_callback = cb;
    current_buffer = (volatile uint8_t *) buffer;
    slave_addr = addr & 0xFE; // адрес для записи
    rcv_bytes = count;
    mode = MODE_TX_RX;
    tx_buff = tx_data;
    tx_buff_count = tx_count;
    twi(TWI_START);
}

void twi_init() {

    TWSR &= ~(_BV(TWPS0) | _BV(TWPS1)); // биты предделителя
    TWBR = 114;
    twi_reset_line();

}

void twi_transmit_data(uint8_t addr, uint8_t count, uint8_t *buffer, twi_callback_t  cb) {
    if(!twi_ready())
        return;
    current_buffer = (volatile uint8_t *) buffer;
    current_callback = cb;
    slave_addr = addr;
    mode = MODE_TRANSMIT;
    tx_bytes = count;
    twi(TWI_START);
}

uint8_t twi_ready() {
    if (mode != MODE_IDLE) return 0;
    return 0xFF;
}

void twi_reset_line() {

    cli();
    mode = MODE_RESET;
    TWCR = 0;
    uint8_t ddrc = DDRC;
    uint8_t portc = PORTC;
    RESET(PORTC, PC5);
    RESET(PORTC, PC4);
    SET(DDRC, PC4);
    for (uint8_t i = 0; i < 10; ++i) {
        SET(DDRC, PC5);
        _delay_us(10);
        RESET(DDRC, PC5);
        _delay_us(10);
    }
    PORTC = portc;
    DDRC = ddrc;
    mode = MODE_IDLE;
    sei();


}

void twi_force_stop() {
    TWCR = _BV(TWSTO) | _BV(TWEN) | _BV(TWINT);
    mode = MODE_IDLE;
    twi_reset_line();
    //twi_restarted();
}

ISR(TWI_vect) {
    uint8_t status = TWSR & 0xF8;

    switch (status) {

        case 0x08: // START передан
            // Нужно передать адрес
            TWDR = slave_addr;
            twi(TWI_TRANSMIT);
            break;
        case 0x40: // передали адрес SLA+R, получили ACK
            //начинаем принимать
            if (rcv_bytes > 1) // если нужно принять больше одного байта - получаем с ACK
                twi(TWI_RECEIVE_ACK);
            else  // Иначе получаем без ACK
                twi(TWI_RECEIVE_NACK);
            break;

        case 0x48: // передали адрес SLA+R, не получили ACK:
            twi(TWI_STOP);
            mode = MODE_IDLE;
            if(current_callback)
                (*current_callback)(TWI_STATUS_ERROR);

            break;

        case 0x50: // Получили байт, передали ACK. Будем получать еще
            --rcv_bytes;
            *current_buffer++ = TWDR;
            if (rcv_bytes > 1) // если нужно принять больше одного байта - получаем с ACK
                twi(TWI_RECEIVE_ACK);
            else  // Иначе получаем без ACK
                twi(TWI_RECEIVE_NACK);
            break;
        case 0x58: // Получили байт, передали NACK, больше ничего получать не надо.
            *current_buffer = TWDR;
            twi(TWI_STOP);
            mode = MODE_IDLE;
            if(current_callback)
                (*current_callback)(TWI_STATUS_OK);


            break;
        case 0x20:// Передали адрес SLA+W, ACK не получили - устройства нет на линии
            twi(TWI_STOP);
            mode = MODE_IDLE;
            if(current_callback)
                (*current_callback)(TWI_STATUS_ERROR);
            break;

        case 0x18: // Передали адрес SLA+W, получили ACK
            // передаем данные
            if (mode == MODE_TRANSMIT) {
                if (tx_bytes--) {
                    TWDR = *current_buffer++;
                    twi(TWI_TRANSMIT);
                } else {
                    twi(TWI_STOP);
                    mode = MODE_IDLE;
                    if(current_callback)
                        (*current_callback)(TWI_STATUS_OK);

                }
            }
            if (mode == MODE_TX_RX) {
                if (tx_buff_count--) {
                    TWDR = *tx_buff++;
                    twi(TWI_TRANSMIT);
                } else {
                    // все данные адреса переданы, надо рестарт делать
                    twi(TWI_RESTART);

                    break;
                }
            }
            break;
        case 0x10: // отправили ReSTART

            if (mode == MODE_TX_RX) {
                mode = MODE_RECEIVE;
                slave_addr |= 1;
                TWDR = slave_addr;
                twi(TWI_TRANSMIT);
                break;
            }

            mode = MODE_IDLE;
            twi(TWI_STOP);


            break;
        case 0x28: // Отправили байт, получили ACK
            if (mode == MODE_TRANSMIT) {
                if (tx_bytes--) {
                    TWDR = *current_buffer++;
                    twi(TWI_TRANSMIT);
                } else {
                    twi(TWI_STOP);
                    mode = MODE_IDLE;
                    if(current_callback)
                        (*current_callback)(TWI_STATUS_OK);

                }
                break;
            }
            if (mode == MODE_TX_RX) {
                if (tx_buff_count--) {
                    TWDR = *tx_buff++;
                    twi(TWI_TRANSMIT);
                } else {
                    // все данные адреса переданы, надо рестарт делать
                    twi(TWI_RESTART);

                    break;
                }
            }
        case 0x30: // Отправили байт, ACK не получили - больше данные устройству не нужны
            twi(TWI_STOP);
            mode = MODE_IDLE;
            if(current_callback)
                (*current_callback)(TWI_STATUS_OK_NOACK);

            break;

        case 0: // хер знает, что за статус, просто сбросим прерывание
            TWCR = _BV(TWINT);
            break;
        default:
            TWCR = _BV(TWINT);
            twi_reset_line();
            //twi_restarted();
            break;
    }


}
