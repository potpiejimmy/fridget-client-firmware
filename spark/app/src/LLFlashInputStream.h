/* 
 * File:   LLFlashInputStream.h
 * Author: thorsten
 *
 * Created on November 8, 2014, 3:02 PM
 */

#ifndef _com_myfridget_LLFLASHINPUTSTREAM_H
#define	_com_myfridget_LLFLASHINPUTSTREAM_H

#include "LLFlashUtil.h"
#include "LLInputStream.h"

namespace com_myfridget
{
    class LLFlashInputStream : public LLInputStream
    {

        public:
            LLFlashInputStream(uint32_t startAddress);

            // BEGIN IMPLEMENTATION OF INPUTSTREAM
            virtual int read(unsigned char* b, int len);
            virtual unsigned char read();
            virtual void close();
            virtual bool eos();
            // END IMPLEMENTATION OF INPUTSTREAM

        private:
            uint32_t address;
    };
}

#endif	/* _com_myfridget_LLFLASHINPUTSTREAM_H */

