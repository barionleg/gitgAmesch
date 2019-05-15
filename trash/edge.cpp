#include <iostream>
#include <sys/types.h>
#include <math.h>

#include "vertex.h"
#include "face.h"
#include "edge.h"

Edge::Edge( Vertex *setVertA, Vertex *setVertB, Face *setFace ) 
	: Primitive( _PRIMITIVE_NOT_INDEXED_ ) {
	//! Constructor
	vertA = setVertA;
	vertB = setVertB;
	// connect with other primitives:
// 	faceList.insert( setFace );
//	vertA->connectToEdge( this );
// 	vertB->connectToEdge( this );
}

Edge::Edge( Vertex* setVertA, Vertex* setVertB )
	: Primitive( _PRIMITIVE_NOT_INDEXED_ ) {
	//! Constructor
	vertA = setVertA;
	vertB = setVertB;
}

Edge::~Edge() {
	//! Destructor sets properties to not a number or NULL.
	//! See Edge::unSetValues()
	unSetValues();
}

void Edge::unSetValues() {
	//! Set alls pointer to NULL, values to not-a-number.
	//! Only to be called by destructors! 
	//! See als Primitive::unSetValues.
	Primitive::unSetValues();
	// disconnect from related or neighbouring primitives:
	//cout << "[Edge] destroyed => disconnect from Vertex " << vertA->getIdx() << " and " << vertB->getIdx() << endl;
// 	vertA->disconnectEdge( this );
// 	vertB->disconnectEdge( this );
// 	if( !isSolo() ) {
// 		cerr << "[Edge] destroyed without destroying faces first!" << endl;
// 	}

	// just in case we referer to an object still in the memory, but already destroyed:
	vertA  = NULL;
	vertB  = NULL;
}

Vertex* Edge::getVertA() {
	//! Returns the first Vertex of an Edge.
	//!
	//! Remark: as our approach generates non-directional Edges don't get fooled
	//! by the term 'first Vertex' as this order is random.
	return vertA;
}

Vertex* Edge::getVertB() {
	//! Returns the second Vertex of an Edge.
	//!
	//! Remark: as our approach generates non-directional Edges don't get fooled
	//! by the term 'second Vertex' as this order is random.
	return vertB;
}

int Edge::getType() {
	//! Return the type (=class) of this object as an id.
	return Primitive::IS_EDGE;
}

bool Edge::connects( Vertex* someVert1, Vertex* someVert2 ) {
	//! Return true if vertA and vertB equal the given vertices.
	//cout << vertA << " =? " << someVert1 << endl;
	if( ( vertA == someVert1 ) && ( vertB == someVert2 ) ) {
	//if( ( vertA->getIndexOriginal() == someVert1->getIndexOriginal() ) && ( vertB->getIndexOriginal() == someVert2->getIndexOriginal() ) ) {
		//cout << "x";
		return true;
	}
	if( ( vertA == someVert2 ) && ( vertB == someVert1 ) ) {
	//if( ( vertA->getIndexOriginal() == someVert2->getIndexOriginal() ) && ( vertB->getIndexOriginal() == someVert1->getIndexOriginal() ) ) {
		//cout << "x";
		return true;
	}
	//cout << "o";
	return false;
}

bool Edge::connectsTo( Vertex* otherVertex ) {
	//! Returns true, when the given Vertex references matches one of the two 
	//! Edges Vertices (A,B). Otherwise false is returned.
	//!
	//! The same method exists for a set of Vertices.
 
	if( vertA == otherVertex ) {
		return true;
	}
	if( vertB == otherVertex ) {
		return true;
	}
	return false;
}

bool Edge::connectsTo( set<Vertex*>* otherVertices ) {
	//! Returns true, when the one Vertex references within the list/set matches 
	//! one of the two  Edges Vertices (A,B). Otherwise false is returned.
	//!
	//! The same method exists for a single Vertex, which is faster when your
	//! list contains only one Vertex ;)

	set<Vertex*>::iterator it;
	for ( it=otherVertices->begin(); it!=otherVertices->end(); it++ ) {
		if( connectsTo( (*it) ) ) {
			return true;
		}
	}
	return false;
}

// used for some mesh-checking -------------------------------------------------

