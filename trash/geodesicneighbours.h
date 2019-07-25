#ifndef GEODESCINEIGHBOURS_H
#define GEODESCINEIGHBOURS_H

#include <map>
#include <list>
#include <set>
#include <sys/types.h>

#include "apsu.h"
#include "vertex.h"
#include "edge.h"
#include "face.h"

//!
//! \brief Class handling the geodesic neighbourhood. (Layer -1)
//!
//! Belongs to Vertex. 
//! 

class GeodesicNeighbours : private Apsu {

	public:
		// constructor and deconstructor:
		GeodesicNeighbours();
		~GeodesicNeighbours();

		// setup and estimation of the geodesic neighbourhood:
		void addVertex( pair<Vertex*,float> pairVertex ); // adds a vertex and its geodesic distance - prevents duplicates and keeps smallest distance 
		bool addEdge( Edge* frontEdge, Face* fromFace );  // also sorts!
		void addBorderEdge( Edge* frontEdge );            // organizes the boundary
		void addFace( Face* faceVisited );                // adds a face

		void get( Edge** frontEdge, float* distA, float* distB, Face** fromFace ); // returns first in stack - with smallest geodesic distance
		bool has( Edge* edgeSome );                                                // check if an edge is in the stack
		bool has( Face* faceVisited );                                             // check if a face already was visited
		bool remove( Edge* edgeSome );                                             // check if an edge is in the stack and remove it

		// retrieve information:
		map<Vertex*,float> getGeodesicDistances();   // returns all estimated geodesic distances
		float              getMaxGeodesicDistance(); // returns the maximum geodesic distances
		list<Vertex*>      getBorderVertices();      // returns the Vertices along the neighbourhoods border
		uint               sortedStackSize();        // size of the sorted stack (used during estimation)

		// debuging:
		void dumpHashInfo();

	private:
		struct geodesicFrontEdge { // internal pair 
			Edge* frontEdge;
			Face* fromFace;
		};
		vector<geodesicFrontEdge*> edgeListSorted;     // list to store the marching front (used during estimation - empty otherwise)
		map<Vertex*,float>         vertexGeodesicDist; // list with estimated geodesic distance
		vector<Edge*>              borderEdges;        // list of edges enclosing the geodeisc neighbours 
		set<Face*>                 facesIncluded;      // or faces visited - needed to prevent walking along the surface in circles as well to select faces for further processing

};

#endif
