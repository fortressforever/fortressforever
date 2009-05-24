//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//


#include "dt_send.h"
#include "mathlib.h"
#include "vector.h"
#include "tier0/dbg.h"
#include "dt_utlvector_common.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if !defined(_STATIC_LINKED) || defined(GAME_DLL)


static CNonModifiedPointerProxy *s_pNonModifiedPointerProxyHead = NULL;


void SendProxy_UInt8ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
void SendProxy_UInt16ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
void SendProxy_UInt32ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);

char *s_ElementNames[MAX_ARRAY_ELEMENTS] =
{
	"000", "001", "002", "003", "004", "005", "006", "007", "008", "009", 
	"010", "011", "012", "013", "014", "015", "016", "017", "018", "019",
	"020", "021", "022", "023", "024", "025", "026", "027", "028", "029",
	"030", "031", "032", "033", "034", "035", "036", "037", "038", "039",
	"040", "041", "042", "043", "044", "045", "046", "047", "048", "049",
	"050", "051", "052", "053", "054", "055", "056", "057", "058", "059",
	"060", "061", "062", "063", "064", "065", "066", "067", "068", "069",
	"070", "071", "072", "073", "074", "075", "076", "077", "078", "079",
	"080", "081", "082", "083", "084", "085", "086", "087", "088", "089",
	"090", "091", "092", "093", "094", "095", "096", "097", "098", "099",
	"100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
	"110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
	"120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
	"130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
	"140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
	"150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
	"160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
	"170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
	"180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
	"190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
	"200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
	"210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
	"220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
	"230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
	"240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
	"250", "251", "252", "253", "254", "255"
};


CNonModifiedPointerProxy::CNonModifiedPointerProxy( SendTableProxyFn fn )
{
	m_pNext = s_pNonModifiedPointerProxyHead;
	s_pNonModifiedPointerProxyHead = this;
	m_Fn = fn;
}


CStandardSendProxiesV1::CStandardSendProxiesV1()
{
	m_Int8ToInt32 = SendProxy_Int8ToInt32;
	m_Int16ToInt32 = SendProxy_Int16ToInt32;
	m_Int32ToInt32 = SendProxy_Int32ToInt32;

	m_UInt8ToInt32 = SendProxy_UInt8ToInt32;
	m_UInt16ToInt32 = SendProxy_UInt16ToInt32;
	m_UInt32ToInt32 = SendProxy_UInt32ToInt32;
	
	m_FloatToFloat = SendProxy_FloatToFloat;
	m_VectorToVector = SendProxy_VectorToVector;
}

CStandardSendProxies::CStandardSendProxies()
{	
	m_DataTableToDataTable = SendProxy_DataTableToDataTable;
	m_SendLocalDataTable = SendProxy_SendLocalDataTable;
	m_ppNonModifiedPointerProxies = &s_pNonModifiedPointerProxyHead;
	
}
CStandardSendProxies g_StandardSendProxies;


// ---------------------------------------------------------------------- //
// Proxies.
// ---------------------------------------------------------------------- //
void SendProxy_AngleToFloat( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	float angle;

	angle = *((float*)pData);
	pOut->m_Float = anglemod( angle );

	Assert( IsFinite( pOut->m_Float ) );
}

void SendProxy_FloatToFloat( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Float = *((float*)pData);
	Assert( IsFinite( pOut->m_Float ) );
}

void SendProxy_QAngles( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	QAngle *v = (QAngle*)pData;
	pOut->m_Vector[0] = anglemod( v->x );
	pOut->m_Vector[1] = anglemod( v->y );
	pOut->m_Vector[2] = anglemod( v->z );
}

void SendProxy_VectorToVector( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	Vector& v = *(Vector*)pData;
	Assert( v.IsValid() );
	pOut->m_Vector[0] = v[0];
	pOut->m_Vector[1] = v[1];
	pOut->m_Vector[2] = v[2];
}

#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!
void SendProxy_QuaternionToQuaternion( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	Quaternion& q = *(Quaternion*)pData;
	Assert( q.IsValid() );
	pOut->m_Vector[0] = q[0];
	pOut->m_Vector[1] = q[1];
	pOut->m_Vector[2] = q[2];
	pOut->m_Vector[3] = q[3];
}
#endif

