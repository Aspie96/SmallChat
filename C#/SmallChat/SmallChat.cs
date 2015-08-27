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
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Net.NetworkInformation;
using System.Timers;

namespace SmallChat
{
    /// <summary>Provides information about a communication session on a host.</summary>
    public struct SCHostInfo
    {
        private IPEndPoint ipNport;
        private string nickname;
        private string chatID;

        /// <summary>The IP address of the host.</summary>
        public IPAddress Ip
        {
            get { return this.ipNport.Address; }
            set
            {
                if (ipNport == null)
                {
                    this.ipNport = new IPEndPoint(value, 0);
                }
                else
                {
                    this.ipNport.Address = value;
                }
            }
        }

        /// <summary>Indicates the port the host is using for communicating (the port and the chat ID identify a channel).</summary>
        public int Port
        {
            get { return this.ipNport.Port; }
            set
            {
                if (ipNport == null)
                {
                    this.ipNport = new IPEndPoint(0, value);
                }
                else
                {
                    this.ipNport.Port = value;
                }
            }
        }

        /// <summary>Indicates the nickname associated with the IP of the host.</summary>
        public string Nickname
        {
            get { return this.nickname; }
            set { this.nickname = value; }
        }

        /// <summary>Indicates the chat ID (the port and the chat ID identify a channel).</summary>
        public string ChatID
        {
            get { return this.chatID; }
            set { this.chatID = value; }
        }

        /// <summary>Initializes a new instance of the <see cref="SCHostInfo"/> structure.</summary>
        /// <param name="ip">The IP address associated with the host.</param>
        /// <param name="port">The port the host is using (it must always be the same for the same channel).</param>
        /// <param name="nickname">The nickname associated with the IP address of the host.</param>
        /// <param name="chatID">The chat ID (it must always be the same for the same channel).</param>
        public SCHostInfo(IPAddress ip, int port, string nickname, string chatID)
        {
            this.ipNport = new IPEndPoint(ip, port);
            this.nickname = nickname;
            this.chatID = chatID;
        }
    }

    /// <summary>Specify the type of a SmallChat PDU.</summary>
    public enum SCPduType
    {
        /// <summary>
        /// Hello ("HLO") PDUs are used to discover other hosts to communicate with.
        /// They are requests for a Welcome PDU from all other hosts and they contain the nickname to be associated with the sender's IP.
        /// </summary>
        Hello,

        /// <summary>
        /// Welcome ("ACK") PDUs are sent as a response to Hello PDUs.
        /// They contain the nickname to be associated with the sender's IP.
        /// </summary>
        Welcome,

        /// <summary>
        /// Leave ("LEV") PDUs are sent by hosts which are leaving the communication.
        /// They have no content.
        /// </summary>
        Leave,

        /// <summary>Message ("MSG") PDUs are used to send ordinary chat messages to other hosts.</summary>
        Message,

        /// <summary>
        /// Malformed PDU notifications ("BAD") are sent as a response to broken PDUs and PDUs which have been encrypted with the wrong key.
        /// They contain the content of the received PDU (not re-encrypted).
        /// </summary>
        MalformedPduNotification,

        /// <summary>
        /// Nickname conflict notifications ("CNF") are sent when the IPs of two different hosts are associated with the same nickname to the involved hosts.
        /// They contain the dot representation of the IP address of the host the receiver is in conflict with (or "0.0.0.0" if the receiver is in conflict with the sender).
        /// </summary>
        NicknameConflictNotification
    }

    /// <summary>The exception that is thrown by <see cref="SCPdu.FromBinary(byte[], key)"/> when a PDU cannot be decrypted and parsed.</summary>
    public class MalformedPduException : Exception
    {
        private const string defaultMessage = "The given SmallChat PDU is malformed!";

        /// <summary>
        /// Initializes a new instance of the <see cref="MalformedPduException"/> class.
        /// A default message will be used.
        /// </summary>
        public MalformedPduException() : base(MalformedPduException.defaultMessage) { }

        /// <summary>Initializes a new instance of the <see cref="MalformedPduException"/> class.</summary>
        /// <param name="message">The message to be associated with the exception.</param>
        public MalformedPduException(string message) : base(message) { }

