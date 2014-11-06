/* 
 * File:   LLInflateInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 4:10 PM
 */

#ifndef _com_myfridget_LLINFLATEINPUTSTREAM_H
#define	_com_myfridget_LLINFLATEINPUTSTREAM_H

#include "LLInputStream.h"
#include "LLBufferedByteInputStream.h"

namespace com_myfridget
{
    class LLInflateInputStream : public LLInputStream
    {

        public:
            LLInflateInputStream(LLBufferedByteInputStream* in);
            virtual ~LLInflateInputStream();

            // BEGIN IMPLEMENTATION OF INPUTSTREAM
            virtual int read(unsigned char* b, int len);
            virtual void close();
            virtual bool eos();
            // END IMPLEMENTATION OF INPUTSTREAM

        private:
            LLBufferedByteInputStream* in;
            
            typedef struct DictionaryEntry {
                unsigned int code;
                unsigned char symbol;
            } DictionaryEntry;
            
            DictionaryEntry* dictionary;
            int dictionarySize;
            
            void buildDictionary();
            void printDictionary();
    };
}

#endif	/* _com_myfridget_LLINFLATEINPUTSTREAM_H */

