#include <usart_printf.h>

uint32_t usart_id;

// Override _write implementation to use a specific hardware peripheral.
extern "C"
{
    int _write(int fd, char *ptr, int len)
    {
        // Typecast to remove compiler warning as len should never be negative.
        for (size_t index=0; index<(size_t)len; ++index)
        {
            usart_send_blocking(usart_id, *ptr);
            ++ptr;
        }
        return len;
    }

}

void usart_printf_init(uint32_t usart, uint32_t baudrate)
{
    usart_id = usart;
    // Setup the gpios and desired uart peripherals
    // Setup clocks.
    switch (usart_id)
    {
        case USART1:
            rcc_periph_clock_enable(RCC_GPIOA);
            rcc_periph_clock_enable(RCC_USART1);
            // GPIO PA9/10 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO9));
            gpio_set_af(GPIOA, GPIO_AF7, (GPIO9)); // AF7 --> USART TX
            break;
        case USART2:
            rcc_periph_clock_enable(RCC_GPIOA);
            rcc_periph_clock_enable(RCC_USART2);
            // GPIO PA2/3 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO2));
            gpio_set_af(GPIOA, GPIO_AF7, (GPIO2)); // AF7 --> USART TX
            break;
        case USART3:
            rcc_periph_clock_enable(RCC_GPIOB);
            rcc_periph_clock_enable(RCC_USART3);
            // GPIO PB10/11 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO10));
            gpio_set_af(GPIOB, GPIO_AF7, (GPIO10)); // AF7 --> USART TX
            break;
        case UART4:
            rcc_periph_clock_enable(RCC_GPIOA);
            rcc_periph_clock_enable(RCC_UART4);
            // GPIO PA0/1 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO0));
            gpio_set_af(GPIOA, GPIO_AF7, (GPIO0)); // AF7 --> USART TX
            break;
        case UART5:
            rcc_periph_clock_enable(RCC_GPIOC);
            //rcc_periph_clock_enable(RCC_GPIOD);
            rcc_periph_clock_enable(RCC_UART5);
            // GPIO PC12/PD2 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO12));
            gpio_set_af(GPIOC, GPIO_AF7, (GPIO12)); // AF7 --> USART TX
            break;
        case USART6:
            rcc_periph_clock_enable(RCC_GPIOC);
            //rcc_periph_clock_enable(RCC_GPIOD);
            rcc_periph_clock_enable(RCC_USART6);
            // GPIO PC6/7 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO6));
            gpio_set_af(GPIOC, GPIO_AF7, (GPIO6)); // AF7 --> USART TX
            break;
        case UART7:
            rcc_periph_clock_enable(RCC_GPIOF);
            rcc_periph_clock_enable(RCC_UART7);
            // GPIO PF7/6 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO7));
            gpio_set_af(GPIOF, GPIO_AF7, (GPIO7)); // AF7 --> USART TX
            break;
        case UART8:
            rcc_periph_clock_enable(RCC_GPIOE);
            rcc_periph_clock_enable(RCC_UART8);
            // GPIO PE1/0 --> TX/RX (We only need TX)
            gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO1));
            gpio_set_af(GPIOF, GPIO_AF7, (GPIO1)); // AF7 --> USART TX
            break;
    }

	usart_set_baudrate(usart_id, baudrate);
	usart_set_databits(usart_id, 8);
	usart_set_stopbits(usart_id, USART_STOPBITS_1);
	usart_set_mode(usart_id, USART_MODE_TX);
	usart_set_parity(usart_id, USART_PARITY_NONE);
	usart_set_flow_control(usart_id, USART_FLOWCONTROL_NONE);
	usart_enable(usart_id);
}
