//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef VSTDLIB_STRTOOLS_H
#define VSTDLIB_STRTOOLS_H

#ifdef _WIN32
#pragma once
#elif _LINUX
#include <ctype.h>
#endif

#include <string.h>
#include <stdlib.h>
#include "vstdlib/vstdlib.h"


template< class T > class CUtlMemory;
template< class T, class A > class CUtlVector;

//-----------------------------------------------------------------------------
// Portable versions of standard string functions
//-----------------------------------------------------------------------------
VSTDLIB_INTERFACE void	_Q_memset (const char* file, int line, void *dest, int fill, int count);
VSTDLIB_INTERFACE void	_Q_memcpy (const char* file, int line, void *dest, const void *src, int count);
VSTDLIB_INTERFACE void  _Q_memmove(const char* file, int line, void *dest, const void *src, int count);
VSTDLIB_INTERFACE int	_Q_memcmp (const char* file, int line, void *m1, void *m2, int count);
VSTDLIB_INTERFACE int	_Q_strlen (const char* file, int line, const char *str);
VSTDLIB_INTERFACE void	_Q_strcpy (const char* file, int line, char *dest, const char *src);
VSTDLIB_INTERFACE char*	_Q_strrchr (const char* file, int line, const char *s, char c);
VSTDLIB_INTERFACE int	_Q_strcmp (const char* file, int line, const char *s1, const char *s2);
VSTDLIB_INTERFACE int	_Q_stricmp( const char* file, int line, const char *s1, const char *s2 );
VSTDLIB_INTERFACE char*	_Q_strstr( const char* file, int line, const char *s1, const char *search );
VSTDLIB_INTERFACE char*	_Q_strupr (const char* file, int line, char *start);
VSTDLIB_INTERFACE char*	_Q_strlower (const char* file, int line, char *start);

#ifdef _DEBUG

	#define Q_memset(dest, fill, count)		_Q_memset   (__FILE__, __LINE__, (dest), (fill), (count))	
	#define Q_memcpy(dest, src, count)		_Q_memcpy	(__FILE__, __LINE__, (dest), (src), (count))	
	#define Q_memmove(dest, src, count)		_Q_memmove	(__FILE__, __LINE__, (dest), (src), (count))	
	#define Q_memcmp(m1, m2, count)			_Q_memcmp	(__FILE__, __LINE__, (m1), (m2), (count))		
	#define Q_strlen(str)					_Q_strlen	(__FILE__, __LINE__, (str))				
	#define Q_strcpy(dest, src)				_Q_strcpy	(__FILE__, __LINE__, (dest), (src))			
	#define Q_strrchr(s, c)					_Q_strrchr	(__FILE__, __LINE__, (s), (c))				
	#define Q_strcmp(s1, s2)				_Q_strcmp	(__FILE__, __LINE__, (s1), (s2))			
	#define Q_stricmp(s1, s2 )				_Q_stricmp	(__FILE__, __LINE__, (s1), (s2) )			
	#define Q_strstr(s1, search )			_Q_strstr	(__FILE__, __LINE__, (s1), (search) )		
	#define Q_strupr(start)					_Q_strupr	(__FILE__, __LINE__, (start))				
	#define Q_strlower(start)				_Q_strlower (__FILE__, __LINE__, (start))				

#else

#ifdef _LINUX
inline char *strupr( char *start )
{
      char *str = start;
      while( str && *str )
      {
              *str = (char)toupper(*str);
              str++;
      }
      return start;
}

inline char *strlwr( char *start )
{
      char *str = start;
      while( str && *str )
      {
              *str = (char)tolower(*str);
              str++;
      }
      return start;
}

#endif

