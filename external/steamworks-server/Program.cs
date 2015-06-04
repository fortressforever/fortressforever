using System;

namespace SteamworksServer
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
           new Server().Start();
        }
    }
}