/*
bool Edge::isNonManifold() {	
	//! Returns true when connected to THREE OR MORE faces.
	//! Returns false when connected to less than three faces.

// 	if( faceList.size() >= 3 ) {
// 		return true;
// 	}
	return false;
}

bool Edge::isManifold() {	
	//! Returns true when connected to TWO faces.
	//! 
// 	if( faceList.size() == 2 ) {
// 		return true;
// 	}
	return false;
}

bool Edge::isBorder() {		// when connected to ONE faces
// 	if( faceList.size() == 1 ) {
// 		return true;
// 	}
	return false;
}

bool Edge::isSolo() {		// when connected to ZERO faces
// 	if( faceList.size() == 0 ) {
// 		return true;
// 	}
	return false;
}

int Edge::getState() {
	if( isSolo() ) {
		return  _PRIMITIVE_STATE_SOLO_;
	}
	if( isBorder() ) {
		return  _PRIMITIVE_STATE_BORDER_;
	}
	if( isManifold() ) {
		return  _PRIMITIVE_STATE_MANIFOLD_;
	}
	if( isNonManifold() ) {
		return  _PRIMITIVE_STATE_NON_MANIFOLD_;
	}
	// we should never reach this point:
// 	cerr << "[Edge] ERROR - negative value for size (" << faceList.size() << ")." << endl;
	return _PRIMITIVE_STATE_ERROR_;
}
*/

// neighbourhood ---------------------------------------------------------------

/*
Face* Edge::getFaceExcluding( Face* faceToExclude ) {
	if( isNonManifold() ) {
		cerr << "[Egde] bad call to getFaceExcluding - works only properly on manifold meshes!!!" << endl;
		return NULL;
	}
	set<Face*>::iterator it;
// 	for( it=faceList.begin(); it!=faceList.end(); it++ ) {
// 		if( (*it) != faceToExclude ) {
// 			return *it;
// 		}
// 	}
	cerr << "[Edge] getFaceExcluding might be called for a border or a solo edge." << endl;
	return NULL;
}
*/
/*
Face* Edge::getFace( Vertex* vertexIncluded ) {
	// find a face consisting of this edge and a given vertex
	// might cause problems if there is a non-manifold triangle consisting of 
	// the exact same vertices ... an extremly rare case and therefore ignored
	set<Face*>::iterator it;
// 	for( it=faceList.begin(); it != faceList.end(); it++ ) {
// 		if( (*it)->requiresVertex( vertexIncluded ) ) {	
// 			return (*it);
// 		}
// 	}
	return NULL;
}

int Edge::getFaces( set<Face*>* connectedFaces, Face* faceToExclude ) {
	int facesAdded = 0;
	set<Face*>::iterator it;
// 	for( it=faceList.begin(); it != faceList.end(); it++ ) {
// 		if( (*it) != faceToExclude ) {	
// 			connectedFaces->insert( (*it) );
// 			facesAdded++;
// 		}
// 	}
	return facesAdded;
}
*/
Vertex* Edge::getOpposingVertex( Vertex* thisVertex ) {
	//! Returns the pointer to the "other" Vertex of this Edge.
	//!
	//! Returns NULL when thisVertex is not part of this Edge.
	//!
	//! If thisVertex == A, then B will be returned.

	if( vertA == thisVertex ) {
		return vertB;
	}
	if( vertB == thisVertex ) {
		return vertA;
	}
	return NULL;
}

// CUDA/OpenCL related - set/use pointers to external array holding (precalculated) values:
//-----------------------------------------------------------------------------------------

void Edge::setCenterOfGravityRef( float* setCogOCL ) {
	//! Sets pointer to array holding the center of gravity.
	cogXYZ = setCogOCL;
}

// estimation of properties (ignoring any pre-calculated values) ---------------

Vector3D Edge::estimateCenterOfGravity() {
	//! Estimates the length of the Edge and returns its length.
	//! getCOG has to be a pointer to an array of floats having a length of 3.
	//!
	//! Can be used to validate any other estimation (e.g. by CUDA/OpenCL).

	double cogX = ( vertA->getX() + vertB->getX() ) / 2;
	double cogY = ( vertA->getY() + vertB->getY() ) / 2;
	double cogZ = ( vertA->getZ() + vertB->getZ() ) / 2;

	if( cogXYZ != NULL ) {
		cogXYZ[0] = cogX;
		cogXYZ[1] = cogY;
		cogXYZ[2] = cogZ;
	}

	return Vector3D( cogX, cogY, cogZ, 1.0 );
}

