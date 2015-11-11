/***************************************************************************
 *   Copyright (C) 2005 by dREI C systems GbR                              *
 *   Author: thorsten.liese@dreicsystems.de                                *
 ***************************************************************************/

#include "FileInputStream.h"

#include <cstdio>

using namespace std;

namespace com_myfridget
{
	FileInputStream::FileInputStream(const string& filename) : avail(0), closed(false)
	{
		init(filename.c_str());
	}

	FileInputStream::FileInputStream(const char* filename) : avail(0), closed(false)
	{
		init(filename);
	}

	void FileInputStream::init(const char* filename)
	{
		file = fopen(filename, "rb");
		if (!file) throw "Could not open file.";
		if (fseek(file, 0, SEEK_END)) throw "Could not determine file size";
		fileSize = ftell(file);
		seek(0);
	}

	FileInputStream::~FileInputStream()
	{
		if (!closed) close();
	}

	void FileInputStream::seek(long pos)
	{
		if (fseek(file, pos, SEEK_SET)) throw "Could not seek.";
		avail = fileSize - pos;
	}

	long FileInputStream::getFileSize()
	{
		return fileSize;
	}

	void FileInputStream::close() 
	{
		fclose(file);
		closed = true;
	}

	int FileInputStream::read(unsigned char* b, int len)
	{
		if (avail==0) return -1; // EOS
		if (len > avail) len = avail;
		len = (int)fread(b, 1, len, file);
		avail -= len;
		return len;
	}

        unsigned char FileInputStream::read() {
            unsigned char b;
            fread(&b, 1, 1, file);
            avail--;
            return b;
        }

	bool FileInputStream::eos()
	{
		return (avail==0);
	}

}

