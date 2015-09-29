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

/* initialize the watchdog */
void wdt_init(uint8_t timeout) {
	/* globally disable all interrupts */
	cli();
	/* enable the watchdog by setting the timer */
	wdt_enable(timeout);
	/* enable the watchdog interrupt in the watchdog timer control register. 
	   the interrupt routine called when watchdog timer expired is below, see ISR(WDT_vect).
	   WDE is automatically set to 0 by this operation, means that WD is in interrupt mode */
	WDTCR |= (1<<WDTIE);   // Enable watchdog	
	/* we could set the WDRF flag in MCUSR to bring the watchdog to reset mode (we don't want that) */
	//MCUSR |= (1<<WDRF);
	/* reset the watchdog, meaning we start the timer */
	wdt_reset();
	/* globally enable all interrupts again */
	sei();
}

/* put the attiny to deep power down mode */
void sleep_now() {
	/* set the sleep mode to power down */
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	/* and finally go to sleep mode */
	sleep_mode();
	/* disable the watchdog, we do not need it any more */
	wdt_disable();
}

void SleepLong(uint16_t cycles)
{
	/* sleep the number of given cycles
	   long-time tests showed that one sleep takes approx. 8.408 seconds
	   we do not know if the difference to exactly 8 seconds is caused
	   by inexactness of watchdog oscillator and/or by time needed for waking
	   up and going to sleep again */
	for (uint16_t i=0 ; i<cycles; i++)
	{
		/* initialize the watchdog */
		wdt_init(WDTO_8S);
		/* and sleep */
		sleep_now();
	}
}

/* Get the number of sleep cycles from spark/photon */
/* use PB4 (Attiny) connected to D1 of Spark as CLK (Attiny output) */
/* use PB3 (Attiny) connected to D0 of Spark as Data (Attiny input) */
uint16_t GetSleepTimeFromSpark()
{
	/* initialize to 0 */
	uint16_t retval = 0b0000000000000000;
	/* read exactly 16 bit (2 bytes) from spark/photon, MSB first */
	for (int i=0;i<16;i++)
	{
		/* shift left by one position */
		retval <<= 1;
		/* now toggle CLK signal with XOR */
		PORTB ^= (1<<PINB4);
		/* we do not know delay, assuming 3ms is sufficient
		   for spark to detect CLK toggle and set D1
		   we tested 12 times with action plan -0001-0002-0003-0004 without any errors for 2ms delay.
		   so we set to 3ms now and guess we are save. */
		_delay_ms(3);
		/* read first bit: if PB3 = high then set last bit to 1 by increasing */
		if (PINB & (1 << PINB3)) retval++;
	}
    /* final toggle to tell Spark/Photon that it can power down */
    PORTB ^= (1<<PINB4);
	/* return the number of sleep cycles read from spark/photon */
	return retval;
}

/* the main routine. startup, wait for spark/photon and display update, sleep */
int main(void)
{
	/* define the port usage: PINB2 is output for LDO shutdown
	   define port PB4 as CLK output for serial communication with Spark */
	DDRB=0b00010100;

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
		PORTB = 0b00000100;
		// give Spark 1.5 seonds to start and wait for PinB3 which is the busy pin of the spark
		// 2015-08-14 Wolf: changed from 1.5s to 2.5s since with photon sometime it did not work
		_delay_ms(2500);
		while (PINB & (1 << PINB3))
		{
			wdt_init(WDTO_250MS);
			sleep_now();
		}
		//ok, spark has finished, now get sleep time
		uint16_t timeToSleep = GetSleepTimeFromSpark();		
		
		// now we also wait for PinB1 which is the busy pin of the spectra display
		while (PINB & (1 << PINB1))
		{
			wdt_init(WDTO_250MS);
			sleep_now();
		}
		
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
