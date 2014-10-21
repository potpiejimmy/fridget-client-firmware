/* 
 * File:   LLRemoteLog.h
 * Author: thorsten
 *
 * Created on October 19, 2014, 4:56 PM
 */

#ifndef _com_myfridget_LLREMOTELOG_H_
#define    _com_myfridget_LLREMOTELOG_H_

#include "LLWebRequest.h"

namespace com_myfridget
{
    class LLRemoteLog
    {
    public:
        LLRemoteLog(IPAddress ipAddress, int port);
        LLRemoteLog(const char* host, int port);
        
        virtual BOOL log(const char* msg);
        virtual BOOL log(const String msg);
        
    private:
        LLWebRequest webRequester;
    };
}

#endif    /* _com_myfridget_LLREMOTELOG_H_ */

