#!/bin/bash
echo "FF AUTOBUILD THING EL WOOTO"

BINFILE=server_i486.so
SRC_DIR=~/ff_src/2.42/
SRC_BUILD_DIR=$SRC_DIR/linux_ff/
MOD_DIR=~/srcds/ff_dev/
SVN_COMMIT_MSG="latest linux binary. built "`date +%x@%X`


cd $SRC_DIR
svn up &> /dev/null
cd $SRC_BUILD_DIR
rm obj/ -rf
rm $BINFILE
export LD_LIBRARY_PATH=.
make -j2 #&> /dev/null
echo $?
if [ $? = 0 ]; then
	echo "Compiled successfully"	
	#pwd
	#strip it to save ~5megs
	strip $BINFILE
	# svn commit the sucker
	cp $BINFILE $MOD_DIR/bin && cd $MOD_DIR/bin
	echo $?
	svn commit $BINFILE -m "$SVN_COMMIT_MSG"
	if [ $? = 0 ]; then
		echo "Updated binary successfully, done!"
	else
		echo "Couldn't svn commit new bin for some reason"
	fi
else
	echo "Couldn't compile for some reason!"
fi
