/**
 ******************************************************************************
 * @file    application.cpp
 * @authors  Satish Nair, Zachary Crockett and Mohit Bhoite
 * @version V1.0.0
 * @date    05-November-2013
 * @brief   Tinker application
 ******************************************************************************
  Copyright (c) 2013 Spark Labs, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/  
#include "application.h"

#include "LLMap.h"
#include "LLWebRequest.h"
#include "LLFlashUtil.h"
#include "LLSpectraCommands.h"
#include "LLFlashInputStream.h"
#include "LLBufferedBitInputStream.h"
#include "LLInflateInputStream.h"
#include "LLRLEInputStream.h"

// is power-on and power-off attiny controlled?
#define ATTINY_CONTROLLED_POWER
// EPD TCON board connected to core?
#define EPD_TCON_CONNECTED

// user states
#define USER_STATE_OFFLINE                0
#define USER_STATE_CONNECTING             1
#define USER_STATE_CONNECTED_AWAITING_IP  2
#define USER_STATE_ONLINE                 3
#define USER_STATE_ONLINE_WITH_CLOUD      4
#define USER_STATE_IDLE                   5

// connect modes (cloud on/off)
#define USER_CONNECT_MODE_CLOUD_ON   1
#define USER_CONNECT_MODE_CLOUD_OFF  2

//#define SERVER_HOST IPAddress(192,168,178,32)
//#define SERVER_HOST_DEBUGNAME "192.168.178.32"
//#define SERVER_PORT 8080
#define SERVER_HOST "www.doogetha.com"
#define SERVER_HOST_DEBUGNAME "www.doogetha.com"
#define SERVER_PORT 80

// The size of flash memory to reserve for one EPD image, must be a multiple of 4KB
#define SIZE_EPD_SEGMENT  0x8000

/* EEPROM entries */
#define EEPROM_ENTRY_PROGRAM_LENGTH  0
#define EEPROM_ENTRY_PROGRAM_COUNTER 1
#define EEPROM_ENTRY_RESERVED        2
#define EEPROM_ENTRY_PROGRAM_START   3

/* WIFI connect time out (ms) */
#define WIFI_CONNECT_TIMEOUT 30000

using namespace com_myfridget;

/* Allocate read buffer (Note: declared in application.h) */
char _buf[_BUF_SIZE];

/* Set up web requester */
LLWebRequest requester(SERVER_HOST, SERVER_PORT);
/* The current user state */
int userState;
/* the current clock divisor */
unsigned int _clockDivisor;
/* server parameter read buffer */
char serverParamsBuf[256];
/* server parameter map */
LLMap serverParams(16);
/* time we switched to connecting mode */
system_tick_t connectingStartTs;

/* Debugging -----------------------------------------------*/
#ifdef _SERIAL_DEBUGGING_
#define _DEBUG(msg) Serial.println(msg)
#else
#define _DEBUG(msg)
#endif

/* Function prototypes -------------------------------------------------------*/
void blinkLED(int on, int off);
void onOnline();
void executeOp();
void enterPowerSaveMode();
void updateDisplay(uint8_t imgNo);
void powerDown(uint16_t interval);
void flashImages();

/* ############################################################# */

/* MANUAL: not connecting to Spark cloud, running user-code loop immediately
 * on power-up. */
SYSTEM_MODE(MANUAL);

/* This function is called once at start up ----------------------------------*/
void setup()
{
    _clockDivisor = 1;
    
    /* start offline */
    userState = USER_STATE_OFFLINE;

    /* Activate the LED output PIN */
    pinMode(D7, OUTPUT);
    
    /* Attiny PINs: */
    pinMode(D0, OUTPUT); // Time Interval Bit DATA
    pinMode(D1, INPUT); // Time Interval Bit CLK (IN)
    pinMode(D4, OUTPUT); // Notification output BUSY
    /* Activate D4 for Attiny notification busy output */
    digitalWrite(D4, HIGH);

#ifdef _SERIAL_DEBUGGING_
    /* For serial debugging only: */
    // Make sure your Serial Terminal app is closed before powering your Core
    Serial.begin(9600);
#endif
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    uint8_t execLen;
    uint8_t execNo;
    
    switch (userState)
    {
    case USER_STATE_OFFLINE:
#ifdef _SERIAL_DEBUGGING_
        // Now open your Serial Terminal, and hit any key to continue!
        if (!Serial.available()) {
            blinkLED(96,96); // flicker LED while waiting
            return; // do nothing, wait for key press
        }
#endif
        _DEBUG("Core up.");
        execLen = EEPROM.read(EEPROM_ENTRY_PROGRAM_LENGTH);
        execNo = EEPROM.read(EEPROM_ENTRY_PROGRAM_COUNTER);
        _DEBUG(String("ExecLen=")+execLen+", ExecNo="+execNo);
        if (execNo >= execLen) {
            // connect to WiFi:
            WiFi.connect();
            userState = USER_STATE_CONNECTING;
            _DEBUG("State: USER_STATE_CONNECTING");
            connectingStartTs = millis();
            if (!WiFi.hasCredentials()) {
                // Note: if no credentials are available, the loop function
                // won't be called until smart config process completed, so
                // we have to update the display now:
                updateDisplay(1); // show setup image
            }
        } else {
            // execute next program step:
            executeOp();
        }
        break;

    case USER_STATE_CONNECTING:
        // do nothing while connecting, wait for ready
        if (WiFi.ready()) {
            userState = USER_STATE_CONNECTED_AWAITING_IP;
            _DEBUG("State: USER_STATE_CONNECTED_AWAITING_IP");
        } else if (millis() - connectingStartTs > WIFI_CONNECT_TIMEOUT) {
            // connecting timed out
            // XXX delete credentials and power down for 10 cycles
            WiFi.clearCredentials();
            powerDown(10);
        }
        break;
    
    case USER_STATE_CONNECTED_AWAITING_IP:
        // wait until IP address is set
        if (WiFi.localIP().raw_address()[0]) {
            userState = USER_STATE_ONLINE;
            _DEBUG("State: USER_STATE_ONLINE");
            onOnline();
        }
        break;
        
    case USER_STATE_ONLINE:
        // flash the image data
        flashImages();
        // execute next program step:
        executeOp();
        break;
        
    case USER_STATE_ONLINE_WITH_CLOUD:
        // call Spark.process() for cloud operations
        Spark.process();
        break;
    
    case USER_STATE_IDLE:
        // do nothing in idle mode
        break;
    }
}

