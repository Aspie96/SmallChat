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

#include "sceda.h"

void sceda_func(unsigned char *output, const unsigned char *original, int blockC, const unsigned char *key, const unsigned char *iv, int decrypting) {
	unsigned char blockKey[49], hashedBlockKey[16], *pt2;
	const unsigned char *pt1;
	int i, j;

	memcpy(blockKey, key, 16);
	memcpy(blockKey + 16, iv, 8);
	memcpy(blockKey + 24, iv, 8);
	memcpy(blockKey + 32, iv, 8);
	memcpy(blockKey + 40, iv, 8);
	blockKey[48] = 0;

	pt1 = original;
	pt2 = output;
	for(i = 0; i < blockC; i++) {
		sceda_digest(hashedBlockKey, blockKey, 49);
		if(decrypting) {
			memcpy(blockKey + 16, pt1, 16);
		} else {
			memcpy(blockKey + 32, pt1, 16);
		}

		for(j = 0; j < 16; j++) {
			pt2[j] = pt1[j] ^ hashedBlockKey[j];
		}

		if(decrypting) {
			memcpy(blockKey + 32, pt2, 16);
		} else {
			memcpy(blockKey + 16, pt2, 16);
		}
		blockKey[32]++;
		pt1 += 16;
		pt2 += 16;
	}
}

int sceda_encrypt(unsigned char *output, const unsigned char *original, int length, const unsigned char *key, const unsigned char *iv) {
	int retVal, i, temp;
	unsigned char *pt, *originalCopy;

	srand(time(NULL) + rand());
	originalCopy = (unsigned char*)malloc(length);
	memcpy(originalCopy, original, length);
	retVal = ((length+15)/16) * 16;
	temp = length;
	pt = output + 6;
	for(i = 0; i < 7; i++) {
		*pt-- = temp % 256;
		temp /= 256;
	}
	pt += 8;
	memcpy(pt, originalCopy, length);
	pt += length;
	for(i = 0; i < retVal - length + 9; i++) {
		*(pt++) = rand();
	}
	pt = output + 7;
	sceda_func(pt, pt, (length+15) / 16, key, pt + retVal, 0);
	retVal += 16;
	for(i = 0; i < retVal/2; i++) {
		temp = output[i];
		output[i] = output[retVal-i-1];
		output[retVal-i-1] = temp;
	}
	sceda_func(output, output, retVal / 16, key, iv, 0);

	free(originalCopy);
	return retVal;
}

int sceda_decrypt(unsigned char *output, const unsigned char *original, int length, const unsigned char *key, const unsigned char *iv) {
	int retVal, i, temp;
	unsigned char *pt, *buffer;

	if(length % 16) {
		return -1;
	}
	buffer = (unsigned char*)malloc(length);
	sceda_func(buffer, original, length / 16, key, iv, 1);
	for(i = 0; i < length/2; i++) {
		temp = buffer[i];
		buffer[i] = buffer[length-i-1];
		buffer[length-i-1] = temp;
	}
	pt = buffer;
	retVal = 0;
	for(i = 0; i < 7; i++) {
		retVal *= 256;
		retVal += *pt++;
	}
	if((retVal < 0) || (retVal > length-16)) {
		return -1;
	}
	sceda_func(buffer, buffer + 7, (retVal+15) / 16, key, buffer + ((retVal+15)/16) * 16 + 7, 1);
	memcpy(output, buffer, retVal);

	free(buffer);
	return retVal;
}

int encrypted_length(int originalLength) {
	return ((originalLength + 31) / 16) * 16;
}

void sceda_generate_key(unsigned char *output) {
	int i;

	srand(time(NULL) + rand());
	for(i = 0; i < 16; i++) {
		output[i] = rand();
	}
}

void sceda_generate_iv(unsigned char *output) {
	int i;

	srand(time(NULL) + rand());
	for(i = 0; i < 8; i++) {
		output[i] = rand();
	}
}
