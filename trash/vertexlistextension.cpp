#include "vertexlistextension.h"

VertexListExtension::VertexListExtension() {
	//! Constructor
}

VertexListExtension::~VertexListExtension() {
	//! Destructor
}

void VertexListExtension::unSetValues() {
	//! Set alls pointer to NULL, values to not-a-number.
	//! Only to be called by destructors!
	vertexList.clear();
}

// basic information ---------------------------------------------------------

uint VertexListExtension::vertexCount() {
	//! Returns the number of adjacent/related vertices (stored in the vertexList).
	return vertexList.size();
}

set<Vertex*> VertexListExtension::getVertexList() {
	//! Returns a list/vector of related/adjacent Vertices.
	return vertexList;
}

Vertex* VertexListExtension::getVertexByIdx( int findIdx ) {
	//! Finds a Vertex with a given index.
	//! Returns NULL if no Vertex was found.
	set<Vertex*>::iterator it;
	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) {	
		if( (*it)->getIndex() == findIdx ) {
			return (*it);
		}
	}
	return NULL;
}

Vertex* VertexListExtension::getVertexByIdxOriginal( int findIdx ) {
	//! Finds a Vertex with a given original(!!) index.
	//! Returns NULL if no Vertex was found.
	set<Vertex*>::iterator it;
	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) {	
		if( (*it)->getIndexOriginal() == findIdx ) {
			return (*it);
		}
	}
	return NULL;
}

// simple Bounding Box properties ----------------------------------------------

float VertexListExtension::vertexGetMinX() {
	//! Returns the minimum x-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minX = FLT_MAX;
	set<Vertex*>::iterator it;

	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) { 
		if( (*it)->getX() < minX ) {
			minX = (*it)->getX();
		}
	}

	return minX;
};

float VertexListExtension::vertexGetMaxX() {
	//! Returns the maximum x-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxX = -FLT_MAX;
	set<Vertex*>::iterator it;

	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) { 
		if( (*it)->getX() > maxX ) {
			maxX = (*it)->getX();
		}
	}

	return maxX;
};

float VertexListExtension::vertexGetMinY() {
	//! Returns the minimum y-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minY = FLT_MAX;
	set<Vertex*>::iterator it;

	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) { 
		if( (*it)->getY() < minY ) {
			minY = (*it)->getY();
		}
	}

	return minY;
};

float VertexListExtension::vertexGetMaxY() {
	//! Returns the maximum y-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxY = -FLT_MAX;
	set<Vertex*>::iterator it;

	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) { 
		if( (*it)->getY() > maxY ) {
			maxY = (*it)->getY();
		}
	}

	return maxY;
};

float VertexListExtension::vertexGetMinZ() {
	//! Returns the minimum z-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minZ = FLT_MAX;
	set<Vertex*>::iterator it;

	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) { 
		if( (*it)->getZ() < minZ ) {
			minZ = (*it)->getZ();
		}
	}

	return minZ;
};

float VertexListExtension::vertexGetMaxZ() {
	//! Returns the maximum z-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxZ = -FLT_MAX;
	set<Vertex*>::iterator it;

	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) { 
		if( (*it)->getZ() > maxZ ) {
			maxZ = (*it)->getZ();
		}
	}

	return maxZ;
};

// adjacent vertex texture map manipulation ----------------------------------

float VertexListExtension::vertexTextureToGrayScale() {
	//! Sets all vertices of the vertexList (related vertices) to grayscale. 
	//! The average gray value is returned.
	//! Typically called for a Mesh.
	if( vertexList.size() == 0 ) {
		return _PRIMITIVE_HAS_EMPTY_VERTEXLIST_;
	}

	float averageGray = 0.0;
	set<Vertex*>::iterator it;
	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) {	
		averageGray += (*it)->setToGrayScale();
	}

	return (averageGray/vertexList.size());
}

float VertexListExtension::vertexTextureNormalize() {
	//! Equalizes the histogramm of the vertexList  (related vertices).
	//! The range used for equalization is returned.
	//! Typically called for a Mesh.
	if( vertexList.size() == 0 ) {
		return _PRIMITIVE_HAS_EMPTY_VERTEXLIST_;
	}

	float bufRGB, rangeRGB;
	float minRGB = 1.0;
	float maxRGB = 0.0;
	set<Vertex*>::iterator it;

	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) {	
		bufRGB = (*it)->getMinRGB();
		//cout << "getMinRGB: " << bufRGB << endl;
		if( bufRGB < minRGB ) {
			minRGB = bufRGB;
		}
		bufRGB = (*it)->getMaxRGB();
		//cout << "getMaxRGB: " << bufRGB << endl;
		if( bufRGB > maxRGB ) {
			maxRGB = bufRGB;
		}
	}
	rangeRGB = maxRGB - minRGB;

	cout << "[VertexListExtension::vertexTextureNormalize] minRGB: " << minRGB << "(" << (minRGB*255) << ") maxRGB: " << maxRGB << endl;
	for( it=vertexList.begin(); it!=vertexList.end(); it++ ) {	
		(*it)->normalizeRGB( minRGB, rangeRGB );
	}
	return rangeRGB;
}

void VertexListExtension::vertexTextureSetMonoRGB( float setR, float setG, float setB ) {
	//! Set all vertices within the vertexList to one RGB-color.
	set<Vertex*>::iterator itVertex;
	for( itVertex=vertexList.begin(); itVertex!=vertexList.end(); itVertex++ ) {
		(*itVertex)->setRGB( setR, setG, setB );
	}
}

void VertexListExtension::vertexTextureSetMonoHSV( float hue, float sat, float val ) {
	//! Set all vertices within the vertexList to one HSV-color.
	set<Vertex*>::iterator itVertex;
	for( itVertex=vertexList.begin(); itVertex!=vertexList.end(); itVertex++ ) {
		(*itVertex)->setHSV( hue, sat, val );
	}
}

// --- Navigation from extern using internal iterator -------------------------------------------------------------------------

Vertex* VertexListExtension::vertexNavSetBegin() {	
	//! Set the internal iterator to the beginning of the Vertex list.
	//! See also vertexNavNext().
	return *vertexList.begin();
}

Vertex* VertexListExtension::vertexNavNext() {
	//! Push the internal Iterator forward.
	//! See also vertexNavSetBegin().
	//! \returns Pointer to Vertex or NULL, when the end of the list is reached.
	itNavVertex++;
 	if( itNavVertex == vertexList.end() ) {
 		return NULL;
 	}
	return (*itNavVertex);
}
