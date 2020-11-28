#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <stdio.h>


volatile uint32_t system_millis;

void setup_system_clock()
{
    // Configure the clock for 216MHz, external 8MHz oscillator from STLink's MCO output.
    //rcc_clock_setup_hse(&rcc_3v3[RCC_CLOCK_3V3_216MHZ], 8);

    // Configure the clock for 216MHz using the internal oscillator.
    rcc_clock_setup_hsi(&rcc_3v3[RCC_CLOCK_3V3_216MHZ]);
}

void setup_systick()
{
    //int((clock_rate - 1)/1000) to get 1ms interrupt.
    systick_set_reload(216000);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    // Do this last.
    systick_interrupt_enable();
}


// This fn is declared in nvic.h
void sys_tick_handler(void)
{
    system_millis++;
}


void delay_ms(uint32_t delay)
{
    uint32_t wake_time = delay + system_millis;
    while (wake_time - system_millis > 0); // Do nothing.
}


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
        delay_ms(500);
        gpio_clear(GPIOB, GPIO7);
        delay_ms(500);
    }
    return 0;
}
