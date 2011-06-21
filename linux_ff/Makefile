#
# FF Makefile for x86 Linux
#
#

#############################################################################
# Developer configurable items
#############################################################################

# the name of the mod binary (_i486.so is appended to the end)
NAME=server

# the location of the vcproj that builds the mod
MOD_PROJ=../dlls/server_scratch-2003.vcproj
# the name of the mod configuration (typically <proj name>_<build type><build target>)
MOD_CONFIG=server_ff_ReleaseFFWin32

# the directory the base binaries (tier0_i486.so, etc) are located
#GAME_DIR=../../srcds
GAME_DIR=/home/ffdev/srcds

# compiler options (gcc 3.4.1 or above is required)
#dexter - edited for 'gcc34' for 3.4.6 bins on CentOS 5
CC=gcc34
CPLUS=g++34
CLINK=gcc34
#CPP_LIB="/usr/lib/gcc-lib/i686-pc-linux-gnu/3.3.5/libstdc++.a /usr/lib/gcc-lib/i686-pc-linux-gnu/3.3.5/libgcc_eh.a"
#CPP_LIB="/usr/lib/gcc/i386-redhat-linux/3.4.3/libstdc++.a /usr/lib/gcc/i386-redhat-linux/3.4.3/libgcc_eh.a"
# dexter - FUUUUUUUU here is path to 3.4.6 on CentOS 5
CPP_LIB="/usr/lib/gcc/i386-redhat-linux/3.4.6/libstdc++.a /usr/lib/gcc/i386-redhat-linux/3.4.6/libgcc_eh.a"
# put any compiler flags you want passed here
# DEBUG - USER_CFLAGS=-g
USER_CFLAGS=

# link flags for your mod, make sure to include any special libraries here
#LDFLAGS="-lm -ldl $(GAME_DIR)/bin/tier0_i486.so $(GAME_DIR)/bin/vstdlib_i486.so"

#LDFLAGS="-L../external/lua/lib -L../external/luabind/lib -lluabind -llua -lm -ldl tier0_i486.so vstdlib_i486.so mathlib_i486.a choreoobjects_i486.a tier1_i486.a"
LDFLAGS="-L../external/lua/lib -L../external/luabind/lib -lluabind -llua -lm -ldl  tier0_i486.so vstdlib_i486.so mathlib_i486.a choreoobjects_i486.a tier1_i486.a"
#LDFLAGS="-lm tier0_i486.so vstdlib_i486.so"
# XERCES 2.6.0 or above ( http://xml.apache.org/xerces-c/ ) is used by the vcproj to makefile converter
# it must be installed before being able to run this makefile
#XERCES_INC_DIR=/home/alfred/tmp/xerces-c-src_2_6_0/include
#XERCES_LIB_DIR=/home/alfred/tmp/xerces-c-src_2_6_0/lib
# if you have xerces installed already you should be able to use the two lines below
XERCES_INC_DIR=/usr/include
XERCES_LIB_DIR=/usr/lib


#############################################################################
# Things below here shouldn't need to be altered
#############################################################################
MAKE=make

# the dir we want to put binaries we build into
BUILD_DIR=.
# the place to put object files
BUILD_OBJ_DIR=$(BUILD_DIR)/obj

# the location of the source code
SOURCE_DIR=..

# the CPU target for the build, must be i486 for now
ARCH=i486
#ARCH_CFLAGS=-mtune=i686 -march=pentium -mmmx -O3
#ARCH_CFLAGS=-mtune=i686 -march=pentium -mmmx
ARCH_CFLAGS=-mtune=i686 -march=pentium3 -mmmx -O3

# -fpermissive is so gcc 3.4.x doesn't complain about some template stuff
BASE_CFLAGS=-fpermissive -D_LINUX -DNDEBUG -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp
SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared -Wl,-Map,$@_map.txt -Wl

#flags passed to the c compiler
CFLAGS="$(USER_CFLAGS) $(DEFINES) $(ARCH_CFLAGS) $(BASE_CFLAGS) -Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead -UPROTECTED_THINGS_ENABLE"

# define list passed to make for the sub makefile
BASE_DEFINES=CC=$(CC) CPLUS=$(CPLUS) CPP_LIB=$(CPP_LIB) \
	BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) \
	SOURCE_DIR=$(SOURCE_DIR) SHLIBLDFLAGS=$(SHLIBLDFLAGS) SHLIBEXT=$(SHLIBEXT) \
	CLINK=$(CLINK) CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) \
	ARCH=$(ARCH) GAME_DIR=$(GAME_DIR) MOD_CONFIG=$(MOD_CONFIG) NAME=$(NAME) \
	XERCES_INC_DIR=$(XERCES_INC_DIR) XERCES_LIB_DIR=$(XERCES_LIB_DIR)

# Project Makefile
MAKE_MOD=Makefile.mod
MAKE_VCPM=Makefile.vcpm
MAKE_PLUGIN=Makefile.plugin

all: check vcpm mod

check:
	if [ -z "$(CC)" ]; then echo "Compiler not defined."; exit; fi
	if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi
	cd $(BUILD_DIR)

vcpm:
	$(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES)

mod: vcpm
	if [ ! -f "tier0_i486.so" ]; then ln -s $(GAME_DIR)/bin/tier0_i486.so .; fi
	if [ ! -f "vstdlib_i486.so" ]; then ln -s $(GAME_DIR)/bin/vstdlib_i486.so .; fi
	./vcpm $(MOD_PROJ)
	$(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES)

plugin:
	$(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES)

clean:
	 $(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES) clean
