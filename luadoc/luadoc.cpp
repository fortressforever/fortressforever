
/////////////////////////////////////////////////////////////////////////////
class BaseEntity
{
public:
	/** 
	 * Emits a sound a sound from the entity.
	 * \param sound	the path to the sound to play
	 */
	void EmitSound(string sound);

	/**
	 * Returns the name of the entity.
	 * \return the name of the entity.
	 */
	string GetName();

	CTeam* GetTeam();
	int GetTeamId();
	bool IsDespenser();
	bool IsGrenade();
	bool IsPlayer();
	bool IsSentryGun();
	bool IsDetpack();
	void SetModel(string model);
	void SetModel(sring model, int skin);
	void SetSkin(int skin);
	Vector GetOrigin();
	void SetOrigin(Vector position);
};

/////////////////////////////////////////////////////////////////////////////
class InfoScript
{
public:
	void Drop(float delay, float speed);
	void Pickup(Player player);
	void Respawn(float delay);
	BaseEntity Return();
};

/////////////////////////////////////////////////////////////////////////////
class TeamBase
{
public:
	/**
	 * Adds points to the team.
	 * \param points	number of points to give to the team
	 */
	void AddScore(int points);
	
	/**
	 * Gets the number of players on the team.
	 * \return	number of players on the team
	 */
	int GetNumPlayers();

	/**
	 * Gets a player on the team.
	 * \param index	index of the player on the team. ranges from [0,NumPlayers)
	 * \return	player at index, or nil if index does not specify a valid player
	 */
	BasePlayer* GetPlayer(int index);

	/**
	 * Gets the unique id of the team
	 * \return unique id of the team
	 */
	int GetTeamId();

	/**
	 * Sets the the name of the team
	 * \param name new name for the team
	 */
	void SetName(string name);
};

/////////////////////////////////////////////////////////////////////////////
class Team
{
public:
	/**
	 * Sets the team's allied
	 * \param teamId	id of team to be allied to
	 */
	void SetAllies(int teamId);

	/**
	 * Sets the maximum number of slots for a given player class
	 * \param classId	id of class
	 * \param limit		maximum number of player for a given class on the team
	 */
	void SetClassLimit(Player::ClassId classId, int limit);

	/**
	 * Sets the maximum number of players allowed to join the team
	 * \param maxNumPlayers		maximum number of players allowed on the team
	 */
	void SetTeamLimit(int maxNumPlayers);

	enum TeamId
	{
		kUnassigned,
		kSpectator,
		kBlue,
		kRed,
		kYellow,
		kGreen,
	}
};

/////////////////////////////////////////////////////////////////////////////
class BasePlayer : public BaseEntity
{
public:
};

/////////////////////////////////////////////////////////////////////////////
class Player : public BasePlayer
{
public:
	enum ClassId
	{
		kScout,
		kSniper,
		kSoldier,
		kDemoman,
		kMedic,
		kHwguy,
		kPyro,
		kSpy,
		kEngineer,
		kCivilian,
	}

public:
	int AddAmmo(string ammoType, int amount);
	int AddArmor(int amount);
	void AddFrags(int amount);
	int AddHealth(int amount);
	ClassId GetClass();
	string GetName();
	bool HasItem(string itemName);
	bool IsFeetDeepInWater();
	bool IsInNoBuild();
	bool IsUnderWater();
	bool IsWaistDeepInWater();
	void MarkRadioTag(Player whoTaggedMe, float startTime, float duration);
	void RemoveAmmo(int amount, string ammoType);
	void RemoveArmor(int amount);
	void RemoveLocation(int entidx);
	void Respawn();
	void SetDisguisable();
	void SetLocation(int entidx, string locationText, Team.TeamId teamId);
	void SetRespawnDelay(int delay);
	void InstaSwitch(Player.ClassId classId);
	BaseEntity GiveWeapon(string weaponName, int subType);
	bool RemoveWeapon(string weaponName);
	void RemoveAllWeapons(bool bRemoveSuit);
};

/////////////////////////////////////////////////////////////////////////////
class Vector
{
public:
	float DistTo(Vector vec);
	float DistToSq(Vector vec);
	float Dot(Vector vec);
	float Length();
	void Normalize();

public:
	float x;
	float y;
	float z;
};

/////////////////////////////////////////////////////////////////////////////
namespace ffmod
{
	void BroadCastMessage(string message);
	void BroadCastMessageToPlayer(Player player, string message);
	void BroadCastSound(string sound);
	void BroadCastSoundToPlayer(Player player, string sound);
	Player CastToPlayer(BaseEntity entity);
	InfoScript CastToInfoScript(BaseEntity entity);
	BaseEntity GetEntity(int item_id);
	BaseEntity GetEntityByName(string name);
//	def("GetEntitiesByName",		&FFLib::GetEntitiesByName,			return_stl_iterator),
//	def("GetEntitiesInSphere",		&FFLib::GetEntitiesInSphere,		return_stl_iterator),
	InfoScript GetInfoScriptById(int item_id);
	InfoScript GetInfoScriptByName(string name);
	Player GetPlayer(player_id);
	Team GetTeam(Team.TeamId teamId);
	bool AreTeamsAllied(Team teamA, Team teamB);
	bool AreTeamsAllied(Team.TeamId teamA, Team.TeamId teamB);
	void IncludeScript(string script);
	void ConsoleToAll(string message);
	int NumPlayers();
	void PrecacheModel(string model);
	void PrecacheSound(string sound);
	float RandomFloat(float min, float max);
	int RandomInt(int min, int max);
	void RemoveEntity(BaseEntity entity);
	void RespawnAllPlayers();
	void SetGlobalRespawnDelay(float delay);
	void SetPlayerLimit(Team.TeamId teamId, int maxPlayers);
//	def("SetPlayerLimits",			&FFLib::SetPlayerLimits),
	void SetClassLimits(Team.TeamId teamId, Player.ClassId classId, int limit);
//	def("SetTeamClassLimit",		&FFLib::SetTeamClassLimit),
	void SetTeamName(Team.TeamId teamId, string name);
	void SmartMessage(int player_id, string playerMsg, string teamMsg, string otherMsg);
	void SmartSound(int player_id, string playerSound, string teamSound, string otherSound);
	void SmartTeamMessage(Team.TeamId teamId, string teamMsg, string otherMsg);
	void SmartTeamSound(Team.TeamId teamId, string teamSound, string otherSound);
}
