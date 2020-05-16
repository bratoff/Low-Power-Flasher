// ATtiny13 based low power LED flasher - 4uA quiescent current, can blink for years on 2xAA batteries
// (c) 2016 F. Stefanec  
//
// Fuse settings: HFUSE is 0xFF, LFUSE is 0x7B - internal 128kHz clock
// The microcontroller stays in power-down mode for most of the time, thus saving power.
// It would otherwise consume about 200uA all the time.
// Power consumption is around 4uA when no LED is on, around 12mA with the LED on (2V red LED)
//
//    |------O-----------------------|
// 3V |      |          __________   |
// -------   | 1uF     | °       |  |
//   ---    ---       -| RST  VCC |--|
//    |     ---       -| PB3  PB2 |-
//    |      |        -| PB4  PB1 |            LED
//    |      |      ___| GND  PB0 |---[1Kohm]--|>|--O
//    |      |     |   |__________|                 |
//    |------O-----O--------------------------------|
//

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define LED PB0

ISR(WDT_vect) {
  MCUCR = (1<<SM1); //set sleep mode back to power-down
  DDRB = (1<<LED);  //enable LED pin as output
  PORTB = (1<<LED); //light LED
  _delay_ms(3);     //for 3 ms
  PORTB = 0;        //then turn off
  DDRB = 0;         //set LED pin as input to save power
}

int main(void) {
  cli();  //Interrupts off during initialization
  
//Do a bunch of stuff to minimize power consumption:  
  PRR = (1<<PRTIM0) | (1<<PRADC); // Kill power to ADC and Timer0
  DIDR0 = 3;  //Shut off analog buffers
  ADCSRA = 0; // disable ADC
  DDRB = 0;  //set LED pin as input so it doesn't consume power
  PORTB = 0;  //shut off all pull-ups
  
  WDTCR = (1<<WDTIE) | (1<<WDCE) | (1<<WDP3); //4 second watchdog timeout
  sei();  // Enable interrupts
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  //setup for deep sleep mode
  
  for(;;) {
    sleep_mode(); //Sleep forever - all work is done in interrupt
  }
}
