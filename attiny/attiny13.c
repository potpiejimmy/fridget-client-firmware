/*
 * GccApplication1.c
 *
 * Created: 08.11.2014 18:44:30
 *  Author: Wolf
 */ 
#define F_CPU 1250000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
//#include <avr/power.h>

void wdt_init() {
	cli();
	wdt_enable(WDTO_8S);
	//MCUSR |= (1<<WDRF);
	WDTCR |= (1<<WDE) | (1<<WDTIE);   // Enable watchdog
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


int main(void)
{
	int i = 0;
	DDRB=0b00000001;
	//MCUCR=0b00010000;

	//PCMSK = (1<<PCINT1);	// pin change mask: listen to portb bit 2
	//GIMSK |= (1<<PCIE);	// enable PCINT interrupt
	//MCUCR |= (1<<ISC01);
	//MCUCR &= 0b11111110;
	//sei();

	//wdt_init();
	
	PORTB = 0b00000000;  // disable LDO power
	_delay_ms(3000);
	
    while(1)
    {
		PORTB = 0b00000001;
		_delay_ms(3000);
		while (PINB & (1 << PINB1))
		{
			_delay_ms(200);
		}
		PORTB = 0b00000000;  // disable LDO power
//		_delay_ms(20000);
//		_delay_ms(20000);
		//set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//
		  //cli();
		  //WDTCR |= (1<<WDTIE); // WDT Interrupt enable
		  //sei();
//		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		//sleep_enable();
	//	sleep_mode();

		wdt_init();
		sleep_now();
//		wdt_disable();
        //TODO:: Please write your application code 
		i++;
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
