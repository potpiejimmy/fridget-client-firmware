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
#define BATTERY A0

namespace com_myfridget
{
    // some helper methods defined below
    void InitializeSPI();
    void UninitializeSPI();
	int GetBatteryLoad();
	void InitializeBatteryOverlay();
	
	byte[][] overlayRaster = new byte[14][2];
	byte[][] overlayPic = new byte [14][2];
	
	byte getOverlay(byte* arr, int x, int y);

#ifdef _EPD_LARGE_SCREEN_
        const int NUMBER_OF_LINES = 1600;
        const int SIZE_OF_LINE = 60;
#else
        const int NUMBER_OF_LINES = 600;
        const int SIZE_OF_LINE = 50;
#endif        

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
		InitializeBatteryOverlay();
        digitalWrite(TC_CS, LOW);
        //wait the minimal time of 1ms according to spec
        delayRealMicros(1000);  //min 1
        //send the header and wait minimal time
#ifdef _EPD_LARGE_SCREEN_
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
                    avail = in->read((unsigned char*)_buf, need);
                    bufIndex = 0;
                }
                SPI.transfer((_buf[bufIndex] & getOverlay(overlayRaster,x,y)) | getOverlay(overlayPic,x,y));
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
#ifdef _EPD_LARGE_SCREEN_
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
		float batteryVoltage = 14.7/10 * (3.27 * analogRead(BATTERY)) / ((float) 4095);
		float normVoltage = (batteryVoltage - 3.56)*25;
		if (normVoltage<0) return 0;
		if (normVoltage>10) return 10;
		return (int) normVoltage;
	}

	void InitializeBatteryOverlay()
	{
		// overlay is as follows ('-'= transparent, 'B'=black, ' '=white, '='=red) :
		// --BBBBBBBBBBBBB-
		// --BBBBBBBBBBBBB-
		// --BB=====     BB
		// --BB=====     BB
		// --BB=====     BB
		// --BBBBBBBBBBBBB-
		// --BBBBBBBBBBBBB-
		// the number of = corresponds to the batteryLoad.
		int batteryLoad = GetBatteryLoad();
		// fill according to batteryLoad
		byte fillLeft, fillRight;
		switch(batteryLoad)
		{
			case 0: fillLeft = 0;
					fillRight = 0;
					break;
			case 1: fillLeft = 8;
					fillRight = 0;
					break;
			case 2: fillLeft = 12;
					fillRight = 0;
					break;
			case 3: fillLeft = 14;
					fillRight = 0;
					break;
			case 4: fillLeft = 15;
					fillRight = 0;
					break;
			case 5: fillLeft = 15;
					fillRight = 128;
					break;
			case 6: fillLeft = 15;
					fillRight = 192;
					break;
			case 7: fillLeft = 15;
					fillRight = 224;
					break;
			case 8: fillLeft = 15;
					fillRight = 240;
					break;
			case 9: fillLeft = 15;
					fillRight = 248;
					break;
			case 10:fillLeft = 15;
					fillRight = 252;
					break;
			default:fillLeft = 0;
					fillRight = 0;
					break;
		}					
		overlayRaster[0][0]=0b11000000;
		overlayRaster[0][1]=0b00000001;
		overlayRaster[1][0]=0b11000000;
		overlayRaster[1][1]=0b00000001;
		overlayRaster[2][0]=0b11000000;
		overlayRaster[2][1]=0b00000000;
		overlayRaster[3][0]=0b11000000;
		overlayRaster[3][1]=0b00000000;
		overlayRaster[4][0]=0b11000000;
		overlayRaster[4][1]=0b00000000;
		overlayRaster[5][0]=0b11000000;
		overlayRaster[5][1]=0b00000001;
		overlayRaster[6][0]=0b11000000;
		overlayRaster[6][1]=0b00000001;
		overlayRaster[7][0]=0b11000000;
		overlayRaster[7][1]=0b00000001;
		overlayRaster[8][0]=0b11000000;
		overlayRaster[8][1]=0b00000001;
		overlayRaster[9][0]=0b11000000;
		overlayRaster[9][1]=0b00000000;
		overlayRaster[10][0]=0b11000000;
		overlayRaster[10][1]=0b00000000;
		overlayRaster[11][0]=0b11000000;
		overlayRaster[11][1]=0b00000000;
		overlayRaster[12][0]=0b11000000;
		overlayRaster[12][1]=0b00000001;
		overlayRaster[13][0]=0b11000000;
		overlayRaster[13][1]=0b00000001;
		overlayPic[0][0]= 0b00111111;
		overlayPic[0][1]=0b11111110;
		overlayPic[1][0]= 0b00111111;
		overlayPic[1][1]=0b11111110;
		overlayPic[2][0]= 0b00110000 | fillLeft;
		overlayPic[2][1]=0b00000011 | fillRight;
		overlayPic[3][0]= 0b00110000 | fillLeft;
		overlayPic[3][1]=0b00000011 | fillRight;
		overlayPic[4][0]= 0b00110000 | fillLeft;
		overlayPic[4][1]=0b00000011 | fillRight;
		overlayPic[5][0]= 0b00111111;
		overlayPic[5][1]=0b11111110;
		overlayPic[6][0]= 0b00111111;
		overlayPic[6][1]=0b11111110;
		overlayPic[7][0]= 0b00111111;
		overlayPic[7][1]=0b11111110;
		overlayPic[8][0]= 0b00111111;
		overlayPic[8][1]=0b11111110;
		overlayPic[9][0]= 0b00110000;
		overlayPic[9][1]=0b00000011;
		overlayPic[10][0]= 0b00110000;
		overlayPic[10][1]=0b00000011;
		overlayPic[11][0]= 0b00110000;
		overlayPic[11][1]=0b00000011;
		overlayPic[12][0]= 0b00111111;
		overlayPic[12][1]=0b11111110;
		overlayPic[13][0]= 0b00111111;
		overlayPic[13][1]=0b11111110;
	}
	/* 
	* this method returns 1 wherever the original pixel shall remain and 0 where the original pixel shall be set to white
	* y = line number
	* x = byte number in line
	*/
	byte getOverlay(byte* arr, int x, int y)
	{
		int yOverlay = y/(NUMBER_OF_LINES/2)*7+(y % (NUMBER_OF_LINES/2))-NUMBER_OF_LINES/2+9;
		if (yOverlay < 0 || yOverlay > 13) return 0xFF;
		else 
		{
			if (x>1) return 0xFF
			else return arr[yOverlay][x];
		}		
	}
}
