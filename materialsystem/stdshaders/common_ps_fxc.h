//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// This is where all common code for pixel shaders go.
#include "common_fxc.h"

HALF Luminance( HALF3 color )
{
	return dot( color, HALF3( HALF_CONSTANT(0.30f), HALF_CONSTANT(0.59f), HALF_CONSTANT(0.11f) ) );
}

HALF LuminanceScaled( HALF3 color )
{
	return dot( color, HALF3( HALF_CONSTANT(0.30f) / MAX_HDR_OVERBRIGHT, HALF_CONSTANT(0.59f) / MAX_HDR_OVERBRIGHT, HALF_CONSTANT(0.11f) / MAX_HDR_OVERBRIGHT ) );
}


HALF AvgColor( HALF3 color )
{
	return dot( color, HALF3( HALF_CONSTANT(0.33333f), HALF_CONSTANT(0.33333f), HALF_CONSTANT(0.33333f) ) );
}

HALF4 DiffuseBump( sampler lightmapSampler,
                   float2  lightmapTexCoord1,
                   float2  lightmapTexCoord2,
                   float2  lightmapTexCoord3,
                   HALF3   normal )
{
	HALF3 lightmapColor1 = tex2D( lightmapSampler, lightmapTexCoord1 );
	HALF3 lightmapColor2 = tex2D( lightmapSampler, lightmapTexCoord2 );
	HALF3 lightmapColor3 = tex2D( lightmapSampler, lightmapTexCoord3 );

	HALF3 diffuseLighting;
	diffuseLighting = saturate( dot( normal, bumpBasis[0] ) ) * lightmapColor1 +
					  saturate( dot( normal, bumpBasis[1] ) ) * lightmapColor2 +
					  saturate( dot( normal, bumpBasis[2] ) ) * lightmapColor3;

	return HALF4( diffuseLighting, LuminanceScaled( diffuseLighting ) );
}


HALF Fresnel( HALF3 normal,
              HALF3 eye,
              HALF2 scaleBias )
{
	HALF fresnel = HALF_CONSTANT(1.0f) - dot( normal, eye );
	fresnel = pow( fresnel, HALF_CONSTANT(5.0f) );

	return fresnel * scaleBias.x + scaleBias.y;
}

HALF4 GetNormal( sampler normalSampler,
                 float2 normalTexCoord )
{
	HALF4 normal = tex2D( normalSampler, normalTexCoord );
	normal.rgb = HALF_CONSTANT(2.0f) * normal.rgb - HALF_CONSTANT(1.0f);

	return normal;
}

HALF3 NormalizeWithCubemap( sampler normalizeSampler, HALF3 input )
{
//	return texCUBE( normalizeSampler, input ) * 2.0f - 1.0f;
	return texCUBE( normalizeSampler, input );
}

HALF4 EnvReflect( sampler envmapSampler,
				 sampler normalizeSampler,
				 HALF3 normal,
				 float3 eye,
				 HALF2 fresnelScaleBias )
{
	HALF3 normEye = NormalizeWithCubemap( normalizeSampler, eye );
	HALF fresnel = Fresnel( normal, normEye, fresnelScaleBias );
	HALF3 reflect = CalcReflectionVectorUnnormalized( normal, eye );
	return texCUBE( envmapSampler, reflect );
}

float CalcWaterFogAlpha( const float flWaterZ, const float flEyePosZ, const float flWorldPosZ, const float flProjPosZ, const float flFogOORange )
{
//	float flDepthFromWater = flWaterZ - flWorldPosZ + 2.0f; // hackity hack . .this is for the DF_FUDGE_UP in view_scene.cpp
	float flDepthFromWater = flWaterZ - flWorldPosZ;

	// if flDepthFromWater < 0, then set it to 0
	// This is the equivalent of moving the vert to the water surface if it's above the water surface
	// We'll do this with the saturate at the end instead.
//	flDepthFromWater = max( 0.0f, flDepthFromWater );

	// Calculate the ratio of water fog to regular fog (ie. how much of the distance from the viewer
	// to the vert is actually underwater.
	float flDepthFromEye = flEyePosZ - flWorldPosZ;
	float f = (flDepthFromWater / flDepthFromEye) * flProjPosZ;

	// $tmp.w is now the distance that we see through water.
	return saturate( f * flFogOORange );
}

float RemapValClamped( float val, float A, float B, float C, float D)
{
	float cVal = (val - A) / (B - A);
	cVal = saturate( cVal );

	return C + (D - C) * cVal;
}

float3 DoFlashlight( float3 flashlightPos, float3 worldPos, float3 worldNormal, 
					float4x4 worldToTexture, float3 attenuationFactors, 
					float farZ, sampler FlashlightSampler )
{
	float3 delta = flashlightPos - worldPos;
	float3 dir = normalize( delta );
	float distSquared = dot( delta, delta );
	float dist = sqrt( distSquared );

	float endFalloffFactor = RemapValClamped( dist, farZ, 0.6f * farZ, 0.0f, 1.0f );
//	return float3( endFalloffFactor, endFalloffFactor, endFalloffFactor );

	// fixme: need to figure out which was is more efficient for this vector by matrix multiply.
	float4 flashlightTexCoord = mul( float4( worldPos, 1.0f ), worldToTexture );
	float3 diffuseLighting;
	if( flashlightTexCoord.w <= 0.0f )
	{
		diffuseLighting = float3( 0.0f, 0.0f, 0.0f );
	}
	float3 flashlightColor = tex2Dproj( FlashlightSampler, flashlightTexCoord );
	diffuseLighting = dot( attenuationFactors, float3( 1.0f, 1.0f/dist, 1.0f/distSquared ) );
	diffuseLighting *= dot( dir, worldNormal );
	diffuseLighting *= flashlightColor;
	diffuseLighting *= endFalloffFactor;
	return diffuseLighting;
}
