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

#include "sc.h"

/* =============================== SCInfo =============================== */
SCInfo *scinfo_create(struct sockaddr_in address, const char *nickname, const char *chatID) {
	SCInfo *retVal;
	retVal = (SCInfo*)malloc(sizeof(SCInfo));
	retVal->address = address;
	retVal->nickname = strdup(nickname);
	retVal->chatID = strdup(chatID);

	return retVal;
}

SCInfo *scinfo_dup(const SCInfo *original) {
	return scinfo_create(original->address, original->nickname, original->chatID);
}

void scinfo_destroy(SCInfo *info) {
	free(info->nickname);
	free(info->chatID);
	free(info);
}


/* =============================== SCPduType =============================== */
SCPduType scpdutype_get(const char *type) {
	if(!strcmp(type, "HLO")) {
		return PDU_HLO;
	}
	if(!strcmp(type, "ACK")) {
		return PDU_ACK;
	}
	if(!strcmp(type, "LEV")) {
		return PDU_LEV;
	}
	if(!strcmp(type, "MSG")) {
		return PDU_MSG;
	}
	if(!strcmp(type, "BAD")) {
		return PDU_BAD;
	}
	if(!strcmp(type, "CNF")) {
		return PDU_CNF;
	}
	return PDU_UNKNOWN;
}

int scpdutype_name(char *output, SCPduType type) {
	switch(type) {
		case PDU_HLO: {
			memcpy(output, "HLO", 4);
			break;
		}
		case PDU_ACK: {
			memcpy(output, "ACK", 4);
			break;
		}
		case PDU_LEV: {
			memcpy(output, "LEV", 4);
			break;
		}
		case PDU_MSG: {
			memcpy(output, "MSG", 4);
			break;
		}
		case PDU_BAD: {
			memcpy(output, "BAD", 4);
			break;
		}
		case PDU_CNF: {
			memcpy(output, "CNF", 4);
			break;
		}
		case PDU_UNKNOWN: {
			return -1;
		}
	}

	return 3;
}


/* =============================== SCPdu =============================== */

SCPdu *scpdu_create(const char *chatID, SCPduType type, KnownEncoding encoding, const unsigned char *payload, int payloadLength) {
	SCPdu *retVal;

	retVal = (SCPdu*)malloc(sizeof(SCPdu));
	retVal->chatID = strdup(chatID);
	retVal->type = type;
	retVal->encoding = encoding;
	retVal->payload = (unsigned char*)malloc(payloadLength);
	memcpy(retVal->payload, payload, payloadLength);
	retVal->payloadLength = payloadLength;

	return retVal;
}

SCPdu *scpdu_dup(const SCPdu *original) {
	return scpdu_create(original->chatID, original->type, original->encoding, original->payload, original->payloadLength);
}

SCPdu *scpdu_from_binary(const unsigned char *pdu, int length, const unsigned char *key) {
	SCPdu *retVal;
	SCPduType type;
	int msgLen;
	unsigned char *msg;
	const unsigned char *pt, *iv;
	char *temp;
	KnownEncoding encoding;

	pt = pdu;
	if(*(pt++)!=0 || *(pt++)!=1) {
		return 0;
	}
	while(pt-pdu < length && *(pt++));
	if(pt-pdu > length + 10) {
		return 0;
	}
	iv = pt;
	pt += 8;
	msg = (unsigned char*)malloc(length);
	msgLen = sceda_decrypt(msg, pt, length - (pt - pdu), key, iv);
	temp = (char*)malloc(4);
	pt = msg;
	memcpy(temp, pt, 3);
	temp[3] = 0;
	type = scpdutype_get(temp);
	if(type == PDU_UNKNOWN) {
		free(msg);
		return 0;
	}
	free(temp);
	pt += 3;
	encoding = get_encoding(pt);
	if(encoding == ENCODING_UNKNOWN) {
		free(msg);
		return 0;
	}
	pt += strlen(pt) + 1;
	retVal = scpdu_create(pdu + 2, type, encoding, pt, msgLen - strlen(msg) - 1);

	free(msg);
	return retVal;
}

