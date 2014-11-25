/*
 * GccApplication1.c
 *
 * Created: 08.11.2014 18:44:30
 *  Author: Wolf
 */ 

// default is clock select 9.6Mhz for Attiny13 and Clock divider 8 = 1.2 Mhz
#define F_CPU 1200000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
//#include <avr/power.h>

void wdt_init(uint8_t timeout) {
	cli();
	wdt_enable(timeout);
	//MCUSR |= (1<<WDRF);
	WDTCR |= /*(1<<WDE) | */(1<<WDTIE);   // Enable watchdog
	wdt_reset();
	//WDTCR = (1<<WDTIE) | WDTO_8S;    // Watchdog interrupt instead of reset with 4s timeout
	sei();
}

void sleep_now() {
	//power_all_disable();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	//sleep_bod_disable();
	sleep_mode();
	sleep_disable();
	wdt_disable();
}

void SleepSeconds(uint16_t seconds)
{
	//granularity 8s
	//
	// some more information: stopping the time showed that sleep time of 256s takes ~5s longer
	// the reason might be the time for waking up and sleeping again in this for-loop
	// every 8 seconds.
	// this might contribute to some little unwanted current consumption
	// the current consumption is low, so not really significant, but it might be a possible 
	// optimization. For this the current consumption should be measured that occurs every 8 seconds.
	uint16_t cycles = seconds / 8;
	for (int i=0 ; i<cycles; i++)
	{
		wdt_init(WDTO_8S);
		sleep_now();
	}
}

uint16_t GetSleepTimeFromSpark()
{
	// get sleep duration code from Spark
	// D1  D0  Spark
	// PB4 PB3 Attiny
	// 0   0   = 256s
	// 0   1   = 1h-3min
	// 1   0   = 1hmin
	// 1   1   = 8s
	if (PINB & (1 << PINB3) && PINB & (1 << PINB4)) return 8;
	if (PINB & (1 << PINB3)) return (3600-180);
	if (PINB & (1 << PINB4)) return 3600;
	return 256/*3600*/;
}


int main(void)
{
	// define the port usage (PINB0 is output. Rest is input)
	DDRB=0b00000001;
	//MCUCR=0b00010000;

	//PCMSK = (1<<PCINT1);	// pin change mask: listen to portb bit 2
	//GIMSK |= (1<<PCIE);	// enable PCINT interrupt
	//MCUCR |= (1<<ISC01);
	//MCUCR &= 0b11111110;
	//sei();

	//wdt_init();
	
	// disable power and wait three seconds to ensure stable start of LDO and spark after three seconds
	// this time can surely be reduced, but it will only play a role after plugging system to battery
	PORTB = 0b00000000;  // disable LDO power
	_delay_ms(3000);
	
    while(1)
    {
		// turn on the LDO which will power the Spark
		PORTB = 0b00000001;
		// give Spark three seonds to start and wait for PinB1 which is the busy pin of the spark
		// we also wait for PinB2 which is the busy pin of the spectra display
		_delay_ms(1500);
		while (PINB & (1 << PINB1) || PINB & (1 << PINB2))
		{
			wdt_init(WDTO_250MS);
			sleep_now();
		}
		
		uint16_t timeToSleep = GetSleepTimeFromSpark();		
		
		// now spark and display is ready...turn off power...
		PORTB = 0b00000000;  // disable LDO power
	
		// and wait long time till next spark action...
		SleepSeconds(timeToSleep);
    }
}

/*
ISR(PCINT0_vect)	     // interrupt service routine
{			     // called when PCINT0 changes state
	if (!(PINB & (1 << PINB1)))
	//	PORTB = (PORTB ^ 0x01);   // toggle red led, portb bit 5, pin 3
	return;
}
*/

// this one is needed as watchdog wakeup routine. Removing this will not work.
ISR(WDT_vect)
{
//	sleep_disable();          // Disable Sleep on Wakeup
//	wdt_disable();
	//PORTB = 0b00000000;
//	_delay_ms(5000);
	// Your code goes here...
	// Whatever needs to happen every 1 second
	//sleep_enable();           // Enable Sleep Mode
}
