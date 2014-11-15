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

// is power-on and power-off attiny controlled?
#define ATTINY_CONTROLLED_POWER
// EPD TCON board connected to core?
#define EPD_TCON_CONNECTED

using namespace com_myfridget;

/* Allocate read buffer (Note: declared in application.h) */
char _buf[_BUF_SIZE];

/* Set up web requester */
LLWebRequest requester(SERVER_HOST, SERVER_PORT);
/* The current user state */
int userState;
/* the current clock divisor */
unsigned int clockDivisor;
/* server parameter read buffer */
char serverParamsBuf[256];
/* server parameter map */
LLMap serverParams(16);

/* Function prototypes -------------------------------------------------------*/
void blinkLED(int on, int off);
void debug(const char* msg);
void debug(const String msg);
void onOnline();
void executeOp();
void enterPowerSaveMode();
void updateDisplay();
void powerDown(uint8_t interval);
void flashTestImage();

/* ############################################################# */

/* MANUAL: not connecting to Spark cloud, running user-code loop immediately
 * on power-up. */
SYSTEM_MODE(MANUAL);

/* This function is called once at start up ----------------------------------*/
void setup()
{
    clockDivisor = 1;
    
    /* start offline */
    userState = USER_STATE_OFFLINE;

    /* Activate the LED output PIN */
    pinMode(D7, OUTPUT);
    
    /* Active D4 for Attiny notification output */
    pinMode(D0, OUTPUT); // Time Interval Bit 0
    pinMode(D1, OUTPUT); // Time Interval Bit 1
    pinMode(D2, OUTPUT); // Time Interval Bit 2
    pinMode(D4, OUTPUT); // Notification BUSY output
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
        debug("Core up.");
        execLen = EEPROM.read(0);
        execNo = EEPROM.read(1);
        debug(String("ExecLen=")+execLen+", ExecNo="+execNo);
        if (execNo >= execLen) {
            // connect to WiFi:
            WiFi.connect();
            userState = USER_STATE_CONNECTING;
            debug("State: USER_STATE_CONNECTING");
        } else {
            // execute next program step:
            executeOp();
        }
        break;

    case USER_STATE_CONNECTING:
        // do nothing while connecting, wait for ready
        //blinkLED(128,128);
        if (WiFi.ready()) {
            userState = USER_STATE_CONNECTED_AWAITING_IP;
            debug("State: USER_STATE_CONNECTED_AWAITING_IP");
        }
        break;
    
    case USER_STATE_CONNECTED_AWAITING_IP:
        // wait until IP address is set
        if (WiFi.localIP().raw_address()[0]) {
            userState = USER_STATE_ONLINE;
            debug("State: USER_STATE_ONLINE");
            onOnline();
        }
        break;
        
    case USER_STATE_ONLINE:
        // flash the image data
        flashTestImage();
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
            debug(String("<<< Received server parameters: ") + serverParamsBuf);
            serverParams.parse(serverParamsBuf);
            return TRUE;
        } else {
            debug(String("Could not connect to ") + SERVER_HOST_DEBUGNAME + "(" + numberOfRetries + ")");
        }
        delayRealMicros(100000);
    }
    return FALSE;
}

void onOnline()
{
    int connectMode = USER_CONNECT_MODE_CLOUD_ON;
    
    if (establishServerConnection()) {
        debug(String("Connected to ") + SERVER_HOST_DEBUGNAME);
        connectMode = atoi(getServerParam("connectmode", "1"/*USER_CONNECT_MODE_CLOUD_ON*/));
    } else {
        debug("Server not available, connecting to cloud.");
    }
    
    if (connectMode == USER_CONNECT_MODE_CLOUD_ON)
    {
        // Connecting cloud:
        Spark.connect();
        userState = USER_STATE_ONLINE_WITH_CLOUD;
        debug("State: USER_STATE_ONLINE_WITH_CLOUD");
        return;
    }
    
    const char* program = getServerParam("exec", "N0");
    uint8_t programSize = strlen(program);
    debug(String("Received program: ") + program);
 
    EEPROM.write(0, (uint8_t)programSize);
    EEPROM.write(1, (uint8_t)0); // program counter
    EEPROM.write(2, (uint8_t)0); // reset image no.
    for (int i=0; i<programSize; i++) EEPROM.write(3+i, program[i]);
}

