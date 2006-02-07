//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "filesystem.h"
#include "sentence.h"
#include "hud_closecaption.h"
#include "engine/ivmodelinfo.h"
#include "engine/ivdebugoverlay.h"
#include "bone_setup.h"
#include "soundinfo.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar g_CV_PhonemeDelay("phonemedelay", "0", 0, "Phoneme delay to account for sound system latency." );
ConVar g_CV_PhonemeFilter("phonemefilter", "0.08", 0, "Time duration of box filter to pass over phonemes." );
ConVar g_CV_FlexRules("flex_rules", "1", 0, "Allow flex animation rules to run." );
ConVar g_CV_BlinkDuration("blink_duration", "0.2", 0, "How many seconds an eye blink will last." );
ConVar g_CV_FlexSmooth("flex_smooth", "1", 0, "Applies smoothing/decay curve to flex animation controller changes." );


IMPLEMENT_CLIENTCLASS_DT(C_BaseFlex, DT_BaseFlex, CBaseFlex)
	RecvPropArray3( RECVINFO_ARRAY(m_flexWeight), RecvPropFloat(RECVINFO(m_flexWeight[0]))),
	RecvPropInt(RECVINFO(m_blinktoggle)),
	RecvPropVector(RECVINFO(m_viewtarget)),

#ifdef HL2_CLIENT_DLL
	RecvPropFloat( RECVINFO(m_vecViewOffset[0]) ),
	RecvPropFloat( RECVINFO(m_vecViewOffset[1]) ),
	RecvPropFloat( RECVINFO(m_vecViewOffset[2]) ),
#endif

END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseFlex )

/*
	// DEFINE_FIELD( C_BaseFlex, m_viewtarget, FIELD_VECTOR ),
	// DEFINE_ARRAY( C_BaseFlex, m_flexWeight, FIELD_FLOAT, 64 ),
	// DEFINE_FIELD( C_BaseFlex, m_blinktoggle, FIELD_INTEGER ),
	// DEFINE_FIELD( C_BaseFlex, m_blinktime, FIELD_FLOAT ),
	// DEFINE_FIELD( C_BaseFlex, m_prevviewtarget, FIELD_VECTOR ),
	// DEFINE_ARRAY( C_BaseFlex, m_prevflexWeight, FIELD_FLOAT, 64 ),
	// DEFINE_FIELD( C_BaseFlex, m_prevblinktoggle, FIELD_INTEGER ),
	// DEFINE_FIELD( C_BaseFlex, m_iBlink, FIELD_INTEGER ),
	// DEFINE_FIELD( C_BaseFlex, m_iEyeUpdown, FIELD_INTEGER ),
	// DEFINE_FIELD( C_BaseFlex, m_iEyeRightleft, FIELD_INTEGER ),
	// DEFINE_FIELD( C_BaseFlex, m_FileList, CUtlVector < CFlexSceneFile * > ),
*/

END_PREDICTION_DATA()

C_BaseFlex::C_BaseFlex() : m_iv_viewtarget( "C_BaseFlex::m_iv_viewtarget" ), m_iv_flexWeight("C_BaseFlex:m_iv_flexWeight" )
{
#ifdef _DEBUG
	((Vector&)m_viewtarget).Init();
#endif

	AddVar( &m_viewtarget, &m_iv_viewtarget, LATCH_ANIMATION_VAR | INTERPOLATE_LINEAR_ONLY );
	AddVar( m_flexWeight, &m_iv_flexWeight, LATCH_ANIMATION_VAR );

	// Fill in phoneme class lookup
	memset( m_PhonemeClasses, 0, sizeof( m_PhonemeClasses ) );

	Emphasized_Phoneme *weak = &m_PhonemeClasses[ PHONEME_CLASS_WEAK ];
	Q_strncpy( weak->classname, "phonemes_weak", sizeof( weak->classname ) );
	weak->required = false;
	Emphasized_Phoneme *normal = &m_PhonemeClasses[ PHONEME_CLASS_NORMAL ];
	Q_strncpy( normal->classname, "phonemes", sizeof( normal->classname ) );
	normal->required = true;
	Emphasized_Phoneme *strong = &m_PhonemeClasses[ PHONEME_CLASS_STRONG ];
	Q_strncpy( strong->classname, "phonemes_strong", sizeof( strong->classname ) );
	strong->required = false;

	m_flFlexDelayedWeight = NULL;

	/// Make sure size is correct
	Assert( PHONEME_CLASS_STRONG + 1 == NUM_PHONEME_CLASSES );
}

