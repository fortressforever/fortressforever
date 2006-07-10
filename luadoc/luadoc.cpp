
/////////////////////////////////////////////////////////////////////////////
class BaseEntity
{
public:
	/** 
	* Emits a sound a sound from the entity.
	* @param sound	the path to the sound to play
	*/
	void EmitSound(string sound);

	/**
	* Gets the name of the entity.
	*/
	string GetName();

	/**
	* Gets the team this entity is in.
	*/
	CTeam* GetTeam();

	/*
	* Get the id of the team this entity is in.
	*/
	int GetTeamId();

	/**
	* Returns true if the entity is a dispenser object, otherwise it returns false.
	*/
	bool IsDispenser();

	/**
	* Returns true if the entity is a grenade, otherwise it returns false.
	*/
	bool IsGrenade();

	/**
	* Returns true if the entity is a player, otherwise it return false.
	*/
	bool IsPlayer();

	/**
	* Returns true if the entity is a sentry gun, otherwise it return false.
	*/
	bool IsSentryGun();

	/**
	* Returns true if the entity is a detpack, otherwise it return false.
	*/
	bool IsDetpack();

	/**
	* Sets the model for entity to display. Be sure to precache the model
	* before calling this function.
	* @param model	path of the model resource
	*/
	void SetModel(string model);

	/**
	* Sets the model and skin for the entity to display. Be sure to
	* precache the model before calling this function.
	* @param model	path of the model resource
	* @param skin	index of the skin to use as the model's texture
	*/
	void SetModel(sring model, int skin);

	/**
	* Sets the skin for the entity to display on its model.
	* @param skin	index of the skin to use as the model's texture
	*/
	void SetSkin(int skin);

	/**
	* Returns the position of the entity.
	*/
	Vector GetOrigin();

	/**
	* Sets the position of the entity.
	* @param position	new position for the entity
	*/
	void SetOrigin(Vector position);

	/**
	* Returns true if an object is on fire
	*/
	bool IsOnFire();
};

/////////////////////////////////////////////////////////////////////////////
class InfoScript : public BaseEntity
{
public:
	/**
	* Forces the object or player carrying this item to drop it.
	* @param delay	number of seconds before the item returns
	* @param speed	an additional velocity coefficent
	*/
	void Drop(float delay, float speed);

	/**
	* Tells the item that the specified player has picked it up.
	* @param player		the player that picked up the item
	*/
	void Pickup(Player player);

	/**
	* Forces the item to respawn.
	* @param delay	number of seconds to delay it should respawn
	*/
	void Respawn(float delay);

	/**
	* Forces the item to return.
	*/
	BaseEntity Return();
};

/////////////////////////////////////////////////////////////////////////////
class BaseTeam
{
public:
	/**
	* Adds points to the team.
	* @param points	number of points to give to the team
	*/
	void AddScore(int points);
	
	/**
	* Gets the number of players on the team.
	* @return	number of players on the team
	*/
	int GetNumPlayers();

	/**
	* Gets a player on the team.
	* @param index	index of the player on the team. ranges from [0,NumPlayers)
	* @return	player at index, or nil if index does not specify a valid player
	*/
	BasePlayer* GetPlayer(int index);

	/**
	* Gets the unique id of the team
	* @return unique id of the team
	*/
	int GetTeamId();

	/**
	* Sets the the name of the team
	* @param name new name for the team
	*/
	void SetName(string name);
};

/////////////////////////////////////////////////////////////////////////////
class Team : public BaseTeam
{
public:
	/**
	* Sets the team's allied
	* @param teamId	id of team to be allied to
	*/
	void SetAllies(int teamId);

	/**
	* Sets the maximum number of slots for a given player class
	* @param classId	id of class
	* @param limit		maximum number of player for a given class on the team
	*/
	void SetClassLimit(Player::ClassId classId, int limit);

	/**
	* Sets the maximum number of players allowed to join the team
	* @param maxNumPlayers		maximum number of players allowed on the team
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
	/**
	* Gives the player some ammo.
	* @param ammoType	name of the ammo to give the player
	* @param amount		amount of ammo to give the player
	* @return the total amount of ammo given to the player
	*/
	int AddAmmo(string ammoType, int amount);

	/**
	* Gives the player some armor.
	* @param amount		amount of armor to give the plaeyr
	* @return the total amount of armor given to the player
	*/
	int AddArmor(int amount);

	/**
	* Adds to the player's frag/score count
	* @param amount		number of points to give the player
	*/
	void AddFrags(int amount);

	/**
	* Gives the player some health.
	* @param amount		amount of health to give the player
	* @return total amount of health given to the player
	*/
	int AddHealth(int amount);

	/**
	* Gets the player's current class
	* @return current class
	*/
	ClassId GetClass();

