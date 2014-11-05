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
            
            struct dictionaryNode {
                char value;
                unsigned char symbol;
                struct dictionaryNode* left;
                struct dictionaryNode* right;
            };
            typedef struct dictionaryNode dictionaryNode;
            
            dictionaryNode* dictionary;
            
            void buildDictionary();
            void printDictionary(dictionaryNode* node, char* pbuf, int pbufc);
            void freeDictionary();
            dictionaryNode* fetchFirst(dictionaryNode** queue, int capacity);
            void enqueue(dictionaryNode** queue, int capacity, dictionaryNode* node);
            int queueSize(dictionaryNode** queue, int capacity);
    };
}

#endif	/* _com_myfridget_LLINFLATEINPUTSTREAM_H */

