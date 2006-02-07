//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "zip_utils.h"
#include "utlbuffer.h"
#include "utlsymbol.h"
#include "zip_uncompressed.h"
#include "checksum_crc.h"

//-----------------------------------------------------------------------------
// Purpose: Container for modifiable pak file which is embedded inside the .bsp file
//  itself.  It's used to allow one-off files to be stored local to the map and it is
//  hooked into the file system as an override for searching for named files.
//-----------------------------------------------------------------------------
class CZipFile
{
public:
	// Construction
				CZipFile( void );
				~CZipFile( void );

	// Public API
	// Clear all existing data
	void		Reset( void );

	// Add buffer to zip as a file with given name
	void		AddBufferToZip( const char *relativename, void *data, int length, bool bTextMode );

	// Initialize the PAK file from a buffer
	void		ParseFromBuffer( byte *buffer, int bufferlength );

	// Write the PAK lump to the .bsp file being created
	void		SaveToBuffer( CUtlBuffer& buffer );

private:
	// Hopefully this is enough
	enum
	{
		MAX_FILES_IN_ZIP = 32768,
	};

	typedef struct
	{
		CUtlSymbol			m_Name;
		int					filepos;
		int					filelen;
	} TmpFileInfo_t;

	// Internal entry for faster searching, etc.
	class CZipEntry
	{
	public:
					CZipEntry( void );
					~CZipEntry( void );

					CZipEntry( const CZipEntry& src );

		// RB tree compare function
		static bool ZipFileLessFunc( CZipEntry const& src1, CZipEntry const& src2 );

		// Name of entry
		CUtlSymbol	m_Name;
		// Offset ( set during write only )
		int			offset;
		// Lenth of data element
		int			length;
		// Raw data
		void		*data;
	};

