#ifndef __OMNIBOT_INTERFACE_H__
#define __OMNIBOT_INTERFACE_H__

namespace Omnibot
{
	#include "Omni-Bot.h"
	#include "FF_Config.h"

	class omnibot_interface
	{
	public:

		static void OnDLLInit();
		static void OnDLLShutdown();

		static bool InitBotInterface();
		static void ShutdownBotInterface();
		static void UpdateBotInterface();
		static void Bot_Interface_LogOutput(const char *_txt);
		static void Bot_SendSoundEvent(int _client, int _sndtype, Omnibot::GameEntity _source);
		static void Bot_Interface_SendEvent(int _eid, int _dest, int _source, int _msdelay, BotUserData * _data);
		static void Bot_Interface_SendGlobalEvent(int _eid, int _source, int _msdelay, BotUserData * _data);
		static void Bot_SendTrigger(TriggerInfo *_triggerInfo);
		static void OmnibotCommand();

	private:

	};

	int obUtilGetWeaponId(const char *_weaponName);

	const int obUtilGetBotClassFromGameClass(int _class);
};

#endif
