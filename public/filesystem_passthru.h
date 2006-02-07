//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FILESYSTEM_PASSTHRU_H
#define FILESYSTEM_PASSTHRU_H
#ifdef _WIN32
#pragma once
#endif


#include "filesystem.h"
#include <stdio.h>
#include <stdarg.h>


//
// These classes pass all filesystem interface calls through to another filesystem
// interface. They can be used anytime you want to override a couple things in 
// a filesystem. VMPI uses this to override the base filesystem calls while 
// allowing the rest of the filesystem functionality to work on the master.
//

template<class Base>
class CInternalFileSystemPassThru : public Base
{
public:
	CInternalFileSystemPassThru()
	{
		m_pBaseFileSystemPassThru = NULL;
	}
	virtual void InitPassThru( IBaseFileSystem *pBaseFileSystemPassThru )
	{
		m_pBaseFileSystemPassThru = pBaseFileSystemPassThru;
	}
	virtual int				Read( void* pOutput, int size, FileHandle_t file )							{ return m_pBaseFileSystemPassThru->Read( pOutput, size, file ); }
	virtual int				Write( void const* pInput, int size, FileHandle_t file )					{ return m_pBaseFileSystemPassThru->Write( pInput, size, file ); }
	virtual FileHandle_t	Open( const char *pFileName, const char *pOptions, const char *pathID )		{ return m_pBaseFileSystemPassThru->Open( pFileName, pOptions, pathID ); }
	virtual void			Close( FileHandle_t file )													{ m_pBaseFileSystemPassThru->Close( file ); }
	virtual void			Seek( FileHandle_t file, int pos, FileSystemSeek_t seekType )				{ m_pBaseFileSystemPassThru->Seek( file, pos, seekType ); }
	virtual unsigned int	Tell( FileHandle_t file )													{ return m_pBaseFileSystemPassThru->Tell( file ); }
	virtual unsigned int	Size( FileHandle_t file )													{ return m_pBaseFileSystemPassThru->Size( file ); }
	virtual unsigned int	Size( const char *pFileName, const char *pPathID )							{ return m_pBaseFileSystemPassThru->Size( pFileName, pPathID ); }
	virtual void			Flush( FileHandle_t file )													{ m_pBaseFileSystemPassThru->Flush( file ); }
	virtual bool			Precache( const char *pFileName, const char *pPathID )						{ return m_pBaseFileSystemPassThru->Precache( pFileName, pPathID ); }
	virtual bool			FileExists( const char *pFileName, const char *pPathID )					{ return m_pBaseFileSystemPassThru->FileExists( pFileName, pPathID ); }
	virtual bool			IsFileWritable( char const *pFileName, const char *pPathID )					{ return m_pBaseFileSystemPassThru->IsFileWritable( pFileName, pPathID ); }
	virtual bool			SetFileWritable( char const *pFileName, bool writable, const char *pPathID )	{ return m_pBaseFileSystemPassThru->SetFileWritable( pFileName, writable, pPathID ); }
	virtual long			GetFileTime( const char *pFileName, const char *pPathID )						{ return m_pBaseFileSystemPassThru->GetFileTime( pFileName, pPathID ); }

protected:
	IBaseFileSystem *m_pBaseFileSystemPassThru;
};


class CBaseFileSystemPassThru : public CInternalFileSystemPassThru<IBaseFileSystem>
{
public:
};


class CFileSystemPassThru : public CInternalFileSystemPassThru<IFileSystem>
{
public:
	typedef CInternalFileSystemPassThru<IFileSystem> BaseClass;

	CFileSystemPassThru()
	{
		m_pFileSystemPassThru = NULL;
	}
	virtual void InitPassThru( IFileSystem *pFileSystemPassThru, bool bBaseOnly )
	{
		if ( !bBaseOnly )
			m_pFileSystemPassThru = pFileSystemPassThru;
		
		BaseClass::InitPassThru( pFileSystemPassThru );
	}

	// IAppSystem stuff.
	// Here's where the app systems get to learn about each other 
	virtual bool Connect( CreateInterfaceFn factory )															{ return m_pFileSystemPassThru->Connect( factory ); }
	virtual void Disconnect()																					{ m_pFileSystemPassThru->Disconnect(); }
	virtual void *QueryInterface( const char *pInterfaceName )													{ return m_pFileSystemPassThru->QueryInterface( pInterfaceName ); }
	virtual InitReturnVal_t Init()																				{ return m_pFileSystemPassThru->Init(); }
	virtual void Shutdown()																						{ m_pFileSystemPassThru->Shutdown(); }


