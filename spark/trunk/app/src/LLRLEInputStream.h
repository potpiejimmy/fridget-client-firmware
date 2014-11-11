/*
 * File:   LLRLEInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#ifndef _com_myfridget_LLRLEINPUTSTREAM_H
#define	_com_myfridget_LLRLEINPUTSTREAM_H

#include "LLInputStream.h"

namespace com_myfridget
{
    class LLRLEInputStream : public LLInputStream
    {

        public:
            LLRLEInputStream(LLInputStream* in);

            // BEGIN IMPLEMENTATION OF INPUTSTREAM
            virtual int read(unsigned char* b, int len);
            virtual unsigned char read();
            virtual void close();
            virtual bool eos();
            // END IMPLEMENTATION OF INPUTSTREAM

        private:
            LLInputStream* in;
            int compression;
            unsigned char currentBit;
            unsigned int currentLen;
            unsigned char nibble;
            bool haveNibble;
            int readBitPos;
            void decodeNext();
            unsigned char readNibble();
    };
}

#endif	/* _com_myfridget_LLRLEINPUTSTREAM_H */

