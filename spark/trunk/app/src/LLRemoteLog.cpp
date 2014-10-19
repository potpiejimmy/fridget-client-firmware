/* 
 * File:   LLRemoteLog.cpp
 * Author: thorsten
 *
 * Created on October 19, 2014, 4:56 PM
 */
#include <stdio.h>

#include "LLRemoteLog.h"

namespace com_myfridget
{
    LLRemoteLog::LLRemoteLog(const char* host, int port) : host(host), port(port)
    {
    }
    
    LLRemoteLog::LLRemoteLog(IPAddress ipAddress, int port) : host(NULL), ipAddress(ipAddress), port(port)
    {
    }
    
    void LLRemoteLog::log(const char* msg)
    {
        if (host ? client.connect(host, port) : client.connect(ipAddress, port))
        {
            int contentLength = strlen(msg) + 2;
            char req[128];
            snprintf(req, 128, "POST /fridget/res/debug/?serial=1234 HTTP/1.0");
            client.println(req);
            client.println("Host: www.doogetha.com");
            snprintf(req, 128, "Content-Length: %d", contentLength);
            client.println(req);
            client.println();
            snprintf(req, 128, "'%s'", msg);
            client.write((uint8_t*)req, contentLength);
            while (client.connected()) client.read();
            client.stop();
        }
        else
        {
            for (int i=0; i<5; i++)
            {
                digitalWrite(D7, HIGH);   // Turn ON the LED pins
                delay(1000);
                digitalWrite(D7, LOW);    // Turn OFF the LED pins
                delay(1000);
            }
        }
    }

}