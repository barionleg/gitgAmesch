#==========================================================================
# Makefile for GigaMesh @ commandline
#-------------------------------------------------------------------------
# Contact: Hubert Mara <hubert.mara@iwr.uni-heidelberg.de>
#-------------------------------------------------------------------------
# Helpfull: http://www.network-theory.co.uk/docs/gccintro/gccintro_25.html
#==========================================================================
BUILDINFO  = -DCOMP_USER=\"`whoami`@`hostname -s`\" \
             -DCOMP_DATE=\"`date +%d.%b.%Y`\" \
             -DCOMP_GITHEAD=\"`git rev-parse HEAD`\" \
             -DVERSION_PACKAGE=\"`git log -1 --format=%ci | cut -b3,4,6,7,9,10`\" \
             -DCOMP_EDIT=\"IWR\ FreeWare\"
             #-DCOMP_EDIT=\"IWR\ Internal\"
BUILDINFO += -DNUM_THREADS=11
#BUILDINFO += -DGIGAMESH_PUBLIC_METHODS_ONLY=\"true\"
#==========================================================================
# Ubuntu build with gcc/g++ Version 8:
# make CC=gcc-8 CXX=g++-8 LINK=g++-8
#==========================================================================
CC           = gcc
CXX          = g++
CFLAGS       = -c ${BUILDINFO} -std=c++17 -pipe -Os -mfpmath=sse -msse2 -W -Wall -Wextra -Wpedantic -Wl,-z,relro -Wl,pie -Werror=format-security \
		-D_GLIBCXX_ASSERTIONS -fasynchronous-unwind-tables -fexceptions -fpie -fstack-protector-strong #\
		-fsanitize=address -fsanitize=undefined
CXXFLAGS     += -c ${BUILDINFO} -std=c++17 -pipe -Os -mfpmath=sse -msse2 -W -Wall -Wextra -Wpedantic -Wl,-z,relro -Wl,pie -Werror=format-security \
		-D_GLIBCXX_ASSERTIONS -fasynchronous-unwind-tables -fexceptions -fpie -fstack-protector-strong #\
		-fsanitize=address -fsanitize=leak -fsanitize=undefined
LDFLAGS	     += -lpthread #-fsanitize=address -fsanitize=leak -fsanitize=undefined
GMESHOBJECTS = geodentry.o bitflagarray.o primitive.o vertex.o vertexofface.o face.o sphere.o rectbox.o line.o \
               showprogress.o userinteraction.o printbuildinfo.o \
               polyline.o polyedge.o plane.o \
               triangularprism.o cube.o \
               triangle/triangle.o triangle/triangleTriangleIntersection.o \
               edgegeodesic.o marchingfront.o \
               meshio.o meshseedext.o mesh_params.o \
               vector3d.o matrix4d.o voxelfilter25d.o \
               meshinfodata.o \
               ../normalSphereSelection/IcoSphereTree.o
GMESHHEADERS = gmcommon.h geodentry.h bitflagarray.h primitive.h vertex.h vertexofface.h face.h sphere.h rectbox.h line.h \
               showprogress.h userinteraction.h printbuildinfo.h \
               polyline.h polyedge.h plane.h \
               triangularprism.h cube.h \
               triangle/triangle.h triangle/triangleTriangleIntersection.h \
               octree.h octnode.h \
               edgegeodesic.h marchingfront.h \
               meshio.h meshseedext.h mesh_params.h \
               vector3d.h matrix4d.h voxelfilter25d.h \
               meshinfodata.h \
               ../normalSphereSelection/IcoSphereTree.h

TIFFLIB = -ltiff

#==========================================================================
# Build on Ubuntu 18.04 LTS having gcc in version 7 as default:
# make -j8 CC=gcc-8 CXX=g++-8 LINK=g++-8
#==========================================================================

all: mesh \
     gigamesh-tolegacy \
     gigamesh-clean \
     gigamesh-info \
     meshgeneratefeaturevectors25d \
     gigamesh-featurevectors \
     matrix4d vector3d image2d voxelcuboid face

package-files: \
     gigamesh-featurevectors \
     gigamesh-clean \
     gigamesh-tolegacy \
     gigamesh-info

gigamesh-tolegacy: $(GMESHOBJECTS) $(GMESHHEADERS) vector3d.h octree.h octnode.h mesh_threads.o gigamesh-tolegacy.o voxelcuboid.o image2d.o Makefile
	$(CXX) $(TIFFLIB) -pthread -o gigamesh-tolegacy $(GMESHOBJECTS) mesh_threads.o gigamesh-tolegacy.o voxelcuboid.o image2d.o -lstdc++fs $(LDFLAGS)
	./gigamesh-tolegacy --version

