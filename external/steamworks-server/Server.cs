using System;
using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace SteamworksServer
{
    internal class Server
    {
        public enum Command
        {
            Unknown             = 0,
            Heartbeat           = 1,
            SetStat             = 10,
            IncrementStat       = 20,
            UnlockAchievement   = 30,
            Quit                = 44,
            ResetAll            = 99,
        }

        private class Message
        {
            private readonly string _raw;

            public Command  Command     { get; private set; }
            public string   Key         { get; private set; }
            public string   Value       { get; private set; }
            public bool     IsValid     { get; private set; }

            public Message(string raw)
            {
                _raw = raw;
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

            public override string ToString()
            {
                return _raw;
            }
        }

        // <cmd>|<key>|<value>!
        private const char delim    = '!';
        private const char kvpSep   = '|';

        // heartbeat is 30 seconds
        private const int           _timeoutMs = 15 * 1000;

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

            _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp) { NoDelay = true, ReceiveTimeout = _timeoutMs, LingerState = new LingerOption(false, 0) };

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
                Debug.WriteLine("[Steamworks Server] Start1: {0}", exp);
                _socket.Close();
            }
            finally 
            {
                if (_steamworksManager != null)
                    _steamworksManager.Shutdown();
            }
        }

        public void Stop()
        {
            Debug.WriteLine("[Steamworks Server] Stop");
            if (_socket == null)
                return;
            // this will cause any client loop to exception & fallthrough
            _socket.Shutdown(SocketShutdown.Both);
            _socket.Close();
        }

        private void ClientLoop(Socket clientSocket)
        {
            Debug.WriteLine("[Steamworks Server] ClientLoop0");
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
            {
                 Debug.WriteLine("[Steamworks Server] HandleMessage0");
                return;
            }
            
            if (_steamworksManager == null)
            {
                Debug.WriteLine("[Steamworks Server] starting steamworks mgr");
                _steamworksManager = new SteamworksManager();
                _steamworksManager.Initialize();
                Debug.WriteLine("[Steamworks Server] steamworks mgr started");
            }

            Debug.WriteLine("[Steamworks Server] HandleMessage1: '{0}'", msg);
            switch (msg.Command)
            {
                case Command.Heartbeat:
                     Debug.WriteLine("[Steamworks Server] Heartbeat");
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
                case Command.Quit:
                    Debug.WriteLine("[Steamworks Server] Quit");
                    Stop();
                    // will cause loop to fall through
                    break;
            }
        }
    }
}
