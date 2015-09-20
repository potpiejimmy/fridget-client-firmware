/* 
 * File:   LLSpectraCommands.cpp
 * Author: wolfram
 *
 */

// implements the commands in header file
#include "LLSpectraCommands.h"

// include application.h for global definitions
#include "application.h"

#define BATTERY A0

namespace com_myfridget
{
    // some helper methods defined below
    void InitializeSPI();
    void UninitializeSPI();
	int GetBatteryLoad();
	void InitializeBatteryOverlay();
	
	byte overlayRaster[14][2];
	byte overlayPic[14][2];
	
	byte getOverlay(byte arr[][2], int x, int y, byte neutralValue);

#if EPD_SCREEN_TYPE==1
        const int NUMBER_OF_LINES = 1600;
        const int SIZE_OF_LINE = 60;
#else
        const int NUMBER_OF_LINES = 600;
        const int SIZE_OF_LINE = 50;
#endif        

    void ShowImage(LLInputStream* in)
    {
        // XXX just a test:
        memset(_buf, 1, _BUF_SIZE);
        
        InitializeSPI();
       // delay(1000); //TODO: unclear how much time we need after SPI initialization. Maybe 0 is ok. To be tried....
       // tested. Works fine without delay ;-)
        // now set EN and CS signal to HIGH. EN=HIGH enables the display. CS = HIGH means inactive (no command yet).
        digitalWrite(TC_EN, HIGH);
        digitalWrite(TC_CS, HIGH);
        // wait the minimal time of 50ms according to spec before starting the command sequence with CS=LOW
        delayRealMicros(50000); //min 50
		InitializeBatteryOverlay();
        digitalWrite(TC_CS, LOW);
        //wait the minimal time of 1ms according to spec
        delayRealMicros(1000);  //min 1
        //send the header and wait minimal time
#if EPD_SCREEN_TYPE==1
        SPI.transfer(0x04);
#else
        SPI.transfer(0x08);
#endif
        SPI.transfer(0xB0);
        delayRealMicros(5000); // min 5
        
        // now start writing the image
        int bufIndex=0;
        int avail=0;
        
        for (int y = 0; y<NUMBER_OF_LINES; y++)
        {
            for (int x = 0; x<SIZE_OF_LINE; x++)
            {
                if (bufIndex == avail)
                {
                    int need = ((NUMBER_OF_LINES-1)-y)*SIZE_OF_LINE+(SIZE_OF_LINE-x);
                    if (need > _BUF_SIZE) need = _BUF_SIZE;
                    avail = need;// XXX in->read((unsigned char*)_buf, need);
                    bufIndex = 0;
                }
                SPI.transfer((_buf[bufIndex] & getOverlay(overlayRaster,x,y, 0xFF)) | getOverlay(overlayPic,x,y, 0x00));
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
        // initialize SPI communication on spark
        SPI.begin();
		
		// setting the clock dividor to operate with correct clock period for TCon module
		// 4.41" Display: Tclk = 2.5µs (2µs min)  ==> 500kHz max.
		// 7"    Display: Tclk = 250ns (min)      ==> 4MHz max.
		// Spark System Clock: 8MHz
		// Photon System Clock: 60MHz
		// ==> to operate 7" with Photon: Divisor 15 is needed (60/15 = 4) ==> SPI_CLOCK_DIV16
		//
		// 7" Photon: DIV16
		// 7" Spark: DIV2
		// 4" Photon: DIV128
		// 4" Spark: DIV16
		//
		// ugly code here: unclear why/how __clockDivisor comes into game here:
#if EPD_SCREEN_TYPE==1
        SPI.setClockDivider(SPI_CLOCK_DIV2);
#else
        SPI.setClockDivider(_clockDivisor == 1 ? SPI_CLOCK_DIV16 : SPI_CLOCK_DIV2);
#endif
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
	* This method returns the battery load as integer from 0 to 10. 
	* 0 = empty battery, up to 10 = full battery.
	* A corresponding indicator will be displayed on spectra display.
	* We assume here that the battery voltage is connected to the BATTERY port defined above
	* and that the voltage is connected via a resistance divisor:
	
	* Battery Voltage --------
	*                        |
	*                       |R1|
	*                        |
	*                        |----- BATTERY Port Spark
	*                        |
	*                       |R2|
	*                        |
	*                       GND
	*
	* With R1 = 4.7kOhm and R2 = 10kOhm
	*
	* Supply Voltage of Spark is 3.27V, so maximum Battery Voltage is 4.8V !
	* Battery Voltage is calculated by sparkVoltage*(R1+R2)/R2
	* Battery load is then calculated by saying all above 4V is 10, and 3.6V is empty = 0.
	*/
	int GetBatteryLoad()
	{
		float batteryVoltage = ReadBatteryVoltage();
		float normVoltage = (batteryVoltage - 3.36)*17;
		if (normVoltage<0) return 0;
		if (normVoltage>10) return 10;
		return (int) normVoltage;
	}
	
	float ReadBatteryVoltage()
	{
		return 14.7/10 * (3.27 * analogRead(BATTERY)) / ((float) 4095);
	}

	void InitializeBatteryOverlay()
	{
		// overlay is as follows ('-'= transparent, 'B'=black, ' '=white, '='=red) :
		// BBBBBBBBBBBBB---
		// BBBBBBBBBBBBB---
		// BB=====     BB--
		// BB=====     BB--
		// BB=====     BB--
		// BBBBBBBBBBBBB---
		// BBBBBBBBBBBBB---
		// the number of = corresponds to the batteryLoad.
		int batteryLoad = GetBatteryLoad();
		// fill according to batteryLoad
		byte fillLeft, fillRight;
		switch(batteryLoad)
		{
			case 0: fillLeft = 0;
					fillRight = 0;
					break;
			case 1: fillLeft = 32;
					fillRight = 0;
					break;
			case 2: fillLeft = 48;
					fillRight = 0;
					break;
			case 3: fillLeft = 56;
					fillRight = 0;
					break;
			case 4: fillLeft = 60;
					fillRight = 0;
					break;
			case 5: fillLeft = 62;
					fillRight = 0;
					break;
			case 6: fillLeft = 63;
					fillRight = 0;
					break;
			case 7: fillLeft = 63;
					fillRight = 128;
					break;
			case 8: fillLeft = 63;
					fillRight = 192;
					break;
			case 9: fillLeft = 63;
					fillRight = 224;
					break;
			case 10:fillLeft = 63;
					fillRight = 240;
					break;
			default:fillLeft = 0;
					fillRight = 0;
					break;
		}					
		overlayRaster[0][0]=0b00000000;
		overlayRaster[0][1]=0b00000111;
		overlayRaster[1][0]=0b00000000;
		overlayRaster[1][1]=0b00000111;
		overlayRaster[2][0]=0b00000000;
		overlayRaster[2][1]=0b00000011;
		overlayRaster[3][0]=0b00000000;
		overlayRaster[3][1]=0b00000011;
		overlayRaster[4][0]=0b00000000;
		overlayRaster[4][1]=0b00000011;
		overlayRaster[5][0]=0b00000000;
		overlayRaster[5][1]=0b00000111;
		overlayRaster[6][0]=0b00000000;
		overlayRaster[6][1]=0b00000111;
		overlayRaster[7][0]=0b00000000;
		overlayRaster[7][1]=0b00000111;
		overlayRaster[8][0]=0b00000000;
		overlayRaster[8][1]=0b00000111;
		overlayRaster[9][0]=0b00000000;
		overlayRaster[9][1]=0b00000011;
		overlayRaster[10][0]=0b00000000;
		overlayRaster[10][1]=0b00000011;
		overlayRaster[11][0]=0b00000000;
		overlayRaster[11][1]=0b00000011;
		overlayRaster[12][0]=0b0000000;
		overlayRaster[12][1]=0b00000111;
		overlayRaster[13][0]=0b00000000;
		overlayRaster[13][1]=0b00000111;
		overlayPic[0][0]= 0b11111111;
		overlayPic[0][1]=0b11111000;
		overlayPic[1][0]= 0b11111111;
		overlayPic[1][1]=0b11111000;
		overlayPic[2][0]= 0b11000000 | fillLeft;
		overlayPic[2][1]=0b00001100 | fillRight;
		overlayPic[3][0]= 0b11000000 | fillLeft;
		overlayPic[3][1]=0b00001100 | fillRight;
		overlayPic[4][0]= 0b11000000 | fillLeft;
		overlayPic[4][1]=0b00001100 | fillRight;
		overlayPic[5][0]= 0b11111111;
		overlayPic[5][1]=0b11111000;
		overlayPic[6][0]= 0b11111111;
		overlayPic[6][1]=0b11111000;
		overlayPic[7][0]= 0b11111111;
		overlayPic[7][1]=0b11111000;
		overlayPic[8][0]= 0b11111111;
		overlayPic[8][1]=0b11111000;
		overlayPic[9][0]= 0b11000000;
		overlayPic[9][1]=0b00001100;
		overlayPic[10][0]= 0b11000000;
		overlayPic[10][1]=0b00001100;
		overlayPic[11][0]= 0b11000000;
		overlayPic[11][1]=0b00001100;
		overlayPic[12][0]= 0b11111111;
		overlayPic[12][1]=0b11111000;
		overlayPic[13][0]= 0b11111111;
		overlayPic[13][1]=0b11111000;
	}
	/* 
	* this method returns 1 wherever the original pixel shall remain and 0 where the original pixel shall be set to white
	* y = line number
	* x = byte number in line
	*/
	byte getOverlay(byte arr[][2], int x, int y, byte neutralValue)
	{
		int yOverlay;
		int offset = 0;
		if (y<(NUMBER_OF_LINES/2))
		{
			yOverlay = y-NUMBER_OF_LINES/2+9;
		}
		else
		{
			yOverlay = y-NUMBER_OF_LINES+9;
			offset += 7;			
		}
		if (yOverlay < 0 || yOverlay > 6) return neutralValue;
		else 
		{
			if (x<(SIZE_OF_LINE-2)) return neutralValue;
			else return arr[yOverlay+offset][x-(SIZE_OF_LINE-2)];
		}		
	}
}
