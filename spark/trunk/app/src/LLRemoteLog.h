/* 
 * File:   LLRemoteLog.h
 * Author: thorsten
 *
 * Created on October 19, 2014, 4:56 PM
 */

#ifndef _com_myfridget_LLREMOTELOG_H_
#define	_com_myfridget_LLREMOTELOG_H_

#include "spark_wiring_tcpclient.h"
#include "spark_wiring_ipaddress.h"

namespace com_myfridget
{
    class LLRemoteLog
    {
    public:
        LLRemoteLog(IPAddress ipAddress, int port);
        LLRemoteLog(const char* host, int port);
        
        virtual void log(const char* msg);
        
    private:
        const char* host;
        IPAddress ipAddress;
        int port;
        TCPClient client;
    };
}

#endif	/* _com_myfridget_LLREMOTELOG_H_ */