const char* getServerParam(const char* param, const char* def)
{
    const char* value = serverParams.getValue(param);
    return value ? value : def;
}

bool establishServerConnection()
{
    char url[128];
    
    int numberOfRetries = 0;
    uint8_t* ipa = WiFi.localIP().raw_address();
    
    snprintf(url, 128, "/fridget/res/debug/%s", Spark.deviceID().c_str());
    
    /* 
     * When using Domain Names instead of IP addresses for TCPClient,
     * the first connection may fail due to a delayed name resolution.
     * So we try to send the initial message to the server twice.
     */

    while (numberOfRetries++ < 3) {
        // say hello to the server, log IP and SSID
        if (requester.request(
                "POST",
                url,
                (String("*** Connected to ") + WiFi.SSID() + ", IP " + ipa[0] + "." + ipa[1] + "." + ipa[2] + "." + ipa[3] + " ***").c_str(),
                serverParamsBuf,
                256)) {
            _DEBUG(String("<<< Received server parameters: ") + serverParamsBuf);
            serverParams.parse(serverParamsBuf);
            return TRUE;
        } else {
            _DEBUG(String("Could not connect to ") + SERVER_HOST_DEBUGNAME + "(" + numberOfRetries + ")");
        }
        delayRealMicros(100000);
    }
    return FALSE;
}

void onOnline()
{
    int connectMode = USER_CONNECT_MODE_CLOUD_ON;
    
    if (establishServerConnection()) {
        _DEBUG(String("Connected to ") + SERVER_HOST_DEBUGNAME);
        connectMode = atoi(getServerParam("connectmode", "1"/*USER_CONNECT_MODE_CLOUD_ON*/));
    } else {
        _DEBUG("Server not available, connecting to cloud.");
    }
    
    if (connectMode == USER_CONNECT_MODE_CLOUD_ON)
    {
        // Connecting cloud:
        Spark.connect();
        userState = USER_STATE_ONLINE_WITH_CLOUD;
        _DEBUG("State: USER_STATE_ONLINE_WITH_CLOUD");
        return;
    }
    
    const char* program = getServerParam("exec", "A0000");
    uint8_t programSize = strlen(program);
    _DEBUG(String("Received program: ") + program);
 
    EEPROM.write(EEPROM_ENTRY_PROGRAM_LENGTH, (uint8_t)programSize);
    EEPROM.write(EEPROM_ENTRY_PROGRAM_COUNTER, (uint8_t)0); // program counter
    //EEPROM.write(EEPROM_ENTRY_RESERVED, (uint8_t)0); // reset image no.
    for (int i=0; i<programSize; i++) EEPROM.write(EEPROM_ENTRY_PROGRAM_START+i, program[i]);
}

void executeOp()
{
    enterPowerSaveMode();
    
    uint8_t execNo = EEPROM.read(EEPROM_ENTRY_PROGRAM_COUNTER); // program counter
    char opName = EEPROM.read(EEPROM_ENTRY_PROGRAM_START+execNo);
    _DEBUG(String("Execute OP ") + opName);
    
    // for now, only '-' (NOOP) or 'A-Z' (IMG UDPATE) allowed)
    if (opName >= 'A') updateDisplay(opName - 'A');
    
    execNo++;
    
    // OP done, now powering down:
    char interval_s[5]; interval_s[4] = 0;
    for (int i=0; i<4; i++) interval_s[i] = EEPROM.read(EEPROM_ENTRY_PROGRAM_START+(execNo++));
    unsigned long interval = strtoul(interval_s, 0, 16);
    _DEBUG(String("Execute Sleep interval ") + interval);
    
    _DEBUG(String("Increasing execNo to ") + execNo);
    EEPROM.write(EEPROM_ENTRY_PROGRAM_COUNTER, execNo);
    
    powerDown((uint16_t)interval);
}

