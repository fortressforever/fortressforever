
#ifndef FF_ESP_SHARED_H
#define FF_ESP_SHARED_H

struct ESP_Shared_s
{
	int		m_iTeam;
	int		m_iClass;
	Vector	m_vecOrigin;

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
		if( ( m_iTeam == hRhs.m_iTeam ) ||
			( m_iClass == hRhs.m_iClass ) ||
			( m_vecOrigin == hRhs.m_vecOrigin ))
			return true;
		
		return false;
	}

	ESP_Shared_s& operator=( const ESP_Shared_s& hRhs )
	{
		m_iTeam = hRhs.m_iTeam;
		m_iClass = hRhs.m_iClass;
		m_vecOrigin = hRhs.m_vecOrigin;

		return *this;
	}
};

#endif
