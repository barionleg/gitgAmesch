#ifndef EDGELISTEXTENSION_H
#define EDGELISTEXTENSION_H

#define _PRIMITIVE_HAS_EMPTY_EDGELIST_          -1

#include <set>

#include <cstdlib> // uint, calloc

#include "edge.h"

struct connects {
	bool operator() ( Edge* lhs, Edge* rhs) const {
		bool edgeExists=false;
		//Vertex* vert1 = lhs->getVertA();
		edgeExists = not( lhs->connects( rhs->getVertA(), rhs->getVertB() ) );
		//edgeExists = not( rhs->connects( lhs->getVertA(), lhs->getVertB() ) );
		return edgeExists;
	}
};

//!
//! \brief Class to extend children of Primitve with a Edge list. (Layer 0)
//!
//! Comfortable and list-like access to vertices. Eventually slow and memory
//! hungry, but convenient for prototyping. Used by Mesh.
//!
//! Layer 0
//!

class EdgeListExtension {
	public:
		// constructor and deconstructor:
		EdgeListExtension();
		~EdgeListExtension();
	protected:
		void unSetValues();

	public:
		// basic information
		uint    edgeCount();

		set<Edge*,connects> getEdgeList();

		Edge* getEdgeByIdx( int findIdx );
		Edge* getEdgeByIdxOriginal( int findIdx );

		// Neighbourhood - manipulation
		void   connectToEdge( Edge* adjacentEdge );
		bool   disconnectEdge( Edge* belonged2Edge );

		// simple Bounding Box properties:
		float   edgeGetMinX();
		float   edgeGetMaxX();
		float   edgeGetMinY();
		float   edgeGetMaxY();
		float   edgeGetMinZ();
		float   edgeGetMaxZ();

		// adjacent primitive texture map manipulation
		float edgesTextureToGrayScale(); // sets all vertices of the edgeList to grayscale - returns the average
		float edgesTextureNormalize();   // histogramm equivalization for the edgeList - returns the range
		void  edgesTextureSetMonoRGB( float setR, float setG, float setB );
		void  edgesTextureSetMonoHSV( float hue, float sat=1.0, float val=1.0 );

	protected:
		set<Edge*,connects> edgeList; //!< a list of related vertices (typically neighbours)
};

#endif