void SendProxy_Int8ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((char*)pData);
}

void SendProxy_Int16ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((short*)pData);
}

void SendProxy_Int32ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((int*)pData);
}

void SendProxy_UInt8ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((unsigned char*)pData);
}

void SendProxy_UInt16ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((unsigned short*)pData);
}

void SendProxy_UInt32ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	*((unsigned long*)&pOut->m_Int) = *((unsigned long*)pData);
}

void SendProxy_StringToString( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_pString = (char*)pData;
}

void* SendProxy_DataTableToDataTable( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	return (void*)pData;
}

void* SendProxy_DataTablePtrToDataTable( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	return *((void**)pData);
}

static void SendProxy_Empty( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
}

//-----------------------------------------------------------------------------
// Purpose: If the recipient is the same as objectID, go ahead and iterate down
//  the m_Local stuff, otherwise, act like it wasn't there at all.
// This way, only the local player receives information about him/herself.
// Input  : *pVarData - 
//			*pOut - 
//			objectID - 
//-----------------------------------------------------------------------------

void* SendProxy_SendLocalDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetOnly( objectID - 1 );
	return ( void * )pVarData;
}





// ---------------------------------------------------------------------- //
// Prop setup functions (for building tables).
// ---------------------------------------------------------------------- //
float AssignRangeMultiplier( int nBits, double range )
{
	unsigned long iHighValue;
	if ( nBits == 32 )
		iHighValue = 0xFFFFFFFE;
	else
		iHighValue = ((1 << (unsigned long)nBits) - 1);

	float fHighLowMul = iHighValue / range;
	
	// If the precision is messing us up, then adjust it so it won't.
	if ( (unsigned long)(fHighLowMul * range) > iHighValue ||
		 (fHighLowMul * range) > (double)iHighValue )
	{
		// Squeeze it down smaller and smaller until it's going to produce an integer
		// in the valid range when given the highest value.
		float multipliers[] = { 0.9999, 0.99, 0.9, 0.8, 0.7 };
		int i;
		for ( i=0; i < ARRAYSIZE( multipliers ); i++ )
		{
			float fHighLowMul = (float)( iHighValue / range ) * multipliers[i];
			if ( (unsigned long)(fHighLowMul * range) > iHighValue ||
				(fHighLowMul * range) > (double)iHighValue )
			{
			}
			else
			{
				break;
			}
		}

		if ( i == ARRAYSIZE( multipliers ) )
		{
			// Doh! We seem to be unable to represent this range.
			Assert( false );
			return 0;
		}
	}

	return fHighLowMul;
}



SendProp SendPropFloat(
	char *pVarName,		
	// Variable name.
	int offset,			// Offset into container structure.
	int sizeofVar,
	int nBits,			// Number of bits to use when encoding.
	int flags,
	float fLowValue,		// For floating point, low and high values.
	float fHighValue,		// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if ( varProxy == SendProxy_FloatToFloat )
	{
		Assert( sizeofVar == 0 || sizeofVar == 4 );
	}

	if(fHighValue == HIGH_DEFAULT)
		fHighValue = (1 << nBits);

	if (flags & SPROP_ROUNDDOWN)
		fHighValue = fHighValue - ((fHighValue - fLowValue) / (1 << nBits));
	else if (flags & SPROP_ROUNDUP)
		fLowValue = fLowValue + ((fHighValue - fLowValue) / (1 << nBits));

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Float;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = fLowValue;
	ret.m_fHighValue = fHighValue;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & (SPROP_COORD | SPROP_NOSCALE | SPROP_NORMAL) )
		ret.m_nBits = 0;

	return ret;
}

SendProp SendPropVector(
	char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,					// Number of bits to use when encoding.
	int flags,
	float fLowValue,			// For floating point, low and high values.
	float fHighValue,			// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_VectorToVector)
	{
		Assert(sizeofVar == sizeof(Vector));
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Vector;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = fLowValue;
	ret.m_fHighValue = fHighValue;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & (SPROP_COORD | SPROP_NOSCALE | SPROP_NORMAL) )
		ret.m_nBits = 0;

	return ret;
}

