#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <millis.h>
#include <usart_printf.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/scb.h>
#include <cstring>
#include <string>

const struct usb_device_descriptor dev_descriptor =
{
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = USB_CLASS_VENDOR,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0483,
    .idProduct = 0x5740,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

const struct usb_endpoint_descriptor data_endp[] =
{
/*
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x01,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    },
*/
    {
        .bLength = USB_DT_ENDPOINT_SIZE,
        .bDescriptorType = USB_DT_ENDPOINT,
        .bEndpointAddress = 0x81,
        .bmAttributes = USB_ENDPOINT_ATTR_BULK,
        .wMaxPacketSize = 64,
        .bInterval = 1,
    }
};

const struct usb_interface_descriptor data_iface[] =
{
    {
        .bLength = USB_DT_INTERFACE_SIZE,
        .bDescriptorType = USB_DT_INTERFACE,
        .bInterfaceNumber = 1,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,//2,
        .bInterfaceClass = USB_CLASS_VENDOR,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface = 0,

        .endpoint = data_endp,
    }
};

const struct usb_interface ifaces[] =
{
    {
        .num_altsetting = 1,
        .altsetting = data_iface,
    }
};

const struct usb_config_descriptor config =
{
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 0,
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0x80,
    .bMaxPower = 0x32,

    .interface = ifaces,
};

const char * usb_strings[] =
{
    "Double Jump Electric", // id vendor
    "DIY DAQ", // id product
    "DEMO",
};


uint8_t usbd_control_buffer[128]; // required for instantiation.
char bulk_buf[64];
int len = 0;

enum usbd_request_return_codes hid_control_request(
            usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)usbd_dev;

/*
	if ((req->bmRequestType != 0x81) || (req->bRequest != USB_REQ_GET_DESCRIPTOR) || (req->wValue != 0x2200))
		return USBD_REQ_NOTSUPP;
*/

    // DO nothing.
	/* Handle the HID report descriptor. */
	//*buf = (uint8_t *)hid_raw_report_descriptor;
	//*len = sizeof(hid_raw_report_descriptor);

	return USBD_REQ_HANDLED;
}


/**
 * configure usb
 */
void set_config(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;

    // Write and read endpoints need to agree in the driver.
    //usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, data_rx_callback);
    usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_BULK, 64, NULL);

    usbd_register_control_callback(
            usbd_dev,
            USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_ENDPOINT,
            USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
            hid_control_request);
}


int main(void)
{
    usbd_device *usbd_daq;

    // Setup system clock before anything else.
    setup_system_clock();
    setup_systick();
    rcc_periph_clock_enable(RCC_OTGFS); // Setup USB peripheral clock.
    usart_printf_init(USART1, 115200); // For optional diagnostics.

    // Enable Clock for GPIO Bank B Peripheral
    rcc_periph_clock_enable(RCC_GPIOB);

    // Set GPIO B7 to Output Mode.
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);


    // Setup GPIOS for usb. Only USB DM and USB DP are needed (?)
    // PA8 --> USB SOF (start of frame, unused)
    // PA9 --> USB Vbus (otg fs vbus)
    // PA10 --> USB ID (otg fs id, unused)
    // PA11 --> USB DM (Data minus)
    // PA12 --> USB DP (Data plus)
    // PG6 --> USB GPIO OUT (unused)
    // TODO: can we drop USB Vbus?
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);

    usbd_daq = usbd_init(&otgfs_usb_driver, &dev_descriptor, &config,
                         usb_strings, 3,
                         usbd_control_buffer, sizeof(usbd_control_buffer));

    usbd_register_set_config_callback(usbd_daq, set_config);

    // Populate with some friendly sample data:
    std::string greeting = "Greetings, comrade.";
    strcpy(bulk_buf, greeting.c_str());

    while(1)
    {
        usbd_ep_write_packet(usbd_daq, 0x81, bulk_buf, len);
        gpio_set(GPIOB, GPIO7);
        delay_ms(500);
        gpio_clear(GPIOB, GPIO7);
        delay_ms(500);
        printf("Hey there.\r\n");
    }
    return 0;
}