int scpdu_to_binary(const SCPdu *pdu, unsigned char *output, const unsigned char *key) {
	unsigned char *pt, iv[8];
	int msgLen;

	if(pdu->type == PDU_UNKNOWN) {
		return -1;
	}
	pt = output;
	*(pt++) = 0;
	*(pt++) = 1;
	memcpy(pt, pdu->chatID, strlen(pdu->chatID) + 1);
	pt += strlen(pdu->chatID) + 1;
	sceda_generate_iv(iv);
	memcpy(pt, iv, 8);
	pt += 8;
	msgLen = scpdutype_name(pt, pdu->type);
	msgLen += get_encoding_name(pt + msgLen, pdu->encoding) + 1;
	memcpy(pt + msgLen, pdu->payload, pdu->payloadLength);
	msgLen += pdu->payloadLength;
	msgLen = sceda_encrypt(pt, pt, msgLen, key, iv);
	return (pt - output) + msgLen;
}

int scpdu_check_id(const unsigned char *pdu, const char *id, int pduSize) {
	int index;

	if(pduSize < strlen(id) + 3) {
		return 0;
	}
	index = 0;
	while(index < strlen(id) + 1) {
		if((pdu+2)[index] != id[index++]) {
			return 0;
		}
	}
	return 1;
}

void scpdu_destroy(SCPdu *pdu) {
	free(pdu->chatID);
	free(pdu->payload);
	free(pdu);
}


/* =============================== SCPdu =============================== */
struct SCInfoList {
	SCInfo *info;
	struct SCInfoList *next;
};

SCHost *schost_create(const char *nickname, const char *chatID, const unsigned char *key, int port) {
	SCHost *retVal;
	struct sockaddr_in loopback;

	retVal = (SCHost*)malloc(sizeof(SCHost));
	loopback.sin_family = AF_INET;
	loopback.sin_port = htons(port);
	loopback.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	bzero(loopback.sin_zero, 8);
	retVal->info = scinfo_create(loopback, nickname, chatID);
	memcpy(retVal->key, key, 16);
	retVal->broadcast.sin_family = AF_INET;
	retVal->broadcast.sin_port = htons(port);
	retVal->broadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	bzero(retVal->broadcast.sin_zero, 8);
	retVal->socket = -1;
	retVal->on_hello = 0;
	retVal->on_welcome = 0;
	retVal->on_leave = 0;
	retVal->on_malformed_notification = 0;
	retVal->on_malformed_received = 0;
	retVal->on_conflict = 0;

	return retVal;
}

int schost_add(SCHost *host, SCInfo *info, int notifyConflict) {
	struct SCInfoList *pt;
	int retVal;
	SCPdu *notification;

	retVal = 1;
	if(host->others) {
		pt = host->others;
		while(pt) {
			if(ntohl(pt->info->address.sin_addr.s_addr) != ntohl(info->address.sin_addr.s_addr)) {
				retVal = 0;
				free(pt->info->nickname);
				pt->info->nickname = strdup(info->nickname);
			}
			pt = pt->next;
		}
		if(retVal && notifyConflict) {
			pt = host->others;
			while(pt) {
				if(!strcmp(pt->info->nickname, info->nickname)) {
					notification = scpdu_create(host->info->chatID, PDU_CNF, ENCODING_ASCII, inet_ntoa(info->address.sin_addr), strlen(inet_ntoa(info->address.sin_addr)));
					schost_manual_send(host, pt->info->address, notification);
					scpdu_destroy(notification);
					notification = scpdu_create(host->info->chatID, PDU_CNF, ENCODING_ASCII, inet_ntoa(pt->info->address.sin_addr), strlen(inet_ntoa(pt->info->address.sin_addr)));
					schost_manual_send(host, info->address, notification);
					scpdu_destroy(notification);
				}
				pt = pt->next;
			}
		}
	}
	if(notifyConflict && !strcmp(info->nickname, host->info->nickname)) {
		if(host->on_conflict) {
			host->on_conflict(NULL, info);
		}
		notification = scpdu_create(host->info->chatID, PDU_CNF, ENCODING_ASCII, inet_ntoa(host->info->address.sin_addr), strlen(inet_ntoa(host->info->address.sin_addr)));
		schost_manual_send(host, info->address, notification);
		scpdu_destroy(notification);
	}
	if(retVal) {
		if(host->others) {
			pt = host->others;
			while(pt->next) {
				pt = pt->next;
			}
			pt->next = (struct SCInfoList*)malloc(sizeof(struct SCInfoList));
			pt->next->next = 0;
			pt->next->info = scinfo_dup(info);
		} else {
			host->others = (struct SCInfoList*)malloc(sizeof(struct SCInfoList));
			host->others->next = 0;
			host->others->info = scinfo_dup(info);
		}
	}

	return retVal;
}

