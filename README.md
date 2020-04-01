<!-- [![pipeline status](https://fcgitlab.iwr.uni-heidelberg.de/gigamesh/GigaMesh/badges/master/pipeline.svg)](https://fcgitlab.iwr.uni-heidelberg.de/gigamesh/GigaMesh/commits/master) -->
# NAME

**GigaMesh** - GigaMesh Software Framework for processing large meshes.

## DESCRIPTION

The **GigaMesh Software Framework** is a modular software for display, editing and visualization of 3D-data typically acquired with structured light or structure from motion.

## EXAMPLES 

#### How to clone the project
```sh
git clone https://gitlab.com/fcgl/GigaMesh.git
```

#### FEATURE EXTRACTION

For computing the `MSII Featurevectors` a radius and a mesh file need to be given
for the input.

    gigamesh-featurevectors -r 5.0 -f mesh.ply

BUILDING GigaMesh
--------------

The following packages are required for building GigaMesh:

* Recent version of cmake
* qt5
* libtiff (optional)
* A recent C++ compiler that supports C++17

GigaMesh ist build via CMake:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

To build for release, use the appropriate build flags.

For single-configuration build files, like Makefiles, this changes the cmake commands to:
```sh
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

For multi-configuration build files, like MSVC solution files, the cmake commands are:
```sh
cmake ..
cmake --build . --config Release
```

GigaMesh is ideally build with the qtcreator IDE. To load the project, open the CMakeLists.txt in the root directory.
Please make sure you have a kit selected, that has a C++17 compatible compiler. For further information see:
* https://doc.qt.io/qtcreator/creator-targets.html
* https://doc.qt.io/qtcreator/creator-tool-chains.html

AUTHOR
------

*GigaMesh* is developed by Hubert Mara

*psalm* is developed by Bastian Rieck (onfgvna@evrpx.eh; use `rot13` to
descramble).
