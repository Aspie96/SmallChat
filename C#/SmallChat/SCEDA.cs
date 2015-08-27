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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SmallChat
{
    /// <summary>The exception that is thrown by methods of the <see cref="SCEDA"/> class when an error occurs.</summary>
    public class ScedaException : Exception
    {
        private const string defaultMessage = "Error while crunching data!";

        public ScedaException() : base(ScedaException.defaultMessage) { }

        public ScedaException(string message) : base(message) { }

        public ScedaException(string message, Exception inner) : base(message, inner) { }

        public ScedaException(Exception inner) : base(ScedaException.defaultMessage, inner) { }
    }

    /// <summary>
    /// The exception that is thrown by <see cref="SCEDA.Decrypt(byte[], byte[], byte[])"/> when unable to decrypt a message (usually because of a wrong key or initializaiton vector).
    /// Please notice: it might not always be possible to detect when the key or the initialization vector are wrong and if they are but no exception is thrown then a wrong message is returned.
    /// </summary>
    public class ScedaDecryptingException : ScedaException
    {
        private const string defaultMessage = "Error while decrypting the message! The key or the initialization vector are probably wrong.";

        public ScedaDecryptingException() : base(ScedaDecryptingException.defaultMessage) { }

        public ScedaDecryptingException(string message) : base(message) { }

        public ScedaDecryptingException(string message, Exception inner) : base(message, inner) { }

        public ScedaDecryptingException(Exception inner) : base(ScedaDecryptingException.defaultMessage, inner) { }
    }

    /// <summary>Provides the possibility of encrypting and decrypting data using SCEDA (SmallChat Encryption-Decryption Algorithm).</summary>
    public static class SCEDA
    {
        private static Random r;

        static SCEDA()
        {
            SCEDA.r = new Random();
        }

        private static byte[] ScedaFunc(byte[] input, byte[] key, byte[] initializationVector, bool decrypting)
        {
            byte[] retVal = new byte[input.Length];

            byte[] blockKey = new byte[49];
            Array.Copy(key, 0, blockKey, 0, 16);
            Array.Copy(initializationVector, 0, blockKey, 16, 8);
            Array.Copy(initializationVector, 0, blockKey, 24, 8);
            Array.Copy(initializationVector, 0, blockKey, 32, 8);
            Array.Copy(initializationVector, 0, blockKey, 40, 8);
            blockKey[48] = 0;

            for (int i = 0; i < input.Length / 16; i++)
            {
                byte[] hashedBlockKey = SCEDA.Digest(blockKey);
                for (int j = 0; j < 16; j++)
                {
                    retVal[i * 16 + j] = (byte)(input[i * 16 + j] ^ hashedBlockKey[j]);
                }

                if (decrypting)
                {
                    Array.Copy(input, i * 16, blockKey, 16, 16);
                    Array.Copy(retVal, i * 16, blockKey, 32, 16);
                }
                else
                {
                    Array.Copy(retVal, i * 16, blockKey, 16, 16);
                    Array.Copy(input, i * 16, blockKey, 32, 16);
                }
                blockKey[32]++;
            }

            return retVal;
        }

        private static byte[] DigestFunc(byte[] input)
        {
            byte[] retVal = new byte[16];

            Array.Copy(input, retVal, 16);
            for (int i = 0; i < 4; i++)
            {
                uint a = (uint)((retVal[0] << 24) + (retVal[1] << 16) + (retVal[2] << 8) + retVal[3]);
                uint b = (uint)((retVal[4] << 24) + (retVal[5] << 16) + (retVal[6] << 8) + retVal[7]);
                uint c = (uint)((retVal[8] << 24) + (retVal[9] << 16) + (retVal[10] << 8) + retVal[11]);
                uint d = (uint)((retVal[12] << 24) + (retVal[13] << 16) + (retVal[14] << 8) + retVal[15]);
                uint k = b ^ c ^ d;
                uint temp = a;
                uint tempA = a;
                a = ((((((~(a >> 16)) << 16) | (a & 0xFFFF))) * (~(((~(b >> 16)) << 16) | (b & 0xFFFF)))) ^ (((((~(a >> 16)) << 16) | (a & 0xFFFF)) + 8191) * ((~(((~(b >> 16)) << 16) | (b & 0xFFFF))) + 8191))) ^ k;
                k = temp ^ a;
                temp = b;
                b = ((((((~(b >> 16)) << 16) | (b & 0xFFFF))) * (~(((~(c >> 16)) << 16) | (c & 0xFFFF)))) ^ (((((~(b >> 16)) << 16) | (b & 0xFFFF)) + 8191) * ((~(((~(c >> 16)) << 16) | (c & 0xFFFF))) + 8191))) ^ k;
                k = temp ^ b;
                temp = c;
                c = ((((((~(c >> 16)) << 16) | (c & 0xFFFF))) * (~(((~(d >> 16)) << 16) | (d & 0xFFFF)))) ^ (((((~(c >> 16)) << 16) | (c & 0xFFFF)) + 8191) * ((~(((~(d >> 16)) << 16) | (d & 0xFFFF))) + 8191))) ^ k;
                k = temp ^ c;
                d = ((((((~(d >> 16)) << 16) | (d & 0xFFFF))) * (~(((~(tempA >> 16)) << 16) | (tempA & 0xFFFF)))) ^ (((((~(d >> 16)) << 16) | (d & 0xFFFF)) + 8191) * ((~(((~(tempA >> 16)) << 16) | (tempA & 0xFFFF))) + 8191))) ^ k;
                retVal[1] = (byte)(a >> 24);
                retVal[2] = (byte)((a >> 16) % 256);
                retVal[3] = (byte)((a >> 8) % 256);
                retVal[4] = (byte)(a % 256);
                retVal[5] = (byte)(c >> 24);
                retVal[6] = (byte)((c >> 16) % 256);
                retVal[7] = (byte)((c >> 8) % 256);
                retVal[8] = (byte)(c % 256);
                retVal[9] = (byte)(b >> 24);
                retVal[10] = (byte)((b >> 16) % 256);
                retVal[11] = (byte)((b >> 8) % 256);
                retVal[12] = (byte)(b % 256);
                retVal[13] = (byte)(d >> 24);
                retVal[14] = (byte)((d >> 16) % 256);
                retVal[15] = (byte)((d >> 8) % 256);
                retVal[0] = (byte)(d % 256);
            }

            return retVal;
        }

        /// <summary>Encrypts a message using SCEDA.</summary>
        /// <param name="input">The binary representation of the message to encrypt.</param>
        /// <param name="key">The key to be used. It must be 16 bytes long.</param>
        /// <param name="initializationVector">The initialization vector to be used (it must be provided and should not always be the same, but it is not required to be a secret). It must be 8 bytes long.</param>
        /// <returns>The binary encrypted message.</returns>
        public static byte[] Encrypt(byte[] input, byte[] key, byte[] initializationVector)
        {
            byte[] retVal = new byte[((input.Length + 31) / 16) * 16];

            int length = ((input.Length + 15) / 16) * 16;
            int temp = input.Length;
            int index = 6;
            for (int i = 0; i < 7; i++)
            {
                retVal[index--] = (byte)(temp % 256);
                temp /= 256;
            }
            index += 8;
            Array.Copy(input, 0, retVal, index, input.Length);
            index += input.Length;
            for (int i = 0; i < length - input.Length + 9; i++)
            {
                retVal[index++] = (byte)SCEDA.r.Next(255);
            }
            index = 7;
            Array.Copy(SCEDA.ScedaFunc(retVal.Skip(index).ToArray(), key, retVal.Skip(index + length).ToArray(), false), 0, retVal, index, ((input.Length + 15) / 16) * 16);
            length += 16;
            for (int i = 0; i < length / 2; i++)
            {
                temp = retVal[i];
                retVal[i] = retVal[length - i - 1];
                retVal[length - i - 1] = (byte)temp;
            }
            retVal = SCEDA.ScedaFunc(retVal, key, initializationVector, false);

            return retVal;
        }

        /// <summary>Decrypts a message encrypted with SCEDA.</summary>
        /// <param name="input">The binary representation of the encrypted message.</param>
        /// <param name="key">The key used to encrypt the message (it must be 16 bytes long).</param>
        /// <param name="initializationVector">The initialization vector used to encrypt the message (it must be 8 bytes long).</param>
        /// <returns>The binary uncrypted message.</returns>
        /// <exception cref="ScedaException">Thrown when the length of the key or the initialization vector is wrong.</exception>
        /// <exception cref="ScedaDecryptingException">
        /// Thrown when the message cannot be decrypted (usually because the key or the initialization vector is wrong).
        /// Please notice: it might not always be possible to detect when the key or the initialization vector are wrong and if they are but no exception is thrown then a wrong message is returned.
        /// </exception>
        public static byte[] Decrypt(byte[] input, byte[] key, byte[] initializationVector)
        {
            byte[] retVal = SCEDA.ScedaFunc(input, key, initializationVector, true);

            if (key.Length != 16 || initializationVector.Length != 8)
            {
                throw new ScedaException("The key must be 16 bytes long and the initialization vector must be 8 bytes long!");
            }
            if ((input.Length % 16) != 0)
            {
                throw new ScedaDecryptingException("The length of a message encrypted with SCEDA must always be a multiple of 16 bytes!");
            }
            for (int i = 0; i < input.Length / 2; i++)
            {
                byte temp = retVal[i];
                retVal[i] = retVal[input.Length - i - 1];
                retVal[input.Length - i - 1] = temp;
            }
            int index = 0;
            int length = 0;
            for (int i = 0; i < 7; i++)
            {
                length *= 256;
                length += retVal[index++];
            }
            if (length > input.Length - 16)
            {
                throw new ScedaDecryptingException();
            }
            Array.Copy(SCEDA.ScedaFunc(retVal.Skip(7).ToArray(), key, retVal.Skip(((length + 15) / 16) * 16 + 7).ToArray(), true), 0, retVal, 0, length);

            return retVal.Take(length).ToArray();
        }

        /// <summary>Applies the ScedaDigest hashing algorithm.</summary>
        /// <param name="input">The array to be hashed.</param>
        /// <returns>The result of the ScedaDigest algorithm (16 bytes long).</returns>
        public static byte[] Digest(byte[] input)
        {
            byte[] retVal = new byte[16];

            int blockC = (input.Length + 17) / 16;
            byte[] buffer = new byte[blockC * 16];
            Array.Copy(input, buffer, input.Length);
            buffer[input.Length] = (byte)((input.Length / 256) % 256);
            buffer[input.Length + 1] = (byte)(input.Length % 256);
            for (int i = input.Length + 2; i < blockC * 16; i++)
            {
                buffer[i] = 170;
            }
            for (int i = 0; i < blockC; i++)
            {
                Array.Copy(SCEDA.DigestFunc(buffer.Skip(i * 16).Take(16).ToArray()), 0, buffer, i * 16, 16);
            }
            Array.Copy(buffer, retVal, 16);
            byte[] temp1 = new byte[16];
            byte[] temp2 = new byte[16];
            for (int i = 1; i < blockC; i++)
            {
                Array.Copy(retVal, temp1, 8);
                Array.Copy(buffer, 16 * i, temp1, 8, 8);
                Array.Copy(buffer, 16 * i + 8, temp2, 0, 8);
                Array.Copy(retVal, 8, temp2, 8, 8);
                temp1 = SCEDA.DigestFunc(temp1);
                temp2 = SCEDA.DigestFunc(temp2);
                for (int j = 0; j < 16; j++)
                {
                    retVal[j] = (byte)(temp1[j] ^ temp2[j]);
                }
            }

            return retVal;
        }

        /// <summary>Generates a random key eligible to be used to encrypt messages with SCEDA.</summary>
        /// <returns>The 16 bytes long random-generated key.</returns>
        public static byte[] GenerateKey()
        {
            byte[] retVal = new byte[16];
            SCEDA.r.NextBytes(retVal);
            return retVal;
        }

        /// <summary>Generates a random initialization vector eligible to be used to encrypt messages with SCEDA (it doesn't have to be a secret).</summary>
        /// <returns>The 8 bytes long random-generated IV.</returns>
        public static byte[] GenerateIV()
        {
            byte[] retVal = new byte[8];
            SCEDA.r.NextBytes(retVal);
            return retVal;
        }
    }
}