void *listener(void *params) {
	SCHost *host;
	int length, fine;
	unsigned char buffer[SC_MAX_PDU];
	struct sockaddr_in sender, cnfAddr;
	socklen_t addressSize;
	SCPdu *received, *response;
	SCInfo *info, *cnfInfo;
	struct SCInfoList *pt, *temp;

	host = (SCHost*)params;
	addressSize = (socklen_t)sizeof(struct sockaddr_in);
	for(;;) {
		if(((length = recvfrom(host->socket, buffer, SC_MAX_PDU, 0, (struct sockaddr*)&sender, &addressSize)) > 0) && (ntohl(sender.sin_addr.s_addr) != ntohl(host->info->address.sin_addr.s_addr)) && scpdu_check_id(buffer, host->info->chatID, length)) {
			if(received = scpdu_from_binary(buffer, length, host->key)) {
				fine = 1;
				schost_get_nickname(host, (char*)buffer, sender);
				info = scinfo_create(sender, buffer, host->info->chatID);
				switch(received->type) {
					case PDU_HLO: {
						free(info->nickname);
						memcpy(buffer, received->payload, received->payloadLength);
						bzero(buffer + received->payloadLength, 4);
						to_ascii(buffer, buffer, received->encoding);
						info->nickname = strdup(buffer);
						if(schost_add(host, info, 1) && host->on_hello) {
							host->on_hello(info);
						}
						response = scpdu_create(host->info->chatID, PDU_ACK, ENCODING_ASCII, host->info->nickname, strlen(host->info->nickname));
						schost_manual_send(host, sender, response);
						scpdu_destroy(response);
						break;
					}
					case PDU_ACK: {
						free(info->nickname);
						memcpy(buffer, received->payload, received->payloadLength);
						bzero(buffer + received->payloadLength, 4);
						to_ascii(buffer, buffer, received->encoding);
						info->nickname = strdup(buffer);
						if(schost_add(host, info, 1) && host->on_welcome) {
							host->on_welcome(info);
						}
						break;
					}
					case PDU_LEV: {
						if(host->others) {
							if(ntohl(host->others->info->address.sin_addr.s_addr) == ntohl(sender.sin_addr.s_addr)) {
								temp = host->others->next;
								scinfo_destroy(host->others->info);
								free(host->others->info);
								host->others->next = temp;
							} else {
								pt = host->others;
								while(pt->next && (ntohl(pt->next->info->address.sin_addr.s_addr) != ntohl(sender.sin_addr.s_addr))) {
									pt = pt->next;
								}
								if(pt) {
									temp = pt->next->next;
									scinfo_destroy(pt->next->info);
									free(pt->next);
									pt->next = temp;
								}
							}
						}
						if(host->on_leave) {
							host->on_leave(info);
						}
						break;
					}
					case PDU_MSG: {
						if(host->on_message) {
							host->on_message(info, received);
						}
						break;
					}
					case PDU_BAD: {
						if(host->on_malformed_notification) {
							host->on_malformed_notification(info, received->payload, received->payloadLength);
						}
						break;
					}
					case PDU_CNF: {
						if(host->on_conflict) {
							memcpy(buffer, received->payload, received->payloadLength);
							bzero(buffer + received->payloadLength, 4);
							to_ascii(buffer, buffer, received->encoding);
							inet_aton((char*)buffer, &(cnfAddr.sin_addr));
							schost_get_nickname(host, buffer, cnfAddr);
							cnfInfo = scinfo_create(cnfAddr, buffer, host->info->chatID);
							host->on_conflict(info, cnfInfo);
							scinfo_destroy(cnfInfo);
						}
						break;
					}
					case PDU_UNKNOWN: {
						fine = 0;
						break;
					}
				}
				scpdu_destroy(received);
			} else {
				info = scinfo_create(sender, "", host->info->chatID);
				fine = 0;
			}
			if(!fine) {
				if(time(0) - host->firstBadNotification > 600) {
					host->remainingBadNotifications = 4;
				}
				host->remainingBadNotifications--;
				if(host->remainingBadNotifications > -1) {
					response = scpdu_create(host->info->chatID, PDU_BAD, ENCODING_ASCII, 0, 0);
					schost_manual_send(host, sender, response);
					scpdu_destroy(response);
					host->firstBadNotification = time(0);
				}
				if(host->on_malformed_notification) {
					host->on_malformed_notification(info, buffer, length);
				}
			}
			scinfo_destroy(info);
		}
	}
}

