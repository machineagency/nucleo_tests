// Code adapted from:
// https://github.com/libopencm3/libopencm3-examples/tree/master/examples/stm32/f1/stm32-h107/usb_simple
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/cm3/nvic.h> // for otg_fs_isr
#include <millis.h>
#include <stdlib.h>

// Define this globally since we need to use in in the ISR and main loop.
usbd_device *usbd_dev;

const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xFF,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0xCAFE,
	.idProduct = 0xCAFE,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

const struct usb_endpoint_descriptor data_endp[] =
{
    {
	    .bLength = USB_DT_ENDPOINT_SIZE,
	    .bDescriptorType = USB_DT_ENDPOINT,
	    .bEndpointAddress = 0x81,
	    .bmAttributes = USB_ENDPOINT_ATTR_BULK,
	    .wMaxPacketSize = 64,
	    .bInterval = 1,
    }
};

const struct usb_interface_descriptor iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = 0xFF,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
    .endpoint = data_endp,
};

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &iface,
}};

const struct usb_config_descriptor config = {
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

const char *usb_strings[] = {
	"Black Sphere Technologies", // Product
	"Simple Device", // Manufacturer
	"1001", // Serial Number
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

/* Callback fn for handling control transfers. */
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

/* handle usb activity via ISR so we don't have to poll it in the main loop. */
/* this fn is weakly defined in NVIC.h */
void otg_fs_isr(void)
{
   usbd_poll(usbd_dev);
}

int main(void)
{
    char bulk_buf[64];
    bulk_buf[0] = 10;
    bulk_buf[1] = 11;
    bulk_buf[2] = 12;
    bulk_buf[3] = 13;

    setup_system_clock();
    setup_systick(); // for delay_ms fn

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
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

	while (1)
    {
		//usbd_poll(usbd_dev); // not needed since we are using interrupts
        delay_ms(1000);
        usbd_ep_write_packet(usbd_dev, 0x81, bulk_buf, sizeof(bulk_buf));
    }
}
