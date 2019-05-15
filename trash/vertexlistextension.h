#ifndef VERTEXLISTEXTENSION_H
#define VERTEXLISTEXTENSION_H

#define _PRIMITIVE_HAS_EMPTY_VERTEXLIST_          -1

#include <set>

#include <cstdlib> // uint, calloc

#include "vertex.h"

//!
//! \brief Class to extend children of Primitve with a Vertex list. (Layer 0)
//!
//! Comfortable and list-like access to vertices. Eventually slow and memory
//! hungry, but convenient for prototyping. Used by Mesh.
//!
//! Layer 0
//!

class VertexListExtension {
	public:
		// constructor and deconstructor:
		VertexListExtension();
		~VertexListExtension();
	protected:
		void unSetValues();

	public:
		// basic information
		uint    vertexCount();

		set<Vertex*> getVertexList();

		Vertex* getVertexByIdx( int findIdx );
		Vertex* getVertexByIdxOriginal( int findIdx );

		// simple Bounding Box properties:
		float   vertexGetMinX();
		float   vertexGetMaxX();
		float   vertexGetMinY();
		float   vertexGetMaxY();
		float   vertexGetMinZ();
		float   vertexGetMaxZ();

		// adjacent primitive texture map manipulation
		float vertexTextureToGrayScale(); // sets all vertices of the vertexList to grayscale - returns the average
		float vertexTextureNormalize();   // histogramm equivalization for the vertexList - returns the range
		void  vertexTextureSetMonoRGB( float setR, float setG, float setB );
		void  vertexTextureSetMonoHSV( float hue, float sat=1.0, float val=1.0 );

		// Navigation from extern using internal iterator
		Vertex* vertexNavSetBegin();
		Vertex* vertexNavNext();

	protected:
		set<Vertex*> vertexList; //!< a list of related vertices (typically neighbours)
	private:
		set<Vertex*>::iterator itNavVertex; //! Internal Iteratur to avoid external iteration.
};

#endif