// retrival of pre-calculated properties ---------------------------------------

Vector3D Edge::getCenterOfGravity() {
	//! Returns the pre-calculated center of gravity of the Edge stored in an 
	//! referenced array. If no external, pre-calculated COG is refered it
	//! will be estimated (slow).
	//!
	//! Remark: after operations like translation or rotation the center of 
	//! gravity has to be re-estimated.
	float cogX;
	float cogY;
	float cogZ;
	if( cogXYZ != NULL ) {
		cogX = cogXYZ[0];
		cogY = cogXYZ[1];
		cogZ = cogXYZ[2];
	} else {
		cogX = ( vertA->getX() + vertB->getX() ) / 2.0;
		cogY = ( vertA->getY() + vertB->getY() ) / 2.0;
		cogZ = ( vertA->getZ() + vertB->getZ() ) / 2.0;
	}
	return Vector3D( cogX, cogY, cogZ, 1.0 );
}

//---------------------------------------------------------------------------------

float Edge::getLength() {
	//! Estimates the length of the Edge and returns its length.
	//!
	//! Can be used to validate any other estimation (e.g. by CUDA/OpenCL).

	float dX, dY, dZ, len;

	dX = vertB->getX() - vertA->getX();
	dY = vertB->getY() - vertA->getY();
	dZ = vertB->getZ() - vertA->getZ();
	len = sqrt( dX*dX + dY*dY + dZ*dZ );

	return len;
}

// methods for parallel processing ---------------------------------------------

void Edge::getVertCoordinates( float *coordArr ) {
	//! Returns an array of length 6 with the veritces coordinates (xA,yA,zA,xB,yB,zB).
	coordArr[0] = vertA->getX();
	coordArr[1] = vertA->getY();
	coordArr[2] = vertA->getZ();
	coordArr[3] = vertB->getX();
	coordArr[4] = vertB->getY();
	coordArr[5] = vertB->getZ();
}

// for surface integrals -------------------------------------------------------

bool Edge::intersectsSphere1( Vector3D positionVec, float radius ) {
	//! Tests if the Edge intersects with a given sphere.
	//! Method (1) by exclusion.
	//!
	//! Consider the three cases shown here:
	//! http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	// Shift our vertices A, B and P (=positionVec), so that the 
	// sphere's center P equals the origin, which makes some estimations
	// simpler and faster:
	Vector3D vecA0Pos = vertA->getPositionVector() - positionVec;
	Vector3D vecB0Pos = vertB->getPositionVector() - positionVec;

	float vecALen3 = vecA0Pos.getLength3();
	float vecBLen3 = vecB0Pos.getLength3();

	// A on the spheres surface:
	if( vecALen3 == radius ) {
		return true;
	}
	// B on the spheres surface: 
	if( vecBLen3 == radius ) {
		return true;
	}

	// A inside and B outside the sphere:
	if( ( vecALen3 < radius ) && ( vecBLen3 > radius ) ) {
		return true;
	}

	// B inside and A outside the sphere:
	if( ( vecBLen3 < radius ) && ( vecALen3 > radius ) ) {
		return true;
	}

	// A and B inside the sphere:
	if( ( vecALen3 < radius ) && ( vecBLen3 < radius ) ) {
		return false;
	}

	// still there is one more case, where the Edges points are outside
	// the sphere, but the edge intersects it - we can find these edges by:

	// 1. estimation of the minimum distance between the line (defined 
	// by the edge) and the spheres center:

	// as we shifted everytthing, so that positionVec = ( 0,0,0 ) we can simplify
	// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
	// to:
	float lenAB   = abs3( ( vecB0Pos - vecA0Pos ) );
	float distMin = abs3( ( ( vecB0Pos - vecA0Pos ) % ( vecA0Pos ) ) ) / lenAB;
	if( distMin > radius ) {
		return false;
	}
	//cout << "[Edge] intersectsSphere distMin: " << distMin << " < radius: " << radius << endl;

	// 2. find the point P' on the line closest to the sphere has to be between
	// the vertices of the edges. inspired by:
	// http://planetmath.org/encyclopedia/Project.html
	float angleABP = angle( vecB0Pos - vecA0Pos, -vecA0Pos ); // remember: P = ( 0,0,0 )
	if( abs( angleABP ) > M_PI/2.0 ) { // > 90° => P' (P projected on line AB) is outside the edge segment A-B 
		return false;
	}
	//cout << "[Edge] intersectsSphere angleABP: " << angleABP*180.0/M_PI << "°" << endl;
	//cout << "[Edge] intersectsSphere " << vecALen3 * cos( angleABP ) << " < " << lenAB << endl;
	if( vecALen3 * cos( angleABP ) < lenAB ) {
		cout << "[Edge] intersectsSphere INTERSECT."<< endl;
		return true;
	}

	// otherwise no intersection:
	return false;
}

