vs.1.1

# DYNAMIC: "DOWATERFOG"				"0..1"

#include "FF_macros.vsh"

;------------------------------------------------------------------------------
; Vertex blending
;------------------------------------------------------------------------------

&AllocateRegister( \$worldPos );

; Transform position from object to world
dp4 $worldPos.x, $vPos, $cModel0
dp4 $worldPos.y, $vPos, $cModel1
dp4 $worldPos.z, $vPos, $cModel2

&AllocateRegister( \$projPos );

; Transform position from object to projection space
dp4 $projPos.x, $vPos, $cModelViewProj0
dp4 $projPos.y, $vPos, $cModelViewProj1
dp4 $projPos.z, $vPos, $cModelViewProj2
dp4 $projPos.w, $vPos, $cModelViewProj3

mov oPos, $projPos

;------------------------------------------------------------------------------
; Fog
;------------------------------------------------------------------------------
&CalcFog( $worldPos, $projPos );

&FreeRegister( \$projPos );

;------------------------------------------------------------------------------
; Lighting
;------------------------------------------------------------------------------

; Transform tangent space basis vectors to env map space (world space)
; This will produce a set of vectors mapping from tangent space to env space
; We'll use this to transform normals from the normal map from tangent space
; to environment map space. 
; NOTE: use dp3 here since the basis vectors are vectors, not points

dp3 oT1.x, $vTangentS, $cModel0
dp3 oT2.x, $vTangentS, $cModel1
dp3 oT3.x, $vTangentS, $cModel2

dp3 oT1.y, $vTangentT, $cModel0
dp3 oT2.y, $vTangentT, $cModel1
dp3 oT3.y, $vTangentT, $cModel2

dp3 oT1.z, $vNormal, $cModel0
dp3 oT2.z, $vNormal, $cModel1
dp3 oT3.z, $vNormal, $cModel2
 
; Compute the vector from vertex to camera
&AllocateRegister( \$worldEyeVect );
sub $worldEyeVect.xyz, $cEyePos, $worldPos
&FreeRegister( \$worldPos );

; eye vector
mov oT4.xyz, $worldEyeVect

alloc $tangentEyeVect

; transform the eye vector to tangent space
dp3 $tangentEyeVect.x, $worldEyeVect, $vTangentS
dp3 $tangentEyeVect.y, $worldEyeVect, $vTangentT
dp3 $tangentEyeVect.z, $worldEyeVect, $vNormal

; Get the magnitude of worldEyeVect
dp3 $worldEyeVect.w, $worldEyeVect, $worldEyeVect
rsq $worldEyeVect.w, $worldEyeVect.w
rcp $worldEyeVect.w, $worldEyeVect.w

; calculate the cheap water blend factor and stick it into oD0.a
; NOTE: This won't be perspective correct!!!!!
; OPTIMIZE: This could turn into a mad.
add $worldEyeVect.w, $worldEyeVect.w, -$SHADER_SPECIFIC_CONST_2.x
mul oD0, $worldEyeVect.w, $SHADER_SPECIFIC_CONST_2.y

; stick the tangent space eye vector into oT5.xyz
mov oT5.xyz, $tangentEyeVect

&FreeRegister( \$worldEyeVect );
&FreeRegister( \$tangentEyeVect );

;------------------------------------------------------------------------------
; Texture coordinates
;------------------------------------------------------------------------------
dp4 oT0.x, $vTexCoord0, $SHADER_SPECIFIC_CONST_0
dp4 oT0.y, $vTexCoord0, $SHADER_SPECIFIC_CONST_1



