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

#include <stdio.h>
#include <unistd.h>
#include "sc.h"

void on_hello(const SCInfo*);
void on_welcome(const SCInfo*);
void on_leave(const SCInfo*);
void on_message(const SCInfo*, const SCPdu*);
void on_malformed(const SCInfo*, const char*, int);
void on_conflict(const SCInfo*, const SCInfo*);

int main(int argc, char **argv) {
	char chatID[101], nickname[101], key[101], message[101], input[101];
	SCHost *myself;
	int port;

	chatID[0] = '\0';
	key[0] = '\0';
	while(strlen(chatID) == 0) {
		printf("Chat ID: ");
		fgets(chatID, 100, stdin);
		chatID[strlen(chatID)-1] = '\0';
	}
	nickname[0] = '\0';
	while(strlen(nickname) == 0) {
		printf("Nickname: ");
		fgets(nickname, 100, stdin);
		nickname[strlen(nickname)-1] = '\0';
	}
	printf("Key: ");
	fgets(key, 100, stdin);
	key[strlen(key)-1] = '\0';

	sceda_digest(key, key, strlen(key));
	myself = schost_create(nickname, chatID, key, SC_DEFAULT_PORT);
	myself->on_hello = on_hello;
	myself->on_welcome = on_welcome;
	myself->on_leave = on_leave;
	myself->on_message = on_message;
	myself->on_malformed_received = on_malformed;
	myself->on_malformed_notification = on_malformed;
	myself->on_conflict = on_conflict;
	schost_start(myself);
	printf("Begin to chat now!\n\n");
	for(;;) {
		fgets(message, 100, stdin);
		message[strlen(message)-1] = '\0';
		schost_send(myself, message);
	}
	/*schost_destroy(myself);
	return 0;*/
}

void on_hello(const SCInfo *info) {
	printf("%s has joined the chat!\n", info->nickname);
}

void on_welcome(const SCInfo *info) {
	printf("%s is online!\n", info->nickname);
}

void on_leave(const SCInfo *info) {
	printf("%s is offline!\n", info->nickname);
}

void on_message(const SCInfo *info, const SCPdu *pdu) {
	char *buffer = (char*)malloc(pdu->payloadLength + 4);
	memcpy(buffer, pdu->payload, pdu->payloadLength);
	bzero(buffer + pdu->payloadLength, 4);
	to_ascii(buffer, buffer, pdu->encoding);
	printf("%s: %s\n", info->nickname, buffer);
}

void on_malformed(const SCInfo *info, const char *pdu, int pduSize) {
	printf("Problem while communicating with %s!\n", info->nickname);
}

int conflictNotified = 0;
void on_conflict(const SCInfo *informerInfo, const SCInfo *rivalInfo) {
	if(!conflictNotified) {
		printf("Nickname collision detected!\n");
		conflictNotified = 1;
	}
}
