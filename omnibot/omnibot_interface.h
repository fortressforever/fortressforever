#ifndef __OMNIBOT_INTERFACE_H__
#define __OMNIBOT_INTERFACE_H__

class CFFInfoScript;

namespace Omnibot
{
	#include "Omni-Bot.h"
	#include "FF_Config.h"

	class omnibot_interface
	{
	public:

		static void OnDLLInit();
		static void OnDLLShutdown();

		static void LevelInit();
		static bool InitBotInterface();
		static void ShutdownBotInterface();
		static void UpdateBotInterface();
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
	void Notify_GameEnded(int _winningteam);	

	void Notify_ChatMsg(CBasePlayer *_player, const char *_msg);
	void Notify_TeamChatMsg(CBasePlayer *_player, const char *_msg);
	void Notify_Spectated(CBasePlayer *_player, CBasePlayer *_spectated);

	void Notify_ClientConnected(CBasePlayer *_player, bool _isbot, int _team = RANDOM_TEAM_IF_NO_TEAM, int _class = RANDOM_CLASS_IF_NO_CLASS);
	void Notify_ClientDisConnected(CBasePlayer *_player);

	void Notify_AddWeapon(CBasePlayer *_player, const char *_item);
	void Notify_RemoveWeapon(CBasePlayer *_player, const char *_item);
	void Notify_RemoveAllItems(CBasePlayer *_player);	

	void Notify_Hurt(CBasePlayer *_player, CBaseEntity *_attacker);
	void Notify_Death(CBasePlayer *_player, CBaseEntity *_attacker, const char *_weapon);
	void Notify_KilledSomeone(CBasePlayer *_player, CBaseEntity *_victim, const char *_weapon);

	void Notify_ChangedTeam(CBasePlayer *_player, int _newteam);
	void Notify_ChangedClass(CBasePlayer *_player, int _oldclass, int _newclass);

	void Notify_Build_MustBeOnGround(CBasePlayer *_player, int _buildable);
	void Notify_Build_CantBuild(CBasePlayer *_player, int _buildable);
	void Notify_Build_AlreadyBuilt(CBasePlayer *_player, int _buildable);
	void Notify_Build_NotEnoughAmmo(CBasePlayer *_player, int _buildable);
	void Notify_Build_BuildCancelled(CBasePlayer *_player, int _buildable);

	void Notify_CantDisguiseAsTeam(CBasePlayer *_player, int _disguiseTeam);
	void Notify_CantDisguiseAsClass(CBasePlayer *_player, int _disguiseClass);
	void Notify_Disguising(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass);
	void Notify_Disguised(CBasePlayer *_player, int _disguiseTeam, int _disguiseClass);
	void Notify_DisguiseLost(CBasePlayer *_player);
	void Notify_UnCloaked(CBasePlayer *_player);
	void Notify_CantCloak(CBasePlayer *_player);
	void Notify_Cloaked(CBasePlayer *_player);
	
	void Notify_RadarDetectedEnemy(CBasePlayer *_player, CBaseEntity *_ent);
	void Notify_RadioTagUpdate(CBasePlayer *_player, CBaseEntity *_ent);
	void Notify_BuildableDamaged(CBasePlayer *_player, int _type, CBaseEntity *_buildableEnt);

	void Notify_DispenserBuilding(CBasePlayer *_player, CBaseEntity *_buildEnt);
	void Notify_DispenserBuilt(CBasePlayer *_player, CBaseEntity *_buildEnt);
	void Notify_DispenserEnemyUsed(CBasePlayer *_player, CBaseEntity *_enemyUser);
	void Notify_DispenserDestroyed(CBasePlayer *_player, CBaseEntity *_attacker);
	void Notify_DispenserDetonated(CBasePlayer *_player);
	void Notify_DispenserDismantled(CBasePlayer *_player);

	void Notify_SentryUpgraded(CBasePlayer *_player, int _level);
	void Notify_SentryBuilding(CBasePlayer *_player, CBaseEntity *_buildEnt);
	void Notify_SentryBuilt(CBasePlayer *_player, CBaseEntity *_buildEnt);
	void Notify_SentryDestroyed(CBasePlayer *_player, CBaseEntity *_attacker);
	void Notify_SentryBuildCancel(CBasePlayer *_player);
	void Notify_SentryDetonated(CBasePlayer *_player);
	void Notify_SentryDismantled(CBasePlayer *_player);
	void Notify_SentrySpottedEnemy(CBasePlayer *_player);
	void Notify_SentryAimed(CBasePlayer *_player, CBaseEntity *_buildEnt, const Vector &_dir);

	void Notify_DetpackBuilding(CBasePlayer *_player, CBaseEntity *_buildEnt);
	void Notify_DetpackBuilt(CBasePlayer *_player, CBaseEntity *_buildEnt);
	void Notify_DispenserBuildCancel(CBasePlayer *_player);
	void Notify_DetpackDetonated(CBasePlayer *_player);

	void Notify_DispenserSabotaged(CBasePlayer *_player, CBaseEntity *_saboteur);
	void Notify_SentrySabotaged(CBasePlayer *_player, CBaseEntity *_saboteur);	

	void Notify_PlayerShoot(CBasePlayer *_player, int _weapon, CBaseEntity *_projectile);
	void Notify_PlayerUsed(CBasePlayer *_player, CBaseEntity *_entityUsed);

	void Notify_GotSpannerArmor(CBasePlayer *_target, CBasePlayer *_engy, int _before, int _after);
	void Notify_GaveSpannerArmor(CBasePlayer *_engy, CBasePlayer *_target, int _before, int _after);
	void Notify_GotMedicHealth(CBasePlayer *_target, CBasePlayer *_medic, int _before, int _after);
	void Notify_GaveMedicHealth(CBasePlayer *_medic, CBasePlayer *_target, int _before, int _after);

	void Notify_GotDispenserAmmo(CBasePlayer *_player);

	// Goal Stuff
	enum BotGoalTypes
	{
		kNone,
		kBackPack_Ammo,
		kBackPack_Armor,
		kBackPack_Health,
		kBackPack_Grenades,
		kFlag,
		kFlagCap,
		kTrainerSpawn
	};
	void Notify_GoalInfo(CBaseEntity *_entity, int _type, int _teamflags);

	void Notify_ItemRemove(CBaseEntity *_entity);
	void Notify_ItemRestore(CBaseEntity *_entity);
	void Notify_ItemDropped(CBaseEntity *_entity);
	void Notify_ItemPickedUp(CBaseEntity *_entity, CBaseEntity *_whodoneit);
	void Notify_ItemRespawned(CBaseEntity *_entity);
	void Notify_ItemReturned(CBaseEntity *_entity);

	void Notify_FireOutput(const char *_entityname, const char *_output);

	void BotSendTriggerEx(const char *_entityname, const char *_action);
	void SendBotSignal(const char *_signal);

	void SpawnBotAsync(const char *_name, int _team, int _class, CFFInfoScript *_spawnpoint = 0);
	void SpawnBot(const char *_name, int _team, int _class, CFFInfoScript *_spawnpoint = 0);
};

#endif
