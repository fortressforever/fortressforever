//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef UTLCACHEDFILEDATA_H
#define UTLCACHEDFILEDATA_H
#if defined( WIN32 )
#pragma once
#endif

#include "filesystem.h" // FileNameHandle_t
#include "utlrbtree.h"
#include "utlbuffer.h"
#include "utlvector.h"
#include "vstdlib/strtools.h"

#include "tier0/memdbgon.h"

// If you change to serialization protocols, this must be bumped...
#define UTL_CACHE_SYSTEM_VERSION		2

#define UTL_CACHED_FILE_DATA_UNDEFINED_DISKINFO	(long)-2

// Cacheable types must derive from this and implement the appropriate methods...
class IBaseCacheInfo
{
public:
	virtual void Save( CUtlBuffer& buf ) = 0;
	virtual void Restore( CUtlBuffer& buf ) = 0;

	virtual void Rebuild( char const *filename ) = 0;
};

typedef unsigned int (*PFNCOMPUTECACHEMETACHECKSUM)( void );

typedef enum
{
	UTL_CACHED_FILE_USE_TIMESTAMP = 0,
	UTL_CACHED_FILE_USE_FILESIZE,
} UtlCachedFileDataType_t;

template <class T>
class CUtlCachedFileData
{
public:
	CUtlCachedFileData
	( 
		char const *repositoryFileName, 
		int version, 
		PFNCOMPUTECACHEMETACHECKSUM checksumfunc = NULL, 
		UtlCachedFileDataType_t fileCheckType = UTL_CACHED_FILE_USE_TIMESTAMP,
		bool nevercheckdisk = false,
		bool readonly = false,
		bool savemanifest = false
	)
		: m_Elements( 0, 0, FileNameHandleLessFunc ),
		m_pszRepositoryFileName( repositoryFileName ),
		m_nVersion( version ),
		m_pfnMetaChecksum( checksumfunc ),
		m_bDirty( false ),
		m_bInitialized( false ),
		m_uCurrentMetaChecksum( 0u ),
		m_fileCheckType( fileCheckType ),
		m_bNeverCheckDisk( nevercheckdisk ),
		m_bReadOnly( readonly ),
		m_bSaveManifest( savemanifest )
	{
		Assert( m_pszRepositoryFileName && m_pszRepositoryFileName[ 0 ] );
	}

	virtual ~CUtlCachedFileData()
	{
		m_Elements.RemoveAll();
		int c = m_Data.Count();
		for ( int i = 0; i < c ; ++i )
		{
			delete m_Data[ i ];
		}
		m_Data.RemoveAll();
	}

	T* Get( char const *filename );
	const T* Get( char const *filename ) const;

	T* operator[]( int i );
	const T* operator[]( int i ) const;

	int Count() const;

	void GetElementName( int i, char *buf, int buflen )
	{
		buf[ 0 ] = 0;
		if ( !m_Elements.IsValidIndex( i ) )
			return;

		filesystem->String( m_Elements[ i ].handle, buf, buflen );
	}

	bool EntryExists( char const *filename ) const
	{
		ElementType_t element;
		element.handle = filesystem->FindOrAddFileName( filename );
		int idx = m_Elements.Find( element );
		return idx != m_Elements.InvalidIndex() ? true : false;
	}

	void SetElement( char const *name, long fileinfo, T* src )
	{
		SetDirty( true );

		int idx = GetIndex( name );

		Assert( idx != m_Elements.InvalidIndex() );

		ElementType_t& e = m_Elements[ idx ];

		CUtlBuffer buf( 0, 0, false );

		Assert( e.dataIndex != m_Data.InvalidIndex() );

		T *dest = m_Data[ e.dataIndex ];

		Assert( dest );

		// I suppose we could do an assignment operator, but this should save/restore the data element just fine for
		//  tool purposes
		((IBaseCacheInfo *)src)->Save( buf );
		((IBaseCacheInfo *)dest)->Restore( buf );

		e.fileinfo = fileinfo;
		// Force recheck
		e.diskfileinfo = UTL_CACHED_FILE_DATA_UNDEFINED_DISKINFO;
	}

	// If you create a cache and don't call init/shutdown, you can call this to do a quick check to see if the checksum/version
	//  will cause a rebuild...
	bool	IsUpToDate();

	void	Shutdown();
	bool	Init();

	void	Save();

	void	Reload();