C_BaseFlex::~C_BaseFlex()
{
	delete[] m_flFlexDelayedWeight;
}

//-----------------------------------------------------------------------------
// Purpose: initialize fast lookups when model changes
//-----------------------------------------------------------------------------

studiohdr_t *C_BaseFlex::OnNewModel()
{
	studiohdr_t *hdr = BaseClass::OnNewModel();
	
	// init to invalid setting
	m_iBlink = -1;
	m_iEyeUpdown = -1;
	m_iEyeRightleft = -1;
	m_iMouthAttachment = 0;

	delete[] m_flFlexDelayedWeight;
	m_flFlexDelayedWeight = NULL;

	if (hdr)
	{
		if (hdr->numflexdesc)
		{
			m_flFlexDelayedWeight = new float [hdr->numflexdesc];

			for (int i = 0; i < hdr->numflexdesc; i++)
			{
				m_flFlexDelayedWeight[i] = 0.0;
			}
		}
		m_iMouthAttachment = Studio_FindAttachment( hdr, "mouth" ) + 1;
	}

	return hdr;
}


//-----------------------------------------------------------------------------
// Purpose: place "voice" sounds on mouth
//-----------------------------------------------------------------------------

bool C_BaseFlex::GetSoundSpatialization( SpatializationInfo_t& info )
{
	bool bret = BaseClass::GetSoundSpatialization( info );
	// Default things it's audible, put it at a better spot?
	if ( bret )
	{
		if (info.info.nChannel == CHAN_VOICE && m_iMouthAttachment > 0)
		{
			Vector origin;
			QAngle angles;
			
			C_BaseAnimating::PushAllowBoneAccess( true, false );

			if (GetAttachment( m_iMouthAttachment, origin, angles ))
			{
				if (info.pOrigin)
				{
					*info.pOrigin = origin;
				}

				if (info.pAngles)
				{
					*info.pAngles = angles;
				}
			}

			C_BaseAnimating::PopBoneAccess();
		}
	}

	return bret;
}


//-----------------------------------------------------------------------------
// Purpose: run the interpreted FAC's expressions, converting flex_controller 
//			values into FAC weights
//-----------------------------------------------------------------------------
void C_BaseFlex::RunFlexRules( float *dest )
{
	int i, j;

	if (!g_CV_FlexRules.GetInt())
		return;
	
	studiohdr_t *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	// FIXME: this shouldn't be needed, flex without rules should be stripped in studiomdl
	for (i = 0; i < hdr->numflexdesc; i++)
	{
		dest[i] = 0;
	}

	for (i = 0; i < hdr->numflexrules; i++)
	{
		float stack[32];
		int k = 0;
		mstudioflexrule_t *prule = hdr->pFlexRule( i );

		mstudioflexop_t *pops = prule->iFlexOp( 0 );

		// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), i + 1, 0, "%2d:%d\n", i, prule->flex );

		for (j = 0; j < prule->numops; j++)
		{
			switch (pops->op)
			{
			case STUDIO_ADD: stack[k-2] = stack[k-2] + stack[k-1]; k--; break;
			case STUDIO_SUB: stack[k-2] = stack[k-2] - stack[k-1]; k--; break;
			case STUDIO_MUL: stack[k-2] = stack[k-2] * stack[k-1]; k--; break;
			case STUDIO_DIV:
				if (stack[k-1] > 0.0001)
				{
					stack[k-2] = stack[k-2] / stack[k-1];
				}
				else
				{
					stack[k-2] = 0;
				}
				k--; 
				break;
			case STUDIO_CONST: stack[k] = pops->d.value; k++; break;
			case STUDIO_FETCH1: 
				{ 
				int m = hdr->pFlexcontroller(pops->d.index)->link;
				stack[k] = g_flexweight[m];
				k++; 
				break;
				}
			case STUDIO_FETCH2: stack[k] = dest[pops->d.index]; k++; break;
			}

			pops++;
		}
		dest[prule->flex] = stack[0];
	}
}