void executeOp()
{
    enterPowerSaveMode();
    
    uint8_t execNo = EEPROM.read(1); // program counter
    char opName = EEPROM.read(3+execNo);
    debug(String("Execute OP ") + opName);
    
    // for now, only 'D' (display update) or 'N' (NOOP) allowed
    if (opName == 'D') updateDisplay();
    
    execNo++;
    
    // OP done, now powering down:
    uint8_t interval = EEPROM.read(3+execNo) - '0';
    debug(String("Execute Sleep interval ") + interval);
    execNo++;
    
    debug(String("Increasing execNo to ") + execNo);
    EEPROM.write(1, execNo);
    
    powerDown(interval);
}

void enterPowerSaveMode()
{
    debug("Disabling WiFi and LED");
    // WiFi und RGB aus
    WiFi.off();
    RGB.control(true);
    RGB.color(0);
    
    debug("Set IWDG maximum timeout (about 26 sec.)");
    IWDG_Reset_Enable(0xFFFF); // set IWDG timeout to a maximum value
    
    // runtertakten:
#ifdef EPD_TCON_CONNECTED
    RCC_HCLKConfig(RCC_SYSCLK_Div8);
    clockDivisor=8;
#endif
}

void updateDisplay()
{
    const int decodeBufSize = 1024;
    unsigned char decodeBuf[decodeBufSize];
    
    uint8_t imageNo = EEPROM.read(2);
    
    // write the image from flash to the display:
    debug(String("Updating display with image no. ") + imageNo);
    
    // Now link from FLASH to DECODEBUF to HUFFMAN-INFLATE to RLE-INFLATE
    LLFlashInputStream flashIn(imageNo * SIZE_EPD_SEGMENT); // XXX
    LLBufferedBitInputStream bufIn(&flashIn, decodeBuf, decodeBufSize);
    LLInflateInputStream inflateIn(&bufIn);
    LLRLEInputStream rleIn(&inflateIn);
    
#ifdef EPD_TCON_CONNECTED
    ShowImage(&rleIn);
#endif
    
    EEPROM.write(2, ++imageNo);
}

void powerDown(uint8_t interval)
{
    // deep-sleep for sleepTime seconds
    debug(String("Going to sleep with sleep interval #") + interval);
    
#ifdef ATTINY_CONTROLLED_POWER
    userState = USER_STATE_IDLE;
    debug("State: USER_STATE_IDLE");
    
    digitalWrite(D0, (interval&1));
    digitalWrite(D1, (interval&2)>>1);
    digitalWrite(D2, (interval&4)>>2);
    digitalWrite(D4, LOW);    // Notify Attiny
    
    delay(100);
    
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
#else
#ifdef _SERIAL_DEBUGGING_
    delay(200);
#endif
    Spark.sleep(SLEEP_MODE_DEEP, 1+interval);
#endif
}

void flashTestImage()
{
    char url[128];
    
    bool flash = atoi(getServerParam("flashimage", "0"));
    
    int index = 0;
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
                int readSoFar = 0;
                char imgLenBuf[2]; // two bytes of length
                requester.readAll(imgLenBuf, 2);
                int imgLen = ((int)imgLenBuf[0])<<8|imgLenBuf[1];
                while (readSoFar < imgLen)
                {
                    int shouldRead = imgLen - readSoFar;
                    if (shouldRead > _BUF_SIZE) shouldRead = _BUF_SIZE;
                    debug(String("Reading image data ") + index + " [" + readSoFar + "-" + (readSoFar + shouldRead - 1) + "]");
                    int readNow = requester.readAll(_buf, shouldRead);

                    if (readNow != shouldRead)
                    {
                        debug(String("Failed, received only ") + readNow);
                        done = TRUE;
                        break;
                    }
                    if (flash) {
                        // okay, burn it
                        debug("Writing to flash.");
                        if (!LLFlashUtil::flash((const uint8_t*)_buf, (index * SIZE_EPD_SEGMENT) + readSoFar, readNow)) {
                            badBlocks++;
                        }
                        debug("Done.");
                    }
                    readSoFar += readNow;
                }
                overallSize += readSoFar;
                index++;
            } while (!done);
        }
        requester.stop();
        debug(String("Wrote ") + overallSize + " bytes to flash.");
        if (badBlocks > 0) {
            debug(String("XXX BAD FLASH BLOCKS: ") + badBlocks);
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

void debug(const char* msg)
{
#ifdef _SERIAL_DEBUGGING_
    Serial.println(msg);
#endif
}

void debug(const String msg)
{
#ifdef _SERIAL_DEBUGGING_
    Serial.println(msg);
#endif
}

void delayRealMicros(unsigned long us)
{
    delayMicroseconds(us / clockDivisor + (us % clockDivisor > 0 ? 1 : 0));
}