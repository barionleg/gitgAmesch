#ifndef FACELISTEXTENSION_H
#define FACELISTEXTENSION_H

#define _PRIMITIVE_HAS_EMPTY_FACELIST_          -1

#include <set>

#include <cstdlib> // uint, calloc

#include "face.h"


//!
//! \brief Class to extend children of Primitve with a Face list. (Layer 0)
//!
//! Comfortable and list-like access to vertices. Eventually slow and memory
//! hungry, but convenient for prototyping. Used by Mesh.
//!
//! Layer 0
//!

class FaceListExtension {
	public:
		// constructor and deconstructor:
		FaceListExtension();
		~FaceListExtension();
	protected:
		void unSetValues();

	public:
		// basic information
		uint    faceCount();

		set<Face*> getFaceList();

		Face* getFaceByIdx( int findIdx );
		Face* getFaceByIdxOriginal( int findIdx );

		// Neighbourhood - manipulation
		void   connectToFace( Face* adjacentFace );
		bool   disconnectFace( Face* belonged2Face );

		// simple Bounding Box properties:
		float   faceGetMinX();
		float   faceGetMaxX();
		float   faceGetMinY();
		float   faceGetMaxY();
		float   faceGetMinZ();
		float   faceGetMaxZ();

		// adjacent primitive texture map manipulation
		float facesTextureToGrayScale(); // sets all vertices of the faceList to grayscale - returns the average
		float facesTextureNormalize();   // histogramm equivalization for the faceList - returns the range
		void  facesTextureSetMonoRGB( float setR, float setG, float setB );
		void  facesTextureSetMonoHSV( float hue, float sat=1.0, float val=1.0 );

	protected:
		set<Face*> faceList; //!< a list of related vertices (typically neighbours)
};

#endif
