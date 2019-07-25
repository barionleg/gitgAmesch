#include "edgelistextension.h"

EdgeListExtension::EdgeListExtension() {
	//! Constructor
}

EdgeListExtension::~EdgeListExtension() {
	//! Destructor
}

void EdgeListExtension::unSetValues() {
	//! Set alls pointer to NULL, values to not-a-number.
	//! Only to be called by destructors!
	edgeList.clear();
}

// basic information ---------------------------------------------------------

uint EdgeListExtension::edgeCount() {
	//! Returns the number of adjacent/related vertices (stored in the edgeList).
	return edgeList.size();
}

set<Edge*,connects> EdgeListExtension::getEdgeList() {
	//! Returns a list/vector of related/adjacent Vertices.
	return edgeList;
}

Edge* EdgeListExtension::getEdgeByIdx( int findIdx ) {
	//! Finds a Edge with a given index.
	//! Returns NULL if no Edge was found.
	set<Edge*>::iterator it;
	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) {	
		if( (*it)->getIndex() == findIdx ) {
			return (*it);
		}
	}
	return NULL;
}

Edge* EdgeListExtension::getEdgeByIdxOriginal( int findIdx ) {
	//! Finds a Edge with a given original(!!) index.
	//! Returns NULL if no Edge was found.
	set<Edge*>::iterator it;
	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) {	
		if( (*it)->getIndexOriginal() == findIdx ) {
			return (*it);
		}
	}
	return NULL;
}

// Neighbourhood - manipulation ------------------------------------------------

void EdgeListExtension::connectToEdge( Edge* adjacentEdge ) {
	//! Establishes a link to a Edge a EdgeListExtension relates to.
	//! Typically called when a Edge is created.
	edgeList.insert( adjacentEdge );
}

bool EdgeListExtension::disconnectEdge( Edge* belonged2Edge ) {
	//! Removes a link to a Edge a primitive relates to.
	//! Typically called when a Edge is deleted.
	bool executeErase = false;
	set<Edge*>::iterator it;
	for ( it=edgeList.begin(); it!=edgeList.end(); it++ ) {
		if( (*it) == belonged2Edge ) {	
			executeErase = true;
			//cout << "[" << getTypeStr() << "] " << getAidx() << "-" << getBidx() << " disconnect edge: " << (*it)->getIdx() << endl;
			break;
		}
	}
	if( executeErase ) {
		edgeList.erase( it );
	}
	return executeErase;
}

// simple Bounding Box properties ----------------------------------------------

float EdgeListExtension::edgeGetMinX() {
	//! Returns the minimum x-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minX = FLT_MAX;
	set<Edge*>::iterator it;

	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) { 
		if( (*it)->getX() < minX ) {
			minX = (*it)->getX();
		}
	}

	return minX;
};

float EdgeListExtension::edgeGetMaxX() {
	//! Returns the maximum x-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxX = -FLT_MAX;
	set<Edge*>::iterator it;

	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) { 
		if( (*it)->getX() > maxX ) {
			maxX = (*it)->getX();
		}
	}

	return maxX;
};

float EdgeListExtension::edgeGetMinY() {
	//! Returns the minimum y-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minY = FLT_MAX;
	set<Edge*>::iterator it;

	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) { 
		if( (*it)->getY() < minY ) {
			minY = (*it)->getY();
		}
	}

	return minY;
};

float EdgeListExtension::edgeGetMaxY() {
	//! Returns the maximum y-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxY = -FLT_MAX;
	set<Edge*>::iterator it;

	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) { 
		if( (*it)->getY() > maxY ) {
			maxY = (*it)->getY();
		}
	}

	return maxY;
};

float EdgeListExtension::edgeGetMinZ() {
	//! Returns the minimum z-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minZ = FLT_MAX;
	set<Edge*>::iterator it;

	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) { 
		if( (*it)->getZ() < minZ ) {
			minZ = (*it)->getZ();
		}
	}

	return minZ;
};

float EdgeListExtension::edgeGetMaxZ() {
	//! Returns the maximum z-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxZ = -FLT_MAX;
	set<Edge*>::iterator it;

	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) { 
		if( (*it)->getZ() > maxZ ) {
			maxZ = (*it)->getZ();
		}
	}

	return maxZ;
};

// adjacent edge texture map manipulation ----------------------------------

float EdgeListExtension::edgesTextureToGrayScale() {
	//! Sets all vertices of the edgeList (related vertices) to grayscale. 
	//! The average gray value is returned.
	//! Typically called for a Mesh.
	if( edgeList.size() == 0 ) {
		return _PRIMITIVE_HAS_EMPTY_EDGELIST_;
	}

	float averageGray = 0.0;
	set<Edge*>::iterator it;
	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) {	
		averageGray += (*it)->setToGrayScale();
	}

	return (averageGray/edgeList.size());
}

float EdgeListExtension::edgesTextureNormalize() {
	//! Equalizes the histogramm of the edgeList  (related vertices).
	//! The range used for equalization is returned.
	//! Typically called for a Mesh.
	if( edgeList.size() == 0 ) {
		return _PRIMITIVE_HAS_EMPTY_EDGELIST_;
	}

	float bufRGB, rangeRGB;
	float minRGB = 1.0;
	float maxRGB = 0.0;
	set<Edge*>::iterator it;

	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) {	
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

	cout << "[EdgeListExtension::edgesTextureNormalize] minRGB: " << minRGB << "(" << (minRGB*255) << ") maxRGB: " << maxRGB << endl;
	for( it=edgeList.begin(); it!=edgeList.end(); it++ ) {	
		(*it)->normalizeRGB( minRGB, rangeRGB );
	}
	return rangeRGB;
}

void EdgeListExtension::edgesTextureSetMonoRGB( float setR, float setG, float setB ) {
	//! Set all vertices within the edgeList to one RGB-color.
	set<Edge*>::iterator itEdge;
	for( itEdge=edgeList.begin(); itEdge!=edgeList.end(); itEdge++ ) {
		(*itEdge)->setRGB( setR, setG, setB );
	}
}

void EdgeListExtension::edgesTextureSetMonoHSV( float hue, float sat, float val ) {
	//! Set all vertices within the edgeList to one HSV-color.
	set<Edge*>::iterator itEdge;
	for( itEdge=edgeList.begin(); itEdge!=edgeList.end(); itEdge++ ) {
		(*itEdge)->setHSV( hue, sat, val );
	}
}

