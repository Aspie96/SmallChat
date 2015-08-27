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

#ifndef SC_H
#define SC_H

#define SC_MAX_PDU 4096
#define SC_DEFAULT_PORT 4412

#include <arpa/inet.h>
#include <pthread.h>	/* -lpthread */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include "encodings.h"
#include "sceda.h"

/**
 * Provides information about a communication session on a host.
 */
struct SCInfo {
	struct sockaddr_in address;
	char *nickname;
	char *chatID;
};
typedef struct SCInfo SCInfo;

/**
 * Dynamically allocates and initializes a new instance of the {@link SCHostInfo} structure.
 *
 * @param   address     The value to be associated with the {@link SCHostInfo#address} field.
 * @param   nickname    The value to be associated with the {@link SCHostInfo#nickname} field (it will be duplicated).
 * @param   chatID      The value to be associated with the {@link SCHostInfo#chatID} field (it will be duplicated).
 * @return  A pointer to the allocated instance of {@link SCHostInfo}.
 */
SCInfo *scinfo_create(struct sockaddr_in, const char*, const char*);

/**
 * Duplicates an instance of the {@link SCHostInfo} structure created with {@link scinfo_create} or {@link scinfo_dup}.
 *
 * @param   original    A pointer to the instance to be duplicated.
 * @return  A pointer to the created duplicate.
 */
SCInfo *scinfo_dup(const SCInfo*);

/**
 * Destroys an instance of the {@link SCHostInfo} structure created with {@link scinfo_create} or {@link scinfo_dup}.
 *
 * @param   info    A pointer to the instance of the {@link SCHostInfo} to be destroyed (it must have been dynamically allocated).
 */
void scinfo_destroy(SCInfo*);


/**
 * Specify the type of a SmallChat PDU.
 */
enum SCPduType {
	/**
	 * This is the default output value of {@link scpdutype_get} and shall not be used for any other purpose.
	 */
	PDU_UNKNOWN,

	/**
	 * Hello ("HLO") PDUs are used to discover other hosts to communicate with. They are requests for a Welcome PDU from all other hosts and they contain the nickname to be associated with the sender's IP.
	 */
	PDU_HLO,

	/**
	 * Welcome ("ACK") PDUs are send as a response to Hello PDUs. They contain the nickname to be associated with the sender's IP.
	 */
	PDU_ACK,

	/**
	 * Leave ("LEV") PDUs are sent by hosts which are leaving the communication. They have no content.
	 */
	PDU_LEV,

	/**
	 * Message ("MSG") PDUs are used to send ordinary chat messages to other hosts.
	 */
	PDU_MSG,

	/**
	 * Malformed PDU notifications ("BAD") are sent as a response to broken PDUs and PDUs which have been encrypted with the wrong key. They contain the content of the received PDU (not re-encrypted).
	 */
	PDU_BAD,

	/**
	 * Nickname conflict notifications ("CNF") are sent when the IPs of two different hosts are associated with the same nickname to the involved hosts. They contain the dot representation of the IP address of the host the receiver is in conflict with (or "0.0.0.0" if the receiver is in conflict with the sender).
	 */
	PDU_CNF
};
typedef enum SCPduType SCPduType;

/**
 * Converts the acronym of a type of a certain type of PDU into an option of the {@link SCPduType} enumerator.
 *
 * @param   type    The acronym to be converted (it must be composed of three uppercase letters).
 * @return  The corresponding {@link SCPduType} option (or {@link PDU_UNKNOWN} if the acronym is not known).
 */
SCPduType scpdutype_get(const char*);

/**
 * Converts an option of the {@link SCPduType} enumerator into its corresponding acronym.
 *
 * @param   output  The buffer to be written the acronym into.
 * @param   type    The option of {@link SCPduType} to be converted.
 * @return  The number of letters of the acronym (equivalent to {@code strlen(output)}), which is always {@code 3}, or {@code -1} if the value to be converted is {@link PDU_UNKNOWN}.
 */
int scpdutype_name(char*, SCPduType);


/**
 * Represents a SmallChat PDU.
 */
struct SCPdu {
	char *chatID;
	SCPduType type;
	KnownEncoding encoding;
	unsigned char *payload;
	int payloadLength;
};
typedef struct SCPdu SCPdu;

