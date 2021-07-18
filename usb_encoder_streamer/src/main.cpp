#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/nvic.h> // for otg_fs_isr and other isr's
#include <libopencm3/stm32/timer.h>  // for timer constants.
#include <usbd_setup.h>
#include <millis.h>
#include <stdlib.h>

uint8_t DATAPTS_PER_PKT = 15;

// bulk buf will be stored like this:
// [1 byte for pkt index, 3 bytes undefined,
//  7 uint32_t's of sequential amt encoder data,
//  7 uint32_t's of sequential heds encoder data]

// Most datatypes marked 'volatile' are being modified within ISRs.

// Callback fn for handling control transfers.
static enum usbd_request_return_codes simple_control_cb(
        usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)buf;
	(void)len;
	(void)complete;
	(void)usbd_dev;

    // Ignore all other requests besides a Vendor Request type.
	if (req->bmRequestType != 0x40) // Vendor Request
		return USBD_REQ_NOTSUPP;

    // Toggle the LED based on wValue
	if (req->wValue & 1)
		gpio_set(GPIOB, GPIO7);
	else
		gpio_clear(GPIOB, GPIO7);

	return USBD_REQ_HANDLED;
}


static void usb_set_config_cb(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;

    // Setup bulk transfer output.
    usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    // Callback fn for handling data from a received control transfer.
	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_VENDOR,
				USB_REQ_TYPE_TYPE,
				simple_control_cb);
}


void setup_encoder_counters(void)
{
/*
    // Setup Encoder Counters on Timer's 2 and 3.
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_TIM3);

    timer_set_period(TIM2, 0xFFFF);
    timer_set_period(TIM3, 0xFFFF);

    timer_slave_set_mode(TIM2, TIM_SMCR_SMS_EM3);
    timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM3);

    timer_ic_set_input(TIM2, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM2, TIM_IC2, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI1);

    timer_enable_counter(TIM2);
    timer_enable_counter(TIM3);
*/
}


// Setup Sampling Rate on Timer7
void setup_sampling_timer(void)
{
    rcc_periph_clock_enable(RCC_TIM7);
    nvic_enable_irq(NVIC_TIM7_IRQ);
    rcc_periph_reset_pulse(RST_TIM7); // reset timer 7 to defaults.

    timer_set_mode(TIM7,
                   TIM_CR1_CKD_CK_INT, // No additional clock division ratio.
                   TIM_CR1_CMS_EDGE, // Edge alignment.
                   TIM_CR1_DIR_UP); // Up-counting counter

    // TODO: why 4800? (Isn't rcc_apb1_frequency defaulting to 16000000?)
    timer_set_prescaler(TIM7, rcc_apb1_frequency/4800); // 10KHz per increment
    timer_set_period(TIM7, 1000); // Set TIM_ARR value. 1000 ticks at 10KHz = 10Hz.
    //timer_set_period(TIM7, 333); // Set TIM_ARR value. 333 ticks at 10KHz = 30Hz
    // TODO: should this be 999? Is 0 counted?

    timer_disable_preload(TIM7);
	timer_continuous_mode(TIM7);
    timer_enable_counter(TIM7);

    // When the timer updates, the interrupt should fire.
    timer_enable_irq(TIM7, TIM_DIER_UIE); // Timer 7 update interrupt enable.
}

// Two buffers for aggregating Encoder Data.
// Write to one while transferring the other. Then swap.
volatile uint8_t ping[BULK_BUFFER_SIZE];
volatile uint8_t pong[BULK_BUFFER_SIZE];
// Pointers to demarcate who is being written-to and who is being transferred.
volatile uint8_t* sampling_buffer;
volatile uint8_t* usb_writing_buffer;

// A packet ID that increments once per stuffed packet.
// Note that packets can fly as fast as 1KHz on Full Speed, so this value
// should be sufficiently large to ID packets before rollover.
volatile uint16_t usb_pkt_index = 0;
// Index into the packet to determine where to write next.
volatile uint8_t usb_pkt_write_index = 0;
// Flag to indicate data is ready for writing.
volatile bool usb_output_data_ready = false;