	void	ForceRecheckDiskInfo();
	// Iterates all entries and gets filesystem info and optionally causes rebuild on any existing items which are out of date
	void	CheckDiskInfo( bool force_rebuild );

	void	SaveManifest();
	bool	ManifestExists();

	long	GetFileInfo( char const *filename )
	{
		ElementType_t element;
		element.handle = filesystem->FindOrAddFileName( filename );
		int idx = m_Elements.Find( element );
		if ( idx == m_Elements.InvalidIndex() )
		{
			return 0L;
		}

		return m_Elements[ idx ].fileinfo;
	}

	int		GetNumElements()
	{
		return m_Elements.Count();
	}

private:

	int			GetIndex( const char *filename )
	{
		ElementType_t element;
		element.handle = filesystem->FindOrAddFileName( filename );
		int idx = m_Elements.Find( element );
		if ( idx == m_Elements.InvalidIndex() )
		{
			T *data = new T();

			int dataIndex = m_Data.AddToTail( data );
			idx = m_Elements.Insert( element );
			m_Elements[ idx ].dataIndex = dataIndex;
		}

		return idx;
	}

	void		CheckInit();

	void		SetDirty( bool dirty )
	{
		m_bDirty = dirty;
	}

	bool		IsDirty() const
	{
		return m_bDirty;
	}

	void		RebuildCache( char const *filename, T *data );

	struct ElementType_t
	{
		ElementType_t() :
			handle( 0 ),
			fileinfo( 0 ),
			diskfileinfo( UTL_CACHED_FILE_DATA_UNDEFINED_DISKINFO ),
			dataIndex( -1 )
		{
		}

		FileNameHandle_t	handle;
		long				fileinfo;
		long				diskfileinfo;
		int					dataIndex;
	};

	static bool FileNameHandleLessFunc( ElementType_t const &lhs, ElementType_t const &rhs )
	{
		return lhs.handle < rhs.handle;
	}

	CUtlRBTree< ElementType_t >		m_Elements;
	CUtlVector< T * >				m_Data;
	char const						*m_pszRepositoryFileName;
	int								m_nVersion;
	bool							m_bDirty;
	bool							m_bInitialized;
	PFNCOMPUTECACHEMETACHECKSUM		m_pfnMetaChecksum;
	unsigned int					m_uCurrentMetaChecksum;
	UtlCachedFileDataType_t			m_fileCheckType;
	bool							m_bNeverCheckDisk;
	bool							m_bReadOnly;
	bool							m_bSaveManifest;
};

template <class T>
T* CUtlCachedFileData<T>::Get( char const *filename )
{
	int idx = GetIndex( filename );

	ElementType_t& e = m_Elements[ idx ];

	long cachefileinfo = e.fileinfo;
	// Set the disk fileinfo the first time we encounter the filename
	if ( e.diskfileinfo == UTL_CACHED_FILE_DATA_UNDEFINED_DISKINFO )
	{
		if ( m_bNeverCheckDisk )
		{
			e.diskfileinfo = cachefileinfo;
		}
		else
		{
			if ( m_fileCheckType == UTL_CACHED_FILE_USE_FILESIZE ) 
			{
				e.diskfileinfo = filesystem->Size( filename );
			}
			else
			{
				e.diskfileinfo = filesystem->GetFileTime( filename );
			}
		}
	}

	Assert( e.dataIndex != m_Data.InvalidIndex() );

	T *data = m_Data[ e.dataIndex ];

	Assert( data );

	// Compare fileinfo to disk fileinfo and rebuild cache if out of date or not correct...
	if ( cachefileinfo != e.diskfileinfo )
	{
		if ( !m_bReadOnly )
		{
			RebuildCache( filename, data );
		}
		e.fileinfo = e.diskfileinfo;
	}

	return data;
}

template <class T>
const T* CUtlCachedFileData<T>::Get( char const *filename ) const
{
	int idx = GetIndex( filename );

	ElementType_t& e = m_Elements[ idx ];

	long cachefileinfo = e.fileinfo;
	// Set the disk fileinfo the first time we encounter the filename
	if ( e.diskfileinfo == UTL_CACHED_FILE_DATA_UNDEFINED_DISKINFO )
	{
		if ( m_bNeverCheckDisk )
		{
			e.diskfileinfo = cachefileinfo;
		}
		else
		{
			if ( m_fileCheckType == UTL_CACHED_FILE_USE_FILESIZE ) 
			{
				e.diskfileinfo = filesystem->Size( filename );
			}
			else
			{
				e.diskfileinfo = filesystem->GetFileTime( filename );
			}
		}
	}

	Assert( e.dataIndex != m_Data.InvalidIndex() );

	T *data = m_Data[ e.dataIndex ];

	Assert( data );

	// Compare fileinfo to disk fileinfo and rebuild cache if out of date or not correct...
	if ( cachefileinfo != e.diskfileinfo )
	{
		if ( !m_bReadOnly )
		{
			RebuildCache( filename, data );
		}
		e.fileinfo = e.diskfileinfo;
	}

	return data;
}

