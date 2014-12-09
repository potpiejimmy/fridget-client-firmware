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

void SleepLong(uint16_t time)
{
	// time is encoded in the following way:
	// 0-59 corresponds to sleep time in seconds
	// if time is >59 it corresponds to sleep time in minutes
	// e.g. 
	// 58 = 58s
	// 59 = 59s
	// 60 = 1min
	// 61 = 2min
	// 62 = 3min
	// and so on
	//
	// granularity is 8s (watchdog sleep time). time values less then 9s will result in no sleep time at all
	//
	// some more information: stopping the time showed that sleep time of 256s takes ~5s longer
	// the reason might be the time for waking up and sleeping again in this for-loop
	// every 8 seconds.
	// this might contribute to some little unwanted current consumption
	// the current consumption is low, so not really significant, but it might be a possible 
	// optimization. For this the current consumption should be measured that occurs every 8 seconds.

	uint16_t seconds = time;
	if (time>59)
	{
		seconds = (time-59)*60; // will overflow if time is greater than 2^16/60+59
	}
	//
	// time drift measurements with prototype A showed that actual time is 8.408 seconds for one sleep loop.
	// so we do divide by 8.408 here, not by 8.
		
	uint16_t cycles = seconds / 8.408;
	for (uint16_t i=0 ; i<cycles; i++)
	{
		wdt_init(WDTO_8S);
		sleep_now();
	}
}

uint16_t GetSleepTimeFromSpark()
{
	// use PB4 (Attiny) connected to D1 of Spark as CLK (Attiny output)
	// use PB3 (Attiny) connected to D0 of Spark as Data (Attiny input)
	
	uint16_t retval = 0b0000000000000000;
	// read exactly 16 bit (2 bytes) from spark, MSB first
	for (int i=0;i<16;i++)
	{
		// shift left by one position
		retval <<= 1;
		// if PB3 = high then set last bit to 1 by increasing
		if (PINB & (1 << PINB3)) retval++;
		// now toggle CLK signal with XOR
		PORTB ^= (1<<PINB4);
		// we do not know delay, assuming 10ms is sufficient
		// for spark to detect CLK toggle and set D1
		_delay_ms(10);
	}

	return retval;
}


int main(void)
{
	// define the port usage (PINB0 is output. Rest is input)
	// define port PB4 as CLK output for serial communication with Spark
	DDRB=0b00010001;
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
		SleepLong(timeToSleep);
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