class CFlexSceneFileManager : CAutoGameSystem
{
public:

	virtual bool Init()
	{
		// Trakcer 16692:  Preload these at startup to avoid hitch first time we try to load them during actual gameplay
		FindSceneFile( "phonemes" );
		FindSceneFile( "phonemes_weak" );
		FindSceneFile( "phonemes_strong" );
		return true;
	}

	// Tracker 14992:  We used to load 18K of .vfes for every C_BaseFlex who lipsynced, but now we only load those files once globally.
	// Note, we could wipe these between levels, but they don't ever load more than the weak/normal/strong phoneme classes that I can tell
	//  so I'll just leave them loaded forever for now
	virtual void Shutdown()
	{
		DeleteSceneFiles();
	}

	void *FindSceneFile( const char *filename )
	{
		// See if it's already loaded
		int i;
		for ( i = 0; i < m_FileList.Size(); i++ )
		{
			CFlexSceneFile *file = m_FileList[ i ];
			if ( file && !stricmp( file->filename, filename ) )
			{
				return file->buffer;
			}
		}
		
		// Load file into memory
		FileHandle_t file = filesystem->Open( VarArgs( "expressions/%s.vfe", filename ), "rb" );

		if ( !file )
			return NULL;

		int len = filesystem->Size( file );

		// read the file
		byte *buffer = new unsigned char[ len ];
		Assert( buffer );
		filesystem->Read( buffer, len, file );
		filesystem->Close( file );

		// Create scene entry
		CFlexSceneFile *pfile = new CFlexSceneFile;
		// Remember filename
		Q_strncpy( pfile->filename, filename, sizeof( pfile->filename ) );
		// Remember data pointer
		pfile->buffer = buffer;
		// Add to list
		m_FileList.AddToTail( pfile );

		// Fill in translation table
		flexsettinghdr_t *pSettinghdr = ( flexsettinghdr_t * )pfile->buffer;
		Assert( pSettinghdr );
		for (i = 0; i < pSettinghdr->numkeys; i++)
		{
			*(pSettinghdr->pLocalToGlobal(i)) = C_BaseFlex::AddGlobalFlexController( pSettinghdr->pLocalName( i ) );
		}

		// Return data
		return pfile->buffer;
	}

private:

	void DeleteSceneFiles()
	{
		while ( m_FileList.Size() > 0 )
		{
			CFlexSceneFile *file = m_FileList[ 0 ];
			m_FileList.Remove( 0 );
			delete[] file->buffer;
			delete file;
		}
	}

	CUtlVector< CFlexSceneFile * > m_FileList;
};

CFlexSceneFileManager g_FlexSceneFileManager;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *filename - 
//-----------------------------------------------------------------------------
void *C_BaseFlex::FindSceneFile( const char *filename )
{
	return g_FlexSceneFileManager.FindSceneFile( filename );
}

