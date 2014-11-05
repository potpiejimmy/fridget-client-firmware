/*
 * File:   LLBufferedByteInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#ifndef _com_myfridget_LLBUFFEREDBYTEINPUTSTREAM_H
#define	_com_myfridget_LLBUFFEREDBYTEINPUTSTREAM_H

#include "LLInputStream.h"

namespace com_myfridget
{
    class LLBufferedByteInputStream : public LLInputStream
    {

        public:
            LLBufferedByteInputStream(LLInputStream* in, unsigned char* buf, int len);

            virtual unsigned char readByte();
            virtual bool readBit();
            
            // BEGIN IMPLEMENTATION OF INPUTSTREAM
            virtual int read(unsigned char* b, int len);
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

#endif	/* _com_myfridget_LLBUFFEREDBYTEINPUTSTREAM_H */

