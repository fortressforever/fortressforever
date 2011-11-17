
// ff_lualib_util.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_scriptman.h"
#include "filesystem.h"
#include "ff_utils.h"
#include "utlbuffer.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IFileSystem **pFilesystem = &filesystem;

#define LUADATA_CNIL		'-' /* 0x2D (45) */
#define LUADATA_CFALSE		'0' /* 0x30 (48) */
#define LUADATA_CTRUE		'1' /* 0x31 (49) */
#define LUADATA_CDOUBLE		'D' 
#define LUADATA_CINT		'N' 
#define LUADATA_CTINYINT	'n' 
#define LUADATA_CSTRING		'S' /* 0x53 (83) */
#define LUADATA_CTABLE		'T' /* 0x54 (84) */
#define LUADATA_CENDTABLE	'E' /* 0x45 (69) */

#define LUADATA_MAXSUFFIXLEN 64

//---------------------------------------------------------------------------
using namespace luabind;

// only allow alphanumeric and underscore
void ValidateSuffix( const char *suffix, char *valid )
{
	int len = Q_strlen( suffix );
	int validlen = 0;
	for( int i=0; i<len && validlen<LUADATA_MAXSUFFIXLEN-1; i++ )
	{
		if (isalnum( suffix[i] ) || suffix[i] == '_')
		{
			valid[validlen] = suffix[i];
			validlen++;
		}
	}
	valid[validlen] = 0;
}

//---------------------------------------------------------------------------
// FFLib Namespace
//---------------------------------------------------------------------------
namespace FFLib
{

	bool LuaData_SaveValue( const luabind::adl::object &data, CUtlBuffer &buffer );
	bool LuaData_SaveTable( const luabind::adl::object &data, CUtlBuffer &buffer );

	// general write lua object to file as binary
	bool WriteData( const luabind::adl::object &data, const char *filename=NULL )
	{
		if (!filename)
			return false;

		IBaseFileSystem *filesystem = *pFilesystem;

		char path[ 512 ];
		Q_strncpy( path, filename, sizeof( path ) );
		Q_StripFilename( path );

		((IFileSystem *)filesystem)->CreateDirHierarchy( path, "MOD" );

		if ( ((IFileSystem *)filesystem)->FileExists( filename, "MOD" ) && 
			!((IFileSystem *)filesystem)->IsFileWritable( filename, "MOD" ) )
		{
			((IFileSystem *)filesystem)->SetFileWritable( filename, true, "MOD" );
		}
		
		// create a write file
		FileHandle_t f = filesystem->Open(filename, "wb");

		if ( f == FILESYSTEM_INVALID_HANDLE )
		{
			Msg("[SCRIPT] Error saving data (File \"%s\" could not be opened for writing)\n",
				   filename?filename:"NULL");
			return false;
		}

		CUtlBuffer buf( 0, 0, false );
		bool result = LuaData_SaveValue( data, buf );

		filesystem->Write( buf.Base(), buf.TellPut(), f );
		filesystem->Close(f);

		return result;
	}

	// write a non-table value to the buffer
	bool LuaData_SaveValue( const luabind::adl::object &data, CUtlBuffer &buffer )
	{
		if ( buffer.IsText() ) // must be a binary buffer
			return false;

		if ( !buffer.IsValid() ) // must be valid, no overflows etc
			return false;

		if ( !data.is_valid() )
			return false;

		// write type
		switch ( luabind::type( data ) )
		{
		case LUA_TNIL:
			{
				buffer.PutUnsignedChar( LUADATA_CNIL );
				break;
			}
		case LUA_TBOOLEAN:
			{
				buffer.PutUnsignedChar( (luabind::object_cast<bool>(data)) ? LUADATA_CTRUE : LUADATA_CFALSE );
				break;
			}
		case LUA_TNUMBER:
			{
				double num = luabind::object_cast<double>(data);
				// if there are no decimals, then we don't need to store it as a double
				if (num == static_cast<int>(num))
				{
					// if its small enough, just store it as an unsigned char
					if ( num >= 0 && num < 256 )
					{
						buffer.PutUnsignedChar( LUADATA_CTINYINT );
						buffer.PutUnsignedChar( (unsigned char)num );
					}
					else
					{
						buffer.PutUnsignedChar( LUADATA_CINT );
						buffer.PutInt( (int)num );
					}
				}
				else
				{
					buffer.PutUnsignedChar( LUADATA_CDOUBLE );
					buffer.PutDouble( num );
				}
				break;
			}
		case LUA_TSTRING:
			{
				buffer.PutUnsignedChar( LUADATA_CSTRING );
				buffer.PutString( luabind::object_cast<const char *>(data) );
				break;
			}
		case LUA_TTABLE:
			{
				LuaData_SaveTable( data, buffer );
				break;
			}
		default:
			break;
		}

		return buffer.IsValid();
	}

