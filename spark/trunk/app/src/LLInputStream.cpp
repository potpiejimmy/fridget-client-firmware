/* 
 * File:   LLInputStream.cpp
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#include "LLInputStream.h"

namespace com_myfridget
{
	LLInputStream::~LLInputStream()	{
		close();
	}

	void LLInputStream::close() {
	}
}

