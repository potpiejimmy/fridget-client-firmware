/* 
 * File:   LLRemoteLog.cpp
 * Author: thorsten
 *
 * Created on October 19, 2014, 4:56 PM
 */
#include <stdio.h>

#include "LLWebRequest.h"

#include "application.h"

namespace com_myfridget
{
    LLWebRequest::LLWebRequest(const char* host, int port) : host(host), port(port)
    {
    }
    
    LLWebRequest::LLWebRequest(IPAddress ipAddress, int port) : host(NULL), ipAddress(ipAddress), port(port)
    {
    }
    
    bool LLWebRequest::request(const char* httpMethod, const char* url, const char* contentData, char* readBuffer, size_t readBufferLength)
    {
        bool result = FALSE;
        if (readBuffer) readBuffer[0] = 0; // terminate result buffer in case of failure
        
        if (sendRequest(httpMethod, url, contentData))
        {
            if (readBuffer)
            {
                // read HTTP headers:
                if (readHeaders())
                {
                    // read content data as c-string, keep 1 byte for termination:
                    int bytesRead = readAll(readBuffer, readBufferLength - 1);
                    
                    readBuffer[bytesRead] = 0; // Terminate read buffer with \0
                    result = TRUE;
                }
            }
            else
            {
                // okay, not expecting any result
                result = TRUE;
            }
            stop();
        }
        return result;
    }

    bool LLWebRequest::sendRequest(const char* httpMethod, const char* url, const char* contentData) {
        
        if (host ? client.connect(host, port) : client.connect(ipAddress, port))
        {
            int contentLength = contentData ? strlen(contentData) + 2 : 0;
            char req[128];
            snprintf(req, 128, "%s %s HTTP/1.0", httpMethod, url);
            client.println(req);
            client.println("Host: www.doogetha.com");
            snprintf(req, 128, "Content-Length: %d", contentLength);
            client.println(req);
            client.println();
            if (contentData)
            {
                snprintf(req, 128, "'%s'", contentData);
                client.write((uint8_t*)req, contentLength);
            }
            return TRUE;
        }
        return FALSE;
    }
    
    bool LLWebRequest::readHeaders() {
        // read HTTP headers:
        const char* HEADER_END = "\r\n\r\n";
        int detected = 0;
        while (client.connected() && detected < strlen(HEADER_END)) {
            int c = client.read();
            if (c == HEADER_END[detected]) detected++;
            else detected = 0;
        }
        return (detected == strlen(HEADER_END));
    }
    
    int LLWebRequest::readAll(char* readBuffer, size_t readBufferLength) {
        int bytesRead = 0;
        while (client.connected() && bytesRead < readBufferLength) {
            int readNow = client.readBytes(readBuffer + bytesRead, readBufferLength - bytesRead);
            if (readNow <= 0) break;
            bytesRead += readNow;
        }
        return bytesRead;
    }
    
    void LLWebRequest::stop() {
        // read the rest (if any) until server disconnects
        while (client.available()) client.read();
        client.stop();
    }
}