	// write a table to the buffer
	bool LuaData_SaveTable( const luabind::adl::object &data, CUtlBuffer &buffer )
	{
		if ( buffer.IsText() ) // must be a binary buffer
			return false;

		if ( !buffer.IsValid() ) // must be valid, no overflows etc
			return false;

		if ( !data.is_valid() || luabind::type( data ) != LUA_TTABLE )
			return false;

		buffer.PutUnsignedChar( LUADATA_CTABLE );

		// Iterate through the table
		for( iterator ib( data ), ie; ib != ie; ++ib )
		{
			luabind::adl::object key = ib.key();
			luabind::adl::object val = *ib;

			LuaData_SaveValue( key, buffer );
			LuaData_SaveValue( val, buffer );
		}
		
		buffer.PutUnsignedChar( LUADATA_CENDTABLE );

		return buffer.IsValid();
	}
	
	bool LuaData_LoadValue( luabind::adl::object &data, CUtlBuffer &buffer );
	bool LuaData_LoadTable( luabind::adl::object &data, CUtlBuffer &buffer );
	
	// general load lua object from a formatted binary file
	luabind::adl::object ReadData( const char *filename )
	{
		//lua_State* L = _scriptman.GetLuaState();

		IBaseFileSystem *filesystem = *pFilesystem;
		
		// load the file for reading
		FileHandle_t f = filesystem->Open(filename, "rb");

		if ( f == FILESYSTEM_INVALID_HANDLE )
		{
			Msg("[SCRIPT] Error loading data (File \"%s\" could not be opened for writing)\n", 
				filename?filename:"NULL" );
			return luabind::adl::object();
		}

		CUtlBuffer buf(0,0,false);

		((IFileSystem *)filesystem)->ReadToBuffer( f, buf );
		
		filesystem->Close( f );	// close file after reading

		luabind::adl::object data;
		//bool result = LuaData_LoadValue( data, buf );
		LuaData_LoadValue( data, buf );

		return data;
	}
	
	// load a non-table lua value from the buffer
	bool LuaData_LoadValue( luabind::adl::object &data, CUtlBuffer &buffer )
	{
		if ( buffer.IsText() ) // must be a binary buffer
			return false;

		if ( !buffer.IsValid() ) // must be valid, no overflows etc
			return false;
		
		lua_State* L = _scriptman.GetLuaState();

		unsigned char type = buffer.GetUnsignedChar();
		switch ( type )
		{
		case LUADATA_CNIL:
			{
				data = luabind::adl::object();
				break;
			}
		case LUADATA_CTRUE:
			{
				data = luabind::adl::object(L, true);
				break;
			}
		case LUADATA_CFALSE:
			{
				data = luabind::adl::object(L, false);
				break;
			}
		case LUADATA_CINT:
			{
				int i = buffer.GetInt();
				data = luabind::adl::object(L, i);
				break;
			}
		case LUADATA_CTINYINT:
			{
				int ti = (int)buffer.GetUnsignedChar();
				data = luabind::adl::object(L, ti);
				break;
			}
		case LUADATA_CDOUBLE:
			{
				double dbl = buffer.GetDouble();
				data = luabind::adl::object(L, dbl);
				break;
			}
		case LUADATA_CSTRING:
			{
				char in[4096];
				buffer.GetString( in );
				std::string str = in;
				data = luabind::adl::object(L, str);
				break;
			}
		case LUADATA_CTABLE:
			{
				data = luabind::newtable(L);
				LuaData_LoadTable( data, buffer );
				break;
			}
		default:
			Msg("[SCRIPT] Error loading data (Data format is not recognized)\n");
			return false;
			break;
		}

		return buffer.IsValid();
	}

