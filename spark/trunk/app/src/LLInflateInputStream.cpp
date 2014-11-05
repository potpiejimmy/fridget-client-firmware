/* 
 * File:   LLInflateInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 4:10 PM
 */

#include "LLInflateInputStream.h"

#include <cstdio>
#include "LLBufferedByteInputStream.h"
#include "LLInputStream.h"

namespace com_myfridget
{

    LLInflateInputStream::LLInflateInputStream(LLBufferedByteInputStream* in) 
    : in(in), dictionary(0) {
    }
    
    LLInflateInputStream::~LLInflateInputStream() {
        freeDictionary();
    }

    int LLInflateInputStream::read(unsigned char* b, int len) {
        if (!dictionary) buildDictionary();
        return 0;
    }
    
    void LLInflateInputStream::buildDictionary() {
        printf("BUILD DIC\n");
        // first byte in stream must be dictionary size:
        unsigned char size = in->readByte();
        printf("DIC SIZE %d\n", size);
        dictionaryNode* queue[size];
        for (int i=0; i<size; i++) {
            dictionaryNode* node = new dictionaryNode;
            node->left = 0;
            node->right = 0;
            node->symbol = in->readByte(); // symbol
            node->value = in->readByte(); // prefix length
            queue[i] = node;
        }
        
        while (queueSize(queue, size)>1) {
            dictionaryNode* left = fetchFirst(queue, size);
            dictionaryNode* right = fetchFirst(queue, size);
            
            int depthLeft = (left->left) ? left->symbol : 0;
            int depthRight = (right->left) ? right->symbol : 0;
            
            dictionaryNode* node = new dictionaryNode;
            node->left = depthLeft > depthRight ? right : left;
            node->right = depthLeft > depthRight ? left : right;
            node->value = left->value - 1; // new prefix length
            node->symbol = depthLeft > depthRight ? depthLeft + 1 : depthRight + 1;
            enqueue(queue, size, node);
        }
        
        dictionary = fetchFirst(queue, size);
        char pbuf[128];
        printDictionary(dictionary, pbuf, 0);
    }

    void LLInflateInputStream::printDictionary(LLInflateInputStream::dictionaryNode* node, char* pbuf, int pbufc) {
        if (node->left) {
            pbuf[pbufc] = '0';
            printDictionary(node->left, pbuf, pbufc + 1);
            
            pbuf[pbufc] = '1';
            printDictionary(node->right, pbuf, pbufc + 1);
            
            pbuf[pbufc] = 0;
        } else {
            // a leaf, print
            pbuf[pbufc] = 0;
            printf("%u\t%s\n", node->symbol, pbuf);
        }
    }
    
    LLInflateInputStream::dictionaryNode* LLInflateInputStream::fetchFirst(LLInflateInputStream::dictionaryNode** queue, int capacity) {
        int found = 0;
        dictionaryNode* first = 0;
        for (int i=0; i<capacity; i++) {
            if (queue[i]) {
                if (!first || queue[i]->value > first->value ||
                     (queue[i]->value == first->value && ((!queue[i]->left && first->left) || queue[i]->symbol < first->symbol))) {
                    found = i;
                    first = queue[i];
                }
            }
        }
        // unqueue:
        queue[found] = 0;
        printf("Unqueue %d\n", found);
        return first;
    }

    void LLInflateInputStream::enqueue(LLInflateInputStream::dictionaryNode** queue, int capacity, LLInflateInputStream::dictionaryNode* node) {
        for (int i=0; i<capacity; i++) {
            if (!queue[i]) {queue[i] = node; return;}
        }
    }
    
    int LLInflateInputStream::queueSize(LLInflateInputStream::dictionaryNode** queue, int capacity) {
        int size = 0;
        for (int i=0; i<capacity; i++) {
            if (queue[i]) size++;
        }
        printf("QUEUE SIZE %d\n", size);
        return size;
    }
            
    void LLInflateInputStream::freeDictionary() {
        // XXX TODO
    }
    
    void LLInflateInputStream::close() {
        
    }
    
    bool LLInflateInputStream::eos() {
        return in->eos();
    }
}