template <class T>
T* CUtlCachedFileData<T>::operator[]( int i )
{
	return m_Data[ m_Elements[ i ].dataIndex ];
}

template <class T>
const T* CUtlCachedFileData<T>::operator[]( int i ) const
{
	return m_Data[ m_Elements[ i ].dataIndex ];
}

template <class T>
int CUtlCachedFileData<T>::Count() const
{
	return m_Elements.Count();
}

template <class T>
void CUtlCachedFileData<T>::Reload()
{
	Shutdown();
	Init();
}

template <class T>
bool CUtlCachedFileData<T>::IsUpToDate()
{
	// Don't call Init/Shutdown if using this method!!!
	Assert( !m_bInitialized );

	if ( !m_pszRepositoryFileName || !m_pszRepositoryFileName[ 0 ] )
	{
		Error( "CUtlCachedFileData:  Can't IsUpToDate, no repository file specified." );
		return false;
	}

	// Always compute meta checksum
	m_uCurrentMetaChecksum = m_pfnMetaChecksum ? (*m_pfnMetaChecksum)() : 0;

	FileHandle_t fh;

	fh = filesystem->Open( m_pszRepositoryFileName, "rb", "MOD" );
	if ( fh == FILESYSTEM_INVALID_HANDLE )
	{
		return false;
	}

	// Version data is in first 12 bytes of file
	byte	header[ 12 ];
	filesystem->Read( header, sizeof( header ), fh );
	filesystem->Close( fh );

	int cacheversion = *( int *)&header[ 0 ];

	if ( UTL_CACHE_SYSTEM_VERSION != cacheversion )
	{
		DevMsg( "Discarding repository '%s' due to cache system version change\n", m_pszRepositoryFileName );
		filesystem->RemoveFile( m_pszRepositoryFileName, "MOD" );
		return false;
	}

	// Now parse data from the buffer
	int version = *( int *)&header[ 4 ];
	if ( version != m_nVersion )
	{
		DevMsg( "Discarding repository '%s' due to version change\n", m_pszRepositoryFileName );
		filesystem->RemoveFile( m_pszRepositoryFileName, "MOD" );
		return false;
	}

	// This is a checksum based on any meta data files which the cache depends on (supplied by a passed in
	//  meta data function
	unsigned int cache_meta_checksum = (unsigned int)*( int *)&header[ 8 ];

	if ( cache_meta_checksum != m_uCurrentMetaChecksum )
	{
		DevMsg( "Discarding repository '%s' due to meta checksum change\n", m_pszRepositoryFileName );
		filesystem->RemoveFile( m_pszRepositoryFileName, "MOD" );
		return false;
	}

	// Looks valid
	return true;
}

