/* 
 * File:   LLRemoteLog.h
 * Author: thorsten
 *
 * Created on October 19, 2014, 4:56 PM
 */

#ifndef _com_myfridget_LLWEBREQUEST_H_
#define    _com_myfridget_LLWEBREQUEST_H_

#include "spark_wiring_tcpclient.h"
#include "spark_wiring_ipaddress.h"

namespace com_myfridget
{
    class LLWebRequest
    {
    public:
        LLWebRequest(IPAddress ipAddress, int port);
        LLWebRequest(const char* host, int port);
        
        virtual BOOL request(const char* httpMethod, const char* url, const char* contentData, char* readBuffer, size_t readBufferLength);
        
    private:
        const char* host;
        IPAddress ipAddress;
        int port;
        TCPClient client;
    };
}

#endif    /* _com_myfridget_LLWEBREQUEST_H_ */

