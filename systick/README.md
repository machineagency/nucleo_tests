# System Timer Example

The system timer, aka SysTick, is a timer built into the ARM architecture.
It can be used to generate a periodic interrupt.
As a result, you can write timing-related code that's flexible enough to run on a variety of Cortex-M chips from mulitple brands, since they all include the System Timer.
The suggested use for SysTick is for scheduling managment of tasks running on RTOSes.

## SysTick Nuts and Bolts
Systick counts *down*, not up. When it reaches zero, it starts from the top (some predefined value) and repeats. When it reaches zero, you can setup an interrupt to trigger at this point.

### Setting Tick Frequency
For starters, you will need to know the clock frequency of the microcontroller you're using. For the Nucleo, the default is 72MHz, but it can get bumped up to 216MHz.

## Project Plan
We'll setup the SysTick to keep time via a 1-ms "heartbeat" that we can query. We'll togle the Nucleo-767ZI's blue LED between delays to prove out delay function works.

## Wiring
None!

## Code
LipopenCM3 kindly performs lots of the boilerplate setup for SysTick including setting up the interrupts to trigger when the Systick.

## Discussion
Architecturally, there are more efficient ways to create a delay without having the processor literally spin its wheels, but applying them is fairly tightly coupled with other setups.


## References
* [1Bitsy msleep example](https://github.com/1Bitsy/1bitsy-examples/blob/master/examples/1bitsy/tick_blink/tick_blink.c)
* [Video Explanation](https://www.youtube.com/watch?v=aLCUDv_fgoU&ab_channel=EmbeddedSystemswithARMCortex-MMicrocontrollersinAssemblyLanguageandC)
* [Writing Delay Funcs with Systick Reddit Discussion](https://www.reddit.com/r/embedded/comments/djymi4/understanding_how_to_create_a_delay_function_with/)
