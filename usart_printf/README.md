# printf over usart example

On a pc, printing to the screen with printf() makes sense.
Characters get displayed on a screen where we can read them.
But on a microcontroller has no screen by default.
Instead we can redefine the behavior of printf() to output characters to a hardware serial port instead.
This new printf() behavior can be super useful for debugging.

## Implementation
To implement this behavior, we just need to overrite the **_write(...)** function which is called by printf() under the hood.
Since our code is being compiled with C++ and **_write()** is part of a C library, we need to wrap our implementation in an **extern "C"** block to get the compiler to link to the correct function from the stdio.h library (aka: cstdio in C++).

## Code
LipopenCM3 kindly performs lots of the boilerplate setup for setting up a usart.

## Notes
make sure you setup the oscillator first, or the timing-related settings (i.e: the baud rate) will be incorrect.)

## References
* [Libopencm3 Example](https://github.com/libopencm3/libopencm3-examples/blob/master/examples/stm32/f4/nucleo-f411re/usart-stdio/usart-stdio.c)
