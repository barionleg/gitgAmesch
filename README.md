<!-- [![pipeline status](https://fcgitlab.iwr.uni-heidelberg.de/gigamesh/GigaMesh/badges/master/pipeline.svg)](https://fcgitlab.iwr.uni-heidelberg.de/gigamesh/GigaMesh/commits/master) -->
# NAME

**GigaMesh** - GigaMesh Software Framework for processing large meshes.

## DESCRIPTION

The **GigaMesh Software Framework** is a modular software for display, editing and visualization of 3D-data typically acquired with structured light or structure from motion.


## EXAMPLES 

#### How to clone the project with submodules
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

AUTHOR
------

*GigaMesh* is developed by Hubert Mara

*psalm* is developed by Bastian Rieck (onfgvna@evrpx.eh; use `rot13` to
descramble).

FILES
-----

*GigaMesh* is shipped with several example meshes:

- `test.ply`: Extra weights from U. Reif's publication *A
  unified approach to subdivision algorithms near extraordinary
  vertices*
- `Hexahedron.ply`: A cube in `PLY` format. The mesh is used courtesy of
  [John Burkardt](http://people.sc.fsu.edu/~jburkardt).