gigamesh-clean: $(GMESHOBJECTS) $(GMESHHEADERS) vector3d.h octree.h octnode.h mesh_threads_psalm.o gigamesh-clean.o voxelcuboid.o image2d.o printbuildinfo.o Makefile
	$(CXX) $(TIFFLIB) -pthread -o gigamesh-clean $(GMESHOBJECTS) mesh_threads_psalm.o gigamesh-clean.o voxelcuboid.o image2d.o ../external/libpsalmBoostless/libpsalm.a -lstdc++fs $(LDFLAGS)
	./gigamesh-clean --version

gigamesh-info:  $(GMESHOBJECTS) $(GMESHHEADERS) vector3d.h octree.h octnode.h mesh_threads.o gigamesh-info.o voxelcuboid.o image2d.o printbuildinfo.o Makefile
	$(CXX) $(TIFFLIB) -pthread -o gigamesh-info $(GMESHOBJECTS) mesh_threads.o gigamesh-info.o voxelcuboid.o image2d.o -lstdc++fs $(LDFLAGS)
	./gigamesh-info --version

meshgeneratefeaturevectors25d: $(GMESHOBJECTS) $(GMESHHEADERS) mesh.o meshgeneratefeaturevectors25d.o image2d.o
	$(CXX) $(TIFFLIB)          -o meshgeneratefeaturevectors25d         $(GMESHOBJECTS) meshgeneratefeaturevectors25d.o         image2d.o mesh.o -lstdc++fs $(LDFLAGS)

# multi-threaded version fo meshgeneratefeaturevectors25d
gigamesh-featurevectors:  $(GMESHOBJECTS) $(GMESHHEADERS) mesh_threads.o meshgeneratefeaturevectors25d_threads.o image2d.o
	$(CXX) $(TIFFLIB) -pthread -o gigamesh-featurevectors               $(GMESHOBJECTS) meshgeneratefeaturevectors25d_threads.o image2d.o mesh_threads.o -lstdc++fs $(LDFLAGS)

meshgeneratefeaturevectors25d.o: meshgeneratefeaturevectors25d.cpp
	$(CXX) -I. -I..           $(CFLAGS) meshgeneratefeaturevectors25d.cpp -o meshgeneratefeaturevectors25d.o

meshgeneratefeaturevectors25d_threads.o: meshgeneratefeaturevectors25d.cpp
	$(CXX) -I. -I.. -DTHREADS $(CFLAGS) meshgeneratefeaturevectors25d.cpp -o meshgeneratefeaturevectors25d_threads.o

#-------------------------------------------------------------------------
# For testing
#-------------------------------------------------------------------------

mesh: $(GMESHOBJECTS) $(GMESHHEADERS) vector3d.h octree.h octnode.h mesh.o mesh_main.o voxelcuboid.o image2d.o Makefile
	$(CXX) $(TIFFLIB) -o mesh $(GMESHOBJECTS) mesh.o mesh_main.o voxelcuboid.o image2d.o -lstdc++fs $(LDFLAGS)

#-------------------------------------------------------------------------
# Mesh with and without Multi-threading and libpsalm
#-------------------------------------------------------------------------

mesh.o: mesh.h mesh.cpp
	$(CXX) -I. -I.. -I../voxel $(CFLAGS) mesh.cpp -o mesh.o $(LDFLAGS)

mesh_psalm.o: mesh.h mesh.cpp
	$(CXX) -I. -I.. -I../voxel -I../external/libpsalmBoostless/include -DLIBPSALM $(CFLAGS) mesh.cpp -o mesh_psalm.o $(LDFLAGS)

#../external/libpsalm/libpsalm.a
# NEW: ../external/libpsalmBoostless/libpsalm.a

mesh_threads.o: mesh.h mesh.cpp
	$(CXX) -I. -I.. -I../voxel -DTHREADS $(CFLAGS) mesh.cpp -o mesh_threads.o $(LDFLAGS)

mesh_threads_psalm.o: mesh.h mesh.cpp
	$(CXX) -I. -I.. -I../voxel -I../external/libpsalmBoostless/include -DTHREADS -DLIBPSALM $(CFLAGS) mesh.cpp -o mesh_threads_psalm.o $(LDFLAGS)

