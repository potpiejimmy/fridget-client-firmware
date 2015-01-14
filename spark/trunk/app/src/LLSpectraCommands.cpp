/* 
 * File:   LLSpectraCommands.cpp
 * Author: wolfram
 *
 */

// implements the commands in header file
#include "LLSpectraCommands.h"

// include application.h for global definitions
#include "application.h"

// this is the spark pins we will use beside the SPI pins.
// for the meaning of EN and CS and BUSY, refer to ApplicationNote_EPD441_Spectra_v01.pdf in common/docs/Specifications
#define TC_EN D7
#define TC_CS D3
#define TC_BUSY D5

namespace com_myfridget
{
    // some helper methods defined below
    void InitializeSPI();
    void UninitializeSPI();

    void ShowImage(LLInputStream* in)
    {
        InitializeSPI();
       // delay(1000); //TODO: unclear how much time we need after SPI initialization. Maybe 0 is ok. To be tried....
       // tested. Works fine without delay ;-)
        // now set EN and CS signal to HIGH. EN=HIGH enables the display. CS = HIGH means inactive (no command yet).
        digitalWrite(TC_EN, HIGH);
        digitalWrite(TC_CS, HIGH);
        // wait the minimal time of 50ms according to spec before starting the command sequence with CS=LOW
        delayRealMicros(50000); //min 50
        digitalWrite(TC_CS, LOW);
        //wait the minimal time of 1ms according to spec
        delayRealMicros(1000);  //min 1
        //send the header and wait minimal time
        SPI.transfer(0x08);
        SPI.transfer(0xB0);
        delayRealMicros(5000); // min 5
        
        // now start writing the image
        int bufIndex=0;
        int avail=0;
        for (int y = 0; y<600; y++)
        {
            for (int x = 0; x<50; x++)
            {
                if (bufIndex == avail)
                {
                    int need = (599-y)*50+(50-x);
                    if (need > _BUF_SIZE) need = _BUF_SIZE;
                    avail = in->read((unsigned char*)_buf, need);
                    bufIndex = 0;
                }
                SPI.transfer(_buf[bufIndex]);
                delayMicroseconds(1); //min 1
                bufIndex++;
            }
            delayRealMicros(1000); //typ 1
           // if (y==299) delayRealMicros(1000);  // this one definitely is required between the two frames. Otherwise display shows nonsense.
        } 
        digitalWrite(TC_CS, HIGH);
		delayRealMicros(200000);
		UninitializeSPI();
//        while (digitalRead(TC_BUSY)==HIGH)        
//        {
//            delayRealMicros(200000);
//        }
//
//        digitalWrite(TC_CS, LOW);
//        digitalWrite(TC_EN, LOW);
    }
  
    
    /*
    * Private helper methods 
    */
    void InitializeSPI()
    {
        pinMode(TC_EN, OUTPUT);
        pinMode(TC_CS, OUTPUT);
        pinMode(TC_BUSY, INPUT);
    
        // initialize SPI communication on spark
        SPI.begin();
        // setting the clock devider to 16, assuming
        // that spark clock is 8Mhz and that TCon module
        // of spectra display works up to 500Khz, so I set to 16, 
        // saying that TCon is operated at 500Khz (upper limit)
        SPI.setClockDivider(_clockDivisor == 1 ? SPI_CLOCK_DIV16 : SPI_CLOCK_DIV2) ;
        // setting bit order to MSBFirst according the TCon module spec
        SPI.setBitOrder(MSBFIRST);
        // according to TCon Spec CPOL = 0 (CLK is low when not sending data) and CPHA = 0 (Bit wird bei positiver Flanke ausgelesen) which corresponds to mode 0
        // see also http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus
        SPI.setDataMode(SPI_MODE0);
    }
    
    void UninitializeSPI()
    {
        // initialize SPI communication on spark
        SPI.end();
    }


/*
// just to keep this one. It is a sequence that worked. writing alternating vertical lines of white, red, black.
    void ShowImage(uint32_t address)
    {
        InitializeSPI();
        delay(1000); //unknown
        digitalWrite(TC_EN, HIGH);
        digitalWrite(TC_CS, HIGH);
        delay(50); //min 50
        digitalWrite(TC_CS, LOW);
        delay(1);  //min 1
        SPI.transfer(0x08);
        SPI.transfer(0xB0);
        delay(5); // min 5
        for (int y = 0; y<300; y++)
        {
            for (int x = 0; x<16; x++)
            {
                SPI.transfer(0x49);
                delayMicroseconds(1); //min 1
                SPI.transfer(0x24);
                delayMicroseconds(1); //min 1
                SPI.transfer(0x92);
                delayMicroseconds(1); //min 1
            }
            SPI.transfer(0x49);
            delayMicroseconds(1); //min 1
            SPI.transfer(0x24);
            delayMicroseconds(1); //min 1
            delay(1); //typ 1
        } 
        delay(1); // typ 1
        for (int y = 0; y<300; y++)
        {
            for (int x = 0; x<16; x++)
            {
                SPI.transfer(0x24);
                delayMicroseconds(1); //min 1
                SPI.transfer(0x92);
                delayMicroseconds(1); //min 1
                SPI.transfer(0x49);
                delayMicroseconds(1); //min 1
            }
            SPI.transfer(0x24);
            delayMicroseconds(1); //min 1
            SPI.transfer(0x92);
            delayMicroseconds(1); //min 1
            delay(1); //typ 1
        }
        digitalWrite(TC_CS, HIGH);
        do
        {
            delay(200);
        }
        while (digitalRead(TC_BUSY)==HIGH);
        digitalWrite(TC_CS, LOW);
        digitalWrite(TC_EN, LOW);
        UninitializeSPI();
    }*/
}