        /// <summary>Initializes a new instance of the <see cref="MalformedPduException"/> class.</summary>
        /// <param name="message">The message to be associated with the exception.</param>
        /// <param name="inner">An inner exception.</param>
        public MalformedPduException(string message, Exception inner) : base(message, inner) { }

        /// <summary>
        /// Initializes a new instance of the <see cref="MalformedPduException"/> class.
        /// A default message will be used.
        /// </summary>
        /// <param name="inner">An inner exception.</param>
        public MalformedPduException(Exception inner) : base(MalformedPduException.defaultMessage, inner) { }
    }

    /// <summary>Represents a SmallChat PDU.</summary>
    public struct SCPdu
    {
        private string chatID;
        private SCPduType type;
        private Encoding encoding;
        private byte[] payload;

        /// <summary>Get or sets the field representing the chat ID.</summary>
        public string ChatID
        {
            get { return this.chatID; }
            set { this.chatID = value; }
        }

        /// <summary>Gets or sets the field representing the type of this PDU.</summary>
        public SCPduType Type
        {
            get { return this.type; }
            set { this.type = value; }
        }

        /// <summary>Gets or sets the character encoding used in the payload of this PDU.</summary>
        public Encoding Encoding
        {
            get { return this.encoding; }
            set { this.encoding = value; }
        }

        /// <summary>Gets or sets the non-encrypted binary payload of this PDU (it must be calculated accordingly to the <see cref="SCPdu.Type"/> property.</summary>
        public byte[] Payload
        {
            get { return this.payload; }
            set { this.payload = value; }
        }

        /// <summary>Initializes a new instance of the <see cref="SCPdu"/> structure.</summary>
        /// <param name="chatID">The field representing the chat ID.</param>
        /// <param name="type">The field representing the PDU type.</param>
        /// <param name="encoding">The field representing the character encoding used in the payload of this PDU.</param>
        /// <param name="payload">The field representing the non-encrypted binary content of this PDU (calculated accordingly to the encoding).</param>
        public SCPdu(string chatID, SCPduType type, Encoding encoding, byte[] payload)
        {
            this.chatID = chatID;
            this.type = type;
            this.payload = payload;
            this.encoding = encoding;
        }

        /// <summary>This factory method decrypt a binary PDU and convert it into an instance of the <see cref="SCPdu"/> structure.</summary>
        /// <param name="pdu">The binary content of the PDU.</param>
        /// <param name="key">The encryption key used for encrypting the content of the PDU.</param>
        /// <returns>The parsed PDU.</returns>
        /// <exception cref="MalformedPduException">Thrown when the PDU is broken or the given encryption key is wrong.</exception>
        public static SCPdu FromBinary(byte[] pdu, byte[] key)
        {
            SCPdu retVal = new SCPdu();
            if (key.Length != 16)
            {
                throw new ScedaException("A SCEDA key must be 16 bytes long.");
            }
            if (pdu.Length < 12)
            {
                throw new MalformedPduException("The PDU is too short.");
            }
            if (pdu[0] != 0 || pdu[1] != 1)
            {
                throw new MalformedPduException("Unknown ScedaDigest version.");
            }
            try
            {
                retVal.ChatID = Encoding.ASCII.GetString(pdu.Skip(2).TakeWhile(item => item != 0).ToArray());
                int displacement = retVal.ChatID.Length + 3;
                byte[] iv = pdu.Skip(displacement).Take(8).ToArray();
                displacement += 8;
                byte[] msg = SCEDA.Decrypt(pdu.Skip(displacement).ToArray(), key, iv);
                displacement = 0;
                string type = Encoding.ASCII.GetString(msg.Take(3).ToArray());
                displacement += 3;
                switch (type)
                {
                    case "HLO":
                        retVal.Type = SCPduType.Hello;
                        break;
                    case "ACK":
                        retVal.Type = SCPduType.Welcome;
                        break;
                    case "LEV":
                        retVal.Type = SCPduType.Leave;
                        break;
                    case "MSG":
                        retVal.Type = SCPduType.Message;
                        break;
                    case "BAD":
                        retVal.Type = SCPduType.MalformedPduNotification;
                        break;
                    case "CNF":
                        retVal.Type = SCPduType.NicknameConflictNotification;
                        break;
                    default:
                        throw new Exception("PDU Type not known.");
                }
                string encoding = Encoding.ASCII.GetString(msg.Skip(displacement).TakeWhile(item => item != 0).ToArray());
                retVal.Encoding = Encoding.GetEncoding(encoding);
                displacement += encoding.Length + 1;
                retVal.Payload = msg.Skip(displacement).ToArray();
                if ((retVal.Type == SCPduType.Hello || retVal.Type == SCPduType.Welcome) && retVal.Payload.Length == 0)
                {
                    throw new Exception("Hello and Welcome PDUs must have a non-empty payload.");
                }
            }
            catch (Exception e)
            {
                throw new MalformedPduException(e);
            }
            return retVal;
        }

