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

#ifndef SCEDA_H
#define SCEDA_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "digest.h"

/**
 * Encrypts a message using SCEDA.
 *
 * @param   output      A pointer to the buffer to be written the output into.
 * @param   original    A pointer to the binary representation of the message to be encrypted.
 * @param   length      The length of the message to be encrypted.
 * @param   key         A pointer to the key to be used to encrypted the message (it must be 16 bytes long).
 * @param   iv          A pointer to the initialization vector to be used to encrypt the message (it must be 8 bytes long).
 * @return  The length of the encrypted message (which is also the output of {@code encrypted_length(length)}).
 */
int sceda_encrypt(unsigned char*, const unsigned char*, int, const unsigned char*, const unsigned char*);

/**
 * Decrypts a message encrypted with SCEDA.
 *
 * @param   output      A pointer to the buffer to be written the output into.
 * @param   original    A pointer to the binary message to be decrypted.
 * @param   length      The length of the encrypted message.
 * @param   key         A pointer to the key used to encrypt the message (it must be 16 bytes long).
 * @param   iv          A pointer to the initialization vector used to encrypt the message (it must be 16 bytes long).
 * @return  The length of the decrypted message.
 */
int sceda_decrypt(unsigned char*, const unsigned char*, int, const unsigned char*, const unsigned char*);

/**
 * Guesses the length of an encrypted message by its length when non encrypted without actually encrypting it.
 *
 * @param   originalLength  The length of the clear message.
 */
int encrypted_length(int);

/**
 * Generates a random key eligible to be used to encrypt messages with SCEDA.
 *
 * @param   output  A pointer to the buffer to be written the 16 bytes long generated key into.
 */
void sceda_generate_key(unsigned char*);

/**
 * Generates a random initialization vector eligible to be used to encrypt messages with SCEDA.
 *
 * @param   output  A pointer to the buffer to be written the 8 bytes long generated initialization vector into.
 */
void sceda_generate_iv(unsigned char*);

#endif // SCEDA_H
