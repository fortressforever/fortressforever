#!/usr/bin/perl
#

# this is just a really rough script to fix the vcpm output
# it thinks everything is a cpp and our lua stuff isn't
undef $/;

$lua	 = "\$(DLLS__LUA_SRC_OBJ_DIR)/%.o: \$(DLLS__LUA_SRC_SRC_DIR)/%.c\n\t\$(DO_C)";
$lua_lib = "\$(DLLS__LUA_SRC_LIB_OBJ_DIR)/%.o: \$(DLLS__LUA_SRC_LIB_SRC_DIR)/%.c\n\t\$(DO_C)";

while(<>) {
	$line = $_;

	$line =~ s/\$\(DLLS__LUA_SRC_OBJ_DIR\)\/%\.o: \$\(DLLS__LUA_SRC_SRC_DIR\)\/%\.cpp\n\t\$\(DO_CC\)/$lua/gs;
	$line =~ s/\$\(DLLS__LUA_SRC_LIB_OBJ_DIR\)\/%\.o: \$\(DLLS__LUA_SRC_LIB_SRC_DIR\)\/%\.cpp\n\t\$\(DO_CC\)/$lua_lib/gs;
	
	print $line;
}
