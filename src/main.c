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
//    |      |        -| PB4  PB1 |-           LED
//    |      |      ___| GND  PB0 |---[82ohm]--|>|--O
//    |      |     |   |__________|                 |
//    |------O-----O--------------------------------|
//
// 16May20  brr change flash rate by testing pins 2 and 3 at startup time:
//                1 second  - Pins 2 and 3 (PB3 and PB4) grounded
//                2 seconds - Pin 3 (PB4) grounded
//                8 seconds - Pin 2 (PB3) grounded
//                4 seconds - both pins open (default)
//
//

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define LED	PB0

ISR(WDT_vect) {
	MCUCR = (1<<SM1);	//set sleep mode back to power-down
	DDRB = (1<<LED);		//set LED pin as output
	PORTB = (1<<LED);	//turn LED on
	_delay_ms(3);				//for 3 ms
	PORTB = 0;					//then turn LED off
	DDRB = 0;						//and turn port back to input to save power
}

int main(void) {
  register uint8_t TCRbits;  // will hold Watchdog configuration flags
  uint8_t inBits;
	cli();
	PRR = (1<<PRTIM0) | (1<<PRADC);	// Kill power to ADC and Timer 0
	DIDR0 = 3;	// Shut off analog buffers
	ADCSRA = 0;	// Disable ADC
	DDRB = 0;	// set LED pin as input so it doesn't consume power
  PORTB = 0b00011000; // turn on pull-ups to read option pins
  _delay_ms(5);
  inBits = PINB & 0b00011000;
  switch(inBits) {
    case 0b00000000:
      // Pins 2 and 3 (PB3 and PB4) grounded
      TCRbits = (1<<WDTIE) | (1<<WDP1) | (1<<WDP2); // 1 second flash rate
      break;
    case 0b00001000:
      // Pin 3 (PB4) grounded
      TCRbits = (1<<WDTIE) | (1<<WDP0) | (1<<WDP1) | (1<<WDP2); // 2 second flash rate
      break;
    case 0b00010000:
      // Pin 2 (PB3) grounded
      TCRbits = (1<<WDTIE) | (1<<WDP0) | (1<<WDP3); // 8 second flash rate
      break;
    case 0b00011000:
      // Both pins open (default)
    default:
      TCRbits = (1<<WDTIE) | (1<<WDP3); //4 second flash rate
      break;
  }
  PORTB = 0;  //shut off all pull-ups to save power
  WDTCR |= (1<<WDCE);  // Set up to change watchdog
  WDTCR = TCRbits;
	sei();
	while(1) {
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);	//deep sleep mode
		sleep_mode();
	}
}