bool Edge::intersectsSphere2( Vector3D positionVec, float radius ) {
	//! Tests if the Edge intersects with a given sphere.
	//! Method (2) by estimation of intersection points and 
	//! assuring that one of the intersections is on the egde
	//! between A and B (= line segment).

	Vector3D interSec1;
	Vector3D interSec2;
	int noIntersectionAtAll;
	
	noIntersectionAtAll = pointOfSphereEdgeIntersection( positionVec, radius, &interSec1, &interSec2 );
	if( noIntersectionAtAll == 0 ) {
		return false;
	}

	return true;
}

int Edge::pointOfSphereLineIntersection( Vector3D positionVec, float radius, Vector3D* interSec1, Vector3D* interSec2 ) {
	//! Estimates the points of intersection of the edge with a given sphere.
	//! Returns false if no intersection is found;
	//!
	//! See: http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	// Shift our vertices A, B and P (=positionVec), so that A equals the origin, 
	// which makes some estimations simpler and faster. 
	// According to wiki: "Equation for a line starting at (0,0,0)"
	Vector3D vecLineUnit  = normalize3( vertB->getPositionVector() - vertA->getPositionVector() );
	Vector3D sphereCenter = positionVec - vertA->getPositionVector();

	float b = pow( dot3( vecLineUnit, sphereCenter ), 2 ) - dot3( sphereCenter, sphereCenter ) + pow( radius, 2 );
	// There is no solution if sqrt( b ) is imaginary, which is the case when:
	if( b < 0.0 ) {
		//cout << "[Edge] pointOfSphereIntersection: NO interseciont." << endl;
		return 0;
	}

	float a = dot3( vecLineUnit, sphereCenter );
	float d1 = a + sqrt( b );
	float d2 = a - sqrt( b );

	Vector3D interSection1 = vecLineUnit * d1 + vertA->getPositionVector();
	Vector3D interSection2 = vecLineUnit * d2 + vertA->getPositionVector();
	//interSection1.dumpInfo();
	//interSection2.dumpInfo();
	interSec1->set( &interSection1 );
	interSec2->set( &interSection2 );

	// The rare case - the line is a tangent:
	if( d1 == d2 ) {
		return 1;
	}

	return 2;
}

