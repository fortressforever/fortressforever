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
		static void Bot_SendSoundEvent(int _client, int _sndtype, Omnibot::GameEntity _source);
		static void Bot_Interface_SendEvent(int _eid, int _dest, int _source, int _msdelay, BotUserData * _data);
		static void Bot_Interface_SendGlobalEvent(int _eid, int _source, int _msdelay, BotUserData * _data);
		static void Bot_SendTrigger(TriggerInfo *_triggerInfo);
		static void OmnibotCommand();

	private:

	};

	int obUtilGetWeaponId(const char *_weaponName);

	const int obUtilGetBotClassFromGameClass(int _class);
	const int obUtilGetBotTeamFromGameTeam(int _team);

	const int obUtilGetBotWeaponFromGameWeapon(int _gameWpn);
	const int obUtilGetGameWeaponFromBotWeapon(int _botWpn);

	// Message Helpers
	void Notify_GameStarted();

	void Notify_ChatMsg(CBasePlayer *_player, const char *_msg);
	void Notify_TeamChatMsg(CBasePlayer *_player, const char *_msg);

	void Notify_ClientConnected(CBasePlayer *_player, bool _isbot);
	void Notify_ClientDisConnected(CBasePlayer *_player);

	void Notify_Spawned(CBasePlayer *_player);
	void Notify_Hurt(CBasePlayer *_player, edict_t *_attacker);
	void Notify_Death(CBasePlayer *_player, edict_t *_attacker, const char *_weapon);

	void Notify_ChangedTeam(CBasePlayer *_player, int _newteam);
	void Notify_ChangedClass(CBasePlayer *_player, int _oldclass, int _newclass);

	void Notify_Build_MustBeOnGround(CBasePlayer *_player, int _buildable);
	void Notify_Build_CantBuild(CBasePlayer *_player, int _buildable);
	void Notify_Build_AlreadyBuilt(CBasePlayer *_player, int _buildable);
	void Notify_Build_NotEnoughAmmo(CBasePlayer *_player, int _buildable);

	void Notify_Disguising(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass);
	void Notify_Disguised(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass);
	void Notify_DisguiseLost(CBasePlayer *_player);
	void Notify_UnFeigned(CBasePlayer *_player);
	void Notify_CantFeign(CBasePlayer *_player);
	void Notify_Feigned(CBasePlayer *_player);
	
	void Notify_RadarDetectedEnemy(CBasePlayer *_player, edict_t *_ent);
	void Notify_BuildableDamaged(CBasePlayer *_player, int _type, edict_t *_buildableEnt);

	void Notify_DispenserBuilding(CBasePlayer *_player, edict_t *_buildEnt);
	void Notify_DispenserBuilt(CBasePlayer *_player, edict_t *_buildEnt);
	void Notify_DispenserEnemyUsed(CBasePlayer *_player, edict_t *_enemyUser);
	void Notify_DispenserDestroyed(CBasePlayer *_player, edict_t *_attacker);
	void Notify_DispenserDetonated(CBasePlayer *_player);
	void Notify_DispenserDismantled(CBasePlayer *_player);

	void Notify_SentryUpgraded(CBasePlayer *_player, int _level);
	void Notify_SentryBuilding(CBasePlayer *_player, edict_t *_buildEnt);
	void Notify_SentryBuilt(CBasePlayer *_player, edict_t *_buildEnt);
	void Notify_SentryDestroyed(CBasePlayer *_player, edict_t *_attacker);
	void Notify_SentryDetonated(CBasePlayer *_player);
	void Notify_SentryDismantled(CBasePlayer *_player);
	void Notify_SentrySpottedEnemy(CBasePlayer *_player);
	void Notify_SentryAimed(CBasePlayer *_player);

	void Notify_DetpackBuilding(CBasePlayer *_player, edict_t *_buildEnt);
	void Notify_DetpackBuilt(CBasePlayer *_player, edict_t *_buildEnt);
	void Notify_DetpackDetonated(CBasePlayer *_player);

	void Notify_DispenserSabotaged(CBasePlayer *_player, edict_t *_saboteur);
	void Notify_SentrySabotaged(CBasePlayer *_player, edict_t *_saboteur);	

	void Notify_PlayerShoot(CBasePlayer *_player, int _weapon);
};

#endif
