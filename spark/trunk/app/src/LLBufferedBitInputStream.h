/*
 * File:   LLBufferedBitInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#ifndef _com_myfridget_LLBUFFEREDBITINPUTSTREAM_H
#define	_com_myfridget_LLBUFFEREDBITINPUTSTREAM_H

#include "LLInputStream.h"

namespace com_myfridget
{
    class LLBufferedBitInputStream : public LLInputStream
    {

        public:
            LLBufferedBitInputStream(LLInputStream* in, unsigned char* buf, int len);

            virtual bool readBit();
            
            // BEGIN IMPLEMENTATION OF INPUTSTREAM
            virtual int read(unsigned char* b, int len);
            virtual unsigned char read();
            virtual void close();
            virtual bool eos();
            // END IMPLEMENTATION OF INPUTSTREAM

        private:
            LLInputStream* in;
            unsigned char* buf;
            int len;
            int pos, bitPos, availPos;
            
            bool empty();
            bool fillBuffer();
    };
}

#endif	/* _com_myfridget_LLBUFFEREDBITINPUTSTREAM_H */