template <class T>
bool CUtlCachedFileData<T>::Init()
{
	if ( m_bInitialized )
	{
		return true;
	}

	m_bInitialized = true;

	if ( !m_pszRepositoryFileName || !m_pszRepositoryFileName[ 0 ] )
	{
		Error( "CUtlCachedFileData:  Can't Init, no repository file specified." );
		return false;
	}

	// Always compute meta checksum
	m_uCurrentMetaChecksum = m_pfnMetaChecksum ? (*m_pfnMetaChecksum)() : 0;

	FileHandle_t fh;

	fh = filesystem->Open( m_pszRepositoryFileName, "rb", "MOD" );
	if ( fh == FILESYSTEM_INVALID_HANDLE )
	{
		// Nothing on disk, we'll recreate everything from scratch...
		SetDirty( true );
		return true;
	}

	int cacheversion = 0;
	filesystem->Read( &cacheversion, sizeof( cacheversion ), fh );

	bool deletefile = false;

	if ( UTL_CACHE_SYSTEM_VERSION == cacheversion )
	{
		// Now parse data from the buffer
		int version = 0;
		filesystem->Read( &version, sizeof( version ), fh );
		
		if ( version == m_nVersion )
		{
			// This is a checksum based on any meta data files which the cache depends on (supplied by a passed in
			//  meta data function
			unsigned int cache_meta_checksum = 0;
			
			filesystem->Read( &cache_meta_checksum, sizeof( cache_meta_checksum ), fh );

			if ( cache_meta_checksum == m_uCurrentMetaChecksum )
			{
				int count = 0;
				
				filesystem->Read( &count, sizeof( count ), fh );

				Assert( count < 2000000 );

				CUtlBuffer buf( 0, 0, false );

				for ( int i = 0 ; i < count; ++i )
				{
					int bufsize = 0;
					filesystem->Read( &bufsize, sizeof( bufsize ), fh );

					Assert( bufsize < 1000000 );

					buf.EnsureCapacity( bufsize );
					
					filesystem->Read( buf.Base(), bufsize, fh );

					buf.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );

					// Read the element name
					char elementFileName[ 512 ];
					buf.GetString( elementFileName, sizeof( elementFileName ) );

					// Now read the element
					int slot = GetIndex( elementFileName );

					Assert( slot != m_Elements.InvalidIndex() );

					ElementType_t& element = m_Elements[ slot ];

					element.fileinfo = buf.GetInt();

					Assert( element.dataIndex != m_Data.InvalidIndex() );

					T *data = m_Data[ element.dataIndex ];

					Assert( data );

					((IBaseCacheInfo *)data)->Restore( buf );
				}
			}
			else
			{
				Msg( "Discarding repository '%s' due to meta checksum change\n", m_pszRepositoryFileName );
				deletefile = true;
			}
		}
		else
		{
			Msg( "Discarding repository '%s' due to version change\n", m_pszRepositoryFileName );
			deletefile = true;
		}
	}
	else
	{
		DevMsg( "Discarding repository '%s' due to cache system version change\n", m_pszRepositoryFileName );
		deletefile = true;
	}

	filesystem->Close( fh );

	if ( deletefile )
	{
		filesystem->RemoveFile( m_pszRepositoryFileName, "MOD" );
		SetDirty( true );
	}

	return true;
}

template <class T>
void CUtlCachedFileData<T>::Save()
{
	char path[ 512 ];
	Q_strncpy( path, m_pszRepositoryFileName, sizeof( path ) );
	Q_StripFilename( path );

	filesystem->CreateDirHierarchy( path, "MOD" );

	if ( filesystem->FileExists( m_pszRepositoryFileName, "MOD" ) && 
		!filesystem->IsFileWritable( m_pszRepositoryFileName, "MOD" ) )
	{
		filesystem->SetFileWritable( m_pszRepositoryFileName, true, "MOD" );
	}

	// Now write to file
	FileHandle_t fh;
	fh = filesystem->Open( m_pszRepositoryFileName, "wb" );
	if ( FILESYSTEM_INVALID_HANDLE == fh )
	{
		Warning( "Unable to persist cache '%s', check file permissions\n", m_pszRepositoryFileName );
	}
	else
	{
		SetDirty( false );

		int v = UTL_CACHE_SYSTEM_VERSION;
		filesystem->Write( &v, sizeof( v ), fh );
		v = m_nVersion;
		filesystem->Write( &v, sizeof( v ), fh );
		v = (int)m_uCurrentMetaChecksum;
		filesystem->Write( &v, sizeof( v ), fh );

		// Element count
		int c = Count();

		filesystem->Write( &c, sizeof( c ), fh );

		// Save repository back out to disk...
		CUtlBuffer buf( 0, 0, false );

		for ( int i = m_Elements.FirstInorder(); i != m_Elements.InvalidIndex(); i = m_Elements.NextInorder( i ) )
		{
			buf.SeekPut( CUtlBuffer::SEEK_HEAD, 0 );

			ElementType_t& element = m_Elements[ i ];

			char fn[ 512 ];
			filesystem->String( element.handle, fn, sizeof( fn ) );

			buf.PutString( fn );
			buf.PutInt( element.fileinfo );

			Assert( element.dataIndex != m_Data.InvalidIndex() );

			T *data = m_Data[ element.dataIndex ];

			Assert( data );

			((IBaseCacheInfo *)data)->Save( buf );

			int bufsize = buf.TellPut();
			filesystem->Write( &bufsize, sizeof( bufsize ), fh );
			filesystem->Write( buf.Base(), bufsize, fh );
		}

		filesystem->Close( fh );
	}

	if ( m_bSaveManifest )
	{
		SaveManifest();
	}
}

