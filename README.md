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


COPYRIGHT AND LICENCE
---------------------

COPYRIGHT AND LICENCE TO BE DONE FOR GIGAMESH




*psalm* Copyright 2010, Bastian Rieck. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

-	Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
- 	Redistributions in binary form must reproduce the above
	copyright notice, this list of conditions and the following
	disclaimer in the documentation and/or other materials provided
	with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.