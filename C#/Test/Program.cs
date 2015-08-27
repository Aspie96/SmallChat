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
using System.Net;
using SmallChat;

namespace Test
{
    class Program
    {
        static string ConsoleInput(string description, string defaultValue = "", bool hidden = false)
        {
            char input;
            string retVal;
            retVal = defaultValue;
            Console.Write(description + ": " + defaultValue);
            while ((input = Console.ReadKey(hidden).KeyChar) != '\r')
            {
                if (input == '\0')
                {
                    Console.Write('\b');
                }
                else
                {
                    if (input == '\b')
                    {
                        if (retVal.Length == 0)
                        {
                            Console.SetCursorPosition(0, Console.CursorTop);
                            Console.Write(description + ": ");
                        }
                        else
                        {
                            if (hidden)
                            {
                                Console.Write('\b');
                            }
                            Console.Write(" \b");
                            retVal = retVal.Remove(retVal.Length - 1);
                        }
                    }
                    else
                    {
                        if (hidden)
                        {
                            Console.Write('*');
                        }
                        retVal += input;
                    }
                }
            }
            Console.WriteLine();
            return retVal;
        }

        static void ConsoleWriteInColor(string str, ConsoleColor color)
        {
            Console.ForegroundColor = color;
            Console.Write(str);
            Console.ResetColor();
        }

        static void ConsoleInsertLine(out int x, out int y)
        {
            x = Console.CursorLeft;
            y = Console.CursorTop + 1;
            Console.MoveBufferArea(0, Console.CursorTop, Console.WindowWidth, 1, 0, y);
            Console.SetCursorPosition(0, y - 1);
        }

        static void Main(string[] args)
        {
            Console.WindowWidth = 80;
            Console.Title = "SmallChat - by Valentino Giudice";

            string chatId;
            while ((chatId = ConsoleInput("Chat ID")) == "") ;
            string nick;
            while ((nick = ConsoleInput("Nickname")) == "") ;
            string key = ConsoleInput("Key", hidden: true);
            IPAddress broadcast;
            while (!IPAddress.TryParse(ConsoleInput("Broadcast address", "255.255.255.255"), out broadcast) || broadcast.Equals(IPAddress.Any)) ;

            SCHost myself = new SCHost(nick, chatId, SCEDA.Digest(Encoding.ASCII.GetBytes(key)), broadcast);
            myself.OnReceive += myself_OnReceive;
            myself.OnWelcome += myself_OnWelcome;
            myself.OnHello += myself_OnHello;
            myself.OnLeave += myself_OnLeave;
            myself.OnConflictNotification += myself_OnConflictNotification;
            myself.OnMalformedReceived += myself_OnMalformed;
            myself.OnMalformedNotification += myself_OnMalformed;

            Console.CancelKeyPress += delegate
            {
                myself.Dispose();
            };

            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("Begin to chat!\n");
            Console.ResetColor();
            char input;
            for (; ; )
            {
                string msg = "";
                Console.ForegroundColor = ConsoleColor.Magenta;
                Console.Write(nick + ": ");
                Console.ResetColor();
                while (((input = Console.ReadKey().KeyChar) != '\r') && msg.Length + nick.Length + 2 < Console.WindowWidth - 1)
                {
                    if (input == '\0')
                    {
                        Console.Write('\b');
                    }
                    else
                    {
                        if (input == '\b')
                        {
                            if (Console.CursorLeft < nick.Length + 2)
                            {
                                Console.SetCursorPosition(0, Console.CursorTop);
                                Console.ForegroundColor = ConsoleColor.Magenta;
                                Console.Write(nick + ": ");
                                Console.ResetColor();
                            }
                            else
                            {
                                Console.Write(" \b");
                                msg = msg.Remove(msg.Length - 1);
                            }
                        }
                        else
                        {
                            msg += input;
                        }
                    }
                }
                if (msg.Length + nick.Length + 2 < Console.WindowWidth - 1)
                {
                    Console.WriteLine();
                }
                myself.Send(msg);
            }
        }


        static void myself_OnReceive(SCHostInfo info, SCPdu pdu, string message)
        {
            int left;
            int top;
            ConsoleInsertLine(out left, out top);
            if (info.Nickname == "")
            {
                ConsoleWriteInColor("unknown: ", ConsoleColor.Yellow);
            }
            else
            {
                ConsoleWriteInColor(info.Nickname + ": ", ConsoleColor.Cyan);
            }
            Console.Write(message);
            Console.SetCursorPosition(left, top);
        }

        static void myself_OnWelcome(SCHostInfo info)
        {
            int left;
            int top;
            ConsoleInsertLine(out left, out top);
            ConsoleWriteInColor(info.Nickname + " is online.", ConsoleColor.Blue);
            Console.SetCursorPosition(left, top);
        }

        static void myself_OnHello(SCHostInfo info)
        {
            int left;
            int top;
            ConsoleInsertLine(out left, out top);
            ConsoleWriteInColor(info.Nickname + " has joined the chat!", ConsoleColor.Blue);
            Console.SetCursorPosition(left, top);
        }

        static void myself_OnLeave(SCHostInfo info)
        {
            int left;
            int top;
            ConsoleInsertLine(out left, out top);
            ConsoleWriteInColor(info.Nickname + " is offline!", ConsoleColor.Green);
            Console.SetCursorPosition(left, top);
        }

        static bool conflictNotified = false;
        static void myself_OnConflictNotification(SCHostInfo informerInfo, SCHostInfo rivalInfo)
        {
            if (!conflictNotified)
            {
                conflictNotified = true;
                int left;
                int top;
                ConsoleInsertLine(out left, out top);
                ConsoleWriteInColor("Nickname conflict detected!", ConsoleColor.Red);
                Console.SetCursorPosition(left, top);
            }
        }

        private static void myself_OnMalformed(SCHostInfo info, byte[] pdu)
        {
            int left;
            int top;
            ConsoleInsertLine(out left, out top);
            ConsoleWriteInColor("Problem while communicating with " + info.Nickname, ConsoleColor.Red);
            if (info.Nickname == "")
            {
                ConsoleWriteInColor("unknown", ConsoleColor.Yellow);
            }
            ConsoleWriteInColor("!", ConsoleColor.Red);
            Console.SetCursorPosition(left, top);
        }
    }
}
