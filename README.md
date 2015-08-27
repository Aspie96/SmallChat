# SmallChat
SmallChat is a peer to peer UDP-based protocol created by Valentino Giudice initially designed to create a local network chat without needing to be connected to the Internet. Now, it can actually also be used for communicating with hosts outside the network. Despite it was designed for chat, the protocol can also be used in other scenarios.


## Author
SmallChat has been designed by Valentino Giudice, an Italian Computer Science student.
- E-mail address: valentino.giudice96@gmail.com
- Italian website: http://valentinogiudice.altervista.org/
- Twitter account: http://twitter.com/aspie96


## License
SmallChat is dual-licensed:
- Under the Expat License: http://opensource.org/licenses/MIT
- Under the GNU General Public License version 2 or (at your option) any later version: http://www.gnu.org/licenses/gpl-2.0.html


## Compatibility
SmallChat is available both in C# and C languages.

The C# version has been built using Visual Studio 2013 and has been designed for Windows OSs, but may also be used in other environments.

The C version has been designed for Linux OSs, but should work on any Unix-like operating system.

Either the C# and the C versions work both on 32 bit and on 64 bit architectures.

## Encryption notes
With SmallChat all PDUs are encrypted using SCEDA (SmallChat Encryption-Decryption Algorithm).

SCEDA requires a 16 bytes long key and a randomly generated non necessearly secret initialization vector (those two data are also needed whyle decrypting).

Even encrypting the exact same message twice, using the same key **and the same initialization vector**, the output is completely different (and that makes it even more safe against statistical attacks), but, of course, when the two messages are decrypted the original message is always returned.

It is impossible to only decrypt a part of the message and you can't get the key from an encrypted message and the original one.

If, for some reason, some part of the message changes during transport, it will not be possible to decrypt it (and therefore the broken PDU will be detected).

In order to work, SCEDA needs a cryptographic hash function, so ScedaDigest is used.


### ScedaDigest
ScedaDigest is a hashing function. Its result is 16 bytes long.

ScedaDigest is used both inside SCEDA and by the provided demo SmallChat interfaces to generate the 16 bytes long key.

## IANA Port Number and Service Name
On 18th August 2015 IANA officially assigned the registered UDP port number 4412 to SmallChat, according to [RFC 6335](http://tools.ietf.org/html/rfc6335).

The case-insensitive service name assigned to SmallChat is "smallchat".

The list of all assigned ports is available here:
https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
