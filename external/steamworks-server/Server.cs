using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace SteamworksServer
{
    internal class Server
    {
        public enum Command
        {
            Unknown = 0,
            Heartbeat = 1,
            SetStat = 10,
            IncrementStat = 20,
            UnlockAchievement = 30,
            ResetAll = 99,
        }

        private class Message
        {
            public Command Command { get; private set; }
            public string Key { get; private set; }
            public string Value { get; private set; }
            public bool IsValid { get; private set; }

            public Message(string raw)
            {
                if (string.IsNullOrWhiteSpace(raw))
                    return;

                var chunks = raw.Split(kvpSep);
                if (chunks.Length != 3)
                    return;

                var rawCmd = chunks[0];
                var key = chunks[1].Trim();
                var val = chunks[2].Trim().TrimEnd(delim);
#if DEBUG
                // local testing with putty
                rawCmd = rawCmd.Replace(Environment.NewLine, string.Empty);
                Key = key.Replace(Environment.NewLine, string.Empty);
                Value = val.Replace(Environment.NewLine, string.Empty);
#endif
                int cmd;
                if (!int.TryParse(rawCmd, out cmd))
                    return;
                var command = (Command)cmd;
                if (command == Command.Unknown)
                    return;
                Command = command;
                IsValid = true;
            }
        }

        // <cmd>|<key>|<value>!
        private const char delim = '!';
        private const char kvpSep = '|';

#if DEBUG
        // for manual testing with eg putty, kinda annoying
        private const int           _timeoutMs = 60 * 1000;
#else
        private const int           _timeoutMs = 5 * 1000;
#endif
        //private DateTime            _lastHeartbeat;
        private SteamworksManager   _steamworksManager;
        private Socket              _socket;
        public int Port { get; set; }

        public Server()
        {
            Port = 7802;
        }

        public void Start()
        {
            //NOTE: synchronous, because we really should only have one localhost client ever

            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp) { NoDelay = true, ReceiveTimeout = _timeoutMs };

            try
            {
                var localEndPoint = new IPEndPoint(IPAddress.Any, Port);
                _socket.Bind(localEndPoint);
                _socket.Listen(1);
                var clientSocket = _socket.Accept(); // this will block
                ClientLoop(clientSocket);
            }
            catch (SocketException exp)
            {
                Console.WriteLine(exp);
            }
            finally 
            {
                if (_steamworksManager != null)
                    _steamworksManager.Shutdown();
            }
        }

        public void Stop()
        {
            try 
            {
                if (_socket == null) 
                    return;
                // this will cause any client loop to exception & fallthrough
                _socket.Shutdown(SocketShutdown.Both);
                _socket.Close();
            }
            catch (SocketException)
            {}
        }

        private void ClientLoop(Socket clientSocket)
        {
            var sb = new StringBuilder();
            while (true)
            {
                var buf = new byte[] { 0 };
                while (true)
                {
                    var bytesRead = clientSocket.Receive(buf);
                    if (bytesRead < 1)
                        return;
                    var nextChar = (char)buf[0];
                    sb.Append((char)buf[0]);
                    if (nextChar == delim)
                        break;
                }
                HandleMessage(new Message(sb.ToString()));
                sb.Clear();
            }
        }

        private void HandleMessage(Message msg)
        {
            if (msg == null || !msg.IsValid)
                return;

            if (_steamworksManager == null)
            {
                _steamworksManager = new SteamworksManager();
                _steamworksManager.Initialize();
            }

            switch (msg.Command)
            {
                case Command.Heartbeat:
                    //_lastHeartbeat = DateTime.UtcNow;
                    // used to use heartbeat system, but now we just let socket time out and close
                    break;
                case Command.IncrementStat:
                    int incrementAmount;
                    if (int.TryParse(msg.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out incrementAmount))
                        _steamworksManager.IncrementStat(msg.Key, incrementAmount);
                    break;
                case Command.ResetAll:
                    // TODO: consider using key for including achievements?
                    _steamworksManager.ResetAllStats(true);
                    break;
                case Command.SetStat:
                    int newVal;
                    if (int.TryParse(msg.Value, NumberStyles.Integer, CultureInfo.InvariantCulture, out newVal))
                        _steamworksManager.SetStat(msg.Key, newVal);
                    break;
                case Command.UnlockAchievement:
                    // TODO: consider value for immediate store
                    _steamworksManager.UnlockAchievement(msg.Key);
                    break;
            }
        }
    }
}
