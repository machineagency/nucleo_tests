#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <millis.h>


int main(void)
{
    setup_system_clock();
    setup_systick();

    // Enable Clock for GPIO Bank B Peripheral
    rcc_periph_clock_enable(RCC_GPIOB);

    // Set GPIOs A6 and B7 to Alternate-Function Mode.
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);

    while(1)
    {
        gpio_set(GPIOB, GPIO7);
        delay_ms(250);
        gpio_clear(GPIOB, GPIO7);
        delay_ms(250);
    }
    return 0;
}