	// load a lua table from the buffer
	bool LuaData_LoadTable( luabind::adl::object &data, CUtlBuffer &buffer )
	{
		if ( buffer.IsText() ) // must be a binary buffer
			return false;

		if ( !buffer.IsValid() ) // must be valid, no overflows etc
			return false;

		if ( luabind::type( data ) != LUA_TTABLE )
			return false;

		while ((const unsigned char)(*((const char*)buffer.PeekGet( sizeof(char), 0 ))) != LUADATA_CENDTABLE)
		{
			luabind::adl::object key;
			luabind::adl::object val;
			LuaData_LoadValue( key, buffer );
			LuaData_LoadValue( val, buffer );

			data[ key ] = val;
		}
		
		unsigned char c = buffer.GetUnsignedChar();
		if (c != LUADATA_CENDTABLE)
			return false;

		return buffer.IsValid();
	}

//***********************************
// Bound functions
//***********************************

	bool SaveMapData( const luabind::adl::object &data )
	{
		return WriteData( data, UTIL_VarArgs( "maps/data/%s.luadat", STRING(gpGlobals->mapname) ) );
	}

	bool SaveMapData( const luabind::adl::object &data, const char *suffix )
	{
		char validSuffix[LUADATA_MAXSUFFIXLEN];
		ValidateSuffix( suffix, validSuffix );
		// if the validated suffix string is blank
		if (!validSuffix[0])
		{
			Msg("[SCRIPT] Error saving data to %s_%s.luadat (Suffix must contain alphanumeric characters only)\n",
				   STRING(gpGlobals->mapname), suffix);
			return false;
		}

		return WriteData( data, UTIL_VarArgs( "maps/data/%s_%s.luadat", STRING(gpGlobals->mapname), validSuffix ) );
	}

	luabind::adl::object LoadMapData()
	{
		return ReadData( UTIL_VarArgs( "maps/data/%s.luadat", STRING(gpGlobals->mapname) ) );
	}

	luabind::adl::object LoadMapData(const char *suffix)
	{
		char validSuffix[LUADATA_MAXSUFFIXLEN];
		ValidateSuffix( suffix, validSuffix );
		// if the validated suffix string is blank
		if (!validSuffix[0])
		{
			Msg("[SCRIPT] Error loading data from %s_%s.luadat (Suffix must contain alphanumeric characters only)\n",
				   STRING(gpGlobals->mapname), suffix);
			return luabind::adl::object();
		}

		return ReadData( UTIL_VarArgs( "maps/data/%s_%s.luadat", STRING(gpGlobals->mapname), validSuffix ) );
	}
	
	bool SaveData( const luabind::adl::object &data )
	{
		return WriteData( data, "maps/data/global/global.luadat" );
	}

	bool SaveData( const luabind::adl::object &data, const char *suffix )
	{
		char validSuffix[LUADATA_MAXSUFFIXLEN];
		ValidateSuffix( suffix, validSuffix );
		// if the validated suffix string is blank
		if (!validSuffix[0])
		{
			Msg("[SCRIPT] Error saving data to global_%s.luadat (Suffix must contain alphanumeric characters only)\n",
				   suffix);
			return false;
		}

		return WriteData( data, UTIL_VarArgs( "maps/data/global/global_%s.luadat", validSuffix ) );
	}

	luabind::adl::object LoadData()
	{
		return ReadData( "maps/data/global/global.luadat" );
	}

	luabind::adl::object LoadData( const char *suffix )
	{
		char validSuffix[LUADATA_MAXSUFFIXLEN];
		ValidateSuffix( suffix, validSuffix );
		// if the validated suffix string is blank
		if (!validSuffix[0])
		{
			Msg("[SCRIPT] Error loading data from global_%s.luadat (Suffix must contain alphanumeric characters only)\n",
				   suffix);
			return luabind::adl::object();
		}

		return ReadData( UTIL_VarArgs( "maps/data/global/global_%s.luadat", validSuffix ) );
	}

} // namespace FFLib

//---------------------------------------------------------------------------
void CFFLuaLib::InitData(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		def("LoadMapData",		(luabind::adl::object(*)())&FFLib::LoadMapData),
		def("LoadMapData",		(luabind::adl::object(*)(const char *))&FFLib::LoadMapData),
		def("SaveMapData",		(bool(*)(const luabind::adl::object&))&FFLib::SaveMapData),
		def("SaveMapData",		(bool(*)(const luabind::adl::object&, const char *))&FFLib::SaveMapData),
		def("LoadGlobalData",	(luabind::adl::object(*)())&FFLib::LoadData),
		def("LoadGlobalData",	(luabind::adl::object(*)(const char *))&FFLib::LoadData),
		def("SaveGlobalData",	(bool(*)(const luabind::adl::object&))&FFLib::SaveData),
		def("SaveGlobalData",	(bool(*)(const luabind::adl::object&, const char *))&FFLib::SaveData)
	];
};
