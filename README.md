[![pipeline status](https://fcgitlab.iwr.uni-heidelberg.de/gigamesh/GigaMesh/badges/master/pipeline.svg)](https://fcgitlab.iwr.uni-heidelberg.de/gigamesh/GigaMesh/commits/master)
# NAME

**GigaMesh** - GigaMesh Software Framework for processing large meshes.

## DESCRIPTION

**GigaMesh** is a software Framework for processing large meshes.
Feature vecotors... MSII....

What can the users do?

gigamesh_featurevectors .


## EXAMPLES 

#### How to clone the project with submodules
```sh
git clone --recurse-submodules ssh://git@fcgitlab.iwr.uni-heidelberg.de:2222/gigamesh/GigaMesh.git
```
Make sure to get the master branch of the submodules by running the following
command inside the GigaMesh repo:
```sh
git submodule foreach git pull origin master
```

#### How to get Submodules after cloning (without cloning submodules)
```sh
cd FOLDER_OF_SUBMODULE
git submodule init
git submodule update
```
#### FEATURE EXTRACTION

For computing the `MSII Featurevectors` a radius and a mesh file need to be given
for the input.

    gigamesh-featurevectors -r 5.0 -f mesh.ply

BUILDING GigaMesh
--------------

The following packages are required for building GigaMesh:

* Recent version of cmake
* libtiff
* qt5
* ...

The following packages are recommended:
* Libpsalm (Submodule)
* ALGLIB
* ...

Explain steps...



AUTHOR
------

*GigaMesh* is developed by Hubert Mara

*psalm* is developed by Bastian Rieck (onfgvna@evrpx.eh; use `rot13` to
descramble).

*ALGLIB* is developed by ...


FILES
-----

*GigaMesh* is shipped with several example meshes:

- `test.ply`: Extra weights from U. Reif's publication *A
  unified approach to subdivision algorithms near extraordinary
  vertices*
- `Hexahedron.ply`: A cube in `PLY` format. The mesh is used courtesy of
  [John Burkardt](http://people.sc.fsu.edu/~jburkardt).
