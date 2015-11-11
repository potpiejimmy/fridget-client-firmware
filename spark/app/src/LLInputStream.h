/* 
 * File:   LLInputStream.h
 * Author: thorsten
 *
 * Created on November 5, 2014, 12:29 PM
 */

#ifndef _com_myfridget_LLInputStream_H_
#define _com_myfridget_LLInputStream_H_

namespace com_myfridget
{
	/**
	 * This abstract class is the superclass of all classes representing
	 * an input stream of bytes.
	 *
	 * <p> Applications that need to define a subclass of <code>LLInputStream</code>
	 * must always provide a method that returns the next byte of input.
	 */
	class LLInputStream
	{
	
	public:

		/**
		 * Virtual destructor calls the close method.
		 */
		virtual ~LLInputStream();

		/**
		 * Reads some number of bytes from the input stream and stores them into
		 * the buffer array <code>b</code>. The number of bytes actually read is
		 * returned as an integer.  This method blocks until input data is
		 * available, end of file is detected, or an exception is thrown.
		 *
		 * <p> If the length of
		 * <code>b</code> is zero, then no bytes are read and <code>0</code> is
		 * returned; otherwise, there is an attempt to read at least one byte. If
		 * no byte is available because the stream is at end of file, the value
		 * <code>-1</code> is returned; otherwise, at least one byte is read and
		 * stored into <code>b</code>.
		 *
		 * <p> The first byte read is stored into element <code>b[0]</code>, the
		 * next one into <code>b[1]</code>, and so on. The number of bytes read is,
		 * at most, equal to the length of <code>b</code>. Let <i>k</i> be the
		 * number of bytes actually read; these bytes will be stored in elements
		 * <code>b[0]</code> through <code>b[</code><i>k</i><code>-1]</code>,
		 * leaving elements <code>b[</code><i>k</i><code>]</code> through
		 * <code>b[b.length-1]</code> unaffected.
		 *
		 * <p> If the first byte cannot be read for any reason other than end of
		 * file, then an <code>IOException</code> is thrown. In particular, an
		 * <code>IOException</code> is thrown if the input stream has been closed.
		 *
		 * @param      b   the buffer into which the data is read.
		 * @param      len the length of the buffer specified by b.
		 * @return     the total number of bytes read into the buffer, or
		 *             <code>-1</code> is there is no more data because the end of
		 *             the stream has been reached.
		 */
		virtual int read(unsigned char* b, int len) = 0;

		/**
		 * Reads the next byte of data from the input stream. The value byte is
         * returned as an int in the range 0 to 255. If no byte is available because
		 * the end of the stream has been reached, the value -1 is returned.
         * This method blocks until input data is available, the end of the stream
         * is detected, or an exception is thrown. A subclass must provide an
		 * implementation of this method.
	     */
		virtual unsigned char read() = 0;

		/**
		 * Closes this input stream and releases any system resources associated
		 * with the stream.
		 *
		 * <p> The <code>close</code> method of <code>InputStream</code> does
		 * nothing.
		 *
		 */
		virtual void close();
	
		/**
		 * Returns whether reading from this input stream has reached the end of the stream (End Of Stream EOF).
		 * Implementations are required to return the value true only if a prior call
		 * to the read() or skip() function already reached the end of stream and returned -1. Implementations
		 * are NOT required, however, to return true in any case a future read/skip may be at the
		 * end of stream.
		 */
		virtual bool eos() = 0;
	};
}

#endif /* _com_myfridget_LLInputStream_H_ */