//-----------------------------------------------------------------------------
// Purpose: make sure the eyes are within 30 degrees of forward
//-----------------------------------------------------------------------------
void C_BaseFlex::SetViewTarget( void )
{
  	if ( !GetModelPtr() )
  	{
  		return;
  	}

	// aim the eyes
	Vector tmp = m_viewtarget;

	if (m_iEyeUpdown == -1)
		m_iEyeUpdown = AddGlobalFlexController( "eyes_updown" );

	if (m_iEyeRightleft == -1)
		m_iEyeRightleft = AddGlobalFlexController( "eyes_rightleft" );

	if (m_iEyeAttachment > 0)
	{
		matrix3x4_t attToWorld;
		if (!GetAttachment( m_iEyeAttachment, attToWorld ))
		{
			return;
		}

		Vector local;
		VectorITransform( tmp, attToWorld, local );
		
		float flDist = local.Length();
		// FIXME: clamp distance to something based on eyeball distance
		if (flDist < 4.)
		{
			flDist = 4.0;
			local.Init( 0, 0, 1.0 );
		}

		VectorNormalize( local );

		// calculate animated eye deflection
		Vector eyeDeflect;
		QAngle eyeAng( 0, 0, 0 );
		if ( m_iEyeUpdown != -1)
		{
			eyeAng.x = g_flexweight[ m_iEyeUpdown ];
		}
		
		if ( m_iEyeRightleft != -1)
		{
			eyeAng.y = g_flexweight[ m_iEyeRightleft ];
		}

		// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 0, 0, "%5.3f %5.3f", eyeAng.x, eyeAng.y );

		AngleVectors( eyeAng, &eyeDeflect );
		eyeDeflect.x = 0;

		// reduce deflection the more the eye is off center
		// FIXME: this angles make no damn sense
		eyeDeflect = eyeDeflect * (local.x * local.x);
		local = local + eyeDeflect;
		VectorNormalize( local );

		// check to see if the eye is aiming outside a 30 degree cone
		if (local.x < 0.866) // cos(30)
		{
			// if so, clamp it to 30 degrees offset
			// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 1, 0, "%5.3f %5.3f %5.3f", local.x, local.y, local.z );
			local.x = 0;
			float d = local.LengthSqr();
			if (d > 0.0)
			{
				d = sqrtf( (1.0 - 0.866 * 0.866) / (local.y*local.y + local.z*local.z) );
				local.x = 0.866;
				local.y = local.y * d;
				local.z = local.z * d;
			}
			else
			{
				local.x = 1.0;
			}
		}
		local = local * flDist;
		VectorTransform( local, attToWorld, tmp );
	}

	modelrender->SetViewTarget( tmp );

	/*
	debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), 0, 0, "%.2f %.2f %.2f  : %.2f %.2f %.2f", 
		m_viewtarget.x, m_viewtarget.y, m_viewtarget.z, 
		m_prevviewtarget.x, m_prevviewtarget.y, m_prevviewtarget.z );
	*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void NewMarkovIndex( flexsetting_t *pSetting )
{
	if ( pSetting->type != FS_MARKOV )
		return;

	int weighttotal = 0;
	int member = 0;
	for (int i = 0; i < pSetting->numsettings; i++)
	{
		flexmarkovgroup_t *group = pSetting->pMarkovGroup( i );
		if ( !group )
			continue;

		weighttotal += group->weight;
		if ( !weighttotal || random->RandomInt(0,weighttotal-1) < group->weight )
		{
			member = i;
		}
	}

	pSetting->currentindex = member;
}

#define STRONG_CROSSFADE_START		0.60f
#define WEAK_CROSSFADE_START		0.40f

//-----------------------------------------------------------------------------
// Purpose: 
// Here's the formula
// 0.5 is neutral 100 % of the default setting
// Crossfade starts at STRONG_CROSSFADE_START and is full at STRONG_CROSSFADE_END
// If there isn't a strong then the intensity of the underlying phoneme is fixed at 2 x STRONG_CROSSFADE_START
//  so we don't get huge numbers
// Input  : *classes - 
//			emphasis_intensity - 
//-----------------------------------------------------------------------------
void C_BaseFlex::ComputeBlendedSetting( Emphasized_Phoneme *classes, float emphasis_intensity )
{
	// See which overrides are available for the current phoneme
	bool has_weak	= classes[ PHONEME_CLASS_WEAK ].valid;
	bool has_strong = classes[ PHONEME_CLASS_STRONG ].valid;

	// Better have phonemes in general
	Assert( classes[ PHONEME_CLASS_NORMAL ].valid );

	if ( emphasis_intensity > STRONG_CROSSFADE_START )
	{
		if ( has_strong )
		{
			// Blend in some of strong
			float dist_remaining = 1.0f - emphasis_intensity;
			float frac = dist_remaining / ( 1.0f - STRONG_CROSSFADE_START );

			classes[ PHONEME_CLASS_NORMAL ].amount = (frac) * 2.0f * STRONG_CROSSFADE_START;
			classes[ PHONEME_CLASS_STRONG ].amount = 1.0f - frac; 
		}
		else
		{
			emphasis_intensity = min( emphasis_intensity, STRONG_CROSSFADE_START );
			classes[ PHONEME_CLASS_NORMAL ].amount = 2.0f * emphasis_intensity;
		}
	}
	else if ( emphasis_intensity < WEAK_CROSSFADE_START )
	{
		if ( has_weak )
		{
			// Blend in some weak
			float dist_remaining = WEAK_CROSSFADE_START - emphasis_intensity;
			float frac = dist_remaining / ( WEAK_CROSSFADE_START );

			classes[ PHONEME_CLASS_NORMAL ].amount = (1.0f - frac) * 2.0f * WEAK_CROSSFADE_START;
			classes[ PHONEME_CLASS_WEAK ].amount = frac; 
		}
		else
		{
			emphasis_intensity = max( emphasis_intensity, WEAK_CROSSFADE_START );
			classes[ PHONEME_CLASS_NORMAL ].amount = 2.0f * emphasis_intensity;
		}
	}
	else
	{
		// Assume 0.5 (neutral) becomes a scaling of 1.0f
		classes[ PHONEME_CLASS_NORMAL ].amount = 2.0f * emphasis_intensity;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *classes - 
//			phoneme - 
//			scale - 
//			newexpression - 
//-----------------------------------------------------------------------------
void C_BaseFlex::AddViseme( Emphasized_Phoneme *classes, float emphasis_intensity, int phoneme, float scale, bool newexpression )
{
	int type;

	// Setup weights for any emphasis blends
	bool skip = SetupEmphasisBlend( classes, phoneme );
	// Uh-oh, missing or unknown phoneme???
	if ( skip )
	{
		return;
	}
		
	// Compute blend weights
	ComputeBlendedSetting( classes, emphasis_intensity );

	for ( type = 0; type < NUM_PHONEME_CLASSES; type++ )
	{
		Emphasized_Phoneme *info = &classes[ type ];
		if ( !info->valid || info->amount == 0.0f )
			continue;

		// Assume that we're not using overrieds
		const flexsettinghdr_t *actual_flexsetting_header = info->base;

		const flexsetting_t *pSetting = actual_flexsetting_header->pIndexedSetting( phoneme );
		if (!pSetting)
		{
			continue;
		}

		if ( newexpression )
		{
			if ( pSetting->type == FS_MARKOV )
			{
				NewMarkovIndex( ( flexsetting_t * )pSetting );
			}
		}

		// Determine its index
		int i = pSetting - actual_flexsetting_header->pSetting( 0 );
		Assert( i >= 0 );
		Assert( i < actual_flexsetting_header->numflexsettings );

		// Resolve markov chain for the returned setting, probably not an issue for visemes
		pSetting = actual_flexsetting_header->pTranslatedSetting( i );

		// Check for overrides
		if ( info->override )
		{
			// Get name from setting
			const char *resolvedName = pSetting->pszName();
			if ( resolvedName )
			{
				// See if resolvedName exists in the override file
				const flexsetting_t *override = FindNamedSetting( info->override, resolvedName );
				if ( override )
				{
					// If so, point at the override file instead
					actual_flexsetting_header	= info->override;
					pSetting					= override;
				}
			}
		}

		flexweight_t *pWeights = NULL;

		int truecount = pSetting->psetting( (byte *)actual_flexsetting_header, 0, &pWeights );
		if ( pWeights )
		{
			for (i = 0; i < truecount; i++)
			{
				// Translate to global controller number
				int j = actual_flexsetting_header->LocalToGlobal( pWeights->key );
				// Add scaled weighting in
				g_flexweight[j] += info->amount * scale * pWeights->weight;
				// Go to next setting
				pWeights++;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: A lot of the one time setup and also resets amount to 0.0f default
//  for strong/weak/normal tracks
// Returning true == skip this phoneme
// Input  : *classes - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_BaseFlex::SetupEmphasisBlend( Emphasized_Phoneme *classes, int phoneme )
{
	int i;

	bool skip = false;

	for ( i = 0; i < NUM_PHONEME_CLASSES; i++ )
	{
		Emphasized_Phoneme *info = &classes[ i ];

		// Assume it's bogus
		info->valid = false;
		info->amount = 0.0f;

		// One time setup
		if ( !info->basechecked )
		{
			info->basechecked = true;
			info->base = (flexsettinghdr_t *)FindSceneFile( info->classname );
		}
		info->override = NULL;

		info->exp = NULL;
		if ( info->base )
		{
			Assert( info->base->id == ('V' << 16) + ('F' << 8) + ('E') );
			info->exp = info->base->pIndexedSetting( phoneme );
		}

		if ( info->required && ( !info->base || !info->exp ) )
		{
			skip = true;
			break;
		}

		if ( info->exp )
		{
			info->valid = true;
		}

		// Find overrides, if any exist
		// Also a one-time setup
		if ( !info->overridechecked )
		{
			char overridefile[ 512 ];
			char shortname[ 128 ];
			char modelname[ 128 ];

			Q_strncpy( modelname, modelinfo->GetModelName( GetModel() ), sizeof( modelname ) );

			// Fix up the name
			Q_FileBase( modelname, shortname, sizeof( shortname ) );

			Q_snprintf( overridefile, sizeof( overridefile ), "%s/%s", shortname, info->classname );

			info->overridechecked = true;
			info->override = ( flexsettinghdr_t * )FindSceneFile( overridefile );
		}
	}
	
	return skip;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *classes - 
//			*sentence - 
//			t - 
//			dt - 
//			juststarted - 
//-----------------------------------------------------------------------------
ConVar g_CV_PhonemeSnap("phonemesnap", "1", 0, "Don't force visemes to always consider two phonemes, regardless of duration." );
void C_BaseFlex::AddVisemesForSentence( Emphasized_Phoneme *classes, float emphasis_intensity, CSentence *sentence, float t, float dt, bool juststarted )
{
	studiohdr_t *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	int pcount = sentence->GetRuntimePhonemeCount();
	for ( int k = 0; k < pcount; k++ )
	{
		const CBasePhonemeTag *phoneme = sentence->GetRuntimePhoneme( k );

		if ((!g_CV_PhonemeSnap.GetBool() || (hdr->flags & STUDIOHDR_FLAGS_FORCE_PHONEME_CROSSFADE)) && t > phoneme->m_flStartTime && t < phoneme->m_flEndTime)
		{
			if (k < pcount-1)
			{
				const CBasePhonemeTag *next = sentence->GetRuntimePhoneme( k + 1 );
				if ( next )
				{
					dt = max( dt, min( next->m_flEndTime - t, phoneme->m_flEndTime - phoneme->m_flStartTime ) );
				}
			}
		}

		float t1 = ( phoneme->m_flStartTime - t) / dt;
		float t2 = ( phoneme->m_flEndTime - t) / dt;

		if (t1 < 1.0 && t2 > 0)
		{
			float scale;

			// clamp
			if (t2 > 1)
				t2 = 1;
			if (t1 < 0)
				t1 = 0;

			// FIXME: simple box filter.  Should use something fancier
			scale = (t2 - t1);

			AddViseme( classes, emphasis_intensity, phoneme->m_nPhonemeCode, scale, juststarted );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *classes - 
//-----------------------------------------------------------------------------
void C_BaseFlex::ProcessVisemes( Emphasized_Phoneme *classes )
{
	// Any sounds being played?
	if ( !MouthInfo().IsActive() )
		return;

	// Multiple phoneme tracks can overlap, look across all such tracks.
	for ( int source = 0 ; source < MouthInfo().GetNumVoiceSources(); source++ )
	{
		CVoiceData *vd = MouthInfo().GetVoiceSource( source );
		if ( !vd )
			continue;

		CSentence *sentence = engine->GetSentence( vd->GetSource() );
		if ( !sentence )
			continue;

		float	sentence_length = engine->GetSentenceLength( vd->GetSource() );
		float	timesincestart = vd->GetElapsedTime();

		// This sound should be done...why hasn't it been removed yet???
		if ( timesincestart >= ( sentence_length + 2.0f ) )
			continue;

		// Adjust actual time
		float t = timesincestart - g_CV_PhonemeDelay.GetFloat();

		// Get box filter duration
		float dt = g_CV_PhonemeFilter.GetFloat();

		// Streaming sounds get an additional delay...
		/*
		// Tracker 20534:  Probably not needed any more with the async sound stuff that
		//  we now have (we don't have a disk i/o hitch on startup which might have been
		//  messing up the startup timing a bit )
		bool streaming = engine->IsStreaming( vd->m_pAudioSource );
		if ( streaming )
		{
			t -= g_CV_PhonemeDelayStreaming.GetFloat();
		}
		*/

		// Assume sound has been playing for a while...
		bool juststarted = false;
		/*
		// FIXME:  Do we really want to support markov chains for the phonemes?
		// If so, we'll need to uncomment out these lines.
		if ( timesincestart < 0.001 )
		{
			juststarted = true;
		}
		*/

		// Get intensity setting for this time (from spline)
		float emphasis_intensity = sentence->GetIntensity( t, sentence_length );

		// Blend and add visemes together
		AddVisemesForSentence( classes, emphasis_intensity, sentence, t, dt, juststarted );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseFlex::SetupWeights( )
{
	studiohdr_t *hdr = GetModelPtr();
	if ( !hdr )
	{
		return;
	}

	memset( g_flexweight, 0, sizeof( g_flexweight ) );

	// FIXME: this should assert then, it's too complex a class for the model
	if (hdr->numflexcontrollers == 0)
		return;

	int i, j;

	// FIXME: shouldn't this happen at runtime?
	// initialize the models local to global flex controller mappings
	if (hdr->pFlexcontroller( 0 )->link == -1)
	{
		for (i = 0; i < hdr->numflexcontrollers; i++)
		{
			j = AddGlobalFlexController( hdr->pFlexcontroller( i )->pszName() );
			hdr->pFlexcontroller( i )->link = j;
		}
	}

	// blend weights from server
	for (i = 0; i < hdr->numflexcontrollers; i++)
	{
		mstudioflexcontroller_t *pflex = hdr->pFlexcontroller( i );

		g_flexweight[pflex->link] = m_flexWeight[i];
		// rescale
		g_flexweight[pflex->link] = g_flexweight[pflex->link] * (pflex->max - pflex->min) + pflex->min;
	}

	// check for blinking
	if (m_blinktoggle != m_prevblinktoggle)
	{
		m_prevblinktoggle = m_blinktoggle;
		m_blinktime = gpGlobals->curtime + g_CV_BlinkDuration.GetFloat();
	}

	if (m_iBlink == -1)
		m_iBlink = AddGlobalFlexController( "blink" );
	g_flexweight[m_iBlink] = 0;

	// FIXME: this needs a better algorithm
	// blink the eyes
	float t = (m_blinktime - gpGlobals->curtime) * M_PI * 0.5 * (1.0/g_CV_BlinkDuration.GetFloat());
	if (t > 0)
	{
		// do eyeblink falloff curve
		t = cos(t);
		if (t > 0)
		{
			g_flexweight[m_iBlink] = sqrtf( t ) * 2;
			if (g_flexweight[m_iBlink] > 1)
				g_flexweight[m_iBlink] = 2.0 - g_flexweight[m_iBlink];
		}
	}

	// Drive the mouth from .wav file playback...
	ProcessVisemes( m_PhonemeClasses );

	float destweight[128];
	RunFlexRules( destweight );

	SetViewTarget( );

	if (m_flFlexDelayedWeight && g_CV_FlexSmooth.GetBool())
	{
		float d = 1.0;
		if (gpGlobals->frametime != 0)
		{
			d = ExponentialDecay( 0.8, 0.033, gpGlobals->frametime );
		}
		for ( i = 0; i < hdr->numflexdesc; i++)
		{
			m_flFlexDelayedWeight[i] = m_flFlexDelayedWeight[i] * d + destweight[i] * (1 - d);
		}
		// debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), i-hdr->numflexcontrollers, 0, "%.3f", d );
		modelrender->SetFlexWeights( hdr->numflexdesc, destweight, m_flFlexDelayedWeight );
	}
	else
	{
		modelrender->SetFlexWeights( hdr->numflexdesc, destweight );
	}

	// aim the eyes

	/*
	for (i = 0; i < hdr->numflexdesc; i++)
	{
		debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), i-hdr->numflexcontrollers, 0, "%2d:%s : %3.2f", i, hdr->pFlexdesc( i )->pszFACS(), destweight[i] );
	}
	*/

	/*
	for (i = 0; i < g_numflexcontrollers; i++)
	{
		int j = hdr->pFlexcontroller( i )->link;
		debugoverlay->AddTextOverlay( GetAbsOrigin() + Vector( 0, 0, 64 ), -i, 0, "%s %3.2f", g_flexcontroller[i], g_flexweight[j] );
	}
	*/
}



int C_BaseFlex::g_numflexcontrollers;
char * C_BaseFlex::g_flexcontroller[MAXSTUDIOFLEXCTRL*4];
float C_BaseFlex::g_flexweight[MAXSTUDIOFLEXDESC];

int C_BaseFlex::AddGlobalFlexController( char *szName )
{
	int i;
	for (i = 0; i < g_numflexcontrollers; i++)
	{
		if (stricmp( g_flexcontroller[i], szName ) == 0)
		{
			return i;
		}
	}

	if (g_numflexcontrollers < MAXSTUDIOFLEXCTRL * 4)
	{
		g_flexcontroller[g_numflexcontrollers++] = strdup( szName );
	}
	else
	{
		// FIXME: missing runtime error condition
	}
	return i;
}

const flexsetting_t *C_BaseFlex::FindNamedSetting( const flexsettinghdr_t *pSettinghdr, const char *expr )
{
	int i;
	const flexsetting_t *pSetting = NULL;

	for ( i = 0; i < pSettinghdr->numflexsettings; i++ )
	{
		pSetting = pSettinghdr->pSetting( i );
		if ( !pSetting )
			continue;

		const char *name = pSetting->pszName();

		if ( !stricmp( name, expr ) )
			break;
	}

	if ( i>=pSettinghdr->numflexsettings )
	{
		return NULL;
	}

	return pSetting;
}
