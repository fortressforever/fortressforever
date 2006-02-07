vs.1.1

# DYNAMIC: "FOG_TYPE"				"0..1"
# DYNAMIC: "LIGHT_COMBO"			"0..21"
# DYNAMIC: "NUM_BONES"				"0..3"
# STATIC:  "HALF_LAMBERT"			"0..1"
# STATIC:  "DETAIL"					"0..1"
# STATIC:  "ENVMAP"					"0..1"
# STATIC:  "ENVMAPCAMERASPACE"		"0..1"
# STATIC:  "ENVMAPSPHERE"			"0..1"
# STATIC:  "DECAL"					"0..1"

# can't have envmapshere or envmapcameraspace without envmap
# SKIP: !$ENVMAP && ( $ENVMAPSPHERE || $ENVMAPCAMERASPACE )

# can't have both envmapsphere and envmapcameraspace
# SKIP: $ENVMAPSPHERE && $ENVMAPCAMERASPACE

# decal is by itself
# SKIP: $DECAL && ( $DETAIL || $ENVMAP || $ENVMAPCAMERASPACE || $ENVMAPSPHERE )

#include "VertexLitGeneric_inc.vsh"

&VertexLitGeneric( $DETAIL, $ENVMAP, $ENVMAPCAMERASPACE, $ENVMAPSPHERE, $DECAL );
