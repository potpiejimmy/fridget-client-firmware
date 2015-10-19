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
	void InitializeOverlay(uint8_t step, uint8_t count);
	
    /* 9 lines red. 9 lines black. 2 bytes for battery, 5 bytes for image substep (max 5 images) */
	byte overlayRaster[18][7];
	byte overlayPic[18][7];
	
	byte getOverlay(byte arr[][7], int x, int y, byte neutralValue);

#if EPD_SCREEN_TYPE==1
        const int NUMBER_OF_LINES = 1600;
        const int SIZE_OF_LINE = 60;
#else
        const int NUMBER_OF_LINES = 600;
        const int SIZE_OF_LINE = 50;
#endif        

    void ShowImage(LLInputStream* in, uint8_t step, uint8_t count)
    {
        InitializeSPI();
       // delay(1000); //TODO: unclear how much time we need after SPI initialization. Maybe 0 is ok. To be tried....
       // tested. Works fine without delay ;-)
        // now set EN and CS signal to HIGH. EN=HIGH enables the display. CS = HIGH means inactive (no command yet).
        digitalWrite(TC_EN, HIGH);
        digitalWrite(TC_CS, HIGH);
        // wait the minimal time of 50ms according to spec before starting the command sequence with CS=LOW
        delayRealMicros(50000); //min 50
		InitializeOverlay(step, count);
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
                    avail = in->read((unsigned char*)_buf, need);
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

	void InitializeOverlay(uint8_t step, uint8_t count)
	{
		// overlay is as follows ('-'= transparent, 'B'=black, ' '=white, '='=red) :
        //
		// BBBBBBBBBBBBB           BBBBBBB                
		// BBBBBBBBBBBBB           BBBBBBB                
		// BB=====     BB    ===   BB===BB   ===     ===
		// BB=====     BB    ===   BB===BB   ===     ===
		// BB=====     BB    ===   BB===BB   ===     ===
		// BBBBBBBBBBBBB           BBBBBBB                
		// BBBBBBBBBBBBB           BBBBBBB                
        //
		// the number of = corresponds to the batteryLoad.
		int batteryLoad = GetBatteryLoad();
		// fill according to batteryLoad
		byte fillLeft, fillRight;
		switch(batteryLoad)
		{
			case 0: fillLeft = 0;
					fillRight = 0;
					break;
			case 1: fillLeft = 16;
					fillRight = 0;
					break;
			case 2: fillLeft = 24;
					fillRight = 0;
					break;
			case 3: fillLeft = 28;
					fillRight = 0;
					break;
			case 4: fillLeft = 30;
					fillRight = 0;
					break;
			case 5: fillLeft = 31;
					fillRight = 0;
					break;
			case 6: fillLeft = 31;
					fillRight = 128;
					break;
			case 7: fillLeft = 31;
					fillRight = 192;
					break;
			case 8: fillLeft = 31;
					fillRight = 224;
					break;
			case 9: fillLeft = 31;
					fillRight = 240;
					break;
			case 10:fillLeft = 31;
					fillRight = 248;
					break;
			default:fillLeft = 0;
					fillRight = 0;
					break;
		}					
        /* overlay raster needed for transparency. initialize raster to 0 for the first three bytes (2x battery, 1x substep.) rest is transparent */
        for (int i=0; i<18; i++)
            for (int j=0; j<2; j++)
                overlayRaster[i][j]=0b00000000;
        for (int i=0; i<18; i++)
            for (int j=2; j<7; j++)
            {
                overlayRaster[i][j]=0b11111111;
                overlayPic[i][j]=0b00000000;
            }
        
        /* overlay pic red or black for battery symbol */
        for (int i=0; i<18; i++)
        {
            if (i==0 || i==8 || i==9 || i==17)
            {
                overlayPic[i][0]=0b00000000;
                overlayPic[i][1]=0b00000000;           
                continue;
            }
            if (i==1 || i==2 || i==6 || i==7 || i==10 || i==11 || i==15 || i==16)
            {
                overlayPic[i][0]=0b01111111;
                overlayPic[i][1]=0b11111100;
                continue;                
            }
            if (i==3 || i==4 || i==5)
            {
                overlayPic[i][0]=0b01100000 | fillLeft;
                overlayPic[i][1]=0b00000110 | fillRight;
                continue;
            }
            if (i==12 || i==13 || i==14)
            {
                overlayPic[i][0]=0b01100000;
                overlayPic[i][1]=0b00000110;
                continue;
            }
        }
        
        /* now create the substep overlay */
        for (int z=0; z<count; z++)
        {
            /* first shift all to the right */
            for (int j=6; j>1; j--)
                for (int i=0; i<18; i++)
                {
                    overlayPic[i][j]=overlayPic[i][j-1];
                    overlayRaster[i][j]=overlayRaster[i][j-1];
                }
         
            /* and now set byte correctly */
            for (int i=0; i<18; i++)
            {
                if (i==0 || i==8 || i==9 || i==17)
                {
                    overlayPic[i][2]=0b00000000;
                    continue;
                }
                if (i==1 || i==2 || i==6 || i==7 || i==10 || i==11 || i==15 || i==16)
                {
                    if (z==step)
                        overlayPic[i][2]=0b11111110;
                    else
                        overlayPic[i][2]=0b00000000;
                    continue;                
                }
                if (i==3 || i==4 || i==5)
                {
                    if (z==step)
                        overlayPic[i][2]=0b11111110;
                    else
                        overlayPic[i][2]=0b00111000;
                    continue;                
                }
                if (i==12 || i==13 || i==14)
                {
                    if (z==step)
                        overlayPic[i][2]=0b11000110;
                    else
                        overlayPic[i][2]=0b00000000;
                    continue;                
                }
            }
        }
	}
	/* 
	* this method returns 1 wherever the original pixel shall remain and 0 where the original pixel shall be set to white
	* y = line number
	* x = byte number in line
	*/
	byte getOverlay(byte arr[][7], int x, int y, byte neutralValue)
	{
		int yOverlay;
		int offset = 0;
		if (y<(NUMBER_OF_LINES/2))
		{
			yOverlay = y-NUMBER_OF_LINES/2+11;
		}
		else
		{
			yOverlay = y-NUMBER_OF_LINES+11;
			offset += 9;			
		}
		if (yOverlay < 0 || yOverlay > 8) return neutralValue;
		else 
		{
			if (x>6) return neutralValue;
			else return arr[yOverlay+offset][x];
		}		
	}
}