#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!
SendProp SendPropQuaternion(
	char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,					// Number of bits to use when encoding.
	int flags,
	float fLowValue,			// For floating point, low and high values.
	float fHighValue,			// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_QuaternionToQuaternion)
	{
		Assert(sizeofVar == sizeof(Quaternion));
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Quaternion;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = fLowValue;
	ret.m_fHighValue = fHighValue;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & (SPROP_COORD | SPROP_NOSCALE | SPROP_NORMAL) )
		ret.m_nBits = 0;

	return ret;
}
#endif

SendProp SendPropAngle(
	char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,
	int flags,
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_AngleToFloat)
	{
		Assert(sizeofVar == 4);
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Float;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = 0.0f;
	ret.m_fHighValue = 360.0f;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );

	return ret;
}


SendProp SendPropQAngles(
	char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,
	int flags,
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_AngleToFloat)
	{
		Assert(sizeofVar == 4);
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Vector;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = 0.0f;
	ret.m_fHighValue = 360.0f;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );

	ret.SetProxyFn( varProxy );

	return ret;
}
  
SendProp SendPropInt(
	char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,
	int flags,
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if ( !varProxy )
	{
		if ( sizeofVar == 1 )
		{
			varProxy = SendProxy_Int8ToInt32;
		}
		else if ( sizeofVar == 2 )
		{
			varProxy = SendProxy_Int16ToInt32;
		}
		else if ( sizeofVar == 4 )
		{
			varProxy = SendProxy_Int32ToInt32;
		}
		else
		{
			Assert(!"SendPropInt var has invalid size");
			varProxy = SendProxy_Int8ToInt32;	// safest one...
		}
	}

	// Figure out # of bits if the want us to.
	if ( nBits <= 0 )
	{
		Assert( sizeofVar == 1 || sizeofVar == 2 || sizeofVar == 4 );
		nBits = sizeofVar * 8;
	}

	ret.m_Type = DPT_Int;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );

	// Use UInt proxies if they want unsigned data. This isn't necessary to encode
	// the values correctly, but it lets us check the ranges of the data to make sure
	// they're valid.
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & SPROP_UNSIGNED )
	{
		if( varProxy == SendProxy_Int8ToInt32 )
			ret.SetProxyFn( SendProxy_UInt8ToInt32 );
		
		else if( varProxy == SendProxy_Int16ToInt32 )
			ret.SetProxyFn( SendProxy_UInt16ToInt32 );

		else if( varProxy == SendProxy_Int32ToInt32 )
			ret.SetProxyFn( SendProxy_UInt32ToInt32 );
	}

	return ret;
}

SendProp SendPropString(
	char *pVarName,
	int offset,
	int bufferLen,
	int flags,
	SendVarProxyFn varProxy)
{
	SendProp ret;

	Assert( bufferLen <= DT_MAX_STRING_BUFFERSIZE ); // You can only have strings with 8-bits worth of length.
	
	ret.m_Type = DPT_String;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.SetFlags( flags );
	ret.SetProxyFn( varProxy );

	return ret;
}

SendProp SendPropArray3(
	char *pVarName,
	int offset,
	int sizeofVar,
	int elements,
	SendProp pArrayProp,
	SendTableProxyFn varProxy
	)
{
	SendProp ret;

	Assert( elements <= MAX_ARRAY_ELEMENTS );

	ret.m_Type = DPT_DataTable;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.SetDataTableProxyFn( varProxy );
	
	SendProp *pArrayPropAllocated = new SendProp;
	*pArrayPropAllocated = pArrayProp;
	ret.SetArrayProp( pArrayPropAllocated );
	
	// Handle special proxy types where they always let all clients get the results.
	if ( varProxy == SendProxy_DataTableToDataTable || varProxy == SendProxy_DataTablePtrToDataTable )
	{
		ret.SetFlags( SPROP_PROXY_ALWAYS_YES );
	}

	SendProp *pProps = new SendProp[elements]; // TODO free that again
	
	char *pParentArrayPropName = AllocateStringHelper( "%s", pVarName );

	for ( int i = 0; i < elements; i++ )
	{
		pProps[i] = pArrayProp;	// copy array element property setting
		pProps[i].SetOffset( i*sizeofVar ); // adjust offset
		pProps[i].m_pVarName = s_ElementNames[i];	// give unique name
		pProps[i].SetParentArrayPropName( pParentArrayPropName ); // For debugging...
	}

	SendTable *pTable = new SendTable( pProps, elements, pVarName ); // TODO free that again

	ret.SetDataTable( pTable );

	return ret;
}

