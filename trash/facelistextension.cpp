#include "facelistextension.h"

FaceListExtension::FaceListExtension() {
	//! Constructor
}

FaceListExtension::~FaceListExtension() {
	//! Destructor
}

void FaceListExtension::unSetValues() {
	//! Set alls pointer to NULL, values to not-a-number.
	//! Only to be called by destructors!
	faceList.clear();
}

// basic information ---------------------------------------------------------

uint FaceListExtension::faceCount() {
	//! Returns the number of adjacent/related vertices (stored in the faceList).
	return faceList.size();
}

set<Face*> FaceListExtension::getFaceList() {
	//! Returns a list/vector of related/adjacent Vertices.
	return faceList;
}

Face* FaceListExtension::getFaceByIdx( int findIdx ) {
	//! Finds a Face with a given index.
	//! Returns NULL if no Face was found.
	set<Face*>::iterator it;
	for( it=faceList.begin(); it!=faceList.end(); it++ ) {	
		if( (*it)->getIndex() == findIdx ) {
			return (*it);
		}
	}
	return NULL;
}

Face* FaceListExtension::getFaceByIdxOriginal( int findIdx ) {
	//! Finds a Face with a given original(!!) index.
	//! Returns NULL if no Face was found.
	set<Face*>::iterator it;
	for( it=faceList.begin(); it!=faceList.end(); it++ ) {	
		if( (*it)->getIndexOriginal() == findIdx ) {
			return (*it);
		}
	}
	return NULL;
}

// Neighbourhood - manipulation ------------------------------------------------

void FaceListExtension::connectToFace( Face* adjacentFace ) {
	//! Establishes a link to a Face a FaceListExtension relates to.
	//! Typically called when a Face is created.
	faceList.insert( adjacentFace );
}

bool FaceListExtension::disconnectFace( Face* belonged2Face ) {
	//! Removes a link to a Face a primitive relates to.
	//! Typically called when a Face is deleted.
	bool executeErase = false;
	set<Face*>::iterator it;
	for ( it=faceList.begin(); it!=faceList.end(); it++ ) {
		if( (*it) == belonged2Face ) {	
			executeErase = true;
			//cout << "[" << getTypeStr() << "] " << getAidx() << "-" << getBidx() << " disconnect face: " << (*it)->getIdx() << endl;
			break;
		}
	}
	if( executeErase ) {
		faceList.erase( it );
	}
	return executeErase;
}

// simple Bounding Box properties ----------------------------------------------

float FaceListExtension::faceGetMinX() {
	//! Returns the minimum x-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minX = FLT_MAX;
	set<Face*>::iterator it;

	for( it=faceList.begin(); it!=faceList.end(); it++ ) { 
		if( (*it)->getX() < minX ) {
			minX = (*it)->getX();
		}
	}

	return minX;
};

float FaceListExtension::faceGetMaxX() {
	//! Returns the maximum x-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxX = -FLT_MAX;
	set<Face*>::iterator it;

	for( it=faceList.begin(); it!=faceList.end(); it++ ) { 
		if( (*it)->getX() > maxX ) {
			maxX = (*it)->getX();
		}
	}

	return maxX;
};

float FaceListExtension::faceGetMinY() {
	//! Returns the minimum y-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minY = FLT_MAX;
	set<Face*>::iterator it;

	for( it=faceList.begin(); it!=faceList.end(); it++ ) { 
		if( (*it)->getY() < minY ) {
			minY = (*it)->getY();
		}
	}

	return minY;
};

float FaceListExtension::faceGetMaxY() {
	//! Returns the maximum y-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxY = -FLT_MAX;
	set<Face*>::iterator it;

	for( it=faceList.begin(); it!=faceList.end(); it++ ) { 
		if( (*it)->getY() > maxY ) {
			maxY = (*it)->getY();
		}
	}

	return maxY;
};

float FaceListExtension::faceGetMinZ() {
	//! Returns the minimum z-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float minZ = FLT_MAX;
	set<Face*>::iterator it;

	for( it=faceList.begin(); it!=faceList.end(); it++ ) { 
		if( (*it)->getZ() < minZ ) {
			minZ = (*it)->getZ();
		}
	}

	return minZ;
};

float FaceListExtension::faceGetMaxZ() {
	//! Returns the maximum z-coordinate of the list of vertices.
	//!
	//! Bounding Box related.

	float maxZ = -FLT_MAX;
	set<Face*>::iterator it;

	for( it=faceList.begin(); it!=faceList.end(); it++ ) { 
		if( (*it)->getZ() > maxZ ) {
			maxZ = (*it)->getZ();
		}
	}

	return maxZ;
};

// adjacent face texture map manipulation ----------------------------------

float FaceListExtension::facesTextureToGrayScale() {
	//! Sets all vertices of the faceList (related vertices) to grayscale. 
	//! The average gray value is returned.
	//! Typically called for a Mesh.
	if( faceList.size() == 0 ) {
		return _PRIMITIVE_HAS_EMPTY_FACELIST_;
	}

	float averageGray = 0.0;
	set<Face*>::iterator it;
	for( it=faceList.begin(); it!=faceList.end(); it++ ) {	
		averageGray += (*it)->setToGrayScale();
	}

	return (averageGray/faceList.size());
}

float FaceListExtension::facesTextureNormalize() {
	//! Equalizes the histogramm of the faceList  (related vertices).
	//! The range used for equalization is returned.
	//! Typically called for a Mesh.
	if( faceList.size() == 0 ) {
		return _PRIMITIVE_HAS_EMPTY_FACELIST_;
	}

	float bufRGB, rangeRGB;
	float minRGB = 1.0;
	float maxRGB = 0.0;
	set<Face*>::iterator it;

	for( it=faceList.begin(); it!=faceList.end(); it++ ) {	
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

	cout << "[FaceListExtension::facesTextureNormalize] minRGB: " << minRGB << "(" << (minRGB*255) << ") maxRGB: " << maxRGB << endl;
	for( it=faceList.begin(); it!=faceList.end(); it++ ) {	
		(*it)->normalizeRGB( minRGB, rangeRGB );
	}
	return rangeRGB;
}

void FaceListExtension::facesTextureSetMonoRGB( float setR, float setG, float setB ) {
	//! Set all vertices within the faceList to one RGB-color.
	set<Face*>::iterator itFace;
	for( itFace=faceList.begin(); itFace!=faceList.end(); itFace++ ) {
		(*itFace)->setRGB( setR, setG, setB );
	}
}

void FaceListExtension::facesTextureSetMonoHSV( float hue, float sat, float val ) {
	//! Set all vertices within the faceList to one HSV-color.
	set<Face*>::iterator itFace;
	for( itFace=faceList.begin(); itFace!=faceList.end(); itFace++ ) {
		(*itFace)->setHSV( hue, sat, val );
	}
}