	virtual void			RemoveAllSearchPaths( void )														{ m_pFileSystemPassThru->RemoveAllSearchPaths(); }
	virtual void			AddSearchPath( const char *pPath, const char *pathID, SearchPathAdd_t addType )		{ m_pFileSystemPassThru->AddSearchPath( pPath, pathID, addType ); }
	virtual bool			RemoveSearchPath( const char *pPath, const char *pathID )							{ return m_pFileSystemPassThru->RemoveSearchPath( pPath, pathID ); }
	virtual void			RemoveFile( char const* pRelativePath, const char *pathID )							{ m_pFileSystemPassThru->RemoveFile( pRelativePath, pathID ); }
	virtual void			RenameFile( char const *pOldPath, char const *pNewPath, const char *pathID )		{ m_pFileSystemPassThru->RenameFile( pOldPath, pNewPath, pathID ); }
	virtual void			CreateDirHierarchy( const char *path, const char *pathID )							{ m_pFileSystemPassThru->CreateDirHierarchy( path, pathID ); }
	virtual bool			IsDirectory( const char *pFileName, const char *pathID )							{ return m_pFileSystemPassThru->IsDirectory( pFileName, pathID ); }
	virtual void			FileTimeToString( char* pStrip, int maxCharsIncludingTerminator, long fileTime )	{ m_pFileSystemPassThru->FileTimeToString( pStrip, maxCharsIncludingTerminator, fileTime ); }
	virtual bool			IsOk( FileHandle_t file )															{ return m_pFileSystemPassThru->IsOk( file ); }
	virtual bool			EndOfFile( FileHandle_t file )														{ return m_pFileSystemPassThru->EndOfFile( file ); }
	virtual char			*ReadLine( char *pOutput, int maxChars, FileHandle_t file )							{ return m_pFileSystemPassThru->ReadLine( pOutput, maxChars, file ); }
	virtual int				FPrintf( FileHandle_t file, char *pFormat, ... ) 
	{ 
		char str[8192];
		va_list marker;
		va_start( marker, pFormat );
		_vsnprintf( str, sizeof( str ), pFormat, marker );
		va_end( marker );
		return m_pFileSystemPassThru->FPrintf( file, "%s", str );
	}
	virtual CSysModule 		*LoadModule( const char *pFileName, const char *pPathID, bool bValidatedDllOnly )	{ return m_pFileSystemPassThru->LoadModule( pFileName, pPathID, bValidatedDllOnly ); }
	virtual void			UnloadModule( CSysModule *pModule )													{ m_pFileSystemPassThru->UnloadModule( pModule ); }
	virtual const char		*FindFirst( const char *pWildCard, FileFindHandle_t *pHandle )						{ return m_pFileSystemPassThru->FindFirst( pWildCard, pHandle ); }
	virtual const char		*FindNext( FileFindHandle_t handle )												{ return m_pFileSystemPassThru->FindNext( handle ); }
	virtual bool			FindIsDirectory( FileFindHandle_t handle )											{ return m_pFileSystemPassThru->FindIsDirectory( handle ); }
	virtual void			FindClose( FileFindHandle_t handle )												{ m_pFileSystemPassThru->FindClose( handle ); }
	virtual const char		*GetLocalPath( const char *pFileName, char *pLocalPath, int localPathBufferSize )	{ return m_pFileSystemPassThru->GetLocalPath( pFileName, pLocalPath, localPathBufferSize ); }
	virtual bool			FullPathToRelativePath( const char *pFullpath, char *pRelative, int maxlen )		{ return m_pFileSystemPassThru->FullPathToRelativePath( pFullpath, pRelative, maxlen ); }
	virtual bool			GetCurrentDirectory( char* pDirectory, int maxlen )									{ return m_pFileSystemPassThru->GetCurrentDirectory( pDirectory, maxlen ); }
	virtual void			PrintOpenedFiles( void )															{ m_pFileSystemPassThru->PrintOpenedFiles(); }
	virtual void			PrintSearchPaths( void )															{ m_pFileSystemPassThru->PrintSearchPaths(); }
	virtual void			SetWarningFunc( void (*pfnWarning)( const char *fmt, ... ) )						{ m_pFileSystemPassThru->SetWarningFunc( pfnWarning ); }
	virtual void			SetWarningLevel( FileWarningLevel_t level )											{ m_pFileSystemPassThru->SetWarningLevel( level ); } 
	virtual void			AddLoggingFunc( void (*pfnLogFunc)( const char *fileName, const char *accessType ) ){ m_pFileSystemPassThru->AddLoggingFunc( pfnLogFunc ); }
	virtual void			RemoveLoggingFunc( FileSystemLoggingFunc_t logFunc )								{ m_pFileSystemPassThru->RemoveLoggingFunc( logFunc ); }
	virtual fsasync_t		AsyncFilePrefetch(int numFiles, const asyncFileList_t *fileListPtr)					{ return m_pFileSystemPassThru->AsyncFilePrefetch( numFiles, fileListPtr ); }
	virtual fsasync_t		AsyncFileFinish(int asyncID, bool wait)												{ return m_pFileSystemPassThru->AsyncFileFinish( asyncID, wait ); }
	virtual fsasync_t		AsyncFileAbort(int asyncID)															{ return m_pFileSystemPassThru->AsyncFileAbort( asyncID ); }
	virtual fsasync_t		AsyncFileStatus(int asyncID)														{ return m_pFileSystemPassThru->AsyncFileStatus( asyncID ); }
	virtual fsasync_t		AsyncFlush()																		{ return m_pFileSystemPassThru->AsyncFlush(); }
	virtual const FileSystemStatistics *GetFilesystemStatistics()												{ return m_pFileSystemPassThru->GetFilesystemStatistics(); }
	virtual WaitForResourcesHandle_t WaitForResources( const char *resourcelist )								{ return m_pFileSystemPassThru->WaitForResources( resourcelist ); }
	virtual bool			GetWaitForResourcesProgress( WaitForResourcesHandle_t handle, 
								float *progress, bool *complete )												{ return m_pFileSystemPassThru->GetWaitForResourcesProgress( handle, progress, complete ); }
	virtual void			CancelWaitForResources( WaitForResourcesHandle_t handle )							{ m_pFileSystemPassThru->CancelWaitForResources( handle ); }
	virtual int				HintResourceNeed( const char *hintlist, int forgetEverything )						{ return m_pFileSystemPassThru->HintResourceNeed( hintlist, forgetEverything ); }
	virtual bool			IsFileImmediatelyAvailable(const char *pFileName)									{ return m_pFileSystemPassThru->IsFileImmediatelyAvailable( pFileName ); }
	virtual void			GetLocalCopy( const char *pFileName )												{ m_pFileSystemPassThru->GetLocalCopy( pFileName ); }
	virtual FileNameHandle_t	FindOrAddFileName( char const *pFileName )										{ return m_pFileSystemPassThru->FindOrAddFileName( pFileName ); }
	virtual bool				String( const FileNameHandle_t& handle, char *buf, int buflen )					{ return m_pFileSystemPassThru->String( handle, buf, buflen ); }
	virtual bool			IsOk2( FileHandle_t file )															{ return IsOk(file); }
	virtual void			RemoveSearchPaths( const char *szPathID )											{ m_pFileSystemPassThru->RemoveSearchPaths( szPathID ); }
	virtual bool			IsSteam() const																		{ return m_pFileSystemPassThru->IsSteam(); }
	virtual	FilesystemMountRetval_t MountSteamContent( int nExtraAppId = -1 )									{ return m_pFileSystemPassThru->MountSteamContent( nExtraAppId ); }
	
