/* 
 * File:   LLFlashUtil.h
 * Author: thorsten
 *
 * Created on October 26, 2014, 1:59 PM
 */

#ifndef _com_myfridget_LLFLASHUTIL_H_
#define	_com_myfridget_LLFLASHUTIL_H_

#include "spark_wiring_eeprom.h"

namespace com_myfridget
{
    class LLFlashUtil
    {
    public:
        static bool flash(const uint8_t *pBuffer, uint32_t address, uint32_t len);
        static void read(uint8_t* pBuffer, uint32_t address, uint32_t len);
    };
}

#endif	/* _com_myfridget_LLFLASHUTIL_H_ */