        /// <summary>Encrypts this PDU and provides its binary representation.</summary>
        /// <param name="key">The encryption key to be used.</param>
        /// <returns>The binary representation of the PDU to be sent through the network.</returns>
        public byte[] ToBinary(byte[] key)
        {
            if (key.Length != 16)
            {
                throw new ScedaException("A SCEDA key must be 16 bytes long.");
            }
            string type;
            switch (this.Type)
            {
                case SCPduType.Hello:
                    type = "HLO";
                    break;
                case SCPduType.Welcome:
                    type = "ACK";
                    break;
                case SCPduType.Leave:
                    type = "LEV";
                    break;
                case SCPduType.Message:
                    type = "MSG";
                    break;
                case SCPduType.MalformedPduNotification:
                    type = "BAD";
                    break;
                case SCPduType.NicknameConflictNotification:
                    type = "CNF";
                    break;
                default:
                    type = "";
                    break;
            }
            byte[] iv = SCEDA.GenerateIV();
            return (new byte[] { 0, 1 }).Concat(Encoding.ASCII.GetBytes(this.ChatID + '\0')).Concat(iv).Concat(SCEDA.Encrypt(Encoding.ASCII.GetBytes(type).Concat(Encoding.ASCII.GetBytes(this.Encoding.BodyName + '\0')).Concat(this.Payload).ToArray(), key, iv)).ToArray();
        }

        /// <summary>Checks if the given PDU is associated with the given chat ID without trying to decrypt or parse it.</summary>
        /// <param name="pdu">The binary representation of the PDU to be checked.</param>
        /// <param name="chatID">The chat ID to be checked in the PDU.</param>
        /// <returns><code>true</code> if the PDU is associated with the given chat ID, <code>false</code> otherwise.</returns>
        public static bool CheckChatID(byte[] pdu, string chatID)
        {
            return Encoding.ASCII.GetString(pdu.Skip(2).TakeWhile(item => item != 0).ToArray()) == chatID;
        }
    }

    /// <summary>Represents a local SmallChat client.</summary>
    public class SCHost : IDisposable
    {
        private readonly SCHostInfo myself;
        private readonly byte[] key;
        private readonly IPAddress broadcast;
        private List<SCHostInfo> others;
        private UdpClient socket;
        private Thread listener;
        private bool disposed = false;
        private int remainingMalformedNotifications;
        private System.Timers.Timer malformedTimer;

        /// <summary>Represents the method that will handle the <see cref="SCHost.OnReceive"/> event.</summary>
        /// <param name="info">Information about the sender.</param>
        /// <param name="pdu">The received message PDU.</param>
        /// <param name="message">The clear message contained in the PDU.</param>
        public delegate void OnReceiveEventHandler(SCHostInfo info, SCPdu pdu, string message);

        /// <summary>Occurs when a valid message PDU is received.</summary>
        public event OnReceiveEventHandler OnReceive;

        /// <summary>Represents the method that will handle the <see cref="SCHost.OnHello"/> or <see cref="SCHost.OnWelcome"/> events.</summary>
        /// <param name="info">Information about the added host.</param>
        public delegate void OnAddEventHandler(SCHostInfo info);

        /// <summary>Occurs when a hello PDU from an unknown host is received.</summary>
        public event OnAddEventHandler OnHello;

