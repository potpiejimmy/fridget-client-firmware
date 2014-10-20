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

#define SERVER_HOST IPAddress(192,168,178,32)
#define SERVER_PORT 8080
//#define SERVER_HOST "www.doogetha.com"
//#define SERVER_PORT 80

#define MSG_BUF_SIZE 128

using namespace com_myfridget;

/* Set up remote logging */
LLRemoteLog log(SERVER_HOST, SERVER_PORT);
/* Set up web requester */
LLWebRequest requester(SERVER_HOST, SERVER_PORT);
/* message buffer */
char _msgBuf[MSG_BUF_SIZE];

/* Function prototypes -------------------------------------------------------*/
void blinkLED(int on, int off);
void setupSerial();
void performOnlineTask();

/* MANUAL: not connecting to Spark cloud, running user-code loop immediately
 * on power-up. */
SYSTEM_MODE(MANUAL);


/* This function is called once at start up ----------------------------------*/
void setup()
{
    /* Activate the LED output PIN */
    pinMode(D7, OUTPUT);
    /* For serial debugging only: */
//  setupSerial();    
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    if (WiFi.ready()) {

        // WiFi connected:
        log.log("WiFi ready.");
        
        // As soon as IP is set, perform online task:
        if (WiFi.localIP().raw_address()[0])
        {
            performOnlineTask();
        }
        
    } else if (WiFi.connecting()) {
        // do nothing while connecting
        //blinkLED(128,128);
    } else {
        // if not connecting, connect:
        WiFi.connect();
    }
}

void logOnline()
{
    // we are connected, send a log msg to the server:
    uint8_t* ipa = WiFi.localIP().raw_address();
    snprintf(_msgBuf, MSG_BUF_SIZE, "Awake and connected to %s, IP %d.%d.%d.%d", WiFi.SSID(), (int)ipa[0], (int)ipa[1], (int)ipa[2], (int)ipa[3]);
    log.log(_msgBuf); 
}

int getServerParam(const char* param)
{
    char readBuf[16];
    
    snprintf(_msgBuf, MSG_BUF_SIZE, ">>> Requesting parameter '%s'", param);
    log.log(_msgBuf);
    
    snprintf(_msgBuf, MSG_BUF_SIZE, "/fridget/res/debug/?serial=%s&param=%s", Spark.deviceID().c_str(), param);
    requester.request("GET", _msgBuf, NULL, readBuf, 16);
    
    snprintf(_msgBuf, MSG_BUF_SIZE, "<<< Received from server: %s", readBuf);
    log.log(_msgBuf);
    
    return atoi(readBuf);
}

void performOnlineTask()
{
    // say hello to the server, log IP and SSID
    logOnline();
    
    int wakeTime = getServerParam("waketime");
    int sleepTime = getServerParam("sleeptime");

    // turn LED on for wakeTime seconds, then deep-sleep for sleepTime seconds
    blinkLED(wakeTime * 1000, 0);
    log.log("Going to sleep.");
    
    WiFi.disconnect();
    Spark.sleep(SLEEP_MODE_DEEP, sleepTime);
}

void setupSerial()
{
// -----
  // Make sure your Serial Terminal app is closed before powering your Core
  Serial.begin(9600);
  // Now open your Serial Terminal, and hit any key to continue!
  while(!Serial.available()) SPARK_WLAN_Loop();

  Serial.println(WiFi.localIP());
  Serial.println(WiFi.subnetMask());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.SSID());
// ------    
}

void blinkLED(int on, int off)
{
    digitalWrite(D7, HIGH);   // Turn ON the LED pins
    delay(on);
    digitalWrite(D7, LOW);    // Turn OFF the LED pins
    delay(off);
}