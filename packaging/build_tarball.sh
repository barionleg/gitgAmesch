#!/bin/bash
#To start clean pull the latest version from the git repo and all master branches from the submodules
# exit if error occurs
set -e
STARTDIR=$(pwd)
cd /tmp/
NAME=gigamesh$(uuidgen)
git clone --recurse-submodules ssh://git@fcgitlab.iwr.uni-heidelberg.de:2222/gigamesh/GigaMesh.git $NAME
wait
# Make sure all submodules are up to date!
cd $NAME
git submodule foreach git pull origin master
cd ..
# end update of submodule
wait
cd $NAME
# Get NUM_PROCESSORS and CURRENT_VERSION
NUM_PROCESSORS=$(nproc)
if [ $# -eq 1 ]; then
	CURRENT_VERSION=$1
elif [ $# -eq 0 ]; then
	CURRENT_VERSION=$(git log -1 --format=%ci | cut -b3,4,6,7,9,10)
else
	echo "ERR: parameters must number exactly 0 or 1"
	exit 1
fi

# Update changelog with CURRENT_VERSION
sed -i "/unreleasd/c\Version $CURRENT_VERSION" CHANGELOG

# c-q-make everything
cd external/libpsalmBoostless
mkdir build
cd build
cmake ..
cmake --build . --config Release
cp libpsalm.a ..
cd ..
rm -rf build
cd ..

# make spherical intersection
cd spherical_intersection
mkdir build
cd build
cmake ..
cmake --build . --config Release
cp libspherical_intersection.a ..
cd ..
rm -rf build
cd ..

# Make ALGLIB
unzip alglib-2.6.0.cpp.zip
mv cpp alglib
cd alglib
chmod u+x build
./build gcc
cd ../..

# Make the mesh cli
mkdir meshBuild
cd meshBuild
cmake ..
cmake --build . --config Release
find . -type f -executable -not -name \*.sh -exec strip {} \;
cd ..
# Inside GigaMesh folder now
#make clean
qmake CONFIG+=release
make -j $NUM_PROCESSORS CC=gcc-8 CXX=g++-8 LINK=g++-8
strip gigamesh
#Move Back to packaging
cd packaging

#Create generic binary directory and compress it
ND="gigamesh-$CURRENT_VERSION-linux-x86_64"
mkdir -p $ND/scripts
cp -a ../CHANGELOG $ND

# --- binaries
cp -a ../gigamesh $ND
cp -a ../meshBuild/gigamesh-info  $ND
cp -a ../meshBuild/gigamesh-clean $ND
cp -a ../meshBuild/gigamesh-tolegacy $ND
cp -a ../meshBuild/gigamesh-featurevectors $ND
cp -a ../meshBuild/gigamesh-borders $ND
# --- Scripts ---
# None for now, which is a good thing.

cd $ND
tar --create --bzip2 --preserve-permissions --file ../$ND.tar.bz2 *
cd ..
echo Copying $ND.tar.bz2 into $STARTDIR
cp $ND.tar.bz2 $STARTDIR
rm -R $ND
rm -Rf /tmp/$NAME
