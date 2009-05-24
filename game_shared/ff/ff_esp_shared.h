
#ifndef FF_ESP_SHARED_H
#define FF_ESP_SHARED_H

struct ESP_Shared_s
{
	int		m_iEntIndex;
	int		m_iTeam;
	int		m_iClass;
	bool	m_bDucked;
	Vector	m_vecOrigin;
	bool	m_bActive;
	bool	m_bUpdated;

	// To interp stuff
	Vector	m_vecVel;

	ESP_Shared_s( void )
	{
	}

	~ESP_Shared_s( void )
	{
	}

	ESP_Shared_s( const ESP_Shared_s& hRhs )
	{
		*this = hRhs;
	}

	bool operator!=( const ESP_Shared_s& hRhs ) const
	{
		if( ( m_iEntIndex == hRhs.m_iEntIndex ) ||
			( m_iTeam == hRhs.m_iTeam ) ||
			( m_iClass == hRhs.m_iClass ) ||
			( m_vecOrigin == hRhs.m_vecOrigin ) ||
			( m_bDucked == hRhs.m_bDucked ) ||
			( m_vecVel == hRhs.m_vecVel ) ||
			( m_bActive == hRhs.m_bActive ) ||
			( m_bUpdated == hRhs.m_bUpdated ))
			return true;
		
		return false;
	}

	ESP_Shared_s& operator=( const ESP_Shared_s& hRhs )
	{
		m_iEntIndex = hRhs.m_iEntIndex;
		m_iTeam = hRhs.m_iTeam;
		m_iClass = hRhs.m_iClass;
		m_vecOrigin = hRhs.m_vecOrigin;
		m_bDucked = hRhs.m_bDucked;
		m_vecVel = hRhs.m_vecVel;
		m_bActive = hRhs.m_bActive;
		m_bUpdated = hRhs.m_bUpdated;

		return *this;
	}
};

#endif
