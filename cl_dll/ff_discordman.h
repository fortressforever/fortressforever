// ff_discordman.h

typedef struct DiscordRichPresence {
    const char* state;   /* max 128 bytes */
    const char* details; /* max 128 bytes */
    unsigned long long startTimestamp; // type modified from stdint
    unsigned long long endTimestamp; // type modified from stdint
    const char* largeImageKey;  /* max 32 bytes */
    const char* largeImageText; /* max 128 bytes */
    const char* smallImageKey;  /* max 32 bytes */
    const char* smallImageText; /* max 128 bytes */
    const char* partyId;        /* max 128 bytes */
    unsigned int partySize;
    unsigned int partyMax;
    const char* matchSecret;    /* max 128 bytes */
    const char* joinSecret;     /* max 128 bytes */
    const char* spectateSecret; /* max 128 bytes */
    unsigned short instance; // type modified from stdint
} DiscordRichPresence;

typedef struct DiscordJoinRequest {
    const char* userId;
    const char* username;
    const char* discriminator;
    const char* avatar;
} DiscordJoinRequest;

typedef struct DiscordEventHandlers {
    void (*ready)();
    void (*disconnected)(int errorCode, const char* message);
    void (*errored)(int errorCode, const char* message);
    void (*joinGame)(const char* joinSecret);
    void (*spectateGame)(const char* spectateSecret);
    void (*joinRequest)(const DiscordJoinRequest* request);
} DiscordEventHandlers;


class CFFDiscordManager
{
public:
	CFFDiscordManager();
	~CFFDiscordManager();
	void RunFrame();
	void Init();
	void LevelInit(const char *szMapname);
	void Reset();
	// these have to be static so that discord can use them
	// as callbacks :-(
	static void OnReady();
	static void OnDiscordError(int errorCode, const char *szMessage);

private:
	void InitializeDiscord();
	bool NeedToUpdate();

	void UpdateRichPresence();
	void UpdatePlayerInfo();
	void UpdateNetworkInfo();

	char m_szLatchedMapname[MAX_MAP_NAME];
	bool m_bApiReady;
	bool m_bErrored;
	bool m_bInitializeRequested;
	float m_flLastUpdatedTime;
	DiscordRichPresence m_sDiscordRichPresence;
};

extern CFFDiscordManager _discord;
