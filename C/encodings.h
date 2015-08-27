/*
	Copyright (C) 2015 - Code written 100% by Valentino Giudice
	E-mail: valentino.giudice96@gmail.com
	Website: http://valentinogiudice.altervista.org/
	Twitter: http://twitter.com/aspie96

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


	(This work is also licensed under the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or, at your option, any later version).
*/

#ifndef ENCODINGS_H
#define ENCODINGS_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * Specify a character encoding used by the SmallChat protocol.
 */
enum KnownEncoding {
	/**
	 * This is the default output value of {@link get_encoding} and shall not be used for any other purpose.
	 */
	ENCODING_UNKNOWN,
	ENCODING_ASCII,
	ENCODING_UTF_7,
	ENCODING_UTF_8,
	ENCODING_UTF_16_LE,
	ENCODING_UTF_16_BE,
	ENCODING_UTF_32,
	ENCODING_LATIN1
};
typedef enum KnownEncoding KnownEncoding;

/**
 * Converts the name of a character encoding into an option of the {@link KnownEncoding} enumerator.
 *
 * @param   encoding    The name of the character encoding to be converted (it is not case sensitive).
 * @return  The corresponding {@link KnownEncoding} option (or {@link ENCODING_UNKNOWN} if it is not known).
 */
KnownEncoding get_encoding(const char*);

/**
 * Converts an option of the {@link KnownEncoding} enumerator into the name of its corresponding charset encoding.
 *
 * @param   output      The buffer to be written the name into.
 * @param   encoding    The option of {@link KnownEncoding} to be converted.
 * @return  The number of letters of the name (equivalent to {@code strlen(output)}) or {@code -1} if the value to be converted is {@link ENCODING_UNKNOWN}.
 */
int get_encoding_name(char*, KnownEncoding);

/**
 * Converts a non ASCII string into an ASCII one (Warning: this type of conversion is not always possible and this function might not work as expected. If better alternative are available, then use them).
 *
 * @param	output          The buffer to be written the ASCII string into.
 * @param	original        A pointer to the binary representation of the non ASCII string (which must have a properly sized null terminator).
 * @param	inputEncoding   The encoding of the original string.
 */
int to_ascii(char*, const unsigned char*, KnownEncoding);

#endif // ENCODINGS_H
