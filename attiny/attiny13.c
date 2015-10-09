/*
 * Main.c
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

/* define the wake up modes */
/* next step mode, i.e. the wait time has expired and the photon/spark should execute next program step */
#define WAKEUP_MODE_NEXTSTEP 0
/* switch image, i.e. the user has pressed the button so the system should wake up to switch image */
#define WAKEUP_MODE_SWITCHIMAGE 1
/* go online, i.e. the user has pressed button longer than 1s and the spark/photon shall wake up and go online immediately */
#define WAKEUP_MODE_GOONLINE 3

/* holds the information in which mode the system woke up */
volatile int g_wakeupMode;

/* global variable that holds the number of cycles to sleep in SleepLong method */
volatile uint16_t g_cyclesToSleep;

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

void SleepLong()
{
	/* sleep the number of given cycles
	   long-time tests showed that one sleep takes approx. 8.408 seconds
	   we do not know if the difference to exactly 8 seconds is caused
	   by inexactness of watchdog oscillator and/or by time needed for waking
	   up and going to sleep again */

	/* enable pin change interrupt on pin pcint0 only */
	PCMSK = (1<<PCINT0);
	
	/* go to sleep for the number of cycles given by spark/photon */
	for (uint16_t i=0 ; i<g_cyclesToSleep; i++)
	{
		/* initialize the watchdog */
		wdt_init(WDTO_8S);
		/* and sleep */
		sleep_now();
		/* if woke up by button press, then remember remaining cycles and leave sleep loop */
		if (g_wakeupMode <> WAKEUP_MODE_NEXTSTEP) 
		{
			/* set sleep time to remaining cycles minus 2 (~16s for display update time) */
			g_cyclesToSleep = g_cyclesToSleep-i-2;
			/* leave the for loop */
			break;
		}
	}
	
	/* disable interrupts on pin */
	PCMSK = 0b00000000;
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

/* transfers the wake up mode to spark/photon via bitbanging using same pins as GetSleepTimeFromSpark */
void TransferWakeUpModeToSpark()
{
	/* first we set the very first bit already: if mode = 1 or 3, the last bit is set = button pressed */
	PORTB |= ((g_wakeupMode & 1)<<PINB4);
	/* now wait till spark/photon is ready, i.e. PINB3 goes to high */
	while (PINB & (1 << PINB3) == 0);
	/* ...and set second bit */
	PORTB |= ((g_wakeupMode & 0b10)<<PINB4);
}

/* enables the interrupts on pin0 */
/* pin0 is used to wake up attiny from sleep mode by button */
/* internal pull-up will set level to HIGH. Button will connect to GND and */
/* thus generate a high to low edge */
void EnablePinChangeInterrupt()
{
	/* enable the internal pull-up resistor for PIN0 by setting to HIGH */
	PORTB |= (1<<PCINT0);
	/* define the trigger type: trigger on falling edge isc01=1, isc00=0 */
	MCUCR |= (1<<ISC01);
	/* enable pin change interrupts */
	GIMSK |= (1<<PCIE);
}


/* the main routine. startup, wait for spark/photon and display update, sleep */
int main(void)
{
	/* define the port usage: PINB2 is output for LDO shutdown
	   define port PB4 as CLK output for serial communication with Spark */
	DDRB=0b00010100;
	
	/* initialize the wake up mode with 0 = next step mode */
	g_wakeupMode = WAKEUP_MODE_NEXTSTEP;
	/* initialize the sleep cycles with zero */
	g_cyclesToSleep = 0;

	/* disable power and wait three seconds to ensure stable start of LDO and spark after three seconds
	   this time can surely be reduced, but it will only play a role after plugging system to battery */
	PORTB = 0b00000000;  // disable LDO power and set all ports to LOW
	_delay_ms(3000);
	
	EnablePinChangeInterrupt();

    while(1)
    {
		/* turn on the LDO which will power the Spark */
		PORTB |= (1<<PINB2); 
		/* transfer wake up mode to spark/photon */
		TransferWakeUpModeToSpark();
		/* wait till spark/photon puts PINB3=D1 to low */
		while (PINB & (1 << PINB3))
		{
			wdt_init(WDTO_250MS);
			sleep_now();
		}
		/* ok, spark has finished, now get sleep time if in normal or online mode */
		if (g_wakeupMode <> WAKEUP_MODE_SWITCHIMAGE)
			g_cyclesToSleep = GetSleepTimeFromSpark();		

		/* and set back the button pressed variable to false */
		g_wakeupMode = WAKEUP_MODE_NEXTSTEP;
		/* set PINB4 to LOW (leave switch image mode) */
		//PORTB &= 0b11101111;
		
		/* now we also wait for PinB1 which is the busy pin of the spectra display */
		while (PINB & (1 << PINB1))
		{
			wdt_init(WDTO_250MS);
			sleep_now();
		}
		
		/* now spark and display is ready...turn off power... */
		PORTB &= 0b11111011;  // disable LDO power
	
		/* and wait long time till next spark action... */
		SleepLong();
    }
}

/* Interrupt routine executed when button is pressed */
ISR(PCINT0_vect)	     
{			     
	/* set the button pressed variable to true */
	g_wakeupMode = WAKEUP_MODE_SWITCHIMAGE;
	/* set PINB4 to HIGH to let the spark/photon know that we woke up from button press (switch image mode) */
	//PORTB |= (1<<PINB4);
        /* now wait till button is released. If this takes more than a second, we are in Go Online mode /*
	/* count the number of 100ms wait time */
	int i = 0;
        while (PINB & 1)
        {
            _delay_ms(100);
            i++;
        }
        /* if we waited more than 10 times 100ms, i.e. one second, we are on Go Online mode */
        if (i>=10)
            g_wakeupMode = WAKEUP_MODE_GOONLINE;
}


/* this one is needed as watchdog wakeup routine. Removing this will not work */
ISR(WDT_vect)
{
	/* do nothing */
}
