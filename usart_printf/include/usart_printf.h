#ifndef USART_PRINTF_H
#define USART_PRINTF_H
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <cstdio>
/**
 * Consume a USART peripheral for output via the printf(..) function.
 * This is useful for debugging and status information.
 *
 * TODO: support both input and output and rename this file to usart_stdio.h
 * Works on F4 and F7's, which are pin-compatible.
 * For other implementations, see:
 * https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/nucleo-f411re/usart-stdio/usart-stdio.c
 */

extern uint32_t usart_id;


/**
 * claim a usart for printf
 */
void usart_printf_init(uint32_t usart_id=USART1, uint32_t baud_rate=115200);
#endif //USART_PRINTF_H
