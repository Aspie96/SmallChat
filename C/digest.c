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

#include "digest.h"

#define T1(x)	(((~((x) >> 16)) << 16) | ((x) & 0xFFFF))
#define T2(x)	(~(T1(x)))
#define MV 8191
#define M(x, y)	((((x) * (y)) ^ (((x) + MV) * ((y) + MV))))

void sceda_digest_func(unsigned char *output, const unsigned char *original) {
	unsigned char buffer[16];
	unsigned int k, a, b, c, d, temp, tempA, i;

	memcpy(buffer, original, 16);
	for(i = 0; i < 4; i++) {
		a = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
		b = (buffer[4] << 24) + (buffer[5] << 16) + (buffer[6] << 8) + buffer[7];
		c = (buffer[8] << 24) + (buffer[9] << 16) + (buffer[10] << 8) + buffer[11];
		d = (buffer[12] << 24) + (buffer[13] << 16) + (buffer[14] << 8) + buffer[15];
		k = b ^ c ^ d;
		temp = a;
		tempA = a;
		a = M(T1(a), T2(b)) ^ k;
		k = temp ^ a;
		temp = b;
		b = M(T1(b), T2(c)) ^ k;
		k = temp ^ b;
		temp = c;
		c = M(T1(c), T2(d)) ^ k;
		k = temp ^ c;
		d = M(T1(d), T2(tempA)) ^ k;
		buffer[1] = a >> 24;
		buffer[2] = a >> 16;
		buffer[3] = a >> 8;
		buffer[4] = a;
		buffer[5] = c >> 24;
		buffer[6] = c >> 16;
		buffer[7] = c >> 8;
		buffer[8] = c;
		buffer[9] = b >> 24;
		buffer[10] = b >> 16;
		buffer[11] = b >> 8;
		buffer[12] = b;
		buffer[13] = d >> 24;
		buffer[14] = d >> 16;
		buffer[15] = d >> 8;
		buffer[0] = d;
	}
	memcpy(output, buffer, 16);
}

void sceda_digest(unsigned char *output, const unsigned char *original, int length) {
	unsigned char *buffer, temp1[16], temp2[16];
	int blockC, i, j;

	blockC = (length + 17) / 16;
	buffer = (unsigned char*)malloc(blockC * 16);
	memcpy(buffer, original, length);
	buffer[length] = length / 256;
	buffer[length + 1] = length % 256;
	for(i = length + 2; i < blockC * 16; i++) {
		buffer[i] = 170;
	}
	for(i = 0; i < blockC; i++) {
		sceda_digest_func(buffer + 16 * i, buffer + 16 * i);
	}
	memcpy(output, buffer, 16);
	for(i = 1; i < blockC; i++) {
		memcpy(temp1, output, 8);
		memcpy(temp1 + 8, buffer + 16 * i, 8);
		memcpy(temp2, buffer + 16 * i + 8, 8);
		memcpy(temp2 + 8, output + 8, 8);
		sceda_digest_func(temp1, temp1);
		sceda_digest_func(temp2, temp2);
		for(j = 0; j < 16; j++) {
			output[j] = temp1[j] ^ temp2[j];
		}
	}

	free(buffer);
}
