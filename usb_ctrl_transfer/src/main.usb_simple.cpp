/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2011 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <millis.h>

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


// TODO: this might need to be modified.
static const uint8_t hid_report_descriptor[] = {                                
    0x05, 0x01, /* USAGE_PAGE (Generic Desktop)         */                      
    0x09, 0x02, /* USAGE (Mouse)                        */                      
    0xa1, 0x01, /* COLLECTION (Application)             */                      
    0x09, 0x01, /*   USAGE (Pointer)                    */                      
    0xa1, 0x00, /*   COLLECTION (Physical)              */                      
    0x05, 0x09, /*     USAGE_PAGE (Button)              */                      
    0x19, 0x01, /*     USAGE_MINIMUM (Button 1)         */                      
    0x29, 0x03, /*     USAGE_MAXIMUM (Button 3)         */                      
    0x15, 0x00, /*     LOGICAL_MINIMUM (0)              */                      
    0x25, 0x01, /*     LOGICAL_MAXIMUM (1)              */                      
    0x95, 0x03, /*     REPORT_COUNT (3)                 */                      
    0x75, 0x01, /*     REPORT_SIZE (1)                  */                      
    0x81, 0x02, /*     INPUT (Data,Var,Abs)             */                      
    0x95, 0x01, /*     REPORT_COUNT (1)                 */                      
    0x75, 0x05, /*     REPORT_SIZE (5)                  */                      
    0x81, 0x01, /*     INPUT (Cnst,Ary,Abs)             */                      
    0x05, 0x01, /*     USAGE_PAGE (Generic Desktop)     */                      
    0x09, 0x30, /*     USAGE (X)                        */                      
    0x09, 0x31, /*     USAGE (Y)                        */                      
    0x09, 0x38, /*     USAGE (Wheel)                    */                      
    0x15, 0x81, /*     LOGICAL_MINIMUM (-127)           */                      
    0x25, 0x7f, /*     LOGICAL_MAXIMUM (127)            */                      
    0x75, 0x08, /*     REPORT_SIZE (8)                  */                      
    0x95, 0x03, /*     REPORT_COUNT (3)                 */                      
    0x81, 0x06, /*     INPUT (Data,Var,Rel)             */                      
    0xc0,       /*   END_COLLECTION                     */                      
    0x09, 0x3c, /*   USAGE (Motion Wakeup)              */                      
    0x05, 0xff, /*   USAGE_PAGE (Vendor Defined Page 1) */                      
    0x09, 0x01, /*   USAGE (Vendor Usage 1)             */                      
    0x15, 0x00, /*   LOGICAL_MINIMUM (0)                */                      
    0x25, 0x01, /*   LOGICAL_MAXIMUM (1)                */                      
    0x75, 0x01, /*   REPORT_SIZE (1)                    */                      
    0x95, 0x02, /*   REPORT_COUNT (2)                   */                      
    0xb1, 0x22, /*   FEATURE (Data,Var,Abs,NPrf)        */                      
    0x75, 0x06, /*   REPORT_SIZE (6)                    */                      
    0x95, 0x01, /*   REPORT_COUNT (1)                   */                      
    0xb1, 0x01, /*   FEATURE (Cnst,Ary,Abs)             */                      
    0xc0        /* END_COLLECTION                       */                      
}; 

const struct usb_interface_descriptor iface = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = 0xFF,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
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
    "Black Sphere Technologies",
    "Simple Device",
    "1001",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static enum usbd_request_return_codes simple_control_callback(usbd_device *usbd_dev,
        struct usb_setup_data *req, uint8_t **buf,
        uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    //(void)buf;
    //(void)len;
    (void)complete;
    (void)usbd_dev;

    if ((req->bmRequestType != 0x81) ||
        (req->bRequest != USB_REQ_GET_DESCRIPTOR) ||
        (req->wValue != 0x2200))
        return USBD_REQ_NOTSUPP;

    *buf = (uint8_t *)hid_report_descriptor;
    *len = sizeof(hid_report_descriptor);

    if (req->bmRequestType != 0x40)
        return USBD_REQ_NOTSUPP; /* Only accept vendor request. */

    if (req->wValue & 1)
        gpio_set(GPIOB, GPIO7);
    else
        gpio_clear(GPIOB, GPIO7);

    return USBD_REQ_HANDLED;
}

static void usb_set_config_cb(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;

    usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_BULK, 4, NULL);
    usbd_register_control_callback(
                usbd_dev,
                USB_REQ_TYPE_VENDOR,
                USB_REQ_TYPE_TYPE,
                simple_control_callback);
}


int main(void)
{
    usbd_device *usbd_dev;

    //rcc_clock_setup_in_hse_25mhz_out_72mhz();
    setup_system_clock();

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_OTGFS);
    //rcc_periph_clock_enable(RCC_SYSCFG); // TODO: is this required?

    /* LED output */
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO7);

    // Setup GPIOs for usb. Only USB DM and USB DP are needed (?)
    // PA8 --> USB SOF (start of frame, unused)
    // PA9 --> USB Vbus (otg fs vbus, unused)
    // PA10 --> USB ID (otg fs id)
    // PA11 --> USB DM (Data minus)
    // PA12 --> USB DP (Data plus)
    // PG6 --> USB GPIO OUT (unused)
    // TODO: can we drop USB Vbus?
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO10 | GPIO11 | GPIO12);
    //gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO11 | GPIO12);
    //gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12); 


    usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config, usb_strings, 3,
                         usbd_control_buffer, sizeof(usbd_control_buffer));
    usbd_register_set_config_callback(usbd_dev, usb_set_config_cb);

    while (1)
        usbd_poll(usbd_dev);
}
