#!/usr/bin/env python3
import usb.core # import pyusb

if __name__ == "__main__":
    dev = usb.core.find(idVendor=0xcafe, idProduct=0xcafe)
    if dev is None:
        raise ValueError('Device not found')

    dev.set_configuration()

    value = 0
    while True:
        key = input("press ENTER to toggle the LED.")
        value^=1
        dev.ctrl_transfer(0x40, 0, value, 0, 'Hello World!')

