/**
  ******************************************************************************
  * @file    application.h
  * @authors  Satish Nair, Zachary Crockett, Zach Supalla and Mohit Bhoite
  * @version V1.0.0
  * @date    30-April-2013
  * @brief   User Application File Header
  ******************************************************************************
  Copyright (c) 2013-2015 Particle Industries, Inc.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
  */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "system_version.h"

#ifdef SPARK_PLATFORM
#include "platform_headers.h"
#endif


#include "spark_wiring.h"
#include "spark_wiring_cloud.h"
#include "spark_wiring_interrupts.h"
#include "spark_wiring_string.h"
#include "spark_wiring_print.h"
#include "spark_wiring_usartserial.h"
#include "spark_wiring_usbserial.h"
#include "spark_wiring_usbmouse.h"
#include "spark_wiring_usbkeyboard.h"
#include "spark_wiring_spi.h"
#include "spark_wiring_i2c.h"
#include "spark_wiring_servo.h"
#include "spark_wiring_wifi.h"
#include "spark_wiring_network.h"
#include "spark_wiring_client.h"
#include "spark_wiring_startup.h"
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_tcpserver.h"
#include "spark_wiring_udp.h"
#include "spark_wiring_time.h"
#include "spark_wiring_tone.h"
#include "spark_wiring_eeprom.h"
#include "spark_wiring_version.h"
#include "spark_wiring_thread.h"
#include "fast_pin.h"
#include "string_convert.h"
#include "debug_output_handler.h"

// this was being implicitly pulled in by some of the other headers
// adding here for backwards compatibility.
#include "system_task.h"
#include "system_user.h"

#include "stdio.h"

using namespace spark;

// SERIAL DEBUGGING - if you enable this, you must connect via 9600 8N1 terminal
// and hit any key so that the core can start up
//#define _SERIAL_DEBUGGING_

// Defines the EPD screen type used
// 0 = SPECTRA_DISPLAY_TYPE_441
// 1 = SPECTRA_DISPLAY_TYPE_74
#define EPD_SCREEN_TYPE 1

// Defines target platform PHOTON
#define PLATFORM_PHOTON

// is power-on and power-off attiny controlled?
// note: this controls whether bit-banging is performed with Attiny and
// prevents the firmware from connecting to the Spark cloud on server connection
// failure
#define ATTINY_CONTROLLED_POWER

// EPD TCON board connected to core?
#define EPD_TCON_CONNECTED

#define _BUF_SIZE 0x1000

/* Read buffer */
extern char _buf[_BUF_SIZE];
/* the current clock divisor */
extern unsigned int _clockDivisor;

void delayRealMicros(unsigned long us);

#endif /* APPLICATION_H_ */