	// For fast name lookup and sorting
	CUtlRBTree< CZipEntry, int > m_Files;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CZipFile::CZipEntry::CZipEntry( void )
{
	m_Name = "";
	offset = 0;
	length = 0;
	data = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : src - 
//-----------------------------------------------------------------------------
CZipFile::CZipEntry::CZipEntry( const CZipFile::CZipEntry& src )
{
	m_Name = src.m_Name;
	offset = src.offset;
	length = src.length;
	if ( length > 0 )
	{
		data = malloc( length );
		memcpy( data, src.data, length );
	}
	else
	{
		data = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Clear any leftover data
//-----------------------------------------------------------------------------
CZipFile::CZipEntry::~CZipEntry( void )
{
	free( data );
}

//-----------------------------------------------------------------------------
// Purpose: Construction
//-----------------------------------------------------------------------------
CZipFile::CZipFile( void )
: m_Files( 0, 32, CZipEntry::ZipFileLessFunc )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destroy zip data
//-----------------------------------------------------------------------------
CZipFile::~CZipFile( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Comparison for sorting entries
// Input  : src1 - 
//			src2 - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CZipFile::CZipEntry::ZipFileLessFunc( CZipEntry const& src1, CZipEntry const& src2 )
{
	return ( src1.m_Name < src2.m_Name );
}

//-----------------------------------------------------------------------------
// Purpose: Load pak file from raw buffer
// Input  : *buffer - 
//			bufferlength - 
//-----------------------------------------------------------------------------
void CZipFile::ParseFromBuffer( byte *buffer, int bufferlength )
{
	// Through away old data
	Reset();

	// Initialize a buffer
	CUtlBuffer buf( 0, bufferlength, false );
	buf.Put( buffer, bufferlength );

	buf.SeekGet( CUtlBuffer::SEEK_TAIL, 0 );
	int fileLen = buf.TellGet();

	// Start from beginning
	buf.SeekGet( CUtlBuffer::SEEK_HEAD, 0 );

	int offset;
	ZIP_EndOfCentralDirRecord rec;
	rec.startOfCentralDirOffset = 0;
	rec.nCentralDirectoryEntries_Total = 0;

	bool foundEndOfCentralDirRecord = false;
	for( offset = fileLen - sizeof( ZIP_EndOfCentralDirRecord ); offset >= 0; offset-- )
	{
		buf.SeekGet( CUtlBuffer::SEEK_HEAD, offset );
		buf.Get( &rec, sizeof( ZIP_EndOfCentralDirRecord ) );
		if( rec.signature == 0x06054b50 )
		{
			foundEndOfCentralDirRecord = true;
			break;
		}
	}
	Assert( foundEndOfCentralDirRecord );
	
	buf.SeekGet( CUtlBuffer::SEEK_HEAD, rec.startOfCentralDirOffset );

	// Make sure there are some files to parse
	int numzipfiles = rec.nCentralDirectoryEntries_Total;
	if (  numzipfiles <= 0 )
	{
		// No files, sigh...
		return;
	}

	// Allocate space for directory
	TmpFileInfo_t *newfiles = new TmpFileInfo_t[ numzipfiles ];
	Assert( newfiles );

	int i;
	for( i = 0; i < rec.nCentralDirectoryEntries_Total; i++ )
	{
		ZIP_FileHeader fileHeader;
		buf.Get( &fileHeader, sizeof( ZIP_FileHeader ) );
		Assert( fileHeader.signature == 0x02014b50 );
		Assert( fileHeader.compressionMethod == 0 );
		
		// bogus. . .do we have to allocate this here?  should make a symbol instead.
		char tmpString[1024];
		buf.Get( tmpString, fileHeader.fileNameLength );
		tmpString[fileHeader.fileNameLength] = '\0';
		strlwr( tmpString );
		newfiles[i].m_Name = tmpString;
		newfiles[i].filepos = fileHeader.relativeOffsetOfLocalHeader;
		newfiles[i].filelen = fileHeader.compressedSize;
		buf.SeekGet( CUtlBuffer::SEEK_CURRENT, fileHeader.extraFieldLength + fileHeader.fileCommentLength );
	}

	for( i = 0; i < rec.nCentralDirectoryEntries_Total; i++ )
	{
		buf.SeekGet( CUtlBuffer::SEEK_HEAD, newfiles[i].filepos );
		ZIP_LocalFileHeader localFileHeader;
		buf.Get( &localFileHeader, sizeof( ZIP_LocalFileHeader ) );
		Assert( localFileHeader.signature == 0x04034b50 );
		buf.SeekGet( CUtlBuffer::SEEK_CURRENT, localFileHeader.fileNameLength + localFileHeader.extraFieldLength );
		newfiles[i].filepos = buf.TellGet();
	}

	// Insert current data into rb tree
	for ( i=0 ; i<numzipfiles ; i++ )
	{
		CZipEntry e;
		e.m_Name = newfiles[ i ].m_Name;
		e.length = newfiles[ i ].filelen;
		e.offset = 0;
		
		// Make sure length is reasonable
		if ( e.length > 0 )
		{
			e.data = malloc( e.length );

			// Copy in data
			buf.SeekGet( CUtlBuffer::SEEK_HEAD, newfiles[ i ].filepos );
			buf.Get( e.data, e.length );
		}
		else
		{
			e.data = NULL;
		}

		// Add to tree
		m_Files.Insert( e );
	}

	// Through away directory
	delete[] newfiles;
}

static int GetLengthOfBinStringAsText( const char *pSrc, int srcSize )
{
	const char *pSrcScan = pSrc;
	const char *pSrcEnd = pSrc + srcSize;
	int numChars = 0;
	for( ; pSrcScan < pSrcEnd; pSrcScan++ )
	{
		if( *pSrcScan == '\n' )
		{
			numChars += 2;
		}
		else
		{
			numChars++;
		}
	}
	return numChars;
}

static void CopyTextData( char *pDst, const char *pSrc, int dstSize, int srcSize )
{
	const char *pSrcScan = pSrc;
	const char *pSrcEnd = pSrc + srcSize;
	char *pDstScan = pDst;

#ifdef _DEBUG
	char *pDstEnd = pDst + dstSize;
#endif

	for( ; pSrcScan < pSrcEnd; pSrcScan++ )
	{
		if( *pSrcScan == '\n' )
		{
			*pDstScan = '\r';
			pDstScan++;
			*pDstScan = '\n';
			pDstScan++;
		}
		else
		{
			*pDstScan = *pSrcScan;
			pDstScan++;
		}
	}
	Assert( pSrcScan == pSrcEnd );
	Assert( pDstScan == pDstEnd );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new lump, or overwrites existing one
// Input  : *relativename - 
//			*data - 
//			length - 
//-----------------------------------------------------------------------------
void CZipFile::AddBufferToZip( const char *relativename, void *data, int length, bool bTextMode )
{
	// Lower case only
	char name[ 512 ];
	strcpy( name, relativename );
	strlwr( name );

	int dstLength = length;
	if( bTextMode )
	{
		dstLength = GetLengthOfBinStringAsText( ( const char * )data, length );
	}
	
	// See if entry is in list already
	CZipEntry e;
	e.m_Name = name;
	int index = m_Files.Find( e );

	// If already existing, throw away old data and update data and length
	if ( index != m_Files.InvalidIndex() )
	{
		CZipEntry *update = &m_Files[ index ];
		free( update->data );
		if( bTextMode )
		{
			update->data = malloc( dstLength );
			CopyTextData( ( char * )update->data, ( char * )data, dstLength, length );
			update->length = dstLength;
		}
		else
		{
			update->data = malloc( length );
			memcpy( update->data, data, length );
			update->length = length;
		}
		update->offset = 0;
	}
	else
	{
		// Create a new entry
		e.length	= dstLength;
		if ( dstLength > 0 )
		{
			if( bTextMode )
			{
				e.data = malloc( dstLength );
				CopyTextData( ( char * )e.data, ( char * )data, dstLength, length );
			}
			else
			{
				e.data = malloc( length );
				memcpy(e.data, data, length );
			}
		}
		else
		{
			e.data = NULL;
		}
		e.offset	= 0;

		m_Files.Insert( e );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Store data back out to .bsp file
//-----------------------------------------------------------------------------
void CZipFile::SaveToBuffer( CUtlBuffer& buf )
{
	unsigned int i;
	for( i = 0; i < m_Files.Count(); i++ )
	{
		CZipEntry *e = &m_Files[ i ];
		Assert( e );

		// Fix up the offset
		e->offset = buf.TellPut();

		if ( e->length > 0 && e->data != NULL )
		{
			ZIP_LocalFileHeader hdr;
			hdr.signature = 0x04034b50;
			hdr.versionNeededToExtract = 10;  // This is the version that the winzip that I have writes.
			hdr.flags = 0;
			hdr.compressionMethod = 0; // NO COMPRESSION!
			hdr.lastModifiedTime = 0;
			hdr.lastModifiedDate = 0;

			CRC32_t crc;
			CRC32_Init( &crc );
			CRC32_ProcessBuffer( &crc, e->data, e->length );
			CRC32_Final( &crc );
			hdr.crc32 = crc;
			
			hdr.compressedSize = e->length;
			hdr.uncompressedSize = e->length;
			hdr.fileNameLength = strlen( e->m_Name.String() );
			hdr.extraFieldLength = 0;

			buf.Put( &hdr, sizeof( hdr ) );
			buf.Put( e->m_Name.String(), strlen( e->m_Name.String() ) );
			buf.Put( e->data, e->length );
		}
	}
	int centralDirStart = buf.TellPut();
	int realNumFiles = 0;
	for( i = 0; i < m_Files.Count(); i++, realNumFiles++ )
	{
		CZipEntry *e = &m_Files[ i ];
		Assert( e );
		
		if ( e->length > 0 && e->data != NULL )
		{
			ZIP_FileHeader hdr;
			hdr.signature = 0x02014b50;
			hdr.versionMadeBy = 20; // This is the version that the winzip that I have writes.
			hdr.versionNeededToExtract = 10; // This is the version that the winzip that I have writes.
			hdr.flags = 0;
			hdr.compressionMethod = 0;
			hdr.lastModifiedTime = 0;
			hdr.lastModifiedDate = 0;

			// hack - processing the crc twice.
			CRC32_t crc;
			CRC32_Init( &crc );
			CRC32_ProcessBuffer( &crc, e->data, e->length );
			CRC32_Final( &crc );
			hdr.crc32 = crc;

			hdr.compressedSize = e->length;
			hdr.uncompressedSize = e->length;
			hdr.fileNameLength = strlen( e->m_Name.String() );
			hdr.extraFieldLength = 0;
			hdr.fileCommentLength = 0;
			hdr.diskNumberStart = 0;
			hdr.internalFileAttribs = 0;
			hdr.externalFileAttribs = 0; // This is usually something, but zero is OK as if the input came from stdin
			hdr.relativeOffsetOfLocalHeader = e->offset;

			buf.Put( &hdr, sizeof( hdr ) );
			buf.Put( e->m_Name.String(), strlen( e->m_Name.String() ) );
		}
	}
	int centralDirEnd = buf.TellPut();

	ZIP_EndOfCentralDirRecord rec;
	rec.signature = 0x06054b50;
	rec.numberOfThisDisk = 0;
	rec.numberOfTheDiskWithStartOfCentralDirectory = 0;
	rec.nCentralDirectoryEntries_ThisDisk = realNumFiles;
	rec.nCentralDirectoryEntries_Total = realNumFiles;
	rec.centralDirectorySize = centralDirEnd - centralDirStart;
	rec.startOfCentralDirOffset = centralDirStart;
	rec.commentLength = 0;

	buf.Put( &rec, sizeof( rec ) );
}

//-----------------------------------------------------------------------------
// Purpose: Delete all current data
//-----------------------------------------------------------------------------
void CZipFile::Reset( void )
{
	m_Files.RemoveAll();
}


class CZip : public IZip
{
public:
	CZip()
	{
		m_ZipFile.Reset();
	}

	virtual ~CZip()
	{
		m_ZipFile.Reset();
	}

	virtual void		Reset();

	// Add buffer to zip as a file with given name
	virtual void		AddBufferToZip( const char *relativename, void *data, int length, bool bTextMode );

	virtual void		SaveToBuffer( CUtlBuffer& outbuf );

	virtual void		ParseFromBuffer( unsigned char *buffer, int bufferlength );

private:

	CZipFile			m_ZipFile;
};

static CZip g_ZipUtils;
IZip *zip_utils = &g_ZipUtils;

void CZip::Reset()
{
	m_ZipFile.Reset();
}

// Add buffer to zip as a file with given name
void CZip::AddBufferToZip( const char *relativename, void *data, int length, bool bTextMode )
{
	m_ZipFile.AddBufferToZip( relativename, data, length, bTextMode );
}

void CZip::SaveToBuffer( CUtlBuffer& outbuf )
{
	m_ZipFile.SaveToBuffer( outbuf );
}

void CZip::ParseFromBuffer( unsigned char *buffer, int bufferlength )
{
	m_ZipFile.Reset();
	m_ZipFile.ParseFromBuffer( buffer, bufferlength );
}
