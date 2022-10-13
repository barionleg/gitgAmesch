#!/bin/bash
#To start clean pull the latest version from the git repo and all master branches from the submodules
# exit if error occurs
set -e
STARTDIR=$(pwd)
cd /tmp/
NAME=gigamesh$(uuidgen)
git clone --recurse-submodules https://gitlab.com/fcgl/GigaMesh.git $NAME
wait
# Make sure all submodules are up to date!
cd $NAME
git submodule foreach git pull origin master
#git checkout develop                          # <= Uncomment 1/2 for testing using the develop branch.
#git submodule foreach git pull origin develop # <= Uncomment 2/2 for testing using the develop branch.
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

# cmake everything
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G"Unix Makefiles"
make -j
find . -type f -executable -not -name \*.sh -exec strip {} \;

cd ../packaging

#Create generic binary directory and compress it
ND="gigamesh-$CURRENT_VERSION-linux-x86_64"
mkdir -p $ND/scripts
cp -a ../CHANGELOG $ND

# --- binaries --- new: add all executeable binaries.
find ../build/cli/ ../build/gui/ -executable -type f -exec mv {} $ND \;
# --- binaries --- old style one by one.
#cp -a ../build/gui/gigamesh $ND
#cp -a ../build/cli/gigamesh-info  $ND
#cp -a ../build/cli/gigamesh-clean $ND
#cp -a ../build/cli/gigamesh-tolegacy $ND
#cp -a ../build/cli/gigamesh-featurevectors $ND
#cp -a ../build/cli/gigamesh-borders $ND
#cp -a ../build/cli/gigamesh-gnsphere $ND
#cp -a ../build/cli/gigamesh-meshsplit $ND
#cp -a ../build/cli/spherical_intersection_cli/gigamesh-sphere-profiles $ND
#cp -a ../build/cli/spherical_intersection_cli/gigamesh-featurevectors-sl $ND
# --- Scripts ---
# None for now, which is a good thing.

cd $ND
tar --create --bzip2 --preserve-permissions --file ../$ND.tar.bz2 *
cd ..
echo Copying $ND.tar.bz2 into $STARTDIR
cp $ND.tar.bz2 $STARTDIR
rm -R $ND
rm -Rf /tmp/$NAME
