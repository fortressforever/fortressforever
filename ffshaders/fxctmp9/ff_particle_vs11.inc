class ff_particle_vs11_Static_Index
{
public:
	ff_particle_vs11_Static_Index()
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
class ff_particle_vs11_Dynamic_Index
{
private:
	int m_nFOGTYPE;
#ifdef _DEBUG
	bool m_bFOGTYPE;
#endif
public:
	void SetFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFOGTYPE = i;
#ifdef _DEBUG
		m_bFOGTYPE = true;
#endif
	}
	void SetFOGTYPE( bool i )
	{
		m_nFOGTYPE = i ? 1 : 0;
#ifdef _DEBUG
		m_bFOGTYPE = true;
#endif
	}
public:
	ff_particle_vs11_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bFOGTYPE = false;
#endif // _DEBUG
		m_nFOGTYPE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bFOGTYPE;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nFOGTYPE ) + 0;
	}
};
