// implements the commands in header file
#include "SpectraCommands.h"
// some helper methods
void InitializeSPI();

void ActivateDisplay()
{
    pinMode(D3, OUTPUT);
    // initialize D4 to HIGH = inactive
    // TC_CS is used for sending commands to the TCon
    pinMode(D4, OUTPUT);
    digitalWrite(D4, HIGH);
    // set the output value to LOW which will activate the display
    digitalWrite(D3, LOW);
    // wait for the display to be initialized. 
    // this information is read from TC_BUSY output of TCon board
    // It takes 3ms according the spec till TC_BUSY is active, so let's wait here 10ms to be sure
    delay(10);
    // now loop till TC_BUSY is inactive (high)
    // no, to save power we simply wait another 250ms, since 200ms is the max for init time of Tcon
    delay (250);
}

void ClearDisplay()
{
    InitializeSPI();
    // now send the command
    digitalWrite(D4, LOW);
    // wait according to spec min 6µs
    delayMicroseconds(100);
    // UploadImageData, see spec of TCON
    SPI.transfer(0x20);
    SPI.transfer(0x01);
    SPI.transfer(0x00);
    SPI.transfer(0xFA); //maximal 251 = 0xFA wegen Commandlaenge maximal 255. Bild muss unterteilt werden
    // und mehrmals muss UploadImageData aufgerufen werden.
    
    // usw. hier halt senden...
    // schreibe ich erst fertig wenn das EPD format für Spectra bekannt ist.
    
    // this command returns two byte in MISO. Duplex communication.
    // send out zeros to get the return value
    byte ret1 = SPI.transfer(0x00);
    byte ret2 = SPI.transfer(0x00);
    // and close command by setting TC_CS to HIGH again
    digitalWrite(D4, HIGH);
}

void ShowImageOnDisplay(byte* image)
{
}

/*
* Private helper methods 
*/
void InitializeSPI()
{
    // initialize SPI communication on spark
    SPI.begin();
    // setting the clock devider to 4, assuming
    // that spark clock is 8Mhz and that TCon module
    // of spectry display works up to 3Mhz, so I set to 4, 
    // saying that TCon is operated at 2Mhz
    SPI.setClockDivider(SPI_CLOCK_DIV4) ;
    // setting bit order to MSBFirst according the TCon module spec
    SPI.setBitOrder(MSBFIRST);
    // according to TCon Spec CPOL = 1 and CPHA = 1 which corresponds to mode 3
    // see also http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus
    SPI.setDataMode(SPI_MODE3);
}