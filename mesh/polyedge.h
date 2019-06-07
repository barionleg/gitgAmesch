#ifndef POLYEDGE_H
#define POLYEDGE_H

#include "vertex.h"

#include "face.h"

//!
//! \brief Edge class for Polygonal lines. (Layer 0)
//!
//! ....
//! ....
//!
//! Layer 0
//!

class PolyEdge {
	friend class PolyLine;

	public:
		PolyEdge( Vertex* rSomeVert, Face* rFromSomeFace, Face::eEdgeNames rFromSomeEdge=Face::EDGE_NONE );
		~PolyEdge();

	// Position:
		bool    getPosition( Vector3D* rPos );
		bool    getPositionTransformed( Vector3D* rPos, const Matrix4D& rTransform );
		bool    getNormal( Vector3D* rNormal );

	// Function value:
		bool    setFuncValue( double  rVal );
		bool    getFuncValue( double* rVal ) const;

	private:
		Vertex* mVertPoly;        //!< Required reference to a Vertex defining the position in 3D.
		Face*   mFromFace;        //!< Optional reference to Face e.g. used to compute vertPoly.
		Face::eEdgeNames  mFromEdge;        //!< Optional reference to the edge of the Face fromFace.
		float   mCurvature;       //!< Optional curvature.
};

#endif
