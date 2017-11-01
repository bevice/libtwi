#include "main.h"
#include "libtwi.h"
#include <util/delay.h>
void setup() {
	twi_init();
	DDRB |= _BV(PB5); // включим PB5 на выход, на Arduino Nano там светодиодик

}

#define DATA_SIZE 128
static uint8_t data[DATA_SIZE] ={0};

void twi_cb(uint8_t status){
	if(status!= TWI_STATUS_ERROR) // если все прочиталось - перевернем светодиод
		DDRB ^= _BV(PB5);
}

int main() {
    setup();

    while (1) {
    	uint8_t buff[] = {0x10,1,2,3,4,5};
    	// передаем buff в железку с адресом  0xA0, как закончим передавать - дергаем twi_cb
    	twi_transmit_data(0xA0, sizeof(buff), buff, &twi_cb);
    	_delay_ms(1000);
    	// читаем  DATA_SIZE байт из железки с адресом 0xA0 начиная с регистра 0x10
    	// как в буффер data, как дочитаем - попадем в коллбек twi_cb с соответствующим статусом
		twi_receive_data_adr8(0xA0, 0x10, DATA_SIZE, data, &twi_cb);
		_delay_ms(1000);
    }

    return 0;
}

