using Valve.Steamworks;

namespace SteamworksServer
{
    internal class SteamworksManager
    {
        ISteamUserStats _steamUserStats;
        ISteamUser      _steamUser;
        bool            _isInitialized;

        private bool IsReady
        {
            get { return _isInitialized && _steamUserStats != null && _steamUser != null; }
        }

        // valves provided wrapper is barbaric at best so a few simplifications of basic stats and achievements handling
        public void Initialize()
        {
            SteamAPI.Init(253530);//0);
            _steamUserStats = SteamAPI.SteamUserStats();
            _steamUser = SteamAPI.SteamUser();
            _isInitialized = _steamUser != null && _steamUser.BLoggedOn();
        }

        public void Shutdown()
        {
            _isInitialized = false;
            if (_steamUserStats != null)
                _steamUserStats.StoreStats();

            _steamUserStats = null;
            _steamUser = null;
            SteamAPI.Shutdown();
        }

        public void UnlockAchievement(string achievementName, bool immediatelyStore = true)
        {
            if (!IsReady)
                return;
            _steamUserStats.SetAchievement(achievementName);
            if (immediatelyStore)
                _steamUserStats.StoreStats();
        }

        public void SetStat(string statName, int val, bool immediatelyStore = true)
        {
            if (!IsReady)
                return;
            _steamUserStats.SetStat(statName, val);
            if (immediatelyStore)
                _steamUserStats.StoreStats();
        }

        public int GetStat(string statName)
        {
            var ret = 0;
            if (_steamUserStats.GetStat(statName, ref ret))
                return ret;
            return -1;
        }

        public void IncrementStat(string statName, int incrAmount)
        {
            if (!IsReady)
                return;
            int curVal = 0;
            if (_steamUserStats.GetStat(statName, ref curVal))
            {
                int newVal = curVal + incrAmount;
                _steamUserStats.SetStat(statName, newVal);
                _steamUserStats.StoreStats();
            }
        }

        public void LoadAllStats()
        {
            if (!IsReady)
                return;
            //SteamAPI.RegisterCallback();
            _steamUserStats.RequestCurrentStats();
        }

        public void ResetAllStats(bool includeAcheesies)
        {
            if (!IsReady)
                return;
            _steamUserStats.ResetAllStats(includeAcheesies);
        }
    }
}