        /// <summary>Occurs when a welcome PDU from an unknown host is received.</summary>
        public event OnAddEventHandler OnWelcome;

        /// <summary>Represents the method that will handle the <see cref="SCHost.OnLeave"/> event.</summary>
        /// <param name="info">Information about the host who left the conversation.</param>
        public delegate void OnLeaveEventHandler(SCHostInfo info);

        /// <summary>Occurs when a known or unknown host sends a leave PDU.</summary>
        public event OnLeaveEventHandler OnLeave;

        /// <summary>Represents the method that will handle the <see cref="SCHost.OnMalformedNotification"/> or <see cref="SCHost.OnMalformedReceived"/> events.</summary>
        /// <param name="info">Information about the interlocutor.</param>
        /// <param name="pdu">The original encrypted content of the malformed PDU.</param>
        public delegate void OnMalformedEvetHandler(SCHostInfo info, byte[] pdu);

        /// <summary>Occurs when a malformed PDU notification is received.</summary>
        public event OnMalformedEvetHandler OnMalformedNotification;

        /// <summary>Occurs when a malformed PDU is received.</summary>
        public event OnMalformedEvetHandler OnMalformedReceived;

        /// <summary>Represents the method that will handle the <see cref="OnConflictNotification"/> event.</summary>
        /// <param name="informerInfo">Information about the host who reported the conflict.</param>
        /// <param name="rivalInfo">Information about the host with the same nickname as this one.</param>
        public delegate void OnConflictNotificationEvetHandler(SCHostInfo informerInfo, SCHostInfo rivalInfo);

        /// <summary>Occurs when a nickname conflict notification is received.</summary>
        public event OnConflictNotificationEvetHandler OnConflictNotification;

