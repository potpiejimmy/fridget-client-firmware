/* 
 * File:   LLInflateInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 4:10 PM
 */

#ifndef _com_myfridget_LLMEMORYINPUTSTREAM_H
#define _com_myfridget_LLMEMORYINPUTSTREAM_H

#include "LLInputStream.h"

namespace com_myfridget
{
    class LLMemoryInputStream : public LLInputStream
    {

        public:
            LLMemoryInputStream(unsigned char* memptr);
            virtual ~LLMemoryInputStream();

            // BEGIN IMPLEMENTATION OF INPUTSTREAM
            virtual int read(unsigned char* b, int len);
            virtual unsigned char read();
            virtual void close();
            virtual bool eos();
            // END IMPLEMENTATION OF INPUTSTREAM

        private:
            unsigned char* memptr;
    };
}

#endif /* _com_myfridget_LLMEMORYINPUTSTREAM_H */