SendProp SendPropDataTable(
	char *pVarName,
	int offset,
	SendTable *pTable,
	SendTableProxyFn varProxy
	)
{
	SendProp ret;

	ret.m_Type = DPT_DataTable;
	ret.m_pVarName = pVarName;
	ret.SetOffset( offset );
	ret.SetDataTable( pTable );
	ret.SetDataTableProxyFn( varProxy );
	
	// Handle special proxy types where they always let all clients get the results.
	if ( varProxy == SendProxy_DataTableToDataTable || varProxy == SendProxy_DataTablePtrToDataTable )
	{
		ret.SetFlags( SPROP_PROXY_ALWAYS_YES );
	}
	
	if ( varProxy == SendProxy_DataTableToDataTable && offset == 0 )
	{
		ret.SetFlags( SPROP_COLLAPSIBLE );
	}

	return ret;
}


SendProp InternalSendPropArray(
	const int elementCount,
	const int elementStride,
	char *pName,
	ArrayLengthSendProxyFn arrayLengthFn
	)
{
	SendProp ret;

	ret.m_Type = DPT_Array;
	ret.m_nElements = elementCount;
	ret.m_ElementStride = elementStride;
	ret.m_pVarName = pName;
	ret.SetProxyFn( SendProxy_Empty );
	ret.m_pArrayProp = NULL;	// This gets set in SendTable_InitTable. It always points at the property that precedes
								// this one in the datatable's list.
	ret.SetArrayLengthProxy( arrayLengthFn );
		
	return ret;
}


SendProp SendPropExclude(
	char *pDataTableName,	// Data table name (given to BEGIN_SEND_TABLE and BEGIN_RECV_TABLE).
	char *pPropName		// Name of the property to exclude.
	)
{
	SendProp ret;

	ret.SetFlags( SPROP_EXCLUDE );
	ret.m_pExcludeDTName = pDataTableName;
	ret.m_pVarName = pPropName;

	return ret;
}



// ---------------------------------------------------------------------- //
// SendProp
// ---------------------------------------------------------------------- //
SendProp::SendProp()
{
	m_pVarName = NULL;
	m_Offset = 0;
	m_pDataTable = NULL;
	m_ProxyFn = NULL;
	m_pExcludeDTName = NULL;
	
	m_Type = DPT_Int;
	m_Flags = 0;
	m_nBits = 0;

	m_fLowValue = 0.0f;
	m_fHighValue = 0.0f;
	m_pArrayProp = 0;
	m_ArrayLengthProxy = 0;
	m_nElements = 1;
	m_ElementStride = -1;
}


SendProp::~SendProp()
{
}


int SendProp::GetNumArrayLengthBits() const
{
	Assert( GetType() == DPT_Array );
	return Q_log2( GetNumElements() ) + 1;
}


// ---------------------------------------------------------------------- //
// SendTable
// ---------------------------------------------------------------------- //
SendTable::SendTable()
{
	Construct( NULL, 0, NULL );
}


SendTable::SendTable(SendProp *pProps, int nProps, char *pNetTableName)
{
	Construct( pProps, nProps, pNetTableName );
}


SendTable::~SendTable()
{
//	Assert( !m_pPrecalc );
}


void SendTable::Construct( SendProp *pProps, int nProps, char *pNetTableName )
{
	m_pProps = pProps;
	m_nProps = nProps;
	m_pNetTableName = pNetTableName;
	m_pPrecalc = 0;
	m_bInitialized = false;
	m_bHasBeenWritten = false;
}

#endif
