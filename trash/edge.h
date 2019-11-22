#ifndef EDGE_H
#define EDGE_H

#include "primitive.h"

#include "heightmappixel.h"	

#include <list>
#include <set>

//#include "facelistextension.h"

class Vertex;	// circular dependency
class Face;	// circular dependency

//!
//! \brief Connection between two Vertices (Layer 0)
//!
//! Edges are created/introduced by Faces.
//! 
//! Remark: Edges described by this class are considered bidirectional unlike
//! introduced by faces, where edges are unidirectional. This consideration is
//! based on the fact that we get approximatley half of the (unidirectional) 
//! edges, which are introduced by faces.
//! Therefore do not rely on the direction implied by Vertex A to B as it is 
//! practically random!
//!
//! Layer 0
//!

class Edge : public Primitive/*, public FaceListExtension*/ {
	public:
		Edge( Vertex* setVertA, Vertex* setVertB, Face* setFace );
		Edge( Vertex* setVertA, Vertex* setVertB );
		~Edge();
	protected:
		void unSetValues();

	public:
		Vertex* getVertA();
		Vertex* getVertB();

		int      getType();

		// mesh-setup:
		bool connects( Vertex* someVert1, Vertex* someVert2 );
		bool connectsTo( Vertex* otherVertex );
		bool connectsTo( set<Vertex*>* otherVertices );

		// used for some mesh-checking:
/*
		bool isNonManifold();	// when connected to THREE OR MORE faces
		bool isManifold();		// when connected to TWO faces
		bool isBorder();		// when connected to ONE face 
		bool isSolo();			// when connected to ZERO faces
		int  getState();		// returns _PRIMITIVE_STATE_*
*/

		// neighbourhood:
//		Face*   getFaceExcluding( Face* faceToExclude ); // this ASSUMES MANIFOLDNESS - otherwise warnings/errors will be shown and NULL returned.
// 		Face*   getFace( Vertex* vertexIncluded );       // find a face consisting of this edge and a given vertex
// 		int     getFaces( set<Face*>* connectedFaces, Face* faceToExclude=NULL );   // works also for non-manifolds. faceToExclude is here an optional parameter.
		Vertex* getOpposingVertex( Vertex* thisVertex ); // returns the other vertex of the edge - similar to connectsTo( Vertex* otherVertex );

		// CUDA/OpenCL related - set/use pointers to external array holding (precalculated) values:
		//-----------------------------------------------------------------------------------------
		void     setCenterOfGravityRef( float* setCogOCL );
		// estimation of properties (ignoring any pre-calculated values)
		Vector3D estimateCenterOfGravity();
		// retrival of pre-calculated properties
		Vector3D getCenterOfGravity();
		//-----------------------------------------------------------------------------------------
		float    getLength();
		
		// methods for parallel processing:
		void  getVertCoordinates( float *coordArr );

		// for surface integrals:
		bool intersectsSphere1( Vector3D positionVec, float radius );
		bool intersectsSphere2( Vector3D positionVec, float radius );
		int  pointOfSphereLineIntersection( Vector3D positionVec, float radius, Vector3D* intersec1, Vector3D* intersec2 );
		int  pointOfSphereEdgeIntersection( Vector3D positionVec, float radius, Vector3D* intersec1, Vector3D* intersec2 );

		// rasterization / voxelization => volume integrals:
		vector<HeightMapPixel> rasterViewFromZ();

		// debuging:
		void dumpInfo( bool forMatlab=false, char* someNameTag=NULL );

	private:
		Vertex* vertA;  //!< reference to one out of the two vertices defining this edge. Remark: Edges are here not considered directional! 
		Vertex* vertB;  //!< reference to one out of the two vertices defining this edge. Remark: Edges are here not considered directional! 
};

#endif