	/**
	* Gets the name of the player.
	* @return name of the player
	*/
	string GetName();

	/**
	* Checks if the player is carrying an item.
	* @param itemName	name of the item to check
	* @return returns true if the player has the item, otherwise returns false
	*/
	bool HasItem(string itemName);

	/**
	* Returns true if the player's feet are in water, otherwise returns false
	*/
	bool IsFeetDeepInWater();

	/**
	* Returns true if the player is in a no-build area, otherwise returns false
	*/
	bool IsInNoBuild();

	/**
	* Returns true if the player is under water, otherwise returns false
	*/
	bool IsUnderWater();

	/**
	* Returns true if the player's waist is in water, otherwise returns false
	*/
	bool IsWaistDeepInWater();

	/**
	*
	*/
	void MarkRadioTag(Player whoTaggedMe, float startTime, float duration);

	/**
	* Takes ammo away from the player.
	* @param amount		amount of ammo to take away
	* @param ammoType	name of ammo type to take away
	*/
	void RemoveAmmo(int amount, string ammoType);

	/**
	* Takes away armor from the player.
	* @param amount		amount of armor to take away from the player.
	*/
	void RemoveArmor(int amount);

	/**
	*
	*/
	void RemoveLocation(int entidx);

	/**
	* Forces the player to respawn.
	*/
	void Respawn();

	/**
	* Sets whether the player is allowed to use the spy's disguise ability.
	* @param bDisguisable	can the player use a disguise
	*/
	void SetDisguisable(bool bDisguisable);

	/**
	*
	*/
	void SetLocation(int entidx, string locationText, Team.TeamId teamId);

	/**
	* Sets the time delay for respawns.
	* @param delay	number of seconds before the player can respawn
	*/
	void SetRespawnDelay(int delay);

	/**
	* Force the player to immediately swith to a specific class.
	* @param classId	the class the player to swith the player to
	*/
	void InstaSwitch(Player.ClassId classId);

	/**
	* Gives a weapon to the player.
	* @param weaponName		name of the weapon to give the player
	*/
	BaseEntity GiveWeapon(string weaponName, int subType);

	/**
	* Takes a weapon away from the player
	* @param weaponName		name of the weapon to take away from the player
	*/
	bool RemoveWeapon(string weaponName);

	/**
	* Takes away all of the player's weapons
	* @param bRemoveSuit	take the suit away too?
	*/
	void RemoveAllWeapons(bool bRemoveSuit);

	/**
	/* Returns true if a player is disguised
	*/
	bool IsDisguised();

	/** 
	/* Returns true if a player is feigned
	*/
	bool IsFeigned();

	/** 
	/* Returns the team the player is disguised as
	*/
	int GetDisguisedTeam();

	/** 
	/* Returns the class the player is disguised as
	*/
	int GetDisguisedTeam();

	/**
	/* Returns true if a player is ducking
	*/
	bool IsDucking();

	/** 
	/* Returns true if a player is on the ground
	*/
	bool IsOnGround();

	/** 
	/* Returns true if a player is not on the ground
	*/
	bool IsInAir();

	/** 
	/* Returns true if a player is jumping
	*/
	bool IsJumping();

	/** 
	/* Returns true if a player pressing their +use key
	*/
	bool IsUsing();

	/** 
	/* Returns true if a player pressing their +attack key
	*/
	bool IsAttacking1();

	/** 
	/* Returns true if a player pressing their +attack2 key
	*/
	bool IsAttacking2();
};

/////////////////////////////////////////////////////////////////////////////
class Vector
{
public:
	/**
	* Compute the distance from one vector to another.
	*/
	float DistTo(Vector vec);

	/**
	* Computes the distance squared from one vector to another
	*/
	float DistToSq(Vector vec);

	/**
	* Computes the angle between two vectors
	*/
	float Dot(Vector vec);

	/**
	* Computes the magnitude of the vector. This returns the distance from vector
	* to the origin (0,0,0) of the world.
	*/
	float Length();

	/**
	* Normalizes to a unit vector.
	*/
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
	float GetServerTime();
	bool AreTeamsAllied(Team teamA, Team teamB);
	bool AreTeamsAllied(Team.TeamId teamA, Team.TeamId teamB);
	void IncludeScript(string script);
	bool IsDetpack();
	bool IsDispenser();
	bool IsGrenade();
	bool IsPlayer();
	bool IsSentrygun();	
	void ConsoleToAll(string message);
	int NumPlayers();
	void PrecacheModel(string model);
	void PrecacheSound(string sound);
	float RandomFloat(float min, float max);
	int RandomInt(int min, int max);
	void RemoveEntity(BaseEntity entity);
	void RespawnAllPlayers();
	void KillAndRespawnAllPlayers();
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
