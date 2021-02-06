# USB Bulk Transfers

Fancy Data Acquisition Units (DAQs) capture high speed data from sensors and then transfer large buffered packets over to a PC for plotting.
This process works around the fact that our PCs are not real-time systems, and simply can't poll sensors reliably faster than about 30-60Hz.

By setting up bulk transfers from the Nucleo to our PC, we'll be able to spoof the behavior of a DAQ.
The only caveat is that we'll have to write our own usb driver to unpack and receive the data, but joh on Github has kindly gotten us most of the way there with an [example](https://github.com/libopencm3/libopencm3-examples/tree/5aadcddf0836360959dd28346b9b11ae3ab3dc82/examples/stm32/f4/stm32f4-discovery/usb_blink)

## Hardware Setup
The default 0-ohm jumper configuration is set correctly to communicate over USB.
If you have altered any jumpers, then ensure that the following jumpers on the bottom of the board are set like so:

| Jumper Name | Jumper Status |
|-------------|---------------|
| SB 125      | ON            |
| SB 127      | ON            |
| SB 132      | ON            |
| SB 133      | ON            |
| SB 184      | OFF           |
| SB 185      | ON            |
| SB 186      | OFF           |
| SB 187      | ON            |
| JP 04       | ON            |

## Software Setup
For the python side of this project, you'll need to install [pyusb](https://github.com/pyusb/pyusb)


## Implementation
TODO


## Code
LipopenCM3 kindly performs lots of the boilerplate setup for setting up USB for bulk output transfers.

## Notes
make sure you setup the oscillator first, or the timing-related settings (i.e: the baud rate) will be incorrect.)

## References
* [Mew](https://github.com/konachan700/Mew/tree/master/bootloader/drivers/usb)
* [USB Simple](https://github.com/libopencm3/libopencm3-examples/tree/master/examples/stm32/f1/stm32-h107/usb_simple)
