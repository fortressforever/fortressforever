vs.1.1

# DYNAMIC: "DOWATERFOG"				"0..1"
# STATIC:  "DETAIL"					"0..1"
# STATIC:  "ENVMAP"					"0..1"
# STATIC:  "ENVMAPCAMERASPACE"		"0..0"
# STATIC:  "ENVMAPSPHERE"			"0..1"
# STATIC:  "VERTEXCOLOR"			"0..1"

# can't have envmapshere or envmapcameraspace without envmap
# SKIP: !$ENVMAP && ( $ENVMAPSPHERE || $ENVMAPCAMERASPACE )

# can't have both envmapsphere and envmapcameraspace
# SKIP: $ENVMAPSPHERE && $ENVMAPCAMERASPACE

#include "SDK_LightmappedGeneric_inc.vsh"

&LightmappedGeneric( $DETAIL, $ENVMAP, $ENVMAPCAMERASPACE, $ENVMAPSPHERE, 
				$VERTEXCOLOR );