void schost_start(SCHost *host) {
	struct sockaddr_in any;
	int allowBroadcast;
	socklen_t addressSize;

	host->socket = socket(AF_INET, SOCK_DGRAM, 0);
	any.sin_family = AF_INET;
	any.sin_port = host->info->address.sin_port;
	any.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(any.sin_zero, 8);
	bind(host->socket, (struct sockaddr*)&any, (socklen_t)sizeof(struct sockaddr_in));
	allowBroadcast = 1;
	setsockopt(host->socket, SOL_SOCKET, SO_BROADCAST, &allowBroadcast, sizeof(int));
	addressSize = (socklen_t)sizeof(struct sockaddr_in);
	schost_hello(host);
	recvfrom(host->socket, 0, 0, 0, (struct sockaddr*)&(host->info->address), &addressSize);
	host->remainingBadNotifications = 4;
	pthread_create(&(host->listener), 0, listener, host);
}

void schost_hello(SCHost *host) {
	SCPdu *hello;
	struct SCInfoList *pt, *temp;

	pt = host->others;
	while(pt) {
		temp = pt->next;
		scinfo_destroy(pt->info);
		free(pt);
		pt = temp;
	}
	host->others = 0;

	hello = scpdu_create(host->info->chatID, PDU_HLO, ENCODING_ASCII, host->info->nickname, strlen(host->info->nickname));
	schost_manual_send(host, host->broadcast, hello);
	scpdu_destroy(hello);
}

int schost_get_nickname(const SCHost *host, char *output, struct sockaddr_in address) {
	struct SCInfoList *pt;

	if(host->others) {
		pt = host->others;
		while(pt) {
			if(ntohl(pt->info->address.sin_addr.s_addr) == ntohl(address.sin_addr.s_addr)) {
				memcpy(output, pt->info->nickname, strlen(pt->info->nickname) + 1);
				return strlen(pt->info->nickname);
			}
			pt = pt->next;
		}
	}
	output[0] = 0;
	return -1;
}

void schost_send(SCHost *host, const char *message) {
	struct SCInfoList *pt;

	pt = host->others;
	while(pt) {
		schost_unicast_send(host, pt->info->address, message);
		pt = pt->next;
	}
}

void schost_spartan_send(SCHost *host, const char *message) {
	schost_unicast_send(host, host->broadcast, message);
}

void schost_unicast_send(SCHost *host, struct sockaddr_in address, const char *message) {
	SCPdu *pdu;

	pdu = scpdu_create(host->info->chatID, PDU_MSG, ENCODING_ASCII, message, strlen(message));
	schost_manual_send(host, address, pdu);
	scpdu_destroy(pdu);
}

void schost_manual_send(SCHost *host, struct sockaddr_in address, const SCPdu *pdu) {
	unsigned char binaryPdu[SC_MAX_PDU];
	int length;

	length = scpdu_to_binary(pdu, binaryPdu, host->key);
	sendto(host->socket, binaryPdu, length, 0, (struct sockaddr*)&address, (socklen_t)sizeof(struct sockaddr_in));
}

void schost_destroy(SCHost *host) {
	struct SCInfoList *pt, *temp;
	SCPdu *pdu;

	pdu = scpdu_create(host->info->chatID, PDU_LEV, ENCODING_ASCII, 0, 0);
	pt = host->others;
	while(pt) {
		if(host->socket >= 0) {
			schost_manual_send(host, pt->info->address, pdu);
		}
		temp = pt->next;
		scinfo_destroy(pt->info);
		free(pt);
		pt = temp;
	}
	scpdu_destroy(pdu);
	scinfo_destroy(host->info);
	if(host->socket >= 0) {
		close(host->socket);
		pthread_cancel(host->listener);
	}
	free(host);
}
