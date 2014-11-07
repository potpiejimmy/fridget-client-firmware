/* 
 * File:   LLInflateInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 4:10 PM
 */

#ifndef _com_myfridget_LLINFLATEINPUTSTREAM_H
#define _com_myfridget_LLINFLATEINPUTSTREAM_H

#include "LLInputStream.h"
#include "LLBufferedBitInputStream.h"

#define com_myfridget_LLINFLATEINPUTSTREAM_MAX_PREFIX_LEN 32

namespace com_myfridget
{
    class LLInflateInputStream : public LLInputStream
    {

        public:
            LLInflateInputStream(LLBufferedBitInputStream* in);
            virtual ~LLInflateInputStream();

            // BEGIN IMPLEMENTATION OF INPUTSTREAM
            virtual int read(unsigned char* b, int len);
            virtual unsigned char read();
            virtual void close();
            virtual bool eos();
            // END IMPLEMENTATION OF INPUTSTREAM

        private:
            LLBufferedBitInputStream* in;
            
            unsigned char* dictionary;
            int dictionarySize;
            unsigned char prefixLengths[com_myfridget_LLINFLATEINPUTSTREAM_MAX_PREFIX_LEN];
            
            void buildDictionary();
            
            unsigned char inflateNextByte();
    };
}

#endif /* _com_myfridget_LLINFLATEINPUTSTREAM_H */

