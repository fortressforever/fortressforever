// ff_discordman.h

#ifndef FF_DISCORDMAN_H
#define FF_DISCORDMAN_H

#include <igameevents.h>
#include <windows.h>

#define DISCORD_FIELD_SIZE 128

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


class CFFDiscordManager : public IGameEventListener2
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

	// IGameEventListener interface:
	virtual void FireGameEvent( IGameEvent *event);

private:
	void InitializeDiscord();
	bool NeedToUpdate();

	void UpdateRichPresence();
	void UpdatePlayerInfo();
	void UpdateNetworkInfo();
	void SetLogo();

	bool m_bApiReady;
	bool m_bErrored;
	bool m_bInitializeRequested;
	float m_flLastUpdatedTime;
	DiscordRichPresence m_sDiscordRichPresence;

	// scratch buffers to send in api struct. they need to persist
	// for a short duration after api call it seemed, it must be async
	// using a stack allocated would occassionally corrupt
	char m_szServerInfo[DISCORD_FIELD_SIZE];
	char m_szDetails[DISCORD_FIELD_SIZE];
	char m_szLatchedHostname[255];
	char m_szLatchedMapname[MAX_MAP_NAME];
	HINSTANCE m_hDiscordDLL;
};

extern CFFDiscordManager _discord;

#endif // FF_DISCORDMAN_H
