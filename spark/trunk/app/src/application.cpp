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

// user states
#define USER_STATE_OFFLINE                0
#define USER_STATE_CONNECTING             1
#define USER_STATE_CONNECTED_AWAITING_IP  2
#define USER_STATE_ONLINE                 3
#define USER_STATE_ONLINE_WITH_CLOUD      4

// connect modes (cloud on/off)
#define USER_CONNECT_MODE_CLOUD_ON   1
#define USER_CONNECT_MODE_CLOUD_OFF  2

//#define SERVER_HOST IPAddress(192,168,178,32)
//#define SERVER_HOST_DEBUGNAME "192.168.178.32"
//#define SERVER_PORT 8080
#define SERVER_HOST "www.doogetha.com"
#define SERVER_HOST_DEBUGNAME "www.doogetha.com"
#define SERVER_PORT 80

// The size of an EPD image
#define SIZE_EPD_IMAGE     30000
// The size of flash memory to reserve for one EPD image, must be a multiple of 4KB
#define SIZE_EPD_SEGMENT  0x8000

using namespace com_myfridget;

/* Allocate read buffer (Note: declared in application.h) */
char _buf[_BUF_SIZE];

/* Set up remote logging */
LLRemoteLog log(SERVER_HOST, SERVER_PORT);
/* Set up web requester */
LLWebRequest requester(SERVER_HOST, SERVER_PORT);
/* The current user state */
int userState;

/* Function prototypes -------------------------------------------------------*/
void blinkLED(int on, int off);
void debug(const char* msg);
void debug(const String msg);
void onOnline();
void updateDisplayAndSleep();
void flashTestImage();

/* MANUAL: not connecting to Spark cloud, running user-code loop immediately
 * on power-up. */
SYSTEM_MODE(MANUAL);


/* This function is called once at start up ----------------------------------*/
void setup()
{
    /* start offline */
    userState = USER_STATE_OFFLINE;

    /* Activate the LED output PIN */
    pinMode(D7, OUTPUT);

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
            // just wake and sleep:
            WiFi.off();
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
    }
}

int getServerParam(const char* param, int def)
{
    char readBuf[16];
    char url[128];
    
    log.log(String(">>> Requesting parameter ") + param);
    
    snprintf(url, 128, "/fridget/res/debug/%s/?param=%s", Spark.deviceID().c_str(), param);
    if (requester.request("GET", url, NULL, readBuf, 16))
    {
        log.log(String("<<< Received from server: ") + readBuf);
        int result = atoi(readBuf);
        if (result != 0) return result;
    }
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
        delay(100);
    }
    return FALSE;
}

void onOnline()
{
    int connectMode = USER_CONNECT_MODE_CLOUD_ON;
    
    if (establishServerConnection()) {
        debug(String("Connected to ") + SERVER_HOST_DEBUGNAME);
        connectMode = getServerParam("connectmode", USER_CONNECT_MODE_CLOUD_ON);
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
    
    int sleepTime = getServerParam("sleeptime", 30);
    int connectCycle = getServerParam("connectcycle", 1);
    
    EEPROM.write(0, (uint8_t)connectCycle);
    EEPROM.write(1, (uint8_t)0);
    EEPROM.write(2, (uint8_t)((sleepTime&0xff00)>>8));
    EEPROM.write(3, (uint8_t)((sleepTime&0xff)));
}

void updateDisplay()
{
    ShowImage(EEPROM.read(1) * SIZE_EPD_SEGMENT); // XXX
}

void updateDisplayAndSleep()
{
    uint8_t cycle = EEPROM.read(1) + 1;
    int sleepTime = ((int)EEPROM.read(2))<<8 | EEPROM.read(3);
 
    // write the image from flash to the display:
    updateDisplay();
    
    // deep-sleep for sleepTime seconds
    String msg = String("Going to sleep for ") + sleepTime + " sec.";
    debug(msg); log.log(msg);
    
    debug(String("Increased cycle no. to ") + cycle);
    EEPROM.write(1, cycle);
    
    WiFi.disconnect();
    Spark.sleep(SLEEP_MODE_DEEP, sleepTime);
}

void flashTestImage()
{
    char url[128];
    
    if (getServerParam("flashimage", 0))
    {
        log.log("Requesting image data and writing to flash...");
        int index = 0;
        bool done = FALSE;
        do
        {
            snprintf(url, 128, "/fridget/res/img/%s/?index=%d", Spark.deviceID().c_str(), index);
            if (requester.sendRequest("GET", url, NULL))
            {
                int readSoFar = 0;
				int badBlocks = 0;
                if (requester.readHeaders())
                {
                    while (readSoFar < SIZE_EPD_IMAGE)
                    {
                        int shouldRead = SIZE_EPD_IMAGE - readSoFar;
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
							backBlocks++;
						}
                        debug("Done.");
                        readSoFar += readNow;
                    }
                }
                else done = TRUE;
                
                requester.stop();
                log.log(String("Wrote ") + readSoFar + " bytes to flash.");
				if (backBlocks > 0) {
					log.log(String("XXX BAD FLASH BLOCKS: ") + backBlocks);
				}
            }
            else done = TRUE;
            
            index++;
        } while (!done);
    }
}

void blinkLED(int on, int off)
{
#ifndef RGB_NOTIFICATIONS_OFF
    digitalWrite(D7, HIGH);   // Turn ON the LED pins
#endif
    delay(on);
#ifndef RGB_NOTIFICATIONS_OFF
    digitalWrite(D7, LOW);    // Turn OFF the LED pins
#endif
    delay(off);
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