// ISR to sample the encoder data and transfer it into ping-pong buffer.
void tim7_isr(void)
{
    timer_clear_flag(TIM7, TIM_SR_UIF);

    gpio_toggle(GPIOB, GPIO7);

/*
    uint16_t amt_encoder_ticks = timer_get_counter(TIM2);
    uint16_t heds_encoder_ticks = timer_get_counter(TIM3);
*/

    // For TESTING ONLY.
    uint16_t amt_encoder_ticks = (uint16_t)(system_millis & 0xFFFF);
    uint16_t heds_encoder_ticks = (uint16_t)(system_millis & 0xFFFF);
    // END FOR TESTING ONLY

    // Compute indices into the fifo.
    uint8_t amt_index = 2 + sizeof(amt_encoder_ticks)*usb_pkt_write_index;
    uint8_t heds_index = BULK_BUFFER_SIZE/2 + sizeof(amt_encoder_ticks)*usb_pkt_write_index;
    // Store encoder data in little-endian format.
    sampling_buffer[amt_index] =      amt_encoder_ticks & 0xFF;
    sampling_buffer[amt_index + 1] = (amt_encoder_ticks >> 8) & 0xFF;
    sampling_buffer[heds_index] =     heds_encoder_ticks & 0xFF;
    sampling_buffer[heds_index + 1] = (heds_encoder_ticks >> 8) & 0xFF;

    ++usb_pkt_write_index;

    // Check Buffer index. Switch buffers if necessary.
    if (usb_pkt_write_index > DATAPTS_PER_PKT)
    {
        // ID the packet before transmitting so we can sort them on the receiving end.
        sampling_buffer[0] = usb_pkt_index & 0xFF;
        sampling_buffer[1] = (usb_pkt_index >> 8) & 0xFF;

        usb_pkt_write_index = 0;
        // Swap buffers.
        usb_writing_buffer = sampling_buffer;
        sampling_buffer = (sampling_buffer == ping)? pong: ping;
        usb_output_data_ready = true;
        ++usb_pkt_index;
    }
/*
*/
}


// Handle usb activity via ISR so we don't have to poll it in the main loop.
// this fn is weakly defined in NVIC.h
void otg_fs_isr(void)
{
   usbd_poll(usbd_dev);
}


int main(void)
{
    setup_system_clock(); // do this first.
    setup_systick(); // for delay_ms fn

    // Enable some blinkies:
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);

    // encoder counting and sampling setup:
    setup_encoder_counters();
    setup_sampling_timer();

    // usb init.
	rcc_periph_clock_enable(RCC_OTGFS);

	/* Setup Nucleo's blue LED for output */
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);

    // Relevant Pinouts:
    // PA8 --> USB SOF (start of frame, unused)
    // PA9 --> USB Vbus (otg fs VBus sensing, unused, disabled)
    // PA10 --> USB ID (otg fs id)
    // PA11 --> USB DM (Data minus)
    // PA12 --> USB DP (Data plus)
    // PG6 --> USB GPIO OUT (unused)
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO10 | GPIO11 | GPIO12);

    // Setup interrupts for handling usb events.
    nvic_enable_irq(NVIC_OTG_FS_IRQ);

	usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config,
                         usb_strings, 3,
                         usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, usb_set_config_cb);


    // Main Loop: Check if it's time to write the packet, and write it.
	while (1)
    {
		//usbd_poll(usbd_dev); // not needed since we're using interrupts.
        if (usb_output_data_ready)
        {
            // This casting feels dirty.
            usbd_ep_write_packet(usbd_dev, 0x81, (const void*)usb_writing_buffer,
                                 sizeof(ping)); // usb_writing_buffer points to ping or pong.
            usb_output_data_ready = false; // Clear the flag.
        }
    }
}