        /// <summary>Initializes a new instance of the <see cref="SCHost"/> class.</summary>
        /// <param name="nickname">The nickname to be associated with this host.</param>
        /// <param name="chatID">The chat ID of the communication this host will take part into.</param>
        /// <param name="key">The encryption key used in this communication.</param>
        /// <param name="broadcast">The broadcast address of the local network.</param>
        /// <param name="port">The port to be used in this communication.</param>
        /// <param name="firstHello">If <code>true</code>, makes the host send a broadcast hello PDU once initialized.</param>
        public SCHost(string nickname, string chatID, byte[] key, IPAddress broadcast = null, int port = 4412, bool firstHello = true)
        {
            if (key.Length != 16)
            {
                throw new ScedaException("A SCEDA key must be 16 bytes long.");
            }
            this.myself = new SCHostInfo();
            this.myself.Nickname = nickname;
            this.myself.ChatID = chatID;
            this.myself.Port = port;
            this.key = key;
            this.others = new List<SCHostInfo>();
            this.socket = new UdpClient(this.myself.Port);
            this.socket.Client.EnableBroadcast = true;
            this.myself.Ip = IPAddress.Loopback;
            if (broadcast == null)
            {
                this.broadcast = IPAddress.Broadcast;
            }
            else
            {
                this.broadcast = broadcast;
            }
            this.disposed = false;
            this.remainingMalformedNotifications = 4;
            this.malformedTimer = new System.Timers.Timer(600000);
            this.malformedTimer.Elapsed += delegate
            {
                this.remainingMalformedNotifications = 4;
            };
            this.listener = new Thread(delegate()
            {
                for (; ; )
                {
                    IPEndPoint endPoint = new IPEndPoint(IPAddress.Any, this.myself.Port);
                    byte[] binaryPdu = this.socket.Receive(ref endPoint);
                    bool local = endPoint.Address.Equals(IPAddress.Loopback);
                    foreach (IPAddress item in Dns.GetHostAddresses(Dns.GetHostName()))
                    {
                        if (endPoint.Address.Equals(item))
                        {
                            local = true;
                        }
                    }
                    if (!local && SCPdu.CheckChatID(binaryPdu, this.myself.ChatID))
                    {
                        SCHostInfo info = new SCHostInfo(endPoint.Address, endPoint.Port, this.GetNicknameFromIp(endPoint.Address), this.myself.ChatID);
                        try
                        {
                            SCPdu pdu = SCPdu.FromBinary(binaryPdu, this.key);
                            switch (pdu.Type)
                            {
                                case SCPduType.Hello:
                                    {
                                        info.Nickname = pdu.Encoding.GetString(pdu.Payload);
                                        this.ManualSend(endPoint.Address, new SCPdu(this.myself.ChatID, SCPduType.Welcome, Encoding.Default, Encoding.Default.GetBytes(this.myself.Nickname)));
                                        if (this.Add(info, true) && this.OnHello != null)
                                        {
                                            this.OnHello(info);
                                        }
                                        break;
                                    }

                                case SCPduType.Welcome:
                                    {
                                        info.Nickname = pdu.Encoding.GetString(pdu.Payload);
                                        if (this.Add(info, true) && this.OnWelcome != null)
                                        {
                                            this.OnWelcome(info);
                                        }
                                        break;
                                    }

                                case SCPduType.Leave:
                                    {
                                        this.others.RemoveAll(item => item.Ip.Equals(info.Ip));
                                        if (this.OnLeave != null)
                                        {
                                            this.OnLeave(info);
                                        }
                                        break;
                                    }
                                case SCPduType.Message:
                                    {
                                        if (this.OnReceive != null)
                                        {
                                            this.OnReceive(info, pdu, pdu.Encoding.GetString(pdu.Payload));
                                        }
                                        break;
                                    }
                                case SCPduType.MalformedPduNotification:
                                    {
                                        if (this.OnMalformedNotification != null)
                                        {
                                            this.OnMalformedNotification(info, pdu.Payload);
                                        }
                                        break;
                                    }
                                case SCPduType.NicknameConflictNotification:
                                    {
                                        if (this.OnConflictNotification != null)
                                        {
                                            this.OnConflictNotification(info, new SCHostInfo(IPAddress.Parse(pdu.Encoding.GetString(pdu.Payload)), this.myself.Port, this.myself.Nickname, this.myself.ChatID));
                                        }
                                        break;
                                    }
                            }
                        }
                        catch (MalformedPduException)
                        {
                            this.remainingMalformedNotifications--;
                            if (this.remainingMalformedNotifications > -1)
                            {
                                this.ManualSend(endPoint.Address, new SCPdu(this.myself.ChatID, SCPduType.Hello, Encoding.Default, new byte[0]));
                                this.malformedTimer.Start();
                            }
                            if (this.OnMalformedReceived != null)
                            {
                                this.OnMalformedReceived(info, binaryPdu);
                            }
                        }
                    }
                }
            });
            this.listener.Start();
            if (firstHello)
            {
                this.Hello();
            }
        }

        /// <summary>
        /// Add a host to the list of known hosts to communicate with and raise the <see cref="SCHost.OnAdd"/> event.
        /// If there is already a host with the same IP address, then its nickname will be updated, no host will actually added and the <see cref="SCHost.OnAdd"/> event will not be raised.
        /// </summary>
        /// <param name="info">Information about the host to be added.</param>
        /// <param name="notifyConflict">If <code>true</code> detects nickname conflicts if any (raising the <see cref="SCHost.OnConflictNotification"/> and sending a conflict notification PDU to the other host).</param>
        /// <returns><code>true</code> if a host has actually been added, <code>false</code> if a nickname has been updated.</returns>
        public bool Add(SCHostInfo info, bool notifyConflict = true)
        {
            bool alreadyInserted = false;
            for (int i = 0; i < this.others.Count; i++)
            {
                if (this.others[i].Ip.Equals(info.Ip))
                {
                    alreadyInserted = true;
                    this.others[i] = info;
                }
                else if (notifyConflict && this.others[i].Nickname == info.Nickname)
                {
                    SCPdu notification = new SCPdu(this.myself.ChatID, SCPduType.NicknameConflictNotification, Encoding.Default, Encoding.Default.GetBytes(info.Ip.ToString()));
                    this.ManualSend(this.others[i].Ip, notification);
                    notification.Payload = Encoding.Default.GetBytes(this.others[i].Ip.ToString());
                    this.ManualSend(info.Ip, notification);
                }
            }
            if (notifyConflict && info.Nickname == this.myself.Nickname)
            {
                SCPdu notification = new SCPdu(this.myself.ChatID, SCPduType.NicknameConflictNotification, Encoding.Default, Encoding.Default.GetBytes(IPAddress.Any.ToString()));
                this.ManualSend(info.Ip, notification);
                if (this.OnConflictNotification != null)
                {
                    this.OnConflictNotification(this.myself, info);
                }
            }
            if (!alreadyInserted)
            {
                this.others.Add(info);
            }
            return !alreadyInserted;
        }

