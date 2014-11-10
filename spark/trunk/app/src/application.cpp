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

#include "LLRemoteLog.h"
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
// Disable all logging and parameter requests
#define FRIDGET_SPEED_OPTIMIZED

using namespace com_myfridget;

/* Allocate read buffer (Note: declared in application.h) */
char _buf[_BUF_SIZE];

/* Set up remote logging */
LLRemoteLog log(SERVER_HOST, SERVER_PORT);
/* Set up web requester */
LLWebRequest requester(SERVER_HOST, SERVER_PORT);
/* The current user state */
int userState;
/* the current clock divisor */
unsigned int clockDivisor;

/* Function prototypes -------------------------------------------------------*/
void blinkLED(int on, int off);
void debug(const char* msg);
void debug(const String msg);
void onOnline();
void updateDisplay();
void updateDisplayAndSleep();
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
    pinMode(D4, OUTPUT);
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
    uint8_t cycleLen;
    uint8_t cycleNo;
    
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
        cycleLen = EEPROM.read(0);
        cycleNo = EEPROM.read(1);
        debug(String("CycleLen=")+cycleLen+", CycleNo="+cycleNo);
        if (cycleNo >= cycleLen) {
            // connect to WiFi:
            WiFi.connect();
            userState = USER_STATE_CONNECTING;
            debug("State: USER_STATE_CONNECTING");
        } else {
            // just update display and sleep:
            updateDisplayAndSleep();
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
        // flash the test image
        flashTestImage();
        // perform display update and sleep
        updateDisplayAndSleep();
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

int getServerParam(const char* param, int def)
{
#ifndef FRIDGET_SPEED_OPTIMIZED
    char readBuf[16];
    char url[128];
    
    log.log(String(">>> Requesting parameter ") + param);
    debug(String(">>> Requesting parameter ") + param);
    
    snprintf(url, 128, "/fridget/res/debug/%s/?param=%s", Spark.deviceID().c_str(), param);
    if (requester.request("GET", url, NULL, readBuf, 16))
    {
        log.log(String("<<< Received from server: ") + readBuf);
        int result = atoi(readBuf);
        if (result != 0) return result;
    }
#endif
    // failed, return default:
    return def;
}

bool establishServerConnection()
{
    int numberOfRetries = 0;
    uint8_t* ipa = WiFi.localIP().raw_address();
    
    /* 
     * When using Domain Names instead of IP addresses for TCPClient,
     * the first connection may fail due to a delayed name resolution.
     * So we try to send the initial message to the server twice.
     */

    while (numberOfRetries++ < 3) {
        // say hello to the server, log IP and SSID
        if (log.log(String("*** Connected to ") + WiFi.SSID() + ", IP " + ipa[0] + "." + ipa[1] + "." + ipa[2] + "." + ipa[3] + " ***")) {
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
        connectMode = getServerParam("connectmode", USER_CONNECT_MODE_CLOUD_OFF);
    } else {
        debug("Server not available, connecting to cloud.");
    }
    
    if (connectMode == USER_CONNECT_MODE_CLOUD_ON)
    {
        // Connecting cloud:
        log.log("Connecting to cloud...");
        Spark.connect();
        log.log("Connected to cloud.");
        userState = USER_STATE_ONLINE_WITH_CLOUD;
        debug("State: USER_STATE_ONLINE_WITH_CLOUD");
        return;
    }
    
    int sleepTime = getServerParam("sleeptime", 1);
    int connectCycle = getServerParam("connectcycle", 24);
    
    EEPROM.write(0, (uint8_t)connectCycle);
    EEPROM.write(1, (uint8_t)0);
    EEPROM.write(2, (uint8_t)((sleepTime&0xff00)>>8));
    EEPROM.write(3, (uint8_t)((sleepTime&0xff)));
}

void updateDisplay()
{
    const int decodeBufSize = 1024;
    unsigned char decodeBuf[decodeBufSize];
    
    // Now link from FLASH to DECODEBUF to HUFFMAN-INFLATE to RLE-INFLATE
    LLFlashInputStream flashIn(EEPROM.read(1) * SIZE_EPD_SEGMENT); // XXX
    LLBufferedBitInputStream bufIn(&flashIn, decodeBuf, decodeBufSize);
    LLInflateInputStream inflateIn(&bufIn);
    LLRLEInputStream rleIn(&inflateIn);
    
#ifdef EPD_TCON_CONNECTED
    ShowImage(&rleIn);
#endif
}

void updateDisplayAndSleep()
{
    debug("Disabling WiFi and LED");
    // WiFi und RGB aus
    WiFi.off();
    RGB.control(true);
    RGB.color(0);
    
    // runtertakten:
#ifdef EPD_TCON_CONNECTED
    RCC_HCLKConfig(RCC_SYSCLK_Div8);
    clockDivisor=8;
#endif
    
    uint8_t cycle = EEPROM.read(1) + 1;
    int sleepTime = ((int)EEPROM.read(2))<<8 | EEPROM.read(3);
 
    // write the image from flash to the display:
    debug("Updating display");
    updateDisplay();
    
    // deep-sleep for sleepTime seconds
    debug(String("Going to sleep for ") + sleepTime + " sec.");
    
    debug(String("Increased cycle no. to ") + cycle);
    EEPROM.write(1, cycle);
    
#ifdef ATTINY_CONTROLLED_POWER
    digitalWrite(D4, LOW);    // Notify Attiny
    userState = USER_STATE_IDLE;
    debug("State: USER_STATE_IDLE");
    
    /* Request to enter STOP mode with regulator in low power mode */
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);
#else
    Spark.sleep(SLEEP_MODE_DEEP, sleepTime);
#endif
}

void flashTestImage()
{
    char url[128];
    
    if (getServerParam("flashimage", 1))
    {
#ifndef FRIDGET_SPEED_OPTIMIZED
        log.log("Requesting image data and writing to flash...");
#endif
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
                        // okay, burn it
                        debug("Writing to flash.");
                        if (!LLFlashUtil::flash((const uint8_t*)_buf, (index * SIZE_EPD_SEGMENT) + readSoFar, readNow)) {
                            badBlocks++;
                        }
                        debug("Done.");
                        readSoFar += readNow;
                    }
                    overallSize += readSoFar;
                    index++;
                } while (!done);
            }
            requester.stop();
#ifndef FRIDGET_SPEED_OPTIMIZED
            log.log(String("Wrote ") + overallSize + " bytes to flash.");
            if (badBlocks > 0) {
                log.log(String("XXX BAD FLASH BLOCKS: ") + badBlocks);
            }
#endif
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