void enterPowerSaveMode()
{
    _DEBUG("Disabling WiFi and LED");
    // WiFi und RGB aus
    WiFi.off();
    RGB.control(true);
    RGB.color(0);
    
    _DEBUG("Set IWDG maximum timeout (about 26 sec.)");
    IWDG_Reset_Enable(0xFFFF); // set IWDG timeout to a maximum value
    
    // runtertakten:
#ifdef EPD_TCON_CONNECTED
    RCC_HCLKConfig(RCC_SYSCLK_Div8);
    _clockDivisor=8;
#endif
}

void updateDisplay(uint8_t imgNo)
{
    const int decodeBufSize = 1024;
    unsigned char decodeBuf[decodeBufSize];
    
    // write the image from flash to the display:
    _DEBUG(String("Updating display with image no. ") + imgNo);
    
    // Now link from FLASH to DECODEBUF to HUFFMAN-INFLATE to RLE-INFLATE
    LLFlashInputStream flashIn(imgNo * SIZE_EPD_SEGMENT);
    LLBufferedBitInputStream bufIn(&flashIn, decodeBuf, decodeBufSize);
    LLInflateInputStream inflateIn(&bufIn);
    LLRLEInputStream rleIn(&inflateIn);
    
#ifdef EPD_TCON_CONNECTED
    ShowImage(&rleIn);
#endif
}

void powerDown(uint16_t interval)
{
    // deep-sleep for interval cycles
    _DEBUG(String("Going to sleep with sleep interval #") + interval);
    
#ifdef ATTINY_CONTROLLED_POWER
    userState = USER_STATE_IDLE;
    _DEBUG("State: USER_STATE_IDLE");
    
    // perform bit-banging with Attiny.
    // D0 as Data (Attiny input, Spark output)
    // D1 as CLK (Attiny output, Spark input)
    const int MAXBIT = 0x8000; // highest bit to start with
    int clk = digitalRead(D1); // initial value of CLK
    for (int i=MAXBIT; i>0; i>>=1) {
        digitalWrite(D0, (interval&i) ? HIGH : LOW);
        // Notify Attiny about start of bit-banging after first bit is set
        if (i == MAXBIT) digitalWrite(D4, LOW);
        while (digitalRead(D1) == clk) delayRealMicros(1000);
        clk ^= HIGH;
    }
    
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
#else
#ifdef _SERIAL_DEBUGGING_
    delay(200);
#endif
    Spark.sleep(SLEEP_MODE_DEEP, interval);
#endif
}

void flashImages()
{
    char url[128];
    
    bool done = FALSE;
    snprintf(url, 128, "/fridget/res/img/%s/", Spark.deviceID().c_str());
    if (requester.sendRequest("GET", url, NULL))
    {
        int overallSize = 0;
        int badBlocks = 0;

        if (requester.readHeaders())
        {
            do
            {
                // one byte image index:
                uint8_t index = 0;
                int valread = requester.readAll((char*)&index, 1);
                if (!valread) { done = TRUE; break; }
                
                // two bytes of length
                char imgLenBuf[2];
                valread = requester.readAll(imgLenBuf, 2);
                int imgLen = ((int)imgLenBuf[0])<<8|imgLenBuf[1];
                if (!valread) { done = TRUE; break; }
                
                int readSoFar = 0;
                while (readSoFar < imgLen)
                {
                    int shouldRead = imgLen - readSoFar;
                    if (shouldRead > _BUF_SIZE) shouldRead = _BUF_SIZE;
                    _DEBUG(String("Reading image data ") + index + " [" + readSoFar + "-" + (readSoFar + shouldRead - 1) + "]");
                    int readNow = requester.readAll(_buf, shouldRead);

                    if (readNow != shouldRead)
                    {
                        _DEBUG(String("Failed, received only ") + readNow);
                        done = TRUE;
                        break;
                    }
                    // okay, burn it
                    _DEBUG("Writing to flash.");
                    if (!LLFlashUtil::flash((const uint8_t*)_buf, (index * SIZE_EPD_SEGMENT) + readSoFar, readNow)) {
                        badBlocks++;
                    }
                    _DEBUG("Done.");
                    readSoFar += readNow;
                }
                overallSize += readSoFar;
            } while (!done);
        }
        requester.stop();
        _DEBUG(String("Wrote ") + overallSize + " bytes to flash.");
        if (badBlocks > 0) {
            _DEBUG(String("XXX BAD FLASH BLOCKS: ") + badBlocks);
        }
    }
}

void blinkLED(int on, int off)
{
#ifndef RGB_NOTIFICATIONS_OFF
    digitalWrite(D7, HIGH);   // Turn ON the LED pins
#endif
    delayRealMicros(on*1000);
#ifndef RGB_NOTIFICATIONS_OFF
    digitalWrite(D7, LOW);    // Turn OFF the LED pins
#endif
    delayRealMicros(off*1000);
}

void delayRealMicros(unsigned long us)
{
    delayMicroseconds(us / _clockDivisor + (us % _clockDivisor > 0 ? 1 : 0));
}