#../external/libpsalm/libpsalm.a
# NEW: ../external/libpsalmBoostless/libpsalm.a

#-------------------------------------------------------------------------

mesh_main.o: mesh.o mesh_main.c
	$(CXX) -I. -I.. -I../voxel $(CFLAGS) mesh_main.c -o mesh_main.o $(LDFLAGS)

gigamesh-tolegacy.o: mesh.o gigamesh-tolegacy.cpp
	$(CXX) -I. -I..  -DTHREADS $(CFLAGS) gigamesh-tolegacy.cpp -o gigamesh-tolegacy.o $(LDFLAGS)

gigamesh-clean.o: mesh_threads_psalm.o printbuildinfo.o gigamesh-clean.cpp
	$(CXX) -I. -I..  -DTHREADS -DLIBPSALM $(CFLAGS) gigamesh-clean.cpp -o gigamesh-clean.o $(LDFLAGS)

gigamesh-info.o: mesh.o printbuildinfo.o gigamesh-info.cpp
	$(CXX) -I. -I..  -DTHREADS $(CFLAGS) gigamesh-info.cpp -o gigamesh-info.o $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@ $(LDFLAGS)

#--- vectors and matrices ------------------------------------------------------------------

matrix4d: matrix4d.o matrix4d_main.o vector3d.o vector3d.h matrix4d.h
	$(CXX) -o matrix4d matrix4d.o matrix4d_main.o vector3d.o $(LDFLAGS)

face:   face.o face_main.o vertex.o vertexofface.o primitive.o plane.o vector3d.o matrix4d.o geodentry.o edgegeodesic.o bitflagarray.o line.o \
        edgegeodesic.h geodentry.h face.h vertex.h vertexofface.h primitive.h plane.h vector3d.h matrix4d.h bitflagarray.h line.h
	$(CXX) -o face face.o face_main.o vertex.o vertexofface.o primitive.o plane.o vector3d.o matrix4d.o geodentry.o edgegeodesic.o bitflagarray.o line.o $(LDFLAGS)

vector3d: matrix4d.o vector3d.o vector3d_main.o vector3d.h matrix4d.h
	$(CXX) -o vector3d vector3d.o vector3d_main.o matrix4d.o $(LDFLAGS)

matrix4d_main.o:
	$(CXX) $(CFLAGS) matrix4d_main.c -o matrix4d_main.o $(LDFLAGS)

vector3d_main.o:
	$(CXX) $(CFLAGS) vector3d_main.c -o vector3d_main.o $(LDFLAGS)

face_main.o:  
	$(CXX) $(CFLAGS) face_main.c -o face_main.o $(LDFLAGS)

#--- voxels ------------------------------------------------------------------

voxelcuboid: voxelcuboid_main.o voxelcuboid.o image2d.o vector3d.o matrix4d.o
	$(CXX) $(TIFFLIB) -I. -I.. -I../voxel -o voxelcuboid voxelcuboid_main.o voxelcuboid.o image2d.o vector3d.o matrix4d.o $(LDFLAGS)

voxelcuboid_main.o:
	$(CXX) -I. -I.. -I../voxel $(CFLAGS) ../voxel/voxelcuboid_main.c -o voxelcuboid_main.o $(LDFLAGS)

voxelcuboid.o:
	$(CXX) -I. -I../voxel $(CFLAGS) ../voxel/voxelcuboid.cpp -o voxelcuboid.o $(LDFLAGS)

#--- images ------------------------------------------------------------------

image2d: image2d.o image2d_main.o
	$(CXX) $(TIFFLIB) -o image2d image2d.o image2d_main.o $(LDFLAGS)

image2d_main.o:
	$(CXX) -I. -I.. $(CFLAGS) ../image2d_main.c -o image2d_main.o $(LDFLAGS)

image2d.o:
	$(CXX) -I. -I.. $(CFLAGS) ../image2d.cpp -o image2d.o $(LDFLAGS)

#--- CLEAN ------------------------------------------------------------------

clean:
	rm -f *.o *.a *.linkinfo a.out *~ image2d matrix4d mesh vector3d voxelcuboid
	rm -f triangle/*.o
	rm -f gigamesh-tolegacy
	rm -f gigamesh-clean
	rm -f gigamesh-info
	rm -f meshgeneratefeaturevectors
	rm -f meshgeneratefeaturevectors_threads
	rm -f meshgeneratefeaturevectors25d 
	rm -f gigamesh-featurevectors
