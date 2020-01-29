#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "vertex.h"
#include "face.h"
#include "geodentry.h"

// Circular dependencies:
class Vertex;
class Face;

//!
//! \brief Class handling edges having a geodesic distance class. (Layer -0.5)
//!
//! ...
//!
//! Layer -0.5
//!

class EdgeGeodesic {
	//friend class ...;

	public:
		// Constructor and deconstructor:
		EdgeGeodesic( Face* rFromFace, Face::eEdgeNames rEdgeIdx, GeodEntry* rGeoDistA, GeodEntry* rGeoDistB );
		~EdgeGeodesic() = default;

		// Information retrival
		Face*       getFace();
		bool        getGeoDistA( double* rGeodDist );
		bool        getGeoDistB( double* rGeodDist );
		bool        getGeodAngleA( double* rGeodAngle );
		bool        getGeodAngleB( double* rGeodAngle );
		GeodEntry*  getGeoDistARef();
		GeodEntry*  getGeoDistBRef();
		double      getGeoDistMin();
		double      getGeoDistMax();
		Face::eEdgeNames  getEdgeIdx();

		Vertex*     getVertA();
		Vertex*     getVertB();
		bool        getPositionA( Vector3D* vertAPos );
		bool        getPositionB( Vector3D* vertBPos );
		bool        getFuncValueA( double* funcValA );
		bool        getFuncValueB( double* funcValB );

		// Comparison - Heap related
		static bool shorterThan( EdgeGeodesic* edge1, EdgeGeodesic* edge2 );

		// Debuging:
		void        dumpInfo();

	private:
		Face*       mFromFace;
		Face::eEdgeNames mEdgeIdx; // Face::eEdgeNames
		GeodEntry*  mGeoDistA;
		GeodEntry*  mGeoDistB;
};

#endif
