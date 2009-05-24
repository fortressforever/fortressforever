//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include <stdlib.h>
#include <ctype.h>
#include "UtlBuffer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Advances until non-whitespace hit
//-----------------------------------------------------------------------------
wchar_t *AdvanceOverWhitespace(wchar_t *Start)
{
	while (*Start != 0 && iswspace(*Start))
	{
		Start++;
	}

	return Start;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
wchar_t *ReadUnicodeToken(wchar_t *start, wchar_t *token, int tokenBufferSize, bool &quoted)
{
	// skip over any whitespace
	start = AdvanceOverWhitespace(start);
	quoted = false;
	*token = 0;

	if (!*start)
	{
		return start;
	}

	// check to see if it's a quoted string
	if (*start == '\"')
	{
		quoted = true;
		// copy out the string until we hit an end quote
		start++;
		int count = 0;
		while (*start && *start != '\"' && count < tokenBufferSize)
		{
			// check for special characters
			if (*start == '\\' && *(start+1) == 'n')
			{
				start++;
				*token = '\n';
			}
			else if (*start == '\\' && *(start+1) == '\"')
			{
				start++;
				*token = '\"';
			}
			else
			{
				*token = *start;
			}

			start++;
			token++;
			count++;
		}

		if (*start == '\"')
		{
			start++;
		}
	}
	else
	{
		// copy out the string until we hit a whitespace
		int count = 0;
		while (*start && !iswspace(*start) && count < tokenBufferSize)
		{
			// no checking for special characters if it's not a quoted string
			*token = *start;

			start++;
			token++;
			count++;
		}
	}

	*token = 0;
	return start;
}

//-----------------------------------------------------------------------------
// Purpose: Same as above but no translation of \n
//-----------------------------------------------------------------------------
wchar_t *ReadUnicodeTokenNoSpecial(wchar_t *start, wchar_t *token, int tokenBufferSize, bool &quoted)
{
	// skip over any whitespace
	start = AdvanceOverWhitespace(start);
	quoted = false;
	*token = 0;

	if (!*start)
	{
		return start;
	}

	// check to see if it's a quoted string
	if (*start == '\"')
	{
		quoted = true;
		// copy out the string until we hit an end quote
		start++;
		int count = 0;
		while (*start && *start != '\"' && count < tokenBufferSize)
		{
			// check for special characters
			/*
			if (*start == '\\' && *(start+1) == 'n')
			{
				start++;
				*token = '\n';
			}
			else
			*/
			if (*start == '\\' && *(start+1) == '\"')
			{
				start++;
				*token = '\"';
			}
			else
			{
				*token = *start;
			}

			start++;
			token++;
			count++;
		}

		if (*start == '\"')
		{
			start++;
		}
	}
	else
	{
		// copy out the string until we hit a whitespace
		int count = 0;
		while (*start && !iswspace(*start) && count < tokenBufferSize)
		{
			// no checking for special characters if it's not a quoted string
			*token = *start;

			start++;
			token++;
			count++;
		}
	}

	*token = 0;
	return start;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the first character after the next EOL characters
//-----------------------------------------------------------------------------
wchar_t *ReadToEndOfLine(wchar_t *start)
{
	if (!*start)
		return start;

	while (*start)
	{
		if (*start == 0x0D || *start== 0x0A)
			break;
		start++;
	}

	while (*start == 0x0D || *start== 0x0A)
		start++;

	return start;
}

//-----------------------------------------------------------------------------
// Purpose: file writing
//-----------------------------------------------------------------------------
void WriteUnicodeString(CUtlBuffer &buf, const wchar_t *string, bool addQuotes)
{
	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}

	for (const wchar_t *ws = string; *ws != 0; ws++)
	{
		// handle special characters
		if (addQuotes && *ws == '\"')
		{
			buf.PutUnsignedShort('\\');
		}
		// write the character
		buf.PutUnsignedShort(*ws);
	}

	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}
}

//-----------------------------------------------------------------------------
// Purpose: file writing
//-----------------------------------------------------------------------------
void WriteAsciiStringAsUnicode(CUtlBuffer &buf, const char *string, bool addQuotes)
{
	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}

	for (const char *sz = string; *sz != 0; sz++)
	{
		// handle special characters
		if (addQuotes && *sz == '\"')
		{
			buf.PutUnsignedShort('\\');
		}
		buf.PutUnsignedShort(*sz);
	}

	if (addQuotes)
	{
		buf.PutUnsignedShort('\"');
	}
}