inline void		Q_memset (void *dest, int fill, int count)			{ memset( dest, fill, count ); }
inline void		Q_memcpy (void *dest, const void *src, int count)	{ memcpy( dest, src, count ); }
inline void		Q_memmove (void *dest, const void *src, int count)	{ memmove( dest, src, count ); }
inline int		Q_memcmp (void *m1, void *m2, int count)			{ return memcmp( m1, m2, count ); } 
inline int		Q_strlen (const char *str)							{ return strlen ( str ); }
inline void		Q_strcpy (char *dest, const char *src)				{ strcpy( dest, src ); }
inline char*	Q_strrchr (const char *s, char c)					{ return strrchr( s, c ); }
inline int		Q_strcmp (const char *s1, const char *s2)			{ return strcmp( s1, s2 ); }
inline int		Q_stricmp( const char *s1, const char *s2 )			{ return stricmp( s1, s2 ); }
inline char*	Q_strstr( const char *s1, const char *search )		{ return strstr( s1, search ); }
inline char*	Q_strupr (char *start)								{ return strupr( start ); }
inline char*	Q_strlower (char *start)							{ return strlwr( start ); }

#endif

VSTDLIB_INTERFACE int	Q_strncmp (const char *s1, const char *s2, int count);
VSTDLIB_INTERFACE void	Q_strcat (char *dest, const char *src);
VSTDLIB_INTERFACE int	Q_strcasecmp (const char *s1, const char *s2);
VSTDLIB_INTERFACE int	Q_strncasecmp (const char *s1, const char *s2, int n);
VSTDLIB_INTERFACE int	Q_strnicmp (const char *s1, const char *s2, int n);
VSTDLIB_INTERFACE int	Q_atoi (const char *str);
VSTDLIB_INTERFACE float	Q_atof (const char *str);
VSTDLIB_INTERFACE char*	Q_stristr( char* pStr, char const* pSearch );
VSTDLIB_OVERLOAD  char const*	Q_stristr( char const* pStr, char const* pSearch );


// These are versions of functions that guarantee null termination.
//
// maxLen is the maximum number of bytes in the destination string.
// pDest[maxLen-1] is always null terminated if pSrc's length is >= maxLen.
//
// This means the last parameter can usually be a sizeof() of a string.
VSTDLIB_INTERFACE void Q_strncpy( char *pDest, char const *pSrc, int maxLen );
VSTDLIB_INTERFACE int Q_snprintf( char *pDest, int destLen, char const *pFormat, ... );
#define COPY_ALL_CHARACTERS -1
VSTDLIB_INTERFACE char *Q_strncat(char *, const char *, size_t destBufferSize, int max_chars_to_copy );
VSTDLIB_INTERFACE char *Q_strnlwr(char *, size_t);

// UNDONE: Find a non-compiler-specific way to do this
#ifdef _WIN32
#ifndef _VA_LIST_DEFINED
#ifdef  _M_ALPHA
typedef struct {
        char *a0;       /* pointer to first homed integer argument */
        int offset;     /* byte offset of next parameter */
} va_list;
#else
typedef char *  va_list;
#endif
#define _VA_LIST_DEFINED
#endif

#elif _LINUX
#include <stdarg.h>
#endif
VSTDLIB_INTERFACE int Q_vsnprintf( char *pDest, int maxLen, char const *pFormat, va_list params );

// Prints out a pretified memory counter string value ( e.g., 7,233.27 Mb, 1,298.003 Kb, 127 bytes )

VSTDLIB_INTERFACE char *Q_pretifymem( float value, int digitsafterdecimal = 2, bool usebinaryonek = false );

// Functions for converting hexidecimal character strings back into binary data etc.
//
// e.g., 
// int output;
// Q_hextobinary( "ffffffff", 8, &output, sizeof( output ) );
// would make output == 0xfffffff or -1
// Similarly,
// char buffer[ 9 ];
// Q_binarytohex( &output, sizeof( output ), buffer, sizeof( buffer ) );
// would put "ffffffff" into buffer (note null terminator!!!)
VSTDLIB_INTERFACE void Q_hextobinary( char const *in, int numchars, unsigned char *out, int maxoutputbytes );
VSTDLIB_INTERFACE void Q_binarytohex( const unsigned char *in, int inputbytes, char *out, int outsize );

// Tools for working with filenames

