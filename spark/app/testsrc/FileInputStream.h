/***************************************************************************
 *   Copyright (C) 2005 by dREI C systems GbR                              *
 *   Author: thorsten.liese@dreicsystems.de                                *
 ***************************************************************************/


#ifndef _com_dreic_utils_FileInputStream_H_
#define _com_dreic_utils_FileInputStream_H_

#include "LLInputStream.h"

#include <cstdio>
#include <string>

namespace com_myfridget
{
	/**
	 * This class provides an input stream for reading from memory.
	 */
	class FileInputStream : public LLInputStream
	{
	
		public:
		
			/**
			 * Construct new file input stream using the
			 * given file name
			 * @param filename a file name
			 */
			FileInputStream(const char* filename);

			/**
			 * Construct new file input stream using the
			 * given file name
			 * @param filename a file name
			 */
			FileInputStream(const std::string& filename);

			/**
			 * Destruct
			 */
			virtual ~FileInputStream();

			/**
			 * Seek the given absolute file position
			 * @param pos file position
			 */
			void seek(long pos);

			/**
			 * Get the file size
			 */
			long getFileSize();

			// BEGIN IMPLEMENTATION OF INPUTSTREAM
			virtual int read(unsigned char* b, int len);
                        virtual unsigned char read();
			virtual void close();
			virtual bool eos();
			// END IMPLEMENTATION OF INPUTSTREAM

		private:
			
			void init(const char* filename);
			FILE* file;
			int avail;
			long fileSize;
			bool closed;
	};
}

#endif
