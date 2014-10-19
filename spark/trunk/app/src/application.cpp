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

using namespace com_myfridget;

LLRemoteLog log(SERVER_HOST, SERVER_PORT);
LLWebRequest requester(SERVER_HOST, SERVER_PORT);

/* Function prototypes -------------------------------------------------------*/
void blinkLED(int on, int off);
void setupSerial();


SYSTEM_MODE(MANUAL);


/* This function is called once at start up ----------------------------------*/
void setup()
{
    pinMode(D7, OUTPUT);
//  setupSerial();    
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    if (WiFi.ready()) {
        
        // wait for IP to be set:
        while (!WiFi.localIP().raw_address()[0]) delay(100);
        
        // we are connected, send a log msg to the server:
        char msg[128];
        uint8_t* ipa = WiFi.localIP().raw_address();
        snprintf(msg, 128, "Awake and connected to %s, IP %d.%d.%d.%d", WiFi.SSID(), (int)ipa[0], (int)ipa[1], (int)ipa[2], (int)ipa[3]);
        log.log(msg); 
        
        // now, just for fun, request a value from the server:
        log.log(">>> Requesting the current date from the server");
        char serverTime[128];
        requester.request("GET", "/fridget/res/debug/?param=servertime", NULL, serverTime, 128);
        snprintf(msg, 128, "<<< Received from server: %s", serverTime);
        log.log(msg);
        
        // turn LED on 5 sec, then deep-sleep
        blinkLED(5000,0);
        WiFi.disconnect();
        Spark.sleep(SLEEP_MODE_DEEP, 20);
    } else if (WiFi.connecting()) {
        // do nothing while connecting
        //blinkLED(128,128);
    } else {
        // if not connecting, connect:
        WiFi.connect();
    }
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