	virtual const char		*FindFirstEx( 
		const char *pWildCard, 
		const char *pPathID,
		FileFindHandle_t *pHandle
		)																										{ return m_pFileSystemPassThru->FindFirstEx( pWildCard, pPathID, pHandle ); }
	virtual void			MarkPathIDByRequestOnly( const char *pPathID, bool bRequestOnly )					{ m_pFileSystemPassThru->MarkPathIDByRequestOnly( pPathID, bRequestOnly ); }
	virtual bool			AddPackFile( const char *fullpath, const char *pathID )								{ return m_pFileSystemPassThru->AddPackFile( fullpath, pathID ); }
	virtual fsasync_t		AsyncFileWrite(const char *pFileName, const void *pSrc, int nSrcBytes, bool bFreeMemory) { return m_pFileSystemPassThru->AsyncFileWrite( pFileName, pSrc, nSrcBytes, bFreeMemory); }
	virtual fsasync_t		AsyncFileAppendFile(const char *pDestFileName, const char *pSrcFileName)			{ return m_pFileSystemPassThru->AsyncFileAppendFile(pDestFileName, pSrcFileName); }
	virtual void			AsyncFileFinishAllWrites()															{ m_pFileSystemPassThru->AsyncFileFinishAllWrites(); }
	virtual fsasync_t		AsyncFileSetPriority(int asyncID, int newPriority)									{ return m_pFileSystemPassThru->AsyncFileSetPriority(asyncID, newPriority); }


protected:
	IFileSystem *m_pFileSystemPassThru;
};


// This is so people who change the filesystem interface are forced to add the passthru wrapper into CFileSystemPassThru,
// so they don't break VMPI.
inline void GiveMeACompileError()
{
	CFileSystemPassThru asdf;
}


#endif // FILESYSTEM_PASSTHRU_H
