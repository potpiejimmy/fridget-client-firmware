/* 
 * File:   LLRemoteLog.cpp
 * Author: thorsten
 *
 * Created on October 19, 2014, 4:56 PM
 */
#include <stdio.h>

#include "LLRemoteLog.h"

#include "spark_utilities.h"

namespace com_myfridget
{
    LLRemoteLog::LLRemoteLog(const char* host, int port) : webRequester(LLWebRequest(host,port))
    {
    }
    
    LLRemoteLog::LLRemoteLog(IPAddress ipAddress, int port) : webRequester(LLWebRequest(ipAddress,port))
    {
    }
    
    bool LLRemoteLog::log(const char* msg)
    {
        char url[128];
        
        snprintf(url, 128, "/fridget/res/debug/%s", Spark.deviceID().c_str());
        return webRequester.request("POST", url, msg, NULL, 0);
    }
    
    bool LLRemoteLog::log(const String msg)
    {
        return log(msg.c_str());
    }

}