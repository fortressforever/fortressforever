
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

};
