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

#include "encodings.h"

KnownEncoding get_encoding(const char *encoding) {
	KnownEncoding retVal;
	int i;
	char *lower;

	lower = (char*)malloc(strlen(encoding) + 1);
	for(i = 0; i <= strlen(encoding); i++) {
		lower[i] = tolower(encoding[i]);
	}

	if(!(strcmp(lower, "us-ascii") && strcmp(lower, "ascii"))) {
		retVal = ENCODING_ASCII;
	} else if(!strcmp(lower, "utf-7")) {
		retVal = ENCODING_UTF_7;
	} else if(!strcmp(lower, "utf-8")) {
		retVal = ENCODING_UTF_8;
	} else if(!(strcmp(lower, "utf-16") && strcmp(lower, "utf-16le"))) {
		retVal = ENCODING_UTF_16_LE;
	} else if(!strcmp(lower, "utf-16be")) {
		retVal = ENCODING_UTF_16_BE;
	} else if(!strcmp(lower, "utf-32")) {
		retVal = ENCODING_UTF_32;
	} else if(!(strcmp(lower, "cp819") && strcmp(lower, "csisolatin1") && strcmp(lower, "ibm819") && strcmp(lower, "iso-8859-1") && strcmp(lower, "iso-ir-100") && strcmp(lower, "l1") && strcmp(lower, "latin1"))) {
		retVal = ENCODING_LATIN1;
	} else {
		retVal = ENCODING_UNKNOWN;
	}

	free(lower);
	return retVal;
}

int get_encoding_name(char *output, KnownEncoding encoding) {
	switch(encoding) {
		case ENCODING_ASCII: {
			memcpy(output, "us-ascii", strlen("us-ascii") + 1);
			break;
		}
		case ENCODING_UTF_7: {
			memcpy(output, "utf-7", strlen("utf-7") + 1);
			break;
		}
		case ENCODING_UTF_8: {
			memcpy(output, "utf-8", strlen("utf-8") + 1);
			break;
		}
		case ENCODING_UTF_16_LE: {
			memcpy(output, "utf-16", strlen("utf-16") + 1);
			break;
		}
		case ENCODING_UTF_16_BE: {
			memcpy(output, "utf-16be", strlen("utf-16be") + 1);
			break;
		}
		case ENCODING_UTF_32: {
			memcpy(output, "utf-32", strlen("utf-32") + 1);
			break;
		}
		case ENCODING_LATIN1: {
			memcpy(output, "iso-8859-1", strlen("iso-8859-1") + 1);
			break;
		}
		case ENCODING_UNKNOWN: {
			return -1;
		}
	}

	return strlen(output);
}

int to_ascii(char *output, const unsigned char *original, KnownEncoding inputEncoding) {
	int retVal;
	const unsigned char *pt;

	pt = original;
	retVal = 0;
	switch(inputEncoding) {
		case ENCODING_UNKNOWN: {
			retVal = -1;
			break;
		}
		case ENCODING_ASCII: ENCODING_UTF_7: ENCODING_LATIN1: {
			memcpy(output, original, strlen(original));
			retVal = strlen(original);
			break;
		}
		case ENCODING_UTF_8: {
			while(*pt) {
				if(*pt < 128) {
					output[retVal++] = *pt;
					pt++;
				} else {
					pt += 2;
				}
			}
			output[retVal] = 0;
		}
		case ENCODING_UTF_16_LE: {
			while(*pt || *(pt+1)) {
				output[retVal++] = *pt;
				pt += 2;
			}
			output[retVal] = 0;
			break;
		}
		case ENCODING_UTF_16_BE: {
			pt++;
			while(*(pt-1) || *pt) {
				output[retVal++] = *pt;
				pt += 2;
			}
			output[retVal] = 0;
			break;
		}
		case ENCODING_UTF_32: {
			while(*pt || *(pt+1) || *(pt+2) || *(pt+3)) {
				output[retVal++] = *pt;
				pt += 4;
			}
			output[retVal] = 0;
			break;
		}
	}

	return retVal;
}