template <class T>
void CUtlCachedFileData<T>::Shutdown()
{
	if ( !m_bInitialized )
		return;

	m_bInitialized = false;

	if ( IsDirty() )
	{
		Save();
	}
	// No matter what, create the manifest if it doesn't exist on the HD yet
	else if ( m_bSaveManifest && !ManifestExists() )
	{
		SaveManifest();
	}

	m_Elements.RemoveAll();
}

template <class T>
bool CUtlCachedFileData<T>::ManifestExists()
{
	char manifest_name[ 512 ];
	Q_strncpy( manifest_name, m_pszRepositoryFileName, sizeof( manifest_name ) );

	Q_SetExtension( manifest_name, ".manifest", sizeof( manifest_name ) );

	return filesystem->FileExists( manifest_name, "MOD" );
}

template <class T>
void CUtlCachedFileData<T>::SaveManifest()
{
	// Save manifest out to disk...
	CUtlBuffer buf( 0, 0, true );

	for ( int i = m_Elements.FirstInorder(); i != m_Elements.InvalidIndex(); i = m_Elements.NextInorder( i ) )
	{
		ElementType_t& element = m_Elements[ i ];

		char fn[ 512 ];
		filesystem->String( element.handle, fn, sizeof( fn ) );

		buf.Printf( "\"%s\"\r\n", fn );
	}

	char path[ 512 ];
	Q_strncpy( path, m_pszRepositoryFileName, sizeof( path ) );
	Q_StripFilename( path );

	filesystem->CreateDirHierarchy( path, "MOD" );

	char manifest_name[ 512 ];
	Q_strncpy( manifest_name, m_pszRepositoryFileName, sizeof( manifest_name ) );

	Q_SetExtension( manifest_name, ".manifest", sizeof( manifest_name ) );

	if ( filesystem->FileExists( manifest_name, "MOD" ) && 
		!filesystem->IsFileWritable( manifest_name, "MOD" ) )
	{
		filesystem->SetFileWritable( manifest_name, true, "MOD" );
	}

	// Now write to file
	FileHandle_t fh;
	fh = filesystem->Open( manifest_name, "wb" );
	if ( FILESYSTEM_INVALID_HANDLE != fh )
	{
		filesystem->Write( buf.Base(), buf.TellPut(), fh );
		filesystem->Close( fh );

		// DevMsg( "Persisting cache manifest '%s' (%d entries)\n", manifest_name, c );
	}
	else
	{
		Warning( "Unable to persist cache manifest '%s', check file permissions\n", manifest_name );
	}
}

template <class T>
void CUtlCachedFileData<T>::RebuildCache( char const *filename, T *data )
{
	Assert( !m_bReadOnly );

	// Recache item, mark self as dirty
	SetDirty( true );

	((IBaseCacheInfo *)data)->Rebuild( filename );
}

template <class T>
void CUtlCachedFileData<T>::ForceRecheckDiskInfo()
{
	for ( int i = m_Elements.FirstInorder(); i != m_Elements.InvalidIndex(); i = m_Elements.NextInorder( i ) )
	{
		ElementType_t& element = m_Elements[ i ];
		element.diskfileinfo = UTL_CACHED_FILE_DATA_UNDEFINED_DISKINFO;
	}
}

// Iterates all entries and causes rebuild on any existing items which are out of date
template <class T>
void	CUtlCachedFileData<T>::CheckDiskInfo( bool forcerebuild )
{
	for ( int i = m_Elements.FirstInorder(); i != m_Elements.InvalidIndex(); i = m_Elements.NextInorder( i ) )
	{
		ElementType_t& element = m_Elements[ i ];

		char fn[ 512 ];
		filesystem->String( element.handle, fn, sizeof( fn ) );

		if ( forcerebuild )
		{
			Get( fn );
		}
		else
		{
			if ( element.diskfileinfo == UTL_CACHED_FILE_DATA_UNDEFINED_DISKINFO )
			{
				if ( m_bNeverCheckDisk )
				{
					element.diskfileinfo = element.fileinfo;
				}
				else
				{
					if ( m_fileCheckType == UTL_CACHED_FILE_USE_FILESIZE ) 
					{
						element.diskfileinfo = filesystem->Size( fn );
					}
					else
					{
						element.diskfileinfo = filesystem->GetFileTime( fn );
					}
				}
			}
		}
	}
}

#include "tier0/memdbgoff.h"

#endif // UTLCACHEDFILEDATA_H