int Edge::pointOfSphereEdgeIntersection( Vector3D positionVec, float radius, Vector3D* interSec1, Vector3D* interSec2 ) {
	//! Estimates the points of intersection of the line defined by the edge with a given sphere.
	//! Returns number of intersections (0,1,2 - everything else is bogus!).
	//!
	//! See: http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	// Shift our vertices A, B and P (=positionVec), so that A equals the origin, 
	// which makes some estimations simpler and faster. 
	// According to wiki: "Equation for a line starting at (0,0,0)"
	Vector3D vecLineUnit  = normalize3( vertB->getPositionVector() - vertA->getPositionVector() );
	Vector3D sphereCenter = positionVec - vertA->getPositionVector();

	float b = pow( dot3( vecLineUnit, sphereCenter ), 2 ) - dot3( sphereCenter, sphereCenter ) + pow( radius, 2 );
	// There is no solution if sqrt( b ) is imaginary, which is the case when:
	if( b < 0.0 ) {
		//cout << "[Edge] pointOfSphereIntersection: NO intersection." << endl;
		return 0;
	}

	float a = dot3( vecLineUnit, sphereCenter );
	float d1 = a + sqrt( b );
	float d2 = a - sqrt( b );
	float dMax = abs3( vertB->getPositionVector() - vertA->getPositionVector() );
	Vector3D interSection1 = vecLineUnit * d1 + vertA->getPositionVector();
	Vector3D interSection2 = vecLineUnit * d2 + vertA->getPositionVector();

	// only d1 is probably valid, so we have only one or no intersection:
	if( ( d2 < 0.0 ) || ( d2 > dMax ) ) {
		// check if d1 is in a valid range or not:
		if( ( d1 < 0.0 ) || ( d1 > dMax ) ) {
			return 0;
		} else {
			interSec1->set( &interSection1 );
			return 1;
		}
	} else {
		// d2 is valid, so we have to check about d1 as well
		if( ( d1 < 0.0 ) || ( d1 > dMax ) ) {
			interSec1->set( &interSection2 );
			return 1;
		} else {
			interSec1->set( &interSection1 );
			interSec2->set( &interSection2 );
			return 2;
		}
	}
	cerr << "[Edge] pointOfSphereEdgeIntersection IMPOSSIBLE SOLUTION." << endl;
	return -1;
}

// rasterization / voxelization => volume integrals ----------------------------

vector<HeightMapPixel> Edge::rasterViewFromZ() {
	//! Rasters (renders) the top-view of an Edge for use in a height map.
	//!
	//! Will be called typically from Mesh::rasterViewFromZ
	//!
	//! see: http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#Optimization
	//!
	//! for our purpose an update to:
	//! http://lifc.univ-fcomte.fr/home/~ededu/projects/bresenham/ (Eugen Dedu)
	//! may make sense as we will get a more precise volume. The question is:
	//! do we need this extra bit of accuracy? 

	vector<HeightMapPixel> pixelList;
	HeightMapPixel singlePixel;

	float x0 = vertA->getX();
	float x1 = vertB->getX();
	float y0 = vertA->getY();
	float y1 = vertB->getY();

	bool steep = abs(y1 - y0) > abs(x1 - x0);

	if( steep ) {
		// swap(x0, y0), swap(x1, y1)
		x0 = vertA->getY();
		y0 = vertA->getX();
		x1 = vertB->getY();
		y1 = vertB->getX();
	}

	if( x0 > x1 ) {
		// swap(x0, x1), swap(y0, y1)
		float tmp;
		tmp = x1; x1 = x0; x0 = tmp;  
		tmp = y1; y1 = y0; y0 = tmp;  
	}

	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);
	int error  = deltax / 2;
	int ystep;
	int y = y0;
	
	if( y0 < y1 ) {
		ystep = 1;
	} else { 
		ystep = -1;
	}

	// for x from x0 to x1
	for( float x=x0; x<x1; x++ ) {		
		if( steep ) {
			// plot(y,x)
			singlePixel.x = y;
			singlePixel.y = x;
			singlePixel.z = 0.0;
			pixelList.push_back( singlePixel );
		} else {
			// plot(x,y)
			singlePixel.x = x;
			singlePixel.y = y;
			singlePixel.z = 0.0;
			pixelList.push_back( singlePixel );
		}
		error -= deltay;
		if( error < 0 ) {
			y += ystep;
             error += deltax;
		}
	}

	return pixelList;
}

// debuging --------------------------------------------------------------------

void Edge::dumpInfo( bool forMatlab, char* someNameTag ) {
	//! Dumps some basic information to stdout.

	cout << "[Edge]: " << someNameTag << " " << vertA->getIndex() << "-" << vertB->getIndex() << endl;

	if( forMatlab ) {
		vertA->getPositionVector().dumpInfo( true, (char*) "vertA" );
		vertB->getPositionVector().dumpInfo( true, (char*) "vertB" );
		return;
	}

	cout << "        faces: ";
	set<Face*>::iterator it;
// 	for( it=faceList.begin(); it!=faceList.end(); it++ ) {
// 		cout << (*it)->getIndex() << ", ";
// 	}
// 	cout << endl;
}


