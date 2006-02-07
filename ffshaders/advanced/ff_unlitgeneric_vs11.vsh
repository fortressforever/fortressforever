vs.1.1

# DYNAMIC: "FOG_TYPE"				"0..1"
# DYNAMIC: "NUM_BONES"				"0..3"
# STATIC:  "DETAIL"					"0..1"
# STATIC:  "ENVMAP"					"0..1"
# STATIC:  "ENVMAPCAMERASPACE"		"0..1"
# STATIC:  "ENVMAPSPHERE"			"0..1"
# STATIC:  "VERTEXCOLOR"			"0..1"

# can't have envmapshere or envmapcameraspace without envmap
# SKIP: !$ENVMAP && ( $ENVMAPSPHERE || $ENVMAPCAMERASPACE )

# can't have both envmapsphere and envmapcameraspace
# SKIP: $ENVMAPSPHERE && $ENVMAPCAMERASPACE

#include "ff_UnlitGeneric_inc.vsh"

&UnlitGeneric( $DETAIL, $ENVMAP, $ENVMAPCAMERASPACE, $ENVMAPSPHERE, $VERTEXCOLOR );