/**
 * Dynamically allocates and initializes a new instance of the {@link SCPdu} structure.
 *
 * @param   chatID          The value to be associated with the {@link SCPdu#chatID} field (it will be duplicated).
 * @param   type            The value to be associated with the {@link SCPdu#type} field.
 * @param   encoding        The value to be associated with the {@link SCPdu#encoding} field.
 * @param   payload         The value to be associated with the {@link SCPdu#payload} field (it will be duplicated).
 * @param   payloadLength   The value to be associated with the {@link SCPdu#payloadLength} field.
 * @return  A pointer to the allocated instance of {@link SCHostInfo}.
 */
SCPdu *scpdu_create(const char*, SCPduType, KnownEncoding, const unsigned char*, int);

/**
 * Duplicates an instance of the {@link SCPdu} structure created with {@link scpdu_create}, {@link scpdu_dup} or {@link scpdu_from_binary}.
 *
 * @param   original    A pointer to the instance to be duplicated.
 * @return  A pointer to the created duplicate.
 */
SCPdu *scpdu_dup(const SCPdu*);

/**
 * Converts the binary representation of a SmallChat PDU into an instance of the {@link SCPdu} structure.
 *
 * @param   pdu     A pointer to the binary representation of the PDU.
 * @param   length  The size of the PDU.
 * @param   key     A pointer to the key to be used to encrypt the PDU (it must be 16 bytes long).
 *
 * @return  A pointer to the created instance of {@link SCPdu} (or {@code NULL} if it could not be converted).
 */
SCPdu *scpdu_from_binary(const unsigned char*, int, const unsigned char*);

/**int scpdu_to_binary(const SCPdu *pdu, unsigned char *output, const unsigned char *key)
 * Converts an instance of the {@link SCPdu} structure into the binary representation of the PDU.
 *
 * @param   pdu     A pointer to the instance of {@link SCPdu} to be converted.
 * @param   output  A pointer to the buffer to be written the binary output into.
 * @param   key     A pointer to the key to be used to encrypt the PDU (it must be 16 bytes long).
 * @return  The length of the binary representation of the PDU.
 */
int scpdu_to_binary(const SCPdu*, unsigned char*, const unsigned char*);

/**
 * Checks if the chatID of a PDU is the given one without trying to parse the PDU itself.
 *
 * @param   pdu     A pointer to the binary representation of the PDU.
 * @param   id      The chatID to be checked.
 * @param   pduSize The size of the binary representation of the PDU.
 * @return  {@code 1} if the chatID of the PDU is the given one, {@code 0} otherwise.
 */
int scpdu_check_id(const unsigned char*, const char*, int);

/**
 * Destroys an instance of the {@link SCPdu} structure created with {@link scpdu_create}, {@link scpdu_dup} or {@link scpdu_from_binary}.
 *
 * @param   pdu	A pointer to the instance of the {@link SCPdu} to be destroyed (it must have been dynamically allocated).
 */
void scpdu_destroy(SCPdu*);


struct SCInfoList;

/**
 * Represents a local SmallChat client.
 */
struct SCHost {
	SCInfo *info;
	unsigned char key[16];
	struct sockaddr_in broadcast;
	struct SCInfoList *others;
	int socket;
	pthread_t listener;
	int remainingBadNotifications;
	time_t firstBadNotification;

	/**
	 * Called when a valid message PDU is received.
	 * @param   info    A pointer to the instance of {@link SCInfo} which provides information about the sender.
	 * @param   pdu     A pointer to received PDU.
	 */
	void (*on_message)(const SCInfo*, const SCPdu*);

	/**
	 * Called when a valid hello PDU is received.
	 * @param   info    A pointer to the instance of {@link SCInfo} which provides information about the sender.
	 */
	void (*on_hello)(const SCInfo*);

	/**
	 * Called when a valid welcome PDU is received.
	 * @param   info    A pointer to the instance of {@link SCInfo} which provides information about the sender.
	 */
	void (*on_welcome)(const SCInfo*);

	/**
	 * Called when a valid leave PDU is received.
	 * @param   info    A pointer to the instance of {@link SCInfo} which provides information about the sender.
	 */
	void (*on_leave)(const SCInfo*);