// Extracts the base name of a file (no path, no extension, assumes '/' or '\' as path separator)
VSTDLIB_INTERFACE void Q_FileBase( const char *in, char *out,int maxlen );
// Remove the final characters of ppath if it's '\' or '/'.
VSTDLIB_INTERFACE void Q_StripTrailingSlash( char *ppath );
// Remove any extension from in and return resulting string in out
VSTDLIB_INTERFACE void Q_StripExtension( const char *in, char *out, int outLen );
// Make path end with extension if it doesn't already have an extension
VSTDLIB_INTERFACE void Q_DefaultExtension( char *path, const char *extension, int pathStringLength );
// Strips any current extension from path and ensures that extension is the new extension
VSTDLIB_INTERFACE void Q_SetExtension( char *path, const char *extension, int pathStringLength );
// Removes any filename from path ( strips back to previous / or \ character )
VSTDLIB_INTERFACE void Q_StripFilename( char *path );
// Remove the final directory from the path
VSTDLIB_INTERFACE bool Q_StripLastDir( char *dirName, int maxlen );

// Copy out the path except for the stuff after the final pathseparator
VSTDLIB_INTERFACE bool Q_ExtractFilePath( const char *path, char *dest, int destSize );
// Copy out the file extension into dest
VSTDLIB_INTERFACE void Q_ExtractFileExtension( const char *path, char *dest, int destSize );

// This removes "./" and "../" from the pathname. pFilename should be a full pathname.
// Returns false if it tries to ".." past the root directory in the drive (in which case 
// it is an invalid path).
VSTDLIB_INTERFACE bool Q_RemoveDotSlashes( char *pFilename );

// If pPath is a relative path, this function makes it into an absolute path
// using the current working directory as the base, or pStartingDir if it's non-null.
// Returns false if it runs out of room in the string, or if pPath tries to ".." past the root directory.
VSTDLIB_INTERFACE void Q_MakeAbsolutePath( char *pOut, int outLen, const char *pPath, const char *pStartingDir = NULL );

// Adds a path separator to the end of the string if there isn't one already. Returns false if it would run out of space.
VSTDLIB_INTERFACE void Q_AppendSlash( char *pStr, int strSize );

// Returns true if the path is an absolute path.
VSTDLIB_INTERFACE bool Q_IsAbsolutePath( const char *pPath );

// Scans pIn and replaces all occurences of pMatch with pReplaceWith.
// Writes the result to pOut.
// Returns true if it completed successfully.
// If it would overflow pOut, it fills as much as it can and returns false.
VSTDLIB_INTERFACE bool Q_StrSubst( 
	const char *pIn, 
	const char *pMatch,
	const char *pReplaceWith,
	char *pOut,
	int outLen,
	bool bCaseSensitive=false
	);

// Split the specified string on the specified separator.
// Returns a list of strings separated by pSeparator.
// You are responsible for freeing the contents of outStrings (call outStrings.PurgeAndDeleteElements).
VSTDLIB_INTERFACE void Q_SplitString( const char *pString, const char *pSeparator, CUtlVector<char*, CUtlMemory<char*> > &outStrings );

// Just like Q_SplitString, but it can use multiple possible separators.
VSTDLIB_INTERFACE void Q_SplitString2( const char *pString, const char **pSeparators, int nSeparators, CUtlVector<char*, CUtlMemory<char*> > &outStrings );


// This function takes a slice out of pStr and stores it in pOut.
// It follows the Python slice convention:
// Negative numbers wrap around the string (-1 references the last character).
// Large numbers are clamped to the end of the string.
VSTDLIB_INTERFACE void Q_StrSlice( const char *pStr, int firstChar, int lastCharNonInclusive, char *pOut, int outSize );

// Chop off the left nChars of a string.
VSTDLIB_INTERFACE void Q_StrLeft( const char *pStr, int nChars, char *pOut, int outSize );

// Chop off the right nChars of a string.
VSTDLIB_INTERFACE void Q_StrRight( const char *pStr, int nChars, char *pOut, int outSize );


#ifdef _WIN32
#define CORRECT_PATH_SEPARATOR '\\'
#define INCORRECT_PATH_SEPARATOR '/'
#elif _LINUX
#define CORRECT_PATH_SEPARATOR '/'
#define INCORRECT_PATH_SEPARATOR '\\'
#endif

// Force slashes of either type to be = separator character
VSTDLIB_INTERFACE void Q_FixSlashes( char *pname, char separator = CORRECT_PATH_SEPARATOR );

#endif	// VSTDLIB_STRTOOLS_H
