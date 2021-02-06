#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <millis.h>
#include <usart_printf.h>


int main(void)
{
    setup_system_clock();
    setup_systick();
    usart_printf_init(USART1, 115200);

    // Enable Clock for GPIO Bank B Peripheral
    rcc_periph_clock_enable(RCC_GPIOB);

    // Set GPIO B7 to Output Mode.
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);

    while(1)
    {
        gpio_set(GPIOB, GPIO7);
        delay_ms(100);
        gpio_clear(GPIOB, GPIO7);
        delay_ms(100);
        printf("Hey there.\r\n");
    }
    return 0;
}
