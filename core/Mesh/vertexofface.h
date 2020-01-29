#ifndef VERTEXOFFACE_H
#define VERTEXOFFACE_H

#include "vertex.h"

class Face;
struct s1RingSectorPrecomp;

class VertexOfFace : public Vertex {
	public:
		VertexOfFace();
		VertexOfFace( const int rSetIdx, const sVertexProperties& rSetProps );
		VertexOfFace( Vector3D vertPos );
		VertexOfFace( unsigned int rSetIdx, double rPosX, double rPosY, double rPosZ );
		~VertexOfFace();

		// Value access:
		virtual bool     estNormalAvgAdjacentFaces(); // ***
		virtual double   get1RingArea(); // ***
		virtual uint64_t get1RingFaceCount() const; // ***
		virtual double   get1RingSumAngles(); // ***

		// Fetch sphere:
		virtual Vertex*  advanceInSphere( double* sphereCenter, double radius, // ***
		                                  uint64_t* vertBitArrayVisited, std::set<Vertex*>* nextArray,
		                                  uint64_t* faceBitArrayVisited, bool rOrderToFuncVal=false, double* rSeqNr=NULL );
		virtual bool     mark1RingVisited( uint64_t* rVertBitArrayVisited, uint64_t* rFaceBitArrayVisited, // ***
		                                   bool           rOrderToFuncVal,      double*        rSeqNr );

		// Function value:
		virtual bool     isFuncValLocalMinimum(); // ***
		virtual bool     isFuncValLocalMaximum(); // ***
		virtual bool     funcValMedianOneRing( double* rMedianValue, double rMinDist );  // ***
		virtual bool     funcValMeanOneRing( double* rMeanValue, double rMinDist );  // ***

		// Feature vector smoothing:
		virtual bool     getFeatureVecMedianOneRing( std::vector<double>& rMedianValues, double rMinDist );
		virtual bool     getFeatureVecMeanOneRing(   std::vector<double>& rMeanValues,   double rMinDist );
		virtual bool     getFeatureVecMeanOneRing(   const double& rMinDist, const std::vector<s1RingSectorPrecomp>& r1RingSecPrecomp,
		                                             std::vector<double>& rMeanValues );

		// Precompute 1-ring sector
		virtual bool     get1RingSectors( const double& rNormDist, std::vector<s1RingSectorPrecomp>& r1RingSectorPrecomp );

        //Transformation:
        virtual Vertex* applyNormalShift(float offsetDistance, int index);

		// Neighbourhood - related to labeling!
		virtual void     getNeighbourVertices( std::set<Vertex*>* someVertList ); // ***
		virtual bool     getAdjacentVerticesExcluding( std::vector<Vertex*>* rSomeVerts, uint64_t* rVertBitArrayVisited ); // ***
		virtual Vertex*  getAdjacentNextBorderVertex( uint64_t* rVertBitArrayUnVisited ); // ***
		virtual Vertex*  getConnection( std::set<Vertex*>* someVertList, bool reverse ); // ***

		// Mesh setup:
		virtual void     connectToFace( Face* someFace ); // ***
		virtual void     disconnectFace( Face* someFace ); // ***
		virtual bool     isAdjacent( Face* someFace ); // ***
		virtual void     getFaces( Vertex* otherVert, std::set<Face*>* neighbourFaces, Face* callingFace ); // ***
		virtual void     getFaces( std::set<Face*>* someFaceList ); // ***
		virtual void     getFaces( std::vector<Face*>* someFaceList ); // ***

		// Mesh checking:
		virtual bool     belongsToFace(); // ***
		virtual int      connectedToFacesCount(); // returns how many faces are connected with this vertex // ***
		virtual bool     isSolo();                // true, when connected to ZERO faces and ZERO edges // ***
		virtual bool     isBorder();              // true, when the vertex is part of one or more border edges // ***
		virtual bool     isManifold(); // ***
		virtual bool     isNonManifold(); // ***
		virtual bool     isDoubleCone();          // checks for a vertex as tip of two cones (as this annoys labeling) // ***
		virtual bool     isPartOfZeroFace();      // checks if the vertex belongs to face having an area of zero // ***
		virtual bool     isInverse();             // check if this vertex is part of an edge connecting faces with improper orientation // ***

		// Debuging:
		virtual void     dumpInfo(); // ***

	private:
		// Neighbourhood:
		int           mAdjacentFacesNr;  //!< Number adjacent Faces.
		Face**        mAdjacentFaces;    //!< References to adjacent Faces.
};

#endif // VERTEXOFFACE_H
