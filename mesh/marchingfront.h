#ifndef MARCHINGFRONT_H
#define MARCHINGFRONT_H

#include <map>
#include <list>
#include <set>
#include <sys/types.h>

#include "vertexofface.h"
#include "face.h"

//!
//! \brief Class handling a marching front. (Layer -1)
//!
//! Either a Face or a Vertex can be used as a seed primitve.
//!
//! Typically called by a Mesh. 
//! 

class MarchingFront {

	public:
		// constructor and deconstructor:
// 		MarchingFront( set<Edge*> seedEdges );
		MarchingFront( Vertex*    useSeedVertex );
		MarchingFront( Face*      useSeedFace );
		~MarchingFront();

		// information retrival:
		std::set<Face*> getFrontFaces();
		std::set<Face*> getVisitedFaces();
		int getVisitedFaces( std::set<Face*>* someFaceList );

		// manipulation by n-rings:
		bool advanceOne();
		bool advanceMulti( int nrOfAdvances = 0 );
		// manipulation by n-rings and euclidian distance:
		bool advanceOneDistToCoord( float maxDist, float x, float y, float z, bool advanceOnParts );
		bool advanceMultiDistToCoord( float maxDist, float x, float y, float z, bool advanceOnParts=true );

	private:
		std::set<Face*>   facesVisited;  //!< Faces already visited.
		std::set<Face*>   frontFaces;    //!< Faces right before the marching front.
		Vertex*      seedVertex;    //!< Pointer to the seed Vertex, will be set when created with MarchingFront( Vertex* seedVertex ).
		Face*        seedFace;      //!< Pointer to the seed Face, will be set when created with MarchingFront( Face* seedFace ).

};

#endif