	/**
	 * Called when a malformed PDU notification is received.
	 * @param   info    A pointer to the instance of {@link SCInfo} which provides information about the sender.
	 * @param   pdu     A pointer to the binary representation of the malformed PDU which has been received by the sender of the notification.
	 * @param   pduSize The length of the malformed PDU.
	 */
	void (*on_malformed_notification)(const SCInfo*, const char*, int);

	/**
	 * Called when a malformed PDU is received.
	 * @param   info    A pointer to the instance of {@link SCInfo} which provides information about the sender.
	 * @param   pdu     A pointer to the binary representation of the received malformed PDU.
	 * @param   pduSize The length of the PDU.
	 */
	void (*on_malformed_received)(const SCInfo*, const char*, int);

	/**
	 * Called when a nickname conflict with another host is detected (it might be called more than once for the same conflict).
	 * @param	informerInfo	A pointer to the instance of {@link SCInfo} which provides information about the sender of the conflict notification (or {@code NULL} if no notification has ben received).
	 * @param	rivalInfo		A pointer to the instance of {@link SCInfo} which provides information about the host this host is in conflict with.
	 */
	void (*on_conflict)(const SCInfo*, const SCInfo*);
};
typedef struct SCHost SCHost;

/**
 * Dynamically allocates and initializes a new instance of the {@link SCHost} structure. {@link schost_start} should be called then (after the function pointers in the instance are set).
 *
 * @param   nickname    The nickname to be associated with this host (it will be duplicated).
 * @param   chatID      The chatID of the communication this host will take part into (it will be duplicated).
 * @param   key         A pointer to the encryption key used in this communication (it must be 16 bytes long and it will be duplicated).
 * @param   port        The port to be used in this communication.
 * @return  A pointer to the allocated instance of {@link SCHost}.
 */
SCHost *schost_create(const char*, const char*, const unsigned char*, int);

/**
 * Initializes some fields of an instance of the {@link SCHost} struct, sends a broadcast hello PDU and starts a new thread to listen to messages from other hosts. It shall be called only once for each {@link SCHost} instance.
 *
 * @param   host    A pointer to the host to be started.
 */
void schost_start(SCHost*);

/**
 * Sends a broadcast hello PDU and updates the list of known hosts.
 *
 * @param   host    A pointer to the host which has to update its list of known hosts ({@link schost_start} should have been called for this host).
 */
void schost_hello(SCHost*);

/**
 * Return the nickname associated with a given known host.
 *
 * @param   host    A pointer to the host of which the known hosts list is to be used ({@link schost_start} should have been called for this host).
 * @param   output  The buffer to be written the nickname into (if the host is not found, {@code '\0'} will be written in the first byte.
 * @param   address	The address of the host to be looked for in the list of known hosts (or {@code -1} if it has not been found).
 */
int schost_get_nickname(const SCHost*, char*, struct sockaddr_in);

/**
 * Sends a unicast message PDU to all known hosts.
 *
 * @param   host    A pointer to the host which has to send the message ({@link schost_start} must have been called for this host).
 * @param   message The message to be sent.
 */
void schost_send(SCHost*, const char*);

/**
 * Sends a broadcast message PDU.
 *
 * @param   host    A pointer to the host which has to send the message ({@link schost_start} must have been called for this host).
 * @param   message The message to be sent.
 */
void schost_spartan_send(SCHost*, const char*);

/**
 * Sends a unicast message PDU to the given host.
 *
 * @param   host    A pointer to the host which has to send the message ({@link schost_start} must have been called for this host).
 * @param   message The message to be sent.
 */
void schost_unicast_send(SCHost*, struct sockaddr_in, const char*);

/**
 * Sends a unicast PDU to the given host.
 *
 * @param   host    A pointer to the host which has to send the message ({@link schost_start} must have been called for this host).
 * @param   address The address of the received host.
 * @param   pdu     A pointer to the PDU to be sent.
 */
void schost_manual_send(SCHost*, struct sockaddr_in, const SCPdu*);

/**
 * Destroys an instance of the {@link SCHost} structure created with {@link schost_create}.
 *
 * @param   host    A pointer to the instance of the {@link SCHost} to be destroyed (it must have been dynamically allocated).
 */
void schost_destroy(SCHost*);

#endif // SC_H 
