/**
* @file modbus_macro.h
* @date 23.07.15
* @author Alexander A. Kuzkin <xbevice@gmail.com>
*/
#ifndef _MACRO_H
#define _MACRO_H

#ifndef ENABLE
#define ENABLE 0xFF ///< Включено
#endif

#ifndef DISABLE
#define DISABLE 0   ///< Выключено
#endif


#define SET(x, y) (x |= _BV(y))             ///< @brief Установить бит y в переменной x
#define RESET(x, y) (x &=~_BV(y))           ///< @brief Сбросить бит y в переменной x
#define INVERT(x, y) (x ^=_BV(y))           ///< @brief Инвертировать бит y в переменной x
#define CHECK(x,y) (x & _BV(y))             ///< @brief Проверить бит y в переменной x
#define CHECKNOT(x,y) (!CHECK(x,y))         ///< @brief Проверить на ноль бит y в переменной x

#define UART_UBBR_VALUE (((F_CPU/BAUD_RATE) >> 4 ) - 1)     ///< @brief Расчет UBRR из F_CPU и BAUD_RATE

#endif //_MACRO_H
