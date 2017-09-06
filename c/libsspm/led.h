#ifndef _LED_H_
#define _LED_H_

#include <machine/patmos.h>

#define LED ( *( ( volatile _IODEV unsigned * ) 0xF0090000))
#define MS_CLOCK (18000)

/*
Turns on the core's corresponding LED.
If the LED is already on, has no effect.
*/
void led_on();


/*
Turns off the core's corresponding LED.
If the LED is already off, has no effect.
*/
void led_off();

/*
Will continuously turn on the core's corresponding LED
for the specified amount of time (in milliseconds).

The timing is not precise.
This function is effectively blocking for the specified time.
The LED will not flicker or blink, but stay continuously on.
The LED will not turn off when the call terminates, therefore, this
function just ensures the LED is on for at least the specified time.
*/
void led_on_for(int ms);

/*
Will continuously turn off the core's corresponding LED
for the specified amount of time (in milliseconds).

The timing is not precise.
This function is effectively blocking for the specified time.
The LED will not flicker or blink, but stay continuously off.
The LED will not turn on when the call terminates, therefore, this
function just ensures the LED is off for at least the specified time.
*/
void led_off_for(int ms);

/*
Will continuously blink the core's corresponding LED with the specified
period.
Will never terminate.
*/
void led_blink(int period);





#endif
