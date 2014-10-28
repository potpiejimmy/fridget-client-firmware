// implements the commands in header file
#include "SpectraCommands.h"
// some helper methods
void InitializeSPI();
void UninitializeSPI();

#define TC_EN D7
#define TC_CS D3
#define TC_BUSY D5

bool SPI_INITIALIZED = false;

void ActivateDisplay()
{
    if(!SPI_INITIALIZED) InitializeSPI();
    // TC_CS is used for sending commands to the TCon. Default is disabled = HIGH.
    digitalWrite(TC_CS, HIGH);
    // set the output value to LOW which will activate the display
    digitalWrite(TC_EN, HIGH);
    // wait for the display to be initialized. 
    // this information is read from TC_BUSY output of TCon board
    // It takes 3ms according the spec till TC_BUSY is active, so let's wait here 10ms to be sure
    delay(10);
    // now loop till TC_BUSY is inactive (high)
    // no, to save power we simply wait another 250ms, since 200ms is the max for init time of Tcon
    delay (2500);
}

void DeactivateDisplay()
{
    if(SPI_INITIALIZED)
    {
        digitalWrite(TC_CS, LOW);
        // set the output value to LOW which will activate the display
        digitalWrite(TC_EN, LOW);
        
        UninitializeSPI();
        delay (2000);
    }
}

void ClearDisplay()
{
    if(!SPI_INITIALIZED) InitializeSPI();
    // now send the command (TC_CS = LOW)
    digitalWrite(TC_CS, LOW);
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
    //byte ret1 = SPI.transfer(0x00);
    //byte ret2 = SPI.transfer(0x00);
    // and close command by setting TC_CS to HIGH again
    digitalWrite(TC_CS, HIGH);
}

int RefreshDisplay()
{
    if(!SPI_INITIALIZED) InitializeSPI();
    digitalWrite(TC_CS, LOW);
    // wait according to spec min 6µs
    delayMicroseconds(1000000);
    // UploadImageData, see spec of TCON
    SPI.transfer(0x24);
    SPI.transfer(0x01);
    SPI.transfer(0x00);

    delayMicroseconds(1000000);

    digitalWrite(TC_CS, HIGH);
    
    delay(5000);

    digitalWrite(TC_CS, LOW);
    // wait according to spec min 6µs
    delayMicroseconds(1000000);
    byte ret1 = SPI.transfer(0x00);
    byte ret2 = SPI.transfer(0x00);
    delayMicroseconds(1000000);

    digitalWrite(TC_CS, HIGH);
    
    int retval = 0;
    retval |= ret1;
    retval = retval << 8;
    retval |= ret2;
    return retval;
}

void ShowImageOnDisplay(byte* image)
{
}

/*
* Private helper methods 
*/
void InitializeSPI()
{
    pinMode(TC_EN, OUTPUT);
    pinMode(TC_CS, OUTPUT);

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
    
    SPI_INITIALIZED = true;
}

void UninitializeSPI()
{
    // initialize SPI communication on spark
    SPI.end();

    SPI_INITIALIZED = false;
}



// JUST CODE FOR ME TO SHOW HOW FLASH ACCESS WORKS
int SparkFlash_read(int address)
{
  if (address & 1)
    return -1; // error, can only access half words

  uint8_t values[2];
  sFLASH_ReadBuffer(values, 0x80000 + address, 2);
  return (values[0] << 8) | values[1];
}

int SparkFlash_write(int address, uint16_t value)
{
  if (address & 1)
    return -1; // error, can only access half words

  uint8_t values[2] = {
    (uint8_t)((value >> 8) & 0xff),
    (uint8_t)(value & 0xff)
  };
  sFLASH_WriteBuffer(values, 0x80000 + address, 2);
  return 2; // or anything else signifying it worked
}

/*
Additionally, as you can see and probably guess, it's much more efficient to write large buffers than to write a half word at a time. If your surrounding code:

    guarantees even addresses and numbers of bytes
    adds 0x80000 to addresses, and
    converts data to/from a byte array

then you would do better to just call sFLASH_ReadBuffer or sFLASH_WriteBuffer directly with a larger number of bytes as the final argument.
*/