        /// <summary>Gets the nickname associated with the given IP address.</summary>
        /// <param name="ip">The IP address of the host associated with the wanted nickname.</param>
        /// <returns>The found nickname or <see cref="string.Empty"/> if the given IP address is unknown.</returns>
        public string GetNicknameFromIp(IPAddress ip)
        {
            foreach (SCHostInfo item in this.others)
            {
                if (item.Ip.Equals(ip))
                {
                    return item.Nickname;
                }
            }
            return "";
        }

        /// <summary>Sends a leave PDU to all known hosts.</summary>
        protected virtual void Leave()
        {
            SCPdu pdu = new SCPdu(this.myself.ChatID, SCPduType.Leave, Encoding.Default, new byte[0]);
            foreach (SCHostInfo item in this.others)
            {
                this.ManualSend(item.Ip, pdu);
            }
        }

        /// <summary>Sends a broadcast hello PDU.</summary>
        public void Hello()
        {
            this.others.Clear();
            this.Hello(this.broadcast);
        }

        /// <summary>Sends a unicast hello PDU to the given address.</summary>
        /// <param name="ip">The IP address of the host to be sent the PDU.</param>
        public void Hello(IPAddress ip)
        {
            this.ManualSend(ip, new SCPdu(this.myself.ChatID, SCPduType.Hello, Encoding.Default, Encoding.Default.GetBytes(this.myself.Nickname)));
        }

        /// <summary>Gets all the hosts this host is communicating with.</summary>
        /// <returns>A copy of the array of known hosts.</returns>
        public SCHostInfo[] GetOthers()
        {
            return this.others.ToArray();
        }

        /// <summary>Sends a unicast message PDU to the given host.</summary>
        /// <param name="ip">The IP address of the host to be sent the message.</param>
        /// <param name="message">The message to be sent.</param>
        public void UnicastSend(IPAddress ip, string message)
        {
            this.ManualSend(ip, new SCPdu(this.myself.ChatID, SCPduType.Message, Encoding.Default, Encoding.Default.GetBytes(message)));
        }

        /// <summary>Sends a unicast message PDU to all known hosts.</summary>
        /// <param name="message">The message to be sent.</param>
        public void Send(string message)
        {
            foreach (SCHostInfo item in this.others)
            {
                this.UnicastSend(item.Ip, message);
            }
        }

        /// <summary>Sends a broadcast message PDU.</summary>
        /// <param name="message">The message to be sent.</param>
        public void SpartanSend(string message)
        {
            this.UnicastSend(this.broadcast, message);
        }

        /// <summary>Sends a unicast PDU to the given host.</summary>
        /// <param name="ip">The IP of the host to be sent the PDU.</param>
        /// <param name="pdu">The PDU to be sent.</param>
        public void ManualSend(IPAddress ip, SCPdu pdu)
        {
            byte[] binaryPdu = pdu.ToBinary(this.key);
            socket.Send(binaryPdu, binaryPdu.Length, new IPEndPoint(ip, this.myself.Port));
        }

        /// <summary>Makes this instance of the <see cref="SCHost"/> class stop receiving and sending messages after calling the <see cref="SCHost.Leave()"/> method and than clears the list of known hosts and other resources.</summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>Makes this instance of the <see cref="SCHost"/> class stop receiving and sending messages after calling the <see cref="SCHost.Leave()"/> method.</summary>
        /// <param name="disposing">If <code>true</code> clears the list of known hosts and other resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposed)
            {
                this.Leave();
                this.listener.Abort();
                if (disposing)
                {
                    this.others.Clear();
                    this.socket.Close();
                }
                this.disposed = true;
            }
        }

        ~SCHost()
        {
            this.Dispose(false);
        }
    }
}
