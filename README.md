[![pipeline status](https://gitlab.com/fcgl/GigaMesh/badges/master/pipeline.svg)](https://gitlab.com/fcgl/GigaMesh/-/commits/master)

# NAME

**GigaMesh** - GigaMesh Software Framework for processing large meshes.

## DESCRIPTION

The **GigaMesh Software Framework** is a modular software for display, editing and visualization of 3D-data 
typically acquired with structured light scanning (SLS) or structure from motion (SfM).

It provides numerous functions for analysis of archaeological objects like cuneiform tablets, ceramics or converted LiDAR data.
Typically applications are unwrappings (or rollouts), profile cuts (or cross sections) as well as visualizations of distances and curvature, 
which can be exported as raster graphics or vector graphics.

The retrieval of text in 3D like damaged cuneiform tablets or weathered medieval headstones using Multi Scale Integral Invariant (MSII) filtering, 
which is a core function of the software. Furthermore small or faint surface details like fingerprints can be visualized.
The polygonal meshes of the 3D-models can be inspected, cleaned and repaired to provide optimal filtering results. 
The repaired datasets are suitable for 3D printing and for digital publishing in a dataverse.

Source: [Wikipedia article about GigaMesh](https://en.wikipedia.org/wiki/GigaMesh_Software_Framework)

WebSites: 
- [GigaMesh](https://gigamesh.eu) (official, with [tutorials](https://gigamesh.eu/tutorials) and [package downloads](https://gigamesh.eu/downloads))
- [Project on ResearchGate](https://www.researchgate.net/project/GigaMesh-Software-Framework)
- [Open Access 3D Datasets](https://heidata.uni-heidelberg.de/dataverse/iwrgraphics)

Video Tutorials:
- [YouTube Channel](https://www.youtube.com/channel/UCJSOsw9GX8DnkqnciyVwmLw)
- [Heidelberg University Document Server (HeiDOk)](http://archiv.ub.uni-heidelberg.de/volltextserver/cgi/search/simple?q=+GigaMesh+Software+Framework+Tutorial&_action_search=Search&_action_search=Search&_order=bytitle&basic_srchtype=ALL&_satisfyall=ALL)

# Executables

All executables provide details by adding `--help` or `-h` on the command line.

## Graphical user interface (GUI)
- `gigamesh` ... for all functions except the MSII filter.

## Command line interface (CLI)
- `gigamesh-featurevectors` ...  MSII filter for single files.
- `gigamesh-clean` ... batch cleaning and repairing multiple files.
- `gigamesh-info` ... to retrieve mesh properties for multiple files. Output as CSV or multiple sidecar files.
- `gigamesh-tolegacy` ... convert multiple files to legacy Stanford Polygon (PLY) as ASCII and binary.
- `gigamesh-border` ... extraction of mesh borders as polylines (ASCII formatted .pline files).

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

*GigaMesh* is developed by Hubert Mara.

Robert Kühl is the senior developer.

*psalm* is developed by Bastian Rieck (onfgvna@evrpx.eh; use `rot13` to
descramble). The original code is available via [GitHub](https://github.com/Pseudomanifold/psalm).
